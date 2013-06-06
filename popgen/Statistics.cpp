#include "Statistics.h"
#include "StatsError.h"

#include <cmath>
#include <iostream>
#include <sstream>
using namespace std;

#ifdef NET_QT
#include <QThread>
#endif

//map<Sequence, list<Sequence> > Statistics::_identicalSeqMap = map<Sequence, list<Sequence> >();
//vector<Sequence> Statistics::_alignment = vector<Sequence>();


Statistics::Statistics(const vector<Sequence*> & alignment, const vector<bool> & mask, Sequence::CharType datatype)
{
  _datatype = datatype;
  _tmpAlignment = &alignment;
  _tmpMask = &mask;
}

void Statistics::setupStats()
{
//#ifdef NET_QT
//  _executionTimer.start();
//#endif

  unsigned cSeqCount = 0;
  
  for (unsigned i = 0; i < _tmpAlignment->size(); i++)
  {
    _name2idx[_tmpAlignment->at(i)->name()] = i;
    _idx2name.push_back(_tmpAlignment->at(i)->name());
    Sequence maskedSeq(*(_tmpAlignment->at(i))); // *alignment.at(i));
    maskedSeq.maskChars(*_tmpMask);
    _alignment.push_back(maskedSeq);
    if (i == 0)
      _nsites = maskedSeq.length();
      
    map<Sequence, list<Sequence> >::iterator mapIt = _identicalSeqMap.find(maskedSeq);
      
    if (mapIt == _identicalSeqMap.end())
    {
      _identicalSeqMap[maskedSeq] = list<Sequence>();
      _seqCounts[maskedSeq.name()] = 1;
      _orig2condidx.push_back(cSeqCount++);
    }
      
    else
    {
      mapIt->second.push_back(maskedSeq);
      _seqCounts[mapIt->first.name()] ++;
      _orig2condidx.push_back(_name2idx[mapIt->first.name()]);
    }
  } 
  
  _nseqs = _alignment.size();
  
  updateProgress(10);

  condenseSitePats();
  updateProgress(90);
  assessSites();
  computeDistances();
  updateProgress(100);

#ifdef NET_QT
  thread()->exit();
#endif
}

#ifdef NET_QT
/**
 * Progress should be a percentage (between 0 and 100)
 */
void Statistics::updateProgress(int progress)
{
  //cout << "progress: " << progress << " time: " << _executionTimer.elapsed() << endl;
  if (progress < 0 || progress > 100)  throw StatsError("Progress is not a percentage.");
  emit progressUpdated(progress);
}
#else
void Statistics::updateProgress(int progress)
{
  cout << '.';
  if (progress < 0 || progress > 100)  throw StatsError("Progress is not a percentage.");
  if (! progress % 10)  cout << "] " << progress << "%\n[";
  cout.flush();
}
#endif



