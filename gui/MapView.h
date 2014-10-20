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
#include "MapLegendDialog.h"
#include "NetworkView.h"
#include "Trait.h"

// Marble headers
#include <GeoDataCoordinates.h>
#include <GeoDataPlacemark.h>
#include <MarbleWidget.h>

// Qt headers
#include <QBrush>
#include <QContextMenuEvent>
#include <QFont>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QVector>
#include <QWidget>

#include <utility>
#include <vector>

class HapMapWidget;

class MapView : public QWidget
{
  Q_OBJECT
public:
  MapView(QWidget * = 0);
  virtual ~MapView();
  //void checkFloatObjects() const;
  void addHapLocations(const std::vector<Trait *> &);
  ColourTheme::Theme colourTheme() const { return _currentTheme; };
  static ColourTheme::Theme defaultColourTheme() { return _defaultTheme; };
  const QFont & legendFont() const;
  void changeLegendFont(const QFont &);

public slots:
  void setColourTheme(ColourTheme::Theme = _defaultTheme);
  
  const QColor & colour(int) const;
  void setColour(int, const QColor &);
  void setClickableCursor(bool);
  void setExternalLegend(bool);
  void setTheme(const QString &);
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
  bool _externalLegend;

  QSlider *_zoomSlider;
  QString _geoPos;

  QVector<HapLocation *> _locations;
  HapLayer *_hapLayer;
  MapLegendDialog *_legendDlg; 
  QFont emptyFont;

private slots:
  void updateGeoPosition(QString);
  void updateDirtyRegion(const QRegion &);
  void requestChangeSeqColour(int);
  void changeCoordinate(int);
  void setMapToolTip(const QString &);
  void resetMapToolTip(const QString &);

signals:
  void positionChanged(const QString &);
  void seqColourChangeRequested(int);
  void locationSet(unsigned, std::pair<float,float>);

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
