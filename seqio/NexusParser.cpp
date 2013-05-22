/*
 * NexusParser.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */

#include <sstream>
using namespace std;

#include "NexusParser.h"
#include "SeqParseError.h"
#include "ParserTools.h"

int NexusParser::_seqidx = -1;


NexusParser::NexusParser()
{
  resetParser();
  setupMaps();
}


NexusParser::~NexusParser() 
{
  if (_keystr)
    delete _keystr;
   
  if (_treestr)
    delete _treestr;
  
  _kwdMap.clear();
  _blockMap.clear();
  for (unsigned i = 0; i < _traits.size(); i++)
    delete _traits.at(i);
  _traits.clear();
  _traitNames.clear();
}


void NexusParser::resetParser()
{
  
  _headerWritten = false;
  _firstLineRead = false;
  _seqReadCount = 0;
  _taxonCount = 0;
  _seqWriteCount = 0;
  _gap = '-';
  _missing = '?';
  _missingTrait = '?';
  _match = '.';
  _traitSep = ' ';
  _respectCase = false;
  _inComment = false;
  _inSeq = false;
  _interleave = false;
  _traitsLabeled = false;
  _newTaxa = false;
  _taxLabels = false;
  _ntraits = 0;

  _currentBlock = NoBlock;
  _currentKeyWord = NoKwd;
  _seqidx = -1;

  setCharType(StandardType);//DNAType);
  setNchar(0);
  setNseq(0);

  //_taxa.clear();
  //_keystr.clear();
  //_treestr.clear();
  _treestr = 0;
  _keystr = 0;
  _taxonIndexMap.clear();
  _taxonVect.clear();
  _topSeq.clear();
  _taxSets.clear();
  _seqSeqVect.clear();
  _exSets.clear();
  _treeTaxMap.clear();

  for (unsigned i = 0; i < _trees.size(); i++)
    delete _trees.at(i);
  _trees.clear();

  for (unsigned i = 0; i < _traits.size(); i++)
    delete _traits.at(i);
  _traits.clear();
  _traitNames.clear();
}

void NexusParser::setupMaps()
{
  _kwdMap["begin"] = Begin;
  _kwdMap["end"] = End;
  _kwdMap["dimensions"] = Dimensions;
  _kwdMap["format"] = Format;
  _kwdMap["matrix"] = Matrix;
  _kwdMap["taxlabels"] = TaxLabels;
  _kwdMap["traitlabels"] = TraitLabels; 
  _kwdMap["translate"] = Translate;
  _kwdMap["tree"] = TreeKW;
  _kwdMap["charstatelabels"] = CharstateLabels;


  _blockMap["data"] = Data;
  _blockMap["taxa"] = Taxa;
  _blockMap["characters"] = Characters;
  _blockMap["trees"] = Trees;
  _blockMap["assumptions"] = Assumptions;
  //_blockMap["codons", Codons));
  //_blockMap["notes", Notes));
  _blockMap["unaligned"] = Unaligned;
  //_blockMap["distances", Distances));
  //_blockMap["paup", Paup));
  //_blockMap["mrbayes", MrBayes));
  //_blockMap["st_splits", StSplits));
  //_blockMap["ha_traits", HaTraits));  
  _blockMap["traits"] = Traits;
  _blockMap["sets"] = Sets;


}

