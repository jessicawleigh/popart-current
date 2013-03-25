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


/*void AncestralSeqNet::setupAncestors()
{
  computeAncestralSeqs(_alpha);
  _seqsComputed = true;
}*/

/*void AncestralSeqNet::computeDistances()
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
}*/

void AncestralSeqNet::computeGraph()
{

  //if (! _seqsComputed)
 //   throw NetworkError("Can't compute graph until ancestors have been set up!");
  map<const string, Vertex*> seq2Vert;
  set<string> leafSeqs;
  vector<unsigned> seqCount;
  vector<unsigned> edgeCount;

  //cout << "leaves:" << endl;


  /*
   * !!!!!!!!!!!!!!!!!!
   * Maybe problem here: think about whether we need original or condensed sequences. Parsimony trees will have same sequence multiple times, because they have original sequences.
   * This is possibly OK, depending on how (and where) conversion from original sequences to condensed happens
   */
  for (unsigned i = 0; i < nseqs(); i++)
  {
    //const Sequence *s = new Sequence(seqName(i, true), condensedSeqSeq(i));
    //cout << seqSeq(i) << endl;
    seq2Vert[seqSeq(i)] = newVertex(seqName(i), &seqSeq(i));
    leafSeqs.insert(string(seqSeq(i)));
    seqCount.push_back(0);

  }

  //cout << "ancestors:" << endl;

  unsigned niter = 1000; // Do something about this! Take into account number of trees.

  for (unsigned i = 0; i < niter; i++)
  {
    const list<pair<const string, const string> > edgelist = sampleEdge();

    // for each edge, check to see if sequences are new. If neither new, check to see if already adjacent
    list<pair<const string, const string> >::const_iterator edgeIt = edgelist.begin();
    set<unsigned> verticesSeen;
    set<unsigned> edgesSeen;

    while (edgeIt != edgelist.end())
    {
      //const pair<const string &, const string &> & mypairref = *edgeIt;
      //const string &seqFrom = mypairref.first;//(*edgeIt).first;
      //const string &seqTo = mypairref.second;//(*edgeIt).second;
      string seqFrom(edgeIt->first);
      string seqTo(edgeIt->second);

      //cout << "edge from " << seqFrom << " to " << seqTo << endl;

      bool seqFromNew = false;
      bool seqToNew = false;

      Vertex *u, *v;

      map<const string, Vertex *>::iterator seqIt = seq2Vert.find(seqFrom);

      if (seqIt != seq2Vert.end())
      {
        u = seqIt->second;
        //seqCount[u->index()]++;
        verticesSeen.insert(u->index());
      }

      else
      {
        u = newVertex("");
        seq2Vert[seqFrom] = u;
        //cout << seqFrom << endl;
        //seqCount.push_back(1);
        verticesSeen.insert(u->index());
        seqFromNew = true;
      }

      seqIt = seq2Vert.find(seqTo);

      if (seqIt != seq2Vert.end())
      {
        v = seqIt->second;
        //seqCount[v->index()]++;
        verticesSeen.insert(v->index());

      }

      else
      {
        v = newVertex("");
        seq2Vert[seqTo] = v;
        //cout << seqTo << endl;
        //seqCount.push_back(1);
        verticesSeen.insert(v->index());
        seqToNew = true;
      }

      if (seqToNew || seqFromNew)
      {
        Edge *e = newEdge(u, v);
        edgesSeen.insert(e->index());
      }

      else if (u != v && ! v->isAdjacent(u))
      {
        Edge *e = newEdge(u, v);
        edgesSeen.insert(e->index());

      }

      else if (u != v)
      {
        const Edge *e = v->sharedEdge(u);
        edgesSeen.insert(e->index());
      }

      //else if (u != v) cout << u->index() << " and " << v->index() << " are adjacent already." << endl;

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
  }

  
  /*for (unsigned i = 0; i < ancestorCount(); i++)
  {
    newVertex("");
  }



  computeMSN();*/

  //cout << *(dynamic_cast<Graph*>(this)) << endl;

  // Check that we don't produce a leaf with degree 0? (this could happen if all its possible ancestors are rarely observed)
  for (unsigned i = edgeCount.size(); i > 0; i--)
  {
    if ((double)edgeCount.at(i - 1)/niter < _alpha)
    {
      cout << "proportion for edge: " << (double)edgeCount.at(i-1)/niter;
      cout << " alpha: " << _alpha;
      cout << " removing edge " << i - 1 << endl;
      removeEdge(i - 1);

    }
  }

  for (unsigned i = seqCount.size(); i > nseqs(); i--)
  {
    cout << "count for vertex " << i - 1 << ": " << seqCount.at(i - 1) << endl;
    if (vertex(i-1)->degree() == 0)
    {
      cout << "removed vertex." << endl;
      removeVertex(i - 1);
    }
  }

  return;

  for (unsigned i = seqCount.size(); i > nseqs(); i--)
  {
    if ((double)seqCount.at(i - 1)/niter < _alpha)
    {
      cout << "proportion for vertex: " << (double)seqCount.at(i-1)/niter;
      cout << " removing vertex " << i - 1 << endl;
      removeVertex(i - 1);

    }
    else
      cout << "proportion for vertex: " << (double)seqCount.at(i-1)/niter << endl;

  }
  cout << "alpha: " << _alpha << endl;

  for (unsigned i = 0; i < seqCount.size(); i++)
    cout << i << ": " << seqCount.at(i) << endl;
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
