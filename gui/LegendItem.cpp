#include "LegendItem.h"

LegendItem::LegendItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
  : QGraphicsEllipseItem(x, y, width, height, parent)
{
  _clicked = false;
}

void LegendItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  _clickPoint = event->pos();
  _clicked = true;
  
  //QGraphicsEllipseItem::mousePressEvent(event);
}

void LegendItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{  

  if (_clicked && event->pos() == _clickPoint)
    emit clicked(this);
  
  else
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}
