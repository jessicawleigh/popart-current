/*
 * Graph.h
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

/*
 * TODO
 * Store edges/vertices as sets instead of vectors?
 *   Note that current (vector) system makes it slow to delete nodes/edges, but faster to access BY INDEX, not in any other way.
 * Add standard graph algorithm stuff: Dijkstra, etc.
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <limits>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <stack>
#include <queue>

#include "Edge.h"
#include "Vertex.h"


class Graph {
public:
  Graph();
  virtual ~Graph();

  class VertIterator
  {
  public:
    VertIterator(Graph*, bool = false);
    virtual VertIterator & operator++() = 0;
    Vertex * operator*() const;
    virtual bool operator==(const VertIterator &) const;
    virtual bool operator!=(const VertIterator &) const;
  protected:
    bool isEnd() const;

    void setEnd();
    void setVertPtr(Vertex*);
    Vertex* vertPtr();
    std::vector<Vertex*>::iterator & vertIt();
    const std::vector<Vertex*>::iterator & vertsEnd();
    Graph *graph();

  private:
    std::vector<Vertex*>::iterator _vertIt;
    //const
    std::vector<Vertex*>::iterator _vertsEnd;
    std::vector<Vertex*> * _verts;
    bool _isEnd;
    Graph *_graph;
    Vertex *_vertPtr;
    
  };

  class DFSIterator : public VertIterator
  {
  public:
    DFSIterator(Graph*, bool = false);
    virtual VertIterator & operator++();
  private:
    std::stack<Vertex*> _vertStack;
  };

  class BFSIterator : public VertIterator
  {
  public:
    BFSIterator(Graph*, bool = false);
    virtual VertIterator & operator++();
  private:
    std::queue<Vertex*> _vertQueue;
  };
  
  class PathIterator : public VertIterator
  {
  public:
    PathIterator(Graph *, Vertex *, Vertex*, bool = false);
    virtual VertIterator & operator++();
  private:
    void reconstructPath(Vertex *, Vertex *);
    
    std::list<Vertex *> _path;
    std::list<Vertex *>::const_iterator _pathIt;
    Graph *_graph;
  };


  Edge * newEdge(Vertex *, Vertex *, double = 1, const void * = NULL);
  Vertex * newVertex(const std::string &, const void * = NULL);

  Edge * edge(unsigned);
  const Edge * edge(unsigned) const;
  Vertex * vertex(unsigned);
  const Vertex * vertex(unsigned) const;
  
  Vertex * opposite(const Vertex *, const Edge *);
  const Vertex * opposite(const Vertex *, const Edge *) const;

  unsigned edgeCount() const;
  unsigned vertexCount() const;
  
  void moveEdge(unsigned, Vertex *, Vertex *);

  void removeEdge(unsigned);
  void removeVertex(unsigned);
  
  void unmarkEdges();
  void unmarkVertices();
  
  double pathLength(const Vertex*, const Vertex*);

  DFSIterator beginDFS();
  DFSIterator endDFS();
  BFSIterator beginBFS();
  BFSIterator endBFS();
  
  PathIterator beginPath(Vertex *, Vertex *);
  PathIterator endPath();

  friend std::ostream &operator<<(std::ostream &, const Graph &);

protected:
private:
  
  void updateFloydWarshall();
  
  std::vector<Edge*> *_edges;
  std::vector<Vertex*> *_vertices;
  std::vector<double> _pathLengths;
  std::vector<int> _nextFW;
  bool _FloydWarshallUpdated;
};

#endif /* GRAPH_H_ */
