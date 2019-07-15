#ifndef VERTEXITEM_H_
#define VERTEXITEM_H_

#include <QBrush>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QStyleOptionGraphicsItem>
#include <QVariant>
#include <QWidget>
#include <QColor>



class VertexItem : public QObject, public QGraphicsEllipseItem
{
  Q_OBJECT
public:
  VertexItem(qreal, qreal, qreal, qreal, QGraphicsItem * = 0);
  
  virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  virtual QRectF boundingRect() const;
public slots:
  void setBackgroundColour(QColor &colour) { _backgroundColour = colour; };
  void setHighlightColour(QColor &colour) { _highlightColour = colour; };
  //void enableTrackHover(
  
protected:
  virtual QVariant itemChange(GraphicsItemChange, const QVariant &);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);

private:
  
  const static unsigned HALOWIDTH = 5;
  bool _isSelected;
  //bool _trackHover;
  
  QRectF _outerRect;
  static QColor _highlightColour;
  static QColor _backgroundColour;

signals:
  //void aboutToMove(QGraphicsItem *);
  void moved(const QPointF &);
  void hoverEntered(QGraphicsItem*);
  void hoverLeft();
  
};

#endif