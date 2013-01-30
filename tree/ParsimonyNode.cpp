#include "ParsimonyNode.h"
#include "TreeError.h"

#include <limits>
#include <iostream>
using namespace std;

const unsigned ParsimonyNode::_subCosts[] = { 0, 1, 1, 1,
                                              1, 0, 1, 1,
                                              1, 1, 0, 1, 
                                              1, 1, 1, 0 };

const char ParsimonyNode::nucleotideChars[] = {'A', 'G', 'C', 'T'};
const ParsimonyNode::Nucleotide ParsimonyNode::nucleotides[] = {ParsimonyNode::A, ParsimonyNode::G, ParsimonyNode::C, ParsimonyNode::T};

/* 
 * IUPAC codes indexed by bitwise operations on nucleotides 
 * 0:  -
 * 1:  A
 * 2:  G
 * 3:  A | G = Y
 * 4:  C
 * 5:  A | C = M
 * 6:  G | C = S
 * 7:  A | G | C = V
 * 8:  T
 * 9:  A | T = W
 * 10: G | T = K
 * 11: A | G | T = D
 * 12: C | T = R
 * 13: A | C | T = H
 * 14: G | C | T = B
 * 15: A | G | C | T = N
 */
const char ParsimonyNode::_nuc2chr[] = {'-', 'A', 'G', 'Y',
                                        'C', 'M', 'S', 'V', 
                                        'T', 'W', 'K', 'D', 
                                        'R', 'H', 'B', 'N'};

const short unsigned ParsimonyNode::_chr2nuc[] =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 14, 4, 11, 0, 0, 2, 13, 0, 0, 10, 0, 5, 15, 0, // 'A' = 65, 'B' = 66, 'C' = 67, 'D' = 68, 'G' = 71, 'H' = 72, 'K' = 75, 'M' = 77, 'N' = 78
    0, 0, 12, 6, 8, 8, 7, 9, 0, 3, 0, 0, 0, 0, 0, 0, // 'R' = 82, 'S' = 83, 'T' = 84, 'U' = 85, 'V' = 86, 'W' = 87, 'Y' = 89
    0, 1, 14, 4, 11, 0, 0, 2, 13, 0, 0, 10, 0, 5, 15, 0, // 'a' = 97, 'b' = 98, 'c' = 99, 'd'=100, 'g' = 103, 'h' = 104, 'k' = 107, 'm' = 109, 'n' = 110
    0, 0, 12, 6, 8, 8, 7, 9, 0, 3, 0, 0, 0, 0, 0, 0, // 'r' = 114, 's' = 115, 't' = 116, 'u' = 117, 'v'=118, 'w'=119, 'y' = 121
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


ParsimonyNode::ParsimonyNode(Sequence *seq, const string & label, double brlen)
  : SeqNode(seq, label, brlen)
{
  _stateCosts = NULL;
  
  if (seq)  setParsimonySeqFromSeq();
  
  else  _parsimonySeq = 0;
}

ParsimonyNode::~ParsimonyNode()
{ 
  
  if (_stateCosts)
  {
    vector<unsigned *>::iterator costit = _stateCosts->begin();
  
    while (costit != _stateCosts->end())
    {
      unsigned *sitecost = *costit;
      delete [] sitecost;
    
      ++costit;
    }
    
    delete _stateCosts;
  }
  
  if (_parsimonySeq)  delete _parsimonySeq;
}

TreeNode* ParsimonyNode::newNodeVirtual(const string & label, double brlen) const
{
   return new ParsimonyNode(NULL, label, brlen); 
}




const char & ParsimonyNode::at(size_t pos) const
{
  if (isLeaf())  return SeqNode::at(pos);
  
  if (pos >= _parsimonySeq->length())  throw TreeError("Sequence index out of range.");
  
 
  // NucleotideComparitor does this checking
  // 2 ^alphabetSize is number of redundant codes
  //if (c >= 1 << alphabetSize())  throw TreeError("Sequence character not a valid redundancy code");
  
  return _nuc2chr[_parsimonySeq->at(pos).uShortValue()];
}

ParsimonyNode::NucleotideComparitor & ParsimonyNode::stateAt(size_t pos)
{
  
  // Move _parsimonySeq around if necessary
  parsimonySeq();
  
  if (_parsimonySeq == 0)  throw TreeError("Attempt to access null parsimony sequence.");
 
  
  // NucleotideComparitor takes care of this
  // 2 ^alphabetSize is number of redundant codes
  /*if (_parsimonySeq->at(pos) >= 1 << alphabetSize())  
    throw TreeError("Sequence character not a valid redundancy code");*/
  
  return _parsimonySeq->at(pos);
}

