#ifndef HAPLAYER_H_
#define HAPLAYER_H_

#include "HapLocation.h"

#include <marble/GeoPainter.h>
#include <marble/LayerInterface.h>
#include <marble/MarbleWidget.h>
#include <marble/MarbleMap.h>
#include <marble/MarbleModel.h>

#include <QBrush>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QVector>

class HapLayer : public Marble::LayerInterface
{
public:
  HapLayer(QVector<HapLocation*>);// locations) : _hapLocations(locations) {};

  virtual QStringList renderPosition() const;
  virtual bool render(Marble::GeoPainter *, Marble::ViewportParams *, const QString &, Marble::GeoSceneLayer *);

  void setColours(const QVector<QBrush> &colours) { _colours = colours; };
  const QBrush & hapBrush(int) const;
  const QFont & defaultFont() const { return _defaultFont; };
  const QFont & smallFont() const { return _smallFont; };
  const QFont & legendFont() const { return _legendFont; };

  
private:
  QVector<HapLocation*> _hapLocations;
  QVector<QBrush> _colours;
  QBrush _defaultBrush;
  QFont _defaultFont;
  QFont _smallFont;
  QFont _legendFont;
};

#endif
