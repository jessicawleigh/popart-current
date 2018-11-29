/*
 * StatsError.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */
using namespace std;

#include "StatsError.h"

StatsError::StatsError()
  : exception(), _msg("Unspecified statistics error.")
{}

StatsError::StatsError(const string &message)
  : exception(), _msg(message)
{}


const char* StatsError::what() const throw()
{
  return _msg.c_str();
}

