/*
 * SeqParser.h
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#ifndef SEQPARSER_H_
#define SEQPARSER_H_

#include <fstream>
#include <queue>
#include <string>
#include <vector>

#include "Sequence.h"

class Sequence;

class SeqParser {
  
public:

  typedef enum
  {
    Fasta,
    Phylip,
    Nexus,
    Stockholm
  } SeqType;


  // TODO write function to guess chartype
  typedef enum CHARTYPE
  {
    AAType,
    DNAType,
    StandardType
  } CharType;

  SeqParser() { _eol = '\n'; };
  virtual ~SeqParser();

  virtual Sequence & getSeq(std::istream &, Sequence &) = 0;
  virtual void putSeq(std::ostream &, const Sequence &) = 0;
  virtual void resetParser() = 0;

  void setNchar(int);
  void setNseq(int);
  void setCharType(CharType);
  int nChar() const;
  int nSeq() const;
  CharType charType() const;
  std::string getWarning();
  void setEOLChar(char c) { _eol = c; };
  char eolChar() const { return _eol; };
  
protected:
  void warn(std::string msg) { _warnings.push(msg); };

private:
  int _nchar;
  int _nseq;
  char _eol;
  CharType _charType;
  std::queue<std::string> _warnings;

};

#endif /* SEQPARSER_H_ */
