#include "ParsimonyTree.h"
#include "SeqNode.h"
#include "../seqio/Sequence.h"
#include "Tree.h"
#include "TreeError.h"

#include <limits>
#include <cstdlib>
#include <iostream>
using namespace std;

ParsimonyTree::ParsimonyTree()
  : SeqTree()
{
  _score = 0;
   
}


// TODO make this function copy other important ParsimonyNode attributes
ParsimonyTree::ParsimonyTree(const Tree & other)
  : SeqTree()
{
  _score = 0;  

  copyRoot(other.root());
  if (! other.root()->isLeaf())
    copyTree(root()->out(), other.root()->out());    
  
  copyTree(root(), other.root());
}

ParsimonyTree::~ParsimonyTree()
{}

TreeNode * ParsimonyTree::newNode() const
{
  return new ParsimonyNode();
}

unsigned ParsimonyTree::computeScore()
{
  if (! root())  return 0;
  
  
  SankoffUp();
  
  return _score;
}

// When an ambiguous character is encountered, set it randomly to an acceptable value
void ParsimonyTree::computeAncestors()
{
  SankoffDown(dynamic_cast<ParsimonyNode *>(root()), false);
}

// Allow ambiguous sequences
void ParsimonyTree::computeAmbiguousAncestors()
{
  SankoffDown(dynamic_cast<ParsimonyNode *>(root()), true);
}

void ParsimonyTree::SankoffUp()
{
  ParsimonyNode *pnode;
  Iterator nit = begin();
  
  
  while (nit != end())
  {
    pnode = dynamic_cast<ParsimonyNode *>(*nit);
    
    // case 1: pnode is a leaf
    if (pnode->isLeaf())
    {
      const Sequence *seq = pnode->sequence();

      for (unsigned i = 0; i < nchar(); i++)
      {
        char c = (*seq)[i];
        switch(c)
        {
          case 'A':
          case 'a':
            pnode->setCost(i, ParsimonyNode::A, 0);
            pnode->setCost(i, ParsimonyNode::G, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::C, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::T, numeric_limits<unsigned>::max());
            break;
          case 'G':
          case 'g':
            pnode->setCost(i, ParsimonyNode::A, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::G, 0);
            pnode->setCost(i, ParsimonyNode::C, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::T, numeric_limits<unsigned>::max());
            break;
          case 'C':
          case 'c':
            pnode->setCost(i, ParsimonyNode::A, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::G, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::C, 0);
            pnode->setCost(i, ParsimonyNode::T, numeric_limits<unsigned>::max());
            break;
          case 'T':
          case 't':
          case 'U':
          case 'u':
            pnode->setCost(i, ParsimonyNode::A, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::G, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::C, numeric_limits<unsigned>::max());
            pnode->setCost(i, ParsimonyNode::T, 0);
           break;
          case '-':
            pnode->setCost(i, ParsimonyNode::A, 0);
            pnode->setCost(i, ParsimonyNode::G, 0);
            pnode->setCost(i, ParsimonyNode::C, 0);
            pnode->setCost(i, ParsimonyNode::T, 0);
            break;
          default:
            throw TreeError("Unknown character state.");
        }
      }
    }
    
    else // if pnode is not a leaf
    {
      for (unsigned i = 0; i < nchar(); i++)
      {
        for (unsigned j = 0; j < pnode->alphabetSize(); j++)
        {
          unsigned cost = 0;
          TreeNode::ChildIterator cit = pnode->childrenBegin();
          
          while (cit != pnode->childrenEnd())
          {
            unsigned costForChild;
            ParsimonyNode *child = dynamic_cast<ParsimonyNode *>(*cit);
            costForChild = child->minCost(i, ParsimonyNode::nucleotides[j]);
            ++cit;
            cost += costForChild;
            
          }
          
          if (isRoot(pnode))
          {
            unsigned costForChild;
            ParsimonyNode *child = dynamic_cast<ParsimonyNode *>(root()->out());
            costForChild = child->minCost(i, ParsimonyNode::nucleotides[j]);
            cost += costForChild;
          }
          
          pnode->setCost(i, ParsimonyNode::nucleotides[j], cost);
        }    
      }
    }
    
    ++nit;
  }
  
  _score = 0;
  pnode = dynamic_cast<ParsimonyNode*>(root());
  
  for (unsigned i = 0; i < nchar(); i++)
  {
    unsigned w = weight(i);
    unsigned costi = pnode->cost(i, ParsimonyNode::A);
    unsigned costij;
    for (unsigned j = 1; j < pnode->alphabetSize(); j++)
    {
      costij = pnode->cost(i, ParsimonyNode::nucleotides[j]);
      if (costij < costi)  costi = costij;
    }
    _score += w * costi;
  }
}



