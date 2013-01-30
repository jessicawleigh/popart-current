#include "SeqNode.h"
#include "TreeError.h"

#include <iostream>
using namespace std;

SeqNode::SeqNode(const Sequence *seq, const string & label, double brlen)
  : TreeNode(label, brlen)
{
  
  _seq = seq;
}

SeqNode::~SeqNode()
{}

TreeNode* SeqNode::newNodeVirtual(const string & label, double brlen) const
{
   return new SeqNode(NULL, label, brlen); 
}

void SeqNode::setSequence(const Sequence *seq)
{
  _seq = seq;
}

/* I've set this up to allow one sequence per cluster of internal nodes.
 * This may not be sensible if, e.g., nodes across an edge should point
 * to the same sequence.
 */
const Sequence * SeqNode::sequence()
{
  
  SeqNode *p = dynamic_cast<SeqNode *>(in());
  
  while (_seq == NULL && p != this)
  {
    _seq = p->_seq;
    p->_seq = NULL;
    p = dynamic_cast<SeqNode *>(p->in());
  }
  
  return _seq;
}


/* See note on mutable version of this function.
 */
const Sequence * SeqNode::sequence() const
{
  
  if (_seq)  return _seq;
  
  const SeqNode *p = dynamic_cast<const SeqNode *>(in());
  
  while (p->_seq == NULL && p != this)
    p = dynamic_cast<const SeqNode *>(p->in());
  
  return p->_seq;
}

const char & SeqNode::at(size_t pos) const
{
  const Sequence *s = sequence();
  if (pos >= s->length())  throw TreeError("Sequence index out of range.");
  
  return s->at(pos);
}  



