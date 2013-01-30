#include "TreeNode.h"
#include "TreeError.h"

#include <cstring>
using namespace std;

TreeNode::TreeNode(const string & label, double brlen)
{
  _label = label;
  _brlen = brlen;
  
  _depth = 0;
  _leafRank = -1;
  _in = _out = this;
}

/* 
 * Wrapper function around constructor so that correct type will be 
 * in, e.g., addChild
 */
TreeNode* TreeNode::newNodeVirtual(const string & label, double brlen) const
{
  return new TreeNode(label, brlen);
}

TreeNode::~TreeNode()
{ }

TreeNode * TreeNode::addChild(const string & label, double brlen)
{
  TreeNode *child = newNodeVirtual(label, brlen);
  
  child->setOut(this);
  setOut(child);
  
  return child;
}

const TreeNode * TreeNode::in() const
{
  return _in;
}



const TreeNode * TreeNode::out() const
{
  return _out;
}

TreeNode * TreeNode::in() 
{
  return _in;
}

TreeNode * TreeNode::out() 
{
  return _out;
}

bool TreeNode::isLeaf() const
{
  return _in == this;
}

const string & TreeNode::label() const
{
  return _label;
}

double TreeNode::brLen() const
{
  return _brlen;
}

double TreeNode::depth() const
{
  return _depth;
}

double TreeNode::leafRank() const
{
  return _leafRank;
}

double TreeNode::maxLeafRank() const
{
  return _maxLeafRank;
}

double TreeNode::minLeafRank() const
{
  return _minLeafRank;
}

void TreeNode::setIn(TreeNode *node)
{
  _in = node;
}

void TreeNode::setOut(TreeNode *node)
{
  _out = node;
}

void TreeNode::setLabel(const string & l)
{
  _label = l;
}

void TreeNode::setBrLen(double bl)
{
  _brlen = bl;
}

void TreeNode::updateDepth(double depth)
{
  TreeNode *p = _in;
  
  _depth = depth;
  
  while (p != this)
  {
    p->out()->updateDepth(_depth + p->brLen());
    p = p->in();
  }
}

void TreeNode::setDepth(double d)
{
  _depth = d;
}

void TreeNode::clear()
{  

  TreeNode *p = in();
  TreeNode *q;
  
  //TreeNode *last = p;
  
  //while (last->in() != p)  last = last->in();

  while (p != this)
  {
    q = p->out();
    q->clear();
    q = p;
    p = p->in();
    delete q;
  } //while (p != node);
  
  //q = node->out();
  //clear(q);
  delete this;

}


void TreeNode::setLeafRank(double minrank, double maxrank, double rank)
{
  _leafRank = rank;
  _minLeafRank = minrank;
  _maxLeafRank = maxrank;
}

void TreeNode::updateLeafRank(unsigned & leafCount)
{
  if (this == _in)
  {
    _leafRank = leafCount;
    leafCount++;
  }
  
  else
  {
    _minLeafRank = -1;
    _maxLeafRank = _minLeafRank;
    TreeNode *p = _in;
    
    while (p != this)
    {
      p->out()->updateLeafRank(leafCount);
      
      if ((_minLeafRank < 0) || (p->out()->leafRank() < _minLeafRank))
      {
        _minLeafRank = p->out()->leafRank();
      }
      if (_maxLeafRank < p->out()->leafRank())
      {
        _maxLeafRank = p->out()->leafRank();
      }
      p = p->in();
    }
    
    _leafRank = (_maxLeafRank + _minLeafRank) / 2.0;    
  }
}

TreeNode::ChildIterator TreeNode::childrenBegin()
{
  return ChildIterator(this);
}

TreeNode::ChildIterator TreeNode::childrenEnd()
{
  return ChildIterator(this, true);
}

TreeNode::ChildIterator::ChildIterator(TreeNode *parent, bool end)
{
  _end = end;
  p = parent;
  
  if (p->in() == p)  
  {
    _end = true;
    q = NULL;
  }
  else  q = p->in();
}

TreeNode::ChildIterator::~ChildIterator()
{}

TreeNode::ChildIterator & TreeNode::ChildIterator::operator++()
{
  q = q->in();
  
  if (q == p)  
  {
    _end = true;
    q = NULL;
  }
  
  return *this;
}

TreeNode* TreeNode::ChildIterator::operator*() const
{
  return q->out();
}

bool TreeNode::ChildIterator::operator==(const ChildIterator & other) const
{
  if (_end && other._end)  return true;
  
  if (_end != other._end)  return false;
  
  return (p == other.p && q == other.q);
}

bool TreeNode::ChildIterator::operator!=(const ChildIterator & other) const
{
  return ! operator==(other);
}

ostream & operator<<(ostream & output, const TreeNode & node)
{
  if (node.in() == &node)  output << node.label();
  else
  {
    output << '(';
    const TreeNode *p = node.in();
    
    while (p != &node)
    {
      output << *(p->out());
      if (p->in() != &node)  output << ',';
      p = p->in();
    }
    
    output << ')';
    if (! p->label().empty())  output << p->label();
  }
  
  if (node.brLen() >= 0)  output << ':' << node.brLen();
  
  return output;
}
