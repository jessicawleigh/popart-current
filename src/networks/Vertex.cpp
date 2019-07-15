/*
 * Node.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

using namespace std;

#include "NetworkError.h"
#include "Vertex.h"


Vertex::Vertex(const string &label, int index, const void *info, Colour col)
  : _label(label)
{
  _index = index;
  _info = info;
  _marked = false;
  _colour = col;
}

Vertex::~Vertex()
{
  // TODO Auto-generated destructor stub
}


void Vertex::addIncidentEdge(const Edge *e)
{
  if (e->from() != this && e->to() != this)
    throw NetworkError("Edge is not incident!");

  _incidences.push_back(e);
}

void Vertex::removeIncidentEdge(const Edge *e)
{
  list<const Edge*>::iterator edgeIt = _incidences.begin();
  bool found = false;

  while (edgeIt != _incidences.end())
  {
    if (*edgeIt == e)
    {
      _incidences.erase(edgeIt);
      found = true;
      edgeIt = _incidences.end();
    }

    ++edgeIt;
  }

  if (! found)  throw NetworkError("Edge not found in incidences!");
}

bool Vertex::isAdjacent(const Vertex *v) const
{
  list<const Edge*>::const_iterator edgeIt = _incidences.begin();

  while (edgeIt != _incidences.end())
  {
    if ((*edgeIt)->from() == v || (*edgeIt)->to() == v)
      return true;
    edgeIt++;
  }
  
  return false;
}

const Edge * Vertex::sharedEdge(const Vertex *v) const
{
  list<const Edge*>::const_iterator edgeIt = _incidences.begin();

  while (edgeIt != _incidences.end())
  {
    if ((*edgeIt)->from() == v || (*edgeIt)->to() == v)
      return (*edgeIt);
    edgeIt++;
  }
  
  return 0; 
}

bool Vertex::operator==(const Vertex &other) const
{
  if (_index != other.index())  return false;
  if (_label != other.label())  return false;
  if (_info != other.info()) return false;
  if (degree() != other.degree())  return false;


  /*for (int i = 0; i < degree(); i++)
    if (_incidences.at(i) != other._incidences.at(i)) return false;*/
  EdgeIterator edgeIt = begin();
  EdgeIterator otherEIt = other.begin();

  while (edgeIt != end())
  {
    if (*edgeIt != *otherEIt)  return false;
    ++edgeIt;
    ++otherEIt;
  }

  return true;
}

bool Vertex::operator!=(const Vertex &other) const
{
  return ! operator==(other);
}

Vertex::EdgeIterator Vertex::begin() const
{
  return EdgeIterator(this);
}

Vertex::EdgeIterator Vertex::rbegin() const
{
  return EdgeIterator(this, false, true);
}

Vertex::EdgeIterator Vertex::end() const
{
  return EdgeIterator(this, true);
}

Vertex::EdgeIterator Vertex::rend() const
{
  return EdgeIterator(this, true, true);
}

Vertex::EdgeIterator::EdgeIterator(const Vertex* node, bool isEnd, bool isReverse)
  : _edges(&(node->_incidences))
{
  _isEnd = isEnd;
  _isReverse = isReverse;
  //_edges = &(node->_incidences);

  
  if (_isReverse)
  {
    if (! _isEnd)
      _rEdgeIt = _edges->rbegin();
    else
      _rEdgeIt = _edges->rend();
    if (_rEdgeIt == _edges->rend()) 
      _isEnd = true;
  }
  
  else
  {
    if (! _isEnd)
      _edgeIt = _edges->begin();
    else
      _edgeIt = _edges->end();   
    
    if (_edgeIt == _edges->end()) 
      _isEnd = true;
  }

}

Vertex::EdgeIterator & Vertex::EdgeIterator::operator++()
{
  if (_isReverse) 
  {
    ++_rEdgeIt;
    if (_rEdgeIt == _edges->rend())  _isEnd = true;
  }
  else  
  {
    ++_edgeIt;
    if (_edgeIt == _edges->end())  _isEnd = true;
  }

  return *this;
}


/*
 * Need to write a copy constructor that works properly before doing this.
Node::EdgeIterator Node::EdgeIterator::operator++(int)
{
  EdgeIterator newIt = *this;
  ++(*this);

  return newIt;
}*/

const Edge* Vertex::EdgeIterator::operator*() const
{
  if (_isEnd)  return 0;
  
  else if (_isReverse)  return (*_rEdgeIt);
  
  else return (*_edgeIt);
}

bool Vertex::EdgeIterator::operator==(const Vertex::EdgeIterator & other) const
{
  if (_isEnd && other.isEnd())  return true;
  else if (_isEnd)  return false;
  else if (_isReverse)
  {
    if (! other.isReverse())  return false;
    return (*_rEdgeIt) == (*other);
  }
  
  else 
  {
    if (other.isReverse())  return false;
    return (*_edgeIt) == (*other);
  }
}

bool Vertex::EdgeIterator::operator!=(const Vertex::EdgeIterator &other) const
{
  // Why doesn't this work?
  //return ! (this == other);

  /*if (_isEnd && other.isEnd())  return false;
  else if (_isEnd)  return true;
  else return (*_edgeIt) != (*other);*/
  
  return ! operator==(other);
}

ostream &operator<<(ostream &os, const Vertex &n)
{
  os << n.index() << ". " << n.label();

  return os;
}

void setIndex(unsigned idx, Vertex &v)
{
  v.setIndex(idx);
}