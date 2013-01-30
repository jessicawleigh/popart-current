/*
 * NetworkError.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */
using namespace std;

#include "NetworkError.h"

NetworkError::NetworkError()
  : exception(), _msg("Unspecified error in network class.")
{}

NetworkError::NetworkError(const string &message)
  : exception(), _msg(message)
{}


const char* NetworkError::what() const throw()
{
  return _msg.c_str();
}

