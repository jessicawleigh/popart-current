#include "Statistics.h"
#include "StatsError.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stack>
using namespace std;

#ifdef NET_QT
#include <QApplication>
#include <QThread>
#endif

//map<Sequence, list<Sequence> > Statistics::_identicalSeqMap = map<Sequence, list<Sequence> >();
//vector<Sequence> Statistics::_alignment = vector<Sequence>();

unsigned Statistics::permuteCount = 0;


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
  if (thread() != QApplication::instance()->thread())
    thread()->exit();
#endif
}

#ifdef NET_QT
/**
 * Progress should be a percentage (between 0 and 100)
 */
void Statistics::updateProgress(int progress)
{

  // deal with invalid progress values
  //if (progress < 0 || progress > 100)  throw StatsError("Progress is not a percentage.");  
  if (progress < 0)
    emit progressUpdated(0);
  if (progress > 100)
    emit progressUpdated(100);
  
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
  ostringstream *buffers = new ostringstream[_nseqs];
  
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
    
  delete [] buffers;

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

// Allow a sequence to be associated with different traits with different sample counts
void Statistics::setFreqsFromTraits(const vector<Trait *> & traitVect)
{
  
  if (traitVect.empty())  return;
  
  _nseqs = 0;
  
  _traitMat.clear();
  _traitGroups.clear();
  
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
    _traitGroups.push_back(traitVect.at(i)->group());
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
  double pi = 0;
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

  if (Dmax <= D)
  {
	  cerr << "D: " << D << " Dmax: " << Dmax << endl;
	  Dmax = D;
  }

  if (Dmin >= D)
  {
	  cerr << "D: " << D << " Dmin: " << Dmin << endl;
	  Dmin = D;
  }

  double Dprime;

  if (Dmax == Dmin)
  {
	  cerr << "D: " << D << " Dmin: " << Dmin << " Dmax: " << Dmax << endl;
	  Dprime = 0;
  }
  else
    Dprime = (Dmax - D) / (Dmax - Dmin);
  
  double alpha = - (1 + Dmin * Dmax) * Dmax / (Dmax - Dmin);
  double beta = (1 + Dmin * Dmax) * Dmin / (Dmax - Dmin);
  //cout << "alpha: " << alpha << " beta: " << beta << " Dprime: " << Dprime << endl;

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

// TODO write permutation-based significance assessment
// Check that there's more than one group
Statistics::amovatab Statistics::nestedAmova() const
{

  if (_traitMat.empty())
    throw StatsError("Traits must be associated prior to AMOVA calculation.");

  //const unsigned Iterations = 1000;
  unsigned totalN = 0; // total number of individuals 
  unsigned nunique = _distances.size();
  unsigned npop = _traitGroups.size();
  unsigned ngroup = 0;
  //vector<map<unsigned, unsigned> > popCounters(npop);
  vector<unsigned> ncopies;
  vector<vector<unsigned> > ncopiesByGroup;
  //vector<vector<unsigned> > popMat(nunique, vector<unsigned>(npop, 0));
  
  // do we need this? Need number of copies of each unique genotype, not just totalN
  for (unsigned i = 0; i < nunique; i++)
  {
    unsigned ni = 0;
    for (unsigned p = 0; p < npop; p++)
    {
      ni += _traitMat.at(i).at(p);
      if (_traitGroups.at(p) >= ngroup)  
      {
        ngroup = _traitGroups.at(p) + 1;
        ncopiesByGroup.resize(ngroup, vector<unsigned>(nunique, 0));
      }
      
      ncopiesByGroup.at(_traitGroups.at(p)).at(i) += _traitMat.at(i).at(p);
      //if (_traitMat.at(i).at(p) > 0)
      //  popCounters.at(p)[i] = _traitMat.at(i).at(p);


    }
    totalN += ni;
    ncopies.push_back(ni);
  }
  
  amovatab result;
  nestedAmovaPrivate(_traitMat, _traitGroups, result);
  
  /*vector<double> tmpweights;
  int tmparray[] = {15, 32, 0, 6, 6, 0, 16, 29, 18, 42};
  tmpweights.assign(tmparray, tmparray + 10);
  
  DiscreteDistribution dd(tmpweights);
  
  vector<double>::const_iterator probit = dd.probabilities().begin();
  
  cout << "probabilities:";
  while (probit != dd.probabilities().end())
  {
    cout << ' ' << *probit;
    ++probit;
  }
  
  cout << endl;
  
  vector<unsigned> tmpcounts(tmpweights.size(), 0);
  for (unsigned i = 0; i < Iterations; i++)
    tmpcounts.at(dd.sample()) ++;
  
  cout << "counts:";
  for (unsigned i = 0; i < tmpcounts.size(); i++)
    cout << ' ' << tmpcounts.at(i);
  
  cout << endl;*/
    
  // Don't need all these counters, probably... either the sigmas or the phis.
  // counters for number of as-or-more extreme random values
  unsigned sigma2cSmaller = 0, phiSTbigger = 0;
  unsigned sigma2bBigger = 0, phiSCbigger = 0;
  unsigned sigma2aBigger = 0, phiCTbigger = 0;
  
  amovatab permutedResult;
  vector<vector<unsigned > > traitMatCopyAll(_traitMat);
  vector<vector<unsigned > > traitMatCopyGroups(_traitMat);
  vector<unsigned> traitGroupsCopy(_traitGroups);
  
  /*ofstream sigmafile("sigma.out");
  sigmafile << "sigma2c\tsigma2b\tsigma2a\n";
  ofstream phifile("phi.out");
  phifile << "phiST\tphiSC\tphiCT\n";*/
  
  for (unsigned i = 0; i < Iterations; i++)
  { 
    // phiST: how similar are individuals in populations relative to randomised populations?
    // test phiST by permuting individuals among populations 
    // big phiST is more extreme
	cout << "iteration: " << (i + 1) << endl;
    permuteAll(traitMatCopyAll, ncopies);
    cout << "done with first permutation" << endl;
    nestedAmovaPrivate(traitMatCopyAll, _traitGroups, permutedResult);
    cout << "finished nested amovaprivate." << endl;
    if (permutedResult.sigma2_c < result.sigma2_c)  sigma2cSmaller++;
    if (permutedResult.phiST.value > result.phiST.value)  phiSTbigger++;
    //sigmafile << permutedResult.sigma2_c << '\t';
    //phifile << permutedResult.phiST.value << '\t';
    
    // phiSC: how similar are individuals in populations relative to randomised populations within groups?
    // test phiSC by permuting individuals among populations, but within groups
    permuteInGroups(traitMatCopyGroups, _traitGroups, ncopiesByGroup);
    cout << "done with second permutation" << endl;
    nestedAmovaPrivate(traitMatCopyGroups, _traitGroups, permutedResult);
    if (permutedResult.sigma2_b > result.sigma2_b)  sigma2bBigger++;
    if (permutedResult.phiSC.value > result.phiSC.value)  phiSCbigger++;
    //sigmafile << permutedResult.sigma2_b << '\t';
    //phifile << permutedResult.phiSC.value << '\t';
    
    // phiCT: how similar are individuals in groups relative to randomised groups?
    //test phiCT by permuting populations among groups
    random_shuffle(traitGroupsCopy.begin(), traitGroupsCopy.end());
    cout << "done with third permutation" << endl;
    nestedAmovaPrivate(_traitMat, traitGroupsCopy, permutedResult);
    if (permutedResult.sigma2_a > result.sigma2_a)  sigma2aBigger++;
    if (permutedResult.phiCT.value > result.phiCT.value)  phiCTbigger++;
    //sigmafile << permutedResult.sigma2_a << '\n';
    //phifile << permutedResult.phiCT.value << '\n';
    
    //sigmafile << permutedResult.sigma2_c << '\t' << permutedResult.sigma2_b << '\t' << permutedResult.sigma2_a << '\n';
    //phifile << permutedResult.phiST.value << '\t' << permutedResult.phiSC.value << '\t' << permutedResult.phiCT.value << '\n';
    
  }
  
  //sigmafile.close(); phifile.close();
      
      //test sigma2a by permuting populations among groups    
  
  result.phiCT.prob = ((double)phiCTbigger)/Iterations;
  result.phiSC.prob = ((double)phiSCbigger)/Iterations;
  result.phiST.prob = ((double)phiSTbigger)/Iterations;
  
  /*cout << "phiST: " << result.phiST.value << " phiSC: " << result.phiSC.value << " phiCT: " << result.phiCT.value << endl;
  cout << "sigma2c: " << result.sigma2_c << " sigma2b: " << result.sigma2_b << " sigma2a: " << result.sigma2_a << endl; 
  
  cout << "sigma2c bigger: " << sigma2cSmaller << " phiST smaller: " << phiSTbigger << endl;
  cout << "sigma2b smaller: " << sigma2bBigger << " phiSC smaller: " << phiSCbigger << endl;
  cout << "sigma2a smaller: " << sigma2aBigger << " phiCT smaller: " << phiCTbigger << endl;*/
  
  
  //cout << "permutation count: " << permuteCount << endl;
  return result;
    
}

void Statistics::permuteAll(vector<vector<unsigned> > &popMat, const vector<unsigned> &ncopies) const
{
    
  unsigned nunique = popMat.size();
  unsigned npop = popMat.at(0).size();

  // create a discrete distribution of unique seqs
  
  //vector<double> weights(ncopies.begin(), ncopies.end());
  DiscreteDistribution uniqueDistrib(ncopies);
  
  for (unsigned a = 0; a < nunique; a++)
  {
    vector<unsigned> popMatRow(popMat.at(a));
    for (unsigned p = 0; p < npop; p++)
    {
      for (unsigned i = 0; i < popMatRow.at(p); i++)
      {
        unsigned b = uniqueDistrib.sample();
        if (a != b)
        {
          unsigned u = rand() % ncopies.at(b);
          unsigned q = 0, cumulative = 0;
          //for (unsigned cumulative = 0, q = 0; q < npop; q++)
          while (q < npop)
          {
            cumulative += popMat.at(b).at(q);
            if (cumulative > u)  break;
            q++;
          }
          
          // swap a_i in p and b_u in q
          popMat.at(a).at(q)++;
          popMat.at(a).at(p)--;
          popMat.at(b).at(p)++;
          popMat.at(b).at(q)--;
          //permuteCount++;
        }
        
        //else
        //  permuteCount++; // count a == b as "silent" permutations          
      }
    }
  }   
}

void Statistics::permuteInGroups(vector<vector<unsigned> > &popMat, const vector<unsigned> &popGroups, const vector<vector<unsigned> > &ncopiesByGroup) const
{
  
  unsigned nunique = popMat.size();
  unsigned npop = popMat.at(0).size();
  unsigned ngroup = ncopiesByGroup.size();
  
  vector<DiscreteDistribution> distributions;
  
  for (unsigned g = 0; g < ngroup; g++)  distributions.push_back(DiscreteDistribution(ncopiesByGroup.at(g)));
  
  for (unsigned a = 0; a < nunique; a++)
  {
    vector<unsigned> popMatRow(popMat.at(a));
    for (unsigned p = 0; p < npop; p++)
    {
      unsigned group = popGroups.at(p);
      for (unsigned i = 0; i < popMatRow.at(p); i++)
      {
        unsigned b = distributions.at(group).sample();
        if (a != b)
        {
          unsigned u = rand() % ncopiesByGroup.at(group).at(b);
          unsigned q = 0, cumulative = 0;
          //for (unsigned cumulative = 0, q = 0; q < npop; q++)
          while (q < npop)
          {
            if (popGroups.at(q) == group)
            {
              cumulative += popMat.at(b).at(q);
              if (cumulative > u)  break;
            }
            q++;
          }
          
          // swap a_i in p and b_u in q
          popMat.at(a).at(q)++;
          popMat.at(a).at(p)--;
          popMat.at(b).at(p)++;
          popMat.at(b).at(q)--;
          //permuteCount++;
        }
        
        //else
        //  permuteCount++; // count a == b as "silent" permutations
      }
    }
  }   
}

void Statistics::nestedAmovaPrivate(const vector<vector<unsigned> > &popMat, const vector<unsigned> &popGroups, amovatab &result) const
{

	cout << "in nestedamovaPrivate" << endl;
  unsigned totalN = 0; // total number of individuals 
  
  /* sum of squares: total, among groups, among populations (within groups), 
   * among individuals (within populations) */
  double sst = 0, ssag = 0, ssap = 0, sswp = 0; 
  
  unsigned npop, ngroup, nunique;
  nunique = _distances.size(); // number of unique haplotypes
  
  npop = popGroups.size();
  ngroup = 0;//max_element(popGroups) + 1;
  
  for (unsigned p = 0; p < popGroups.size(); p++)
    if (popGroups.at(p) >= ngroup)  ngroup = popGroups.at(p) + 1;
      
  vector<unsigned> popSizes(npop, 0);
  vector<double> popSSW(npop, 0);
  vector<unsigned> groupSizes(ngroup, 0);
  vector<double> groupSSW(ngroup, 0);
  
  for (unsigned i = 0; i < nunique; i++)
  {
    unsigned ni = 0;
    for (unsigned p = 0; p < npop; p++)
    {
      ni += popMat.at(i).at(p);
      popSizes.at(p) += popMat.at(i).at(p);
      groupSizes.at(popGroups.at(p)) += popMat.at(i).at(p);
    }
    totalN += ni;

    for (unsigned j = 0; j < nunique; j++)
    {
      unsigned nj = 0;
      double dist2 = pow((double)(_distances.at(i).at(j)), 2);
      
      for (unsigned p = 0; p < npop; p++)
      {
        if (popMat.at(i).at(p) > 0 && popMat.at(j).at(p) > 0)
          popSSW.at(p) += popMat.at(i).at(p) * popMat.at(j).at(p) * dist2;
        
        for (unsigned q = 0; q < npop; q++)
        {
          if (popMat.at(j).at(q) > 0 && popGroups.at(p) == popGroups.at(q))
            groupSSW.at(popGroups.at(p)) += popMat.at(i).at(p) * popMat.at(j).at(q) * dist2;      
        }
        
        nj += popMat.at(j).at(p);
      }
      
      sst += ni * nj * dist2;
    }
  }
  
  
  sst /= (2 * totalN);
  
  // Excoffier's notation: average population sizes
  double n = totalN, nprime = 0, nprimeprime = 0;
  unsigned sumPop2sizes = 0;
  unsigned sumGroup2sizes = 0;
  //unsigned sumGroupPop2sizes = 0;
  vector<unsigned> groupPop2sizes(ngroup, 0);

  for (unsigned p = 0; p < npop; p++)
  {
    sswp += popSSW.at(p) / (2 * popSizes.at(p));
    unsigned pop2size = pow(popSizes.at(p), 2);
    sumPop2sizes += pop2size;
    groupPop2sizes.at(popGroups[p]) += pop2size;
  }
  
  for (unsigned g = 0; g < ngroup; g++)
  {   
    ssap += groupSSW.at(g) / (2 * groupSizes.at(g));
    sumGroup2sizes += pow(groupSizes.at(g), 2);//group2size;
    //sumGroupPop2sizes += groupPop2sizes.at(g);
    
    n -= ((double)groupPop2sizes.at(g)) / groupSizes.at(g);
    nprime += ((double)groupPop2sizes.at(g)) / groupSizes.at(g);
  }
  
  n /= (npop - ngroup);
  nprime -= ((double)sumPop2sizes)/totalN;
  nprime /= (ngroup - 1);
  nprimeprime = (totalN - ((double)sumGroup2sizes) / totalN) / (ngroup - 1);
    
  ssap -= sswp;
  
  ssag = sst - ssap - sswp;

  double msag = ssag / (ngroup - 1);
  double msap = ssap / (npop - ngroup);
  double mswp = sswp / (totalN - npop);
  
  
  /*cout << "totalN: " << totalN << " npop: " << npop << " ngroup: " << ngroup << endl;
  cout << "ssag: " << ssag << " msag: " << msag << " dfag: " << (ngroup - 1) << endl;
  cout << "ssap: " << ssap << " msap: " << msap << " dfap: " << (npop - ngroup) << endl;
  cout << "sswp: " << sswp << " mswp: " << mswp << " dfwp: " << (totalN - npop) << endl;
  
  cout << "n: " << n << " n': " << nprime << " n'': " << nprimeprime << endl;*/
  
  double sigma2c = mswp; 
  double sigma2b = (msap - sigma2c)/n; 
  double sigma2a = (msag - sigma2c - (nprime * sigma2b))/nprimeprime;
  
  //cout << "sigma2a: " << sigma2a << " sigma2b: " << sigma2b << " sigma2c: " << sigma2c << endl;

  result.ss_ag = ssag;
  result.ss_ap = ssap;
  result.ss_wp = sswp;
  result.df_ag = ngroup -1;
  result.df_ap = npop - ngroup;
  result.df_wp = totalN - npop;
  result.ms_ag = msag;
  result.ms_ap = msap;
  result.ms_wp = mswp;
  result.sigma2_a = sigma2a;
  result.sigma2_b = sigma2b;
  result.sigma2_c = sigma2c;
  result.phiST.value = (sigma2a + sigma2b)/(sigma2a + sigma2b + sigma2c);
  result.phiCT.value = sigma2a/(sigma2a + sigma2b + sigma2c);
  result.phiSC.value = sigma2b/(sigma2b + sigma2c);
  
  //cout << "phiST: " << tab.phiST << " phiCT: " << tab.phiCT << " phiSC: " << tab.phiSC << endl;
  
}

// TODO combine amova and nested amova?
// 

Statistics::amovatab Statistics::amova() const
{
  if (_traitMat.empty())
    throw StatsError("Traits must be associated prior to AMOVA calculation.");

  //const unsigned Iterations = 1000;

  amovatab result;
  amovaPrivate(_traitMat, result);
  
  amovatab permutedResult;
  vector<vector<unsigned> > traitMatCopy(_traitMat);
  
  unsigned nunique = _traitMat.size();
  unsigned npop = _traitMat.at(0).size();
  
  vector<unsigned> ncopies(nunique, 0);
  
  for (unsigned i = 0; i < nunique; i++)
  {
    for (unsigned p = 0; p < npop; p++)
      ncopies.at(i) += _traitMat.at(i).at(p);
  }
  
  unsigned phiSTbigger = 0;
  
  for (unsigned i = 0; i < Iterations; i++)
  {
    permuteAll(traitMatCopy, ncopies);
    amovaPrivate(traitMatCopy, permutedResult);
    //cout << permutedResult.phiST.value << endl;
    
    if (permutedResult.phiST.value > result.phiST.value)
      phiSTbigger++;
  }
  
  result.phiST.prob = ((double)phiSTbigger)/Iterations;
  
  return result;
}

void Statistics::amovaPrivate(const std::vector<std::vector<unsigned> > &popMat, amovatab &result) const
{
  unsigned n = _distances.size();  
  unsigned k = popMat.at(0).size();//_traitMat.at(0).size();

  double Wk = 0.;
  double Bk = 0.;  
  double Tk = 0;      
  double meanGroupSize = 0;
  
  unsigned totalN = 0;
  vector<unsigned> groupSizes(k, 0);
  vector<double> groupSSW(k, 0);
  
  for (unsigned i = 0; i < n; i++)
  {
    unsigned ni = 0;
    for (unsigned c = 0; c < k; c++)
    {
      ni += popMat.at(i).at(c);//_traitMat.at(i).at(c);
      groupSizes.at(c) += popMat.at(i).at(c);//_traitMat.at(i).at(c);
    }
    totalN += ni;
    
    for (unsigned j = 0; j < n; j++)
    {
      unsigned nj = 0;
      double dist2 = pow((double)(_distances.at(i).at(j)), 2);// / dnsites;
      
      for (unsigned c = 0; c < k; c++)
      {
        if (popMat.at(i).at(c) > 0 && popMat.at(j).at(c) > 0)
        {
          groupSSW.at(c) += popMat.at(i).at(c) * popMat.at(j).at(c) * dist2;

        }
          //Wk += popMat.at(i).at(c) * popMat.at(j).at(c) * dist;
       
        nj += popMat.at(j).at(c);
      }
      Tk += ni * nj * dist2;
    }
  }
  
  
  Tk /= (2 * totalN);  
  
  for (unsigned c = 0; c < k; c++)
  {
    Wk += groupSSW.at(c)/(2 * groupSizes.at(c));
    meanGroupSize += groupSizes.at(c);
  }
  
  // Not quite the mean... see Arlequin 3.5 manual p. 147, what Excoffier calls 'n' 
  meanGroupSize /= (k - 1);

  Bk = Tk - Wk;
  
  unsigned dfw = totalN - k;
  unsigned dfb = k - 1;

  double msw = Wk / dfw;//(totalN - k);
  double msb = Bk / dfb;//(k - 1);
  
  /* double Famova = msb / msw;
  
  unsigned df1 = k - 1;
  unsigned df2 = totalN - k;
  double x = df1 * Famova / (df1 * Famova + df2);
  double pamova = betaI(df1/2.0, df2/2.0, x);*/
  
  //cout << "F: " << Famova << " p: " << setprecision(10) << pamova << endl;
  double sigma2a, sigma2b = msw;  
  sigma2a = (msb - sigma2b)/meanGroupSize;
  

  /*anovatab amovaStat;
  amovaStat.ssb = Bk;
  amovaStat.ssw = Wk;
  amovaStat.dfFac = df1;
  amovaStat.dfRes = df2;
  amovaStat.msb = msb;
  amovaStat.msw = msw;
  amovaStat.F = Famova;
  amovaStat.prob = 1 - pamova;
  amovaStat.phiST = (sigma2a / (sigma2a + sigma2b));


  return amovaStat;*/
  
  result.ss_ag = NAN;
  result.ss_ap = Bk;
  result.ss_wp = Wk;
  result.df_ag = NAN;
  result.df_ap = dfb;
  result.df_wp = dfw;
  result.ms_ag = NAN;
  result.ms_ap = msb;
  result.ms_wp = msw;
  result.sigma2_a = sigma2a;
  result.sigma2_b = sigma2b;
  result.sigma2_c = NAN;
  result.phiST.value = sigma2a / (sigma2a + sigma2b);
  result.phiCT.value = result.phiSC.value = NAN;
  
}

Statistics::DiscreteDistribution::DiscreteDistribution(const vector <double> &weights)
 : _weights(weights), _nitems(weights.size())
{
  setupTables();
}

Statistics::DiscreteDistribution::DiscreteDistribution(const vector <unsigned> &weights)
 : _weights(weights.begin(), weights.end()), _nitems(weights.size())
{
  setupTables();
}

void Statistics::DiscreteDistribution::setupTables()
{
  double totalweight = 0;
  for (unsigned i = 0; i < _nitems; i++)
    totalweight += _weights.at(i);
  
  double avgweight = totalweight/_nitems;
  
  stack<unsigned> large;
  stack<unsigned> small;
  _alias.resize(_nitems);
    
  for (unsigned i = 0; i < _nitems; i++)
  {
    _probs.push_back(_weights.at(i)/totalweight);
    _weights.at(i) /= avgweight;
    
    if (_weights.at(i) < 1)  small.push(i);
    else  large.push(i);
  }
  
  while (! small.empty() && ! large.empty())
  {
    unsigned s = small.top();
    small.pop();
    
    unsigned l = large.top();
    large.pop();
    
    _alias[s] = l;
    _weights[l] = _weights[l] + _weights[s] - 1;
    
    if (_weights[l] < 1)  small.push(l);
    else  large.push(l);
  }
  
  while (! large.empty())
  {
    unsigned l = large.top();
    large.pop();
    
    _weights[l] = 1;
  }
  
  while (! small.empty())
  {
    unsigned s = small.top();
    small.pop();
    
    _weights[s] = 1;
  }
}

unsigned Statistics::DiscreteDistribution::sample() const
{
  unsigned column = rand() % _nitems;
  double u = (rand() % LARGE) / ((double)LARGE);
  
  if (u <= _weights[column])  return column;
  
  return _alias[column];
}


