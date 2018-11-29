/*
 * TightSpanWalker.h
 *
 *  Created on: Sep 14, 2012
 *      Author: jleigh
 */

#ifndef TIGHTSPANWALKER_H_
#define TIGHTSPANWALKER_H_

#include "Edge.h"
#include "Graph.h"
#include "HapNet.h"
#include "Vertex.h"
#include "../seqio/Sequence.h"

#include <limits>
#include <list>
#include <queue>
#include <string>
#include <utility>
#include <vector>

class TightSpanWalker : public HapNet
{
public:
  TightSpanWalker(const std::vector <Sequence*> &, const std::vector<bool> &);
  virtual ~TightSpanWalker();

  // These three functions may not be needed
  //virtual size_t nseqs() const;
  //virtual const std::string & seqName(unsigned, bool=false) const;
  //virtual const std::string & seqSeq(unsigned, bool=false) const;

protected:
  virtual void setDistance(unsigned, unsigned, unsigned);
  virtual unsigned distance(unsigned, unsigned) const;
  


private:

  typedef struct {
    Vertex *first;
    Vertex *second;
    float dT;
  } VertPair;
  
  class VPComparitor
  {
  public:
    VPComparitor(const bool &reverse = false) : _reverse(reverse) {};
    VPComparitor(const VPComparitor &other) : _reverse(other._reverse) {};
    
    bool operator() (const VertPair &lhs, const VertPair &rhs) const  { return (_reverse ? (lhs.dT > rhs.dT) : (lhs.dT < rhs.dT)) ; } ; 
  private:
    bool _reverse;
  };
  
  typedef std::priority_queue<VertPair, std::vector<VertPair>, VPComparitor> VPPQ;
  
  virtual void computeGraph();
  void computeDT();
  static bool aboutEqual(float, float);
  float dT(unsigned, unsigned) const;
  void setDT(float, unsigned, unsigned);
  
  void writeExceptionGraph();

  // Make this use instance variables instead of pointers?
  //void findFandG(unsigned *, unsigned *, const forward_list<Vertex *> *, const forward_list<Vertex *>*, const vector<Vertex *> *, unsigned) const;
  std::pair<Vertex *, Vertex *> findFandG(unsigned, unsigned, int, int, const vector<Vertex *> &);
  void fixPath(Vertex *, Vertex *, const list<Vertex *>&, const vector<Vertex *>&);
  unsigned geodesic(Vertex *, Vertex *);//, int, int, Vertex * = 0);

  Graph *_K;
  std::vector<std::vector<float > > _dT;
  unsigned _nsamples;
  unsigned _ninternalVerts;
  /*vector<int> _components;*/
  std::list<std::list<Vertex *> > _components;
  std::vector<int> _componentIDs;
  
  std::map<vector<float> , const Vertex *> _vertexMap;
  
};

#endif /* TIGHTSPANWALKER_H_ */
