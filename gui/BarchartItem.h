#ifndef BARCHARTPOPUP_H_
#define BARCHARTPOPUP_H_

#include <QBrush>
#include <QPen>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QVector>


class BarchartItem : public QGraphicsRectItem
{
public:
  BarchartItem(QGraphicsItem * = 0);
  
  void setTraits(const QVector<double> &, const QVector<QBrush> &);
  
protected:
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  
private:
  static const int WIDTH = 100;
  static const int HEIGHT = 60;
  static const int MARGIN = 5;
  static const int MINBARWIDTH = 5;
  QVector<QBrush> _colours;
  QVector<double> _traits;
  
  QColor _bgColour;
  QPen _outlinePen;
  QPen _borderPen;
  QPen _axisPen;
  
};

#endif