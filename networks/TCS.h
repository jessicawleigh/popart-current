#ifndef TCS_H_
#define TCS_H_

#include "HapNet.h"
#include "../seqio/Sequence.h"

#include <vector>

class TCS : public HapNet
{
public:
  TCS(const std::vector <Sequence *> &, const std::vector<bool> &);
  virtual ~TCS();
  
  virtual void computeGraph();
  
private:
  
  unsigned findIntermediates(std::pair<Vertex *, Vertex *>&, const Vertex *, const Vertex *, unsigned);
  int computeScore(const Vertex *, const Vertex *, int, int, unsigned, unsigned);
  void newCompositePath(Vertex *, Vertex *, unsigned);
  
  
  static const int BONUS = 20;
  static const int SHORTCUTPENALTY = 10;
  static const int LONGPENALTY = 5;
  
  std::vector<int> _componentIDs;
};


#endif
