/*
 * TightSpanWalker.cpp
 *
 *  Created on: Sep 14, 2012
 *      Author: jleigh
 */

#include "TightSpanWalker.h"
#include "NetworkError.h"
#include <QTime>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
using namespace std;

TightSpanWalker::TightSpanWalker(const vector <Sequence*> & seqs, const vector<bool> & mask)
 : HapNet(seqs, mask), _nsamples(HapNet::nseqs()), _ninternalVerts(0)
{
  _K = new Graph();
  //setupGraph();
}

TightSpanWalker::~TightSpanWalker()
{
  delete _K;
}

void TightSpanWalker::computeGraph()
{
  computeDT();
  
  double progperseq = 200./(nseqs() * (nseqs() - 1));
  double progress = 0;
  
  for (unsigned i = 0; i < nseqs(); i++)
  {
    newVertex(seqName(i), &seqSeq(i));
    _K->newVertex(seqName(i));
    vector<float> dTvect;
    for (unsigned j = 0; j < nseqs(); j++)
      dTvect.push_back(dT(i,j));
    
    _vertexMap[dTvect] = vertex(i);
  }
  QTime executionTimer;  
  executionTimer.start();
  for (unsigned i = 0; i < nseqs(); i++)
  {
    for (unsigned j = 0; j < i; j++)
    {
      geodesic(vertex(i), vertex(j));
      progress += progperseq;
      updateProgress(progress);
    }
  }
  

 /* ofstream kf("Mel.cytoscape");
  for (unsigned i = 0; i < edgeCount(); i++)
  {
    kf <<  edge(i)->to()->index() << '\t' << edge(i)->weight() << '\t'  << edge(i)->from()->index() << endl;    
  }
  kf.close();*/
  
  updateProgress(100);
}

