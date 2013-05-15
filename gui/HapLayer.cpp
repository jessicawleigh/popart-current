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

HapLayer::HapLayer(QVector<HapLocation*> locations, QObject *parent)
  : QObject(parent), _hapLocations(locations), _defaultBrush(Qt::black)
{
  //qDebug() << "in constructor, address:" << this;
  _defaultFont.setFamily("Baskerville");
  _defaultFont.setPointSize(10);
  _legendFont = _defaultFont;
  
  _smallFont = QFont(_defaultFont);
  _smallFont.setPointSize(6); 

  _target = 0;
  //_legendRegion = 0;
  _clickedInLegend = false;
  _legendStart.setX(-1);
  _legendStart.setY(-1);
  //_filter = new HapLayerFilter(this, parent);
  //installEventFilter(_filter);
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
  //_filter->clearClusters();
  _clusters.clear();
  _clustLabels.clear();
  //if (_legendRegion)  delete _legendRegion;
  _legendKeys.clear();

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
      _clusters.push_back(painter->regionFromEllipse(_hapLocations.at(i)->location(), diametre, diametre, false, 1));
      _clustLabels.push_back(_hapLocations.at(i)->name());
  
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
  
    //QPointF legendStart(x - legendWidth, y - legendHeight);//x[repeats - 1], y - legendHeight);
    if (_legendStart.x() < 0)
    {
      _legendStart.setX(x - legendWidth);
      _legendStart.setY(y - legendHeight);
    }
    
    painter->setBrush(Qt::white);
    painter->drawRect(_legendStart.x(), _legendStart.y(), legendWidth, legendHeight);
    qreal lon, lat;
    bool inglobe = viewport->geoCoordinates(_legendStart.x(), _legendStart.y(), lon, lat);

    
    // Will this ever be false? Some sort of default legend region?
    if (inglobe)
      _legendRegion = painter->regionFromRect(GeoDataCoordinates(lon, lat, 0,  GeoDataCoordinates::Degree), legendWidth, legendHeight, false, 1);
    
    double currentY = _legendStart.y() + margin;
    double keyX = _legendStart.x() + legendWidth / 2 - diam5seq/2;
    
    painter->setBrush(Qt::transparent);
    painter->drawEllipse(keyX, currentY, diam5seq, diam5seq);
    QString key("10 samples");
    
    double textX = _legendStart.x() + legendWidth/2 - painterMetric.width(key)/2;//smallMetric.width(key)/2;
    painter->drawText(textX, currentY + diam5seq/2, key);
     
    currentY += (diam5seq - vertRadUnit);
    keyX = _legendStart.x() + legendWidth / 2 - vertRadUnit/2;
    painter->drawEllipse(keyX, currentY, vertRadUnit, vertRadUnit);
    
    currentY += vertRadUnit + margin; //smallMetric.ascent();
    key = QString("1 sample");
    textX = _legendStart.x() + legendWidth/2 - painterMetric.width(key)/2;//smallMetric.width(key)/2;
    painter->drawText(textX, currentY , key);
    
    currentY += margin;
    keyX = _legendStart.x() + margin;
    textX = keyX + vertRadUnit + margin;    
    painter->setFont(legendFont());
    painterMetric = painter->fontMetrics();

    _legendKeys = QVector<QRegion>(seqIDs.size());
    seqNameIt = seqIDs.constBegin();
    
    while (seqNameIt != seqIDs.constEnd())
    {
      painter->setBrush(hapBrush(seqNameIt.value()));
      
      painter->drawEllipse(keyX, currentY, vertRadUnit, vertRadUnit);
      painter->drawText(textX, currentY + (painterMetric.ascent() + vertRadUnit)/2, seqNameIt.key());
      /*textBox = metric.boundingRect(seqNameIt.key());
      painterRect = painter->boundingRect(textBox, Qt::AlignLeft, seqNameIt.key());
      qDebug() << "text width:" << metric.width(seqNameIt.key()) << "textBox width:" << textBox.width() << "painter box width:" << painterRect.width();*/
      _legendKeys[seqNameIt.value()] = QRegion(keyX, currentY, vertRadUnit, vertRadUnit, QRegion::Ellipse);
      
      currentY += entryHeight;
      ++seqNameIt;
    }
    
    
    
  //}
  
  //else
  //  qDebug() << "Not visible.";
 
  
  //qDebug() << "deleted";
  
  return true;
}

void HapLayer::updateLegendPos(const QPoint &p)
{
  _legendStart.rx() += p.x();
  _legendStart.ry() += p.y();

  // shouldn't be necessary, gets destroyed in render anyway
  _legendRegion.translate(p);
}

bool HapLayer::eventFilter(QObject *object, QEvent *event)
{
  if (object != _target)  return false;

  QMouseEvent *mEvent = dynamic_cast<QMouseEvent*>(event);

  if (! mEvent)  return false;

  bool returnVal = false;

  switch (event->type())
  {
  case QEvent::MouseButtonPress:

    if (_legendRegion.contains(mEvent->pos()))
    {
      //qDebug() << "pressed in legend";
      _clickedInLegend = true;
      for (unsigned i = 0; i < _legendKeys.size(); i++)
      {
        if (_legendKeys.at(i).contains(mEvent->pos()))
        {
          qDebug() << "pressed in legend key" << i;
          _clickedInKey = i;
        }
      }
      _mouseDownPos = mEvent->pos();
      returnVal = true;
      break;
    }

    else  _clickedInLegend = false;

    for(unsigned i = 0; i < _clusters.size(); i++)
    {
      const QRegion &clust = _clusters.at(i);
      if (clust.contains(mEvent->pos()))
      {
        qDebug() << "pressed in cluster" << _clustLabels.at(i);
        returnVal = true;
      }
    }
    break;

  case QEvent::MouseMove:
    if (_clickedInLegend)
    {
      //qDebug() << "clicked in legend, moving";
      QPoint moved = mEvent->pos() - _mouseDownPos;
      //qDebug() << "moved:" << moved.x() << moved.y();

      // A bit useless, unless I learn to update part of a MapWidget
      QRegion region(_legendRegion);
      updateLegendPos(moved);
      region = region.intersected(_legendRegion);

      emit dirtyRegion(region);

      _mouseDownPos = mEvent->pos();

      returnVal = true;
    }
    break;

  case QEvent::MouseButtonRelease:
    if (_legendRegion.contains(mEvent->pos()))
    {
      //qDebug() << "released in legend";
      returnVal = true;
    }
    _clickedInLegend = false;
    break;

  default:
    returnVal = false;
    break;
  }

  return returnVal;
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


