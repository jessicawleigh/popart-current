#include <sstream>
using namespace std;

#include "Tree.h"
#include "TreeError.h"

Tree::Tree()
{
  _root = NULL;
  _nleaves = 0;
}

Tree::Tree(const Tree &other)
{
  _nleaves = 0;
  
  copyRoot(other.root());
  
  if (! other.root()->out()->isLeaf())
    copyTree(root()->out(), other.root()->out());
  
  copyTree(root(), other.root());
}

void Tree::copyRoot(const TreeNode *otherRoot)
{
  if (otherRoot)
  {
    setupRoot();
    root()->setLabel(otherRoot->label());
    root()->setBrLen(otherRoot->brLen());
  
    root()->addChild();
    root()->out()->setLabel(otherRoot->out()->label());
    root()->out()->setBrLen(otherRoot->out()->brLen());


    if (otherRoot->out()->isLeaf())  _nleaves++;

  }
}

// Note if calling this from derived classes that other attributes of nodes
// that do not exist in TreeNode will need to be added.
void Tree::copyTree(TreeNode *node, const TreeNode *otherNode)
{
  
  const TreeNode *p = otherNode->in();
  TreeNode *q = node;
  TreeNode *in;
  
  while  (p != otherNode)
  {
    in = newNode();
    in->setBrLen(p->brLen());
    in->setLabel(p->label());
    in->setIn(node);
    q->setIn(in);

    in->addChild();
    in->out()->setBrLen(p->out()->brLen());
    in->out()->setLabel(p->out()->label());
    
    //if (p->out()->isLeaf())  in->out()->setLabel(p->out->label());

    //else 
    if (p->out()->isLeaf())  _nleaves++;
    else  copyTree(in->out(), p->out());
    
    
    q = q->in();   
    p = p->in();
  }  
}

TreeNode * Tree::newNode() const
{
  //cout << "inside Tree::newNode (probably not right)" << endl;
  return new TreeNode();
}

void Tree::setupRoot()
{
  _root = newNode();
  //cout << "set up root." << endl;
}


Tree::~Tree()
{
  clear();
  delete _root;
  _root = NULL;
  _nleaves = 0;
}

/* Determine whether node is a member of the root "in" group. */
bool Tree::isRoot(TreeNode *node) const
{
  
  TreeNode *p = node;
  
  do
  {
    if  (p == _root)  return true;
    p = p->in();
    
  }  while (p != node);
  
  return false;
}

TreeNode * Tree::root() const
{
  return _root;
}

void Tree::reRoot(TreeNode* newroot)
{
  // check to make sure newroot is actually in tree!!!
  
  _root = newroot;
}

void Tree::clear()
{
  TreeNode *p = _root->in();
  TreeNode *q;
  
  if (_root->out() != _root)
    _root->out()->clear();
  
  while (p != _root)
  {
    p->out()->clear();
    q = p;
    p = p->in();

    delete q;
  }
  
  _root->setIn(_root);
  _root->setOut(_root);
  
}

void Tree::updateDepth()
{
  //_root->updateDepth(0);
  
  TreeNode *p = _root;
  
  do
  {
    p->out()->updateDepth(p->brLen());
    p = p->in();
    p->setDepth(0);
  } while (p != _root);
}

void Tree::updateLeafRank()
{
  _nleaves = 0;
  
  TreeNode *p = _root;
  
  double maxrank = -1 ;
  double minrank = maxrank;
  
  do
  {
    p->setLeafRank(-1, 0, 0);
    p->out()->updateLeafRank(_nleaves);

    if (minrank < 0 || minrank > p->out()->leafRank())
      minrank = p->out()->leafRank();
    if (maxrank < p->out()->leafRank())
      maxrank = p->out()->leafRank();
    p = p->in();
  } while (p != _root);
  
  p->setLeafRank(minrank, maxrank, (minrank + maxrank) / 2);
}

unsigned Tree::nleaves() const
{
  //if (_root && !_nleaves)  updateLeafRank();
  return _nleaves;
}

Tree::Iterator Tree::begin()
{
  return Iterator(this);
}

Tree::Iterator Tree::end()
{
  return Iterator(this, true);
}

Tree::LeafIterator Tree::leafBegin()
{
  return LeafIterator(this);
}

Tree::LeafIterator Tree::leafEnd()
{
  return LeafIterator(this, true);
}

Tree * Tree::parseTreeString(const string & tstr)
{
  Tree *t = new Tree();
  istringstream iss(tstr);
  
  iss >> *t;
  
  return t;
}

