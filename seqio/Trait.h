#ifndef TRAIT_H_
#define TRAIT_H_

#include <string>
#include <map>
#include <vector>

class Trait 
{
public:
  Trait(const std::string &);
  virtual ~Trait();
  
  void setName(const std::string &);
  const std::string & name() const;
  
  void addSeq(const std::string &, unsigned );
  unsigned seqCount(const std::string &) const;
  
  std::vector<std::string> seqNames() const;
  
private:
  std::string _traitName;
  std::map<std::string, unsigned> _seqCounts;
};

#endif
