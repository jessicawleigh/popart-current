/*
 * NexusParser.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: jleigh
 */

#include <sstream>
using namespace std;

#include "NexusParser.h"
#include "ParserTools.h"
#include "SeqParseError.h"

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
  
  for (unsigned i = 0; i < _geoTagTraits.size(); i++)
    delete _geoTagTraits.at(i);
  _geoTagTraits.clear();

  _traitNames.clear();
  _traitLocations.clear();
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
  _geotagSep = ' ';
  _respectCase = false;
  _inComment = false;
  _inSeq = false;
  _interleave = false;
  _traitsLabeled = false;
  _geotagsLabeled = false;
  _newTaxa = false;
  _taxLabels = false;
  _ntraits = 0;
  _nclusts = 0;
  _latCount = 0;
  _lonCount = 0;
  _nverts = 0;
  _nedges = 0;
  _hasTraits = false;
  _hasGeoTags = false;
  //_geoDataSaved = false;

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
  _plotRect.clear();
  _vertices.clear();
  _edges.clear();
  _vLabels.clear();
  
  initGraphicsParams(_netGraphicsParams);
  

  for (unsigned i = 0; i < _trees.size(); i++)
    delete _trees.at(i);
  _trees.clear();

  for (unsigned i = 0; i < _traits.size(); i++)
    delete _traits.at(i);
  _traits.clear();
  _traitGroups.clear();
  _traitGroupLabels.clear();
  
  for (unsigned i = 0; i < _geoTagTraits.size(); i++)
    delete _geoTagTraits.at(i);
  _geoTagTraits.clear();
  _geoGroupLabels.clear();

  _traitNames.clear();
  _traitLocations.clear();
}

void NexusParser::initGraphicsParams(NexusParser::GraphicsParams &gp)
{
  gp.font.erase();
  gp.legendFont.erase();
  gp.vColour.erase();
  gp.eColour.erase();
  gp.bgColour.erase();
  gp.eView.erase();
  gp.vSize = -1;
  gp.lPos.first = -1; gp.lPos.second = -1;
  gp.lColours.clear();
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
  _kwdMap["traitlatitude"] = TraitLatitude;
  _kwdMap["traitlongitude"] = TraitLongitude;
  _kwdMap["traitpartition"] = TraitPartition;
  _kwdMap["clustlabels"] = ClustLabels; 
  _kwdMap["clustlatitude"] = ClustLatitude;
  _kwdMap["clustlongitude"] = ClustLongitude;
  _kwdMap["clustpartition"] = ClustPartition;  
  _kwdMap["translate"] = Translate;
  _kwdMap["tree"] = TreeKW;
  _kwdMap["charstatelabels"] = CharstateLabels;
  _kwdMap["vertices"] = Vertices;
  _kwdMap["edges"] = Edges;
  _kwdMap["vlabels"] = VLabels;


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
  _blockMap["geotags"] = GeoTags;
  _blockMap["sets"] = Sets;
  _blockMap["network"] = Network;


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

  if ((! _interleave) || _seqidx < 0)
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
          //cerr << "line here:" << line << endl;
          if (_currentKeyWord == NoKwd)
          {
            wordlist.clear();
            ParserTools::tokenise(wordlist, line);
            worditer = wordlist.begin();
            if (worditer == wordlist.end())
            {
              //cerr << "line: " << line << endl;
              throw SeqParseError("Empty wordlist, but not empty line. This shouldn't happen.");
            }

            ParserTools::lower(word = *worditer);
            if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
            kwdResult = _kwdMap.find(word);
            if (kwdResult == _kwdMap.end())
            {
              _currentKeyWord = Unknown;
              _kwdText = *worditer;
            }

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
                  //cerr << "about to throw exception. nchar: " << nChar() << " chars read: " << sequence.length() << " seq name: " << sequence.name() << endl;
                  //cerr << "line: " << line << endl;
                  throw SeqParseError("Too many characters read!");
                }
              }
            }
          }
        }
      }
    }
  }
  
  // Clean up interleaved sequences once all sequences have been read.
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

