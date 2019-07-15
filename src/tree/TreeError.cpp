#include "TreeError.h"

#include <cstring>
#include <string>
#include <exception>
using namespace std;

TreeError::TreeError()
  : exception(), _msg("Tree or TreeNode-related error.")
{}

TreeError::TreeError(const string &message)
  : exception(), _msg(message)
{}

const char* TreeError::what() const throw()
{
  return _msg.c_str();
}

