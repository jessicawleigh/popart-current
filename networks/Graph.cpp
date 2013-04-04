/*
 * Graph.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */
#include "Graph.h"
#include "NetworkError.h"

#include <limits>
#include <list>
using namespace std;



Graph::Graph()
{
  _edges = new vector<Edge *>;
  _vertices = new vector<Vertex *>;
  _FloydWarshallUpdated = false;
}

Graph::~Graph()
{
  vector<Edge *>::iterator eit;
  for (eit = _edges->begin(); eit != _edges->end(); eit++)  delete *eit;

  _edges->clear();
  delete _edges;

  vector<Vertex *>::iterator vit;
  for (vit = _vertices->begin(); vit != _vertices->end(); ++vit)  delete *vit;

  _vertices->clear();
  delete _vertices;

}

Edge * Graph::newEdge(Vertex *u, Vertex *v, double weight, const void *info)
{
  // Check u and v to make sure they belong to this graph:
  int uidx = u->index();
  int vidx = v->index();

  if (_vertices->at(uidx) != u || _vertices->at(vidx) != v)
    throw NetworkError("Attempting to add an edge between vertices that do not belong to this graph.");

  Edge *e = new Edge(u, v, _edges->size(), weight, info);

  _edges->push_back(e);
  u->addIncidentEdge(e);
  v->addIncidentEdge(e);
  
  _FloydWarshallUpdated = false;

  return e;
}

Vertex * Graph::newVertex(const string &label, const void *info)
{
  Vertex *n = new Vertex(label, _vertices->size(), info);

  _vertices->push_back(n);

  _FloydWarshallUpdated = false;

  return n;
}


Edge * Graph::edge(unsigned index)
{
  if (index >= _edges->size())
    throw NetworkError("Edge index out of bounds.");

  return _edges->at(index);
}

const Edge * Graph::edge(unsigned index) const
{
  if (index >= _edges->size())
    throw NetworkError("Edge index out of bounds.");

  return _edges->at(index);
}



const Vertex * Graph::vertex(unsigned index) const
{
  if (index >= _vertices->size())
    throw NetworkError("Vertex index out of bounds.");

  return _vertices->at(index);
}

Vertex * Graph::vertex(unsigned index)
{
  if (index >= _vertices->size())
    throw NetworkError("Vertex index out of bounds.");

  return _vertices->at(index);
}

Vertex * Graph::opposite(const Vertex *v, const Edge *e) 
{
  const Vertex *u = e->from();
  if (u == v)  return vertex(e->to()->index());

  u = e->to();
  if (u == v)  return vertex(e->from()->index());

  throw NetworkError("Vertex is not adjacent to Edge.");
}

const Vertex * Graph::opposite(const Vertex *v, const Edge *e) const
{
  const Vertex *u = e->from();
  if (u == v)  return e->to();

  u = e->to();
  if (u == v)  return e->from();

  throw NetworkError("Vertex is not adjacent to Edge.");
}

unsigned Graph::vertexCount() const
{
  return _vertices->size();
}

unsigned Graph::edgeCount() const
{
  return _edges->size();
}


void Graph::removeEdge(unsigned index)
{
  if (index >= _edges->size())
    throw NetworkError("Edge index out of bounds.");



  vector<Edge *>::iterator eit = _edges->begin();

  eit += index;

  Vertex *u = vertex((*eit)->from()->index());
  Vertex *v = vertex((*eit)->to()->index());

  u->removeIncidentEdge((*eit));
  v->removeIncidentEdge((*eit));

  delete (*eit);

  eit = _edges->erase(eit);

  while (eit != _edges->end())
  {
    //(*eit)->setIndex((*eit)->index() - 1);
    setIndex((*eit)->index() - 1, **eit);
    eit++;
  }
  
  _FloydWarshallUpdated = false;
}