const ParsimonyNode::NucleotideComparitor & ParsimonyNode::stateAt(size_t pos) const
{
  // Move _parsimonySeq around if necessary
  const ParsimonyNode *p = dynamic_cast<const ParsimonyNode*>(in());
  
  while (p->_parsimonySeq != 0 && p != this)
    p = dynamic_cast<const ParsimonyNode*>(in());
  
  if (p->_parsimonySeq == 0)  throw TreeError("Attempt to access null parsimony sequence.");
 
  // checked by NucleotideComparitor
  // 2 ^alphabetSize is number of redundant codes
  //if (p->_parsimonySeq->at(pos) >= 1 << alphabetSize())  
  //  throw TreeError("Sequence character not a valid redundancy code");
  
  return p->_parsimonySeq->at(pos);
}

void ParsimonyNode::resizeSeq(size_t newsize)
{
  parsimonySeq();
  
  if (_parsimonySeq == 0)
    _parsimonySeq = new basic_string<NucleotideComparitor>(newsize, (unsigned short)0);

  else
    _parsimonySeq->resize(newsize, (unsigned short)O);
}

unsigned ParsimonyNode::substitutionCost(Nucleotide src, Nucleotide dst)
{
  unsigned srcIdx = 0;
  unsigned dstIdx = 0;
  
  while (src >> srcIdx > 1)  srcIdx++;
  while (dst >> dstIdx > 1)  dstIdx++;
  
  return _subCosts[srcIdx * alphabetSize() + dstIdx];
}

char ParsimonyNode::nuc2char(short unsigned nucleotide)
{
  if (nucleotide >= 1 << _alphabetSize)  throw TreeError("Value out of range of alphabet size.");
  
  return _nuc2chr[nucleotide];
}

char ParsimonyNode::nuc2char(const NucleotideComparitor &nucleotide)
{
  return nuc2char(nucleotide.uShortValue());
}

const unsigned short & ParsimonyNode::char2nuc(const char & charval)
{
  if (charval == '-')  return _chr2nuc[charval];
  
  unsigned short ushortval = _chr2nuc[charval];
  if (ushortval == 0)  throw TreeError("Attempting to convert invalid character to nucleotide.");
  
  return _chr2nuc[charval];
}


void ParsimonyNode::setParsimonySeqFromSeq()
{
  const Sequence * seq = sequence();
  
  if (seq)
    _parsimonySeq = new basic_string<NucleotideComparitor>(seq->length(), (unsigned short)0);
    
  for (unsigned i = 0; i < seq->length(); i++)
    _parsimonySeq->at(i) = _chr2nuc[seq->at(i)];
  
  return;
  if (seq)
  {
    _parsimonySeq = new basic_string<NucleotideComparitor>(seq->length(), (unsigned short)0);


    for (unsigned i = 0; i < seq->length(); i++)
    {
      unsigned value = 0;
      switch(seq->at(i))
      {
        case 'A':
        case 'a':
          value = A;
          break;
        case 'G':
        case 'g':
          value = G;
          break;
        case 'C':
        case 'c':
          value = C;
          break;
        case 'T':
        case 't':
        case 'U':
        case 'u':
          value = T;
          break;
        case '-':
          break;
        case 'N':
        case 'n':
          value = A | G | C | T;
          break;
        case 'Y':
        case 'y':
          value = A | G;
          break;
        case 'M':
        case 'm':
          value = A | C;
          break;
        case 'S':
        case 's':
          value = G | C;
          break;
        case 'V':
        case 'v':
          value = A | G | C;
          break;
        case 'W':
        case 'w':
          value = A | T;
          break;
        case 'K':
        case 'k':
          value = G | T;
          break;
        case 'D':
        case 'd':
          value = A | G | T;
          break;
        case 'R':
        case 'r':
          value = C | T;
          break;
        case 'H':
        case 'h':
          value = A | C | T;
          break;
        case 'B':
        case 'b':
          value = G | C | T;
          break;
        default:
          throw TreeError("Undefined nucleotide character encountered.");
          break;
      }
      
      _parsimonySeq->at(i).setUShortValue(value);
    }
  }
}

const basic_string<ParsimonyNode::NucleotideComparitor> * ParsimonyNode::parsimonySeq()
{
  
  if (isLeaf() && _parsimonySeq == 0)
  {
    setParsimonySeqFromSeq();
    return _parsimonySeq;
  }
    
  ParsimonyNode *p = dynamic_cast<ParsimonyNode *>(in());
  
  while (_parsimonySeq == NULL && p != this)
  {
    _parsimonySeq = p->_parsimonySeq;
    p->_parsimonySeq = NULL;
    p = dynamic_cast<ParsimonyNode *>(p->in());
  }
  
  return _parsimonySeq;
}

