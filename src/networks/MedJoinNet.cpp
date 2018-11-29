#include <limits>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stack>
using namespace std;

#include "MedJoinNet.h"
#include "Vertex.h"
#include "Edge.h"
#include "NetworkError.h"

#include <QTime>

MedJoinNet::MedJoinNet(const std::vector<Sequence *> & seqs, const vector<bool> & mask, unsigned epsilon)
  : HapNet(seqs, mask), _nsamples(HapNet::nseqs())
{

  _medianSeqCount = 0;
  _epsilon = epsilon;
  _distances = 0;
    
  //setupGraph();
}

MedJoinNet::~MedJoinNet()
{
  if (_distances)  delete [] _distances;
}

size_t MedJoinNet::nseqs() const
{
  return HapNet::nseqs() + _medianSeqs.size();
}

const string & MedJoinNet::seqName(unsigned idx, bool isOrig) const
{
  if (isOrig || idx < _nsamples)
    return HapNet::seqName(idx, isOrig);
  
  else  if (idx >= nseqs())
    throw NetworkError("Index is greater than the number of vertices in graph!");
  
  else
    return vertex(idx)->label();

}

const string & MedJoinNet::seqSeq(unsigned idx, bool isOrig) const
{
  if (isOrig || idx < _nsamples)
    return HapNet::seqSeq(idx, isOrig);
  
  else  if (idx >= nseqs())
    throw NetworkError("Index is greater than the number of vertices in graph!");
  
  else
    return _medianSeqs.at(idx - _nsamples);
}



void MedJoinNet::computeGraph()
{
  
  for (unsigned i = 0; i < nseqs(); i++)
    newVertex(seqName(i), &seqSeq(i));
  computeMJN();
  

  bool changed;
  do {
    // remove all edges before computing final network
    for (unsigned i = edgeCount(); i > 0; i--)  removeEdge(i - 1);

    map<unsigned,Edge*> feasibleLinks;
    computeMSN(&feasibleLinks);


    map<unsigned,Edge*>::iterator flinkIter;
    vector<Edge*> notFeasible;
    for (unsigned i = 0; i < edgeCount(); i++)
    {
      flinkIter = feasibleLinks.find(i);
      if (flinkIter == feasibleLinks.end())  notFeasible.push_back(edge(i));
    }

    for (vector<Edge*>::iterator edgeIt = notFeasible.begin(); edgeIt != notFeasible.end(); ++edgeIt)
      removeEdge((*edgeIt)->index());


    changed = removeObsoleteVerts();

  }  while (changed);
  
  //cout << "time: " << _timer.elapsed() << endl;
  updateProgress(100);
}

