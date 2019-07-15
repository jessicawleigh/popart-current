#include "VertexItem.h"

#include <QPainterPath>
#include <QRadialGradient>
#include <QRectF>
#include <QStyle>

#include <iostream>
using namespace std;

//const QBrush VertexItem::HALOBRUSH(QColor(50, 0, 128));
QColor VertexItem::_backgroundColour(Qt::transparent);
QColor VertexItem::_highlightColour(50, 0, 128);

VertexItem::VertexItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
  : QGraphicsEllipseItem(x, y, width, height, parent)
{
  _isSelected = false;
}

QVariant VertexItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (change == ItemPositionChange && scene())
  {
    //emit aboutToMove(this);
    emit moved(value.toPointF());
  }

  return QGraphicsEllipseItem::itemChange(change, value);
}

void VertexItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{ 
  QStyleOptionGraphicsItem opt = *option;
  QRectF outerRect = boundingRect();
  QRectF innerRect = QGraphicsEllipseItem::boundingRect();
  
  if (opt.state & QStyle::State_Selected)
  {
      
    if (! _isSelected)
    {     
      
      outerRect.setX(outerRect.x() - HALOWIDTH);
      outerRect.setY(outerRect.y() - HALOWIDTH);
      outerRect.setWidth(outerRect.width() + HALOWIDTH);
      outerRect.setHeight(outerRect.height() + HALOWIDTH);
    }
    
    QRadialGradient grad(outerRect.center(), outerRect.width()/2, outerRect.center());
    grad.setColorAt(0, _highlightColour);
    grad.setColorAt(innerRect.width()/outerRect.width(), _highlightColour);
    grad.setColorAt(1, _backgroundColour);
    
    QBrush haloBrush(grad);
    
    painter->setBrush(haloBrush);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(outerRect);
    
    opt.state &= ~QStyle::State_Selected;
    QGraphicsEllipseItem::paint(painter, &opt, widget);
    prepareGeometryChange();
    _isSelected = true;
    
  }
  
  else
  {
    if (_isSelected)
      prepareGeometryChange();
    
    _isSelected = false;  

    opt.state &= ~QStyle::State_Selected;
    QGraphicsEllipseItem::paint(painter, &opt, widget);  
    

  } 
}

QRectF VertexItem::boundingRect() const
{
  if (_isSelected)
  {
    QRectF outerRect = QGraphicsEllipseItem::boundingRect();
    //double extraWidth = outerRect.width() * 0.5;
    //double extraHeight = outerRect.height() * 0.5;
    
    outerRect.setX(outerRect.x() - HALOWIDTH);
    outerRect.setY(outerRect.y() - HALOWIDTH);
    outerRect.setWidth(outerRect.width() + HALOWIDTH);
    outerRect.setHeight(outerRect.height() + HALOWIDTH);
    
    return outerRect;
  }
  
  else
    return QGraphicsEllipseItem::boundingRect();
  
}

void VertexItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{    
  emit hoverEntered(this);
}

void VertexItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{  
  emit hoverLeft();
}


