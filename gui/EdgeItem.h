#ifndef EDGEITEM_H_
#define EDGEITEM_H_

#include "VertexItem.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include <QPainter>
#include <QPointF>
#include <QStyleOptionGraphicsItem>
#include <QVariant>
#include <QWidget>


class EdgeItem : public QObject, public QGraphicsLineItem
{
  Q_OBJECT
public:
  EdgeItem(const VertexItem *, const VertexItem *, double, QGraphicsItem * = 0);
  //EdgeItem(qreal, qreal, qreal, qreal, QGraphicsItem * = 0);
  virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = 0);

public slots:
  void moveStart(const QPointF &);
  void moveEnd(const QPointF &);

private:
  const QPointF _originalStart;
  const QPointF _originalEnd;
  const double _weight;
  //double _dx;
  //double _dy;
 // double _dist;
  
};

#endif