void MedJoinNet::computeMJN()
{
  int oldLength = -1;
  bool changed;
  for (unsigned i = 0; i < nseqs(); i++)  _allSeqSet.insert(seqSeq(i));
  
  do  {
    // remove all edges before beginning
    for (unsigned i = edgeCount(); i > 0; i--)  removeEdge(i - 1);

    map<unsigned,Edge*> feasibleLinks;
    computeDistances();
    computeMSN(&feasibleLinks);
    
    int msnLength;
      
    map<unsigned,Edge*>::iterator flinkIter;
    vector<Edge*> notFeasible;
    for (unsigned i = 0; i < edgeCount(); i++)
    {
      flinkIter = feasibleLinks.find(i);
      if (flinkIter == feasibleLinks.end())  notFeasible.push_back(edge(i));
      else  msnLength += edge(i)->weight();
    }
    
    if (oldLength > 0)
      updateProgress(100. * pow((double)oldLength / msnLength, 2) + 0.5);
      //cout << "Progress: " << (100. * msnLength / oldLength);
    else 
      updateProgress(0);
      //cout << "Progress: " << 0;
    //cout << " time: " << _timer.elapsed() << endl;
    
    //cout << "msnLength: " << msnLength << " oldLength: " << oldLength << endl;
    oldLength = msnLength;
    
    for (vector<Edge*>::iterator edgeIt = notFeasible.begin(); edgeIt != notFeasible.end(); ++edgeIt)
    {
      removeEdge((*edgeIt)->index());  
    }      

    changed = removeObsoleteVerts();
    
    Vertex *u, *v, *w;
    double minCost = numeric_limits<double>::max();

    for (unsigned i = 0; i < vertexCount(); i++)
    {
      u = vertex(i);
      const string seqU = seqSeq(u->index());
      
      for (Vertex::EdgeIterator edgeIt = u->begin(); edgeIt != u->end(); ++edgeIt)
      {
        v = opposite(u, (*edgeIt));
        const string seqV = seqSeq(v->index());
        
        for (Vertex::EdgeIterator edgeIt2 = u->begin(); edgeIt2 != edgeIt; ++edgeIt2)
        {
          w = opposite(u, (*edgeIt2));
          const string seqW = seqSeq(w->index());
          
          set<string> uvwMedians = computeQuasiMedianSeqs(seqU, seqV, seqW);
          set<string>::iterator medIter = uvwMedians.begin();
          while (medIter != uvwMedians.end())
          {
            set<string>::iterator seqIter = _allSeqSet.find(*medIter);
            if (seqIter == _allSeqSet.end())
            {
              double cost = computeCost(seqU, seqV, seqW, *medIter);
              if (cost < minCost)  minCost = cost;
            }

            ++medIter;
          } // end while medIter != ...
        } // end for edgeIt2 = u->begin()...
      } // end for edgeIt = u->begin()...
    } // end for unsigned i = 0... (loop over nodes)
    
    flinkIter = feasibleLinks.begin();

    // TODO stupid, couldn't we just keep track of feasible triplets ranked by cost?
    while (flinkIter != feasibleLinks.end())
    {
      Edge *e = flinkIter->second;
      u = vertex(e->from()->index());
      const string seqU = seqSeq(u->index());
      v = vertex(e->to()->index());
      const string seqV = seqSeq(v->index());

      for (map<unsigned,Edge*>::iterator flinkIter2 = feasibleLinks.begin(); flinkIter2 != flinkIter; ++flinkIter2)
      {
        if (*(flinkIter2->second->from()) == *u || *(flinkIter2->second->from()) == *v)
          w = vertex(flinkIter2->second->to()->index());
        else if (*(flinkIter2->second->to()) == *u || *(flinkIter2->second->to()) == *v)
          w = vertex(flinkIter2->second->from()->index());
        else  continue; // this edge is not incident to u or v

        const string seqW = seqSeq(w->index());
        set<string> uvwMedians = computeQuasiMedianSeqs(seqU, seqV, seqW);
        set<string>::iterator medIter = uvwMedians.begin();
        while (medIter != uvwMedians.end())
        {
          set<string>::iterator seqIter = _allSeqSet.find(*medIter);

          if (seqIter == _allSeqSet.end())
          {
            double cost = computeCost(seqU, seqV, seqW, *medIter);
            if (cost <= minCost + _epsilon)
            {
              ostringstream oss;
              //oss << "int" << _medianSeqCount++;
              
              _medianNames.push_back(oss.str());
              _medianSeqs.push_back(*medIter);
              _allSeqSet.insert(*medIter);
              newVertex(_medianNames.back(), &(_medianSeqs.back()));
              changed = true;
            }
          }

          ++medIter;
        } // end while medIter != ...
      } // end for flinkIter2 = feasibleLinks...

      ++flinkIter;
    } // end while flinkIter != ...
    
  }  while (changed == true);
}

