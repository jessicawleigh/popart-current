#ifndef LABELITEM_H_
#define LABELITEM_H_


#include <QGraphicsSimpleTextItem>
#include <QGraphicsItem>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

class LabelItem :  public QObject, public QGraphicsSimpleTextItem
{
  Q_OBJECT
public:
  LabelItem(const QString &, QGraphicsItem * = 0);  
  virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

protected:
  virtual QVariant itemChange(GraphicsItemChange, const QVariant &);

signals:
  //void aboutToMove(QGraphicsItem *);
  void moved(const QPointF &);
};

#endif