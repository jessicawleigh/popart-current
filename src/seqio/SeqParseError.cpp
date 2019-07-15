/*
 * SeqParseError.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */
#include <cstring>
#include <iostream>
#include <string>
using namespace std;

#include "SeqParseError.h"


SeqParseError::SeqParseError()
  : exception(), _msg("Unspecified error parsing sequence file.")
{}

SeqParseError::SeqParseError(const string & message)
  : exception(), _msg(message)
{}

const char* SeqParseError::what() const throw()
{
  return _msg.c_str();
}

SeqParseWarning::SeqParseWarning(const string & message)
  : exception(), _msg(message)
{}

const char* SeqParseWarning::what() const throw()
{
  return _msg.c_str();
}