/*void TightSpanWalker::computeGraph()
{
  
  computeDT();
  // These should maybe be lists, not vectors
  vector <unsigned> componentSizes;
  unsigned ncomps = nseqs();

  list<list<Vertex *> >::iterator compIt = _components.end();
  for (unsigned i = 0; i < nseqs(); i++)
  {
    _components.push_back(list<Vertex *>());
    if (compIt == _components.end())  compIt = _components.begin();
    else  ++compIt;
    (*compIt).push_back(newVertex(seqName(i), &seqSeq(i)));
    _K->newVertex(seqName(i));
    _componentIDs.push_back(i);
    componentSizes.push_back(1);
  }
  
  int compA = -1, compB = -1;
  bool merging = false;
  
  // New vertices not yet assigned to a component
  vector<Vertex *> noMansLand; 
  vector<VertPair> otherPairs;
  unsigned njoined;
  
  VPComparitor revcomp(true);
  
  VPPQ pairsByDist(revcomp); 
  // VertContainer can be used to store vertex pairs with the same 
  // distance. Here, it will store a single pair only
  VertPair vp;

  for (unsigned i = 0; i < nseqs(); i++)
  {
    for (unsigned j = 0; j < i; j++)
    {
      vp.first = vertex(i);
      vp.second = vertex(j);
      vp.dT = dT(i, j);
      
      pairsByDist.push(vp);
    }
  }
  
  while (ncomps > 1)
  {
    if (pairsByDist.empty())  throw NetworkError("Pairs queue is empty but multiple components remain!");
    
    vp = pairsByDist.top();
    pairsByDist.pop();
    
    unsigned u = vp.first->index();
    int compU = _componentIDs.at(u);
    unsigned v = vp.second->index();
    int compV = _componentIDs.at(v);
    
    if (compU > compV)
    {
      swap(compU, compV);
      swap(u, v);
    }
        
    if (compU == compV)
    {
      //delete vcptr;   
      vector<VertPair> tmpvect;
      
      while (!pairsByDist.empty())
      {
        tmpvect.push_back(pairsByDist.top());
        pairsByDist.pop();
      }
      
      for (unsigned i = 0; i < tmpvect.size(); i++)  
        pairsByDist.push(tmpvect.at(i));
      tmpvect.clear();
    }
    
    else
    {
      if (! merging)
      {
        compA = compU;
        compB = compV;
        merging = true;
        njoined = 0;
      }
      
      if (merging)
      {
        if (compU != compA || compV != compB)
          otherPairs.push_back(vp);
        
        else
        {

          //vector<pair<Vertex*, Vertex*> > fandg;
          //if (edgeCount() >= 343)
           // cout << "break here" << endl;
          pair<Vertex*,Vertex*> fandg = findFandG(u, v, compA, compB, noMansLand);//&f, &g, compListA, compListB, vcptr->distance);
          
          // Add any newly created vertices to NML
          for (unsigned i = _componentIDs.size(); i < vertexCount(); i++)
          {
            _componentIDs.push_back(-1);
            noMansLand.push_back(vertex(i));
          }
          


          if (fandg.first != fandg.second && ! fandg.first->isAdjacent(fandg.second))
          {
            Vertex *f = fandg.first;
            Vertex *g = fandg.second;
            double dp = pathLength(f, g);
            
            // if there is no path, or if the path is longer than dT
            if (dp == numeric_limits<double>::max() || ! aboutEqual((float)dp, dT(f->index(), g->index())))
            {
              unsigned vertsAdded = geodesic(f, g);

              // add all newly created vertices to nomansland
              for (unsigned i = 0; i < vertsAdded; i++)
              {
                unsigned idx = vertexCount() - vertsAdded + i;
                noMansLand.push_back(vertex(idx));
                _componentIDs.push_back(-1);
              }
            }
          }
          
          njoined++;
        
          // finished examining all pairs for compA and compB?
          if (njoined == (componentSizes.at(compA) * componentSizes.at(compB)))
          {
            
            componentSizes.at(compA) += componentSizes.at(compB);
            componentSizes.erase(componentSizes.begin() + compB);
            
            vector<VertPair>::iterator vpit = otherPairs.begin();
            while(vpit != otherPairs.end())
            {
              pairsByDist.push(*vpit);
              ++vpit;
            }
            otherPairs.clear();
            
            
            // relabel components
            for (unsigned j = 0; j < _componentIDs.size(); j++)
            {
              if (_componentIDs.at(j) > compB)
                _componentIDs.at(j)--;
              
              else if (_componentIDs.at(j) < 0 || _componentIDs.at(j) == compB)
                _componentIDs.at(j) = compA;
            }
              
            // merge component lists
            list<list<Vertex *> >::iterator compListIt = _components.begin();
            for (unsigned j = 0; j < compA; j++)  ++compListIt;
            list<Vertex *> *compListA = &(*compListIt);
            
            for (unsigned j = compA; j < compB; j++)  ++compListIt;
            list<Vertex *> *compListB = &(*compListIt);
            
            compListA->insert(compListA->end(), compListB->begin(), compListB->end());
            compListA->insert(compListA->end(), noMansLand.begin(), noMansLand.end());
            
            _components.erase(compListIt);
            noMansLand.clear();
            
            merging = false;
            njoined = 0;
            ncomps--;
          }
        }
      }              
    }         
  }
}

pair<Vertex *, Vertex*> TightSpanWalker::findFandG(unsigned u, unsigned v, int U, int V, const vector<Vertex *> &noMansLand)
{
  
  list<list<Vertex *> >::iterator compListIt = _components.begin();
  for (unsigned i = 0; i < U; i++)  compListIt++;
  list<Vertex *> *compU = &(*compListIt);
  for (unsigned i = U; i < V; i++)  compListIt++;
  list<Vertex *> *compV = &(*compListIt);

  float mindTfg = numeric_limits<float>::max();
  pair<Vertex *, Vertex *> fandg;
  
  list<Vertex *>::const_iterator compUit = compU->begin();
  vector<Vertex *>::const_iterator neutralIt = noMansLand.begin();
  bool switchlists = false;
  
  while (compUit != compU->end() || neutralIt != noMansLand.end())
  {
    
    Vertex *f;
    if (switchlists)  f = *neutralIt;
    else f = *compUit;
    
    list<Vertex *>::const_iterator compVit = compV->begin();
    vector<Vertex *>::const_iterator otherIt = noMansLand.begin();
    bool switchinnerlist = false;
    
    while (compVit != compV->end() || otherIt != noMansLand.end())
    {
      Vertex *g;
      if (switchinnerlist)  g = *otherIt;
      else g = *compVit;
      
      if (f != g)
      {
      float dTfg = dT(f->index(), g->index());

      float dp = dT(f->index(), u) + dTfg + dT(v, g->index());

      //float dp = pathLength(f, vertex(u)) + dTfg + pathLength(vertex(v), g);
      
      if (aboutEqual(dp, dT(u, v)) && (dTfg < mindTfg))
      {
        mindTfg = dTfg;

        fandg.first = f;
        fandg.second = g;
      }
      }
            
      if (switchinnerlist) ++otherIt;
      else 
      {
        ++compVit;
        if (compVit == compV->end()) 
          switchinnerlist = true;
      }
      
    }
    
    if (switchlists) ++neutralIt;
    else 
    {
      ++compUit;
      if (compUit == compU->end()) 
        switchlists = true;
    }
  }
  
  // check path lengths
  
  float fupath = pathLength(fandg.first, vertex(u));
  float gvpath = pathLength(fandg.second, vertex(v));
  
  if (! aboutEqual(fupath, dT(fandg.first->index(), u)))
    fixPath(vertex(u), fandg.first, *compU, noMansLand);
  
  if (! aboutEqual(gvpath, dT(fandg.second->index(), v)))
    fixPath(vertex(v), fandg.second, *compV, noMansLand);
 
  return fandg;
}

void TightSpanWalker::fixPath(Vertex *u, Vertex *v, const list<Vertex *> &component, const vector<Vertex *> &otherVerts)
{
  
  if (aboutEqual(pathLength(u, v), dT(u->index(), v->index())))
    return;
  
  pair<Vertex*, Vertex*> toJoin(u, v);
  
  float dTuv = dT(u->index(), v->index());
  float mindTxy = dTuv;
  
  list<Vertex *>::const_iterator it = component.begin();
  list<Vertex *>::const_iterator it2(it);
  vector<Vertex *>::const_iterator otherIt = otherVerts.begin();
  vector<Vertex *>::const_iterator otherIt2(otherIt);
  bool switchLists = false;
  Vertex *x, *y;
  
  while (it != component.end() || otherIt != otherVerts.end())
  {
    if (switchLists)
    {
      x = *otherIt;
      ++otherIt;
      otherIt2 = otherIt;
    }
    
    else
    {
      x = *it;
      ++it;
      it2 = it;
      
      if (it2 == component.end())
        switchLists = true;
      
      otherIt2 = otherVerts.begin();

    }
    
    while (it2 != component.end() || otherIt2 != otherVerts.end())
    {
      
      if (it2 == component.end())
        y = *otherIt2;
      else
        y = *it2;
      
       
      float dTxy = dT(x->index(), y->index());
      
      if (dTxy < mindTxy && ! aboutEqual(pathLength(x, y), dTxy))
      {
        if (aboutEqual(dTuv, dT(u->index(), x->index()) + dTxy + dT(y->index(), v->index())))
        {
            toJoin.first = x;
            toJoin.second = y;
            mindTxy = dTxy;
        }
        
        else if (aboutEqual(dTuv, dT(u->index(), y->index()) + dTxy + dT(x->index(), v->index())))
        {
          toJoin.second = x;
          toJoin.first = y;     
          mindTxy = dTxy;
        }
      }
      
      if (it2 == component.end())  
         ++otherIt2;
       else       
        ++it2;
    }
  }
    
  // while there are other vertices along this path that aren't correctly joined, join them recursively
  float xupath = pathLength(toJoin.first, u);
  float yvpath = pathLength(toJoin.second, v);
  
  if (! aboutEqual(xupath, dT(toJoin.first->index(), u->index())))
    fixPath(u, toJoin.first, component, otherVerts);
  
  if (! aboutEqual(yvpath, dT(toJoin.second->index(), v->index())))
    fixPath(v, toJoin.second, component, otherVerts);
  
  geodesic(toJoin.first, toJoin.second);
}*/


