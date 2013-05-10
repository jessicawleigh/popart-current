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
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>

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
  : QWidget(parent)
{
  _currentTheme = _defaultTheme;
  _mapWidget = new MarbleWidget(this);
  _hapLayer = 0;
  setupWidget();
}

MapView::~MapView()
{
  /*cout << "Size of placemarks QVector: " << _placemarks.size() << endl;
  foreach (HapDataPlacemark *p, _placemarks)
  {
    cout << "deleting p" << endl;
    delete p;
  }*/
  //_placemarks.clear();

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

  /*QVector<HapLocation*> locations;

  HapLocation *location = new HapLocation("Zimbabwe");
  location->addSeq("First", 5);
  location->addSeq("second", 10);

  _locations.push_back(location);*/
}

// TODO call clearHapLocations, delete _layer

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
  _mapWidget->addLayer(_hapLayer);
  _mapWidget->update();






  /*GeoDataDocument *document = new GeoDataDocument;
  document->append(place);

  _placemarks.push_back(place);

    // Add the document to MarbleWidget's tree model
  _mapWidget->model()->treeModel()->addDocument(document);
  MarbleRunnerManager* manager = new MarbleRunnerManager(_mapWidget->model()->pluginManager(), this );
  manager->setModel( _mapWidget->model() );

  QVector<GeoDataPlacemark*> searchResult = manager->searchPlacemarks( "Karlsruhe" );

  cout << "Found " << searchResult.size() << " places." << endl;
  foreach( GeoDataPlacemark* placemark, searchResult ) {
      qDebug() << "Found " << placemark->name() << "at" << placemark->coordinate().toString();
  }*/

}

void MapView::clearHapLocations()
{
  for (unsigned i = 0; i < _locations.size(); i++)
    delete _locations.at(i);

  _locations.clear();
}

GeoDataCoordinates MapView::lookupLocation(HapLocation *location)
{
  MarbleRunnerManager* manager = new MarbleRunnerManager(_mapWidget->model()->pluginManager(), this);
  manager->setModel( _mapWidget->model() );

  QVector<GeoDataPlacemark *> searchResult = manager->searchPlacemarks(location->name());

  if (searchResult.size() == 1)
    location->setLocation(searchResult.at(0)->coordinate());

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




  /*
  foreach (GeoDataPlacemark *placemark, searchResult)
  {
    delete placemark;
  } */

}

void  MapView::setColourTheme(ColourTheme::Theme theme)
{
  _currentTheme = theme;

  _colourTheme.clear();

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
  }
}

void MapView::updateGeoPosition(QString pos)
{
  _geoPos = pos;

  if (pos != "not available")
    emit positionChanged(_geoPos);
}