Sequence & NexusParser::getSeq(istream &input, Sequence &sequence)
{

  string line;
  size_t commentstart;
  size_t semicolonpos;
  vector<string> wordlist;
  vector<string>::iterator worditer;
  string word;
  strblockmap::const_iterator blockResult;
  strkwdmap::const_iterator kwdResult;

  if (! _interleave || _seqidx < 0)
  {
    while (input.good())
    {
      getline(input, line, eolChar());

      if (! _firstLineRead)
      {
        size_t headerstart = ParserTools::caselessfind("#nexus", line);
        if (headerstart == string::npos)  throw SeqParseError("No Nexus header!");

        else  _firstLineRead = true;
      }

      else
      {
        do
        {
          _inComment = cleanComment(line, _inComment);
          commentstart = line.find('[');
        } while (commentstart != string::npos);

        ParserTools::strip(line);

        if (! line.empty() && ! _inComment)
        {
          if (_currentKeyWord == NoKwd)
          {
            wordlist.clear();
            ParserTools::tokenise(wordlist, line);
            worditer = wordlist.begin();
            if (worditer == wordlist.end())
              throw SeqParseError("Empty wordlist, but not empty line. This shouldn't happen.");

            ParserTools::lower(word = *worditer);
            if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
            kwdResult = _kwdMap.find(word);
            if (kwdResult == _kwdMap.end())
              _currentKeyWord = Unknown;

            else
              _currentKeyWord = kwdResult->second;

            parseLine(line, sequence);
            if ((semicolonpos = line.find(';')) != string::npos)
            {
              if (semicolonpos != (line.length() - 1))
                throw SeqParseError("Semi-colon should appear at the end of a line!");

              _currentKeyWord = NoKwd;
            }
          }

          else
          {
            parseLine(line, sequence);

            if (line.find(';') != string::npos)  _currentKeyWord = NoKwd;
            if ((_currentBlock == Data || _currentBlock == Characters) &&
                _currentKeyWord == Matrix)
            {
              
              if (!_interleave)
              {
                if (sequence.length() == nChar())
                {

                  if (_topSeq.empty())  _topSeq.setSeq(sequence.seq());
  
                  else
                  {
                    string fixedseq = sequence.seq();

                    for(unsigned i = 0; i < fixedseq.length() && i < _topSeq.length(); i++)
                    {
                      if (fixedseq.at(i) == _match)
                        fixedseq.at(i) = _topSeq.at(i);
                    }
                    sequence.setSeq(fixedseq);
                  }
                
                  _inSeq = false;
                  _seqReadCount++;
                  if (_seqReadCount < nSeq())
                    return sequence;
                }
                
                else if (sequence.length() < nChar())
                  _inSeq = true;
                
                else 
                {
                  cerr << "about to throw exception. nchar: " << nChar() << " chars read: " << sequence.length() << " seq name: " << sequence.name() << endl;
                  cerr << "line: " << line << endl;
                  throw SeqParseError("Too many characters read!");
                }
              }
            }
          }
        }
      }
    }
  }
  
  if (_interleave)
  {
    if (_seqidx < 0)
    {
      input.clear();
      input.seekg(ios_base::beg);
       _seqidx = 0;
    }
    
    sequence.setName(_taxonVect.at(_seqidx));
    
    if (sequence.name() != _topSeq.name())
    {
      string fixedseq = _seqSeqVect.at(_seqidx);

      for(unsigned i = 0; i < fixedseq.length() && i < _topSeq.length(); i++)
      {
        if (fixedseq.at(i) == _match)
          fixedseq.at(i) = _topSeq.at(i);
      }
      
      sequence.setSeq(fixedseq);
    }

    else   
      sequence.setSeq(_seqSeqVect.at(_seqidx));
    
    _seqidx++;
    
    if (_seqidx >= nSeq())
    {
      input.seekg(ios_base::end);
      input.setstate(ios_base::eofbit);
    }
    
    return sequence;
  }
  
  _seqReadCount = 0;
  
  return sequence;
}

const vector<Tree *> & NexusParser::treeVector() const
{
  return _trees;
}

const vector<Trait *> & NexusParser::traitVector() const
{
  return _traits;
}

