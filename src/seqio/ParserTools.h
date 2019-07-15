/*
 * ParserTools.h
 *
 *  Created on: May 22, 2013
 *      Author: jleigh
 */

#ifndef PARSERTOOLS_H_
#define PARSERTOOLS_H_

#include <iostream>
#include <string>
#include <vector>

class ParserTools
{
public:

  static std::string & strip(std::string &);
  static std::string & rstrip(std::string &);
  static std::string & lstrip(std::string &);
  static void tokenise(std::vector<std::string> &, const std::string &, const std::string &  = " \t\n\r", bool = true);
  static bool caselessmatch(char, char);
  static size_t caselessfind(const std::string &, const std::string &);
  static std::string & replaceChars(std::string &, char, char);
  static std::string & eraseChars(std::string &, char);
  static std::string & lower(std::string &);
  static std::string & upper(std::string &);
  static char getEOLchar(std::istream &);
};

#endif /* PARSERTOOLS_H_ */
