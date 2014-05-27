#include "GeoTrait.h"
#include "ParserTools.h"
#include "SequenceError.h"

#ifdef NET_QT
#include <QApplication>
#include <QThread>
#include <QDebug>
#endif

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

const double GeoTrait::PI = std::atan(1) * 4;

vector<pair<float,float> > GeoTrait::_centroids;
vector<unsigned> GeoTrait::_clusters;

vector<pair<float,float> > GeoTrait::_statCoords;
vector<string> GeoTrait::_statSeqNames;
vector<unsigned> GeoTrait::_statSeqCounts;
vector<pair<float,float> > GeoTrait::_statClustCoords;
vector<string> GeoTrait::_statClustNames;
unsigned GeoTrait::_statNClusts = 0;
vector<GeoTrait *> GeoTrait::_statGeoTraits;

GeoTrait *GeoTrait::statTrait = new GeoTrait(pair<float,float>(0,0), "static");

#ifdef NET_QT
GeoTrait::GeoTrait(const pair<float,float> &location, const string &name, QObject *parent)
 : QObject(parent), Trait(name)
{
  _location = location;
  
  /*cout << "pi: " << PI << endl;
  cout << "pi, in degrees: " << degrees(PI) << endl;
  cout << "45 degrees, in rad: " << radians(45) << endl;
  cout << "45.0 degrees, in rad: " << radians(45.0) << endl;
  cout << "PI * 180 / PI: " << PI * 180 / PI << endl;
  cout << "3.14159 * 180 / 3.14159: " << 3.14159 * 180 / 3.14159 << endl;
  cout << "PI * 180 / 3.14159: " << PI * 180 / 3.14159 << endl;
  cout << "3.14159 * 180 / PI: " << 3.14159 * 180 / PI << endl;
  
  
  pair<float,float> p1(-40, 170);
  pair<float,float> p2(5, -170);
  
  coord3d c3d = sph2cart(p1);
  
  cout << p1.first << "," << p1.second << " in Cartesian: (" << c3d.x << "," << c3d.y << "," << c3d.z << ")" << endl;
  
  cout << "Great circle distance (r = 1): " << greatcircle(p1, p2) << endl;
  cout << "Great circle distance (r = 6371): " << greatcircle(p1, p2, 6371) << endl;*/
}

GeoTrait::GeoTrait(const std::pair<float,float> &location, const Trait &trait, QObject *parent)
 : QObject(parent), Trait(trait)
{
  _location = location;
}

GeoTrait::GeoTrait(const GeoTrait &other, QObject *parent)
  : QObject(parent), Trait(other)
{
  _location = other._location;
  _seqLocs = other._seqLocs;
  _seqCounts = other._seqCounts;
}

#else

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

#endif

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

void GeoTrait::updateProgress(int progress)
{
#ifdef NET_QT
  emit progressUpdated(progress);
#else
  cout << "clustering progress: " << progress << '%' << endl;
#endif
}

void GeoTrait::setupStaticData(const vector<pair<float,float> >& seqLocations, const vector<string> &seqNames, const vector<unsigned> &seqCounts, unsigned nClusters, const vector<pair<float,float> > &clusterLocations, const vector<string> &clusterNames)
{
  _statCoords = seqLocations;
  _statSeqNames = seqNames;
  _statSeqCounts = seqCounts;
  _statNClusts = nClusters;
  _statClustCoords = clusterLocations;
  _statClustNames = clusterNames;
}

void GeoTrait::processClustering()
{
  _statGeoTraits = clusterSeqs(_statCoords, _statSeqNames, _statSeqCounts, _statNClusts, _statClustCoords, _statClustNames);
  
}

const vector<GeoTrait*> & GeoTrait::getClusterResult() const
{
  return _statGeoTraits;
}

