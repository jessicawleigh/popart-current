#include "IntNJ.h"
#include "NetworkError.h"
#include "Graph.h"

#include <lp_lib.h>

#include <algorithm>
#include <limits>
#include <iostream>
using namespace std;

IntNJ::IntNJ(const vector<Sequence *> & seqs, const vector<bool> & mask, unsigned epsilon)
  : HapNet::HapNet(seqs, mask), _leafCount(HapNet::nseqs()), _epsilon(epsilon)
{
  _distances = 0;
  _internalVertCount = 0;
  
  //setupGraph();
}

IntNJ::~IntNJ()
{
  if (_distances)  delete [] _distances;
}
  
size_t IntNJ::nseqs() const
{
  return _leafCount + _internalVertCount;
}

const string & IntNJ::seqName(unsigned idx, bool isOrig) const
{
  if (isOrig || idx < _leafCount)
    return HapNet::seqName(idx, isOrig);
  
  else  if (idx >= nseqs())
    throw NetworkError("Index is greater than the number of vertices in graph!");
  
  else
    return vertex(idx)->label();
}

const string & IntNJ::seqSeq(unsigned idx, bool isOrig) const
{
  if (isOrig || idx < _leafCount)
    return HapNet::seqSeq(idx, isOrig);
  
  else  
    throw NetworkError("No sequence associated with this index, or index out of range.");

}

