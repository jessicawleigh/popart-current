#include "SeqTree.h"
#include "TreeError.h"

#include <map>
#include <iostream>
using namespace std;

SeqTree::SeqTree()
  : Tree()
{
  _nchar = 0;
  _weights = 0;
}

SeqTree::SeqTree(const Tree & other)
  : Tree()
{
  // TODO copy other node attributes that aren't covered in TreeNode
  _nchar = 0;
  _weights = 0;

  copyRoot(other.root());
  if (! other.root()->isLeaf())
    copyTree(root()->out(), other.root()->out());
  
  copyTree(root(), other.root());
}



SeqTree::~SeqTree()
{}

TreeNode * SeqTree::newNode() const
{
  return new SeqNode();
}

void SeqTree::setLeafSequences(const vector<const Sequence *> & seqvect, const vector<unsigned> & weights)
{
  if (seqvect.size() != nleaves())
    throw TreeError("Number of sequences is not equal to number of leaves!");
  
  
  map<string, const Sequence*> seqmap;
  
  for (vector<const Sequence *>::const_iterator seqit = seqvect.begin(); seqit != seqvect.end(); ++seqit)
    seqmap[(*seqit)->name()] = *seqit;
  
  setLeafSequences(seqmap, weights);
} 

void SeqTree::setLeafSequences(const map<string, const Sequence *> & seqmap, const vector<unsigned> & weights)
{
  
  map<string, const Sequence*>::const_iterator seqit;
  
  SeqNode * rootnode = dynamic_cast<SeqNode *>(root());
  if (!rootnode)
  {
    cerr << "root either isn't set or can't be cast to SeqNode. This is a problem." << endl;
  }
  
  for (LeafIterator lit = leafBegin(); lit != leafEnd(); ++lit)
  {
    SeqNode *seqnode = dynamic_cast<SeqNode *>(*lit);
    
    if (seqnode == NULL) 
      throw TreeError("Nodes can't be cast to SeqNode*!");
    
    seqit = seqmap.find(seqnode->label());
    if (seqit == seqmap.end())  
      throw TreeError("Node label not found!");
    
    if (lit == leafBegin())  _nchar = seqit->second->length();
    else  if (_nchar != seqit->second->length())
      throw TreeError("Sequences are of different lengths!");
    
    seqnode->setSequence(seqit->second);
  }
  
  if (weights.size() > 0 && weights.size() != _nchar) 
    throw TreeError("Weights vector is not the same length as sequences!");
    
  _weights = &weights;
}

unsigned SeqTree::nchar() const
{
  return _nchar;
}

unsigned SeqTree::weight(unsigned idx) const
{
  if (idx >= nchar())  throw TreeError("Weight index out of range.");
  
  if (_weights->size() > 0)
    return _weights->at(idx);
  
  return 1;
}
