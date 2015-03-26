#include "NetworkScene.h"
#include "BorderRectItem.h"

#include <QList>
#include <QTransform>

#include <iostream>
using namespace std;

NetworkScene::NetworkScene(QObject *parent)
  : QGraphicsScene(parent)
{
}

void NetworkScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QPointF mousePos(event->buttonDownScenePos(Qt::LeftButton).x(),
                   event->buttonDownScenePos(Qt::LeftButton).y());  
  
  QGraphicsItem *movingItem = itemAt(mousePos.x(), mousePos.y(), QTransform());  
  
  BorderRectItem::BorderGrip *gripItem = dynamic_cast<BorderRectItem::BorderGrip *>(movingItem);
  
  
  if (movingItem && ! gripItem) 
  {
    
    // Vertices are several items stacked on top of each other
    // want bottom-most item with the same "data" (i.e., same vertex ID)
    if (movingItem->data(0) != QVariant())
    {
      int intValue = movingItem->data(0).toInt();
      QList<QGraphicsItem*> itemsHere = collidingItems(movingItem);
    
      QList<QGraphicsItem*>::iterator itemIt = itemsHere.begin();
    
      while (itemIt != itemsHere.end())
      {
        QVariant data = (*itemIt)->data(0);
        if (data != QVariant() && data.toInt() == intValue)
        {
          movingItem = *itemIt;
        }
        
        ++itemIt;
      }      
    }
    
    if (movingItem && event->button() == Qt::LeftButton) 
    {      
      QList<QGraphicsItem *> items = selectedItems();
      
      if (items.isEmpty() || ! movingItem->isSelected())
      {
        _movingItems.push_back(QPair<QGraphicsItem *, QPointF >(movingItem, movingItem->pos()));
        
        // TODO check if control is pressed
        // important if movingItem isn't selected, irrelevant if there's no selection
        clearSelection(); 
        
      }
      
      else
      {
        int itemCount = 0;
        QList<QGraphicsItem *>::iterator itemIt = items.begin();
        
        while (itemIt != items.end())
        {        
          // Deselect child if parent is also selected
          if ((*itemIt)->parentItem() && (*itemIt)->parentItem()->isSelected())
            (*itemIt)->setSelected(false);
          else
            _movingItems.push_back(QPair<QGraphicsItem *, QPointF >(*itemIt, (*itemIt)->pos()));
          
          itemCount++;
          ++itemIt;
        }
               
      }
    }
      
  }
  
  else  clearSelection();
  
  QGraphicsScene::mousePressEvent(event);
}

void NetworkScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (! _movingItems.isEmpty() && event->button() == Qt::LeftButton) 
  {
    QGraphicsItem *item = _movingItems.front().first;
    QPointF & oldPos = _movingItems.front().second;
    QList<QPair<QGraphicsItem *, QPointF > >::iterator itemIt = _movingItems.begin();
    
    // child items' positions remain constant if parent moves too
    while (item->parentItem() && itemIt != _movingItems.end())
    {
      item = itemIt->first;
      oldPos = itemIt->second;
      ++itemIt;
    }
        
    if (oldPos != item->pos())   
      emit itemsMoved(_movingItems);

  }
  
  _movingItems.clear();
  
  
  QGraphicsScene::mouseReleaseEvent(event);  
  
}