void IntNJ::computeGraph()
{
  integerNJ();
  updateProgress(10);
  optimiseEdges();
  updateProgress(50);
  
  for (unsigned i = edgeCount(); i > 0; i--)
  {
    Edge *e = edge(i - 1);
    if (e->weight() == 0)
    {
      Vertex *u = vertex(e->from()->index());
      Vertex *v = vertex(e->to()->index());
      Vertex *w;

      // if u and v are both leaves, do nothing
      if (u->index() < _leafCount && v->index() < _leafCount)
        continue;
      
      // we will delete one of these, don't want it to be a leaf
      if (u->index() > v->index())
        swap(u, v);
      
      vector<const Edge*> incidentEdges;//v->degree(), 0);
      //for (
      Vertex::EdgeIterator eit = v->begin(); 
      while (eit != v->end())
      {
        incidentEdges.push_back(*eit);
        ++eit;
      }
            
      for (unsigned i = incidentEdges.size(); i > 0; i--)
      {   
        if (incidentEdges.at(i - 1) != e)
        {
          w = opposite(v, incidentEdges.at(i - 1));
          moveEdge(incidentEdges.at(i - 1)->index(), u, w);           
        }        
      }
      removeVertex(v->index());   
    }
    
  }  
  
  VCPtrComparitor revcomp(true);
  VCPQ pairsByDist(revcomp);
  map<unsigned int, VertContainer*> dist2pairs;
  VertContainer *vcptr;
  unsigned npairs = _leafCount * (_leafCount - 1) / 2;
  
  // Think about how to do this better
  for (unsigned i = 0; i < _leafCount; i++)
  {
    for (unsigned j = 0; j < i; j++)
    {
      unsigned cost = pathLength(vertex(i), vertex(j)) - distance(i,j);
      if (cost == 0)  continue;
      
      map<unsigned, VertContainer*>::iterator mapIt = dist2pairs.find(cost);
       
      if (mapIt == dist2pairs.end())
      {
        vcptr = new VertContainer(cost);         
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
  unsigned pairsleft = npairs;
  
  while (! pairsByDist.empty())
  {
    vcptr = pairsByDist.top();
    pairsByDist.pop();
    
    VertContainer::Iterator pairIt = vcptr->begin();
    
    while (pairIt != vcptr->end())
    {
      Vertex *u = vertex((*pairIt)[0]->index());
      Vertex *v = vertex((*pairIt)[1]->index());
      int actualCost = (int)(pathLength(u, v)) - distance(u->index(), v->index());
      
      if (actualCost < 0)  throw NetworkError("Path length is shorter than allowed!");
      
      if (actualCost > 0)
      {
        vector<Vertex *> path;
        path.push_back(u);
    
        PathIterator pathIt = beginPath(u, v);
        
        while (pathIt != endPath())
        {
          path.push_back(*pathIt);
          ++pathIt;
        }
        path.push_back(v);
        
        int minNewLength = numeric_limits<int>::max();
        
        const Edge * intermediates[2];
        int offset[2];
        
        
        for (unsigned i = 0; i < (path.size() - 2); i++)
        {
          Vertex *w_i = path.at(i);
          const Edge *e_i = w_i->sharedEdge(path.at(i + 1));
          unsigned duwi = pathLength(u, w_i);
          
          for (unsigned j = path.size() - 1; j > (i + 1); j--)
          {
            Vertex *w_j = path.at(j);
            const Edge *e_j = w_j->sharedEdge(path.at(j - 1)); 
            unsigned dvwj = pathLength(v, w_j);
            
            for (unsigned x_i = 0; x_i < e_i->weight(); x_i++)
            {
              
              for (unsigned x_j = 0; x_j < e_j->weight(); x_j++)
              {
                int newLength = distance(u->index(), v->index()) - duwi - x_i - dvwj - x_j;
                
                if (newLength >= 1 && newLength < minNewLength)
                {
                  if (isLegal(w_i, e_i, x_i, w_j, e_j, x_j, newLength))
                  {
                    intermediates[0] = e_i;
                    if (w_i == e_i->from())
                      offset[0] = x_i;
                    else
                      offset[0] = e_i->weight() - x_i;
                    
                    intermediates[1] = e_j;
                    if (w_j == e_j->from())
                      offset[1] = x_j;
                    else
                      offset[1] = e_j->weight() - x_j;
                    
                    minNewLength = newLength;
                  }
                }
              }
            }
          }
        } // end for i...
        
        Vertex *x, *y;
        if (minNewLength < (actualCost + _epsilon)) //numeric_limits<unsigned>::max())
        {
          if (offset[0] == 0)
            x = vertex(intermediates[0]->from()->index());
            
          else if (offset[0] == intermediates[0]->weight())
            x = vertex(intermediates[0]->to()->index());
          
          // need to add a new vertex at offset
          else
          {
            x = newVertex("");
            newEdge(x, vertex(intermediates[0]->to()->index()), intermediates[0]->weight() - offset[0]);
            newEdge(vertex(intermediates[0]->from()->index()), x, offset[0]);
            removeEdge(intermediates[0]->index());
          }
          
          if (offset[1] == 0)
            y = vertex(intermediates[1]->from()->index());
            
          else if (offset[1] == intermediates[1]->weight())
            y = vertex(intermediates[1]->to()->index());
          
          // need to add a new vertex at offset
          else
          {
            y = newVertex("");
            newEdge(y, vertex(intermediates[1]->to()->index()), intermediates[1]->weight() - offset[1]);
            newEdge(vertex(intermediates[1]->from()->index()), y, offset[1]);
            removeEdge(intermediates[1]->index());
          }
          
          
          newEdge(x, y, minNewLength);
        }
        
      }
      
      ++pairIt;
      pairsleft--;
    }
    delete vcptr;
    updateProgress(100 - (unsigned)(0.5 + 50 * (pairsleft/(double)npairs)));

  }
  updateProgress(100);
}

void IntNJ::integerNJ()
{
  vector<const Vertex *> NJVect;
  Edge *e;
  
  for (unsigned i = 0; i < _leafCount; i++)
    NJVect.push_back(newVertex(seqName(i), &seqSeq(i)));
  
  unsigned ncomponents = _leafCount;
  unsigned minpair[2];// = {-1,-1};
  //unsigned *scratchDists = new unsigned[_leafCount * _leafCount];
  vector<double> scratchDists(_leafCount * _leafCount, 0);
  
  
  for (unsigned i = 0; i < _leafCount; i++)
  {
    for (unsigned j = i; j < _leafCount; j++)
    {
      scratchDists.at(i * _leafCount + j) = scratchDists.at(j * _leafCount + i) = distance(i, j);
    }
  }
  
  vector<double> Qmat;
  
  while (ncomponents > 2)
  {
    

    Qmat.resize(ncomponents * ncomponents, 0);
    //int *Qmat = new int[_leafCount * _leafCount];
    //minpair[0] = minpair[1] = -1;
    double minQval = numeric_limits<double>::max();//UINT_MAX;
    
    for (unsigned i = 0; i < (ncomponents * ncomponents); i++)  Qmat.at(i) = 0;
        
    for (unsigned i = 0; i < ncomponents; i++)
    {
      for (unsigned j = 0; j < i; j++)
      {
        unsigned qIndex = i * ncomponents + j;
        Qmat.at(qIndex) = (ncomponents - 2) * scratchDists.at(qIndex);//distance(i, j);
        for (unsigned k = 0; k < ncomponents; k++)
        {
          Qmat.at(qIndex) -= scratchDists.at(i * ncomponents + k);
          Qmat.at(qIndex) -= scratchDists.at(j * ncomponents + k);
        }        
        
        if (Qmat.at(qIndex) < minQval)
        {
          minQval = Qmat.at(qIndex);
          minpair[0] = i;
          minpair[1] = j;
        }
      }
    }
    
    
    
    double dfu, dgu;
    NJReduce(scratchDists, ncomponents, minpair[0], minpair[1], &dfu, &dgu);    
    
    Vertex *ancestor;// = newNode("");
    Vertex *u = vertex(NJVect.at(minpair[0])->index());
    Vertex *v = vertex(NJVect.at(minpair[1])->index());

    
    // set distances >= 0
    if (dfu < 0)  dfu = 0;
    if (dgu < 0)  dgu = 0;
    
    ancestor = newVertex("");
    e = newEdge(ancestor, v, dgu);
    
    e = newEdge(ancestor, u, dfu);

    
    // Always: minpair[1] < minpair[0]
    
    NJVect.at(minpair[1]) = ancestor;
    
    NJVect.erase(NJVect.begin() + minpair[0]);
    
    ncomponents--;
  }
  
  Vertex *u = vertex(NJVect.at(0)->index());
  Vertex *v = vertex(NJVect.at(1)->index());
  
  newEdge(u, v, scratchDists.at(2));
  
}
  
void IntNJ::NJReduce(vector<double> &dmat, unsigned nrow, unsigned idx1, unsigned idx2, double *dfu, double *dgu)
{
  
  if (idx2 >= idx1)  
  {
    //cerr << "Matrix not treated as lower triangular. Problem?" << endl;
    throw NetworkError("Matrix not treated as lower triangular");
  }
  
 *dfu = newDist(dmat, nrow, idx1, idx2);
 if (*dfu < 0)  *dfu = 0;
 
 *dgu = newDist(dmat, nrow, idx2, idx1);
 if (*dgu < 0)  *dgu = 0;
  
 double * newmat = new double[(nrow - 1) * (nrow - 1)];
 double newdist;
 
 for (unsigned i = 0; i < nrow - 1; i++)
 {
   newmat[i * (nrow - 1) + i] = 0;
   for (unsigned j = 0; j < i; j++)
   {
     if (i == idx2)
     {
       if (j >= idx1)
         newdist = 0.5 * (dmat.at(idx1 * nrow + j + 1) - *dfu + dmat.at(idx2 * nrow + j + 1) - *dgu);
       else 
         newdist = 0.5 * (dmat.at(idx1 * nrow + j) - *dfu + dmat.at(idx2 * nrow + j) - *dgu);
     }
     
     else if (j == idx2)
     {
       if (i >= idx1)
         newdist = 0.5 * (dmat.at((i + 1) * nrow + idx1) - *dfu + dmat.at((i + 1) * nrow + idx2) - *dgu);
       else
         newdist = 0.5 * (dmat.at(i * nrow + idx1) - *dfu + dmat.at(i * nrow + idx2) - *dgu);
     }
     
     else if (i >= idx1)
     {
        if (j >= idx1)
          newdist = dmat.at((i + 1) * nrow + j + 1);
        else
          newdist = dmat.at((i + 1) * nrow + j);
     }
     
     else
     {
       if (j >= idx1)
         throw NetworkError("This shouldn't have happened, indices are messed up.");
       else
         newdist = dmat.at(i * nrow + j);

     }
     
     newmat[i * (nrow - 1) + j]  = newdist;
     newmat[j * (nrow - 1) + i]  = newdist;
   }
 }
 
 dmat.resize((nrow - 1) * (nrow - 1));
 
 for (unsigned i = 0; i < (nrow - 1) * (nrow - 1); i++)  dmat.at(i) = newmat[i];
 
 delete [] newmat;
    
}

double IntNJ::newDist(vector<double> &dmat, unsigned nrow, unsigned idx1, unsigned idx2)
{
  double dist = 0.5 * dmat.at(idx1 * nrow + idx2);
  double sum = 0;
  for (unsigned i = 0; i < nrow; i++)
  {
    sum += dmat.at(idx1 * nrow + i);
    sum -= dmat.at(idx2 * nrow + i);
  }
  
  dist += sum/(2 * (nrow - 2));
  
  return dist;
}

void IntNJ::optimiseEdges()
{
  // dimensions of the splits matrix (and LP problem) 
  unsigned m = (_leafCount * (_leafCount - 1)) / 2;
  unsigned n = edgeCount();
  double **splitsMat = new double*[m];

  for (unsigned i = 0; i < m; i++)
  {
    // lpsolve uses base 1 arrays
    splitsMat[i] = new double[n + 1];
    for (unsigned j = 0; j <= n; j++)  
      splitsMat[i][j] = 0;
    
    // trick my own code to think this is base 0
    splitsMat[i]++;
  }
  
  const Edge *e = edge(edgeCount() - 1);
  traverseSplits(e->from(), e, splitsMat);
  
  Vertex::EdgeIterator eit = e->to()->begin();
  while (eit != e->to()->end())
  {
    if (*eit != e)
      traverseSplits(opposite(e->to(), *eit), *eit, splitsMat);
    ++eit;
  }
    
  // move back to the allocated base 1 arrays  
  for (unsigned i = 0; i < m; i++) 
    splitsMat[i]--;
    
  double *objectiveFn = new double[n + 1];
  
  for (unsigned i = 0; i < n; i++)
    objectiveFn[i + 1] = 1;

  objectiveFn[0] = 0;
  
  // a column vector of all pairwise distances
  vector<double> seqDists(m, 0);
  seqDists.resize(m, 0);
  for (unsigned i = 0; i < _leafCount; i++)
    for (unsigned j = i + 1; j < _leafCount; j++)
      seqDists.at(i * _leafCount + j - ((i + 1) * (i + 2))/2) = distance(i, j);
      
  lprec *lp;
  
  lp = make_lp(0, n);
  
  if (lp == NULL)  throw NetworkError("Unable to create new LP model");
  
  set_verbose(lp, IMPORTANT);
  
  for (unsigned i = 0; i < m; i++)
    add_constraint(lp, splitsMat[i], GE, seqDists[i]);

  set_obj_fn(lp, objectiveFn);
  for (unsigned i = 0; i < n; i++)
    set_int(lp, (i + 1), true);  
  
  solve(lp);
  
  double *branchLengths;
  get_ptr_variables(lp, &branchLengths);
  
  for (unsigned i = 0; i < n; i++)  
    edge(i)->setWeight(branchLengths[i]);
 
  
  // cleanup
  delete_lp(lp);

  delete [] objectiveFn;
  for (unsigned i = 0; i < m; i++)     
    delete [] splitsMat[i];
  
  delete [] splitsMat;  
}


vector<unsigned>  IntNJ::traverseSplits(const Vertex *parent, const Edge *up, double **splitsMat) const
{
  vector<unsigned> children;
  vector<bool> isChild(_leafCount, false);
  isChild.resize(_leafCount, false);
  
  if (parent->degree() == 1)
  {
    isChild.at(parent->index()) = true;
    children.push_back(parent->index());
  }
    
  else
  {
    Vertex::EdgeIterator eit = parent->begin();
    const Vertex *v;
  
    while (eit != parent->end())
    {
      if (*eit != up)
      {
        v = opposite(parent, *eit);
        vector<unsigned> grandchildren = traverseSplits(v, *eit, splitsMat);
        children.insert(children.end(), grandchildren.begin(), grandchildren.end());
      }
      ++eit;
    }
  
    vector<unsigned>::iterator childIt = children.begin();
    while (childIt != children.end())
    {
      isChild.at(*childIt) = true;
      ++childIt;
    } 
  }
  
  for (unsigned i = 0; i < _leafCount; i++)
  {
    for (unsigned j = i + 1; j < _leafCount; j++)
    {
      if (isChild[i] != isChild[j])  
        splitsMat[i * _leafCount + j - ((i + 1) * (i + 2))/2][up->index()] = 1;
    }
  }
  
  return children;
}

bool IntNJ::isLegal(const Vertex *v_i, const Edge *e_i, unsigned x_i, const Vertex *v_j, const Edge *e_j, unsigned x_j, unsigned length)
{
  const Vertex *u_i = opposite(v_i, e_i);
  const Vertex *u_j = opposite(v_j, e_j);
  
  for (unsigned k = 0; k < _leafCount; k++)
  {
    // distance from i to x_i via an endpoint
    unsigned dki = min(pathLength(vertex(k), v_i) + x_i, pathLength(vertex(k), u_i) + e_i->weight() - x_i);
    unsigned dkj = min(pathLength(vertex(k), v_j) + x_j, pathLength(vertex(k), u_j) + e_j->weight() - x_j);
    
    for (unsigned l = 0; l < k; l++)
    {
      unsigned dli = min(pathLength(vertex(l), v_i) + x_i, pathLength(vertex(l), u_i) + e_i->weight() - x_i);
      unsigned dlj = min(pathLength(vertex(l), v_j) + x_j, pathLength(vertex(l), u_j) + e_j->weight() - x_j);
      
      if (dki + length + dlj < distance(k, l))  return false;
      if (dkj + length + dli < distance(k, l))  return false;
    }
  }
  
  return true;
}

// Create a direct path of length dist from start to end with one-step edges
void IntNJ::newCompositeEdge(Vertex *start, Vertex *end, unsigned dist)
{
  Vertex *u = start, *v;
  
  for (unsigned i = 1; i < dist; i++)
  {
    v = newVertex("");
    newEdge(u, v, 1);
    u = v;
  }
  
  newEdge(u, end, 1);
}

  

