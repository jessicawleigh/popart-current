#ifndef NETWORKSCENE_H_
#define NETWORKSCENE_H_

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QList>
#include <QPair>
#include <QPointF>


class NetworkScene : public QGraphicsScene
{
Q_OBJECT
public:
  NetworkScene(QObject * = 0);
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

private:
  //QGraphicsItem *_movingItem;
  //QPointF _oldPos;
  QList <QPair<QGraphicsItem *, QPointF> > _movingItems;

signals:
  //void itemMoved(QGraphicsItem *, const QPointF &);
  void itemsMoved(QList<QPair<QGraphicsItem *, QPointF> >);
};

#endif
