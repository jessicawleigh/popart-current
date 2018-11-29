#ifndef INTNJ_H_
#define INTNJ_H_

#include "HapNet.h"
#include "Vertex.h"
#include "Edge.h"

#include <queue>
#include <string>
#include <vector>

class IntNJ : public HapNet
{
public:
  //const static int EPSILON = 1;
  IntNJ(const std::vector<Sequence *> &, const std::vector<bool> &, double = 0, int = 1);
  virtual ~IntNJ();
  
  virtual size_t nseqs() const;
  virtual const std::string & seqName(unsigned, bool=false) const;
  virtual const std::string & seqSeq(unsigned, bool=false) const;
  void setUseRetTol(bool useRetTol) { _useRetTol = useRetTol; };
  bool useRetTol() const { return _useRetTol; };
  
protected:
  virtual void computeGraph();
  void newCompositeEdge(Vertex *, Vertex *, unsigned);
  
private:
  typedef std::priority_queue<VertContainer*, std::vector<VertContainer*>, VCPtrComparitor> VCPQ;
  typedef struct { unsigned from; unsigned to; double weight; } EdgeDescriptor;

  void integerNJ();
  void NJReduce(std::vector<double> &, unsigned, unsigned, unsigned, double *, double *);
  double newDist(std::vector<double> &, unsigned, unsigned, unsigned);
  vector<unsigned> traverseSplits(const Vertex *, const Edge *, double**) const;
  void optimiseEdges();
  
  bool isLegal(const Vertex *, const Edge *, unsigned, const Vertex *, const Edge *, unsigned, unsigned);
  
  const unsigned _leafCount;
  double _alpha;
  const int _epsilon;
  unsigned *_distances;
  unsigned _internalVertCount;
  bool _useRetTol;
};


#endif
