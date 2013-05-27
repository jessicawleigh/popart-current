#include "HapLayer.h"
#include "NetworkItem.h"

#include <marble/GeoDataCoordinates.h>
#include <marble/ViewportParams.h>

#include <QAction>
#include <QBrush>
#include <QColor>
#include <QContextMenuEvent>
#include <QDebug>
#include <QFontMetrics>
#include <QHoverEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QRectF>

using namespace Marble;

HapLayer::HapLayer(QVector<HapLocation*> locations, QObject *parent)
  : QObject(parent), _hapLocations(locations), _defaultBrush(Qt::black)
{
  _defaultFont.setFamily("Baskerville");
  _defaultFont.setPointSize(10);
  _legendFont = _defaultFont;
  
  _smallFont = QFont(_defaultFont);
  _smallFont.setPointSize(6); 

  _target = 0;
  _clickedInLegend = false;
  _legendStart.setX(-1);
  _legendStart.setY(-1);
  _enteredPos = QPoint(-1,-1);
}


QStringList HapLayer::renderPosition() const
{
  //QStringList layers = QStringList() << "SURFACE" << "HOVERS_ABOVE_SURFACE";
  //layers << "ORBIT" << "USER_TOOLS" << "STARS";


  // which layer to paint in? ORBIT is above location names, HOVERS_ABOVE_SURFACE is under
  return QStringList() << "ORBIT";//"HOVERS_ABOVE_SURFACE"; //layers.at(index);

}

bool HapLayer::render(GeoPainter *painter, ViewportParams *viewport, const QString &renderPos, GeoSceneLayer *layer)
{
  _clusters.clear();
  _clustLabels.clear();
  _legendKeys.clear();

  double vertRadUnit = NetworkItem::VERTRAD;
  painter->setPen(Qt::SolidLine);

  const unsigned circleUnits = 5760;

  for (unsigned i = 0; i < _hapLocations.size(); i++)
  {
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
  

  const QMap<QString, unsigned> & seqIDs = HapLocation::seqIDs();
  
  // TODO set a bool if using another painter (i.e., to save a file) and use new QFontMetrics to do font geometry
  // draw legend to a separate window if too long
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

  double diam5seq = sqrt(10) * vertRadUnit;
  double minLegendWidth = diam5seq + 2 * margin;
  double legendWidth = maxwidth + vertRadUnit + 3 * margin;

  if (legendWidth < minLegendWidth)  legendWidth = minLegendWidth;

  painter->setFont(smallFont());
  painterMetric = painter->fontMetrics();

  double legendHeight = entryHeight * seqIDs.size() + painterMetric.height() + diam5seq + 3 * margin;

  if (_legendStart.x() < 0)
  {
    _legendStart.setX(x - legendWidth);
    _legendStart.setY(y - legendHeight);
    
    if (_legendStart.y() < 0)
      _legendStart.setY(0);
  }

  painter->setBrush(Qt::white);
  painter->drawRect(_legendStart.x(), _legendStart.y(), legendWidth, legendHeight);
  qreal lon, lat;
  bool inglobe;
  double goodX = _legendStart.x();
  double goodY = _legendStart.y();
  double goodWidth = legendWidth;
  double goodHeight = legendHeight;
    
  if (goodX < 0)  goodX = 0;
  if (goodY < 0)  goodY = 0;
  
  if (goodWidth > painterRect.width() - goodX)
    goodWidth = painterRect.width() - goodX;
  
  if (goodHeight > painterRect.height() - goodY)
    goodHeight = painterRect.height() - goodY;
    
  inglobe = viewport->geoCoordinates(goodX + goodWidth/2, goodY + goodHeight/2, lon, lat);

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
    _legendKeys[seqNameIt.value()] = QRegion(keyX, currentY, vertRadUnit, vertRadUnit, QRegion::Ellipse);
    
    currentY += entryHeight;
    ++seqNameIt;
  }
      
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
  QContextMenuEvent *cEvent = dynamic_cast<QContextMenuEvent*>(event);
  QHoverEvent *hEvent = dynamic_cast<QHoverEvent*>(event);
  //if (! mEvent)  return false;

  bool returnVal = false;

  switch (event->type())
  {
  case QEvent::ContextMenu:
    if (_legendRegion.contains(cEvent->pos()))
    {
      //qDebug() << "pressed in legend";
      for (unsigned i = 0; i < _legendKeys.size(); i++)
      {
        if (_legendKeys.at(i).contains(cEvent->pos()))
        {
          _clickedInKey = i;
          QMenu menu;
          QAction *a = menu.addAction(tr("Change sequence colour"));
          connect(a, SIGNAL(triggered()), this, SLOT(changeColour()));
          //a = menu.addAction(tr("Change sequence label"));
          a = menu.exec(_target->mapToGlobal(cEvent->pos()));
          break;
        }
      }
    }
    returnVal = true;
    break;
  case QEvent::MouseButtonPress:

    if (_legendRegion.contains(mEvent->pos()))
    {
      returnVal = true;
      if (mEvent->button() == Qt::LeftButton)
      {
         _clickedInLegend = true;
         _mouseDownPos = mEvent->pos();
      }

      else  _clickedInLegend = false;
    }
    
    for(unsigned i = 0; i < _clusters.size(); i++)
    {
      const QRegion &clust = _clusters.at(i);
      if (clust.contains(mEvent->pos()))
      {
        //qDebug() << "pressed in cluster" << _clustLabels.at(i);
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

    else
    {
      for (unsigned i = 0; i < _clusters.size(); i++)
      {
        if (_clusters.at(i).contains(mEvent->pos()))
        {
          _enteredPos = mEvent->pos();
          emit entered(_clustLabels.at(i));
          returnVal = true;
        }

        else if (_enteredPos.x() > 0 && _clusters.at(i).contains(_enteredPos))
        {
          emit left(_clustLabels.at(i));
          _enteredPos.setX(-1);

          returnVal = true;
        }
      }
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

  case QEvent::HoverMove:
    for (unsigned i = 0; i < _clusters.size(); i++)
    {
      if (_clusters.at(i).contains(hEvent->pos()))
      {
        //_target->setToolTip(_clustLabels.at(i));
        emit entered(_clustLabels.at(i));
        qDebug() << "entered" << _clustLabels.at(i);
        returnVal = true;
      }

      else if (_clusters.at(i).contains(hEvent->oldPos()))
      {
        //_target->setToolTip("");
        emit left(_clustLabels.at(i));

        returnVal = true;
        qDebug() << "left" << _clustLabels.at(i);
      }
    }
    break;

  default:
    returnVal = false;
    break;
  }

  return returnVal;
}


const QBrush & HapLayer::hapBrush(int hapID) const
{
  if (_colours.empty() || hapID < 0)  return _defaultBrush;
  return _colours.at(hapID % _colours.size());
}

void HapLayer::changeColour()
{
  emit colourChangeTriggered(_clickedInKey);
}


