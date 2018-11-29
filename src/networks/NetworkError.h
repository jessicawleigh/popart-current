/*
 * NetworkError.h
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */

#ifndef NETWORKERROR_H_
#define NETWORKERROR_H_

#include <exception>
#include <string>

class NetworkError: public std::exception {
public:
  NetworkError();
  NetworkError(const std::string &);
  virtual const char* what() const throw();
  virtual ~NetworkError() throw() {};

private:
  const std::string _msg;
};

#endif /* NETWORKERROR_H_ */
