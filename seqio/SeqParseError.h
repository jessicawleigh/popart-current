/*
 * SeqParseError.h
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */

#ifndef SEQPARSEERROR_H_
#define SEQPARSEERROR_H_

#include <exception>
#include <string>

class SeqParseError : public std::exception
{
public:
  SeqParseError();
  SeqParseError(const std::string &);
  virtual const char* what() const throw();
  virtual ~SeqParseError() throw() {};

private:
  const std::string _msg;
};


class SeqParseWarning : public std::exception
{
public:
  SeqParseWarning(const std::string &);
  virtual const char* what() const throw();
  virtual ~SeqParseWarning() throw() {};

private:
  const std::string _msg;
};



#endif /* SEQPARSEERROR_H_ */
