#ifndef HAPLAYER_H_
#define HAPLAYER_H_

#include "HapLocation.h"

#include <marble/GeoPainter.h>
#include <marble/LayerInterface.h>
#include <marble/MarbleWidget.h>
#include <marble/MarbleMap.h>
#include <marble/MarbleModel.h>

#include <QBrush>
#include <QMap>
#include <QEvent>
#include <QFont>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QVector>

class HapLayer : public QObject, public Marble::LayerInterface
{
  Q_OBJECT
public:
  HapLayer(QVector<HapLocation*>, QObject * = 0);// locations) : _hapLocations(locations) {};

  virtual QStringList renderPosition() const;
  virtual bool render(Marble::GeoPainter *, Marble::ViewportParams *, const QString &, Marble::GeoSceneLayer *);
  virtual bool eventFilter(QObject *, QEvent *);

  void setColours(const QVector<QBrush> &colours) { _colours = colours; };
  
  const QBrush & hapBrush(int) const;
  const QFont & defaultFont() const { return _defaultFont; };
  const QFont & smallFont() const { return _smallFont; };
  void setSmallFont(const QFont & font) { _smallFont = font; };
  const QFont & legendFont() const { return _legendFont; };
  void setLegendFont(const QFont & font) { _legendFont = font; };
  void changeLegendFont(const QFont & font);
  
  void setDrawLegend(bool draw) { _drawLegend = draw; };

  void setTarget(const Marble::MarbleWidget *target) { _target = target; };

  
private:
  void updateLegendPos(const QPoint &);

  QVector<HapLocation*> _hapLocations;
  QVector<QBrush> _colours;
  QBrush _defaultBrush;
  QFont _defaultFont;
  QFont _smallFont;
  QFont _legendFont;

  QVector<QRegion> _clusters;
  QVector<QString> _clustLabels;
  const Marble::MarbleWidget *_target;
  QRegion _legendRegion; // why is this a pointer?
  QPoint _legendStart;
  QVector<QRegion> _legendKeys;

  bool _drawLegend;
  bool _clickedInLegend;
  int _clickedInKey;
  int _clickedInCluster;
  QPoint _mouseDownPos;
  QPoint _enteredPos;

private slots:
  void changeColour();
  void changeCoordinates();

signals:
  void dirtyRegion(const QRegion &);
  void colourChangeTriggered(int);
  void coordinateChangeTriggered(int);
  void entered(const QString &);
  void left(const QString &);
  void clickable(bool);
};

/*class HapLayerFilter : public QObject
{
  Q_OBJECT
public:
  HapLayerFilter(const HapLayer *target, QObject * parent = 0) : QObject(parent) { _target = target; };
  virtual bool eventFilter(QObject *, QEvent *);

  //void setLegend(const QRegion *legend) { _legendRegion = legend; };
  void addCluster(const QRegion & clust, const QString &label) { _clusters.push_back(clust); _clustLabels.push_back(QString(label)); };
  // update regions? Inside HapLayer::render
  void clearClusters() { _clusters.clear(); _clustLabels.clear(); };

private:
  QVector<QRegion> _clusters;
  QVector<QString> _clustLabels;
  // QRegion _legendRegion;
  const HapLayer *_target;
};*/

#endif
