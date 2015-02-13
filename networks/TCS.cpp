#include "TCS.h"
#include "NetworkError.h"

#include <iostream>
#include <iomanip>
using namespace std;

TCS::TCS(const vector<Sequence *> &seqvect, const vector<bool> &mask)
 : HapNet(seqvect, mask)
{
  //setupGraph();
}

TCS::~TCS() { }

void TCS::computeGraph()
{
  VCPtrComparitor revcomp(true);
  VCPQ pairsByDist(revcomp);
  VertContainer *vcptr;
  vector<pair<const Vertex *, const Vertex*> > otherPairs;
  map<unsigned int, VertContainer*> dist2pairs;
  map<unsigned, VertContainer*>::iterator mapIt;

  int tmpcount = 0;

  
  for (unsigned i = 0; i < nseqs(); i++)
  {
    newVertex(seqName(i), &seqSeq(i));
    _componentIDs.push_back(i);
    
    for (unsigned j = 0; j < i; j++)
    {
      mapIt = dist2pairs.find(distance(i,j));
      
      if (mapIt == dist2pairs.end())
      {
        vcptr = new VertContainer(distance(i, j));
        vcptr->addPair(vertex(j), vertex(i));
        pairsByDist.push(vcptr);
        dist2pairs[vcptr->distance()] = vcptr;
        tmpcount++;
        cout << "tmpcount: " << tmpcount << endl;
        tmpcount = 0;
      }
      
      else
      {
        vcptr = mapIt->second;
        vcptr->addPair(vertex(j), vertex(i));
        tmpcount++;
      }
    }
  }

  cout << "tmpcount: " << tmpcount << endl;
  
  unsigned npairs = nseqs() * (nseqs() - 1) / 2;
  cout << "npairs: " << npairs << " pairsByDist size: " << pairsByDist.size()  << endl;
  unsigned paircount = 0;
  /*vector<VertContainer*> temp;
  while (! pairsByDist.empty())
  {
    vcptr = pairsByDist.top();
    pairsByDist.pop();
    
    temp.push_back(vcptr);
  }
  
  for (unsigned i = 0; i < temp.size(); i++)
    pairsByDist.push(temp.at(i));
  
  temp.clear();*/
  
  while (! pairsByDist.empty())
  {
    int compA = -1, compB = -1;

    vcptr = pairsByDist.top();
    unsigned M = vcptr->distance();

    pairsByDist.pop();
    
    VertContainer::Iterator pairIt = vcptr->begin();
    
    tmpcount = 0;
    for (; pairIt != vcptr->end(); ++pairIt)
    {
      tmpcount++;
      const Vertex *u = (*pairIt)[0];
      const Vertex *v = (*pairIt)[1];
      int compU = _componentIDs.at(u->index());
      int compV = _componentIDs.at(v->index());
      
      if (compU == compV)  continue;
      
      if (compU > compV)
      {
        swap(compU, compV);
        swap(u, v);
      }
          
      if (compA < 0)
      {
        compA = compU;
        compB = compV;
      }
      
      if (compU == compA && compV == compB)
      {
        if (M == 1)
          newEdge(vertex(u->index()), vertex(v->index()), 1);
        else
        {
          pair<Vertex *, Vertex *> intermediates;
          unsigned newPathLength = findIntermediates(intermediates, u, v, M);
          
          
          // test whether these are already connected by the appropriate length path
          //unsigned existingPath = pathLength(intermediates.first, intermediates.second);
          double existingPath = pathLength(intermediates.first, intermediates.second);
          bool pathExists = (existingPath < numeric_limits<double>::max());
          
          
          
          if (pathExists && existingPath < newPathLength)
          {
            throw NetworkError("Shorter path already exists between these vertices!");
          }
          
          else if (! pathExists || existingPath > newPathLength)
            newCompositePath(intermediates.first, intermediates.second, newPathLength);

        }
      }
      
      else
      {
        otherPairs.push_back(pair<const Vertex *, const Vertex *>(u, v));
      }
      paircount++;
    } // end for pairIt...
    cout << "paircount: " << paircount << " tmpcount: " << tmpcount << endl;

    // if compA and compB haven't been set, no distinct components were found at distance M
    if (compA >= 0)
    {
      for (unsigned i = 0 ; i < _componentIDs.size(); i++)
      {
        if (_componentIDs.at(i) < 0 || _componentIDs.at(i) == compB)
          _componentIDs.at(i) = compA;
        else if (_componentIDs.at(i) > compB)
          _componentIDs.at(i)--;
      }
    }
    
    if (otherPairs.empty())
    {
      //pairsByDist.pop();
      delete vcptr;
    }
    
    else
    {
      delete vcptr;
      vcptr = new VertContainer(M);
      
      for (unsigned i = 0; i < otherPairs.size(); i++)
        vcptr->addPair(otherPairs.at(i).first, otherPairs.at(i).second);
      
      otherPairs.clear();
      pairsByDist.push(vcptr);
    }

    updateProgress(100.0 * paircount / npairs + 0.5);
  }  

  unsigned vertidx = nseqs();

  while (vertidx < vertexCount())
  {
    Vertex *v = vertex(vertidx);
    if (v->degree() > 2)
      vertidx++;

    else
    {
      if (v->degree() != 2)  throw NetworkError("Intermediate vertex has degree less than 2.");
      vector<const Edge *> oldEdges;

      Vertex::EdgeIterator eit = v->begin();

      while (eit != v->end())
      {
        oldEdges.push_back(*eit);//opposite(v, *eit));
        ++eit;
      }

      if (oldEdges.size() != 2)  throw NetworkError("Vertex with degree 2 does not have 2 neighbours.");

      Vertex *u = opposite(v, oldEdges.at(0));
      Vertex *w = opposite(v, oldEdges.at(1));

      if (u == w || v == u || v == w)
        throw NetworkError("Unexpected multiple edges or self edge.");

      double newweight = oldEdges.at(0)->weight() + oldEdges.at(1)->weight();
      removeVertex(v->index());
      newEdge(u, w, newweight);
    }
  }
}

