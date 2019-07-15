#ifndef HAPAPPERROR_H_
#define HAPAPPERROR_H_

#include <exception>
#include <string>

class HapAppError: public std::exception {
public:
  HapAppError();
  HapAppError(const std::string &);
  virtual const char* what() const throw();
  virtual ~HapAppError() throw() {};

private:
  const std::string _msg;
};

#endif /* NETWORKERROR_H_ */
