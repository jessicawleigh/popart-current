/*
 * HapNet.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

#include <map>
#include <queue>
#include <sstream>
using namespace std;

#include "HapNet.h"
#include "NetworkError.h"
#include "TreeError.h"


#ifdef NET_QT
#include <QThread>
#include <QApplication>
#include <QDebug>

#endif

const vector<unsigned> HapNet::_emptyTraits;

HapNet::HapNet(const vector<Sequence*> & seqs, const vector<bool> & mask)
{

  _datatype = seqs.at(0)->charType();
  
  if (_datatype == Sequence::AAType)  throw NetworkError("Haplotype networks shouldn't be inferred from protein data.");
  
  for (unsigned i = 0; i < seqs.size(); i++)
  {
    _origSeqs.push_back(new Sequence(*(seqs.at(i))));
    if (! mask.empty())
      _origSeqs.at(i)->maskChars(mask);
  }
  
  _nseqs = 0;
  _nsites = _origSeqs.at(0)->length();//seqs.at(0)->length();
  _nCsites = 0;
  _traits = 0;


  _orig2cond = new unsigned[_origSeqs.size()];
  _oPos2CPos = 0;

  condenseSeqs();
  condenseSitePats();

  _distances = new unsigned[_nseqs * _nseqs];
  _isGraphSetup = false;

}

HapNet::~HapNet()
{
  if (_traits)
  {
    for (unsigned i = 0; i < _nseqs; i++)
      _traits[i].empty();
      //if (_traits[i])  delete [] _traits[i];

    delete [] _traits;
  }

  delete [] _orig2cond;
  delete [] _distances;
  //delete [] _weights;
  delete [] _oPos2CPos;
  delete [] _freqs;

}


void HapNet::condenseSeqs()
{
  unsigned seqCount = 0;
  unsigned seqlen;
  vector<Sequence *>::const_iterator seqIt;
  map<string, unsigned>::iterator idxIt;
  map<string, unsigned> seq2idx;
  

  for (seqIt = _origSeqs.begin(); seqIt != _origSeqs.end(); seqIt++, seqCount++)
  {
    seqlen = (*seqIt)->length();
    if (seqlen != _nsites)
    {
      cerr << "Sequences are not all the same length!" << endl;
      _nsites = max(seqlen, (unsigned)_nsites);
    }

    idxIt = seq2idx.find((*seqIt)->seq());

    if (idxIt == seq2idx.end())
    {
      _condensedSeqs.push_back((*seqIt)->seq());
      vector<unsigned> newvect;
      newvect.push_back(seqCount);
      _cond2orig.push_back(newvect); // Keeps track of first index of sequences in _origSeqs

      seq2idx[_condensedSeqs.back()] = _nseqs;
      _orig2cond[seqCount] = _nseqs++;
    }

    else
    {
      _orig2cond[seqCount] = idxIt->second;
      _cond2orig.at(idxIt->second).push_back(seqCount);
    }
  }
  
  // Number of observations of unique sequences
  _freqs = new unsigned[_nseqs];
  fill_n(_freqs, _nseqs, 0);

  for (unsigned i = 0; i < seqCount; i++)  _freqs[_orig2cond[i]]++;  
}

void HapNet::condenseSitePats()
{  
  unsigned samePosAs[_nsites];
  for (unsigned i = 0; i < _nsites; i++)   samePosAs[i] = i;

  // Find identical site patterns
  for (unsigned i = 0; i < _nsites; i++) 
  {
    // Check for all gaps in column i
    bool allgaps = true;
    for (unsigned t = 0; allgaps && t < _nseqs; t++)
      if (! Sequence::isAmbiguousChar(_condensedSeqs.at(t).at(i), _datatype))  allgaps = false;
      
    if (allgaps)  samePosAs[i] = _nsites;
    
    for (unsigned j = i + 1; j < _nsites; j++) 
    {
      bool same = true;
      char i2j[256];
      char j2i[256];
      
      for (unsigned k = 0; k < 256; k++)  i2j[k] = j2i[k] = 0;


      for (unsigned t = 0; same && t < _nseqs; t++) 
      {
        char chari = _condensedSeqs.at(t).at(i);
        char charj = _condensedSeqs.at(t).at(j);
        
        //if ((chari == '-' || charj == '-')  && chari != charj)
        if ((Sequence::isAmbiguousChar(chari, _datatype) || Sequence::isAmbiguousChar(charj, _datatype)) && chari != charj)
          same = false; 

        else if (i2j[chari] == (char) 0) 
        {
          i2j[chari] = charj;
          if (j2i[charj] == (char) 0)  j2i[charj] = chari;
          else if (j2i[charj] != chari)  same = false;
        } 
        
        else if (i2j[chari] != charj)  same = false;
        
      } // end for unsigned t...
                
      if (same) 
      {
        samePosAs[j] = samePosAs[i];
        break;
      }
    }
  }
  
  // new site indices in condensed seq vect... Should this be an instance variable?
  _oPos2CPos = new unsigned[_nsites]; 
  ostringstream buffers[_nseqs];
  
  unsigned newPos = 0;
  
  // Include identical site patterns only once
  for (unsigned i = 0; i < _nsites; i++)
  {
    _oPos2CPos[i] = 0;

    if (samePosAs[i] == _nsites)  _oPos2CPos[i] = _nsites; 
    else if (samePosAs[i] < i)  _oPos2CPos[i] = _oPos2CPos[samePosAs[i]];
    else 
    {
      // This should never happen: should be equal to i
      if (samePosAs[i] > i)  throw NetworkError("Serious error condensing site patterns.");
      
      _oPos2CPos[i] = newPos++;
      for (unsigned j = 0; j < _nseqs; j++)  buffers[j] << _condensedSeqs.at(j).at(i);
    }    
  }

  _nCsites = newPos;
    
  _weights.clear();
  for (unsigned i = 0; i < _nCsites; i++)  
    _weights.push_back(0);
  
  for (unsigned i = 0; i < _nsites; i++) 
    if (_oPos2CPos[i] < _nsites)  _weights.at(_oPos2CPos[i])++;

  for (unsigned i = 0; i < _nseqs; i++)  
    _condensedSeqs.at(i) = buffers[i].str();

}


void HapNet::associateTraits(const vector<Trait *> & traitVect)
{
  
  if (_traits)
  {
    for (unsigned i = 0; i < _nseqs; i++)
      _traits[i].empty();
      //if (_traits[i])  delete [] _traits[i];

    delete [] _traits;
  }
  
  _traitNames.clear();
    
  map<string, unsigned> name2origIdx;
  _traits = new vector<unsigned>[_nseqs];//unsigned*[_nseqs];
  
  for (unsigned i = 0; i < _origSeqs.size(); i++)
    name2origIdx[_origSeqs.at(i)->name()] = i;

  
  for (unsigned i = 0; i < _nseqs; i++)
  {
    if (! traitVect.empty())
    {
      _freqs[i] = 0;

      for (unsigned j = 0; j < traitVect.size(); j++)
        _traits[i].push_back(0);
      
    }
  }
  
  vector<Trait *>::const_iterator traitIt = traitVect.begin();
  unsigned nTraits = 0;
  
  while (traitIt != traitVect.end())
  {
    vector<string> seqNames = (*traitIt)->seqNames();
    vector<string>::const_iterator nameIt = seqNames.begin();
    _traitNames.push_back((*traitIt)->name());
    
    while (nameIt != seqNames.end())
    {
      map<string, unsigned>::const_iterator oidxIt = name2origIdx.find(*nameIt);
      
      if (oidxIt == name2origIdx.end())
      {
        throw NetworkError("Sequence names in trait vector do not match sequences in HapNet.");
      }
      
      unsigned oidx = oidxIt->second;
      unsigned nsamps = (*traitIt)->seqCount(*nameIt);
      
      _freqs[_orig2cond[oidx]] += nsamps;
      _traits[_orig2cond[oidx]].at(nTraits) += nsamps;
      
      ++nameIt;
    }
    
     nTraits++;
    ++traitIt;
  }
  
  
  if (nTraits != traitVect.size())
  {
    //cerr << "wrong trait count error." << endl;
    throw NetworkError("This shouldn't happen, but the traits count is wrong.");
  
  }
  
#ifdef NET_QT
  emit traitsChanged();
#endif
  // 
 // _freqs[_orig2cond[i]]++;
  // For each trait, 
  
  // For sequences that are identical, map these to the same traitcounter
  
  // A traits vector for each sequence
}

void HapNet::setupGraph()
{
  try
  {
    computeDistances();
    computeGraph();
    _isGraphSetup = true;
  }
#ifdef NET_QT
  catch (NetworkError &ne)
  {
    _errorMsg = ne.what();
    emit caughtException(_errorMsg);
  }

  catch (TreeError &te)
  {
    _errorMsg = te.what();
    emit caughtException(_errorMsg);
  }

  catch (exception &e)
  {
    _errorMsg = tr("An unknown error has occurred");
    emit caughtException(_errorMsg);
  }

  
  if (thread() != QApplication::instance()->thread())
    thread()->exit();
#else
  catch (exception)
  {
    throw();
  }
#endif
}

bool HapNet::isGraphSetup() const
{
  return _isGraphSetup;
}

void HapNet::computeDistances()
{
  for (unsigned i = 0; i < _nseqs; i++)
  {
    _distances[i * _nseqs + i] = 0;
    for (unsigned j = 0; j  < i; j++)
      _distances[i * _nseqs + j] = _distances[j * _nseqs + i] = pairwiseDistance(_condensedSeqs.at(i), _condensedSeqs.at(j));
  }
}

unsigned HapNet::pairwiseDistance(const string &seq1, const string &seq2) const
{
  unsigned dist = 0;

  unsigned seqlen = seq1.length();
  /*int seq2len = seq2.length();

  if (seq2len > seqlen)  dist += seq2len - seqlen;
  else if (seq2len < seqlen)
  {
    dist += seqlen - seq2len;
    seqlen = seq2len;
  }*/

  if (seq2.length() != seqlen)  throw NetworkError("Sequences are not the same length!");

  for (unsigned i = 0; i < seqlen; i++)
    if (Sequence::isAmbiguousChar(seq1.at(i), _datatype) || Sequence::isAmbiguousChar(seq2.at(i), _datatype))  continue;
    
    else if (seq1.at(i) != seq2.at(i))  
    {
      if (_datatype == Sequence::DNAType && 
         ((seq1.at(i) == 'R' && (seq2.at(i) == 'A' || seq2.at(i) == 'G')) ||
          (seq2.at(i) == 'R' && (seq1.at(i) == 'A' || seq1.at(i) == 'G')) ||
          (seq1.at(i) == 'Y' && (seq2.at(i) == 'C' || seq2.at(i) == 'T' || seq2.at(i) == 'U')) ||
          (seq2.at(i) == 'Y' && (seq1.at(i) == 'C' || seq1.at(i) == 'T' || seq1.at(i) == 'U'))))
        continue;
        
      else dist += weight(i);
    }

  return dist;
}

