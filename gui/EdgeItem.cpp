#include "EdgeItem.h"

#include <QFontMetrics>
#include <QLineF>
#include <QPointF>
#include <QRectF>

#include <cmath>
#include <iostream>
using namespace std;

EdgeItem::EdgeItem(const VertexItem *startVert, const VertexItem *endVert, double weight, QGraphicsItem *parent)
  : QGraphicsLineItem(QLineF(startVert->boundingRect().center(), endVert->boundingRect().center()), parent),
  _originalStart(startVert->boundingRect().center()),
  _originalEnd(endVert->boundingRect().center()),
  _weight(weight)
{
  connect(startVert, SIGNAL(moved(const QPointF &)), this, SLOT(moveStart(const QPointF&)));
  connect(endVert, SIGNAL(moved(const QPointF &)), this, SLOT(moveEnd(const QPointF&)));

  //cout << "connected." << endl;
}

void EdgeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *parent)
{
  QGraphicsLineItem::paint(painter, option, parent);
  bool showdashes = false;
  bool showellipses = false;
  bool shownums = true;

  if (showdashes)
  {

    QPointF midpoint = boundingRect().center();
    QPointF start = line().p1();
    QPointF end = line().p2();

    /*cout << "midpoint: " << midpoint.x() << "," << midpoint.y() << endl;
    cout << "start: " << start.x() << "," << start.y() << endl;
    cout << "end: " << end.x() << ","<< end.y() << endl;*/

    double length = 10;
    double r = sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2));
    double spacing = r / (_weight + 1);

    if (spacing > length/2)
      spacing = length/2;
    double xspacing = spacing * (end.x() - start.x())/ r;
    double yspacing = spacing * (end.y() - start.y())/ r;


    //double width = boundingRect().width();
    //double height = boudningRect().height();

    double dx, dy;

    if (end.x() == start.x())
    {
      dx = length;
      dy = 0;

    }

    else if (end.y() == start.y())
    {
      dx = 0;
      dy = length;
    }

    else
    {
      double slope = (end.y() - start.y())/(end.x() - start.x());
      dy = sqrt(pow(length, 2) / (pow(slope, 2) + 1));
      dx = - slope * dy;
    }


    double currentx = midpoint.x() - xspacing * _weight * 0.5;
    double currenty = midpoint.y() - yspacing * _weight * 0.5;


    for (unsigned i = 0; i < _weight; i++)
    {
      painter->drawLine(currentx - dx/2, currenty - dy/2, currentx + dx/2, currenty + dy/2);
      currentx += xspacing;
      currenty += yspacing;
    }
  }

  else if (showellipses && _weight > 1)
  {
    QPointF midpoint = boundingRect().center();
    QPointF start = line().p1();
    QPointF end = line().p2();
    unsigned diam = 10;

    /*cout << "midpoint: " << midpoint.x() << "," << midpoint.y() << endl;
    cout << "start: " << start.x() << "," << start.y() << endl;
    cout << "end: " << end.x() << ","<< end.y() << endl;*/

    double r = sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2));
    double spacing = r / _weight;

    double xspacing = spacing * (end.x() - start.x())/ r;
    double yspacing = spacing * (end.y() - start.y())/ r;

    double currentx = midpoint.x() - xspacing * (_weight * 0.5 - 1);
    double currenty = midpoint.y() - yspacing * (_weight * 0.5 - 1);


    for (unsigned i = 1; i < _weight; i++)
    {
      painter->setBrush(Qt::black);
      painter->drawEllipse(currentx - diam/2, currenty - diam/2, diam, diam);
      currentx += xspacing;
      currenty += yspacing;
    }

  }

  else if (shownums)
  {
    QPointF midpoint = boundingRect().center();
    QPointF start = line().p1();
    QPointF end = line().p2();
    double slope = (end.y() - start.y())/(end.x() - start.x());
    // TODO: set font to be consistent with NetworkView

    QString numstr = tr("(%1)").arg(_weight);

    QRectF textrect = painter->fontMetrics().boundingRect(numstr);
    if (slope < 0)  textrect.moveTo(midpoint.x(), midpoint.y());
    else  textrect.moveTo(midpoint.x(), midpoint.y() - textrect.height());

    painter->drawText(textrect, Qt::AlignLeft, numstr);
  }
  //painter->drawLine(midpoint.x() - dx/2, midpoint.y() - dy/2, midpoint.x() + dx/2, midpoint.y() + dy/2);

  // TODO figure out new bounding rectangle for line, if necessary
}


// TODO figure out how to use transforms
void EdgeItem::moveStart(const QPointF & startMove)
{
  QPointF newStart = startMove + _originalStart;
  setLine(QLineF(newStart, line().p2()));
}

void EdgeItem::moveEnd(const QPointF & endMove)
{
  QPointF newEnd = endMove + _originalEnd;
  setLine(QLineF(line().p1(), newEnd));
}

