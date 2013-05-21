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
#include <QContextMenuEvent>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QVector>
#include <QWidget>

#include <vector>

class HapMapWidget;

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
  const QColor & colour(int) const;
  void setColour(int, const QColor &);
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
  void updateDirtyRegion(const QRegion &);
  void requestChangeSeqColour(int);
  void setMapToolTip(const QString &);
  void resetMapToolTip(const QString &);

signals:
  void positionChanged(const QString &);
  void seqColourChangeRequested(int);

};

/*class HapMapWidget : public Marble::MarbleWidget
{
  Q_OBJECT
public:
  HapMapWidget(QWidget *parent = 0) : Marble::MarbleWidget(parent) {};
protected:
  virtual void contextMenuEvent(QContextMenuEvent *);
};*/


#endif /* MAPVIEW_H_ */
