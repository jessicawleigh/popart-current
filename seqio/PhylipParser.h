#ifndef PHYLIPPARSER_H
#define PHYLIPPARSER_H

#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;

#include "Sequence.h"
#include "SeqParser.h"

class PhylipSeqParser : public SeqParser
{
public:

  typedef enum PHYLIPVARIANT
  {
    Sequential,
    Interleaved,
    Relaxed
  } PhylipVariant;
  
  PhylipSeqParser(PhylipVariant = Interleaved, int nchar = 0, int nseq = 0);
  virtual Sequence & getSeq(istream &, Sequence &);
  
  /* Note that default arguments are actually very important in Phylip format */
  virtual void putSeq(ostream &, const Sequence &);
  virtual void resetParser();
  void readSeqs(istream &);
  PhylipVariant variant() const;
  void setPhylipVariant(PhylipVariant);
  /*void setNchar(int);
  int nChar() const;
  void setNseq(int);
  int nSeq() const;*/
  
private:
  //Sequence nextSeq();
  //string strip(string) const;
  //vector<string>* tokenise(string , string  = " \t\n\r") const;
  bool seqsLoaded() const;
  //void printVariant() const;
  //bool _headerRead;
  //int _expNchar;
  //int _expNseq;
  bool _seqsloaded;
  bool _headerwritten;
  vector<Sequence> _seqvect;
  vector<Sequence>::const_iterator _seqiter;
  PhylipVariant _variant;
};

#endif
