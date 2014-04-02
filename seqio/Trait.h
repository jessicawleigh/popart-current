#ifndef TRAIT_H_
#define TRAIT_H_

#include <string>
#include <map>
#include <vector>

class Trait 
{
public:
  Trait(const std::string &, unsigned = 0);
  virtual ~Trait();
  
  void setName(const std::string &);
  const std::string & name() const;
  
  void addSeq(const std::string &, unsigned );
  unsigned seqCount(const std::string &) const;
  
  std::vector<std::string> seqNames() const;
  
  void setGroup(unsigned);
  unsigned group() const;
  
private:
  std::string _traitName;
  std::map<std::string, unsigned> _seqCounts;
  unsigned _traitGroup; 
};

#endif
