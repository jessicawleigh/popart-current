/*
 * MapView.h
 *
 *  Created on: Apr 24, 2013
 *      Author: jleigh
 */

#ifndef MAPVIEW_H_
#define MAPVIEW_H_

#include <QLabel>
#include <QSlider>
#include <QString>
#include <QVector>
#include <QWidget>


#include <marble/MarbleWidget.h>

#include "HapDataPlacemark.h"

class MapView : public QWidget
{
  Q_OBJECT
public:
  MapView(QWidget * = 0);
  virtual ~MapView();

private:
  void setupWidget();

  Marble::MarbleWidget *_mapWidget;

  QSlider *_zoomSlider;
  //QLabel *_posLabel;
  QString _geoPos;

  QVector<HapDataPlacemark*> _placemarks;

private slots:
  void updateGeoPosition(QString);

signals:
  void positionChanged(const QString &);

};

#endif /* MAPVIEW_H_ */
