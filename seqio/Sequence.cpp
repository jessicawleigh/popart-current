/*
 * Sequence.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

using namespace std;

#include "Sequence.h"
#include "SequenceError.h"
#include "NexusParser.h"
#include "PhylipParser.h"

SeqParser* Sequence::_parser = 0;

Sequence::Sequence()
{
  _name = "";
  _seq = "";
  _chartype = DNAType;
}

Sequence::Sequence(const string seqname, const string sequence)
{
  _name = seqname;
  _seq = sequence;
   _chartype = DNAType;
}

Sequence::Sequence(const Sequence &other, bool revComp)
{
  _name = other.name();
  size_t len = other.length();

  if (revComp)
  {
    char *cseq = new char[len + 1];
    cseq[len] = '\0';
    char c, ccomp;

    for (unsigned i = 0; i < other.length(); i++)
    {
      c = other[i];
      switch(c)
      {
        case 'A':
          ccomp = 'T';
          break;
        case 'T':
          ccomp = 'A';
          break;
        case 'G':
          ccomp = 'C';
          break;
        case 'C':
          ccomp = 'G';
          break;
        case '-':
          ccomp = '-';
          break;
        case 'N':
          ccomp = 'N';
          break;
        case 'R':
          ccomp = 'Y';
          break;
        case 'Y':
          ccomp = 'R';
          break;
        case 'W':
          ccomp = 'W';
          break;
        case 'S':
          ccomp = 'S';
          break;
        case 'M':
          ccomp = 'K';
          break;
        case 'K':
          ccomp = 'M';
          break;
        case 'B':
          ccomp = 'V';
          break;
        case 'D':
          ccomp = 'H';
          break;
        case 'H':
          ccomp = 'D';
          break;
        case 'V':
          ccomp = 'B';
          break;
        default:
          ccomp = 'N';
          break;
      }
      cseq[len - i - 1] = ccomp;
    }

    _seq = string(cseq);
  }

  else  _seq = other.seq();
  
  _chartype = DNAType;
}

// Make translated copy of sequence
Sequence::Sequence(const Sequence &other, int readingFrame, GeneticCode gencode)
{
  _name = other.name();

  const Sequence *otherptr = &other;

  if (readingFrame < 0)
  {
    otherptr = new Sequence(other, true);
    readingFrame *= -1;
  }

  string otherstr = otherptr->seq();

  int newlength = (other.length() + 3 - readingFrame) / 3;

  char *cseq = new char[newlength + 1];
  cseq[newlength] = '\0';

  for (unsigned i = readingFrame - 1; i < otherstr.length(); i += 3)
  {
    if ((i + 3) <= otherstr.length())
    {
      cseq[i/3] = gencode[otherstr.substr(i, 3)];
    }

    else
      cseq[i/3] = 'X';
  }

  _seq = string(cseq);
}

Sequence::~Sequence()
{
	// TODO Auto-generated destructor stub
}


const string & Sequence::seq() const
{
  return _seq;
}

const string & Sequence::name() const
{
  return _name;
}

char & Sequence::at(size_t pos)
{
  if (pos >= _seq.length())  throw SequenceError("Sequence index out of range.");
  
  return _seq.at(pos);
}

const char & Sequence::at(size_t pos) const
{
  if (pos >= _seq.length())  throw SequenceError("Sequence index out of range.");
  
  return _seq.at(pos);
}  


string Sequence::subseq(int start, int slicelen) const
{

  if (slicelen < 0)
    return _seq.substr(start);
  else
    return _seq.substr(start, slicelen);
}


void Sequence::setSeq(const string newseq)
{
  _seq = newseq;
}

void Sequence::setName(const string newname)
{
  _name = newname;
}
void Sequence::setParser(SeqParser *parser)
{
  _parser = parser;
}

void Sequence::setParser(istream &input)
{
  if (input.eof())  throw SequenceError("Cannot guess sequence format from an empty file!");
  char c = input.peek();
  
  /*if (c == '>')  setParser(new FastaSeqParser());

  else */
  if (c == '#')  //setParser(new NexusSeqParser());
  {
    string line;
    getline(input, line);
    size_t index = SeqParser::caselessfind("nexus", line);
    if (index != string::npos)
    {
      setParser(new NexusParser());
      if (line.at(line.length() - 1) == '\r')
        parser()->setEOLChar('\r');
    }

    /*else if ((index = SeqParser::caselessfind("stockholm", line)) != string::npos)
      setParser(new StockholmSeqParser());*/

    else
      throw SequenceError("Unable to guess alignment type!");

    input.seekg(ios_base::beg);
    input.clear();
  }

  else
  {
    int nseq = -1, nchar = -1;
    input >> nseq >> nchar;
    input.seekg(ios_base::beg);
    if (nseq > 0 && nchar > 0)
      setParser(new PhylipSeqParser());
    else
    {
      /*cerr << "Error, unable to guess sequence type!" << endl;
      setParser(0);*/
      throw SequenceError("Unable to guess alignment type!");
    }
  }
}

SeqParser* Sequence::parser()
{
  return _parser;
}

size_t Sequence::length() const
{
  return _seq.length();
}

void Sequence::clear()
{
  _seq.clear();
  _name.clear();
}

const string & Sequence::replace(size_t pos1, size_t n1, const string &str)
{
  _seq.replace(pos1, n1, str);
  return _seq;
}

void Sequence::delChar(const int index)
{
  delCharRange(index, 1);
}

