/*
 * SequenceError.h
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */

#ifndef SEQUENCEERROR_H_
#define SEQUENCEERROR_H_

#include <exception>
#include <string>

class SequenceError: public std::exception {
public:
  SequenceError();
  SequenceError(const std::string&);
  virtual const char* what() const throw();
  virtual ~SequenceError() throw() {};

private:
  const std::string _msg;
};

#endif /* SEQUENCEERROR_H_ */
