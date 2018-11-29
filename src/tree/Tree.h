#ifndef TREE_H_
#define TREE_H_

#include "TreeNode.h"

#include <stack>
#include <string>
#include <iostream>



class Tree
{
public:
  class Iterator
  {
  public:
    Iterator(Tree*, bool = false);
    Iterator & operator++();
    TreeNode * operator*() const;
    bool operator==(const Iterator &) const;
    bool operator!=(const Iterator &) const;
  protected:
    bool isEnd() const;
    void nextNode();  
  private:
    void nextLeaf();
    std::stack<TreeNode*> _parents;
    TreeNode *p;
    bool _isEnd;
  };
  
  class LeafIterator : public Iterator
  {
  public:
    LeafIterator(Tree*, bool = false);
    LeafIterator & operator++();
    //TreeNode * operator*() const;
    //bool operator==(const Iterator &) const;
    //bool operator!=(const Iterator &) const;
  //private:
    //TreeNode *p;
  };
  
  Tree();
  Tree(const Tree &);
  virtual ~Tree();
  TreeNode * root() const;
  bool isRoot(TreeNode *) const;
  void reRoot(TreeNode*);
  void clear();
  void updateDepth();
  void updateLeafRank();
  unsigned nleaves() const;
  // reroot, others?

  Iterator begin() ;
  LeafIterator leafBegin(); 
  Iterator end() ;
  LeafIterator leafEnd();
  
  static Tree * parseTreeString(const std::string &);
  
 // virtual void virtfunc() const;


  friend std::ostream &operator<<(std::ostream &, const Tree &);
  friend std::istream &operator>>(std::istream &, Tree &);
  
protected:
  virtual void copyRoot(const TreeNode *);
  virtual void copyTree(TreeNode *, const TreeNode *);
  void setupRoot();

private: 
  
  virtual TreeNode* newNode() const;

  TreeNode * _root;
  unsigned _nleaves;
};


#endif