/* if slicelen is < 0, sequence is deleted from start on */
void Sequence::delCharRange(const unsigned start, const int slicelen)
{
  size_t seqlen = length();
  string newseq;

  // Should be an exception
  if (((int)(start + slicelen) > (int)seqlen) || (start > seqlen))
    throw SequenceError("Index out of range.");

  if (slicelen >= 0)
    _seq.erase(start, slicelen);

  else
    _seq.erase(start);

  /*newseq = _seq.substr(0, start);
  if (slicelen >= 0)
    newseq += _seq.substr(start + slicelen, seqlen - (start + slicelen));

  setseq(newseq);*/
}

void Sequence::insertGaps(const unsigned start, const unsigned ngaps)
{
  char *gaps = new char[ngaps + 1];
  for (unsigned i = 0; i < ngaps; i++)
    gaps[i] = '-';
  gaps[ngaps] = '\0';

  insertChars(start, gaps);

  delete gaps;
}

void Sequence::insertChars(const unsigned start, const string substr)
{
  if (length() <= start)
    pad(start + 1);
  _seq.insert(start, substr);
}

void Sequence::pad(unsigned newlength, char padchar)
{
  if (length() < newlength)
    _seq.append (newlength - length(), padchar);
}

void Sequence::maskChars(const vector<bool> & mask)
{
  //vector<bool>::const_iterator maskIter = mask.begin();

  vector<bool>::const_reverse_iterator maskIter = mask.rbegin();
  int stridx = length() - 1;
  //int maskRegionStart;
  int maskRegionEnd;
  bool inMaskRegion = false;

  //while (stridx < length() && maskIter != mask.end())
  while (stridx >= 0 && maskIter != mask.rend())
  {
    if (*maskIter)
    {
      if (inMaskRegion)
      {
        //delCharRange(maskRegionStart, stridx - maskRegionStart);
        delCharRange(stridx + 1, maskRegionEnd - stridx);
        inMaskRegion = false;
      }
    }

    else
    {
      if (! inMaskRegion)
      {
        inMaskRegion = true;
        //maskRegionStart = stridx;
        maskRegionEnd = stridx;
      }
    }

    //stridx++;
    stridx--;
    maskIter++;
  }
  
  if (inMaskRegion)
    delCharRange(0, maskRegionEnd + 1);
}

char Sequence::operator[](unsigned index) const
{
  if (index >= length())  throw SequenceError("Index out of range.");

  return _seq[index];
}

char &Sequence::operator[](unsigned index)
{
  if (index >= length())  throw SequenceError("Index out of range.");

  return _seq[index];
}

Sequence &Sequence::operator+=(const string &s)
{
  _seq += s;

  return (*this);
}

/* note that this doesn't look at names, only sequence data. */
bool Sequence::operator==(const Sequence &other) const
{
  return _seq == other.seq();
}

bool Sequence::operator<(const Sequence &other) const
{
  return _seq < other.seq();
}

bool Sequence::operator>(const Sequence &other) const
{
  return _seq > other.seq();
}

bool Sequence::operator==(const string &s) const
{
  return &_seq == &s;
}

bool Sequence::operator!=(const Sequence &other) const
{
  return !(this == &other);
}

bool Sequence::operator!=(const string &s) const
{
  return !(&_seq == &s);
}

bool Sequence::isAmbiguousChar(char c, CharType datatype)
{
 
  if (c == '-')  return true;
  switch(datatype)
  {
    case AAType:
      if (c == 'X')  return true;
      break;
    // for nucleotides: include IUPAC ambiguity codes
    case DNAType:
      if (c == 'N' || c == 'Y' || c == 'R' || c == 'M' || c == 'S' ||
          c == 'V' || c == 'W' || c == 'K' || c == 'D' || c == 'H' ||
          c == 'B')
        return true;
      break;
    case BinaryType:
      if (c == '?')  
      {
        return true;
      }
      break;
    default:
      return false;
      break;
  }
  
  return false;
}

bool Sequence::isValidChar(char c, CharType datatype)
{
  switch (datatype)
  {
    case AAType:
      switch (c)
      {
        /*  This order looks somewhat arbitrary. To reduce the average number
         *  of comparisons, these are based in descending order of frequency
         *  in UniProt as of August 2009.
         */
         case 'L': case 'A': case 'G': case 'V': case 'E': case 'S': case 'I':
         case 'K': case 'R': case 'D': case 'T': case 'P': case 'N': case 'Q':
         case 'F': case 'Y': case 'M': case 'H': case 'C': case 'W':
         case '-': case 'X':
          return true;
          break;
        default:
          return false;
      }
      break;
    case DNAType:
      switch (c)
      {
        case 'A': case 'T': case 'G': case 'C': case 'N': case 'U':
        case 'Y': case 'R': case '-':
          return true;
          break;
        default:
          return false;
      }
      break;
    case BinaryType:
      switch(c)
      {
        case '0': case '1': case '-': case '?':
          return true;
        default:
          return false;
      }
      break;
    default:
      throw SequenceError("Invalid character type.");
  }

  return false;
}

ostream &operator<<(ostream &output, const Sequence &seq)
{

  if (!seq.parser())  throw SequenceError("No parser/writer set");
  /*output << ">" << seq.name() << endl;
  const string sequence = seq.seq();

  for (unsigned int i = 0; i < sequence.length(); i+=CHARSPERLINE)
  {
    for (unsigned int j = i; (j < i + CHARSPERLINE) && (j < sequence.length()); j++)
      output << sequence[j];

    output << endl;
  }  */

  seq.parser()->putSeq(output, seq);

  return output;
}

istream &operator>>(istream &input, Sequence &seq)
{
  if (!seq.parser())
    seq.setParser(input);

  if (input.good())
    seq.parser()->getSeq(input, seq);

  return input;
}

