#include "AbstractMSN.h"
#include "NetworkError.h"

#include <limits>
#include <map>
#include <iostream>
using namespace std;

AbstractMSN::AbstractMSN(const std::vector<Sequence*> &seqs, const vector<bool> & mask, unsigned epsilon)
  : HapNet(seqs, mask)
{
  _epsilon = epsilon;
  if (_epsilon > 0) _strict = false;
  else _strict = true;
    
}


void AbstractMSN::computeMSN()
{

  unsigned *component = new unsigned[vertexCount()];
  _ncomps = vertexCount();//nseqs();
  VCPtrComparitor revcomp(true);
  VCPQ pairsByDist(revcomp);
  map<unsigned int, VertContainer*> dist2pairs;
  long maxValue = numeric_limits<long>::max();

  VertContainer *vcptr;

  for (int i = 0; i < _ncomps; i++)
  {
    component[i] = i;

    for (int j = 0; j < i; j++)
    {
      map<unsigned int, VertContainer*>::iterator mapIt = dist2pairs.find(distance(i,j));

      if (mapIt == dist2pairs.end())
      {
        vcptr = new VertContainer(distance(i,j));//, pair);
        vcptr->addPair(vertex(i), vertex(j));
        pairsByDist.push(vcptr);
        dist2pairs[vcptr->distance()] = vcptr;
      }

      else
      {
        vcptr = mapIt->second;
        vcptr->addPair(vertex(i), vertex(j));
      }
    }
  }



  while (! pairsByDist.empty())
  {
    vcptr = pairsByDist.top();
    pairsByDist.pop();
    unsigned threshold = vcptr->distance();

    if (threshold > maxValue)  break;


    VertContainer::Iterator pairIt = vcptr->begin();
    vector<const Vertex**> newpairs;


    while (pairIt != vcptr->end())//pairs.end())
    {
      const Vertex *u = (*pairIt)[0];
      const Vertex *v = (*pairIt)[1];

      // This 'if' would keep the MSN from being relaxed.
      if (! _strict || component[u->index()] != component[v->index()])
      {

      newpairs.push_back(*pairIt);

      Vertex *uv = vertex(u->index());
      Vertex *vv = vertex(v->index());

      newEdge(uv, vv, (double)(vcptr->distance()));

      }

      ++pairIt;
    }

    vector<const Vertex**>::const_iterator newPairIt = newpairs.begin();

    while (newPairIt != newpairs.end())
    {
      unsigned int compU = component[(*newPairIt)[0]->index()];
      unsigned int compV = component[(*newPairIt)[1]->index()];

      // components not already connected via another edge
      if (compU != compV)
      {
        if (compU < compV)
        {
          unsigned int tmp = compU;
          compU = compV;
          compV = tmp;
        }

        // compU > compV at this point
        for (unsigned i = 0; i < vertexCount(); i++)
        {
          if (component[i] == compU)
            component[i] = compV;
          else if (component[i] > compU)
            component[i]--;
        }
        _ncomps--;
        updateProgress(msnProgress());
      }

      if (_ncomps == 1 && maxValue == numeric_limits<long>::max())
        maxValue = threshold + _epsilon;
      
      newPairIt++;
    }

    delete vcptr;
  }

  if (_ncomps > 1)
  {
    throw NetworkError("pairsByDist is empty before graph is connected!");
  }
  
   // Ditch MSN and make a 7-node cycle: 
  /*for (unsigned i = edgeCount(); i > 0; i--)  removeEdge(i - 1);
  
  for (unsigned i = 1; i < nodeCount(); i++)  newEdge(node(i-1), node(i));
  
  newEdge(node(nodeCount() - 1), node(0));*/

}

int AbstractMSN::msnProgress()
{
  return 100.0 * compCount() / vertexCount() + 0.5;
}

int AbstractMSN::compCount()
{
  return _ncomps;
}




