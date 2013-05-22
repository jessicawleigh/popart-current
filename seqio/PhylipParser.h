#ifndef PHYLIPPARSER_H
#define PHYLIPPARSER_H

#include <fstream>
#include <string>
#include <vector>

#include "Sequence.h"
#include "SeqParser.h"

class PhylipSeqParser : public SeqParser
{
public:

  typedef enum PHYLIPVARIANT
  {
    Unknown,
    Sequential,
    Interleaved,
    Relaxed
  } PhylipVariant;
  
  PhylipSeqParser(PhylipVariant = Unknown, int nchar = 0, int nseq = 0);
  virtual Sequence & getSeq(std::istream &, Sequence &);
  
  virtual void putSeq(std::ostream &, const Sequence &);
  virtual void resetParser();
  void readSeqs(std::istream &);

  bool readSeqsVariant(std::istream &, PhylipVariant);
  PhylipVariant variant() const;
  void setPhylipVariant(PhylipVariant);
  
private:
  bool seqsLoaded() const;
  static const int NAMELENGTH = 10;

  bool _seqsloaded;
  bool _headerwritten;
  std::vector<Sequence> _seqvect;
  std::vector<Sequence>::const_iterator _seqiter;
  PhylipVariant _variant;
};

#endif
