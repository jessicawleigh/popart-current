#ifndef ABSTRACTMSN_H_
#define ABSTRACTMSN_H_


#include "HapNet.h"
#include "../seqio/Sequence.h"

#include <vector>


class AbstractMSN : public HapNet
{
public:
  AbstractMSN(const std::vector<Sequence*> &, const std::vector<bool> & = std::vector<bool>(), unsigned = 0);
  virtual ~AbstractMSN() {}; 
  
protected:
  void computeMSN();
  virtual int msnProgress();
  int compCount();
  
private: 
  unsigned _epsilon;
  bool _strict;
  int _ncomps;

};

#endif
