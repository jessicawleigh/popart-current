/*
 * Edge.h
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#ifndef EDGE_H_
#define EDGE_H_

#include <iostream>

#include "Vertex.h"

class Vertex;

class Edge {
public:
  typedef enum {Black, Red, Green} Colour;
  
  Edge(const Vertex *, const Vertex *, unsigned int, double = 1, const void * = NULL, Colour = Black);
  virtual ~Edge();

  const Vertex * from() const { return _from; };
  void setFrom(const Vertex *v) { _from = v; };
  const Vertex * to() const { return _to; };
  void setTo(const Vertex *v) { _to = v; };
  
  const void * info() const { return _info; };
  void setInfo(void *inf) {_info = inf; };
  
  unsigned int index() const { return _index; };

  double weight() const { return _weight; };
  void setWeight(double w) { _weight = w; };
  
  bool marked() const { return _marked; };
  void mark() { _marked = true; };
  void unmark() { _marked = false; };
  
  Colour colour() const { return _colour; };
  void setColour(Colour col) { _colour = col; };


  friend std::ostream &operator<<(std::ostream &, const Edge &);
  friend void setIndex(unsigned, Edge &);
  
protected:
  void setIndex(unsigned idx) { _index = idx; };

private:

  const Vertex * _from;
  const Vertex * _to;
  unsigned int _index;
  const void * _info;
  double _weight;
  bool _marked;
  Colour _colour;

};

#endif /* EDGE_H_ */
