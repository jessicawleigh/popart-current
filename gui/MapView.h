/*
 * MapView.h
 *
 *  Created on: Apr 24, 2013
 *      Author: jleigh
 */

#ifndef MAPVIEW_H_
#define MAPVIEW_H_

#include "HapDataPlacemark.h"
#include "HapLayer.h"
#include "HapLocation.h"
#include "Trait.h"

#include <marble/GeoDataCoordinates.h>
#include <marble/MarbleWidget.h>

#include <QLabel>
#include <QSlider>
#include <QString>
#include <QVector>
#include <QWidget>

#include <vector>

class MapView : public QWidget
{
  Q_OBJECT
public:
  MapView(QWidget * = 0);
  virtual ~MapView();

  void addHapLocations(const std::vector<Trait *> &);

private:
  void setupWidget();
  void clearHapLocations();
  Marble::GeoDataCoordinates lookupLocation(HapLocation*);

  Marble::MarbleWidget *_mapWidget;

  QSlider *_zoomSlider;
  //QLabel *_posLabel;
  QString _geoPos;

  QVector<HapLocation *> _locations;
  HapLayer *_hapLayer;

private slots:
  void updateGeoPosition(QString);

signals:
  void positionChanged(const QString &);

};

#endif /* MAPVIEW_H_ */