void HapNet::setDistance(unsigned dist, unsigned i, unsigned j)
{
  if (i >= nseqs() || j >= nseqs())  throw NetworkError("Invalid index for distance.");

  _distances[i * nseqs() + j] = dist;
}

unsigned  HapNet::distance(unsigned i, unsigned j) const
{
  if (i >= nseqs() || j >= nseqs())  throw NetworkError("Invalid index for distance.");

  return _distances[i * nseqs() + j];
}

size_t HapNet::nseqs() const
{
  return _nseqs;
}

size_t HapNet::nsites(bool isOrig) const
{
  // if asking about the length of input (pre-condensed) seqs...
  if (isOrig)  return _nsites;
  
  // otherwise, length of condensed seqs...
  return _nCsites;
}

/* Can only access original Sequence objects, condensed only exist as strings */
const Sequence * HapNet::seq(unsigned idx) const
{
  /*if ((isOrig && idx >= _origSeqs.size()) || (! isOrig && idx >= nseqs()))
    throw NetworkError("Sequence index out of range!");

  unsigned oIdx = (isOrig ? idx : _cond2orig.at(idx).at(0));*/
  
  if (idx >= _origSeqs.size())  throw NetworkError("Sequence index out of range!");

  return _origSeqs.at(idx);//oIdx);
}

