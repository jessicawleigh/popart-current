/*
 * MapView.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: jleigh
 */

#include "MapView.h"

// Qt headers
#include <QtAlgorithms>
#include <QButtonGroup>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPushButton>
#include <QRadioButton>
#include <QSvgGenerator>

// Marble headers
#include <GeoDataDocument.h>
#include <GeoDataCoordinates.h>
#include <GeoDataLineString.h>
#include <GeoDataLinearRing.h>
#include <GeoDataTreeModel.h>
#include <GeoDataStyle.h>
#include <GeoDataIconStyle.h>
#include <GeoDataLineStyle.h>
#include <GeoDataPolyStyle.h>
#include <MarbleModel.h>
#include <SearchRunnerManager.h>
using namespace Marble;

#include <iostream>
using namespace std;

const ColourTheme::Theme MapView::_defaultTheme = ColourTheme::Greyscale;

MapView::MapView(QWidget *parent)
  : QWidget(parent), _mapWidget(0), _hapLayer(0), _legendDlg(0)
{
  _currentTheme = _defaultTheme;
  setColourTheme();
  _externalLegend = false;
  _mapWidget = new MarbleWidget(this);
  setupWidget();
}

MapView::~MapView()
{
  if (! _locations.empty())  clearHapLocations();
}


void MapView::setupWidget()
{
  _mapWidget->setProjection(Mercator);
  _mapWidget->setMapThemeId("earth/plain/plain.dgml");
  _mapWidget->setShowOverviewMap(false);
  _mapWidget->setShowScaleBar(false);
  _mapWidget->setShowCompass(false);

  _zoomSlider = new QSlider(Qt::Horizontal, this);
  _zoomSlider->setMinimum(1000);
  _zoomSlider->setMaximum(2400);

  _mapWidget->zoomView(_zoomSlider->value());


  // Centre this on middle of haplotype locations, and zoom appropriately
  // lon, lat, alt, unit, detail
  GeoDataCoordinates home(-60.0, -10.0, 0.0, GeoDataCoordinates::Degree);
  _mapWidget->centerOn(home);

  QObject::connect(_mapWidget, SIGNAL(mouseMoveGeoPosition(QString)),
                    this, SLOT(updateGeoPosition(QString)));

  connect(_zoomSlider, SIGNAL(valueChanged(int)), _mapWidget, SLOT(zoomView(int)));
  connect(_mapWidget, SIGNAL(zoomChanged(int)), _zoomSlider, SLOT(setValue(int)));

  QVBoxLayout *layout = new QVBoxLayout(this);

  layout->addWidget(_mapWidget);
  layout->addWidget(_zoomSlider);
  
//   GeoDataDocument *doc = new GeoDataDocument;
//   
//   GeoDataPolyStyle polyStyle(Qt::red);
//   polyStyle.setFill(true);
//   GeoDataStyle *style = new GeoDataStyle;
//   style->setPolyStyle(polyStyle);
//   
//   GeoDataPlacemark *place;
// 
//   
//   
//   
//   _mapWidget->model()->treeModel()->addDocument(doc);
  
  

}


