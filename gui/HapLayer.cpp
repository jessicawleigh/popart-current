#include "HapLayer.h"
#include "NetworkItem.h"

#include <marble/GeoDataCoordinates.h>
#include <marble/ViewportParams.h>

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QFontMetrics>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QRectF>

using namespace Marble;

HapLayer::HapLayer(QVector<HapLocation*> locations)
  : _hapLocations(locations), _defaultBrush(Qt::black)
{
  //qDebug() << "in constructor, address:" << this;
  _defaultFont.setFamily("Baskerville");
  _defaultFont.setPointSize(10);
  _legendFont = _defaultFont;
  
  _smallFont = QFont(_defaultFont);
  _smallFont.setPointSize(6); 
}


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

  double vertRadUnit = NetworkItem::VERTRAD;
  painter->setPen(Qt::SolidLine);

  const unsigned circleUnits = 5760;

  for (unsigned i = 0; i < _hapLocations.size(); i++)
  {
    //const GeoDataCoordinates coords = _hapLocations.at(i)->location();
    //qDebug() << "coords:" << coords.toString();
    //painter->setBrush(QBrush(Qt::black));
    //painter->drawEllipse(coords, diameter+5, diameter+5);
    //QRect boundingRect = painter->regionFromEllipse(coords, diameter, diameter, false, 1).boundingRect();
    double *x, y;
    int repeats;
    bool behindGlobe;
    x = new double[5]; // allow coordinates to be repeated up to 5 times (should be overkill)

    bool visible = viewport->screenCoordinates(_hapLocations.at(i)->location(), x, y, repeats, behindGlobe);

    if (visible)
    {

      double startAngle = 0;
      double nseqs = _hapLocations.at(i)->totalCount();
      double diametre = vertRadUnit * sqrt(nseqs);
      
      // should only happen if nseqs is 0
      if (diametre < vertRadUnit)  
        diametre = vertRadUnit;
      
      QVector<QString> seqnames = _hapLocations.at(i)->seqNames();
      /*painter->setBrush(QBrush(Qt::blue));

      for (unsigned k = 0; k < repeats; k++)
        painter->drawEllipse(x[k] - diameter/2, y - diameter/2, diameter, diameter);

      qDebug() << "number of seq types:" << seqnames.size() << "nseqs:" << nseqs;*/
  
      if (seqnames.size() == 1)
      {
        painter->setBrush(hapBrush(HapLocation::seqID(seqnames.at(0))));
        for (unsigned k = 0; k < repeats; k++)
          painter->drawEllipse(QRectF(x[k] - diametre/2, y - diametre/2, diametre, diametre));

      }
        
      else
      {
        for (unsigned j = 0; j < seqnames.size(); j++)
        {
          unsigned count = _hapLocations.at(i)->seqCount(seqnames.at(j));
          
          double seqFreq = count/nseqs;
          
          // Do something clever with spanAngle to make sure it goes all the way to 360 degrees
          double spanAngle = seqFreq * circleUnits;
          painter->setBrush(hapBrush(HapLocation::seqID(seqnames.at(j))));
          
          for (unsigned k = 0; k < repeats; k++)
            painter->drawPie(QRectF(x[k] - diametre/2, y - diametre/2, diametre, diametre), startAngle, spanAngle);
          
          startAngle += spanAngle;
        }
      }
    }

    delete [] x;
  }
  
  //addLegend(painter);
  
  // Choose location differently: Make it the bottom left of the viewport
  // Store default location, then allow it to move
  /*GeoDataCoordinates antarctica(-179,-85,0, GeoDataCoordinates::Degree); 
  double *x, y;
  int repeats;
  bool behindGlobe;
  x = new double[5]; // allow coordinates to be repeated up to 5 times (should be overkill)
  
  bool visible = viewport->screenCoordinates(antarctica, x, y, repeats, behindGlobe);*/
  
  //if (visible)
  //{
  
    const QMap<QString, unsigned> & seqIDs = HapLocation::seqIDs();
    
    // TODO font metrics don't seem to match painter.
    QFontMetrics painterMetric = painter->fontMetrics();
    QRect painterRect = painter->viewport();
    int x = painterRect.bottomRight().x();
    int y = painterRect.bottomRight().y();

    double maxwidth = 0;
    
    QMap<QString, unsigned>::const_iterator seqNameIt = seqIDs.constBegin();
    painter->setFont(legendFont());
    
    while (seqNameIt != seqIDs.constEnd())
    {
      double width = painterMetric.width(seqNameIt.key());
      if (width > maxwidth)  maxwidth = width;
      
      ++seqNameIt;
    }
    
    unsigned margin = 15;
    double minHeight = vertRadUnit + 5;
    double entryHeight = painterMetric.height();
    if (minHeight > entryHeight)
      entryHeight = minHeight;
    
    // TODO text always drawn a bit to the right of where I expect, figure this out
    // maybe play with QFontMetricsF::boundingRect
    
    double diam5seq = sqrt(10) * vertRadUnit;
    double minLegendWidth = diam5seq + 2 * margin;
    double legendWidth = maxwidth + vertRadUnit + 3 * margin;
        
    if (legendWidth < minLegendWidth)  legendWidth = minLegendWidth;

    painter->setFont(smallFont());
    painterMetric = painter->fontMetrics();

    double legendHeight = entryHeight * seqIDs.size() + painterMetric.height() + diam5seq + 3 * margin;
  
    QPointF legendStart(x - legendWidth, y - legendHeight);//x[repeats - 1], y - legendHeight);
    
    painter->setBrush(Qt::white);
    painter->drawRect(legendStart.x(), legendStart.y(), legendWidth, legendHeight);
    
    double currentY = legendStart.y() + margin;
    double keyX = legendStart.x() + legendWidth / 2 - diam5seq/2;
    
    painter->setBrush(Qt::transparent);
    painter->drawEllipse(keyX, currentY, diam5seq, diam5seq);
    QString key("10 samples");
    
    double textX = legendStart.x() + legendWidth/2 - painterMetric.width(key)/2;//smallMetric.width(key)/2;
    painter->drawText(textX, currentY + diam5seq/2, key);
     
    currentY += (diam5seq - vertRadUnit);
    keyX = legendStart.x() + legendWidth / 2 - vertRadUnit/2;
    painter->drawEllipse(keyX, currentY, vertRadUnit, vertRadUnit);
    
    currentY += vertRadUnit + margin; //smallMetric.ascent();
    key = QString("1 sample");
    textX = legendStart.x() + legendWidth/2 - painterMetric.width(key)/2;//smallMetric.width(key)/2;
    painter->drawText(textX, currentY , key);
    
    currentY += margin;
    keyX = legendStart.x() + margin;
    textX = keyX + vertRadUnit + margin;    
    painter->setFont(legendFont());
    painterMetric = painter->fontMetrics();

    seqNameIt = seqIDs.constBegin();
    
    while (seqNameIt != seqIDs.constEnd())
    {
      painter->setBrush(hapBrush(seqNameIt.value()));
      
      painter->drawEllipse(keyX, currentY, vertRadUnit, vertRadUnit);
      painter->drawText(textX, currentY + (painterMetric.ascent() + vertRadUnit)/2, seqNameIt.key());
      /*textBox = metric.boundingRect(seqNameIt.key());
      painterRect = painter->boundingRect(textBox, Qt::AlignLeft, seqNameIt.key());
      qDebug() << "text width:" << metric.width(seqNameIt.key()) << "textBox width:" << textBox.width() << "painter box width:" << painterRect.width();*/
      
      currentY += entryHeight;
      ++seqNameIt;
    }
    
    
    
  //}
  
  //else
  //  qDebug() << "Not visible.";
 
  
  //qDebug() << "deleted";
  
  return true;
}

/*void HapLayer::addLegend()
{
  GeoDataCoordinates antarctica(-76.598545,-171.738281, 0);
  
  
}*/

const QBrush & HapLayer::hapBrush(int hapID) const
{
  if (_colours.empty() || hapID < 0)  return _defaultBrush;
  return _colours.at(hapID % _colours.size());
}


