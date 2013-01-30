#ifndef MEDJOINNET_H_
#define MEDJOINNET_H_

#include <map>
#include <queue>
#include <vector>
#include <set>
#include <string>
#include "HapNet.h"
#include "../seqio/Sequence.h"


class MedJoinNet : public HapNet {
public:
  MedJoinNet(const std::vector<Sequence *> &, const std::vector<bool> &, unsigned = 0);
  virtual ~MedJoinNet();
  
  virtual size_t nseqs() const;
  virtual const std::string & seqName(unsigned, bool=false) const;
  virtual const std::string & seqSeq(unsigned, bool=false) const;

protected:
  virtual void setDistance(unsigned, unsigned, unsigned);
  virtual unsigned distance(unsigned, unsigned) const;


private:
  virtual void computeGraph();
  void computeMJN();
  void computeMSN(std::map<unsigned,Edge*> *);
  std::set<std::string> computeQuasiMedianSeqs(const std::string &, const std::string &, const std::string &) const;
  unsigned computeCost(const std::string &, const std::string &, const std::string &, const std::string &) const;
  virtual void computeDistances();
  bool removeObsoleteVerts();//std::map<unsigned,Edge*> &);
  
  bool areConnected(Vertex *, Vertex *, unsigned, bool = true);

  const unsigned _nsamples;
  unsigned _medianSeqCount;
  vector<std::string> _medianSeqs;
  vector<std::string> _medianNames;
  unsigned _epsilon;
  unsigned *_distances;
  std::set<std::string>  _allSeqSet;
    
};


#endif
