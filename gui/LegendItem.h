#ifndef LEGENDITEM_H_
#define LEGENDITEM_H_

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>

class LegendItem : public QObject, public QGraphicsEllipseItem
{
  Q_OBJECT
public:
  LegendItem(qreal, qreal, qreal, qreal, QGraphicsItem * = 0);
  
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
  
private:
  QPointF _clickPoint;
  bool _clicked;
signals:
  void clicked(LegendItem *);
  
};

#endif