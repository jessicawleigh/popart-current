#ifndef SEQNODE_H_
#define SEQNODE_H_


#include "TreeNode.h"
#include "../seqio/Sequence.h"


class SeqNode : public TreeNode
{
public:
  SeqNode(const Sequence * = NULL, const std::string & = "", double = -1);
  virtual ~SeqNode();
  
  virtual TreeNode* newNodeVirtual(const std::string & = "", double = -1) const;
  void setSequence(const Sequence *);
  const Sequence * sequence() const;
  //virtual char & at(size_t);
  virtual const char & at(size_t) const;
  
//protected:
  const Sequence * sequence();

private:
  const Sequence * _seq;
  
};


#endif

