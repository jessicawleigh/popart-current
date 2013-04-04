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


  for (unsigned i = 0; i < _nOrigSeqs; i++)
  {
    // use original sequence indices so leaf count matches seq count
    const Sequence *s = new Sequence(seqName(i, true), condensedSeqSeq(i));
    _seqvect.push_back(s);
  }

  _treeUsed.resize(_trees.size(), false);

  _niter = 100 * _trees.size();
}

ParsimonyNet::~ParsimonyNet()
{ }

const list<pair<const string, const string> > ParsimonyNet::sampleEdge()
{

  unsigned treeIdx = rand() % _trees.size();
  ParsimonyTree *t = _trees.at(treeIdx);

  if (! _treeUsed.at(treeIdx))

  {
    _treeUsed.at(treeIdx) = true;
    t->setLeafSequences(_seqvect, weights());
    t->computeScore();
  }
  // Compute new ancestors, even if we've already used this tree
  t->computeAncestors();


  ParsimonyTree::EdgeIterator edgeIt = t->edgeBegin();

  list<pair<const string, const string> > edgelist;


  while (edgeIt != t->edgeEnd())
  {
    const string firstSeq = edgeIt.first->sequence()->seq();
    const string secondSeq = edgeIt.second->sequence()->seq();

    edgelist.push_back(pair<const string, const string>(firstSeq, secondSeq));
    ++edgeIt;
  }

  return edgelist;
}

const string & ParsimonyNet::ancestralSeq(unsigned index) const
{
  if (index > ancestorCount())  
     NetworkError("Index of ancestral sequence out of range.");
  
  return _ancestors.at(index);
}

