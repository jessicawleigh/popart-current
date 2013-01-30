/*
 * SequenceError.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */
#include <cstring>
#include <string>
using namespace std;

#include "SequenceError.h"

SequenceError::SequenceError()
  : exception(), _msg("Unspecified error in Sequence class.")
{}

SequenceError::SequenceError(const string &message)
  : exception(), _msg(message)
{}


const char* SequenceError::what() const throw()
{
  return _msg.c_str();
}

