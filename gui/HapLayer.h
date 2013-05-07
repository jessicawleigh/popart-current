#ifndef HAPLAYER_H_
#define HAPLAYER_H_

#include "HapLocation.h"

#include <marble/GeoPainter.h>
#include <marble/LayerInterface.h>
#include <marble/MarbleWidget.h>
#include <marble/MarbleMap.h>
#include <marble/MarbleModel.h>

#include <QString>
#include <QStringList>
#include <QVector>

class HapLayer : public Marble::LayerInterface
{
public:
  HapLayer(QVector<HapLocation*> locations) : _hapLocations(locations) {};

  virtual QStringList renderPosition() const;
  virtual bool render(Marble::GeoPainter *, Marble::ViewportParams *, const QString &, Marble::GeoSceneLayer *);

private:
  QVector<HapLocation*> _hapLocations;
};

#endif
