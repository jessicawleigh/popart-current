/*
 * Edge.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

using namespace std;

#include "Edge.h"


Edge::Edge(const Vertex *u, const Vertex *v, unsigned int index, double weight, const void *info, Colour colour)
{
  _from = u;
  _to = v;
  _index = index;
  _weight = weight;//(unsigned)weight;
  _info = info;
  _marked = false;
  _colour = Black;
}

Edge::~Edge()
{
  // TODO Auto-generated destructor stub
}

void setIndex(unsigned idx, Edge &e)
{
  e.setIndex(idx);
}

ostream &operator<<(ostream &os, const Edge &e)
{
  os << e.index() << ". " << (e.from())->index();
  os << " (" << e._weight << ") " << (e.to())->index();

  return os;
}