vector<GeoTrait*> GeoTrait::clusterSeqs(const vector<pair<float,float> >& seqLocations, const vector<string> &seqNames, const vector<unsigned> &seqCounts, unsigned nClusters, const vector<pair<float,float> > &clusterLocations, const vector<string> &clusterNames)
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
  
    double *distances = new double[seqLocations.size() * seqLocations.size()];
    
  
    // calculate great circle distances
    // create a 3d coordinate struct? Or just great circle distances to centroids as well
    // 
    for (unsigned i = 0; i < seqLocations.size(); i++)
    {
      for (unsigned j = 0; j < i; j++)
      {
        double d = greatcircle(seqLocations.at(i), seqLocations.at(j));//sqrt(pow(seqLocations.at(i).first - seqLocations.at(j).first, 2) + pow(seqLocations.at(i).second - seqLocations.at(j).second, 2));
        distances[i * seqLocations.size() + j] = d;
        distances[j * seqLocations.size() + i] = d;
      }      
    }
  
  if (nClusters > 0)
  {
    if (clusterLocations.empty())
      iterativeKmeans(nClusters, seqLocations, distances, ITERATIONS, true);
      //kmeans(nClusters, seqLocations);
    
    else
    {
      _centroids.assign(clusterLocations.begin(), clusterLocations.end());
      optimiseClusters(seqLocations); 
      statTrait->updateProgress(100);
    }
  }
  
  else
  {
    double bestScore = 0;
    vector<unsigned> bestClusters;
    vector<pair<float,float> > bestCentroids;
    
    
    //cout << "K\tCH" << endl;//\tSSW\tSSB" << endl;
    
    double progPerIter = 1./(seqLocations.size() - 2);
    double prog = 0;
    
    for (unsigned i = 2; i < seqLocations.size(); i++)
    {
      //cout << i << '\t';
      double bestScoreK = iterativeKmeans(i, seqLocations, distances, ITERATIONS);
      //cout << bestScoreK;// << '\t' << bestSSWk << '\t' << ssb;
      
      
      if (bestScoreK > bestScore)
      {
        bestScore = bestScoreK;
        bestClusters.assign(_clusters.begin(), _clusters.end());
        bestCentroids.assign(_centroids.begin(), _centroids.end());
        //cout << "*";
      }
      
      //cout << endl;
      prog += progPerIter;
      statTrait->updateProgress((int)(prog * 100 + 0.5));
    }
    
    if (bestCentroids.size() == seqLocations.size())
    {
      cerr << "Warning, CH index is inconclusive for estimating number of clusters." << endl;
    }
    
    _centroids.assign(bestCentroids.begin(), bestCentroids.end());
    _clusters.assign(bestClusters.begin(), bestClusters.end());
    
    delete [] distances;
    
  }
  
  vector<GeoTrait *> geoTraits;
  
  for (unsigned i = 0; i < _centroids.size(); i++)
  {
    ostringstream oss;
    if (clusterNames.empty())
      oss << "cluster" << (i + 1);
    else
      oss << clusterNames.at(i);
    
    geoTraits.push_back(new GeoTrait(_centroids.at(i), oss.str()));
  }
  
  for (unsigned i = 0; i < _clusters.size(); i++)
  {
    unsigned count = 1;
    if (! seqCounts.empty())
      count = seqCounts.at(i);
    geoTraits.at(_clusters.at(i))->addSeq(seqLocations.at(i), seqNames.at(i), count);
  }

#ifdef NET_QT
  if (statTrait->thread() != QApplication::instance()->thread())
    statTrait->thread()->exit();
#endif

  return geoTraits;
}

double GeoTrait::greatcircle(const std::pair<float,float>& coords1, const std::pair<float,float>& coords2, double r)
{  
  double lat1 = radians(coords1.first);// * pi/180;
  double lon1 = radians(coords1.second);// * pi/180;
  double lat2 = radians(coords2.first);// * pi/180;
  double lon2 = radians(coords2.second);// * pi/180;
    
  double dLat = lat2 - lat1;
  double dLon = lon2 - lon1;
    
  double h = pow(sin(dLat/2), 2) + cos(lat1) * cos(lat2) * pow(sin(dLon/2), 2);
      
  return 2 * r * asin(sqrt(h));
    

}