void NexusParser::parseLine(string line, Sequence &sequence)
{

  vector<string> wordlist; 
  vector<string>::iterator worditer;
  string word;


  if (line == ";")  return;
            
  fixEquals(line);

  switch (_currentKeyWord)
  {
    case Begin:
    {
      ParserTools::lower(line);
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);
      worditer = wordlist.begin() + 1;

      if (worditer == wordlist.end()) 
        throw SeqParseError("Block name required after 'begin'!");

      word = *worditer;
      if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
      strblockmap::const_iterator blockResult = _blockMap.find(word);

      if (blockResult == _blockMap.end())
        _currentBlock = Other;

      else
        _currentBlock = blockResult->second;
      
      if (_currentBlock == Traits)
        _taxonCount = 0;
      
    }
    break;

    case (End):
    {
      if (_currentBlock == NoBlock)  throw SeqParseError("End keyword found outside block!");

      _currentBlock = NoBlock;
    }
    break;

    case Dimensions:
    {
      if (_currentBlock == Other)  return;
      else if (_currentBlock == Taxa || _currentBlock == Characters ||
          _currentBlock == Data || _currentBlock == Unaligned ||_currentBlock == Traits) //|| _currentBlock == Distances)
      {
        ParserTools::lower(line);
        //fixEquals(line);

        if (line.at(line.length() - 1) == ';')  line.erase(line.length() - 1);

        string key, val;
        istringstream iss;
        size_t eqpos;
        wordlist.clear(); 
        ParserTools::tokenise(wordlist, line);
        worditer = wordlist.begin();
        if (*worditer == "dimensions")  worditer++;

        //bool newtaxa = false;

        while (worditer != wordlist.end())
        {
          eqpos = (*worditer).find('=');

          if (eqpos == string::npos)
          {
            key = string(*worditer);
            val.clear();
          }

          else
          {
            key = (*worditer).substr(0, eqpos);
            val = (*worditer).substr(eqpos + 1);
          }

          if (! val.empty())
            iss.str(val + ' ');

          if (key == "nchar")
          {
            int nchar;
            if (val.empty())  throw SeqParseError("No value given for nchar!");
            if (_currentBlock == Taxa || _currentBlock == Unaligned)
              throw SeqParseError("nchar not allowed in taxa or unaligned blocks!");

            iss >> nchar;
            setNchar(nchar);

          }

          else if (key == "ntax")
          {
            if (val.empty())  throw SeqParseError("No value given for ntax!");

            if (_newTaxa)
            {
              int newseqs;
              iss >> newseqs;
              setNseq(nSeq() + newseqs);
              _seqSeqVect.resize(nSeq());
            }

            else
            {
              int nseq;
              if (_currentBlock == Characters)  throw SeqParseError("ntax only allowed in characters block along with newtaxa!");

              iss >> nseq;
              setNseq(nseq);
              _seqSeqVect.resize(nseq);
            }
          }

          else if (key == "newtaxa")
          {
            if (! val.empty() || _currentBlock == Taxa)
              throw SeqParseError("Invalid use of newtaxa directive!");

            _newTaxa = true;
          }
          
          else if (key == "ntraits")
          {
            int ntraits;
            if (val.empty() || _currentBlock != Traits)
              throw SeqParseError("ntraits can only be used in Traits block.");
            iss >> ntraits;
            _ntraits = ntraits;
          }

          else  throw SeqParseError("Invalid directive!");

          worditer++;
        }      
      }

      else  throw SeqParseError("Dimensions keyword not allowed in this block!");
    }
    break;

    case Format:
    {
      if (_currentBlock == Other)  return;
      else if (_currentBlock == Characters || _currentBlock == Data ||
               _currentBlock == Unaligned || _currentBlock == Traits) // _currentBlock == Distances)
      {
        ParserTools::lower(line);
        //fixEquals(line);

        if (line.at(line.length() - 1) == ';')  line.erase(line.length() - 1);

        string key, val;
        bool inquotedval = false;
        istringstream iss;
        size_t eqpos, quotepos;
        char quotechar = '\0';
        wordlist.clear();
        ParserTools::tokenise(wordlist, line);
        worditer = wordlist.begin();
        if (*worditer == "format")  worditer++;

        while (worditer != wordlist.end())
        {
          if (inquotedval)
          {
            if (quotechar != '\0')
              quotepos = (*worditer).find(quotechar);
            else
              quotepos = (*worditer).find_first_of("\"'");
            if (quotepos != string::npos)
            {
              if  (quotechar != '\0')  
                quotechar = (*worditer).at(quotepos);
              
              val += " " + (*worditer).substr(0, quotepos);
              inquotedval = false;
            }

            else
            {
              val += " " + *worditer;
              ++worditer;
              continue;
            }
          }

          else
          {
            eqpos = (*worditer).find('=');
            if (eqpos == (*worditer).size() - 1)
              throw SeqParseError("Nexus keyword found without a required value!");

            if (eqpos == string::npos)
            {
              key = string(*worditer);
              val.clear();
            }

            else
            {
              key = (*worditer).substr(0, eqpos);

              if ((*worditer).at(eqpos + 1) == '"' || (*worditer).at(eqpos + 1) == '\'')
              {
                quotechar = (*worditer).at(eqpos + 1);
                size_t lastidx = (*worditer).length() - 1;
                if ((*worditer).at(lastidx) == quotechar)
                  val = (*worditer).substr(eqpos + 2, lastidx);
                else
                {
                  cout << "last character is not a quote. last char: " << (*worditer).at(lastidx) << endl;
                  val = (*worditer).substr(eqpos + 2);
                  inquotedval = true;
                  ++worditer;
                  continue;
                }
              }

              else  val = (*worditer).substr(eqpos + 1);

            }
          }

          if (! val.empty())
            iss.str(val);

          if (key == "datatype")
          {
            if (val == "dna" || val == "rna" || val == "nucleotide")
              setCharType(DNAType);
            else if (val == "protein")
              setCharType(AAType);
            else if (val == "standard")
              setCharType(StandardType);
            else  throw SeqParseError("Non-molecular sequence type found!");
          }

          else if (key == "respectcase")
            _respectCase = true;

          else if (key == "missing")
          {
            if (val.empty())  throw SeqParseError("No value given for missing char!");
            iss >> _missing;
          }

          else if (key == "gap")
          {
            if (val.empty())  throw SeqParseError("No value given for gap char!");
            iss >> _gap;
          }

          /* Should maybe store symbols, but adds substantial overhead */
          else if (key == "symbols")
            warn("Ignoring symbols directive.");

          /* Not ideal; should support this */
          else if (key == "equate")
            warn("Ignoring equate directive.");

          else if (key == "matchchar" || key == "match")
            iss >> _match;

          else if (key == "labels") 
          {
            if (_currentBlock == Traits)
            {
              if (val.empty()) 
              {
                cerr << "line: " << line << endl;
                throw SeqParseError("Trait labels must be specified yes or no.");
              }
              if (val == "yes")
                _traitsLabeled = true;
              else  if (val == "no")
                _traitsLabeled = false;
              else  throw SeqParseError("Trait labels must be specified yes or no.");
            }
          }

          else if (key == "nolabels")
            throw SeqParseError("Unable to parse Nexus with nolabels!");
          
          else if (key == "separator")
          {
            if (val == "comma")  _traitSep = ',';
            else if (val == "spaces")  _traitSep = ' ';
            else if (val == "tab")  _traitSep = '\t';
            else throw SeqParseError("Trait separator must be tab, spaces, or comma.");
          }

          else if (key == "transpose")
          {
            if (val == "yes")  throw SeqParseError("Unabled to parse transposed Nexus format!");
            else 
              warn("Ignoring transpose directive.");
          }

          else if (key == "interleave")
          {            
            if (val == "yes" || val.empty())
              _interleave = true;
          }

          else if (key == "items")
            warn("Ignoring items directive.");

          else if (key == "statesformat")
            warn("Ignoring statesformat directive.");

          else if (key == "tokens")
            warn("Ignoring statesformat directive.");

          /* notokens is the default for molecular datatypes */
          else if (key == "notokens") ;

          else  throw SeqParseError("Unrecognised subcommand in format description!");

          worditer++;
        }
        
      }

      else throw SeqParseError("Format keyword not allowed in this block!");
      break;

    case Matrix:
      if (_currentBlock == Other)  return;
      else if (_currentBlock == Traits)
      {
        if (ParserTools::caselessfind("matrix", line) == string::npos)
        {
          if (line.at(line.length() - 1) == ';')
            line.erase(line.length() - 1);
          
          string traitstr;
          string seqname;

          if (_traitsLabeled)
          {
            size_t quotestart = line.find_first_of("\"'");

            if (quotestart != string::npos)
            {
              char quotechar = line.at(quotestart);
              size_t quoteend = line.find(quotechar, quotestart + 1);
              if (! quoteend)
                throw SeqParseError("Quoted taxon name has no end quote!");

              seqname = line.substr(quotestart + 1, quoteend - quotestart - 1);
              //ParserTools::replaceChars(seqname, ' ', '_');
              ParserTools::strip(seqname);
              traitstr = line.substr(quoteend + 1);
              ParserTools::eraseChars(traitstr, ' ');
              ParserTools::eraseChars(traitstr, '\t');
            }
            
            else
            {
              size_t nameend = line.find_first_of(" \t");
              if (nameend == string::npos)
              {
                throw SeqParseError("No space separating sequence and name!");
              }

              seqname = line.substr(0, nameend);
              traitstr = line.substr(nameend);
              ParserTools::eraseChars(traitstr, ' ');
              ParserTools::eraseChars(traitstr, '\t');
            }
            
            if (seqname.empty())  throw SeqParseError("Taxon name empty.");
            map<string, int>::iterator taxonIt = _taxonIndexMap.find(seqname);
            if (taxonIt == _taxonIndexMap.end())
              throw SeqParseError("Taxon found in Traits not defined in prior TaxLabels or Data blocks.");
          }
          
          else // traits unlabeled
          {
            if (_taxonCount >= _taxonVect.size())  
              throw SeqParseError("Too many rows in Traits block.");
            seqname = _taxonVect.at(_taxonCount++);
            traitstr = line;
            ParserTools::eraseChars(traitstr, ' ');
            ParserTools::eraseChars(traitstr, '\t');
          }
          
          int counter = 0;
          wordlist.clear();
          string sep(1, _traitSep);
          ParserTools::tokenise(wordlist, traitstr, sep);
          worditer = wordlist.begin();
          int nsamples;
          istringstream iss;
         
          while (worditer != wordlist.end())
          {
            if (counter > _traitNames.size())  
              throw SeqParseError("Too many columns in Traits block.");
            if (_traits.size() <= counter)
              _traits.push_back(new Trait(_traitNames.at(counter)));
            
            // treat missing trait data as 0 samples
            if ((*worditer).at(0) == _missingTrait)
              nsamples = 0;
            
            else
            {
              iss.str(*worditer + ' ');
              iss >> nsamples;
            }
            
            if (nsamples > 0)
              _traits.at(counter)->addSeq(seqname, nsamples);
            
            counter++;
            ++worditer;
          }
        }
      }
      
      else if (_currentBlock == Characters || _currentBlock == Data ||
               _currentBlock == Unaligned)
      {
        if (ParserTools::caselessfind("matrix", line) == string::npos)
        {
          if (line.at(line.length() - 1) == ';')  line.erase(line.length() - 1);

          size_t quotestart = line.find_first_of("\"'");

          if (quotestart != string::npos)
          {
            if (_inSeq)  throw SeqParseError("Quotes found mid-sequence! Is nchar correct?");
            char quotechar = line.at(quotestart);
            size_t quoteend = line.find(quotechar, quotestart + 1);
            if (! quoteend)
              throw SeqParseError("Quoted taxon name has no end quote!");

            string seqname = line.substr(quotestart + 1, quoteend - quotestart - 1);
            //ParserTools::replaceChars(seqname, ' ', '_');
            ParserTools::strip(seqname);
            
            // if there's a TaxLabels block, and no newtaxa directive, check that this label matches
            // TODO count taxa that don't match to make sure they don't exceed newtaxa ntax
            int taxIdx = -1;
            if (_taxLabels && ! _newTaxa)
            {
              map<string, int>::iterator taxIt = _taxonIndexMap.find(seqname);
              
              if (taxIt == _taxonIndexMap.end())  
                throw SeqParseError("Taxon labels must match those in TaxLabels exactly.");
              
              taxIdx = taxIt->second;
            }
            
            if (taxIdx < 0)
            {
              taxIdx = _taxonVect.size();
              _taxonIndexMap[seqname] = taxIdx;
              _taxonVect.push_back(seqname);
            }
            
            if (! _interleave && ! _inSeq)
              sequence.setName(seqname);

            string seq = line.substr(quoteend + 1);
            ParserTools::eraseChars(seq, ' ');
            ParserTools::eraseChars(seq, '\t');

            if (_gap != '-')
              ParserTools::replaceChars(seq, _gap, '-');

            if (charType() == DNAType && tolower(_missing) != 'n')
            {
              ParserTools::replaceChars(seq, toupper(_missing), 'N');
              ParserTools::replaceChars(seq, tolower(_missing), 'n');
            }

            else if (charType() == AAType && tolower(_missing) != 'x')
            {
              ParserTools::replaceChars(seq, toupper(_missing), 'X');
              ParserTools::replaceChars(seq, tolower(_missing), 'x');
            }
            
            else if (charType() == StandardType && _missing != '?')
            {
              // consider upper and lower case, in case missing is a letter
              ParserTools::replaceChars(seq, toupper(_missing), '?');
              ParserTools::replaceChars(seq, tolower(_missing), '?');
            }

            if ( !_interleave)
            {
              if (_inSeq)  sequence += seq;
              else sequence.setSeq(seq);
              if (_topSeq.empty())
              {
                _topSeq.setName(seqname);
                _topSeq.setSeq(seq);
                
              }
            }
            
            else
            {
              if (_seqSeqVect.size() <= taxIdx)
                throw new SeqParseError("More seq names than specified by nseq.");
              _seqSeqVect.at(taxIdx) += seq;
              
              
              if (_topSeq.name() == seqname)
                _topSeq += seq;
             }
          }

          else // taxon name not quoted
          {
            string seqname;
            string seq;
            int taxIdx = -1;
            
            if (_inSeq)
            {
              seqname = sequence.name();
              seq = line;
              ParserTools::eraseChars(seq, ' ');
              ParserTools::eraseChars(seq, '\t'); // probably shouldn't happen
            }
            
            else
            {
              size_t nameend = line.find_first_of(" \t");
              if (nameend == string::npos)
              {
                if (_interleave)
                  throw SeqParseError("No space separating sequence and name!");
                seqname = line; 
              }

              else
              {
                //string 
                seqname = line.substr(0, nameend);
                //string 
                seq = line.substr(nameend);
                ParserTools::eraseChars(seq, ' ');
                ParserTools::eraseChars(seq, '\t');
              }

              
              if (_taxLabels && ! _newTaxa)
              {
                map<string, int>::iterator taxIt = _taxonIndexMap.find(seqname);
                
                if (taxIt == _taxonIndexMap.end())  
                  throw SeqParseError("Taxon labels must match those in TaxLabels exactly.");
                
                taxIdx = taxIt->second;
              }
              
              if (taxIdx < 0)
              {
                taxIdx = _taxonVect.size();
                _taxonIndexMap[seqname] = taxIdx;
                _taxonVect.push_back(seqname);
              }

              if (! _interleave)
                sequence.setName(seqname);
              
             } // end not _inSeq

            if (_gap != '-')
              ParserTools::replaceChars(seq, _gap, '-');

            if (charType() == DNAType && tolower(_missing) != 'n')
            {
              ParserTools::replaceChars(seq, toupper(_missing), 'N');
              ParserTools::replaceChars(seq, toupper(_missing), 'n');
            }

            else if (charType() == AAType && tolower(_missing) != 'x')
            {
              ParserTools::replaceChars(seq, toupper(_missing), 'X');
              ParserTools::replaceChars(seq, tolower(_missing), 'x');
            }
            
            else if (charType() == StandardType && _missing != '?')
            {
              ParserTools::replaceChars(seq, toupper(_missing), '?');
              ParserTools::replaceChars(seq, tolower(_missing), '?');
            }

            if ( !_interleave)
            {
              if (_inSeq) sequence += seq;
              else  sequence.setSeq(seq);
            }
            
            else
            {
              if (_seqSeqVect.size() <= taxIdx)
                throw new SeqParseError("More seq names than specified by nseq.");
              _seqSeqVect.at(taxIdx) += seq;
            }
            
            if (_topSeq.empty())
            {
              _topSeq.setName(seqname);
              _topSeq.setSeq(seq);
              
            }
            
            else if (_topSeq.name() == seqname)
              _topSeq += seq;

          }
        }
      }
      else  throw SeqParseError("Matrix keyword not allowed in this block!");
      }

      break;

    case TaxLabels:
    {
      if (_currentBlock == Other)  return;

      else if (_currentBlock == Taxa || _currentBlock == Data || _currentBlock == Characters)
      {

        _taxLabels = true;
        wordlist.clear();
        ParserTools::tokenise(wordlist, line);
        worditer = wordlist.begin();

        if (worditer == wordlist.end())
          throw SeqParseError("Empty word list, but line isn't empty!");

        ParserTools::lower(word = (*worditer));
        if (word == "taxlabels")  worditer++;

        while (worditer != wordlist.end())
        {
          word = *worditer;
          if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
          if (! word.empty())  
          {
            string taxname = word;
            size_t quotestart = word.find_first_of("\"'");

            if (quotestart != string::npos)
            {
              char quotechar = word.at(quotestart);
              size_t quoteend = word.find(quotechar, quotestart + 1);
              if (! quoteend)
                throw SeqParseError("Quoted taxon name has no end quote!");

              taxname = word.substr(quotestart + 1, quoteend - quotestart - 1);
              //ParserTools::replaceChars(seqname, ' ', '_');
              ParserTools::strip(taxname);
            }
            
            //_taxa.insert(word);
            _taxonIndexMap[taxname] = _taxonCount++;
            _taxonVect.push_back(taxname);
          }
          worditer++;
        }        
      }

      else  throw SeqParseError("Taxlabels keyword not allowed in this block!");
    }
    break;
    
    case TraitLabels:
    {
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);

      worditer = wordlist.begin();

      if (worditer == wordlist.end())
        throw SeqParseError("Empty word list, but line isn't empty!");

      ParserTools::lower(word = (*worditer));
      if (word == "traitlabels")  worditer++;

      while (worditer != wordlist.end())
      {
        word = *worditer;
        if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
        if (! word.empty()) 
          _traitNames.push_back(word);
        ++worditer;
      }
    }
    break;

    case NoKwd:
      break;
    
    // TODO this doesn't work if there are quotes in names
    // Also, consider this if parsing SplitsTree Network block
    case Translate:
    {
      if (_currentBlock != Trees)  break;
      
      size_t translatestart = ParserTools::caselessfind("translate", line);
      
      if (translatestart != string::npos)  
      {
        // 9 is length of string "translate"
        translatestart = line.find_first_not_of("\t ", translatestart + 9);
        if (translatestart == string::npos)  line.clear();//translatestart = line.length();

        else  line = line.substr(translatestart);
      }
            
      if (! line.empty() && line.at(line.length() - 1) == ';')  line.erase(line.length() - 1);
      
      if (line.empty())  return;
      
      vector<string> pairlist;
      ParserTools::tokenise(pairlist, line, ",");
      vector<string>::iterator pairiter = pairlist.begin();
      while (pairiter != pairlist.end())
      {
        wordlist.clear();
        ParserTools::tokenise(wordlist, *pairiter);
        
        if (wordlist.size() == 0 || wordlist.size() > 2)
          throw SeqParseError("Error parsing translate line.");
        
        if (wordlist.size() == 1)
        {
          if (_keystr) 
          {
            //_right = new const string(wordlist.at(0));
            _treeTaxMap[string(*_keystr)] = wordlist.at(0);
            delete _keystr;
            _keystr = 0;
            
          }
          
          else  _keystr = new string(wordlist.at(0));
        }
        
        else 
        {
          _treeTaxMap[wordlist.at(0)] = wordlist.at(1);
        }
        ++pairiter;
      }
    }
    break;
    
    case TreeKW:
    { 
      size_t eqpos = line.find('='); 
      
      if (eqpos == string::npos)
      {
        if (_treestr == 0)  throw SeqParseError("Mid-tree value, and no tree string allocated!");

        *_treestr += line;
      }
            
      else
      {   
        if (_treestr != 0)  
          throw SeqParseError("Last tree string didn't end and new tree string has begun!");

        line = line.substr(eqpos + 1);
        ParserTools::strip(line);
        _treestr = new string(line); 
          
      }
            
      if (_treestr->at(_treestr->length() - 1) == ';')
      {
        istringstream iss(*_treestr);
        Tree *t = new Tree;
        iss >> *t;
        delete _treestr;
        _treestr = 0;
        
        
        if (_treeTaxMap.empty())  _trees.push_back(t);
        
        else
        {
          map<string, string>::iterator taxonIter;
          
          Tree::LeafIterator lit = t->leafBegin();
          
          while (lit != t->leafEnd())
          {
            taxonIter = _treeTaxMap.find((*lit)->label());
            if (taxonIter == _treeTaxMap.end())  
              throw SeqParseError("Tree labels do not match alignment labels.");
           
            else  
            {
              (*lit)->setLabel(taxonIter->second);
            }
            
            ++lit;
          }
          
          _trees.push_back(t);
        }
        
        //delete _treestr;
        //_treestr = 0;
      }
      
      if (line.empty())  return;
   
      
      
      // TODO after translating taxon names, check that they're actually taxon names
    }
    break;
        
    /*case TraitKW:
    {
      // TODO make this more error tolerant allowing line breaks mid-trait
      if (line.at(line.length() - 1) == ';')  line.erase(line.length() - 1);

      size_t traitpos = ParserTools::caselessfind("trait", line);
      size_t eqpos = line.find('='); 
            
      
      if (traitpos == string::npos || eqpos == string::npos)  throw SeqParseError("Error parsing traits line.");
      
      string traitname = line.substr(traitpos + 5, eqpos - (traitpos + 6));
      ParserTools::strip(traitname);
      
      Trait *trait = new Trait(traitname);
      
      string traitdef = line.substr(eqpos + 1);
      wordlist.clear();
      ParserTools::tokenise(wordlist, traitdef, ",");
      
      vector<string>::iterator worditer = wordlist.begin();
      
      while (worditer != wordlist.end())
      {
        istringstream iss(*worditer);

        string seqname;
        iss >> seqname;
        
        unsigned nsamps;
        iss >> nsamps;
        
        trait->addSeq(seqname, nsamps);
        ++worditer;
      }
      
      _traits.push_back(trait);
      
    }
    break;*/
    case CharstateLabels:
      warn("Ignoring keyword CharStateLabels");
      break;

    /* TODO need to add support for sets and assumptions blocks */
    case Unknown: // Only a problem if this is a block we can deal with

    {
      /*if (_currentBlock != Other &&
      _currentBlock != Assumptions &&
      _currentBlock != Sets)*/
      if (_currentBlock == Data || _currentBlock == Taxa || _currentBlock == Characters || _currentBlock == Trees || _currentBlock == Traits)
        warn("Unknown keyword.");
        //throw SeqParseError("Unknown keyword!");
    }
    break;

    default:
    {
      /* This should never happen */
      throw SeqParseError("Invalid keyword!");
    }
    break;
  }
}

