/*
 * MapView.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: jleigh
 */

#include <QLayout>

#include <marble/GeoDataDocument.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataPlacemark.h>
#include <marble/GeoDataLineString.h>
#include <marble/GeoDataLinearRing.h>
#include <marble/GeoDataTreeModel.h>
#include <marble/GeoDataStyle.h>
#include <marble/GeoDataIconStyle.h>
#include <marble/GeoDataLineStyle.h>
#include <marble/GeoDataPolyStyle.h>
#include <marble/MarbleModel.h>


#include "MapView.h"

MapView::MapView(QWidget *parent)
  : QWidget(parent)
{
  _mapWidget = new MarbleWidget(this);
  setupWidget();
}

void MapView::setupWidget()
{
  _mapWidget->setProjection(Mercator);
  _mapWidget->setMapThemeId("earth/srtm/srtm.dgml");
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

  // Connect the map widget to the position label.
  QObject::connect(_mapWidget, SIGNAL(mouseMoveGeoPosition(QString)),
                    this, SLOT(updateGeoPosition(QString)));

  connect(_zoomSlider, SIGNAL(valueChanged(int)), _mapWidget, SLOT(zoomView(int)));
  connect(_mapWidget, SIGNAL(zoomChanged(int)), _zoomSlider, SLOT(setValue(int)));

  QVBoxLayout *layout = new QVBoxLayout(this);

  layout->addWidget(_mapWidget);
  layout->addWidget(_zoomSlider);
  //layout->addWidget(_posLabel);


  // Subclass GeoDataPlacemark to add haplotype data instead
  // Also change geometry to pie charts
  GeoDataPlacemark *place = new GeoDataPlacemark("Bucharest");
  place->setCoordinate(25.97, 44.43, 0.0, GeoDataCoordinates::Degree);
  place->setPopulation(1877155);
  place->setCountryCode ("Romania");


  GeoDataDocument *document = new GeoDataDocument;
  document->append(place);

    // Add the document to MarbleWidget's tree model
  _mapWidget->model()->treeModel()->addDocument(document);

}

MapView::~MapView()
{
  // TODO Auto-generated destructor stub
}

// TODO Add an setHaplotypeData method with sequences and traits
// Compute sequence frequency vector for each trait
// Find location for each trait
// subclass GeoDataPlacemark as HapDataPlacemark with new paint method
// subclass GeoDataDocument as HapDataDocument to store relevant data

void MapView::updateGeoPosition(QString pos)
{
  _geoPos = pos;

  if (pos != "not available")
    emit positionChanged(_geoPos);
}



