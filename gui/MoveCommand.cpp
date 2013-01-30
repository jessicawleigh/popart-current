#include <QGraphicsScene>

#include "MoveCommand.h"


#include <iostream>
using namespace std;

MoveCommand::MoveCommand(QList<QPair<QGraphicsItem *, QPointF> > items, QUndoCommand *parent)
  : QUndoCommand(parent)
{
  QList<QPair<QGraphicsItem *, QPointF> >::iterator itemIt = items.begin();
  
  while (itemIt != items.end())
  {
    PositionedItem posItem = {itemIt->first, itemIt->second, itemIt->first->pos()};
    _items.push_back(posItem);
    ++itemIt;
  }
}

bool MoveCommand::mergeWith(const QUndoCommand *command)
{
  const MoveCommand *moveCommand = static_cast<const MoveCommand *>(command);
  
  if (! moveCommand)  return false;
  
  //if (_items != moveCommand->_items)  return false;
  QList<PositionedItem>::iterator itemIt = _items.begin();
  QList<PositionedItem>::const_iterator otherIt = moveCommand->_items.begin();
  
  while (itemIt != _items.end())
  {
    if (itemIt->item != otherIt->item)  return false;
    ++itemIt;
    ++otherIt;
  }

  //_newPos = _item->pos();
  //setText(QObject::tr("move item at (%1,%2)\nmove").arg(_newPos.x()).arg(_newPos.y()));
  unsigned itemCount = 0;
  
  itemIt = _items.begin();
  
  while (itemIt != _items.end())
  {
    itemIt->newPos = itemIt->item->pos();
    itemCount++;
    ++itemIt;
  }

  if (itemCount <= 1)  
    setText(QObject::tr("move"));
  else
    setText(QObject::tr("move %1 items").arg(itemCount));
   
  return true;
}



void MoveCommand::undo()
{
 /* _item->setPos(_oldPos);
  _item->scene()->update();

  setText(QObject::tr("move item at (%1,%2)\nmove").arg(_newPos.x()).arg(_newPos.y()));*/
 
  unsigned itemCount = 0;
  
  QList<PositionedItem>::iterator itemIt = _items.begin();
  
  while (itemIt != _items.end())
  {
    itemIt->item->setPos(itemIt->oldPos);
    itemIt->item->scene()->update();
    itemCount++;
    ++itemIt;
  }

  if (itemCount <= 1)  
    setText(QObject::tr("move"));
  else
    setText(QObject::tr("move %1 items").arg(itemCount));
   
}

void MoveCommand::redo()
{
  /*_item->setPos(_newPos);

  setText(QObject::tr("move item at (%1,%2)\nmove").arg(_newPos.x()).arg(_newPos.y()));*/
  unsigned itemCount = 0;
  
  QList<PositionedItem>::iterator itemIt = _items.begin();
  
  while (itemIt != _items.end())
  {
    itemIt->item->setPos(itemIt->newPos);
    itemIt->item->scene()->update();
    itemCount++;
    ++itemIt;
  }

  if (itemCount <= 1)  
    setText(QObject::tr("move"));
  else
    setText(QObject::tr("move %1 items").arg(itemCount));
   
}