const string & HapNet::seqName(unsigned idx, bool isOrig) const
{
  if ((isOrig && (idx >= _origSeqs.size())) || (! isOrig && (idx >= nseqs())))
    throw NetworkError("Sequence index out of range!");

  unsigned oIdx = (isOrig ? idx : _cond2orig.at(idx).at(0));

  return _origSeqs.at(oIdx)->name();
   
}

// If isOrig, return original sequence; otherwise, return condensed seq
const string & HapNet::seqSeq(unsigned idx, bool isOrig) const
{
  if ((isOrig && (idx >= _origSeqs.size())) || (! isOrig && (idx >= nseqs())))
    throw NetworkError("Sequence index out of range!");

  /*unsigned oIdx = (isOrig ? idx : _cond2orig.at(idx).at(0));

  return _origSeqs.at(oIdx)->seq();*/
  
  if (isOrig)  return _origSeqs.at(idx)->seq();
  
  else  return _condensedSeqs.at(idx);

}

// Careful, this returns a condensed sequence based on an original index
const string & HapNet::condensedSeqSeq(unsigned idx) const
{
  if (idx >= _origSeqs.size())
    throw NetworkError("Sequence index out of range!");
  
  return _condensedSeqs.at(_orig2cond[idx]);
}

/*const string & HapNet::cSeq(unsigned idx) const
{
  if (idx >= _condensedSeqs.size())  throw NetworkError("Condensed sequence index out of range!");
  
  return _condensedSeqs.at(idx);
}*/

