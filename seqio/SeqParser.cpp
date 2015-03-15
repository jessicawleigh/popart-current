/*
 * SeqParser.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#include "SeqParser.h"
#include "ParserTools.h"

using namespace std;

void SeqParser::setNchar(int nchar)
{
  _nchar = nchar;
}

void SeqParser::setNseq(int nseq)
{
  _nseq = nseq;
}

void SeqParser::setCharType(CharType type)
{
  _charType = type;
}

int SeqParser::nChar() const
{
  return _nchar;
}

int SeqParser::nSeq() const
{
  return _nseq;
}

string SeqParser::getWarning()
{
  if (_warnings.empty())  return string();
    
  string w = _warnings.front();
  _warnings.pop();
  
  return w;
}

SeqParser::CharType SeqParser::charType() const
{
  return _charType;
}

SeqParser::~SeqParser() {
	// TODO Auto-generated destructor stub
}