void MapView::addHapLocations(const vector<Trait*> &traits)
{

  if (_hapLayer)
  {
    _mapWidget->removeLayer(_hapLayer);
    // delete _hapLayer;
    _hapLayer = 0;
  }

  if (! _locations.empty())  clearHapLocations();
  
  for (unsigned i = 0; i < traits.size(); i++)
  {
    // TODO save coordinates somewhere so that lookup doesn't happen every time
    // Maybe add a coordinates field to trait? Send a signal when a location is found, allow coordinate lookup at some point

    HapLocation *loc = new HapLocation(traits.at(i));

    if (! loc->locationSet())
    {
      
      lookupLocation(loc);
      float lat = (float)(loc->location().latitude(GeoDataCoordinates::Degree));
      float lon = (float)(loc->location().longitude(GeoDataCoordinates::Degree));
      emit locationSet(i, pair<float,float>(lat,lon));
    }

    _locations.push_back(loc);
  }

  _hapLayer = new HapLayer(_locations, this);
  connect(_hapLayer, SIGNAL(dirtyRegion(const QRegion &)), this, SLOT(updateDirtyRegion(const QRegion &)));
  connect(_hapLayer, SIGNAL(colourChangeTriggered(int)), this, SLOT(requestChangeSeqColour(int)));
  connect(_hapLayer, SIGNAL(coordinateChangeTriggered(int)), this, SLOT(changeCoordinate(int)));
  connect(_hapLayer, SIGNAL(entered(const QString &)), this, SLOT(setMapToolTip(const QString &)));
  connect(_hapLayer, SIGNAL(left(const QString &)), this, SLOT(resetMapToolTip(const QString &)));
  connect(_hapLayer, SIGNAL(clickable(bool)), this, SLOT(setClickableCursor(bool)));
  _hapLayer->setColours(_colourTheme);
  _mapWidget->addLayer(_hapLayer);
  _hapLayer->setTarget(_mapWidget);
  _mapWidget->installEventFilter(_hapLayer);
  _mapWidget->update();
  
  _legendDlg = new MapLegendDialog(_locations, this);
  _legendDlg->setColours(_colourTheme);
  connect(_legendDlg, SIGNAL(colourChangeRequested(int)), this, SLOT(requestChangeSeqColour(int)));

  //_legendDlg->show();
  
}

void MapView::updateDirtyRegion(const QRegion &region)
{
  //_mapWidget->repaint(region);
  //qDebug() << "updating dirty legend";
  // maybe just render the legend layer?
  _mapWidget->update();
}

void MapView::setClickableCursor(bool clickable)
{
  if (clickable)
    _mapWidget->setCursor(Qt::PointingHandCursor);

  else
    _mapWidget->setCursor(Qt::OpenHandCursor);
}

void MapView::setExternalLegend(bool external)
{
  _externalLegend = external;
  _hapLayer->setDrawLegend(! external);
  
  if (external)
    _legendDlg->show();
  else
    _legendDlg->hide();
  _mapWidget->update();
}

void MapView::setTheme(const QString &theme)
{
  _mapWidget->setMapThemeId(QString("earth/%1/%1.dgml").arg(theme));
  _mapWidget->setShowOverviewMap(false);
  _mapWidget->setShowScaleBar(false);
  _mapWidget->setShowCompass(false);
  
  if (theme == "bluemarble")
  {
    _mapWidget->setShowClouds(true);
    _mapWidget->setShowBorders(true);
  }
  
}

void MapView::savePDFFile(const QString &filename) const
{
  double width = _mapWidget->width();
  double height = _mapWidget->height();
  
  QPrinter printer;
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(filename);
  
  if (width > height)
    printer.setOrientation(QPrinter::Landscape);
  else
    printer.setOrientation(QPrinter::Portrait);
  
  QPainter painter(&printer);
  painter.setRenderHint(QPainter::Antialiasing);
  _mapWidget->render(&painter);
  painter.end();
}

void MapView::savePNGFile(const QString &filename) const
{
  double width = _mapWidget->width();
  double height = _mapWidget->height();
  
  QImage image(width, height, QImage::Format_ARGB32);
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  _mapWidget->render(&painter);
  image.save(filename);
  painter.end();

}

void MapView::saveSVGFile(const QString &filename) const
{
  double width = _mapWidget->width();
  double height = _mapWidget->height();
  
  QSvgGenerator generator;
  generator.setFileName(filename);
  generator.setSize(QSize(width, height));
  generator.setViewBox(QRect(0, 0, width, height));
  generator.setTitle(tr("PopART SVG Map"));
  generator.setDescription(tr("Geographical distribution of haplotype sequences PopART."));
  QPainter painter;
  painter.begin(&generator);
  _mapWidget->render(&painter);
  painter.end();
}

void MapView::clearHapLocations()
{
  for (unsigned i = 0; i < _locations.size(); i++)
    delete _locations.at(i);

  _locations.clear();
}