unsigned HapNet::freq(unsigned idx) const
{
  if (idx >= _nseqs)  return 0;
    //throw NetworkError("Sequence index out of range!");


  return _freqs[idx];
}

const vector<unsigned> & HapNet::traits(unsigned idx) const
{
  if (idx >= _nseqs)  return _emptyTraits;//return NULL;
    //throw NetworkError("Sequence index out of range!");

  
  return _traits[idx];

}

// TODO maybe store this vector
vector<string> HapNet::identicalTaxa(unsigned idx) const
{
  vector<string> taxa;
  
  if (idx >= _cond2orig.size())  return taxa;
  
  for (unsigned i = 0; i < _cond2orig.at(idx).size(); i++)
    taxa.push_back(seqName(_cond2orig.at(idx).at(i), true));
  
  return taxa;
}

unsigned HapNet::weight(unsigned idx) const
{
  if (idx >= _nCsites)  throw NetworkError("Invalid site index given for weight.");

  return _weights.at(idx);
}

/*size_t HapNet::nTraits() const
{
  return _nTraits;
}*/

const std::vector<unsigned> & HapNet::weights() const
{
  return _weights;
}

const unsigned * HapNet::freqs() const
{
  return _freqs;
}

#ifdef NET_QT
/**
 * Progress should be a percentage (between 0 and 100)
 */
void HapNet::updateProgress(int progress)
{
  if (progress < 0 || progress > 100)  throw NetworkError("Progress is not a percentage.");
  emit progressUpdated(progress);
}
#else
void HapNet::updateProgress(int progress)
{
  cout << '.';
  if (progress < 0 || progress > 100)  throw NetworkError("Progress is not a percentage.");
  if (! progress % 10)  cout << "] " << progress << "%\n[";
  cout.flush();
}
#endif

HapNet::VertContainer::VertContainer(unsigned distance)//, const Node ** pair)
  : _distance(distance), _npairs(0)
{

  //if (pair != NULL)  _pairs.push_back(pair);
}

HapNet::VertContainer::~VertContainer()
{
  list<const Vertex**>::iterator pairIt = _pairs.begin();
  while (pairIt != _pairs.end())
  {
    delete [] (*pairIt);
    ++pairIt;
  }
  //for (unsigned i = 0; i < _pairs.size(); i++)
  //  delete [] _pairs.at(i);
}

