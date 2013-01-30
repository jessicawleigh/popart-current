#ifndef PARSIMONYNODE_H_
#define PARSIMONYNODE_H_

#include "SeqNode.h"
#include "TreeNode.h"

#include <string>
#include <vector>

class ParsimonyNode : public SeqNode
{
private:
  static const size_t _alphabetSize = 4;

  
public:
  typedef enum {O = 0, A = 1, G = 2, C = 4, T = 8} Nucleotide;

  class NucleotideComparitor
  {
  public:
    NucleotideComparitor(unsigned short = 0);
    NucleotideComparitor(const NucleotideComparitor & other)
      : _nuc(other._nuc)  {}
      
    char charValue() const;
    unsigned short uShortValue() const;
    void setUShortValue(unsigned short);
    
    bool operator==(const NucleotideComparitor &) const;
    bool operator>(const NucleotideComparitor &) const;
    bool operator<(const NucleotideComparitor &) const;
    bool operator>=(const NucleotideComparitor &) const;
    bool operator<=(const NucleotideComparitor &) const;
    bool operator!=(const NucleotideComparitor &) const;

  private:
    unsigned short _nuc;
    
  friend std::ostream &operator<<(std::ostream &, const NucleotideComparitor &);
  };
    
  
  ParsimonyNode(Sequence * = NULL, const std::string & = "", double = -1);
  virtual ~ParsimonyNode();
  
  virtual TreeNode* newNodeVirtual(const std::string & = "", double = -1) const;
  void setCost(unsigned, Nucleotide, unsigned);
  unsigned cost(unsigned, Nucleotide);
  unsigned minCost(unsigned, Nucleotide);
  const basic_string<NucleotideComparitor> * parsimonySeq();
  virtual const char & at(size_t) const;
  static char nuc2char(short unsigned);
  static char nuc2char(const NucleotideComparitor &);
  
  static const unsigned short & char2nuc(const char &);
  
  NucleotideComparitor & stateAt(size_t);
  const NucleotideComparitor & stateAt(size_t) const;
  
  virtual size_t alphabetSize() const { return _alphabetSize; }
  virtual unsigned substitutionCost(Nucleotide src, Nucleotide dst);
  
  virtual void resizeSeq(size_t);
  
  static const Nucleotide nucleotides[];
  static const char nucleotideChars[];
  
  
private:
  
  void setParsimonySeqFromSeq();
  vector<unsigned *> *_stateCosts;
  basic_string<NucleotideComparitor> *_parsimonySeq;  
  static const unsigned _subCosts[] ;
  static const char _nuc2chr[];
  static const short unsigned _chr2nuc[];

  
};
 

#endif
 