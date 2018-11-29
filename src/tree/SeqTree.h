#ifndef SEQTREE_H_
#define SEQTREE_H_

#include "Tree.h"
#include "TreeNode.h"
#include "SeqNode.h"
#include "../seqio/Sequence.h"

#include <vector>
#include <map>

class SeqTree : public Tree
{
public:
  SeqTree();
  SeqTree(const Tree &);
  
  virtual ~SeqTree();
  
  void setLeafSequences(const std::vector<const Sequence *> &, const std::vector<unsigned> & = std::vector<unsigned>());
  void setLeafSequences(const std::map<std::string, const Sequence *> &, const std::vector<unsigned> & = std::vector<unsigned>());
  unsigned nchar() const;
  unsigned weight(unsigned) const;
  

private:
  virtual TreeNode * newNode() const;
  unsigned _nchar;
  const std::vector<unsigned> *_weights;
  
  
};

#endif
