#include "TaxBoxItem.h"

#include <QPoint>
#include <iostream>
using namespace std;

TaxBoxItem::TaxBoxItem(QGraphicsItem *parent)
  : QGraphicsRectItem(0, 0, WIDTH, HEIGHT, parent),
  _bgColour(Qt::white),
  _labelFont("Baskerville", 10), 
  _labelColour(50, 0, 128),
  _metric(_labelFont)
{ 
  _borderPen.setWidth(1);
  _borderPen.setStyle(Qt::DashLine);
}

void TaxBoxItem::setLabels(const QVector<QString> & labels)
{
  _labels.clear(); 
  _labels << labels;
  
  
  double height = MARGIN * 2 + _labels.size() * _metric.height();  
  //if (HEIGHT > height)  height = HEIGHT;
  
  double width = MARGIN;
  
  for (unsigned i = 0; i < _labels.size(); i++)
    width = qMax(width, _metric.width(_labels.at(i)));
  
  width += 2 * MARGIN;

  QRectF r = rect();
  r.setWidth(width);
  r.setHeight(height);
  
  setRect(r);
}

void TaxBoxItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{ 
  
  
  QRectF rect = boundingRect();
  
  painter->setPen(_borderPen);
  painter->setBrush(_bgColour);
  painter->setFont(_labelFont);
  painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
  
  double xpos = rect.x() + MARGIN;
  double ypos = rect.y() + _metric.height() + MARGIN;
  
  QPen bluepen;
  bluepen.setBrush(_labelColour);
  //painter->setBrush(_labelColour);
  painter->setPen(bluepen);
  
  for (unsigned i = 0; i < _labels.size(); i++)
  {
    painter->drawText(xpos, ypos, _labels.at(i));
    ypos += _metric.height();
  }
}
