#include "Trait.h"
#include "SequenceError.h"

#include <iostream>
using namespace std;

Trait::Trait(const string &name, unsigned groupID)
  : _traitName(name), _traitGroup(groupID)
{

}

Trait::~Trait()
{}
  
void Trait::setName(const string &name)
{
  _traitName = name;
  
}

const string & Trait::name() const
{
  return _traitName;
}

void Trait::addSeq(const string &seqname, unsigned seqcount)
{
  map<string, unsigned>::const_iterator traitIt = _seqCounts.find(seqname);
  
  if (traitIt == _seqCounts.end())
    _seqCounts[seqname] = seqcount; 
  
  else
    _seqCounts[seqname] += seqcount;
}

unsigned Trait::seqCount(const string &seqname) const
{
  map<string, unsigned>::const_iterator traitIt = _seqCounts.find(seqname);
  
  if (traitIt == _seqCounts.end())  throw SequenceError("Sequence not associated with this trait.");

  return traitIt->second;
}
  
vector<string> Trait::seqNames() const
{
  vector<string> seqnames;
  
  map<string, unsigned>::const_iterator traitIt = _seqCounts.begin();
 
  while (traitIt != _seqCounts.end())
  {
    seqnames.push_back(traitIt->first);
    ++traitIt;
  }
  
  return seqnames;
}

void Trait::setGroup(unsigned groupID)
{
  _traitGroup = groupID;
}

unsigned Trait::group() const
{
  return _traitGroup;
}