#include "BarchartItem.h"

#include <QPoint>
#include <iostream>
using namespace std;

BarchartItem::BarchartItem(QGraphicsItem *parent)
  : QGraphicsRectItem(0, 0, WIDTH, HEIGHT, parent),
  _bgColour(Qt::white)
{ 
  
  _borderPen.setStyle(Qt::DashLine);
  _axisPen.setWidth(2);
  
}

void BarchartItem::setTraits(const QVector<double> &traitfreqs, const QVector<QBrush> &colours)
{
  _traits.clear();
  
  _traits << traitfreqs;
  _colours = colours;
}


void BarchartItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{ 
  QRectF rect = boundingRect();
  
  painter->setPen(_borderPen);
  painter->setBrush(_bgColour);
  painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
  
  if (!_traits.empty())
  {
    QPointF start(rect.x() + MARGIN, rect.y() + rect.height() - MARGIN);
    QPointF end(rect.x() + rect.width() - MARGIN, start.y());
    QPointF upperLeft(start.x(), rect.y() + MARGIN);
    double plotHeight = rect.height() - 2 * MARGIN;
    double plotWidth = rect.width() - 2 * MARGIN;
    double barWidth = (plotWidth - MARGIN)/_traits.size() - MARGIN;
    double barMarg = MARGIN;
    
    if (barWidth < 0)
    {
      QPointF textPos(upperLeft.x() + MARGIN, upperLeft.y() + MARGIN);
      painter->drawText(textPos, "Error");
      return;
    }
    
    else if (barWidth < MINBARWIDTH)
    {
      barMarg -= (MINBARWIDTH - barWidth);
      barWidth = MINBARWIDTH;
    }
        
    painter->setPen(_axisPen);
    painter->drawLine(start, end);
    painter->drawLine(start, upperLeft);
    
    painter->setPen(_outlinePen);
    
    double left = start.x() + MARGIN;
    
    for (unsigned i = 0; i < _traits.size(); i++)
    {
      double barHeight = _traits.at(i) * plotHeight;
      double top = rect.y() + MARGIN + (plotHeight - barHeight);
      
      painter->setBrush(_colours.at(i % _colours.size()));
      painter->drawRect(left, top, barWidth, barHeight);
      left += barWidth + barMarg;
    }
  }
}