void ParsimonyNode::setCost(unsigned idx, Nucleotide nucleotide, unsigned cost)
{
  if (! _stateCosts)
  {
    
    // TODO check dynamic casts for null pointers!
    ParsimonyNode *p = dynamic_cast<ParsimonyNode *>(in());
    
    // Move cost vectors around if tree has been re-rooted
    while (p != this)
    {
      if (p->_stateCosts)
      {
        _stateCosts = p->_stateCosts;
        p->_stateCosts = NULL;
      }
      p = dynamic_cast<ParsimonyNode *>(p->in());
    }
    
    if (!_stateCosts)
      _stateCosts = new vector<unsigned *>();
  }
  
  while (_stateCosts->size() <= idx)
  {
    _stateCosts->push_back(new unsigned[alphabetSize()]);
  }
  
  unsigned nucIdx = 0;
  while (nucleotide >> nucIdx > 1)  nucIdx++;
  _stateCosts->at(idx)[nucIdx] = cost;

}

unsigned ParsimonyNode::cost(unsigned idx, Nucleotide nucleotide)
{
  unsigned nucIdx = 0;
  while (nucleotide >> nucIdx > 1)  nucIdx++;

  return _stateCosts->at(idx)[nucIdx];
}

unsigned ParsimonyNode::minCost(unsigned idx, Nucleotide ancestralState)
{
  // cost of substitution from ancestral state to 0th state (A), plus cost of A in subtree
  unsigned ancIdx = 0;
  while (ancestralState >>ancIdx > 1)  ancIdx++;

  unsigned minval = ((_stateCosts->at(idx)[0] == numeric_limits<unsigned>::max()) ? numeric_limits<unsigned>::max()
                     :  _subCosts[ancIdx * alphabetSize()] + _stateCosts->at(idx)[0]);
  
  for (unsigned i = 1; i < alphabetSize(); i++)
  {
    unsigned val = ((_stateCosts->at(idx)[i] == numeric_limits<unsigned>::max()) ? numeric_limits<unsigned>::max() 
                    : _subCosts[ancIdx * alphabetSize() + i] + _stateCosts->at(idx)[i]);
    if (val < minval)
      minval = val;
  }
  
  return minval;
}

ParsimonyNode::NucleotideComparitor::NucleotideComparitor(unsigned short nucleotide) 
 : _nuc(nucleotide) 
{
  unsigned maxvalue = 1 << _alphabetSize;
  if (nucleotide >= maxvalue)  throw TreeError("Ambiguous nucleotide out of range of alphabet size.");

}

char ParsimonyNode::NucleotideComparitor::charValue() const
{
  return _nuc2chr[_nuc];
}

unsigned short ParsimonyNode::NucleotideComparitor::uShortValue() const
{
  return _nuc;
}


void ParsimonyNode::NucleotideComparitor::setUShortValue(unsigned short value)
{
  
  unsigned maxvalue = 1 << _alphabetSize;
  if (value >= maxvalue)  throw TreeError("Ambiguous nucleotide out of range of alphabet size.");

  _nuc = value;
}

bool ParsimonyNode::NucleotideComparitor::operator==(const NucleotideComparitor &other) const
{
  // should cover the case of two gap characters
  if (_nuc == other._nuc)  return true; 
  return _nuc & other._nuc;
}

bool ParsimonyNode::NucleotideComparitor::operator<(const NucleotideComparitor &other) const
{
  if (operator==(other))  return false;
  
  if (_nuc == 0)  return true;
  if (other._nuc == 0) return false;
  
  for (unsigned i = 0; i < _alphabetSize; i++)
  {
    if (_nuc & nucleotides[i])  return true;
    if (other._nuc & nucleotides[i])  return false;
  }
  
  cerr << "This return statement should never be reached." << endl;

  return false;
}

bool ParsimonyNode::NucleotideComparitor::operator>(const NucleotideComparitor &other) const
{
   if (operator==(other))  return false;
   
   return ! operator<(other);
}

bool ParsimonyNode::NucleotideComparitor::operator<=(const NucleotideComparitor &other) const
{
   if (operator==(other))  return true;
   
   return operator<(other);
}

bool ParsimonyNode::NucleotideComparitor::operator>=(const NucleotideComparitor &other) const
{
   if (operator==(other))  return true;
   
   return operator>(other);
}

bool ParsimonyNode::NucleotideComparitor::operator!=(const NucleotideComparitor &other) const
{
  return ! operator==(other);
}

ostream &operator<<(ostream & os, const ParsimonyNode::NucleotideComparitor & nuc)
{
  
  os << nuc.charValue();
  
  return os;
}


