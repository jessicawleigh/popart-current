#include "AncestralSeqNet.h"
#include "NetworkError.h"

#include <iostream>
#include <map>
#include <set>
using namespace std;


AncestralSeqNet::AncestralSeqNet(const vector <Sequence*> & seqvect, const vector<bool> & mask, unsigned epsilon, double alpha) 
 : /*AbstractMSN(seqvect, mask, epsilon), */HapNet(seqvect, mask), _seqsComputed(false), _alpha(alpha)
{
}


void AncestralSeqNet::computeGraph()
{
  updateProgress(0);
  map<const string, Vertex*> seq2Vert;
  vector<unsigned> seqCount;
  vector<unsigned> edgeCount;

  for (unsigned i = 0; i < nseqs(); i++)
  {
    seq2Vert[seqSeq(i)] = newVertex(seqName(i), &seqSeq(i));
    seqCount.push_back(0);

  }

  double progPerIter = 100./niter();
  double prog;

  for (unsigned i = 0; i < niter(); i++)
  {
    const list<pair<const string, const string> > edgelist = sampleEdge();

    // for each edge, check to see if sequences are new. If neither new, check to see if already adjacent
    list<pair<const string, const string> >::const_iterator edgeIt = edgelist.begin();
    set<unsigned> verticesSeen;
    set<unsigned> edgesSeen;

    while (edgeIt != edgelist.end())
    {

      string seqFrom(edgeIt->first);
      string seqTo(edgeIt->second);


      bool seqFromNew = false;
      bool seqToNew = false;

      Vertex *u, *v;

      map<const string, Vertex *>::iterator seqIt = seq2Vert.find(seqFrom);

      if (seqIt != seq2Vert.end())
      {
        u = seqIt->second;
        verticesSeen.insert(u->index());
      }

      else
      {
        u = newVertex("");
        seq2Vert[seqFrom] = u;

        verticesSeen.insert(u->index());
        seqFromNew = true;
      }

      seqIt = seq2Vert.find(seqTo);

      if (seqIt != seq2Vert.end())
      {
        v = seqIt->second;
        verticesSeen.insert(v->index());
      }

      else
      {
        v = newVertex("");
        seq2Vert[seqTo] = v;

        verticesSeen.insert(v->index());
        seqToNew = true;
      }

      if (seqToNew || seqFromNew)
      {
        double weight = pairwiseDistance(seqFrom, seqTo);
        Edge *e = newEdge(u, v, weight);
        edgesSeen.insert(e->index());
      }

      else if (u != v && ! v->isAdjacent(u))
      {
        double weight = pairwiseDistance(seqFrom, seqTo);
        Edge *e = newEdge(u, v, weight);
        edgesSeen.insert(e->index());

      }

      else if (u != v)
      {
        const Edge *e = v->sharedEdge(u);
        edgesSeen.insert(e->index());
      }


      ++edgeIt;
    }

    set<unsigned>::const_iterator idxIt = verticesSeen.begin();

    while (idxIt != verticesSeen.end())
    {
      if (*idxIt >= seqCount.size())
        seqCount.resize(*idxIt + 1, 0);

      seqCount.at(*idxIt) += 1;

      ++idxIt;
    }
    idxIt = edgesSeen.begin();

    while (idxIt != edgesSeen.end())
    {
      if (*idxIt >= edgeCount.size())
        edgeCount.resize(*idxIt + 1, 0);

      edgeCount.at(*idxIt) += 1;

      ++idxIt;
    }

    prog = (i + 1) * progPerIter;
    updateProgress(prog);
  }


  //cout << *(dynamic_cast<Graph*>(this)) << endl;
  priority_queue<pair<double, Edge*>, vector<pair<double, Edge*> >,  greater<pair<double, Edge*> > > edgeQ;

  for (unsigned i = 0; i < edgeCount.size(); i++)
  {
    double edgeFreq = (double)edgeCount.at(i)/niter();

    if (edgeFreq < _alpha)
      edgeQ.push(pair<double, Edge*>(edgeFreq, edge(i)));
  }

  while (! edgeQ.empty())
  {
    Edge *e = edgeQ.top().second;

    double weight = e->weight();
    Vertex *u = vertex(e->from()->index());
    Vertex *v = vertex(e->to()->index());
    //unsigned idx = e->index();

    removeEdge(e->index());

    if (! areConnected(u, v))
      newEdge(u, v, weight);

    edgeQ.pop();
  }

  for (unsigned i = seqCount.size(); i > nseqs(); i--)
  {
    if (vertex(i-1)->degree() <= 1)
    {
      removeVertex(i - 1);
    }
  }

  updateProgress(100);

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
