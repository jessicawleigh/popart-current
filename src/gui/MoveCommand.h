#ifndef MOVECOMMAND_H_
#define MOVECOMMAND_H_

#include <QGraphicsItem>
#include <QPair>
#include <QPointF>
#include <QList>
#include <QString>
#include <QUndoCommand>

class MoveCommand : public QUndoCommand
{
public:
  MoveCommand(QList<QPair<QGraphicsItem *, QPointF> >, QUndoCommand * = 0);
  
  virtual void undo();
  virtual void redo();
  virtual bool mergeWith(const QUndoCommand *);
  virtual int id() const { return 1337; } // arbitrary value, not really used

private:
  
  typedef struct 
  {
    QGraphicsItem *item;
    QPointF oldPos;
    QPointF newPos;
    
  }
  PositionedItem;
  
  QList<PositionedItem> _items;

} ;

#endif
