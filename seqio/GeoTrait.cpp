#include "GeoTrait.h"
#include "ParserTools.h"
#include "SequenceError.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

vector<pair<float,float> > GeoTrait::_centroids;
vector<unsigned> GeoTrait::_clusters;

GeoTrait::GeoTrait(const pair<float,float> &location, const string &name)
 : Trait(name)
{
  _location = location;
}

GeoTrait::GeoTrait(const std::pair<float,float> &location, const Trait &trait)
 : Trait(trait)
{
  _location = location;
}

void GeoTrait::addSeq(const string &seqname, unsigned seqcount)
{
  addSeq(pair<float,float>(0,0), seqname, seqcount);
}

void GeoTrait::addSeq(const pair<float,float> &location, const string &seqname, unsigned seqcount)
{
  Trait::addSeq(seqname, seqcount);
  
  _seqLocs.insert(pair<string,pair<float,float> >(seqname, location));
  _seqCounts.insert(pair<string,unsigned>(seqname, seqcount)); // import for accessing counts of individual sequences
  //_seqLocs[seqname] = location;
}

std::vector<std::pair<float, float> > GeoTrait::seqLocations(const string &seqname) const
{
  pair<multimap<string, pair<float,float> >::const_iterator,multimap<string, pair<float,float> >::const_iterator> itpair = _seqLocs.equal_range(seqname);
  
  multimap<string, pair<float,float> >::const_iterator traitIt = itpair.first;
  
  if (traitIt == _seqLocs.end())  throw SequenceError("Sequence not associated with this trait.");
  
  vector <pair<float,float> > locs;
  
  while (traitIt != itpair.second)
  {
    locs.push_back(traitIt->second);
    ++traitIt;
  }

  return locs;
}

std::vector<unsigned> GeoTrait::seqCounts(const string &seqname) const
{
  pair<multimap<string,unsigned>::const_iterator,multimap<string,unsigned>::const_iterator> itpair = _seqCounts.equal_range(seqname);

  multimap<string,unsigned>::const_iterator traitIt = itpair.first;

  if (traitIt == _seqCounts.end())  throw SequenceError("Sequence not associated with this trait.");

  vector<unsigned> counts;

  while (traitIt != itpair.second)
  {
    counts.push_back(traitIt->second);
    ++traitIt;
  }

  return counts;
}