unsigned TightSpanWalker::geodesic(Vertex *f, Vertex *g)//, int compF, int compG, Vertex *lastF)
{
  for (unsigned i = _K->edgeCount(); i > 0;  i--)  _K->removeEdge(i - 1);
  
  Vertex *v, *u;
  Edge *e;
  unsigned i, j;
  
  for (i = 0; i < _K->vertexCount(); i++)  
  {
    v =  _K->vertex(i);
    v->setColour(Vertex::Black);
    v->unmark();
  }
  
  for (i = 0; i < _nsamples; i++)
  {
    float fi = dT(f->index(), i);
    
    for (j = 0; j < i; j++)
    {     
      if (j == i)  continue;
      float dij = distance(i, j);
      
      if (aboutEqual(fi + dT(f->index(), j), dij))
        _K->newEdge(_K->vertex(i), _K->vertex(j), dij); 
    }
  }
  
  queue<Vertex*> q;
  
  
  for (i = 0; i < _K->edgeCount(); i++)
  {
    e = _K->edge(i);
    v = _K->vertex(e->from()->index());
    u = _K->vertex(e->to()->index());

    float fv = dT(v->index(), f->index());
    float gv = dT(v->index(), g->index());
    float fu = dT(u->index(), f->index());
    float gu = dT(u->index(), g->index());
    if (fv < gv)
    { 
      if (aboutEqual(fv + dT(f->index(), g->index()) + gu, e->weight()) && (u->colour() == Vertex::Black))
      {
        u->setColour(Vertex::Green);  

        Vertex::EdgeIterator eit = u->begin();
        while (eit != u->end())
        {
          v = _K->opposite(u, *eit);
          v->setColour(Vertex::Red);  

          ++eit;
        }
      }
    }
    
    else if (fu < gu)
    {
      if (aboutEqual(fu + dT(f->index(), g->index()) + gv,  e->weight()) && (v->colour() == Vertex::Black))
      {
        v->setColour(Vertex::Green);  

        Vertex::EdgeIterator eit = v->begin();
        while (eit != v->end())
        {
          u = _K->opposite(v, *eit);
          u->setColour(Vertex::Red);  
          ++eit;
        }
      }
    }
  }
  
  q.push(_K->vertex(0));
  
  while (! q.empty())
  {
    v = q.front();
    q.pop();
    
    v->mark();
    if (v->colour() == Vertex::Red)
    {
      Vertex::EdgeIterator eit = v->begin();
      while (eit != v->end())
      {
        u = _K->opposite(v, *eit);
        if (! u->marked())  
          q.push(u);
        ++eit;
      }
    }
    
    else // v is black or green
    {
      v->setColour(Vertex::Green);
      Vertex::EdgeIterator eit = v->begin();
      while (eit != v->end())
      {
        u = _K->opposite(v, *eit);
        u->setColour(Vertex::Red);
        if (! u->marked())
          q.push(u);
        ++eit;
      }
    }
  } // end while (! q.empty())
  
  /*
  if (edgeCount() >= 343)
  {
    
    ostringstream oss;
    oss << "Kf_" << f->index() << ".cytoscape";
    string fname = oss.str();
    ofstream kf(fname.c_str());
    for (unsigned i = 0; i < _K->edgeCount(); i++)
    {
      kf <<  _K->edge(i)->to()->index() << '\t' << _K->edge(i)->weight() << '\t'  << _K->edge(i)->from()->index() << endl;
      
    }
    kf.close();
    
    oss.str("");
    oss.clear();
    oss << "Kf_" << f->index() << "_colours.cytoscape";
    
    fname = oss.str();
    ofstream kf_colours(fname.c_str());
    
    bool isbipartite = true;
    for (unsigned i = 0; i < _K->vertexCount(); i++)
    {
      //cout << "Vertex " << i << ": ";
      kf_colours << i << " =  ";
      switch(_K->vertex(i)->colour())
      {
        case Vertex::Green:
          kf_colours << "green" << endl;
          break;
        case Vertex::Red:
          kf_colours << "red" << endl;
          break;
        default:
          kf_colours << "uncoloured" << endl;
          break;
      }      
      
    }
    kf_colours.close();
    
    if (isbipartite) cout << "Kf is bipartite." << endl;
    else  cout << "Kf is NOT bipartite." << endl;
  }*/
  
  float delta = numeric_limits<float>::max();
  
  for (unsigned i = 0; i < _K->vertexCount(); i++)
  {
    if (_K->vertex(i)->colour() == Vertex::Green)
    {
      float fi = dT(f->index(), i);
      for (unsigned j = 0; j <= i; j++)
      {
        if (_K->vertex(j)->colour() == Vertex::Green)
          delta = min(delta, (fi + dT(f->index(), j) - distance(i, j)));
      }
    }
  }

  delta /= 2;
  
  
  if (delta < 0)  
  {
    //writeExceptionGraph();
    //cerr << "negative delta error" << endl;
    throw NetworkError("Something is wrong, delta should be positive.");
  }

  if (aboutEqual(dT(f->index(), g->index()), delta))
  {
    if (! f->isAdjacent(g))
      e = newEdge(f, g, delta);
    return 0;
  }
  
  
  // need to create h and call geodesic recursively
  else if (dT(f->index(), g->index()) > delta)
  {
    vector<float> newDTVect; 
    
    // compute dT vector to potential new vertex and see if it already exists
    // if not, create it
    // if so, just create a new edge from f to existing vertex
    
    for (unsigned i = 0; i < _K->vertexCount(); i++)
    {
      float fi = dT(f->index(), i);
      // If distance to vertex decreases between f and h, dT(v, h) is less than dT(v, f)
      if (_K->vertex(i)->colour() == Vertex::Green)
        newDTVect.push_back(fi - delta);
      
      // If distance to vertex increases between f and h, dT(v, h) is greater than dT(v, f)
      else  if (_K->vertex(i)->colour() == Vertex::Red)
        newDTVect.push_back(fi + delta);
      
      else
        throw NetworkError("Uncoloured vertex!");
      
    }
    
    Vertex *h;
    unsigned newverts = 0;
    
    map<vector<float>, const Vertex *>::iterator vertit = _vertexMap.find(newDTVect);
    
    if (vertit == _vertexMap.end())
    {
      newverts++;
      h = newVertex("");
      _vertexMap[newDTVect] = h;
      newDTVect.push_back(0);
      _dT.push_back(newDTVect);
      
      for (unsigned i = 0; i < _K->vertexCount(); i++)
        _dT.at(i).push_back(newDTVect.at(i));

      // calculating dT(i, h) for i not in X
      _dT.at(h->index()).resize(vertexCount(), 0);
      for (unsigned i = _K->vertexCount(); i < vertexCount(); i++)
      {
        float dTih = numeric_limits<float>::min();
        _dT.at(i).push_back(0);
        
        for (unsigned j = 0; j < _K->vertexCount(); j++)
        {
          for (unsigned k = 0; k < _K->vertexCount(); k++)
          {

            // consider both orders, (j, k) and (k, j)
            float dd = distance(j, k) - dT(h->index(), j) - dT(i, k);
            dTih = max(dTih, dd);
            dd = distance(j, k) - dT(h->index(), k) - dT(i, j);
            dTih = max(dTih, dd);
          }
        }
        
        setDT(dTih, h->index(), i);
      }
     e = newEdge(f, h, delta);
      
    }
    
    else // already a vertex with this dT vector
    {
      h = vertex(vertit->second->index());
      if (! f->isAdjacent(h))
        e = newEdge(f, h, delta);
    }
    //Vertex *h = newVertex("");
      

    return newverts + geodesic(h, g);
  }
  
  else  
  {
    //cout << *(dynamic_cast<Graph *>(this)) << endl;
    //cerr << "negative edge error." << endl;
    //writeExceptionGraph();
    throw NetworkError("Apparent negative edge length between vertices g and h");
  
  }
  return 0;
}