GeoTrait::coord3d GeoTrait::sph2cart(const std::pair<float,float> & sphCoords, double r)
{
  coord3d cartesian;
  cartesian.x = r * cos(radians(sphCoords.second)) * cos(radians(sphCoords.first));
  cartesian.y = r * sin(radians(sphCoords.second)) * cos(radians(sphCoords.first));
  cartesian.z = sin(radians(sphCoords.first));
  
  return cartesian;
}

pair<float,float> GeoTrait::cart2sph(const GeoTrait::coord3d &cartCoords, double * rad)
{
  double r = sqrt(pow(cartCoords.x, 2) + pow(cartCoords.y, 2) + pow(cartCoords.z,2));
  double lat = asin(cartCoords.z / r);
  double lon = atan2(cartCoords.y, cartCoords.x);
  
  lat = degrees(lat);
  lon = degrees(lon);
  
  if (rad)
    *rad = r;
  
  return pair<float,float>((float)lat, (float)lon);
}

/*double GeoTrait::radians(double deg)
{
  return deg * pi / 180;
}

double GeoTrait::degrees(double rad)
{
  return rad * 180 / pi;
}
*/

double GeoTrait::iterativeKmeans(unsigned nClusts, const vector<pair<float,float> >& seqLocations, double *distances, unsigned iterations, bool showProgress)
{
  double ssw;
  double ssb;
  double score;
  double bestSSW;
  double bestScore;
  vector<unsigned> bestClusters;
  vector<pair<float,float> > bestCentroids;
  
  double progPerIter = 1./iterations;
  double prog = 0;
  
  for (unsigned j = 0; j < iterations; j++)
  {
    kmeans(nClusts, seqLocations);
    score = chIndex(seqLocations, distances, &ssw, &ssb);
    if (j == 0 || ssw < (bestSSW - SMALL))
    {
      bestSSW = ssw;
      bestScore = score;
      bestClusters.assign(_clusters.begin(), _clusters.end());
      bestCentroids.assign(_centroids.begin(), _centroids.end());
    } 
    
    prog += progPerIter;
    if (showProgress)
      statTrait->updateProgress(prog * 100 + 0.5);
  }
  
  _clusters.assign(bestClusters.begin(), bestClusters.end());
  _centroids.assign(bestCentroids.begin(), bestCentroids.end());
  
  if (showProgress)
    statTrait->updateProgress(prog * 100 + 0.5);
  
  return bestScore;
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
  vector<coord3d> cartesian;
  for (unsigned i = 0; i < _centroids.size(); i++)
  {
    _centroids.at(i).first = 0;
    _centroids.at(i).second = 0;
    cartesian.push_back((coord3d){0,0,0});
    sizes.push_back(0);
  }
  
  for (unsigned i = 0; i < _clusters.size(); i++)
  {
    //pair<float,float> &cent = _centroids.at(_clusters.at(i));
    //cent.first += locations.at(i).first;
    //cent.second += locations.at(i).second;
    coord3d & cent = cartesian.at(_clusters.at(i));
    coord3d locCart = sph2cart(locations.at(i));
    
    cent.x += locCart.x;
    cent.y += locCart.y;
    cent.z += locCart.z;
    
    sizes.at(_clusters.at(i))++;
  }
  
  for (unsigned i = 0; i < _centroids.size(); i++)
  {
    coord3d & cent = cartesian.at(i);
    cent.x /= sizes.at(i);
    cent.y /= sizes.at(i);
    cent.z /= sizes.at(i);
    // _centroids.at(i).first /= sizes.at(i);
    // _centroids.at(i).second /= sizes.at(i);
    _centroids.at(i) = cart2sph(cent);
  }
}

void GeoTrait::optimiseClusters(const vector<pair<float,float> > &locations)
{
  for (unsigned i = 0; i < locations.size(); i++)
  {
    double minDist = -1;
    for (unsigned j = 0; j < _centroids.size(); j++)
    {
      double dist = greatcircle(locations.at(i), _centroids.at(j));
      //sqrt(pow(locations.at(i).first - _centroids.at(j).first, 2) + pow(locations.at(i).second - _centroids.at(j).second, 2));
      
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
