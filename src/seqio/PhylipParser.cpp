#include <sstream>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

#include "PhylipParser.h"
#include "SeqParseError.h"
#include "ParserTools.h"


PhylipSeqParser::PhylipSeqParser(PhylipVariant variant, int nseq, int nchar)
  : SeqParser::SeqParser()
{
  /*_expNseq = nseq;
  _expNchar = nchar;*/
  
  setNseq(nseq);
  setNchar(nchar);  
  
  _variant = variant;
  resetParser();
}

Sequence & PhylipSeqParser::getSeq(istream &input, Sequence &sequence)
{
  if (!seqsLoaded())
  {
    readSeqs(input);
    if (_seqvect.size() == 0)  return sequence;
    input.clear();
    _seqiter = _seqvect.begin();    
  }
  
  sequence = (*_seqiter++);
  if (_seqiter == _seqvect.end())
    input.setstate(ios_base::eofbit);

  return sequence;
}

void PhylipSeqParser::putSeq(ostream &output, const Sequence &sequence)
{
  if (!_headerwritten)
  {
    _headerwritten = true;
    if (nSeq() == 0)
    {
      cerr << "Warning: writing Phylip format with nseq unset.\n\n";
      cerr << "This is a non-fatal error, but probably not what you expect. Call setNchar() and\n";
      cerr << "setNseq() after creating a new parser to prevent seeing this message." << endl;
    }
    
    //output << " " << _expNseq << " " << _expNchar << "\n";
    output << " " << nSeq() << " " << nChar() << endl;
  }
  
  /* get rid of mid-name spaces */
  string name(sequence.name());
  for (int i = name.find(' '); i != name.npos; i = name.find(' ', i+1))
    name.replace(i, 1, 1, '_');
  
  switch (variant())
  {
    case (Interleaved):
      //throw SeqParseError("Cannot write Phylip interleaved format");
      cerr << "Cannot write Phylip interleaved format, switching to Sequential" << endl;
    case (Sequential):
      if (name.length() > 10)
        output << name.substr(0, 10);
      else
        output << setw(10) << name;
      output << " " << sequence.seq();
      break;
      
      case (Relaxed):
        output << name << "  " << sequence.seq();
        break;
        
    default:
      throw SeqParseError("Unknown Phylip variant");
      break;
  }
  
  if (sequence.length() < nChar())
    output << string(nChar() - sequence.length(), '-');
  
  output << endl;
}

void PhylipSeqParser::resetParser()
{
  setCharType(DNAType);
  _seqsloaded = false;
  _headerwritten = false;
  _lbaftername = false;
  _seqvect.clear();
}