void Graph::moveEdge(unsigned index, Vertex *f, Vertex *t)
{
  if (index >= _edges->size())
    throw NetworkError("Edge index out of bounds.");
  
  if (f->index() >= _vertices->size() || t->index() >= _vertices->size())
    throw NetworkError("Vertex index out of bounds.");
  
  if (f != _vertices->at(f->index()) && t != _vertices->at(t->index()))
    throw NetworkError("Vertex is not a member of this graph.");

  Edge *e = _edges->at(index);
  Vertex *v;
  v = vertex(e->from()->index());
  v->removeIncidentEdge(e);
  v = vertex(e->to()->index());
  v->removeIncidentEdge(e);

  
  e->setFrom(f);
  f->addIncidentEdge(e);

  
  e->setTo(t);
  t->addIncidentEdge(e);
  
  _FloydWarshallUpdated = false;
}

void Graph::removeVertex(unsigned index)
{
  if (index > _vertices->size())
    throw NetworkError("Vertex index out of bounds.");

  vector<Vertex *>::iterator vit = _vertices->begin();

  vit += index;
  
  /*Node::EdgeIterator edgeIt = (*nit)->begin();
  
  while (edgeIt != (*nit)->end())void moveEdge(unsigned, Vertex *, Vertex *)
  {
    removeEdge((*edgeIt)->index());
    ++edgeIt;
  }*/
  
  while ((*vit)->degree() > 0)
  {
    Vertex::EdgeIterator edgeIt = (*vit)->begin();
    removeEdge((*edgeIt)->index());
  }
    
  delete (*vit);

  vit = _vertices->erase(vit);

  while (vit != _vertices->end())
  {
    setIndex((*vit)->index() - 1, **vit);
    ++vit;
  }
  _FloydWarshallUpdated = false;

}

void Graph::unmarkEdges()
{
  vector<Edge*>::iterator edgeIt = _edges->begin();
  while (edgeIt != _edges->end())
  {
    (*edgeIt)->unmark();
    ++edgeIt;
  }  
}

void Graph::unmarkVertices()
{
  vector<Vertex*>::iterator vertIt = _vertices->begin();
  while (vertIt != _vertices->end())
  {
    (*vertIt)->unmark();
    ++vertIt;
  }
}

double Graph::pathLength(const Vertex *u, const Vertex *v)
{ 
  if (*(vertex(u->index())) != *u || *(vertex(v->index())) != *v)
    throw NetworkError("At least one of these vertices doesn't seem to belong to this Graph.");
  
  if (! _FloydWarshallUpdated)  updateFloydWarshall();
  
  return _pathLengths.at(u->index() * vertexCount() + v->index());
  
}

/* Note that this is for an UNDIRECTED graph */ 
void Graph::updateFloydWarshall()
{
  
  for (unsigned i = 0; i < _pathLengths.size(); i++)
  {
    _pathLengths.at(i) = numeric_limits<double>::max();
    _nextFW.at(i) = -1;
  }

  _pathLengths.resize(vertexCount() * vertexCount(), numeric_limits<unsigned>::max());
  _nextFW.resize(vertexCount() * vertexCount(), -1);
  
  const Vertex *u, *v;
  
  // Initialise path lengths for adjacent vertices
  for (unsigned i = 0; i < edgeCount(); i++)
  {
    u = edge(i)->from();
    v = edge(i)->to();
    _pathLengths.at(u->index() * vertexCount() + v->index()) = edge(i)->weight();
    _pathLengths.at(v->index() * vertexCount() + u->index()) = edge(i)->weight();
  }
  
  // Initialise path lengths on the diagonal
  for (unsigned i = 0; i < vertexCount(); i++)
    _pathLengths.at(i * vertexCount() + i) = 0;
  
  for (unsigned k = 0; k < vertexCount(); k++)
  {
    for (unsigned i = 0; i < vertexCount(); i++)
    {
      for (unsigned j = 0; j < i; j++)
      {
        double throughK = _pathLengths.at(i * vertexCount() + k) + _pathLengths.at(k * vertexCount() + j);
        if (throughK < _pathLengths.at(i * vertexCount() + j)) 
        {
          _pathLengths.at(i * vertexCount() + j) = throughK;
          _nextFW.at(i * vertexCount() + j) = k;
          _nextFW.at(j * vertexCount() + i) = k;
        }
        _pathLengths.at(j * vertexCount() + i) = _pathLengths.at(i * vertexCount() + j);
      }
    }
  }
  
  _FloydWarshallUpdated = true;
}

