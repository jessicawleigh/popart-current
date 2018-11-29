/*
 * StatsError.h
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */

#ifndef STATSERROR_H_
#define STATSERROR_H_

#include <exception>
#include <string>

class StatsError: public std::exception {
public:
  StatsError();
  StatsError(const std::string &);
  virtual const char* what() const throw();
  virtual ~StatsError() throw() {};

private:
  const std::string _msg;
};

#endif /* STATSERROR_H_ */
