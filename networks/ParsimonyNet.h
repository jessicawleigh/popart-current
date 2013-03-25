#ifndef PARSIMONYNET_H_
#define PARSIMONYNET_H_

#include "AncestralSeqNet.h"
#include "Vertex.h"
#include "Edge.h"
#include "../seqio/Sequence.h"
#include "../tree/ParsimonyTree.h"

#include <list>
#include <string>
#include <utility>
#include <vector>


class ParsimonyNet : public AncestralSeqNet
{
public:
  ParsimonyNet(const std::vector <Sequence*> &, const std::vector<bool> &, const std::vector <ParsimonyTree *> &, unsigned = 0, double = 0.95);
  virtual ~ParsimonyNet();
  
  //void addTree(ParsimonyTree *);
  
private:
  //virtual void computeGraph();
  virtual void computeAncestralSeqs(double = 0.05);
  virtual const std::list<std::pair<const std::string, const std::string> > sampleEdge();
  virtual unsigned ancestorCount() const { return _ancestors.size(); };
  virtual const std::string & ancestralSeq(unsigned) const;
  

  unsigned _nOrigSeqs;
  const std::vector<ParsimonyTree *> _trees;
  std::vector<string> _ancestors;
  //const std::vector<Sequence *> * _seqVectPtr;
  vector<bool> _treeUsed;
};

#endif
