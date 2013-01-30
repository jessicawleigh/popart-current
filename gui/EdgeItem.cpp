#include "EdgeItem.h"

#include <QTransform>
#include <QLineF>

#include <cmath>
#include <iostream>
using namespace std;

EdgeItem::EdgeItem(const VertexItem *startVert, const VertexItem *endVert, QGraphicsItem *parent)
  : QGraphicsLineItem(QLineF(startVert->boundingRect().center(), endVert->boundingRect().center()), parent),
  _originalStart(startVert->boundingRect().center()),
  _originalEnd(endVert->boundingRect().center())
{
  connect(startVert, SIGNAL(moved(const QPointF &)), this, SLOT(moveStart(const QPointF&)));
  connect(endVert, SIGNAL(moved(const QPointF &)), this, SLOT(moveEnd(const QPointF&)));

  //cout << "connected." << endl;
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

