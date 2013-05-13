/*
 * MapView.h
 *
 *  Created on: Apr 24, 2013
 *      Author: jleigh
 */

#ifndef MAPVIEW_H_
#define MAPVIEW_H_

#include "HapLayer.h"
#include "HapLocation.h"
#include "NetworkView.h"
#include "Trait.h"

#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataPlacemark.h>
#include <marble/MarbleWidget.h>

#include <QBrush>
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
  ColourTheme::Theme colourTheme() const { return _currentTheme; };
  static ColourTheme::Theme defaultColourTheme() { return _defaultTheme; };
  
public slots:
  void setColourTheme(ColourTheme::Theme = _defaultTheme);
  void savePDFFile(const QString &filename) const;
  void savePNGFile(const QString &filename) const;  
  void saveSVGFile(const QString &filename) const;

private:
  void setupWidget();
  void clearHapLocations();
  void lookupLocation(HapLocation*);
  void updateColours();

  Marble::MarbleWidget *_mapWidget;
  QVector<QBrush> _colourTheme;
  ColourTheme::Theme _currentTheme;
  static const ColourTheme::Theme _defaultTheme;

  QSlider *_zoomSlider;
  QString _geoPos;

  QVector<HapLocation *> _locations;
  HapLayer *_hapLayer;

private slots:
  void updateGeoPosition(QString);

signals:
  void positionChanged(const QString &);

};

#endif /* MAPVIEW_H_ */
