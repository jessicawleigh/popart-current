#ifndef CONCRETEHAPNET_H_
#define CONCRETEHAPNET_H_

#include "HapNet.h"

class ConcreteHapNet : public HapNet
{
public:
  ConcreteHapNet(const Graph &, const std::vector <Sequence*> &, const std::vector<bool> & = std::vector<bool>());
  
private:
   virtual void computeGraph();
  
};


#endif