// code modified from SplitsTree
void MedJoinNet::computeMSN(map<unsigned,Edge*> *feasibleLinks)
{
  
  for (unsigned i = edgeCount(); i > 0; i--)  removeEdge(i - 1);
  
  unsigned seqCount = nseqs();

  unsigned *msnComp = new unsigned[seqCount];//nseqs()];
  unsigned *thresholdComp = new unsigned[seqCount];//nseqs()];
  unsigned ncomps = nseqs();
  VCPtrComparitor revcomp(true);
  VCPQ pairsByDist(revcomp);
  map<unsigned int, VertContainer*> dist2pairs;
  long maxValue = numeric_limits<long>::max();

  VertContainer *vcptr;

  for (unsigned i = 0; i < ncomps; i++)
  {
    msnComp[i] = i;
    thresholdComp[i] = i;

    for (unsigned j = 0; j < i; j++)
    {
      map<unsigned, VertContainer*>::iterator mapIt = dist2pairs.find(distance(i,j));

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

    // update threshold graph components:
    for (unsigned i = 0; i < nseqs(); i++) 
    {
      for (unsigned j = 0; j < i; j++) 
      {
        if (thresholdComp[i] != thresholdComp[j] && distance(i, j) < (threshold - _epsilon)) 
        {
          unsigned oldComp = thresholdComp[i];
          unsigned newComp = thresholdComp[j];
          
          if (oldComp < newComp)
          {
            oldComp = thresholdComp[j];
            newComp = thresholdComp[i];
           
          }
          
          for (unsigned k = 0; k < nseqs(); k++)
          {
            if (thresholdComp[k] == oldComp)
              thresholdComp[k] = newComp;
            else if (thresholdComp[k] > oldComp)
              thresholdComp[k]--;
          }
        }
      }
    }
    

    VertContainer::Iterator pairIt = vcptr->begin();
    vector<const Vertex**> newpairs;


    while (pairIt != vcptr->end())
    {
      const Vertex *u = (*pairIt)[0];
      const Vertex *v = (*pairIt)[1];

      newpairs.push_back(*pairIt);

      Vertex *uv = vertex(u->index());
      Vertex *vv = vertex(v->index());

      Edge *e = newEdge(uv, vv, (double)(vcptr->distance()));
      if (thresholdComp[u->index()] != thresholdComp[v->index()])
        feasibleLinks->insert(pair<unsigned,Edge*>(e->index(), e));//[e->index()] = e;

      ++pairIt;
    }

    vector<const Vertex**>::const_iterator newPairIt = newpairs.begin();

    while (newPairIt != newpairs.end())
    {
      unsigned int compU = msnComp[(*newPairIt)[0]->index()];
      unsigned int compV = msnComp[(*newPairIt)[1]->index()];

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
        for (unsigned i = 0; i < nseqs(); i++)
        {
          if (msnComp[i] == compU)
            msnComp[i] = compV;
          else if (msnComp[i] > compU)
            msnComp[i]--;
        }
        ncomps--;
      }

      if (ncomps == 1 && maxValue == numeric_limits<long>::max())
        maxValue = threshold + _epsilon;


      newPairIt++;
    }

    delete vcptr;
  }

  if (ncomps > 1)
  {
    throw NetworkError("pairsByDist is empty before graph is connected!");
  }

}



void MedJoinNet::computeDistances()
{

  // TODO if I used a vector here I could either just insert the new distances or copy over the old ones
  // Also... with a 2D vector or array, insertion wouldn't be as inefficient
  if (_distances)  delete [] _distances;

  _distances = new unsigned[nseqs() * nseqs()];

  for (unsigned i = 0; i < nseqs(); i++)
  {
    _distances[i * nseqs() + i] = 0;
    for (unsigned j = 0; j  < i; j++)
      _distances[i * nseqs() + j] = _distances[j * nseqs() + i] = pairwiseDistance(seqSeq(i), seqSeq(j));
  }
}

void MedJoinNet::setDistance(unsigned dist, unsigned i, unsigned j)
{
  if (i >= nseqs() || j >= nseqs())  throw NetworkError("Invalid index for distance.");

  _distances[i * nseqs() + j] = dist;

}

unsigned MedJoinNet::distance(unsigned i, unsigned j) const
{
  if (i >= nseqs() || j >= nseqs())  throw NetworkError("Invalid index for distance.");

  return _distances[i * nseqs() + j];
}