vector<GeoTrait> GeoTrait::clusterSeqs(const vector<pair<float,float> >& seqLocations, const vector<string> &seqNames, const vector<unsigned> &seqCounts, unsigned nClusters, const vector<pair<float,float> > &clusterLocations, const vector<string> &clusterNames)
{
  _centroids.clear();
  _clusters.resize(seqLocations.size(), 0);
  
  if (seqLocations.size() != seqNames.size())
    throw SequenceError("Number of sequence names doesn't match number of locations.");
  
  if (! seqCounts.empty() && seqCounts.size() != seqNames.size())
    throw SequenceError("Number of sequence names doesn't match number of counts.");

  if ((! clusterLocations.empty() && clusterLocations.size() != nClusters) ||
    (! clusterNames.empty() && clusterNames.size() != nClusters))
    throw SequenceError("Number of names or locations doesn't match number of clusters.");
  
  if (clusterLocations.empty() && ! clusterNames.empty())
    throw SequenceError("Cluster names are meaningless without associated locations.");
  
  
  if (nClusters > 0)
  {
    if (clusterLocations.empty())
    {
      kmeans(nClusters, seqLocations);
    }
    
    else
    {
      _centroids.assign(clusterLocations.begin(), clusterLocations.end());
      optimiseClusters(seqLocations); 
    }
  }
  
  else
  {
    double bestScore = 0;
    vector<unsigned> bestClusters;
    vector<pair<float,float> > bestCentroids;
    
    double *distances = new double[seqLocations.size() * seqLocations.size()];
    
    for (unsigned i = 0; i < seqLocations.size(); i++)
    {
      for (unsigned j = 0; j < i; j++)
      {
        double d = sqrt(pow(seqLocations.at(i).first - seqLocations.at(j).first, 2) + pow(seqLocations.at(i).second - seqLocations.at(j).second, 2));
        distances[i * seqLocations.size() + j] = d;
        distances[j * seqLocations.size() + i] = d;
      }
    }
    
    //cout << "K\tCH" << endl;//\tSSW\tSSB" << endl;
    
    for (unsigned i = 2; i < seqLocations.size(); i++)
    {
      double ssw;// = sumSqWithin(seqLocations);
      double ssb;// = sumSqBetween(seqLocations);
      double score;
      
      double bestSSWk;
      double bestScoreK;
      vector<unsigned> bestClustersK;
      vector<pair<float,float> > bestCentroidsK;
      
      
      // choose clustering with minimal ssw for a given n
      //cout << i << '\t';
      for (unsigned j = 0; j < ITERATIONS; j++)
      {
        kmeans(i, seqLocations);
        score = chIndex(seqLocations, distances, &ssw, &ssb);
        if (j == 0 || ssw < (bestSSWk - SMALL))
        {
          bestSSWk = ssw;
          bestScoreK = score;
          //bestClustersK.clear();
          bestClustersK.assign(_clusters.begin(), _clusters.end());
          //bestCentroidsK.clear();
          bestCentroidsK.assign(_centroids.begin(), _centroids.end());
        } 
      }
      //cout << bestScoreK;// << '\t' << bestSSWk << '\t' << ssb;
      
      if (bestScoreK > bestScore)
      {
        bestScore = bestScoreK;
        //bestClusters.clear();
        bestClusters.assign(bestClustersK.begin(), bestClustersK.end());
        //bestCentroids.clear();
        bestCentroids.assign(bestCentroidsK.begin(), bestCentroidsK.end());
        //cout << "*";
      }
      
      //cout << endl;
    }
    
    if (bestCentroids.size() == seqLocations.size())
    {
      cerr << "Warning, CH index is inconclusive for estimating number of clusters." << endl;
    }
    
    //_centroids.clear();
    _centroids.assign(bestCentroids.begin(), bestCentroids.end());
    
    //_clusters.clear();
    _clusters.assign(bestClusters.begin(), bestClusters.end());
    
    /*for (unsigned i = 0; i < _clusters.size(); i++)
      cout << ' ' << _clusters.at(i);
    cout << endl;
    
    for (unsigned i = 0; i < _centroids.size(); i++)
      cout << _centroids.at(i).first << "," << _centroids.at(i).second << endl;*/
    
    delete [] distances;
    
  }
  
  vector<GeoTrait> geoTraits;
  
  for (unsigned i = 0; i < _centroids.size(); i++)
  {
    ostringstream oss;
    if (clusterNames.empty())
      oss << "cluster" << (i + 1);
    else
      oss << clusterNames.at(i);
    
    geoTraits.push_back(GeoTrait(_centroids.at(i), oss.str()));
  }
  
  for (unsigned i = 0; i < _clusters.size(); i++)
  {
    unsigned count = 1;
    if (! seqCounts.empty())
      count = seqCounts.at(i);
    geoTraits.at(_clusters.at(i)).addSeq(seqLocations.at(i), seqNames.at(i), count);
  }
  
  return geoTraits;
}

// a single round of k-means
void GeoTrait::kmeans(unsigned nClusts, const vector<pair<float,float> >& seqLocations)
{
  randomCentroids(nClusts, seqLocations);
  optimiseClusters(seqLocations);
  vector<unsigned> lastClusts(_clusters);
  bool clustsDiff = true;
  
  while (clustsDiff)
  {
    optimiseCentroids(seqLocations);
    optimiseClusters(seqLocations);
    if (_clusters == lastClusts)
      clustsDiff = false;
    else
      lastClusts.assign(_clusters.begin(), _clusters.end());
  }
}

void GeoTrait::randomCentroids(unsigned nClusts, const vector<pair<float,float> >& seqLocations)
{
  vector<unsigned> indices;
  for (unsigned i = 0; i < seqLocations.size(); i++)
    indices.push_back(i);
  
  for (unsigned i = seqLocations.size() - 1; i > 1; i--)
  {
    unsigned j = rand() % (i + 1);
    swap(indices.at(i), indices.at(j));
  }
  
  _centroids.clear();
  
  for (unsigned i = 0; i < nClusts; i++)
    _centroids.push_back(seqLocations.at(indices.at(i)));
}

