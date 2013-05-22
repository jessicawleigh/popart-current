/*
 * TableParser.h
 *
 *  Created on: May 22, 2013
 *      Author: jleigh
 */

#ifndef TABLEPARSER_H_
#define TABLEPARSER_H_

#include "SeqParseError.h"

#include <fstream>
#include <string>
#include <vector>

class TableParser
{
public:
  TableParser(char = ',', bool = false, bool = false, char = '\0');
  virtual ~TableParser();

  void resetParser();

  void setDelimChar(char c) { _delim = c; };
  char delimChar() const { return _delim; };
  void setMergeDelims(bool mergeDelims) { _mergeDelims = mergeDelims; };
  char mergeDelims() const { return _mergeDelims; };
  void setHasHeader(bool hasHeader) { _hasHeader = hasHeader; };
  char hasHeader() const { return _hasHeader; };
  void setEOLChar(char c) { _eol = c; };
  char eolChar() const { return _eol; };
  char dataType(unsigned) const;
  char setDataType(unsigned, char);

  unsigned rows() const { return _nrow; };
  unsigned columns() const { return _ncol; };

  void readTable(std::istream &);
  const std::vector<std::vector<std::string> > & data() const { return _rows; };
  const std::vector<std::string> & headerData() const { return _headerData; };
  const std::string & headerData(unsigned) const;
  const std::string & data(unsigned, unsigned) const;
  double dataDouble(unsigned, unsigned) const;
  int dataInt(unsigned, unsigned) const;


//  template <class T>
//  T data(unsigned row, unsigned col)
//  {
//    if (row >= _nrows || col >= _ncols)  throw SeqParseError("Index out of range.");
//    switch(_dataTypes)
//  };

private:

  char _delim;
  bool _mergeDelims;
  bool _hasHeader;
  char _eol;

  std::vector<std::vector<std::string> > _rows;
  std::vector<char> _dataTypes;
  std::vector<std::string> _headerData;

  unsigned _ncol;
  unsigned _nrow;
};

#endif /* TABLEPARSER_H_ */