istream & operator>>(istream & input, Tree & t)
{
  string tstr;
  char c;
  
  if (t.root() == NULL)
    t.setupRoot();
  
  do 
  {
    input.get(c);
    tstr.push_back(c);
    
  } while (input.good() && c != ';');
  
  t.clear();
  TreeNode *current = t.root();
  TreeNode *in;
  bool inname = false;
  bool inbrlen = false;
  bool endtree = false;
  string name = "";
  string brlenstr = "";
  double brlen;
  bool treestart = true;
  
  istringstream iss;
  
  for (unsigned i = 0; i < tstr.length() && ! endtree; i++)
  {
    switch(tstr.at(i))
    {
      case ' ':
      case '\n':
      case '\r':
      case '\t':
      {
        continue;
        break;
      }
      
      case '(':
      {
        if (treestart)  treestart = false;
        else  
        {
          current = current->addChild();
          in = t.newNode();//new TreeNode();
          in->setIn(current);
          current->setIn(in);
          current = in;
        }
        break;
      }
      
      case ')':
      {
        if (inname)
        {
          //cout << "(1)at " << name << ", nleaves before: " << t.nleaves();
          if (current->out() == current)
          {
            current->addChild(name);
            if (current->out()->in() == current->out())
              (t. _nleaves)++;
          }

          else
          {
            current->setLabel(name);
            current->out()->setLabel(name);
            if (current->in() == current)
              (t. _nleaves)++;
          }


          //cout << " and after: " << t.nleaves() << endl;
          
          name.clear();  
          inname = false;
        }
        
        else if (inbrlen)
        {
          brlenstr.push_back(' ');
          iss.str(brlenstr);
          iss >> brlen;
          current->setBrLen(brlen);
          current->out()->setBrLen(brlen);
          brlenstr.clear();
          inbrlen = false;
        }
        
        current = current->in()->out();
        break;
      }  
      
      case ':':
      {
        inbrlen = true;
        if (inname)
        {
          if (current->out() == current)
            current->addChild(name);

          else
          {
            current->setLabel(name);
            current->out()->setLabel(name);
          }
          
          //cout << "(2)at " << name << ", nleaves before: " << t.nleaves();
          (t. _nleaves)++;
          //cout << " and after: " << t.nleaves() << endl;          
          name.clear();
          inname = false;
        }
        break;
      }
      
      case ',':
      {
        if (inname)
        {
          if (current->out() == current)
            current->addChild(name);

          else
          {
            current->setLabel(name);
            current->out()->setLabel(name);
            
          }
          
          //cout << "(3)at " << name << ", nleaves before: " << t.nleaves();
          (t. _nleaves)++;
          //cout << " and after: " << t.nleaves() << endl;          
          name.clear();
          inname = false;
        }
        
        else if (inbrlen)
        {
          brlenstr.push_back(' ');
          iss.str(brlenstr);
          iss >> brlen;
          current->setBrLen(brlen);
          current->out()->setBrLen(brlen);
          brlenstr.clear();
          inbrlen = false;          
        }
        
        in = t.newNode();
        in->setIn(current->in());
        current->setIn(in);
        current = in;
        
        break;
      }
      
      case ';':
      {
        endtree = true;
        break;
      }
      
      default:
      {
        if (inbrlen)  brlenstr.push_back(tstr.at(i));
        
        else
        {
          if (! inname)  inname = true;
          name.push_back(tstr.at(i));
        }
        break;
      }
    }
  }
  
  //return t;
  return input;
}


ostream & operator<<(ostream & output, const Tree & t)
{
  TreeNode *p = t.root();
  output << '(';
  
  //while (p->in() != t.root())
  do
  {
    output << *(p->out());
    if (p->in() != t.root())  output << ',';
    p = p->in();
  }  while (p != t.root());

  output << ");";

  return output;
}

Tree::Iterator::Iterator(Tree* t, bool end)
{
  p = t->root();
  _isEnd = end;
  if (! end)
  {
    _parents.push(p);
    p = p->out();
    nextLeaf();
  }
}


void Tree::Iterator::nextLeaf()
{
  while(p != p->in())
  {
    _parents.push(p); 
    p = p->in()->out();
  }
}

void Tree::Iterator::nextNode()
{
  if (! _isEnd)
  {
    if (p == _parents.top())
    {
      _parents.pop();
      if (_parents.empty())
        _isEnd = true;
    }
    
    if (! _isEnd)
    {
      p = p->out()->in();
    
      if (p != _parents.top())
      {
        p = p->out();
        nextLeaf();
      }
    }
  } 
}

Tree::Iterator & Tree::Iterator::operator++()
{
  nextNode();
  
  return *this;
}

TreeNode * Tree::Iterator::operator*() const
{
  if (_isEnd)  return 0;

  else return p;
}

bool Tree::Iterator::operator==(const Tree::Iterator & other) const
{
  if (_isEnd && other.isEnd())  return true;
  
  else if (_isEnd)  return false;
  
  else return p == (*other);
  
  //return ! (this != other);
}

bool Tree::Iterator::operator!=(const Tree::Iterator & other) const
{
  if (_isEnd && other.isEnd())  return false;
  
  else if (_isEnd)  return true;
  
  else return p != (*other);
  
  // return ! (this == other);
  
}

bool Tree::Iterator::isEnd() const
{
  return _isEnd;
}

Tree::LeafIterator::LeafIterator(Tree *t, bool end)
 : Tree::Iterator(t, end)
{ }

Tree::LeafIterator & Tree::LeafIterator::operator++()
{  
  
  nextNode();
  
  while (! isEnd() && (**this != (**this)->in()))
    nextNode();  
  
  return *this;
}


