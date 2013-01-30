#include "AncestralSeqNet.h"
#include "NetworkError.h"

#include <iostream>
using namespace std;


AncestralSeqNet::AncestralSeqNet(const vector <Sequence*> & seqvect, const vector<bool> & mask, unsigned epsilon, double alpha) 
 : AbstractMSN(seqvect, mask, epsilon), _seqsComputed(false), _alpha(alpha)
{}


/*void AncestralSeqNet::setupAncestors()
{
  computeAncestralSeqs(_alpha);
  _seqsComputed = true;
}*/

void AncestralSeqNet::computeDistances()
{
  if (! _seqsComputed)
  {
    computeAncestralSeqs(_alpha);
    _seqsComputed = true;

  }
  for (unsigned i = 0; i < (nseqs() + ancestorCount()); i++)
  {
    _distances.push_back(vector<unsigned>(nseqs() + ancestorCount(), 0));
    _distances.at(i).resize(nseqs() + ancestorCount(), 0);
  }
  
  for (unsigned i = 0; i < nseqs(); i++)
  {
    for (unsigned j = i + 1; j  < nseqs(); j++)
    {
      unsigned dij = pairwiseDistance(seqSeq(i), seqSeq(j));
       _distances.at(j).at(i) = dij;
       _distances.at(i).at(j) = dij;
    }
    
    for (unsigned j = 0; j < ancestorCount(); j++)
    {
      unsigned dij = pairwiseDistance(seqSeq(i), ancestralSeq(j));

      if (dij == 0)  cout << "i: " << i << " j: " << j << endl << seqSeq(i) << endl << ancestralSeq(j) << '\n' << endl;
      _distances.at(j + nseqs()).at(i) = dij;
      _distances.at(i).at(j + nseqs()) = dij;
    }    
  }
  
  for (unsigned i = 0; i < ancestorCount(); i++)
  {
    for (unsigned j = i + 1; j < ancestorCount(); j++)
    {
      unsigned dij = pairwiseDistance(ancestralSeq(i), ancestralSeq(j));
      _distances.at(i + nseqs()).at(j + nseqs()) = dij;
      _distances.at(j + nseqs()).at(i + nseqs()) = dij;
    }
  }
}

void AncestralSeqNet::computeGraph()
{

  if (! _seqsComputed) 
    throw NetworkError("Can't compute graph until ancestors have been set up!");
  

  for (unsigned i = 0; i < nseqs(); i++)
  {
    newVertex(seqName(i), &seqSeq(i));
  }
  
  for (unsigned i = 0; i < ancestorCount(); i++)
  {
    newVertex("");
  }



  computeMSN();
}

void AncestralSeqNet::setDistance(unsigned dist, unsigned i, unsigned j)
{
  unsigned totalCount = nseqs() + ancestorCount();
  if (i >= totalCount || j >= totalCount)  
    throw NetworkError("Invalid index.");
                         
  _distances.at(i).at(j) = dist;
}

unsigned AncestralSeqNet::distance(unsigned i, unsigned j) const
{
  unsigned totalCount = nseqs() + ancestorCount();
  if (i >= totalCount || j >= totalCount)  
    throw NetworkError("Invalid index.");
                         
  return _distances.at(i).at(j);
}
