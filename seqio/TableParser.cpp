/*
 * TableParser.cpp
 *
 *  Created on: May 22, 2013
 *      Author: jleigh
 */

#include "TableParser.h"
#include "ParserTools.h"

#include <sstream>
using namespace std;

TableParser::TableParser(char delim, bool mergeDelims, bool hasHeader, char eolChar)
{
  _delim = delim;
  _mergeDelims = mergeDelims;
  _hasHeader = hasHeader;

  _eol = eolChar;
  _ncol = 0;
  _nrow = 0;
}

TableParser::~TableParser()
{
  // TODO Auto-generated destructor stub
}

void TableParser::resetParser()
{
  _rows.clear();
  _dataTypes.clear();
  _headerData.clear();
  _ncol = 0;
  _nrow = 0;

}

void TableParser::readTable(istream &input)
{
  string line;
  vector<string> wordlist;
  string delimstr;
  delimstr += _delim;

  char eol = _eol;
  if (eol == '\0')
    eol = ParserTools::getEOLchar(input);

  resetParser();

  while (input.good())
  {
    getline(input, line, eol);
    if (eol == '\n' && line.at(line.size() - 1) == '\r')
      line.erase(line.size() - 1);

    if (_headerData.empty() && _hasHeader)
    {
      ParserTools::tokenise(_headerData, line, delimstr, _mergeDelims);
      _ncol = _headerData.size();
      _dataTypes.assign(_ncol, 's');
    }

    else
    {
      ParserTools::tokenise(wordlist, line, delimstr, _mergeDelims);
      if (wordlist.size() > _ncol)
      {
        for (unsigned i = 0; i <_rows.size(); i++)
        {
          for (unsigned j = _ncol; j < wordlist.size(); j++)
            _rows.at(i).push_back("");
        }
        _ncol = wordlist.size();
       _dataTypes.resize(_ncol, 's');
      }

      _rows.push_back(wordlist);
      wordlist.clear();
    }
  }

  _nrow = _rows.size();
}

char TableParser::dataType(unsigned col) const
{
  if (col >= _ncol)  throw SeqParseError("Index out of range.");

  return _dataTypes.at(col);
}

char TableParser::setDataType(unsigned col, char type)
{
  if (type != 's' && type != 'i' && type != 'f')  throw SeqParseError("Invalid type.");
  if (col >= _ncol)  throw SeqParseError("Index out of range.");

  _dataTypes.at(col) = type;
}

const string & TableParser::headerData(unsigned col) const
{
  if (col >= _ncol)  throw SeqParseError("Index out of range.");

  return _headerData.at(col);
}

const string & TableParser::data(unsigned row, unsigned col) const
{
  if (row >= _nrow || col >= _ncol)  throw SeqParseError("Index out of range.");

  return _rows.at(row).at(col);
}

double TableParser::dataDouble(unsigned row, unsigned col) const
{
  if (row >= _nrow || col >= _ncol)  throw SeqParseError("Index out of range.");

  if (_dataTypes.at(col) != 'f')  throw SeqParseError("Value is not a double.");

  istringstream iss(_rows.at(row).at(col).data());
  double dval;
  
  iss >> dval;

  return dval;
}

int TableParser::dataInt(unsigned row, unsigned col) const
{
  if (row >= _nrow || col >= _ncol)  throw SeqParseError("Index out of range.");

  if (_dataTypes.at(col) != 'd')  throw SeqParseError("Value is not an integer.");
 
  istringstream iss(_rows.at(row).at(col).data());
  int ival;
  
  iss >> ival;
  return ival;
}



