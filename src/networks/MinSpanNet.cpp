/*
 * MinSpanNet.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */
#include <limits>
using namespace std;

#include "MinSpanNet.h"

MinSpanNet::MinSpanNet(const std::vector<Sequence*> &seqs, const vector<bool> &mask, unsigned epsilon)
  : AbstractMSN(seqs, mask, epsilon)
{  
  //setupGraph();
}

MinSpanNet::~MinSpanNet() { }

void MinSpanNet::computeGraph()
{
  for (unsigned i = 0; i < nseqs(); i++)
    newVertex(seqName(i), &seqSeq(i));

  computeMSN();
  
}






