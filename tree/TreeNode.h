#ifndef TREENODE_H_
#define TREENODE_H_

#include <iostream>
#include <string>

class TreeNode
{
public:
  TreeNode(const std::string & = "", double = -1);
  virtual ~TreeNode();
  
  virtual TreeNode* newNodeVirtual(const std::string & = "", double = -1) const;
  void clear();
  TreeNode * addChild(const std::string & = "", double = -1);
  TreeNode * in();
  TreeNode * out();
  const TreeNode * in() const;
  const TreeNode * out() const;
  bool isLeaf() const;
  const std::string & label() const;
  double brLen() const;
  double depth() const;
  double leafRank() const;
  double minLeafRank() const;
  double maxLeafRank() const;
  void setIn(TreeNode *);
  void setOut(TreeNode *);
  void setLabel(const std::string &);
  void setDepth(double);
  void setBrLen(double);
  void updateDepth(double);
  void setLeafRank(double, double, double);
  void updateLeafRank(unsigned &);
  
  
  class ChildIterator
  {
  public:
    ChildIterator(TreeNode *, bool = false);
    virtual ~ChildIterator();
    ChildIterator & operator++();
    TreeNode *operator*() const;
    bool operator!=(const ChildIterator &) const;
    bool operator==(const ChildIterator &) const;
    
  private:
    TreeNode *p, *q;
    bool _end;
  };
  
  ChildIterator childrenBegin();
  ChildIterator childrenEnd();
  
  friend std::ostream &operator<<(std::ostream &, const TreeNode &);
 
protected:
  /*TreeNode *in();
  TreeNode *out();*/
private:
  TreeNode *_in;
  TreeNode *_out;
  double _brlen;
  double _depth;
  double _leafRank;
  double _minLeafRank;
  double _maxLeafRank;
  std::string _label;
};


#endif