void GeoTrait::optimiseCentroids(const vector<pair<float,float> > &locations)
{
  vector<unsigned> sizes;
  for (unsigned i = 0; i < _centroids.size(); i++)
  {
    _centroids.at(i).first = 0;
    _centroids.at(i).second = 0;
    sizes.push_back(0);
  }
  
  for (unsigned i = 0; i < _clusters.size(); i++)
  {
    pair<float,float> &cent = _centroids.at(_clusters.at(i));
    cent.first += locations.at(i).first;
    cent.second += locations.at(i).second;
    sizes.at(_clusters.at(i))++;
  }
  
  for (unsigned i = 0; i < _centroids.size(); i++)
  {
    _centroids.at(i).first /= sizes.at(i);
    _centroids.at(i).second /= sizes.at(i);
  }
}

void GeoTrait::optimiseClusters(const vector<pair<float,float> > &locations)
{
  for (unsigned i = 0; i < locations.size(); i++)
  {
    double minDist = -1;
    for (unsigned j = 0; j < _centroids.size(); j++)
    {
      double dist = sqrt(pow(locations.at(i).first - _centroids.at(j).first, 2) + pow(locations.at(i).second - _centroids.at(j).second, 2));
      
      if (minDist < 0 || dist < minDist)
      {
        minDist = dist;
        _clusters.at(i) = j;
      }
    }
  }
}

double GeoTrait::chIndex(const std::vector<pair<float,float> > &locations, double *distances, double *ssw, double *ssb)
{
  double sswt = 0;
  double ssbt = 0;
  unsigned n = locations.size();
  unsigned k = _centroids.size();
  
  double x2t, xt, xct, te;
  int nc, idx, ng;
  
  double Wk = 0.;
  double Bk = 0.;
  
  for (unsigned i = 0; i < n; i++)
  {
    x2t = xt = ssbt = sswt = 0.;
    ng = 0;
    
    for (unsigned c = 0; c < k; c++)
    {
      xct = 0.;
      nc = 0;

      for (unsigned j = 0; j < n; j++)
      //for (j = 0; j < rows; j++)     
      {
        if (_clusters[j] == c)
        {
          idx = i * n + j;
          if (distances[idx] >= 0)
          {
            x2t += distances[idx] * distances[idx];
            xct += distances[idx];
            nc++;
            ng++;
          }
        }
      }
      
      // if this cluster had any members...
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
  
  // if ssb and ssw were not null pointers, give them values
  if (ssb)
    *ssb = Bk;
  if (ssw)
    *ssw = Wk;
  
  return (Bk / (k - 1)) / (Wk / (n - k));
}

pair<float,float> GeoTrait::getCoordinate(const string &latstr, const string &lonstr)
{
  
  if (latstr.empty() || lonstr.empty())
    throw SequenceError("Latitude or longitude string is empty.");

  float lat;
  float lon;
  char quadrant;
  
  istringstream iss(latstr);
  iss >> lat;
  
  if (iss.fail())  throw SequenceError("Unknown latitude format."); 
  
  iss >> quadrant;
  
  
  if (quadrant != '\0')
  {
    quadrant = tolower(quadrant);
    
    if (quadrant == 's')
      lat *= -1;
    
    else if (quadrant != 'n')
      throw SequenceError("Unknown latitude format.");
  }
  
  iss.clear();
  
  // e causes problems, assumed to be scientific notation
  if (tolower(lonstr.at(lonstr.size() - 1)) == 'e')
  {
    string lonstrcp(lonstr);
    lonstrcp.erase(lonstr.size() - 1);
    iss.str(lonstrcp);
  }
  
  else
    iss.str(lonstr);
  
  iss >> lon;
  
  if (iss.fail())  throw SequenceError("Unknown longitude format."); 
  
  iss >> quadrant;
  
  
  if (quadrant != '\0')
  {
    quadrant = tolower(quadrant);
    
    if (quadrant == 'w')
      lat *= -1;
   
    else if (quadrant != 'e')
      throw SequenceError("Unknown longitude format.");
  }
  
  
  return pair<float, float>(lat, lon);
}