void MapView::lookupLocation(HapLocation *location)
{
  SearchRunnerManager* manager = new SearchRunnerManager(_mapWidget->model(), this);
  //manager->setModel( _mapWidget->model() );
  
  //const QString tmpstr(location->name());

  QVector<GeoDataPlacemark *> searchResult = manager->searchPlacemarks(location->name());

  if (searchResult.size() == 1)
  {
    //qDebug() << "only one location: " << searchResult.at(0)->coordinate().toString();
    location->setLocation(searchResult.at(0)->coordinate());
  }

  else if (searchResult.size() > 1)
  {

    QDialog dlg(this);
    QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
    QHBoxLayout *hlayout = new QHBoxLayout;

    QLabel *label = new QLabel("Choose a location", &dlg);
    hlayout->addWidget(label);

    QComboBox *comboBox = new QComboBox(&dlg);
    hlayout->addWidget(comboBox);
    vlayout->addLayout(hlayout);

    hlayout = new QHBoxLayout;

    hlayout->addStretch(1);
    QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
    connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
    hlayout->addWidget(okButton, 0, Qt::AlignRight);
    QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
    connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
    hlayout->addWidget(cancelButton, 0, Qt::AlignRight);

    vlayout->addLayout(hlayout);


    foreach (GeoDataPlacemark *placemark, searchResult)
      comboBox->addItem(placemark->name() + ": " + placemark->coordinate().toString());

    int result = dlg.exec();

    if (result != QDialog::Rejected)
      location->setLocation(searchResult.at(comboBox->currentIndex())->coordinate());
  }

  else
  {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setText(tr("<b>No Location Found!</b>"));
    message.setInformativeText(QString("Perhaps %1 is not a location?").arg(location->name()));
    message.setStandardButtons(QMessageBox::Ok);
    message.setDefaultButton(QMessageBox::Ok);

    message.exec();

  }
  
  // delete seems to take place in Marble
  /*foreach (GeoDataPlacemark *placemark, searchResult)
  {
    delete placemark;
  }*/

}

void  MapView::setColourTheme(ColourTheme::Theme theme)
{
  
  const QVector<QColor> *colours;
  _currentTheme = theme;
  
  switch (theme)
  {
    case ColourTheme::Camo:
      colours = &ColourTheme::camo();//_camo;
      break;
      
    case ColourTheme::Pastelle:
      colours = &ColourTheme::pastelle();//&_pastelle;
      break;
    
    case ColourTheme::Vibrant:
      colours = &ColourTheme::vibrant();//&_vibrant;
      break;
    
    case ColourTheme::Spring:
      colours = &ColourTheme::spring();//&_spring;
      break;
      
    case ColourTheme::Summer:
      colours = &ColourTheme::summer();//&_summer;
      break;
      
    case ColourTheme::Autumn:
      colours = &ColourTheme::autumn();//&_autumn;
      break;
      
    case ColourTheme::Winter:
      colours = &ColourTheme::winter();//&_winter;
      break;
      
    case ColourTheme::Greyscale:
    default:
      colours = &ColourTheme::greyscale();//&_greyscale;
      break;
  }
  
  _colourTheme.clear();
  
  QVector<QColor>::const_iterator colIt = colours->constBegin();
  
  while (colIt != colours->constEnd())
  {
    _colourTheme.push_back(QBrush(*colIt));
    ++colIt;
  }
  
  updateColours();
}



void MapView::updateColours()
{
  
  if (_hapLayer)
    _hapLayer->setColours(_colourTheme);
  
  if (_legendDlg)
    _legendDlg->setColours(_colourTheme);
  
  if (_mapWidget)
    _mapWidget->update();
}

void MapView::changeLegendFont(const QFont & font) 
{ 
  if (_hapLayer)
    _hapLayer->changeLegendFont(font);
  if (_legendDlg)
    _legendDlg->changeLegendFont(font);
}

const QFont & MapView::legendFont() const 
{ 
  
  if (_hapLayer)
    return _hapLayer->legendFont(); 
  
  else if (_legendDlg)
    return _legendDlg->legendFont();
 
  else
    return emptyFont;
}

void MapView::updateGeoPosition(QString pos)
{
  _geoPos = pos;

  if (pos != "not available")
    emit positionChanged(_geoPos);
}

void MapView::requestChangeSeqColour(int seqID)
{
  emit seqColourChangeRequested(seqID);
}

