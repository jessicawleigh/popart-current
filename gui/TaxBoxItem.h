#ifndef TAXBOXITEM_H_
#define TAXBOXITEM_H_

#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <QVector>


class TaxBoxItem : public QGraphicsRectItem
{
public:
  TaxBoxItem(QGraphicsItem * = 0);
  
  void setLabels(const QVector<QString> &);
  
protected:
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  
private:
  static const int WIDTH = 100;
  static const int HEIGHT = 60;
  static const int MARGIN = 5;
  
  const QColor _bgColour;
  const QColor _labelColour;
  const QFont _labelFont;
  const QFontMetricsF _metric;
  QPen _borderPen;
  QVector<QString> _labels; 
  
};

#endif