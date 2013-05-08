#include "HapLayer.h"

#include <marble/GeoDataCoordinates.h>
#include <marble/ViewportParams.h>

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QRectF>

using namespace Marble;

QStringList HapLayer::renderPosition() const
{
  // We will paint in exactly one of the following layers.
  // The current one can be changed by pressing the '+' key
  //QStringList layers = QStringList() << "SURFACE" << "HOVERS_ABOVE_SURFACE";
  //layers << "ORBIT" << "USER_TOOLS" << "STARS";

  //int index = m_index % layers.size();
  return QStringList() << "ORBIT";//"HOVERS_ABOVE_SURFACE"; //layers.at(index);

}

bool HapLayer::render(GeoPainter *painter, ViewportParams *viewport, const QString &renderPos, GeoSceneLayer *layer)
{
  //qDebug() << "number of locations:" << _hapLocations.size();

  unsigned diameter = 20;
  const unsigned circleUnits = 5760;

  for (unsigned i = 0; i < _hapLocations.size(); i++)
  {
    const GeoDataCoordinates & coords = _hapLocations.at(i)->location();
    //qDebug() << "coords:" << coords.toString();
    painter->setBrush(QBrush(Qt::black));
    //painter->drawEllipse(coords, diameter+5, diameter+5);
    //QRect boundingRect = painter->regionFromEllipse(coords, diameter, diameter, false, 1).boundingRect();
    double *x, y;
    int repeats;
    bool behindGlobe;
    x = new double[5]; // allow coordinates to be repeated up to 5 times (should be overkill)

    bool visible = viewport->screenCoordinates(coords, x, y, repeats, behindGlobe);

    if (visible)
    {

      double startAngle = 0;
      double nseqs = _hapLocations.at(i)->totalCount();
      QVector<QString> seqnames = _hapLocations.at(i)->seqNames();
      /*painter->setBrush(QBrush(Qt::blue));

      for (unsigned k = 0; k < repeats; k++)
        painter->drawEllipse(x[k] - diameter/2, y - diameter/2, diameter, diameter);

      qDebug() << "number of seq types:" << seqnames.size() << "nseqs:" << nseqs;*/
      for (unsigned j = 0; j < seqnames.size(); j++)
      {
        unsigned count = _hapLocations.at(i)->seqCount(seqnames.at(j));
        double seqFreq = _hapLocations.at(i)->seqCount(seqnames.at(j))/nseqs;

        // Do something clever with spanAngle to make sure it goes all the way to 360 degrees
        double spanAngle = seqFreq * circleUnits;
        painter->setBrush(QBrush(Qt::red));

        for (unsigned k = 0; k < repeats; k++)
          painter->drawPie(QRectF(x[k] - diameter/2, y - diameter/2, diameter, diameter), startAngle, spanAngle);

        startAngle += spanAngle;
      }
    }

    delete [] x;
  }

  return true;
}
