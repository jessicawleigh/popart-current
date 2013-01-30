#include "ParsimonyNet.h"
#include "NetworkError.h"
#include "../tree/ParsimonyNode.h"
#include "../tree/Tree.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <string>
using namespace std;


ParsimonyNet::ParsimonyNet(const vector <Sequence *> &seqvect, const vector<bool> & mask, const vector<ParsimonyTree *> &treevect, unsigned epsilon, double alpha)
  : AncestralSeqNet(seqvect, mask, epsilon, alpha), _trees(treevect)
{
  _nOrigSeqs = seqvect.size();
  //setupAncestors();
  //setupGraph();
}

ParsimonyNet::~ParsimonyNet()
{ }

void ParsimonyNet::computeAncestralSeqs(double alpha)
{  
  
  vector<const Sequence *> seqvect;
  map<const string, unsigned> seqCount;
  set<string> leafSeqs;
  
  for (unsigned i = 0; i < _nOrigSeqs; i++) 
  {
    // use origintal sequence indices so leaf count matches seq count
    const Sequence *s = new Sequence(seqName(i, true), condensedSeqSeq(i));
    seqvect.push_back(s);  
    leafSeqs.insert(string(s->seq()));
  }
  
  vector<bool> treeUsed(_trees.size(), false);
  
  
  unsigned niter = _trees.size() * 100;
  
  for (unsigned i = 0; i < niter; i++)
  {
    setProgress(100.0 * (i + 1) / niter);
    unsigned treeIdx = rand() % _trees.size();
    ParsimonyTree *t = _trees.at(treeIdx);
    
    if (! treeUsed.at(treeIdx))
    {
      treeUsed.at(treeIdx) = true;
      t->setLeafSequences(seqvect, weights());
      t->computeScore();      
    }
    
    // Compute new ancestors, even if we've already used this tree
    t->computeAncestors();
    
    
    ParsimonyTree::EdgeIterator edgeIt = t->edgeBegin();
  
    while (edgeIt != t->edgeEnd())
    {
      const string &firstSeq = edgeIt.first->sequence()->seq();
      const string &secondSeq = edgeIt.second->sequence()->seq();

      set<string>::iterator leafIt = leafSeqs.find(firstSeq);
      map<string, unsigned> ::iterator seqCountIt;

      if (leafIt == leafSeqs.end())
      {
        seqCountIt = seqCount.find(firstSeq);

        if (seqCountIt == seqCount.end())
          seqCount[firstSeq] = 1;
      
        else
          seqCount[firstSeq] ++;
      }
      
      leafIt = leafSeqs.find(secondSeq);
      
      if (leafIt == leafSeqs.end())
      {
        if (firstSeq != secondSeq)
        {
          seqCountIt = seqCount.find(secondSeq);
          if (seqCountIt == seqCount.end())
            seqCount[secondSeq] = 1;
      
        else
          seqCount[secondSeq] ++;
        }
      }
      
      ++edgeIt;
      updateProgress(msnProgress());
    } //end while edgeIt...
  } // end for unsigned i...
  
  
  double cutoff = alpha * niter;//0.05 * _trees.size() * 10;
  
  map<string, unsigned>::iterator seqIt = seqCount.begin();
  while (seqIt != seqCount.end())
  {
    if (seqIt->second >= cutoff)
      _ancestors.push_back(seqIt->first);
    ++seqIt;
  }


  for (unsigned i = 0; i < seqvect.size(); i++)
    delete seqvect.at(i);
}

const string & ParsimonyNet::ancestralSeq(unsigned index) const
{
  if (index > ancestorCount())  
     NetworkError("Index of ancestral sequence out of range.");
  
  return _ancestors.at(index);
}