bool PhylipSeqParser::readSeqsVariant(istream &input, PhylipVariant var)
{
  int start = input.tellg();
  unsigned seqcount = 0;

  string line;
  string strippedline;
  string name;

  switch(var)
  {
  case Relaxed:
    while (input.good())
    {
      vector<string> wordlist;
      getline(input, line, eolChar());
      ParserTools::rstrip(strippedline = line);
      if (strippedline.empty())
        continue;
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);
      if (wordlist.size() != 2)
      {
        _seqvect.clear();
        input.seekg(start);
        input.clear();
        return false;
      }
      ParserTools::strip(wordlist[0]);
      ParserTools::strip(wordlist[1]);
      if (wordlist[1].size() != nChar())
      {
        _seqvect.clear();
        input.seekg(start);
        input.clear();
        return false;
      }
      _seqvect.push_back(Sequence(wordlist[0], wordlist[1]));
    }
    return true;
    break;
  case Interleaved:
    while (input.good())
    {
      getline(input, line, eolChar());
      ParserTools::rstrip(strippedline = line);
      if (strippedline.empty())
        continue;
      
      if (strippedline.size() <= NAMELENGTH)
      {
        // If a line break appears after name, treat this as sequential
        // TODO figure out whether this is legal for interleaved as well!
        _lbaftername = true;
        _seqvect.clear();
        input.seekg(start);
        input.clear();
        return false;
      }
      
      if (seqcount < nSeq())
      {
        name = strippedline.substr(0, NAMELENGTH);
        ParserTools::strip(name); 
        string seq = strippedline.substr(NAMELENGTH);
        ParserTools::strip(seq);
        ParserTools::eraseChars(seq, ' ');
        if (seq.size() > nChar())
        {
          _seqvect.clear();
          input.seekg(start);
          input.clear();
          return false;
        }
        _seqvect.push_back(Sequence(name, seq));
      }

      else
      {
        unsigned idx = seqcount % nSeq();
        string seq = strippedline;
        ParserTools::strip(seq);
        ParserTools::eraseChars(seq, ' ');
        _seqvect.at(idx) += seq;
        if (_seqvect.at(idx).length() > nChar())
        {
          _seqvect.clear();
          input.seekg(start);
          input.clear();
          return false;
        }
      }
      seqcount++;
    }

    for (unsigned i = 0; i < _seqvect.size(); i++)
      if (_seqvect.at(i).length() != nChar())
      {
        _seqvect.clear();
        input.seekg(start);
        input.clear();
        return false;
      }

    return true;
    break;

  case Sequential:
    while (input.good())
    {
      getline(input, line, eolChar());
      ParserTools::rstrip(strippedline = line);
      if (strippedline.empty())
        continue;

      if (name.empty())
      {
        name = strippedline.substr(0, NAMELENGTH);
        ParserTools::strip(name);
        
        if (_lbaftername)
          _seqvect.push_back(Sequence(name, ""));
        
        else
        {
          string seq = strippedline.substr(NAMELENGTH);
          ParserTools::strip(seq);
          ParserTools::eraseChars(seq, ' ');
          _seqvect.push_back(Sequence(name, seq));
        }
      }

      else
      {
        string seq = strippedline;
        ParserTools::strip(seq);
        ParserTools::eraseChars(seq, ' ');
       _seqvect.back() += seq;
      }

      if (_seqvect.back().length() > nChar())
      {
        _seqvect.clear();
        input.seekg(start);
        input.clear();
        return false;
      }

      else if (_seqvect.back().length() == nChar())
        name.clear();
    }

    if (_seqvect.back().length() != nChar())
    {
      _seqvect.clear();
      input.seekg(start);
      input.clear();
      return false;
    }

    return true;
    break;

  default:
    break;
  }

  _seqvect.clear();
  input.seekg(start);
  input.clear();
  return false;
}


void PhylipSeqParser::readSeqs(istream &input)
{

  string header;
  getline(input, header, eolChar());
  istringstream iss(header);
  int nseq, nchar;
  iss >> nseq;
  setNseq(nseq);
  iss >> nchar;
  setNchar(nchar);
  
  if (nSeq() < 0 || nChar() < 0)  
    throw SeqParseError("Error reading Phylip header.");

  if (variant() != Unknown)
  {
    if (! readSeqsVariant(input, variant()))
      throw SeqParseError("Error reading specified Phylip variant.");
  }
  
  else
  {
    if (readSeqsVariant(input, Relaxed))
      setPhylipVariant(Relaxed);

    else if (readSeqsVariant(input, Interleaved))
      setPhylipVariant(Interleaved);

    else if (readSeqsVariant(input, Sequential))
      setPhylipVariant(Sequential);

    else  throw SeqParseError("Unable to determine Phylip variant.");
  }

  if (_seqvect.size() != nSeq())
    throw SeqParseError("Wrong number of sequences.");
    
  _seqsloaded = true;
}


PhylipSeqParser::PhylipVariant PhylipSeqParser::variant() const
{
  return _variant;
}

void PhylipSeqParser::setPhylipVariant(PhylipSeqParser::PhylipVariant variant)
{
  _variant = variant;
}

bool PhylipSeqParser::seqsLoaded() const
{
  return _seqsloaded;
}



