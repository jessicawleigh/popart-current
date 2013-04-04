#ifndef ANCESTRALSEQNET_H_
#define ANCESTRALSEQNET_H_

#include <list>
#include <string>
#include <utility>
#include <vector>
#include "AbstractMSN.h"

#include "../seqio/Sequence.h"



class AncestralSeqNet : public HapNet
{
  
public:
  AncestralSeqNet(const std::vector<Sequence*> &, const std::vector<bool> & = std::vector<bool>(), unsigned = 0, double = 0.05);
  virtual ~AncestralSeqNet() {};
  
protected:
  virtual void setDistance(unsigned, unsigned, unsigned);
  virtual unsigned distance(unsigned, unsigned) const;
  //void setupAncestors();
  void setProgress(double prog) { _progress = prog; };
  double progress() const { return _progress; };
  virtual unsigned niter() const = 0;
  //virtual int msnProgress() { return progress(); };

private:
  
  virtual void computeGraph();
  //virtual void computeDistances();
  
  //virtual void computeAncestralSeqs(double alpha) = 0;
  virtual const std::list<std::pair<const std::string, const std::string> > sampleEdge() = 0;
  virtual unsigned ancestorCount() const = 0;
  virtual const std::string & ancestralSeq(unsigned index) const = 0;
  
  bool _seqsComputed;
  double _alpha;  
  double _progress;
  double _epsilon;
  std::vector<std::vector<unsigned> > _distances;
  
};


#endif

