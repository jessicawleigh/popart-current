#ifndef PARSIMONYTREE_H_
#define PARSIMONYTREE_H_

#include "ParsimonyNode.h"
#include "SeqTree.h"
#include "Tree.h"
#include "TreeNode.h"


class ParsimonyTree : public SeqTree
{
public:
  class EdgeIterator
  {
  public:
    EdgeIterator(ParsimonyTree *, bool=false);
    virtual ~EdgeIterator();
    EdgeIterator & operator++();
    bool operator!=(const EdgeIterator &) const;
    bool operator==(const EdgeIterator &) const;
    
    ParsimonyNode * first;
    ParsimonyNode * second;
    double brLen;
    
  private:
    Tree::Iterator _treeIt;
    bool _end;
    ParsimonyTree *_tree;
  };

  ParsimonyTree();
  ParsimonyTree(const Tree &);
  virtual ~ParsimonyTree();
  
  unsigned computeScore();
  void computeAmbiguousAncestors();
  void computeAncestors();
  
  EdgeIterator edgeBegin();
  EdgeIterator edgeEnd();
  
  
private:
  virtual TreeNode * newNode() const;
  
  void SankoffUp();
  void SankoffDown(ParsimonyNode *, bool = false, const basic_string<ParsimonyNode::NucleotideComparitor> * = NULL);
  
  
  unsigned long _score; 
  bool _ancestorsAmbiguous;
  
  
};


#endif