void HapNet::VertContainer::addPair(const Vertex *u, const Vertex *v)
{
  const Vertex ** pair = new const Vertex*[2];
  pair[0] = u;
  pair[1] = v;
  _pairs.push_back(pair);
  _npairs++;
}

void HapNet::VertContainer::removePair(HapNet::VertContainer::Iterator &position)
{
  if (position != end())
  {
    const Vertex **pair = position.removePair();
    delete [] pair;
    _npairs--;
  }
}

void HapNet::VertContainer::insertPair(HapNet::VertContainer::Iterator &position, const Vertex *u, const Vertex *v)
{
  //if (position != end())
  //  position.removePair();
  
  const Vertex ** pair = new const Vertex*[2];
  pair[0] = u;
  pair[1] = v;
  position.insertPair(pair);
  _npairs++;
  
}


/*const vector<const Node**> & HapNet::NodeContainer::pairs() const
{
  return _pairs;
}*/

unsigned HapNet::VertContainer::distance() const
{
  return _distance;
}

/*const Vertex ** HapNet::VertContainer::at(unsigned index) const
{
  if (index >= size()) throw NetworkError("Vertex container index out of range.");
  
  return _pairs.at(index);
}*/

bool HapNet::VertContainer::operator<(const VertContainer & other) const
{
  bool retval = distance() < other.distance();
  
  return retval;
}

bool HapNet::VertContainer::operator>(const VertContainer & other) const
{
  bool retval = distance() > other.distance();

  return retval;
}


HapNet::VertContainer::Iterator HapNet::VertContainer::begin()
{
  return Iterator(this);
}

HapNet::VertContainer::Iterator HapNet::VertContainer::end()
{
  return Iterator(this, true);
}

HapNet::VertContainer::Iterator::Iterator(HapNet::VertContainer* container, bool isEnd)
{
  _isEnd = isEnd;
  _pairs = container->_pairs;

  if (! isEnd)  _pairIt = _pairs.begin();

  // unnecessary, but makes me feel better:
  else  _pairIt = _pairs.end();
}

const Vertex** HapNet::VertContainer::Iterator::removePair()
{
  if (isEnd())  
    throw NetworkError("Cannot remove a pair past the end.");
 
  const Vertex** pair = *_pairIt;
  _pairs.erase(_pairIt);
  
  return pair;
}

void HapNet::VertContainer::Iterator::insertPair(const Vertex** pair)
{
  if (isEnd())
    _pairs.push_back(pair);
  
  else
    _pairs.insert(_pairIt, pair);
}

bool HapNet::VertContainer::Iterator::isEnd() const
{
  return _isEnd;
}

HapNet::VertContainer::Iterator & HapNet::VertContainer::Iterator::operator++()
{
  _pairIt++;

  if (_pairIt == _pairs.end())  _isEnd = true;

  return *this;
}


/*
 * Need to write a copy constructor that works properly before doing this.
HapNet::NodeContainer::Iterator HapNet::NodeContainer::Iterator::operator++(int)
{
  Iterator newIt = *this;
  ++(*this);

  return newIt;
}*/

const Vertex ** HapNet::VertContainer::Iterator::operator*() const
{
  if (_isEnd)  return 0;
  else return (*_pairIt);
}

bool HapNet::VertContainer::Iterator::operator==(const HapNet::VertContainer::Iterator & other) const
{
  if (_isEnd && other.isEnd())  return true;
  else if (_isEnd)  return false;
  else return (*_pairIt) == (*other);
}

bool HapNet::VertContainer::Iterator::operator!=(const HapNet::VertContainer::Iterator &other) const
{
  if (_isEnd && other.isEnd())  return false;
  else if (_isEnd)  return true;
  else return (*_pairIt) != (*other);
}

bool HapNet::VCPtrComparitor::operator() (const VertContainer *lhs, const VertContainer *rhs) const
{
  if (_reverse) return (*lhs > *rhs);
  else return (*lhs < *rhs);
}

ostream &operator<<(ostream &os, const HapNet &n)
{
  if (n.isGraphSetup())
  {
    const Graph *g = &n;
    os << (*g);
  }

  else
  {
    os << "Graph has not been calculated." << endl;
  }

  return os;
}
