#include "BorderRectItem.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsScene>

const QColor BorderRectItem::_gripColour(50, 0, 128);

BorderRectItem::BorderRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
  : QGraphicsRectItem(x, y, width, height, parent)
{
  init();
}

BorderRectItem::BorderRectItem(const QRectF & rect, QGraphicsItem *parent)
  : QGraphicsRectItem(rect, parent)
{
  init();
}
    
void BorderRectItem::init()
{
  _gripFiltersSet = false;
  _gripBrush.setColor(_gripColour);
  _gripBrush.setStyle(Qt::SolidPattern);
  _gripPen.setBrush(_gripColour);
  
  _grips.push_back(new BorderGrip(0, 0, GRIPWIDTH, GRIPLENGTH, this));
  _grips.at(0)->setBrush(Qt::transparent);
  _grips.at(0)->setPen(_gripPen);
  _grips.at(0)->setData(0, 0);
  _grips.at(0)->setAcceptHoverEvents(true);
  
  _grips.push_back(new BorderGrip(0, 0, GRIPWIDTH, GRIPLENGTH, this));
  _grips.at(1)->setBrush(Qt::transparent);
  _grips.at(1)->setPen(_gripPen);
  _grips.at(1)->setData(0, 1);
  _grips.at(1)->setAcceptHoverEvents(true);
  
  _grips.push_back(new BorderGrip(0, 0, GRIPLENGTH, GRIPWIDTH, this));
  _grips.at(2)->setBrush(Qt::transparent);
  _grips.at(2)->setPen(_gripPen);
  _grips.at(2)->setData(0, 2);
  _grips.at(2)->setAcceptHoverEvents(true);
  
  _grips.push_back(new BorderGrip(0, 0, GRIPLENGTH, GRIPWIDTH, this));
  _grips.at(3)->setBrush(Qt::transparent);
  _grips.at(3)->setPen(_gripPen);  
  _grips.at(3)->setData(0, 3);
  _grips.at(3)->setAcceptHoverEvents(true);
  
  setGripPositions();
  
}

void BorderRectItem::setGripPositions()
{
  QRectF myrect = rect();
  
  double xpos = myrect.left();
  double ypos = myrect.center().y() - GRIPLENGTH / 2;
  _grips.at(0)->setPos(xpos, ypos);
  
  xpos = myrect.right() - GRIPWIDTH;
  _grips.at(1)->setPos(xpos, ypos);
  
  ypos = myrect.top(); 
  xpos = myrect.center().x() - GRIPLENGTH / 2;
  _grips.at(2)->setPos(xpos, ypos);

  ypos = myrect.bottom() - GRIPWIDTH;
  _grips.at(3)->setPos(xpos, ypos);
}

void BorderRectItem::setGripFilters()
{
  
  _grips.at(0)->installSceneEventFilter(this);
  _grips.at(1)->installSceneEventFilter(this);
  _grips.at(2)->installSceneEventFilter(this);
  _grips.at(3)->installSceneEventFilter(this);
  _gripFiltersSet = true;

}

QVariant BorderRectItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
  
  
  if (! _gripFiltersSet && change == ItemSceneHasChanged && scene())
    setGripFilters();
    
  return QGraphicsItem::itemChange(change, value);
}

bool BorderRectItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    
  QGraphicsSceneMouseEvent * mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
  
  QVariant data = watched->data(0);
  
  if (data == QVariant())  return false; // shouldn't happen
  int gripID = data.toInt();
  
  bool returnval = false;
  BorderGrip *grip = _grips.at(gripID);
  
  switch (event->type())
  {
    case QEvent::GraphicsSceneHoverEnter:
      returnval = true;
      grip->setBrush(_gripBrush);
      grip->update(grip->boundingRect());
      break;
    case QEvent::GraphicsSceneHoverLeave:
      returnval = true;
      grip->setBrush(Qt::transparent);
      grip->update(grip->boundingRect());
      break;
    case QEvent::GraphicsSceneMousePress:
      returnval = true;
      grip->setMouseState(BorderGrip::MouseDown);
      grip->setMouseDownPos(mouseEvent->pos());
      break;
    case QEvent::GraphicsSceneMouseRelease:
      returnval = true;
      grip->setMouseState(BorderGrip::MouseUp);
      if (grip->pos() != grip->mouseDownPos())
        scene()->setSceneRect(rect());
      break;
    case QEvent::GraphicsSceneMouseMove:
      returnval = true;
      grip->setMouseState(BorderGrip::MouseMoving);
      break;
    default:
      return false;
      break;
  }
  
  if (mouseEvent && grip->mouseState() == BorderGrip::MouseMoving)
  {
    QPointF eventPos = mouseEvent->pos();
    
    int xsign = 0;
    int ysign = 0;
    int xoffset = 0;
    int yoffset = 0;
    
    switch (gripID)
    {
      case 0: // left
        //xsign = -1;
        xoffset = 1;
        break;
      case 1: // right
        xsign = 1;
        break;
      case 2: // top
        //ysign = -1;
        yoffset = 1;
        break;
      case 3: // bottom
        ysign = 1;
        break;
      default: // will never happen
        break;
    }
  
  
    double xMoved = eventPos.x() - grip->mouseDownPos().x();
    double yMoved = eventPos.y() - grip->mouseDownPos().y();
    
    QRectF newRect(rect());
    
    double maxX = newRect.x() + newRect.width() - MINWIDTH;
    double maxY = newRect.y() + newRect.height() - MINHEIGHT;
    newRect.setX(qMin(newRect.x() + xoffset * xMoved, maxX));
    newRect.setY(qMin(newRect.y() + yoffset * yMoved, maxY));
    
    newRect.setWidth(qMax(newRect.width() + xMoved * xsign, (qreal)MINWIDTH));
    newRect.setHeight(qMax(newRect.height() + yMoved * ysign, (qreal)MINHEIGHT));
     
    
    prepareGeometryChange();
    setRect(newRect);
    setGripPositions();
    //QGraphicsScene *thescene = scene();
    //scene()->setSceneRect(newRect);//rect());
    
  }
   
  return returnval;
}

void BorderRectItem::updateRect()
{
  setRect(scene()->sceneRect());
  setGripPositions();
}


BorderRectItem::BorderGrip::BorderGrip(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
  : QGraphicsRectItem(x, y, width, height, parent)
{}



