/*
 * SeqParser.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#include <algorithm>
#include "SeqParser.h"

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

string & SeqParser::strip(string &str)
{
  string whitespace = " \t\n\r";
  size_t substrstart = str.find_first_not_of(whitespace);

  if (substrstart == string::npos) str.clear();
  
  else
  {
    if (substrstart != string::npos) str.erase(0, substrstart);

    size_t substrend = str.find_last_not_of(whitespace);

    if ((substrend != string::npos) && ((substrend + 1) < str.length()))
      str.erase(substrend + 1);
  }

  return str;
}

void SeqParser::tokenise(vector<string>& tokens, const string & str, const string & delim)
{
  //const int INITSIZE = 10;
  //vector<string> *tokens = new vector<string>;//(INITSIZE);
  //int tokcount = 0;

  size_t tokstart = str.find_first_not_of(delim);
  size_t tokend;

  while (tokstart != string::npos)
  {
    tokend = str.find_first_of(delim, tokstart);
    if (tokend == string::npos)
      tokens.push_back(str.substr(tokstart));

    else
      tokens.push_back(str.substr(tokstart, tokend - tokstart));

    tokstart = str.find_first_not_of(delim, tokend);
  }

  //return tokens;
}

bool SeqParser::caselessmatch(char c1, char c2)
{
  bool returnval = (toupper(c1) == toupper(c2));

  return returnval;
}

size_t SeqParser::caselessfind(const string &needle, const string &haystack)
{
  string::const_iterator pos = search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caselessmatch);

  if (pos == haystack.end())  return string::npos;

  else return pos - haystack.begin();
}

string & SeqParser::replaceChars(string &str, char cold, char cnew)
{
  size_t index = str.find(cold);

  while (index != string::npos)
  {
    str.at(index) = cnew;
    index = str.find(cold, index + 1);
  }

  return str;
}

string & SeqParser::eraseChars(string & str, char c)
{
  size_t index = str.find(c);

  while(index != string::npos)
  {
    str.erase(index, 1);
    index = str.find(c, index);
  }

  return str;
}

string & SeqParser::lower(string & str)
{
  for (unsigned i = 0; i < str.length(); i++)  str.at(i) = tolower(str.at(i));

  return str;
}

string & SeqParser::upper(string & str)
{
  for (unsigned i = 0; i < str.length(); i++)  str.at(i) = toupper(str.at(i));

  return str;
}

SeqParser::~SeqParser() {
	// TODO Auto-generated destructor stub
}

