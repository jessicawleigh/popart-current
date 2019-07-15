#ifndef GENETICCODE_H
#define GENETICCODE_H

#include <map>
#include <cstring>
#include <string>
#include <iostream>
#include <exception>
using namespace std;

#define CODONLENGTH 3

typedef map<string, char, std::less<string> > CodonMap;

class GeneticCode
{
public:
  char operator[](string) const;  
  static GeneticCode StandardCode();
  static GeneticCode AlternateCode(int);
  static string lookupCode(int);
  
  // also a variety of other codes.
  
  friend ostream &operator<<(ostream &, GeneticCode &);
  
private:
  GeneticCode();
  CodonMap & code();
  void setCodeID(int);
  int codeID() const;
  //char & operator[](string);
  
  CodonMap _code;
  int _codeID;
};


class GeneticCodeError : public exception
{
public:
  GeneticCodeError();
  GeneticCodeError(const char *const&);
  virtual const char* what() const throw();
  virtual ~GeneticCodeError() throw();
  
private:
  char *_msg;
};

#endif