#include "LegendItem.h"

#include <QAction>
#include <QDebug>
#include <QMenu>

LegendItem::LegendItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
  : QGraphicsEllipseItem(x, y, width, height, parent)
{
  _clicked = false;
  setAcceptHoverEvents(true);
}

void LegendItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  //qDebug() << "entered";
  emit clickable(true);
}

void LegendItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  //qDebug() << "left";
  emit clickable(false);
}

/*void LegendItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->button() == Qt::RightButton)
  {
    _clickPoint = event->pos();
    _clicked = true;
  }
  
  //QGraphicsEllipseItem::mousePressEvent(event);
}

void LegendItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{  

  if (_clicked && event->pos() == _clickPoint)
    emit clicked(this);
  
  else
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}*/

void LegendItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QAction *a = menu.addAction(tr("Change trait colour"));
  connect(a, SIGNAL(triggered()), this, SLOT(changeColour()));
  // a = menu.addAction(tr("Change trait label"));

  a = menu.exec(event->screenPos());
}

void LegendItem::changeColour()
{
  emit colourChangeTriggered(this);
}