bool Graph::areConnected(Vertex *start, Vertex *end)
{

  if (start == end)  return true;
  vector<Vertex*>::iterator vertIt = _vertices->begin();
  while (vertIt != _vertices->end())
  {
    (*vertIt)->unmark();
    vertIt++;
  }

  queue<Vertex*> vertQueue;
  vertQueue.push(start);
  Vertex *u, *v;


  while (! vertQueue.empty())
  {
    u = vertQueue.front();
    if (! u->marked())
    {
      //if (u == end)  return true;

      Vertex::EdgeIterator edgeIt = u->begin();
      while (edgeIt != u->end())
      {
        v = opposite(u, (*edgeIt));
        if (v == end)  return true;

        if (! v->marked())   vertQueue.push(v);
        ++edgeIt;
      }

      u->mark();
    }

    vertQueue.pop();
  }

  return false;
}

Graph::DFSIterator Graph::beginDFS()
{
  return DFSIterator(this);
}

Graph::DFSIterator Graph::endDFS()
{
  return DFSIterator(this, true);
}

Graph::BFSIterator Graph::beginBFS()
{
  return BFSIterator(this);
}

Graph::BFSIterator Graph::endBFS()
{
  return BFSIterator(this, true);
}

Graph::PathIterator Graph::beginPath(Vertex *start, Vertex *end)
{
  return PathIterator(this, start, end);
}

Graph::PathIterator Graph::endPath()//const Node *, const Node *)
{
  return PathIterator(this, 0, 0, true);
}

Graph::VertIterator::VertIterator(Graph* graph, bool isEndIt)
{
  _vertsEnd = graph->_vertices->end();
  _isEnd = isEndIt;
  _verts = graph->_vertices;
  _graph = graph;

  if (! isEndIt)
  {
    _vertIt = _verts->begin();
    while (_vertIt != _verts->end())
    {
      (*_vertIt)->unmark();
      _vertIt++;
    }


    _vertIt = _verts->begin();
    _vertPtr = *_vertIt;
  }


  // unnecessary, but makes me feel better:
  else
  {
    _vertIt = _verts->end();
    _vertPtr = 0;
  }
}

bool Graph::VertIterator::isEnd() const
{
  return _isEnd;
}

void Graph::VertIterator::setEnd()
{
  _isEnd = true;
}

void Graph::VertIterator::setVertPtr(Vertex *v)
{
  _vertPtr = v;
}

Vertex * Graph::VertIterator::vertPtr()
{
  return _vertPtr;
}

vector<Vertex*>::iterator & Graph::VertIterator::vertIt()
{
  return _vertIt;
}

const vector<Vertex*>::iterator & Graph::VertIterator::vertsEnd()
{
  return _vertsEnd;
}

Graph * Graph::VertIterator::graph()
{
  return _graph;
}



Vertex* Graph::VertIterator::operator*() const
{
  if (_isEnd)  return 0;
  else return (_vertPtr);
}


/*
 * TODO this function is wrong. It will work to check whether the iterator is
 * equal to "end()", but not to actually compare iterators, because a BFS and
 * DFS iterator can be at the same place and not be headed to the same place
 * next or have come from the same place
 */
bool Graph::VertIterator::operator==(const Graph::VertIterator & other) const
{
  if (_isEnd && other.isEnd())  return true;
  else if (_isEnd || other.isEnd())  return false;
  else return _vertPtr == (*other);
}

bool Graph::VertIterator::operator!=(const Graph::VertIterator &other) const
{
  // Why doesn't this work?
  //return ! (this == other);
  return ! this->operator==(other);

 /* if (_isEnd && other.isEnd())  return false;
  else if (_isEnd || other.isEnd())  return true;
  else return _nodePtr != (*other);*/
}

Graph::DFSIterator::DFSIterator(Graph *graph, bool endFlag)
  : Graph::VertIterator::VertIterator(graph, endFlag)
{
  // This is set by default in NodeIterator, but just to be extra safe...
  setVertPtr(*(vertIt()));
}