void ParsimonyTree::SankoffDown(ParsimonyNode *node, bool allowAmbiguous, const vector<ParsimonyNode::NucleotideComparitor> * ancestor)
{
  
  _ancestorsAmbiguous = allowAmbiguous;
  
  if (node->isLeaf())  
    return;

  
  

  if (node->parsimonySeq() == 0 || node->parsimonySeq()->size() != nchar())  
    node->resizeSeq(nchar());
  
  
  unsigned steps = 0;
  for (unsigned i = 0; i < nchar(); i++)
  {
    
    if (ancestor)
    {
      node->stateAt(i) = ParsimonyNode::O;
      
      for (unsigned j = 0; j < node->alphabetSize(); j++)
      {
        // If this is a possible ancestral state, calculate conditional state(s) for node
        if (ancestor->at(i).uShortValue() & ParsimonyNode::nucleotides[j])
        {
           unsigned minCost = numeric_limits<unsigned>::max();
          unsigned short stateJ;

          for (unsigned k = 0; k < node->alphabetSize(); k++)
          {
            unsigned costK = node->cost(i, ParsimonyNode::nucleotides[k]);
            if (costK < numeric_limits<unsigned>::max())  costK +=
            node->substitutionCost(ParsimonyNode::nucleotides[j], ParsimonyNode::nucleotides[k]);
            
            if (costK < minCost)
            {
              minCost = costK;
              stateJ = ParsimonyNode::nucleotides[j];
            }
            
            else if (costK == minCost)
            {
              stateJ = stateJ | ParsimonyNode::nucleotides[k];
            }
          }

          node->stateAt(i).setUShortValue(node->stateAt(i).uShortValue() | stateJ);
        }
      }
    }
    
    else // This is the root, there is no ancestral sequence to consider
    {   
      unsigned minCost = numeric_limits<unsigned>::max();

      for (unsigned j = 0; j < node->alphabetSize(); j++)
      {
        unsigned costJ = node->cost(i, ParsimonyNode::nucleotides[j]);
                
        if (costJ < minCost)
        {
          minCost = costJ;
          node->stateAt(i).setUShortValue(ParsimonyNode::nucleotides[j]);
          
        }
        
        else if (costJ == minCost)
        {
          node->stateAt(i).setUShortValue(node->stateAt(i).uShortValue() | ParsimonyNode::nucleotides[j]);
        }
      }
    }
    
    if (! allowAmbiguous)
    {
      vector<ParsimonyNode::Nucleotide> validStates;
      for (unsigned j = 0; j < node->alphabetSize(); j++)
        if (node->stateAt(i).uShortValue() & ParsimonyNode::nucleotides[j])
        validStates.push_back(ParsimonyNode::nucleotides[j]);
      
      //unsigned u = random();
      node->stateAt(i).setUShortValue(validStates.at(rand() % validStates.size()));
      
      //cout << "random number: " << u << endl;
      
      if (ancestor)
      {
        if  (node->stateAt(i).uShortValue() != ancestor->at(i).uShortValue())  steps++;
      }
      
    }
  }
  
  if (! allowAmbiguous) 
  {
    string s(nchar(), '-');
    
    for (unsigned i = 0; i < nchar(); i++)  
      s.at(i) = node->stateAt(i).charValue();
    Sequence *seq = new Sequence("", s);
    node->setSequence(seq);
    
    if (ancestor)  node->setBrLen(steps);
  }
  
  if (isRoot(node)) 
  {
    ParsimonyNode *child = dynamic_cast<ParsimonyNode *>(node->out());

    if (child->isLeaf())
    {
      if (! allowAmbiguous)
      {
        unsigned steps = 0;
        
        for (unsigned i = 0; i < nchar(); i++)  
        {
          if (child->stateAt(i).uShortValue() != node->stateAt(i).uShortValue())  steps++;
        }
        
        child->setBrLen(steps);
      }
    }

    else  SankoffDown(child, allowAmbiguous, node->parsimonySeq());
    
  }
  
  TreeNode::ChildIterator cit = node->childrenBegin();  
          
  while (cit != node->childrenEnd())
  {
    ParsimonyNode *child = dynamic_cast<ParsimonyNode *>(*cit);
    
    if (child->isLeaf())
    {
      if (! allowAmbiguous)
      {
        unsigned steps = 0;
        
        for (unsigned i = 0; i < nchar(); i++)  
        {
          if (child->stateAt(i).uShortValue() != node->stateAt(i).uShortValue())  steps++;
        }
        
        child->setBrLen(steps);
      }
    }
    
    else  SankoffDown(child, allowAmbiguous, node->parsimonySeq());
    ++cit;
  }
}

ParsimonyTree::EdgeIterator ParsimonyTree::edgeBegin()
{
  return EdgeIterator(this);
}

ParsimonyTree::EdgeIterator ParsimonyTree::edgeEnd()
{
  return EdgeIterator(this, true);
}

ParsimonyTree::EdgeIterator::EdgeIterator(ParsimonyTree *tree, bool end)
  : _treeIt(tree, end)
{
  _end = end;
  _tree = tree;

  if (end)
  {
    first = second = 0;
    brLen = 0;
  }
  
  else
  {
    first = dynamic_cast<ParsimonyNode *>(*_treeIt);
    second = dynamic_cast<ParsimonyNode *>((*_treeIt)->out());
    brLen = (*_treeIt)->brLen();
  }

}

ParsimonyTree::EdgeIterator::~EdgeIterator()
{}

ParsimonyTree::EdgeIterator & ParsimonyTree::EdgeIterator::operator++()
{
  do {
    ++_treeIt;
    if (_treeIt == _tree->end())    _end = true;
  }  while (!_end &&  _tree->isRoot(*_treeIt)); 
  
  if (_end)
  {
    _end = true;
    first = second = 0;
    brLen = 0;
  }
  
  else
  {
    first = dynamic_cast<ParsimonyNode *>(*_treeIt);
    second = dynamic_cast<ParsimonyNode *>((*_treeIt)->out());
    brLen = (*_treeIt)->brLen();
  }
    
  return *this;
}

bool ParsimonyTree::EdgeIterator::operator==(const EdgeIterator &other) const
{
  if (_end && other._end)  return true;
  else if (_end || other._end)  return false;
  
  else  return (*_treeIt == *(other._treeIt));
}

bool ParsimonyTree::EdgeIterator::operator!=(const EdgeIterator &other) const
{
  return ! operator==(other);
  
}