// TODO remove some of the redundancy between HapNet and Statistics
void Statistics::condenseSitePats()
{  
  unsigned samePosAs[_nsites];
  for (unsigned i = 0; i < _nsites; i++)   samePosAs[i] = i;

  // Find identical site patterns
  for (unsigned i = 0; i < _nsites; i++) 
  {
    // Check for all gaps in column i (shouldn't happen if gaps are masked)
    bool allgaps = true;
    for (unsigned t = 0; allgaps && t < _nseqs; t++)
      if (! Sequence::isAmbiguousChar(_alignment.at(t).at(i), _datatype))  allgaps = false;
      
    if (allgaps)  samePosAs[i] = _nsites;
    
    for (unsigned j = i + 1; j < _nsites; j++) 
    {
      bool same = true;
      char i2j[256];
      char j2i[256];
      
      for (unsigned k = 0; k < 256; k++)  i2j[k] = j2i[k] = 0;


      for (unsigned t = 0; same && t < _nseqs; t++) 
      {
        char chari = _alignment.at(t).at(i);
        char  charj = _alignment.at(t).at(j);
        
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
  
  unsigned * oPos2CPos = new unsigned[_nsites]; 
  ostringstream buffers[_nseqs];
  
  unsigned newPos = 0;
  
  // Include identical site patterns only once
  for (unsigned i = 0; i < _nsites; i++)
  {
    oPos2CPos[i] = 0;

    if (samePosAs[i] == _nsites)  oPos2CPos[i] = _nsites; 
    else if (samePosAs[i] < i)  oPos2CPos[i] = oPos2CPos[samePosAs[i]];
    else 
    {
      // This should never happen: should be equal to i
      if (samePosAs[i] > i)  
        throw StatsError("Serious error condensing site patterns.");
      
      oPos2CPos[i] = newPos++;
      for (unsigned j = 0; j < _nseqs; j++)  buffers[j] << _alignment.at(j).at(i);
    }    
  }

  unsigned nCsites = newPos;
    
  _weights.clear();
  for (unsigned i = 0; i < nCsites; i++)  
    _weights.push_back(0);
  
  for (unsigned i = 0; i < _nsites; i++) 
    if (oPos2CPos[i] < _nsites)  _weights.at(oPos2CPos[i])++;

  for (unsigned i = 0; i < _nseqs; i++)  
    _alignment.at(i).setSeq(buffers[i].str());
    // _condensedSeqs.push_back(buffers[i].str());

}

// count segregating and parsimony-informative sites
void Statistics::assessSites()
{
  _nParsimonyInformative = 0;
  _nSegSites = 0;
  
  if (_alignment.empty())  return;
  
  unsigned nPatterns = _weights.size();
  for (unsigned i = 0; i < nPatterns; i++)
  {
    bool isConst = true;
    if (_alignment.at(0).length() <= i)  break;
    char state = _alignment.at(0).at(i);
    map<char,unsigned> charCounts;
    map<char, unsigned>::iterator charIt;
    charCounts[state] = 1;
    
    for (unsigned j = 1; j < _alignment.size(); j++)
    {
      if (_alignment.at(j).length() <= i)
      {
        isConst = true;
        break;
      }
      
      if (_alignment.at(j).at(i) != state)
      {
        isConst = false;
        charIt = charCounts.find(_alignment.at(j).at(i));
        
        if (charIt == charCounts.end())
          charCounts[_alignment.at(j).at(i)] = 1;
        else
          charCounts[_alignment.at(j).at(i)]++;
      }
      
      else
        charCounts[state]++;
    }
    
    if (! isConst)
    {
      _nSegSites += _weights.at(i);
    
      unsigned atLeast2Count = 0;
      charIt = charCounts.begin();
      while (charIt != charCounts.end())
      {
        if (charIt->second >= 2)
          atLeast2Count++;
        ++charIt;
      }
      
      if (atLeast2Count >= 2)
        _nParsimonyInformative += _weights.at(i);
    }
  }  
}

void Statistics::setFreqsFromTraits(const vector<Trait *> & traitVect)
{
  
  if (traitVect.empty())  return;
  
  _nseqs = 0;
  
  _traitMat.clear();
  
  for (unsigned i = 0; i < _distances.size(); i++)
    _traitMat.push_back(vector<unsigned>(traitVect.size(), 0));
  //_traitMat.resize(_distances.size());

  
  map<string, unsigned>::iterator countIt = _seqCounts.begin();
  
  while (countIt != _seqCounts.end())
  {
    countIt->second = 0;
    ++countIt;
  }
  
  /*vector<Trait *>::const_iterator traitIt = traitVect.begin();
  
  while (traitIt != traitVect.end())
  {

    const vector<string> & seqNames = (*traitIt)->seqNames();*/
  for (unsigned i = 0; i < traitVect.size(); i++)
  {
    const vector<string> & seqNames = traitVect.at(i)->seqNames();
    
    for (unsigned j = 0; j < seqNames.size(); j++)
    {
      unsigned count = traitVect.at(i)->seqCount(seqNames.at(j));
      //(*traitIt)->seqCount(seqNames.at(j));
      _nseqs += count;
      unsigned idx = _name2idx[seqNames.at(j)];
      _traitMat.at(idx).at(i) = count;
      
      string & label = _idx2name.at(_orig2condidx[idx]);
      _seqCounts[label] += count;
    }
        
    //++traitIt;
  }  
}

void Statistics::computeDistances()
{
  for (unsigned i = 0; i < _nseqs; i++)
  {
    _distances.push_back(vector<unsigned>());
    _distances.at(i).resize(_nseqs, 0);
  }
  
  for (unsigned i = 0; i < _nseqs; i++)
  {
    for (unsigned j = 0; j < i; j++)
    {
      _distances.at(i).at(j) = pairwiseDistance(i, j);
      _distances.at(j).at(i) = _distances.at(i).at(j);
      //cout << ' ' << _distances.at(i).at(j);
    }
    //cout << endl;
  }
}

unsigned Statistics::pairwiseDistance(unsigned idx1, unsigned idx2) const
{
  unsigned d = 0;
  
  const Sequence & seq1 = _alignment.at(idx1);
  const Sequence & seq2 = _alignment.at(idx2);
  
  unsigned nsites = seq1.length();
  
  if (nsites < seq2.length())  nsites = seq2.length();
  
  for (unsigned i = 0; i < nsites; i++)
  {
    if ((! Sequence::isAmbiguousChar(seq1.at(i), _datatype) &&  ! Sequence::isAmbiguousChar(seq2.at(i), _datatype)) && seq1.at(i) != seq2.at(i))
      d += _weights.at(i);
  }
  
  return d;
}

double Statistics::nucleotideDiversity() const
{
  double pi;
  unsigned freqI, freqJ;
  map<string, unsigned>::const_iterator countIt;
    
  
  for (unsigned i = 0; i < _alignment.size(); i++)
  {
    const Sequence & seqI = _alignment.at(i);
    countIt = _seqCounts.find(seqI.name());
    freqI = countIt->second;
    
    for (unsigned j = 0; j < i; j++)
    {
      const Sequence &seqJ = _alignment.at(j);
      countIt = _seqCounts.find(seqJ.name());
      freqJ = countIt->second;
      pi += _distances.at(i).at(j) * freqI * freqJ;

    }
  }
    
  pi /= (_nsites * _nseqs * (_nseqs - 1) / 2);
  
  return pi;
}

Statistics::stat Statistics::TajimaD() const
{
  stat tajimaStat = {0,0};
  
  unsigned S = _nSegSites;
  if (S == 0)  return tajimaStat;
  unsigned n = _nseqs;
  
  double a1 = 0;
  double a2 = 0;
  
  for (unsigned i = 1; i < n; i++)
  {
    a1 += 1. / i;
    a2 += 1./ (i * i);
  }
  
  double b1 = (n + 1)/(3. * (n - 1));
  double b2 = 2. * (n * n + n + 3) / (9. * n * (n - 1));
  
  double c1 = b1 - (1 / a1);
  double c2 = b2 - (n + 2.)/(a1 * n) + a2 / (a1 * a1);
   
  double e1 = c1 / a1;
  double e2 = c2 / (a1 * a1 + a2);
  
  double khat = nucleotideDiversity() * _nsites;
  
  double D = (khat - S / a1)/ sqrt(e1 * S + e2 * S * (S - 1));
  tajimaStat.value = D;
  
  // Adjust D to calculate probability from a beta distribution with mean = 0, variance = 1
  double Dmin = (2./n -1/a1)/ sqrt(e2);
  double Dmax;
  if (n % 2)
    Dmax = (n / (2. * (n - 1)) - 1./a1)/sqrt(e2);
  else
    Dmax = ((n + 1.)/(2 * n) - 1./a1)/sqrt(e2);

  double Dprime = (Dmax - D) / (Dmax - Dmin);
  
  double alpha = - (1 + Dmin * Dmax) * Dmax / (Dmax - Dmin);
  double beta = (1 + Dmin * Dmax) * Dmin / (Dmax - Dmin);

  double pTajima = betaI(alpha, beta, Dprime);
  
  tajimaStat.prob = pTajima;
  
  return tajimaStat;
}

double Statistics::gammaLn(double xx)
{
  double cof[] = {76.18009172947146, -86.50532032941677, 24.01409824083091, -1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5};
  const unsigned COFSIZ = 6;
  
  double x = xx;
  double y = x;
  double tmp = x + 5.5;
  tmp -= (x + 0.5) * log(tmp);
  
  double ser = 1.000000000190015;
  for (unsigned i = 0; i < COFSIZ; i++)
    ser += cof[i]/++y;
  
  return -tmp + log(2.5066282746310005 * ser/x); 
}

double Statistics::betaI(double a, double b, double x)
{
  double bt;
  if (x < 0 || x > 1)  throw StatsError("x is not a probability.");
  
  if (x == 0 || x == 1)
    bt = 0;
  
  else
    bt = exp(gammaLn(a+b) - gammaLn(a) - gammaLn(b) + a * log(x) + b * log(1 - x));
  
  if (x < ((a + 1)/(a + b + 2)))
    return bt * betaCF(a, b, x)/a;
  else 
    return 1 - bt * betaCF(b, a, 1 - x)/b;
}

double Statistics::betaCF(double a, double b, double x)
{
  double qab = a + b;
  double qap = a + 1;
  double qam = a - 1;
  
  double c = 1;
  double d = 1 - qab * x / qap;
  
  if (abs(d) < FPMIN)
    d = FPMIN;
    
  d = 1./d;
  double h = d;
   
  unsigned m, m2;
  double aa, del;
  for (m = 1; m <= MAXIT; m++)
  {
    m2 = 2 * m;
    aa = m * (b - m) * x / ((qam + m2) * (a + m2));
    
    // first step, even case
    d = 1 + aa * d;
    if (abs(d) < FPMIN)
      d = FPMIN;
      
    c = 1 + aa/c;
    if (abs(c) < FPMIN)
      c = FPMIN;
    
    d = 1/d;
    h *= d * c;
    aa = -(a + m) * (qab + m) * x/ ((a + m2) * (qap + m2));
    
    // second step, odd case
    d = 1 + aa * d;
    if (abs(d) < FPMIN)
      d = FPMIN;
    
    c = 1 + aa/c;
    if (abs(c) < FPMIN)
      c = FPMIN;
      
    d = 1/d;
    del = d * c;
    h *= del;
  
    if (abs(del - 1) < EPS) // done?
      break;
  }
      
  if (m >= MAXIT)
    cerr << "betaCF not converging: a or b too big, or MAXIT too small." << endl;
    
  return h;
}

Statistics::stat Statistics::amova() const
{
  double sswt = 0;
  double ssbt = 0;
  unsigned n = _distances.size();
  double dnsites = (double)_nsites;
  
  if (_traitMat.empty())
    throw StatsError("Traits must be associated prior to AMOVA calculation.");
  unsigned k = _traitMat.at(0).size();

  double x2t, xt, xct, te;
  int nc, ng;

  double Wk = 0.;
  double Bk = 0.;

  for (unsigned i = 0; i < n; i++)
  {
    x2t = xt = ssbt = sswt = 0;
    ng = 0; // number of sequences

    for (unsigned c = 0; c < k; c++)
    {
      xct = 0;
      nc = 0;
      
      for (unsigned j = 0; j < n; j++)
      {
        if (_traitMat.at(j).at(c) > 0 && _distances.at(i).at(j) > 0)
        {
          unsigned dist = _distances.at(i).at(j);// / dnsites;
          x2t += _traitMat.at(j).at(c) * pow((double)dist, 2);
          xct += _traitMat.at(j).at(c) * dist;
          nc += _traitMat.at(j).at(c);
          ng += _traitMat.at(j).at(c);
        }
      }
      
      
      if (nc)
      {
        xt += xct;
        te = (xct * xct)/nc;
        ssbt += te;
        sswt -= te;
      }
    }
    ssbt -= (xt * xt)/ng;
    sswt += x2t;
    
    Wk += sswt;
    Bk += ssbt;
  }
  
  double msw = Wk/(n - k);
  double msb = Bk/(k - 1);
  cout << "Wk: " << Wk << " Bk: " << Bk << endl;
  
  /*
   * for each pair of sequences,
   *   for each cluster, 
   *     if both seqs have non-zero entries,
   *       increment Wk
   *     else if one has a non-zero entry,
   *       increment Bk
   */
  
  Wk = 0;
  Bk = 0;
  double Tk;
  
  unsigned totalN = 0;
  vector<unsigned> clusterSizes(k, 0);
  vector<double> clusterSSW(k, 0);
  
  for (unsigned i = 0; i < n; i++)
  {
    unsigned ni = 0;
    for (unsigned c = 0; c < k; c++)
    {
      ni += _traitMat.at(i).at(c);
      clusterSizes.at(c) += _traitMat.at(i).at(c);
    }
    totalN += ni;
    
    for (unsigned j = 0; j < i; j++)
    {
      unsigned nj = 0;
      double dist = _distances.at(i).at(j);// / dnsites;
      
      for (unsigned c = 0; c < k; c++)
      {
        if (_traitMat.at(i).at(c) > 0 && _traitMat.at(j).at(c) > 0)
          clusterSSW.at(c) += _traitMat.at(i).at(c) * _traitMat.at(j).at(c) * pow(dist,2);
          //Wk += _traitMat.at(i).at(c) * _traitMat.at(j).at(c) * dist;
       
        nj += _traitMat.at(j).at(c);
      }
      Tk += ni * nj * pow(dist, 2);
    }
  }
  

  Tk /= totalN;
  for (unsigned c = 0; c < k; c++)
    Wk += clusterSSW.at(c)/clusterSizes.at(c);

  Bk = Tk - Wk;
  cout << "Wk: " << Wk << " Bk: " << Bk << endl;
  
  msw = Wk / (double)(totalN - k);
  msb = Bk / (double)(k - 1);

  double pamova = 0; // figure this out
  double Famova = msb / msw;

  stat amovaStat;
  amovaStat.value = Famova;
  amovaStat.prob = pamova;

  vector<unsigned> countvect;

  for (unsigned i = 0; i < n; i++)
  {
    countvect.push_back(0);
    for (unsigned c = 0; c < k; c++)
      countvect.at(i) += _traitMat.at(i).at(c);

    cout << "countvect " << i << ": " << countvect.at(i) << endl;
  }

  Tk = 0;
  totalN = 0;
  for (unsigned i = 0; i < n; i++)
  {
    for (unsigned x = 0; x < countvect.at(i); x++)
    {
      totalN++;

      for (unsigned j = 0; j < i; j++)
      {
        unsigned dist2 = pow((double)(_distances.at(i).at(j)), 2);
        for (unsigned y = 0; y < countvect.at(j); y++)
          Tk += dist2;
      }
    }
  }

  cout << "Tk: " << Tk << " totalN: " << totalN << endl;
  cout << "after dividing by totalN: " << ((double)Tk/(double)totalN) << endl;

  return amovaStat;
}

// Phi test
// log all stats to file

