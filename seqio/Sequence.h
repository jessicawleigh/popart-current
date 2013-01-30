/*
 * Sequence.h
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <string>
#include <iostream>
#include <vector>

#include "SeqParser.h"
#include "GeneticCode.h"

// TODO remove methods that are only necessary for sQAled?

#define CHARSPERLINE 70

class SeqParser;


class Sequence {
public:
  typedef enum CHARTYPE
  {
    AAType,
    DNAType,
    BinaryType
  } CharType;

  Sequence(const std::string, const std::string);
  Sequence(const Sequence &, bool = false);
  Sequence(const Sequence &, int, GeneticCode = GeneticCode::StandardCode());
  Sequence();
  virtual ~Sequence();
  const std::string & seq() const;
  const std::string & name() const;
  const char & at(size_t) const;
  char & at(size_t);
  std::string subseq(int, int = -1) const;
  void setSeq(const std::string);
  void setName(const std::string);
  static void setParser(SeqParser *);
  static void setParser(std::istream &);
  static SeqParser* parser();
  void setCharType(CharType ct) { _chartype = ct; } ;
  CharType charType() const { return _chartype; } ;
  size_t length() const;
  bool empty() const { return _seq.empty(); };
  void clear();
  const std::string& replace( size_t, size_t, const std::string &);
  void delChar(const int);
  void delCharRange(const unsigned, const int = -1);
  void insertGaps(const unsigned, const unsigned);
  void insertChars(const unsigned, const std::string);
  void pad(unsigned, char = '-');
  void maskChars(const std::vector<bool> &);

  char operator[](unsigned) const;
  char &operator[](unsigned);
  Sequence &operator+=(const std::string &);
  bool operator==(const Sequence &) const;
  bool operator<(const Sequence &) const;
  bool operator>(const Sequence &) const;
  bool operator<=(const Sequence &other) const { return ! operator>(other); };
  bool operator>=(const Sequence &other) const { return ! operator<(other); };
  
  bool operator==(const std::string &) const;
  bool operator!=(const Sequence &) const;
  bool operator!=(const std::string &) const;

  static bool isValidChar(char, CharType);
  static bool isAmbiguousChar(char, CharType);

  friend std::ostream &operator<<(std::ostream &, const Sequence &);
  friend std::istream &operator>>(std::istream &, Sequence &);

private:
  std::string _name;
  std::string _seq;

  static SeqParser *_parser;
  CharType _chartype;
};

#endif /* SEQUENCE_H_ */
