#ifndef TREEERROR_H_
#define TREEERROR_H_

#include <exception>
#include <string>

class TreeError : public std::exception
{
public:
  TreeError();
  TreeError(const std::string &);
  virtual const char* what() const throw();
  virtual ~TreeError() throw() {};
  
private:
  const std::string _msg;  
};

#endif