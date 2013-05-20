#ifndef LEGENDITEM_H_
#define LEGENDITEM_H_

#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPointF>

class LegendItem : public QObject, public QGraphicsEllipseItem
{
  Q_OBJECT
public:
  LegendItem(qreal, qreal, qreal, qreal, QGraphicsItem * = 0);
  
protected:
  /*virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);*/
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
  

private:
  QPointF _clickPoint;
  bool _clicked;

private slots:
  void changeColour();

signals:
  void clicked(LegendItem *);
  void colourChangeTriggered(LegendItem *);
  //void entered(LegendItem *);
  //void left(LegendItem *);
  void clickable(bool);
  
};

#endif
