/*
 * Vertex.h
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#ifndef VERTEX_H_
#define VERTEX_H_

#include <iostream>
#include <list>
#include <string>

#include "Edge.h"

class Edge;

class Vertex {
public:
  typedef enum {Black, Red, Green} Colour;
  
  Vertex(const std::string &, int, const void *, Colour = Black);
  virtual ~Vertex();
  
  class EdgeIterator
  {
  public:
    EdgeIterator(const Vertex*, bool = false, bool = false);
    EdgeIterator & operator++();
    //EdgeIterator operator++(int);
    const Edge * operator*() const;
    bool operator==(const EdgeIterator &) const;
    bool operator!=(const EdgeIterator &) const;
  protected:
    bool isEnd() const { return _isEnd; };
    bool isReverse() const { return _isReverse; };
  private:
    std::list<const Edge*>::const_iterator _edgeIt;
    std::list<const Edge*>::const_reverse_iterator _rEdgeIt;
    const std::list<const Edge*> * _edges;
    bool _isEnd;
    bool _isReverse;
  };
  
  const std::string & label() const { return _label; };
  
  const void * info() const { return _info; };
  void setInfo(void *inf) {_info = inf; };
  
  unsigned int index() const { return _index; };
  
  unsigned int degree() const { return _incidences.size(); };
  
  bool marked() const { return _marked; };
  void mark() { _marked = true; };
  void unmark() { _marked = false; };
  
  Colour colour() const { return _colour; };
  void setColour(Colour col) { _colour = col; };
  
  EdgeIterator begin() const;
  EdgeIterator end() const;
  EdgeIterator rbegin() const;
  EdgeIterator rend() const;
  void addIncidentEdge(const Edge*);
  void removeIncidentEdge(const Edge*);
  bool isAdjacent(const Vertex*) const;
  const Edge * sharedEdge(const Vertex *) const;

  bool operator==(const Vertex &) const;
  bool operator!=(const Vertex &) const;

  friend std::ostream &operator<<(std::ostream &, const Vertex &);
  friend void setIndex(unsigned, Vertex &);

protected:
  void setIndex(unsigned idx) { _index = idx; };

private:

  const std::string _label;
  unsigned int _index;
  const void *_info;
  std::list<const Edge *> _incidences;
  bool _marked;
  Colour _colour;
};

#endif /* VERTEX_H_ */