unsigned TCS::findIntermediates(pair<Vertex *, Vertex *> & intPair, const Vertex *u, const Vertex *v, unsigned dist)
{
  int compU = _componentIDs.at(u->index());
  int compV = _componentIDs.at(v->index());
  
  if (compU == compV)  throw NetworkError("Attempting to find intermediates within a component.");
  
  int maxScore = numeric_limits<int>::min() + 1;
  bool pathExists;
  unsigned minPathLength = dist;//numeric_limits<unsigned>::max();
  intPair.first = vertex(u->index());
  intPair.second = vertex(v->index());
  
  for (unsigned i = 0; i < _componentIDs.size(); i++)
  {
    // the negativity test allows "no man's land" vertices to be reused
    if (_componentIDs.at(i) != compU && _componentIDs.at(i) >= 0)
      continue;
    
    pathExists = (pathLength(u, vertex(i)) < numeric_limits<double>::max());
    if (! pathExists)
      continue;
    
    unsigned pathUI = pathLength(u, vertex(i));
    if (pathUI >= dist) // -1 ?
      continue;
      
    for (unsigned j = 0; j < _componentIDs.size(); j++)
    {
      if (_componentIDs.at(j) != compV && _componentIDs.at(j) >= 0)  continue;
      
      pathExists = (pathLength(v, vertex(j)) < numeric_limits<double>::max());
      if (! pathExists)
        continue;

      unsigned pathVJ = pathLength(v, vertex(j));
      if (pathVJ + pathUI >= dist) // - 1?
        continue;

      unsigned dP = dist - pathVJ - pathUI;
      int score = computeScore(vertex(i), vertex(j), compU, compV, dP, dist);
      
      if (score > maxScore || 
          (score == maxScore && dP < minPathLength))
      {
        minPathLength = dP;
        maxScore = score;
        intPair.first = vertex(i);
        intPair.second = vertex(j);
      }
    }
  }
  
  return minPathLength;
}

int TCS::computeScore(const Vertex *u, const Vertex *v, int compU, int compV, unsigned dP, unsigned clustDist)
{
  
 int score = 0;
  for (unsigned i = 0; i < nseqs(); i++)
  {
    if (_componentIDs.at(i) != compU)  continue;
    
    for (unsigned j = 0; j < nseqs(); j++)
    {
      if (_componentIDs.at(j) != compV)  continue;
      unsigned totalPath = dP + pathLength(u, vertex(i)) + pathLength(v, vertex(j));
      
      if (totalPath == distance(i, j))
        score += BONUS;
      else if (totalPath > distance(i, j))
        score -= LONGPENALTY;
      
      else
      {
        if (totalPath < clustDist)
          return numeric_limits<int>::min();
        
        else
          score -= SHORTCUTPENALTY;
        
      }
    }
  }
  
  return score;
}


void TCS::newCompositePath(Vertex *start, Vertex *end, unsigned dist)
{
  Vertex *u = start, *v;
  
  for (unsigned i = 1; i < dist; i++)
  {
    v = newVertex("");
    _componentIDs.push_back(-1);
    newEdge(u, v, 1);
    u = v;
  }
  
  newEdge(u, end, 1);
}
