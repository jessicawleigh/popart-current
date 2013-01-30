#ifndef BORDERRECTITEM_H_
#define BORDERRECTITEM_H_

#include <QBrush>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QVariant>
#include <QVector>

class BorderRectItem : public QGraphicsRectItem
{
public:
  BorderRectItem(const QRectF &, QGraphicsItem * = 0);
  BorderRectItem(qreal, qreal, qreal, qreal, QGraphicsItem * = 0 );
  //virtual QRectF boundingRect() const;// { return _rect; };
  void updateRect(); 
 
  class BorderGrip : public QGraphicsRectItem
  {
  public:
    typedef enum {MouseDown, MouseUp, MouseMoving} MouseState;
    
    BorderGrip(qreal, qreal, qreal, qreal, QGraphicsItem * = 0 );
    void setHoverBrush(QBrush brush) { _hoverBrush = brush; };
    const QBrush & hoverBrush() const { return _hoverBrush; };
    void setNormalBrush(QBrush brush) { _normalBrush = brush; setBrush(_normalBrush); };
    const QBrush & normalBrush() const { return _normalBrush; };
    void setMouseDownPos(const QPointF &pos) {_mouseDownPos = pos; };
    const QPointF & mouseDownPos() const { return _mouseDownPos; };
    void setMouseState(MouseState state) { _state = state; };
    MouseState mouseState() const { return _state; };
    
  protected:    
    //virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *) { setBrush(_hoverBrush); };  
    //virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *) { setBrush(_normalBrush); };
    
  private:
    QBrush _hoverBrush;
    QBrush _normalBrush;
    QPointF _mouseDownPos;
    MouseState _state;
  };
  
protected:
  virtual bool sceneEventFilter(QGraphicsItem *, QEvent *);
  virtual QVariant itemChange(GraphicsItemChange, const QVariant &);

private:
  
  void init();
  void setGripPositions();
  void setGripFilters();
  
  static const int GRIPWIDTH = 10;
  static const int GRIPLENGTH = 25;
  static const int MINWIDTH = 50;
  static const int MINHEIGHT = 50;
  static const QColor _gripColour;

  QVector<BorderGrip *> _grips;
  QBrush _gripBrush;
  QPen _gripPen;
  bool _gripFiltersSet;
};

#endif