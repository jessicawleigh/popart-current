#include <sstream>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <iostream>

#include "PhylipParser.h"
#include "SeqParseError.h"


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
}


void PhylipSeqParser::readSeqs(istream &input)
{

  vector<string> wordlist; //*tokens;
  string header;
  getline(input, header);
  istringstream iss(header);
  int nseq, nchar;
  iss >> nseq;
  setNseq(nseq);
  iss >> nchar;
  setNchar(nchar);
  streampos postheader = input.tellg();
  
  if (nSeq() < 0 || nChar() < 0)  
    throw SeqParseError("Error reading Phylip header.");

  _seqvect = vector<Sequence>(nSeq());
  string line;
  string strippedline;
  string name;
  string seq;
  int expCharsPerLine = -1;
  const int NAMELENGTH = 10;
  int linecount = 0;
  bool startagain = false;
  bool isNewSeq = true;
  int seqcount = 0; // number of seqs read
  
  /* Identical loop conditions: if the inner loop finishes without 
     explicitly moving back to the beginning of the file, it was read
     in the correct format */
  while (input.good())
  {  
    while (input.good())
    {
      getline(input, line);
      strip(strippedline = line);
      if (strippedline.length() == 0) continue;
      switch(variant())
      {
        case Interleaved:
          strip(name = line.substr(0, NAMELENGTH));
          strip(seq = line.substr(NAMELENGTH));
          if (expCharsPerLine < 0)  expCharsPerLine = seq.length();

          if (linecount < nSeq())
          {
            if (strippedline.length() == expCharsPerLine || name.length() == 0)
            {
              setPhylipVariant(Sequential);
              seqcount--;
              if (seqcount < 0)  throw SeqParseError("Bad Phylip sequential format!");
              _seqvect[seqcount] += strippedline;
              linecount++;
              if (_seqvect[seqcount].length() > nChar())  
                throw SeqParseError("Bad Phylip sequential format!");
              else if (_seqvect[seqcount].length() == nChar()) 
              {
                seqcount++;
                isNewSeq = true;
              }
            }
          
            else if (seq.length() != expCharsPerLine)
            {
              setPhylipVariant(Relaxed);
              startagain = true;
            }
          
            else
            {
              _seqvect[linecount] = Sequence(name, seq);
              seqcount++;
              linecount++;
            }
          }
        
          else
          {
            _seqvect[linecount % nSeq()] += seq;
            linecount++;
          }
          
          break;

        case Sequential:
          if (isNewSeq)
          {
            strip(name = line.substr(0, NAMELENGTH));
            strip(seq = line.substr(NAMELENGTH));
            if (name.length() == 0 && linecount >= nSeq())
              throw SeqParseError("Bad format: perhaps Phylip interleaved?");
            else if (name.length() == 0 || seq.length() == 0)  
              throw SeqParseError("Bad Phylip sequential format!");
            else
              _seqvect[seqcount] = Sequence(name, seq);
            isNewSeq = false;
          }
        
          else
          {
            strip(seq = line);
            _seqvect[seqcount] += (seq);
          }
        
          if (_seqvect[seqcount].length() > nChar())  
          throw SeqParseError("Bad Phylip sequential format!");
          else if (_seqvect[seqcount].length() == nChar())  
          {
            isNewSeq = true;
            seqcount++;
          }
          linecount++;
          break;
        
        case Relaxed:
          //tokens =
          wordlist.clear();
          tokenise(wordlist, line);
          if (wordlist.size() > 0)
          {
            if (wordlist.size() != 2)  throw SeqParseError("Too many tokens for Phylip relaxed format");
            name = wordlist.at(0);
            seq = wordlist.at(1);
      
            if (name.length() == 0 || seq.length() != nChar())
              throw SeqParseError("Error reading sequence: expected relaxed Phylip format");
            _seqvect[seqcount++] = Sequence(name, seq);
          }
      }
    
      if (startagain)
      {
        linecount = 0;
        seqcount = 0;
        _seqvect = vector<Sequence>(nSeq());
        input.seekg(postheader);
        break;
      }
    
      if (seqcount > nSeq())  throw SeqParseError("Bad Phylip format!");
    }
    
    /* a problem has already been flagged, no need to look for more,
       but don't want to carry startagain flag into next attempt */
    if (startagain)  startagain = false;
    
    else
    {
      for (int i = 0; i < seqcount; i++)
      {
        if (_seqvect[i].length() != nChar())
        {
          if (variant() == Relaxed)
            throw SeqParseError("Bad Phylip relaxed format!");
          else
          {
            setPhylipVariant(Relaxed);
            linecount = 0;
            seqcount = 0;
            _seqvect = vector<Sequence>(nChar());
            input.seekg(postheader);
          }
        }
      }
    }
  }
  
  
  if (seqcount != nSeq())  
    throw SeqParseError("Bad Phylip header: wrong number of sequences");
  
  /* this is probably sequential format, one seq per line */
  if (variant() == Interleaved && seqcount == linecount)
    setPhylipVariant(Sequential);
    
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



