#include <cstring>
#include <string>
using namespace std;

#include "HapAppError.h"

HapAppError::HapAppError()
  : exception(), _msg("Unspecified error in popART.")
{}

HapAppError::HapAppError(const string &message)
  : exception(), _msg(message)
{}

const char* HapAppError::what() const throw()
{
  return _msg.c_str();
}