Graph::VertIterator& Graph::DFSIterator::operator++()
{
  vertPtr()->mark();
  Vertex::EdgeIterator edgeIt = vertPtr()->begin();

  Vertex *v;

  while (edgeIt != vertPtr()->end())
  {
    v = graph()->opposite(vertPtr(), (*edgeIt));

    if (! v->marked())   _vertStack.push(v);
    ++edgeIt;
  }

  while (! _vertStack.empty() && _vertStack.top()->marked())
    _vertStack.pop();

  if (_vertStack.empty())
  {
    if (vertIt() == vertsEnd())
      setEnd();

    else
    {

      //CAREFUL! Think about whether nodeIt should always be ahead of nodePtr or not.
      do  vertIt()++;
      while (vertIt() != vertsEnd() && (*(vertIt()))->marked());

      if (vertIt() == vertsEnd())
      {
        setVertPtr(0);
        setEnd();
      }

      else  setVertPtr(*(vertIt()));
    }
  }

  else
  {
    setVertPtr(_vertStack.top());
    _vertStack.pop();
  }

  return (*this);
}

Graph::BFSIterator::BFSIterator(Graph *graph, bool endFlag)
  : Graph::VertIterator::VertIterator(graph, endFlag)
{
  setVertPtr(*(vertIt()));
}

Graph::VertIterator& Graph::BFSIterator::operator++()
{
  vertPtr()->mark();
  Vertex::EdgeIterator edgeIt = vertPtr()->begin();

  Vertex *v;

  while (edgeIt != vertPtr()->end())
  {
    v = graph()->opposite(vertPtr(), (*edgeIt));

    if (! v->marked())   _vertQueue.push(v);
    ++edgeIt;
  }

  while (! _vertQueue.empty() && _vertQueue.front()->marked())
    _vertQueue.pop();

  if (_vertQueue.empty())
  {
    if (vertIt() == vertsEnd())
      setEnd();

    else
    {

      do  vertIt()++;
      while (vertIt() != vertsEnd() && (*(vertIt()))->marked());

      if (vertIt() == vertsEnd())
      {
        setVertPtr(0);
        setEnd();
      }

      else  setVertPtr(*(vertIt()));
    }
  }

  else
  {
    setVertPtr(_vertQueue.front());
    _vertQueue.pop();
  }

  return (*this);
}

Graph::PathIterator::PathIterator(Graph *g, Vertex *start, Vertex* end, bool endFlag)
  : Graph::VertIterator::VertIterator(g, endFlag)
{
  
  _graph = g;
  
  if (! start || ! end || g->pathLength(start, end) == numeric_limits<double>::max())  setEnd();
  
  if (! isEnd())
  {
    if (! _graph->_FloydWarshallUpdated)  _graph->updateFloydWarshall();
    
    reconstructPath(start, end);
    _pathIt = _path.begin();
    setVertPtr(*_pathIt);
  }
}

Graph::VertIterator & Graph::PathIterator::operator++()
{
  if (! isEnd())
  {
    // if graph has been modified, iterating path is no longer possible
    if (! _graph->_FloydWarshallUpdated) 
    {
      setVertPtr(0); 
      setEnd();
    }
    
    else
    {
      ++_pathIt;
      if (_pathIt == _path.end())
      {
        setVertPtr(0);
        setEnd();
      }
      else  setVertPtr(*_pathIt);
    }
  }
  
  return (*this);
}

void Graph::PathIterator::reconstructPath(Vertex *u, Vertex *v)
{
  int wIdx = _graph->_nextFW.at(u->index() * _graph->vertexCount() + v->index());
  
  // No intermediate, u,v adjacent
  if (wIdx < 0)  return;
  
  else
  {
    Vertex *w = _graph->vertex(wIdx);
    reconstructPath(u, w);
    _path.push_back(w);

    reconstructPath(w, v);
  }  
}

ostream &operator<<(ostream &os, const Graph &g)
{
  os << "Vertices:" << endl;
  vector<Vertex *>::iterator vit;
  for (vit = g._vertices->begin(); vit != g._vertices->end(); ++vit)  os << (**vit) << endl;

  os << "Edges:" << endl;
  vector<Edge *>::iterator eit;
  for (eit = g._edges->begin(); eit != g._edges->end(); ++eit)  os << (**eit) << endl;

  return os;
}

