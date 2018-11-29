#include "LabelItem.h"

#include <QStyle>
#include <QVariant>

#include <iostream>
using namespace std;

LabelItem::LabelItem(const QString &text, QGraphicsItem *parent)
 : QGraphicsSimpleTextItem(text, parent)
{ }

void LabelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  QStyleOptionGraphicsItem opt = *option;
  
  opt.state &= ~QStyle::State_Selected;
  
  QGraphicsSimpleTextItem::paint(painter, &opt, widget);
  
}

QVariant LabelItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (change == ItemPositionChange && scene())
  {
   //emit aboutToMove(this);
   emit moved(value.toPointF());
  }

  return QGraphicsSimpleTextItem::itemChange(change, value);
}
