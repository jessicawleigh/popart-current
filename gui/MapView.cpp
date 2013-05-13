/*
 * MapView.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: jleigh
 */

#include "MapView.h"

#include <QtAlgorithms>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QImage>
#include <QLayout>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPushButton>
#include <QSvgGenerator>

#include <marble/GeoDataDocument.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLineString.h>
#include <marble/GeoDataLinearRing.h>
#include <marble/GeoDataTreeModel.h>
#include <marble/GeoDataStyle.h>
#include <marble/GeoDataIconStyle.h>
#include <marble/GeoDataLineStyle.h>
#include <marble/GeoDataPolyStyle.h>
#include <marble/MarbleModel.h>
#include <marble/MarbleRunnerManager.h>
using namespace Marble;

#include <iostream>
using namespace std;

const ColourTheme::Theme MapView::_defaultTheme = ColourTheme::Greyscale;

MapView::MapView(QWidget *parent)
  : QWidget(parent), _mapWidget(0), _hapLayer(0)
{
  _currentTheme = _defaultTheme;
  setColourTheme();
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

    HapLocation *loc = new HapLocation(*(traits.at(i)));

    lookupLocation(loc);

    _locations.push_back(loc);
  }

  _hapLayer = new HapLayer(_locations);
  _hapLayer->setColours(_colourTheme);
  _mapWidget->addLayer(_hapLayer);
  _mapWidget->update();
  
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
  MarbleRunnerManager* manager = new MarbleRunnerManager(_mapWidget->model()->pluginManager(), this);
  manager->setModel( _mapWidget->model() );

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

  /*_colourTheme.clear();

  switch (theme)
  {
    case ColourTheme::Camo:
      qCopy(ColourTheme::camo().begin(), ColourTheme::camo().end(), _colourTheme.begin());// = &ColourTheme::camo();//_camo;
      break;

    case ColourTheme::Pastelle:
      qCopy(ColourTheme::pastelle().begin(), ColourTheme::pastelle().end(), _colourTheme.begin());
      //colours = &ColourTheme::pastelle();//&_pastelle;
      break;

    case ColourTheme::Vibrant:
      qCopy(ColourTheme::vibrant().begin(), ColourTheme::vibrant().end(), _colourTheme.begin());
      //colours = &ColourTheme::vibrant();//&_vibrant;
      break;

    case ColourTheme::Spring:
      qCopy(ColourTheme::spring().begin(), ColourTheme::spring().end(), _colourTheme.begin());
      //colours = &ColourTheme::spring();//&_spring;
      break;

    case ColourTheme::Summer:
      qCopy(ColourTheme::summer().begin(), ColourTheme::summer().end(), _colourTheme.begin());
      //colours = &ColourTheme::summer();//&_summer;
      break;

    case ColourTheme::Autumn:
      qCopy(ColourTheme::autumn().begin(), ColourTheme::autumn().end(), _colourTheme.begin());
      //colours = &ColourTheme::autumn();//&_autumn;
      break;

    case ColourTheme::Winter:
      qCopy(ColourTheme::winter().begin(), ColourTheme::winter().end(), _colourTheme.begin());
      //colours = &ColourTheme::winter();//&_winter;
      break;

    case ColourTheme::Greyscale:
    default:
      qCopy(ColourTheme::greyscale().begin(), ColourTheme::greyscale().end(), _colourTheme.begin());
      //colours = &ColourTheme::greyscale();//&_greyscale;
      break;
  }*/
  
  updateColours();
}

void MapView::updateColours()
{
  
  if (_hapLayer)
    _hapLayer->setColours(_colourTheme);
  
  if (_mapWidget)
    _mapWidget->update();
}

void MapView::updateGeoPosition(QString pos)
{
  _geoPos = pos;

  if (pos != "not available")
    emit positionChanged(_geoPos);
}



