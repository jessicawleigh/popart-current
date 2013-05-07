#include "HapLayer.h"

#include <marble/GeoDataCoordinates.h>

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QRect>
#include <QRegion>

using namespace Marble;

QStringList HapLayer::renderPosition() const
{
  // We will paint in exactly one of the following layers.
  // The current one can be changed by pressing the '+' key
  //QStringList layers = QStringList() << "SURFACE" << "HOVERS_ABOVE_SURFACE";
  //layers << "ORBIT" << "USER_TOOLS" << "STARS";

  //int index = m_index % layers.size();
  return QStringList() << "HOVERS_ABOVE_SURFACE"; //layers.at(index);

}

bool HapLayer::render(GeoPainter *painter, ViewportParams *viewport, const QString &renderPos, GeoSceneLayer *layer)
{
  qDebug() << "number of locations:" << _hapLocations.size();

  unsigned diameter = 20;
  const unsigned circleUnits = 5760;

  for (unsigned i = 0; i < _hapLocations.size(); i++)
  {
    const GeoDataCoordinates & coords = _hapLocations.at(i)->location();
    painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(coords, diameter+5, diameter+5);
    QRect boundingRect = painter->regionFromEllipse(coords, diameter, diameter, false, 1).boundingRect();

    double startAngle = 0;
    double nseqs = _hapLocations.at(i)->totalCount();
    QVector<QString> seqnames = _hapLocations.at(i)->seqNames();

    for (unsigned j = 0; j < seqnames.size(); j++)
    {
      double seqFreq = _hapLocations.at(i)->seqCount(seqnames.at(j))/nseqs;

      // Do something clever with spanAngle to make sure it goes all the way to 360 degrees
      double spanAngle = seqFreq * circleUnits;
      painter->setBrush(QBrush(Qt::red));
      painter->drawPie(boundingRect, startAngle, spanAngle);

      startAngle += spanAngle;
    }
  }

  return true;
}
