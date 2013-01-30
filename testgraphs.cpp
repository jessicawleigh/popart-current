/*
 * testgraphs.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <exception>
using namespace std;

#include "networks/Graph.h"
#include "networks/MinSpanNet.h"
#include "networks/MedJoinNet.h"
#include "networks/HapNet.h"
#include "networks/NetworkError.h"
#include "networks/ParsimonyNet.h"
#include "networks/IntNJ.h"
#include "networks/TCS.h"
#include "networks/TightSpanWalker.h"
#include "seqio/Sequence.h"
#include "seqio/NexusParser.h"
#include "tree/Tree.h"
#include "tree/ParsimonyTree.h"
#include "tree/TreeError.h"
#include "tree/ParsimonyNode.h"


int main(int argc, char **argv)
{
  
  cout << "testing exception class NetworkError" << endl;
  
  try
  {
    throw NetworkError();
  }
  catch (NetworkError ne)
  {
    cout << "network error was caught." << endl;
    cout << "message: " << ne.what() << endl;
  }

  if (argc < 2)
  {
    cerr << "Usage: testgraphs nexfile.nex" << endl;
    return 1;
  }
  ifstream infile(argv[1]);

  cout << "Opened file " << argv[1] << endl;
  //cout << "Before loop, first char in file: " << infile.peek() << endl;

  vector<Sequence*> seqvect;
  Sequence *seqptr;
  
  srandom(time(0));
  HapNet *network;

  while (infile.good())
  {
    //cout << "top of loop, peeking: " << (char)(infile.peek()) << endl;
    seqptr = new Sequence();
    infile >> (*seqptr);
    seqvect.push_back(seqptr);
    //cout << (*seqptr) << endl;
  }

  infile.close();
  
  network = new TCS(seqvect);
  
  cout << "TCS:\n" << *network << endl;
  
  delete network;
  
  network = new TightSpanWalker(seqvect);
  
  cout << "Tight Span Walker:\n" << *network << endl;


  delete network;
  //return 0;
  
  network = new IntNJ(seqvect);
  
  cout << "IntNJ:\n" << *network << endl;

  /*for (unsigned i = 1; i < seqvect.size(); i++)
  {
    cout << "Path from seq1 to seq" << (i + 1) << ":";
    TCSPlus::PathIterator pathIt = network->beginPath(network->vertex(0), network->vertex(i));
    
    while (pathIt != network->endPath())
    {
      cout << " " << (*pathIt)->index() << ": " << (*pathIt)->label();
      ++pathIt;
    }
    
    cout << "\nPath length: " << network->pathLength(network->vertex(0), network->vertex(i)) << endl;
  }*/
  
  //return 0;
  
  /*if (argc > 2)
  {
    ifstream infile2(argv[2]);
    
    cout << "Opened tree file " << argv[2] << endl;
  
    
    infile2 >> t;
    infile2.close();
    
    cout << "Read tree:\n" << t << endl;
    
    cout << "tree has " << t.nleaves() << " leaves." << endl;
    
    try {
      t.setLeafSequences(seqvect); } catch (TreeError *te) {
        cerr << "what: " << te->what() << endl;
        throw;
        //cerr << "definitely a tree error." << endl;
        //throw;
      } 
      
     cout <<  t.calculateScore() << endl;
  
     /*Tree::Iterator nit = t.begin();
  
     while (nit != t.end())
     {
       cout << *nit << endl;
       ++nit;
     }*/
     
   /* cout << "Testing NucleotideComparitor class." << endl;
    
    
    for (unsigned short i = 0; i < 1 << 4; i++)
    {
      ParsimonyNode::NucleotideComparitor ncI(i);
      for (unsigned short j = 0; j < i; j++)
      {
        ParsimonyNode::NucleotideComparitor ncJ(j);
        cout << ParsimonyNode::nuc2char(i) << " == " << ParsimonyNode::nuc2char(j) << "? " << (ncI == ncJ) << endl;
        cout << ParsimonyNode::nuc2char(i) << " != " << ParsimonyNode::nuc2char(j) << "? " << (ncI != ncJ) << endl;
        cout << ParsimonyNode::nuc2char(i) << " < " << ParsimonyNode::nuc2char(j) << "? " << (ncI < ncJ) << endl;
        cout << ParsimonyNode::nuc2char(i) << " > " << ParsimonyNode::nuc2char(j) << "? " << (ncI > ncJ) << endl;
        cout << ParsimonyNode::nuc2char(i) << " <= " << ParsimonyNode::nuc2char(j) << "? " << (ncI <= ncJ) << endl;
        cout << ParsimonyNode::nuc2char(i) << " >= " << ParsimonyNode::nuc2char(j) << "? " << (ncI >= ncJ) << endl;
      }
    }*/
     
    //return 0;
    
  //}
  
  
  cout << "seqvect size: " << seqvect.size() << endl;

  for (vector<Sequence*>::iterator seqit = seqvect.begin(); seqit != seqvect.end(); ++seqit)
  {
    cout << (*seqit)->name() << endl;
  }


  network = //new MinSpanNet(seqvect, false, 5);
  new MedJoinNet(seqvect, 0);

  
  /*for (int i = 0; i < network->nodeCount(); i++)
    for (int j = 0; j < i; j++)
      cout << "distance(" << i << "," << j << "): " << network->pathLength(network->node(i), network->node(j)) << endl;
  */
  cout << "Median Joining network:\n" << (*network) << endl;
  
  if (argc > 2)
  {
    //ifstream infile2(argv[2]);
    
    vector<const Sequence *> condensed;
    vector<unsigned> weights;
    
    bool keepgoing = true;
    unsigned idx = 0;
    cout << "weights:";
    while (keepgoing)
    {
      try { 
        unsigned w = network->weight(idx++); 
        weights.push_back(w);
        cout << " " << w;
      } catch (NetworkError *ne) {
        keepgoing = false;
      }
    }
    
    cout << endl;
    
    for (unsigned i = 0; i < seqvect.size(); i++)  
    {
      Sequence *cs = new Sequence(network->seqName(i, true), network->seqSeq(i, true));
      cout << "Sequence " << cs->name() << " length: " << cs->length() << endl;
      condensed.push_back(cs);
    }
    cout << "Number of weights: " << weights.size() << endl;

    //cout << "Opened tree file " << argv[2] << endl;
    
    unsigned treecount = 0;
    
    NexusParser *  parser = dynamic_cast<NexusParser *>(Sequence::parser());
    cout << "Number of trees stored in parser: " << parser->treeVector().size() << endl;
    
    vector<Tree *>::const_iterator treeit = parser->treeVector().begin();
    
    vector<ParsimonyTree *> treevect;
    
    while (treeit != parser->treeVector().end())
    {
    
    
    //ParsimonyTree *plaintree = 0;

    /*while (infile2.good())
    {
  
    string whitespace = " \t\n\r";
    string line;
    infile2 >> line;
    
    cout << "a single line: " << line;
    
    // trim whitespace
    line.erase(line.find_last_not_of(whitespace) + 1);
    line.erase(0, line.find_first_not_of(whitespace));
    
    
    if (line.length() == 0)  continue;

    istringstream iss(line);//(infile2.getline());*/
    ParsimonyTree  *t = new ParsimonyTree(**treeit);
    cout << "copied tree: " << *t << endl;
   // iss >> (*t);
    
    cout << "Read tree " << ++treecount << ":\n" << (*t) << endl;
    
    cout << "tree has " << t->nleaves() << " leaves." << endl;
    
    treevect.push_back(t);
    
    /*if (! plaintree)
    {
      plaintree = new ParsimonyTree(*t);
      
      cout << "Copied tree." << endl;
      cout << *t << endl;
      cout << *plaintree << endl;
      
      //exit(0);
    }*/
    
    // Uncomment this block to calculate score here (and fix references to t)
    /*
    try {
      t.setLeafSequences(condensed, weights); } catch (TreeError *te) {
        cerr << "what: " << te->what() << endl;
        throw;
        //cerr << "definitely a tree error." << endl;
        //throw;
      } 
      
      
     unsigned score = t.computeScore();
     
     cout << "##############################################" << endl;
     cout <<  "Score for this tree: " << score << endl;
     cout << "##############################################" << endl;
 
      */
    ++treeit;
    }
    
    // infile2.close();
     
     try
     {
     HapNet *pnet = new ParsimonyNet(seqvect, treevect);
     cout << "Parsiomony Network:\n" << (*pnet) << endl;
     
     delete pnet;
      }
     catch (TreeError *)
     {
       cout << "tree error with parsimony network." << endl;
     }
     
    
     for (unsigned i = 0; i < treevect.size(); i++)
     {
       delete treevect.at(i);
       //cout << "tree " << i << ": " << (*(treevect.at(i))) << endl;
     }
  }
  
 

  for (int i = 0; i < seqvect.size(); i++)
  {

    //cout << (*(seqvect.at(i))) << endl;
    delete seqvect.at(i);
  }

  return 0;
  cout << "Hello world." << endl;

  Graph *g = new Graph;

  vector<string> names;

  names.push_back("node A");
  names.push_back("node B");
  names.push_back("node C");
  names.push_back("node D");
  names.push_back("node E");


  g->newVertex("node A");//names.at(0));
  g->newVertex("node B"); // names.at(1));
  g->newVertex(names.at(2));
  g->newVertex(names.at(3));

  g->newEdge(g->vertex(0), g->vertex(1));

  cout << (*g) << endl;

  g->removeEdge(0);
  g->removeVertex(1);

  g->newVertex(names.at(4));
  g->newEdge(g->vertex(1), g->vertex(3), 0.91);

  try
  {
    Vertex *u = g->vertex(0);
    Vertex *v = g->vertex(3);
    g->newEdge(u, v);
  }
  catch (exception * e)
  {
	cerr << "Caught an exception, rethrowing." << endl;
	cerr << e->what() << endl;
	throw;
  }

  cout << (*g) << endl;

  cout << "Iteration of edges of " << *(g->vertex(3)) << endl;

  Vertex::EdgeIterator eit = g->vertex(3)->begin();
  while (eit != g->vertex(3)->end())
  {
    cout << **eit << endl;
    ++eit;
  }

  cout << "DFS traversal:" << endl;

  Graph::DFSIterator dfsit = g->beginDFS();

  while (dfsit != g->endDFS())
  {
    cout << **dfsit << endl;
    ++dfsit;
  }

  delete g;

  g = new Graph();

  g->newVertex("node F"); //5
  g->newVertex("node A"); //0
  g->newVertex("node B"); //1
  g->newVertex("node C"); //2
  g->newVertex("node D"); //3
  g->newVertex("node E"); //4
  g->newVertex("node G"); //6
  g->newVertex("node H"); //7
  g->newVertex("node I"); //8

  g->newEdge(g->vertex(1), g->vertex(2));
  g->newEdge(g->vertex(1), g->vertex(3));
  g->newEdge(g->vertex(2), g->vertex(8));
  g->newEdge(g->vertex(2), g->vertex(4));
  g->newEdge(g->vertex(3), g->vertex(4));
  g->newEdge(g->vertex(4), g->vertex(5));
  g->newEdge(g->vertex(5), g->vertex(0));
  g->newEdge(g->vertex(5), g->vertex(6));
  g->newEdge(g->vertex(5), g->vertex(7));
  g->newEdge(g->vertex(0), g->vertex(6));

  cout << "new graph: " <<endl;
  cout << (*g) << endl;

  dfsit = g->beginDFS();

  while (dfsit != g->endDFS())
  {
    cout << **dfsit << endl;
    ++dfsit;
  }
  
  cout << "BFS traversal:" << endl;
  Graph::BFSIterator bfsit = g->beginBFS();
  
  while (bfsit != g->endBFS())
  {
    cout << **bfsit << endl;
    ++bfsit;
  }
  
  for (int i = 0; i < g->vertexCount(); i++)
    for (int j = 0; j < i; j++)
    {
     Vertex *u = g->vertex(i);
     Vertex *v = g->vertex(j);
      cout << "distance(" << u->label() << "," << v->label() << "): " << g->pathLength(u, v) << endl;
    }
  

  delete g;

  return 0;
}