/*string & NexusParser::fixEquals(string & str)
{
  string spequals = " = ";
  size_t speqpos = str.find(spequals);

  while (speqpos != string::npos)
  {
    str.replace(speqpos, 3, 1, '=');
    speqpos = str.find(spequals, speqpos + 1);
  }

  return str;
}*/

void NexusParser::putSeq(ostream &output, const Sequence &sequence)
{

  if (! _headerWritten)
  {
    _headerWritten = true;
    _seqWriteCount = 0;
    output << "#NEXUS\nBegin Data" << endl;
    output << "    Dimensions ntax=" << nSeq();
    output << " nchar=" << nChar() << ";\n";
    output << "    Format datatype=";

    if (charType() == DNAType)  output << "DNA missing=N";
    else if (charType() == AAType)  output << "Protein missing=X";
    else  output << "Standard missing=?";
    
    output << " gap=-;" << endl;
    output << "    Matrix" << endl;
  }


  size_t spaceidx = sequence.name().find(' ');
  if (spaceidx == string::npos)
    output << sequence.name();

  else
    output << '"' << sequence.name() << '"';

  output << '\t' << sequence.seq() << endl;
  _seqWriteCount++;

  if (_seqWriteCount == nSeq())
  {
    output << ";\nEnd;" << endl;
    _seqWriteCount = 0;
  }
}