const vector<string> & NexusParser::traitGroups() const
{
  return _traitGroupLabels;
}

const vector<GeoTrait *> & NexusParser::geoTraitVector() const
{
  return _geoTagTraits;
}

const vector<string> & NexusParser::geoGroups() const
{
  return _geoGroupLabels;
}

void NexusParser::setTraitLocation(unsigned idx, std::pair<float,float> coords)
{
  GeoTrait *gt = new GeoTrait(coords, *(_traits.at(idx)));
  delete _traits.at(idx);
  _traits.at(idx) = gt;
}

void NexusParser::setGeoTraitLocation(unsigned idx, std::pair<float,float> coords)
{
  _geoTagTraits.at(idx)->setLocation(coords);
}

void NexusParser::parseLine(string line, Sequence &sequence)
{

  vector<string> wordlist; 
  vector<string>::iterator worditer;
  string word;
  unsigned groupcount;


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
      {
        _taxonCount = 0;
        _hasTraits = true;
        _traitNames.clear();
        _traitLocations.clear();
        _latCount = 0;
        _lonCount = 0;
      }
      
      else if (_currentBlock == GeoTags)
      {
        _hasGeoTags = true;
        _traitNames.clear();
        _traitLocations.clear();
        _latCount = 0;
        _lonCount = 0;
      }
      
      else if (_currentBlock == Network)
      {
        _treeTaxMap.clear();
      }
      
    }
    break;

    case End:
    {
      if (_currentBlock == NoBlock)  throw SeqParseError("End keyword found outside block!");
      
      if (_currentBlock == Traits)
      {
        if (! _traitGroups.empty())
        {
          for (unsigned i = 0; i < _traits.size(); i++)
          {
            if (_traitGroups.at(i) < 0)
              throw SeqParseError("Some, but not all, GeoTag groups specified!");
            _traits.at(i)->setGroup(_traitGroups.at(i));
          }
          
          _traitGroups.clear();

        }
      }

      else if (_currentBlock == GeoTags)
      {
        // TODO this
        // Check all sequences for cluster IDs
        // if all sequences have them, then no clustering necessary
        // otherwise, use GeoTrait::clusterSeqs(coordinates, seqnames, seqcounts, nclusts)
        vector<unsigned> seqcounts;
        vector<string> seqnames;
        vector<pair<float,float> > coordinates;
        bool clusterIDsFound = true;
        
        for (unsigned i = 0; i < _geoTags.size(); i++)
        {
          seqnames.push_back(_geoTags.at(i).name);
          seqcounts.push_back(_geoTags.at(i).nsamples);
          coordinates.push_back(pair<float,float>(_geoTags.at(i).latitude, _geoTags.at(i).longitude));
          
          if (_geoTags.at(i).clusterID == 0)
            clusterIDsFound = false;
        }
        
        if (clusterIDsFound)
        {
          
          vector<pair<float,float> > clustCoords;
          
          vector<SeqGeoData>::const_iterator geoTagIt;
          if (_traitLocations.empty()) // need to find centroids
          {
            vector<float> latitudes(_nclusts, 0);
            vector<float> longitudes(_nclusts, 0);
            vector<unsigned> clustsizes(_nclusts, 0);
            
            
            geoTagIt = _geoTags.begin();
            
            
            while (geoTagIt != _geoTags.end())
            {
              latitudes.at(geoTagIt->clusterID - 1) += geoTagIt->latitude;
              longitudes.at(geoTagIt->clusterID - 1) += geoTagIt->longitude;
              clustsizes.at(geoTagIt->clusterID - 1) ++;
              
              ++geoTagIt;
            }
            
            for (unsigned i = 0; i < _nclusts; i++)
              clustCoords.push_back(pair<float,float>(latitudes.at(i)/clustsizes.at(i), longitudes.at(i)/clustsizes.at(i)));
          }
          
          else // cluster "centroids" read from file input             
            clustCoords.assign(_traitLocations.begin(), _traitLocations.end());
          
          for (unsigned i = 0; i < _nclusts; i++)
          {
            ostringstream oss;
            if (_traitNames.empty())
              oss << "cluster" << (i+1);
            else
              oss << _traitNames.at(i);
                        
            _geoTagTraits.push_back(new GeoTrait(clustCoords.at(i), oss.str()));
          }
          
          
          geoTagIt = _geoTags.begin();
          
          
          while (geoTagIt != _geoTags.end())
          {
            GeoTrait *gt = _geoTagTraits.at(geoTagIt->clusterID - 1);
            gt->addSeq(pair<float,float>(geoTagIt->latitude, geoTagIt->longitude), geoTagIt->name, geoTagIt->nsamples);
            ++geoTagIt;
          }           
        }
        
        else
        {
          _geoTagTraits =  GeoTrait::clusterSeqs(coordinates, seqnames, seqcounts, _nclusts, _traitLocations, _traitNames);
          
          /*for (unsigned i = 0; i < geoTraitRefs.size(); i++)
            _geoTagTraits.push_back(new GeoTrait(geoTraitRefs.at(i)));*/
          
          /*if (_traitLocations.empty())
            _geoTagTraits = GeoTrait::clusterSeqs(coordinates, seqnames, seqcounts, nclusts);
          
          else if (_traitNames.empty())
            _geoTagTraits = GeoTrait::clusterSeqs(coordinates, seqnames, seqcounts, nclusts);*/
        }
        
        // Change this: for either traits or geotraits, loop through list and assign groups HERE
        if (! _traitGroups.empty())
        {
          for (unsigned i = 0; i < _geoTagTraits.size(); i++)
          {
            if (_traitGroups.at(i) < 0)
              throw SeqParseError("Some, but not all, GeoTag groups specified!");
            _geoTagTraits.at(i)->setGroup(_traitGroups.at(i));
          }
          
          _traitGroups.clear();
        }
      }
        
      _currentBlock = NoBlock;
      
    }
    break;

    case Dimensions:
    {
      if (_currentBlock == Other)  return;
      else if (_currentBlock == Taxa || _currentBlock == Characters ||
          _currentBlock == Data || _currentBlock == Unaligned ||
          _currentBlock == Traits || _currentBlock == GeoTags ||
          _currentBlock == Network    ) //|| _currentBlock == Distances)
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

              if (nSeq() == 0 || nSeq() != _seqSeqVect.size())
              {
                iss >> nseq;
                setNseq(nseq);
                _seqSeqVect.resize(nseq);
              }
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
          
          else if (key == "nclusts")
          {
            int nclusts;
            if (val.empty() || _currentBlock != GeoTags)
              throw SeqParseError("nclusts can only be used in GeoTags block.");
            iss >> nclusts;
            _nclusts = nclusts;
          }
          
          else if (key == "nvertices" || key == "nedges" || key == "plotdim")
          {
            if (val.empty() || _currentBlock != Network)
              throw SeqParseError("nvertices, nedges, and plotdim can only be used in Network block.");
            if (key == "nvertices")
              iss >> _nverts;
            else if (key == "nedges")
              iss >> _nedges;
            else //if (key == "plotdim")
            {
              string plotdimstr;
              iss >> plotdimstr;
              vector<string> plotwords;
              ParserTools::tokenise(plotwords, plotdimstr, ",");
              if (plotwords.size() != 4)
                throw SeqParseError("Invalid plotdim specification.");
              
              unsigned count = 3;
              _plotRect.resize(4, 0);
              while (! plotwords.empty())
              {
                iss.clear();
                iss.str(plotwords.back());
                plotwords.pop_back();
                double dim;
                iss >> dim;
                _plotRect.at(count--) = dim;
              }
            }
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
               _currentBlock == Unaligned || _currentBlock == Traits ||
               _currentBlock == GeoTags || _currentBlock == Network)
               // _currentBlock == Distances)
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
            if (val.empty())  throw SeqParseError("No value given for 'missing' char.");
            if (_currentBlock == GeoTags)
              warn("'Missing' directive not allowed in GeoTag block."); 
            else
              iss >> _missing;
          }

          else if (key == "gap")
          {
            if (val.empty())  throw SeqParseError("No value given for 'gap' char.");
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
                //cerr << "line: " << line << endl;
                throw SeqParseError("Trait labels must be specified yes or no.");
              }
              if (val == "yes")
                _traitsLabeled = true;
              else  if (val == "no")
                _traitsLabeled = false;
              else  throw SeqParseError("Trait labels must be specified yes or no.");
            }
            
            if (_currentBlock == GeoTags)
            {
              if (val.empty()) 
              {
                //cerr << "line: " << line << endl;
                throw SeqParseError("GeoTag labels must be specified yes or no.");
              }
              if (val == "yes")
                _geotagsLabeled = true;
              else  if (val == "no")
                _geotagsLabeled = false;
              else  throw SeqParseError("GeoTag labels must be specified yes or no.");
            }

          }

          else if (key == "nolabels")
            throw SeqParseError("Unable to parse Nexus with nolabels!");
          
          else if (key == "separator")
          {
            if (_currentBlock == Traits)
            {
              if (val == "comma")  _traitSep = ',';
              else if (val == "spaces")  _traitSep = ' ';
              else if (val == "tab")  _traitSep = '\t';
              else throw SeqParseError("Trait separator must be tab, spaces, or comma.");
            }
            else if (_currentBlock == GeoTags)
            {
              if (val == "comma")  _geotagSep = ',';
              else if (val == "spaces")  _geotagSep = ' ';
              else if (val == "tab")  _geotagSep = '\t';
              else throw SeqParseError("GeoTag separator must be tab, spaces, or comma.");
            }

          }
          
          else if (key == "font")
          {
            if (_currentBlock != Network)
              throw SeqParseError("Font keyword only allowed in Network block.");
            
            _netGraphicsParams.font = val;

          }
          
          else if (key == "legendfont")
          {
            if (_currentBlock != Network)
              throw SeqParseError("LegendFont keyword only allowed in Network block.");
            
            _netGraphicsParams.legendFont = val;
          }

          else if (key == "vcolour")
          {
            if (_currentBlock != Network)
              throw SeqParseError("VColour keyword only allowed in Network block.");
            
            _netGraphicsParams.vColour = val;
          }
          
          else if (key == "ecolour")
          {
            if (_currentBlock != Network)
              throw SeqParseError("EColour keyword only allowed in Network block.");
            
            _netGraphicsParams.eColour = val;
          }
          
          else if (key == "bgcolour")
          {
            if (_currentBlock != Network)
              throw SeqParseError("BGColour keyword only allowed in Network block.");
            
            _netGraphicsParams.bgColour = val;
          }
          
          else if (key == "eview")
          {
            if (_currentBlock != Network)
              throw SeqParseError("EView keyword only allowed in Network block.");
            
            _netGraphicsParams.eView = val;
          }

          else if (key == "vsize")
          {
            if (_currentBlock != Network)
              throw SeqParseError("VSize keyword only allowed in Network block.");
            iss.str(val);
            
            double size;
            iss >> size;
            
            if (iss.fail())
            {
              warn("Invalid vertex size, will use default.");
              
              _netGraphicsParams.vSize = -1;
              
              iss.clear();
            }
            
            else
              _netGraphicsParams.vSize = size;
          }
          
          else if (key == "lpos")
          {
            if (_currentBlock != Network)
              throw SeqParseError("LPos keyword only allowed in Network block.");
            
            size_t commaPos = val.find_first_of(',');
            
            if (commaPos == string::npos)
            {
              warn("Invalid legend position, will use (0,0).");
              _netGraphicsParams.lPos.first = 0;
              _netGraphicsParams.lPos.second = 0;
            }
              
            else
            {
              double x, y;
              iss.clear();
              iss.str(val.substr(0, commaPos));
              iss >> x;
              
              
              iss.clear();
              iss.str(val.substr(commaPos + 1));    
              iss >> y;
              
              if (iss.fail())
              {
                warn("Invalid legend position, will use (0,0).");
              _netGraphicsParams.lPos.first = 0;
              _netGraphicsParams.lPos.second = 0;
              }
              
              else
              {
                _netGraphicsParams.lPos.first = x;
                _netGraphicsParams.lPos.second = y;
              }
            }
          }

          else if (key == "lcolours")
          {
            if (_currentBlock != Network)
              throw SeqParseError("LCols keyword only allowed in Network block.");
            
            vector<string> colours;
            ParserTools::tokenise(colours, val, ",");
            
            _netGraphicsParams.lColours.assign(colours.begin(), colours.end());
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
        if (_traitNames.empty())
          throw SeqParseError("Traits block must contain a TraitLabels directive!");
        
        if (_traitNames.size() != _ntraits)
          throw SeqParseError("Number of TraitLabels does not match ntraits!");
        
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
              //ParserTools::eraseChars(traitstr, ' ');
              //ParserTools::eraseChars(traitstr, '\t');
            }
            
            else
            {
              size_t nameend = line.find_first_of(" \t");
              if (nameend == string::npos)
              {
                throw SeqParseError("No space separating sequence and name!");
              }

              seqname = line.substr(0, nameend);
              traitstr = line.substr(nameend + 1);
              //ParserTools::eraseChars(traitstr, ' ');
              //ParserTools::eraseChars(traitstr, '\t');
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
            //ParserTools::eraseChars(traitstr, ' ');
            //ParserTools::eraseChars(traitstr, '\t');
          }
          
          unsigned counter = 0;
          wordlist.clear();
          string sep(1, _traitSep);
          ParserTools::tokenise(wordlist, traitstr, sep);
          worditer = wordlist.begin();
          unsigned nsamples;
          istringstream iss;
         
          while (worditer != wordlist.end())
          {
            if (counter >= _ntraits)
              throw SeqParseError("Too many columns in Traits block (should match ntraits).");
            
            if (_traits.size() <= counter)
            {
              Trait *t;
              if (_traitLocations.empty())
                t = new Trait(_traitNames.at(counter));
              else
                t = new GeoTrait(_traitLocations.at(counter), _traitNames.at(counter));
                            
              _traits.push_back(t);
            }
            
            // treat missing trait data as 0 samples
            if ((*worditer).at(0) == _missingTrait)
              nsamples = 0;
            
            else
            {
              iss.clear();
              iss.str(*worditer);// + ' ');
              iss >> nsamples;
            }
            
            if (nsamples > 0)
              _traits.at(counter)->addSeq(seqname, nsamples);
            
            counter++;
            ++worditer;
          }
        }
      } // end Traits handling
      
      else if (_currentBlock == GeoTags)
      {
        if (! _traitNames.empty() && _traitNames.size() != _nclusts)
          throw SeqParseError("Number of GeoTag ClustLabels does not match nclusts!");
        if (ParserTools::caselessfind("matrix", line) == string::npos)
        {
          if (line.at(line.length() - 1) == ';')
            line.erase(line.length() - 1);
          
          string geotagstr;
          string seqname;

          if (_geotagsLabeled)
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
              geotagstr = line.substr(quoteend + 1);
              //ParserTools::eraseChars(geotagstr, ' ');
              //ParserTools::eraseChars(geotagstr, '\t');
            }
            
            else
            {
              size_t nameend = line.find_first_of(" \t");
              if (nameend == string::npos)
              {
                throw SeqParseError("No space separating sequence and name!");
              }

              seqname = line.substr(0, nameend);
              geotagstr = line.substr(nameend + 1);
              //ParserTools::eraseChars(geotagstr, ' ');
              //ParserTools::eraseChars(geotagstr, '\t');
            }
            
            if (seqname.empty())  throw SeqParseError("Taxon name empty.");
            map<string, int>::iterator taxonIt = _taxonIndexMap.find(seqname);
            if (taxonIt == _taxonIndexMap.end())
              throw SeqParseError("Taxon found in GeoTags not defined in prior TaxLabels or Data blocks.");
          }
          
          else // geotags unlabeled
          {
            if (_taxonCount >= _taxonVect.size())  
              throw SeqParseError("Too many rows in GeoTags block.");
            seqname = _taxonVect.at(_taxonCount++);
            geotagstr = line;
            //ParserTools::eraseChars(geotagstr, ' ');
            //ParserTools::eraseChars(geotagstr, '\t');
          }
          
          //int counter = 0;
          /*wordlist.clear();
          string sep(1, _geotagSep);
          ParserTools::tokenise(wordlist, geotagstr, sep);*/
          if (_geotagSep == ',')
            ParserTools::replaceChars(geotagstr, ',', ' '); // replace commas so we can use iss to parse string
          
          float lat;
          char latHemi;
          string lonstr;
          float lon;
          char lonHemi;
          int nsamples;
          int clusterID;
          istringstream iss(geotagstr);
          iss >> lat;
          if (iss.fail())
            throw SeqParseError("Error reading coordinates for sequence.");         
          iss >> latHemi;
          if (isdigit(latHemi) || latHemi == '-')
            iss.putback(latHemi);
          else
          {
            latHemi = tolower(latHemi);
            if (latHemi == 's')
              lat *= -1;
            else if (latHemi != 'n')
              throw SeqParseError("Unknown latitude format");
              
          }
          
          iss >> lonstr;
          lonHemi = tolower(lonstr.at(lonstr.size() - 1));
          if (isdigit(lonHemi) || lonHemi == '-' || lonHemi == 'w')
          {
            istringstream iss2(lonstr);
            iss2 >> lon;
            if (lonHemi == 'w')
              lon *= -1;
          }
          
          else
          {
            if (lonHemi == 'e')
              lonstr.erase(lonstr.size() - 1);
            
            else 
              throw SeqParseError("Unknown longitude format");
            
            istringstream iss2(lonstr);
            iss2 >> lon;
          }
          
          
          if (iss.fail())
            throw SeqParseError("Error reading coordinates for sequence.");
          
          iss >> nsamples;
          if (iss.fail())
            nsamples = 1;
          else if (nsamples < 1)
            throw SeqParseError("Number of samples should be positive.");
          else
            iss >> clusterID;
          if (iss.fail())
            clusterID = 0;
          else if (clusterID < 1)
            throw SeqParseError("Cluster numbers should be positive.");
          
          SeqGeoData sgd = {seqname, lat, lon, (unsigned)nsamples, (unsigned)clusterID};
          _geoTags.push_back(sgd);
          
        }// end if caselessfind("matrix"...)     
      } // end GeoTags handling
      
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
            if ((_taxLabels && ! _newTaxa) || (_interleave && _taxonVect.size() == nSeq()))
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
                throw SeqParseError("More seq names than specified by nseq.");
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
                seqname = line.substr(0, nameend);
                seq = line.substr(nameend);
                ParserTools::eraseChars(seq, ' ');
                ParserTools::eraseChars(seq, '\t');
              }

              
              if ((_taxLabels && ! _newTaxa) || (_interleave && _taxonVect.size() == nSeq()))
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
                throw SeqParseError("More seq names than specified by nseq.");
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
    case ClustLabels:
    {
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);

      worditer = wordlist.begin();

      if (worditer == wordlist.end())
        throw SeqParseError("Empty word list, but line isn't empty!");

      ParserTools::lower(word = (*worditer));
      if (word == "traitlabels" || word == "clustlabels")  ++worditer;

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
    
    case TraitLatitude:
    case ClustLatitude:
    {
     unsigned nitems = _currentKeyWord == TraitLatitude ? _ntraits : _nclusts;
      
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);
      
      worditer = wordlist.begin();

      if (worditer == wordlist.end())
        throw SeqParseError("Empty word list, but line isn't empty!");

      ParserTools::lower(word = (*worditer));
      if (word == "traitlatitude" || word == "clustlatitude")  ++worditer;
      
       if (_traitLocations.empty())
        _traitLocations.resize(nitems, pair<float,float>(0,0));


      float lat;
      while (worditer != wordlist.end())
      {
        word = *worditer;
        if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
        if (! word.empty()) 
        {
          if (_latCount >= nitems)
          {
            if (_currentKeyWord == TraitLatitude)
              throw SeqParseError("Too many trait latitudes read.");
            else 
              throw SeqParseError("Too many geotag cluster latitudes read.");
          }
          
          istringstream iss(word);
          iss >> lat;
          _traitLocations.at(_latCount++).first = lat;
        }
        ++worditer;
      }
         
      break;
    }
    case TraitLongitude:
    case ClustLongitude:
    {
      unsigned nitems = _currentKeyWord == TraitLongitude ? _ntraits : _nclusts;
      
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);
      
      worditer = wordlist.begin();

      if (worditer == wordlist.end())
        throw SeqParseError("Empty word list, but line isn't empty!");

      ParserTools::lower(word = (*worditer));
      if (word == "traitlongitude" || word == "clustlongitude")  ++worditer;
      
      if (_traitLocations.empty())
        _traitLocations.resize(nitems, pair<float,float>(0,0));


      float lon;
      while (worditer != wordlist.end())
      {
         if (_lonCount >= nitems)
         {
           if (_currentKeyWord == TraitLongitude)
             throw SeqParseError("Too many trait longitudes read.");
           else
             throw SeqParseError("Too many geotag cluster longitudes read.");
         }
         
        word = *worditer;
        if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
        if (! word.empty()) 
        {
          istringstream iss(word);
          iss >> lon;
          _traitLocations.at(_lonCount++).second = lon;
        }
        ++worditer;
      }
      
      break;
    }
    
    case TraitPartition:
    case ClustPartition:
    {
     // max groups is number of traits/clusters: check this
      // min groups is 1 (make sure this doesn't make AMOVA upset)
      // ignore the name given to the partition
      // make sure parsing still works if partition written over multiple lines

      unsigned nitems = _currentKeyWord == TraitPartition ? _ntraits : _nclusts;
        _traitGroups.assign(nitems, -1);

      vector<string> & labelvect = _currentKeyWord == TraitPartition ? _traitGroupLabels : _geoGroupLabels;
      
       wordlist.clear();
      
      ParserTools::tokenise(wordlist, line, "=,");
      worditer = wordlist.begin();
      
      if (ParserTools::caselessfind("traitpartition", (*worditer)) != string::npos || ParserTools::caselessfind("clustpartition", (*worditer)) != string::npos)
      {
        ++worditer;
        groupcount = 0;
      }
      

      while (worditer != wordlist.end())
      {        
        word = *worditer;
        if (word.at(word.length() - 1) == ';')  word.erase(word.length() - 1);
        if (! word.empty()) 
        {
          size_t labelend = word.find(':');
          if (labelend == string::npos)
            throw SeqParseError("Error parsing trait/geotag groups: no label found");
          
          string label = word.substr(0, labelend);
          string members = word.substr(labelend + 1);
          ParserTools::strip(label);
          ParserTools::strip(members);
          
          labelvect.push_back(label);
          
          size_t dashpos = members.find('-');
          
          while (dashpos != string::npos)
          {
            // clean up whitespace after a dash
            size_t endspace = members.find_first_not_of(" \t", dashpos + 1);

            if (endspace == string::npos)
              throw SeqParseError("Invalid syntax for trait/geotag group");
            
            if (endspace > dashpos + 1)
              members.replace(dashpos, endspace - dashpos, "-");

            // clean up whitespace before a dash
            endspace = members.find_last_not_of(" \t", dashpos - 1);
            
            if (endspace == string::npos)
              throw SeqParseError("Invalid syntax for trait/geotag group");
            
            if (endspace < dashpos - 1)
              members.replace(endspace + 1, dashpos - endspace, "-");
            
            dashpos = members.find('-', endspace + 2);
          }
                    
          vector<string> memberlist;
          
          ParserTools::tokenise(memberlist, members);
          
          vector<string>::iterator memberiter = memberlist.begin();
          
          while (memberiter != memberlist.end())
          {
            istringstream iss(*memberiter);
            
            unsigned start, end;
            char dash;
            
            iss >> start;
            
            if (iss.good())
            {
              iss >> dash >> end;
              for (unsigned i = start; i <= end; i++)
                _traitGroups.at(i - 1) = groupcount;
            }
            
            else
              _traitGroups.at(start - 1) = groupcount;
              //cout << "got value: " << start << endl;
            
            ++memberiter;
          }
        }
        
        ++worditer;
        groupcount++;
      }
        
      
      // break line on comma to get different groups; each should have the structure label\: ?\d ?- ?\d;? or label\: ?\d( \d)*;?
      // break each token on : to get label in first token;
      //   clean up \d ?- ?\d: strip any whitespace around dash
      // then break on spaces. If the word can be converted to a number, set the cluster's group ID. Otherwise, look for two numbers and a dash, and set the range's group ID.
      
      break;
    }
    
    case Vertices:
    {
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);
      
      if (ParserTools::lower(wordlist.front()) == "vertices")
        break;
        
      
      if (wordlist.size() != 3)
        throw SeqParseError("Invalid vertex descriptor.");
      
      if (line.at(line.size() - 1) == ',')
        line.erase(line.size() - 1);
      
      istringstream iss(line);
      unsigned vid;
      double x;
      double y;
      
      iss >> vid;
      iss >> x;
      iss >> y;
      
      if (iss.fail())
        throw SeqParseError("Invalid vertex format.");
      
      
      // TODO check that vid > 0
      if (_vertices.size() < vid)
        _vertices.resize(vid, pair<double,double>(0,0));
      
      _vertices.at(vid - 1) = pair<double,double>(x,y);
      
      break;
    }
    
    case Edges:
    {
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);
      
      if (ParserTools::lower(wordlist.front()) == "edges")
        break;
      
      if (wordlist.size() < 3 && wordlist.size() > 4)
        throw SeqParseError("Invalid edge descriptor.");
      
      if (line.at(line.size() - 1) == ',')
        line.erase(line.size() - 1);
      
      istringstream iss(line);
      unsigned eid;
      unsigned from;
      unsigned to;
      double weight = 1.0;
      
      EdgeData *edata;
      
      iss >> eid;
      iss >> from;
      iss >> to;
      
      if (iss.good())
        iss >> weight;
      
      if (iss.fail())
        throw SeqParseError("Invalid edge format.");
      
      if (_edges.size() < eid)
        _edges.resize(eid);//pair<unsigned,unsigned>(0,0));
      
      edata = & _edges.at(eid - 1);
      edata->eid = eid;
      edata->from = from;
      edata->to = to;
      edata->weight = weight;
      // = {.eid=eid, .from=from, .to=to, .weight=weight};//pair<unsigned,unsigned>(from,to);
      break;
    }
    
    case VLabels:
    {
      wordlist.clear();
      ParserTools::tokenise(wordlist, line);

      if (ParserTools::lower(wordlist.front()) == "vlabels")
        break;
      
      if (wordlist.size() != 3)
        throw SeqParseError("Invalid vertex label descriptor.");
      
      if (line.at(line.size() - 1) == ',')
        line.erase(line.size() - 1);
      
      istringstream iss(line);
      unsigned vid;
      double x;
      double y;
      
      iss >> vid;
      iss >> x;
      iss >> y;
      
      if (iss.fail())
        throw SeqParseError("Invalid vertex label format.");
      
      if (_vLabels.size() < vid)
        _vLabels.resize(vid, pair<double,double>(0,0));
      
      _vLabels.at(vid - 1) = pair<double,double>(x,y);
      break;
    }

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
      
      ostringstream oss;
      oss << "Unknown keyword: " << _kwdText;
      if (_currentBlock == Data || _currentBlock == Taxa || _currentBlock == Characters || _currentBlock == Trees || _currentBlock == Traits)
        warn(oss.str());
        //throw SeqParseError("Unknown keyword!");
    }
    break;

    default:
    {
      /* This should never happen */
      ostringstream oss;
      oss << "Invalid keyword. Line: " << line;
      throw SeqParseError(oss.str());
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
    output << "#NEXUS\nBegin Data;" << endl;
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
    _headerWritten = false;
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



