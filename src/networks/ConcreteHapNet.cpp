#include "ConcreteHapNet.h"

#include "Graph.h"

using namespace std;

ConcreteHapNet::ConcreteHapNet(const Graph &g, const vector<Sequence*> & seqs, const vector<bool> & mask)
 : HapNet(seqs, mask)
{
  for (unsigned i = 0; i < g.vertexCount(); i++)
  {
    if (i < nseqs())
      newVertex(seqName(i), &seqSeq(i));
    else
      newVertex("");
  }
  
  for (unsigned i = 0; i < g.edgeCount(); i++)
  {
    const Edge *e = g.edge(i);
    newEdge(vertex(e->from()->index()), vertex(e->to()->index()), e->weight());
  }
}

void ConcreteHapNet::computeGraph()
{
  
  updateProgress(100);
}