bool NexusParser::cleanComment(string &line, bool inComment)
{
  size_t openidx, closeidx;

  if (inComment)
  {
    closeidx = line.find(']');
    if (closeidx == string::npos)
    {
      line.clear();
      return true;
    }

    else
    {
      line.erase(0, closeidx + 1);
      return false;
    }
  }

  else
  {
    openidx = line.find('[');
    if (openidx == string::npos)  return false;

    else
    {
      closeidx = line.find(']', openidx);

      if (closeidx == string::npos)
      {
        line.erase(openidx);
        return true;
      }

      else
      {
        line.erase(openidx, closeidx - openidx + 1);
        return false;
      }
    }
  }

  /* This statement should never be reached */
  return inComment;
}

/**
 * Replace space characters before and after '=' characters
 */
string & NexusParser::fixEquals(string &str)
{
  size_t eqpos = str.find('=');
  
  while (eqpos != string::npos)
  {
    int lastNonwhitespace = eqpos - 1;
    while (lastNonwhitespace >= 0 && str.at(lastNonwhitespace) == ' ')
      lastNonwhitespace--;
    
    int firstNonwhitespace = eqpos + 1;
    while (firstNonwhitespace < str.size() && str.at(firstNonwhitespace) == ' ')
      firstNonwhitespace++;
    
    size_t lastpos = eqpos;
    
    // if whitespace either before or after, need to replace
    // if any replacement made, lastpos = firstNonwhitespace - 1
    
    if ((firstNonwhitespace - lastNonwhitespace) > 2)
    {
       lastpos = firstNonwhitespace - 1;
       str.replace(lastNonwhitespace + 1, firstNonwhitespace - lastNonwhitespace - 1, "=");
    }
     
    eqpos = str.find('=', lastpos + 1);
     
  }
  
  return str;
}



