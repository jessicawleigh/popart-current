/*
 * ParserTools.cpp
 *
 *  Created on: May 22, 2013
 *      Author: jleigh
 */

#include <algorithm>
using namespace std;

#include "ParserTools.h"
#include "SeqParseError.h"

string & ParserTools::strip(string &str)
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
string & ParserTools::rstrip(string &str)
{
  string whitespace = " \t\n\r";
  size_t substrend = str.find_last_not_of(whitespace);

  if (substrend == string::npos)  str.clear();

  else if ((substrend + 1) < str.length())
    str.erase(substrend + 1);

  return str;
}

string & ParserTools::lstrip(string &str)
{
  string whitespace = " \t\n\r";
  size_t substrstart = str.find_first_not_of(whitespace);

  if (substrstart == string::npos) str.clear();

  else if (substrstart != string::npos)
    str.erase(0, substrstart);

  return str;
}



void ParserTools::tokenise(vector<string>& tokens, const string & str, const string & delim, bool mergeDelims)
{
  //const int INITSIZE = 10;
  //vector<string> *tokens = new vector<string>;//(INITSIZE);
  //int tokcount = 0;

  size_t tokstart;
  if (mergeDelims)
    tokstart = str.find_first_not_of(delim);
  size_t tokend;

  while (tokstart != string::npos)
  {
    tokend = str.find_first_of(delim, tokstart);
    if (tokend == string::npos)
      tokens.push_back(str.substr(tokstart));

    else
      tokens.push_back(str.substr(tokstart, tokend - tokstart));

    if (mergeDelims)
      tokstart = str.find_first_not_of(delim, tokend);
    else
      tokstart = tokend;
  }

  //return tokens;
}

bool ParserTools::caselessmatch(char c1, char c2)
{
  bool returnval = (toupper(c1) == toupper(c2));

  return returnval;
}

size_t ParserTools::caselessfind(const string &needle, const string &haystack)
{
  string::const_iterator pos = search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caselessmatch);

  if (pos == haystack.end())  return string::npos;

  else return pos - haystack.begin();
}

string & ParserTools::replaceChars(string &str, char cold, char cnew)
{
  size_t index = str.find(cold);

  while (index != string::npos)
  {
    str.at(index) = cnew;
    index = str.find(cold, index + 1);
  }

  return str;
}

string & ParserTools::eraseChars(string & str, char c)
{
  size_t index = str.find(c);

  while(index != string::npos)
  {
    str.erase(index, 1);
    index = str.find(c, index);
  }

  return str;
}

string & ParserTools::lower(string & str)
{
  for (unsigned i = 0; i < str.length(); i++)  str.at(i) = tolower(str.at(i));

  return str;
}

string & ParserTools::upper(string & str)
{
  for (unsigned i = 0; i < str.length(); i++)  str.at(i) = toupper(str.at(i));

  return str;
}


char ParserTools::getEOLchar(istream &input)
{
  char eol = '\n';
  char c;

  if (! input.good())
    throw SeqParseError("Could not read from file.");

  int beginning = input.tellg();



  while (input.good())
  {
    input.get(c);
    if (c == '\r')
    {
      char next = input.peek();
      if (next == '\n')
        eol = '\n';
      else
        eol = '\r';
      break;
    }

    if (c == '\n')
    {
      eol = '\n';
      break;
    }
  }

  input.seekg(beginning);
  input.clear();

  return eol;
}
