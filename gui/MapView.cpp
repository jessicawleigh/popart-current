/*
 * MapView.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: jleigh
 */

#include "MapView.h"
#include "HapLayer.h"
#include "HapLocation.h"

#include <QDebug>
#include <QLayout>
#include <QVector>

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

MapView::MapView(QWidget *parent)
  : QWidget(parent)
{
  _mapWidget = new MarbleWidget(this);
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
  _placemarks.clear();
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

  QVector<HapLocation*> locations;

  // Don't do this here, never freed
  HapLocation *location = new HapLocation("Zimbabwe");
  location->addSeq("First", 5);
  location->addSeq("second", 10);

  locations.push_back(location);


  HapLayer* layer = new HapLayer(locations);
  // Uncomment for older versions of Marble:
  // mapWidget->map()->model()->addLayer(layer);
  _mapWidget->addLayer(layer);




  // Subclass GeoDataPlacemark to add haplotype data instead
  // Also change geometry to pie charts
  //GeoDataPlacemark *place = new GeoDataPlacemark("Bucharest");
  HapDataPlacemark *place = new HapDataPlacemark("Bucharest");
  place->setCoordinate(25.97, 44.43, 0.0, GeoDataCoordinates::Degree);
  place->setPopulation(1877155);
  place->setCountryCode ("Romania");


  GeoDataDocument *document = new GeoDataDocument;
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
  }

}


// TODO Add an setHaplotypeData method with sequences and traits
// Compute sequence frequency vector for each trait
// Find location for each trait
// subclass GeoDataPlacemark as HapDataPlacemark with new paint method
// subclass GeoDataDocument as HapDataDocument to store relevant data

// Notes: Drawing doesn't happen in GeoDataPlacemark, but you can specify geometry there (GeoDataGeometry subclasses will do it)
// But it's not clear that those know what sort of GeoGraphicsItem to draw themselves with. Maybe subclass GeoGraphicsItem as HapGraphicsItem
// and GeoDataGeometry as HapDataGeometry, and then GeometryLayerPrivate as HapGeometryLayerPrivate, making the
// createGraphicsItemFromGeometry function virtual (take a look at the code, should be able to just check whether
// the geometry is HapDataGeometry and return HapGraphicsItem, or call the parent class otherwise)

void MapView::updateGeoPosition(QString pos)
{
  _geoPos = pos;

  if (pos != "not available")
    emit positionChanged(_geoPos);
}