bool MedJoinNet::removeObsoleteVerts()
{
  Vertex *v;
  vector<Vertex*> obsoleteVerts;
  bool changed = true;
  bool vertsRemoved = false;
  
  while (changed)
  {
    // only unsampled sequence verts can be obsolete
    for (unsigned i = _nsamples; i < vertexCount(); i++)
    {
      v = vertex(i);
      if (v->degree() < 2)
        obsoleteVerts.push_back(v);
    }
    
    if (obsoleteVerts.empty())  changed = false;
    else
    {
      vertsRemoved = true;
      vector<Vertex*>::iterator vertIt = obsoleteVerts.begin();
      
      while (vertIt != obsoleteVerts.end())
      {
        _allSeqSet.erase((*vertIt)->label());
        _medianNames.erase(_medianNames.begin() + ((*vertIt)->index() - _nsamples));
        _medianSeqs.erase(_medianSeqs.begin() + ((*vertIt)->index() - _nsamples));
        removeVertex((*vertIt)->index());
        ++vertIt;
      } 
      
      obsoleteVerts.clear();
    }
  }
  
  return vertsRemoved;
}

set<string> MedJoinNet::computeQuasiMedianSeqs(const string &seqA, const string &seqB, const string &seqC) const
{
  //vector<string> median;
  set<string> medianSet;
  string qmSeq(seqA);
  bool hasStar = false;
  
  for (unsigned i = 0; i < seqA.length(); i++) 
  {
    // do nothing: qmSeq is already set to chars in seqA
    if (seqA.at(i) == seqB.at(i) || seqA.at(i) == seqC.at(i)) ;
    else if (seqB.at(i) == seqC.at(i))  qmSeq.at(i) = seqB.at(i);
    else
    {
      qmSeq.at(i) = '*';
      hasStar = true;
    }
  }
  
  if (! hasStar)
  {
    //median.push_back(qmSeq);
    medianSet.insert(qmSeq);
    return medianSet;
  }
  
  else
  {
    stack<string> medianStack;
    //set<string> medianSet;
    medianStack.push(qmSeq);
    
    while (! medianStack.empty())
    {
      string seq = medianStack.top();
      medianStack.pop();
      
      string first(seq);
      string second(seq);
      string third(seq);
      
      size_t firstStar = seq.find('*');     
      size_t nextStar = seq.find('*', firstStar + 1);
      
      if (firstStar == string::npos)
        throw NetworkError("There should be a 'star' position in this sequence!");

      first.at(firstStar) = seqA.at(firstStar);
      second.at(firstStar) = seqB.at(firstStar);
      third.at(firstStar) = seqC.at(firstStar);
      
      if (nextStar == string::npos)
      {
        medianSet.insert(first);
        medianSet.insert(second);
        medianSet.insert(third);
      }
      
      else
      {
        medianStack.push(first);
        medianStack.push(second);
        medianStack.push(third);
      }
    }    
  }

  return medianSet;
}

unsigned MedJoinNet::computeCost(const string &seqU, const string &seqV, const string &seqW, const string &med) const
{
  return pairwiseDistance(seqU, med) + pairwiseDistance(seqV, med) + pairwiseDistance(seqW, med);
}


// To double-check feasibleLinks, debugging only
bool MedJoinNet::areConnected(Vertex *src, Vertex *dst, unsigned threshold, bool outerCall)
{
  if (outerCall)  unmarkVertices();
    
  
  // option 1: we reach dst crossing only edges with weight < threshold
  if (src == dst)  return true;

  if (src->marked()) 
  {
    src->mark();
     
    Vertex::EdgeIterator edgeIt = src->begin();
    while (edgeIt != src->end())
    {
      if ((*edgeIt)->weight() < threshold)
      {
        Vertex *v = opposite(src, (*edgeIt));
        
        // recursively pass a result from a path to dst up the line
        if (areConnected(v, dst, threshold, false))  return true;
      }     
      ++edgeIt;
    }     
  }
  
  // option 2: we have not yet found path to dst with only weights < threshold. If this is the outermost call, no such path exists
  return false;
}