void TightSpanWalker::writeExceptionGraph()
{
  ofstream egraph("exception.graph");
  
  for (unsigned i = 0; i < vertexCount(); i++)
  {
    if (vertex(i)->label().empty())
      egraph << 'i' << i;
    else 
      egraph << vertex(i)->label();
    for (unsigned j = 0; j < _nsamples; j++)
      egraph << '\t' << dT(i, j);
      
    egraph << endl;
  }
  
  egraph.close();
}

void TightSpanWalker::computeDT()
{
  
  for (unsigned i = 0; i < _nsamples; i++)
  {
    _dT.push_back(vector<float>(nseqs(), 0));
    for (unsigned j = 0; j < i; j++)
    {
      float dTij = 0;
      for (unsigned k = 0; k < _nsamples; k++)
      {
        float dd = abs((float)(distance(i, k)) - (float)(distance(j, k)));
        dTij = max(dTij, dd);
      }
      _dT.at(i).at(j) = _dT.at(j).at(i) = dTij;
    }    
  }
}

bool TightSpanWalker::aboutEqual(float a, float b)
{ 
  
  float epsilon = numeric_limits<float>::epsilon();
  float biggest = max(abs(a), abs(b));
  
  if (min(abs(a), abs(b)) == 0)
  {
    return biggest <= (numeric_limits<float>::min() / epsilon);
  }
  
    
  return abs(a - b) <= (biggest * epsilon);
}

float TightSpanWalker::dT(unsigned i, unsigned j) const
{
  if (i >= _dT.size() || j >= _dT.at(i).size())  throw NetworkError("Invalid index for dT distance.");

  return _dT.at(i).at(j);
}

void TightSpanWalker::setDT(float dist, unsigned i, unsigned j)
{
  if (i >= _dT.size() || j >= _dT.at(i).size())  throw NetworkError("Invalid index for dT distance.");

  _dT.at(i).at(j) = _dT.at(j).at(i) = dist;
}

void TightSpanWalker::setDistance(unsigned dist, unsigned i, unsigned j)
{  
  if (i >= _nsamples || j >= _nsamples)  throw NetworkError("Invalid index for distance.");
   
  HapNet::setDistance(dist, i, j);               
}

unsigned TightSpanWalker::distance(unsigned i, unsigned j) const
{
    if (i >= _nsamples || j >= _nsamples)  throw NetworkError("Invalid index for distance.");

    return HapNet::distance(i, j);
}