void MapView::changeCoordinate(int clusterID)
{
  
  if (clusterID < 0 || clusterID >= _locations.size())
    return;
  
  QDialog dlg(this);
  
  GeoDataCoordinates coords = _locations[clusterID]->location();

  
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);  
  QHBoxLayout *hlayout = new QHBoxLayout;
  QVBoxLayout *vlayout2 = new QVBoxLayout;
  
  QDoubleSpinBox *spinbox = new QDoubleSpinBox(this);
  spinbox->setDecimals(2);
  spinbox->setRange(0, 90);
  spinbox->setSingleStep(1);
  
  hlayout->addWidget(spinbox);
  
  hlayout->addWidget(new QLabel("Latitude"));
  
  
  QButtonGroup *buttonGroup = new QButtonGroup(this);
  int idCount = 0;
  QRadioButton *button1 = new QRadioButton("North", &dlg);
  buttonGroup->addButton(button1, idCount++);
  vlayout2->addWidget(button1);
  
  QRadioButton *button2 = new QRadioButton("South", &dlg);
  buttonGroup->addButton(button2, idCount++);
  vlayout2->addWidget(button2);
  
  if (coords.latitude(GeoDataCoordinates::Degree) < 0)
  {
    spinbox->setValue(- coords.latitude(GeoDataCoordinates::Degree));
    button2->setChecked(true);
  }
  
  else
  {
    spinbox->setValue(coords.latitude(GeoDataCoordinates::Degree));
    button1->setChecked(true);
  }
  
  hlayout->addLayout(vlayout2);
  
  vlayout2 = new QVBoxLayout;
  
  QDoubleSpinBox *spinbox2 = new QDoubleSpinBox(this);
  spinbox2->setDecimals(2);
  spinbox2->setRange(0, 180);
  spinbox2->setSingleStep(1);
  
  hlayout->addWidget(spinbox2);
  
  hlayout->addWidget(new QLabel("Longitude"));  
  
  QButtonGroup *buttonGroup2 = new QButtonGroup(this);
  idCount = 0;
  button1 = new QRadioButton("East", &dlg);
  buttonGroup2->addButton(button1, idCount++);
  vlayout2->addWidget(button1);
  
  button2 = new QRadioButton("West", &dlg);
  buttonGroup2->addButton(button2, idCount++);
  vlayout2->addWidget(button2);
  
  if (coords.longitude(GeoDataCoordinates::Degree) < 0)
  {
    spinbox2->setValue(- coords.longitude(GeoDataCoordinates::Degree));
    button2->setChecked(true);
  }
  
  else
  {
    spinbox2->setValue(coords.longitude(GeoDataCoordinates::Degree));
    button1->setChecked(true);
  }

  
  hlayout->addLayout(vlayout2);
  vlayout->addLayout(hlayout);
  
  hlayout = new QHBoxLayout;
  
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);      
  
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;
  
  float lat = spinbox->value();
  float lon = spinbox2->value();
  
  if (buttonGroup->checkedId() == 1)
    lat *= -1;
  
  if (buttonGroup2->checkedId() == 1)
    lon *= -1;
  
  _locations[clusterID]->setLocation(GeoDataCoordinates(lon, lat, 0, GeoDataCoordinates::Degree));
  
  _mapWidget->update();
  emit locationSet(clusterID, pair<float,float>(lat,lon));
}

const QColor & MapView::colour(int seqID) const
{
  return _colourTheme.at(seqID % _colourTheme.size()).color();
}

void MapView::setColour(int seqID, const QColor &col)
{
  if (seqID >= _colourTheme.size())
  {
    int newColours = seqID - _colourTheme.size() + 1;
    for (unsigned i = 0; i < newColours; i++)
      _colourTheme.push_back(_colourTheme.at(i));
  }

  _colourTheme[seqID] = QBrush(col);
  updateColours();
}

void MapView::setMapToolTip(const QString &toolTip)
{
  _mapWidget->setToolTip(toolTip);
}

void MapView::resetMapToolTip(const QString &toolTip)
{
  _mapWidget->setToolTip("");
}





