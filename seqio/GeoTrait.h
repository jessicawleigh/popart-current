#ifndef GEOTRAIT_H_
#define GEOTRAIT_H_

#include "Trait.h"

#include <string>
#include <utility>
#include <vector>

class GeoTrait : public Trait
{
public:
  GeoTrait(const std::pair<float,float> &, const std::string &);
  GeoTrait(const std::pair<float,float> &, const Trait &);
  
  const std::pair<float,float> & location() const { return _location; };
  float latitude() const { return _location.first; };
  float longitude() const { return _location.second; };
  void setLocation(const std::pair<float,float> &location) { _location = location; };
  void setLatitude(float lat) { _location.first = lat; };
  void setLongitude(float lon) { _location.second = lon; };
  
  virtual void addSeq(const std::string &, unsigned);
  virtual void addSeq(const std::pair<float,float> &, const std::string &, unsigned);
  std::vector<std::pair<float,float> > seqLocations(const std::string &) const;
  
  static std::vector<GeoTrait> clusterSeqs(const std::vector<std::pair<float,float> >&, const std::vector<std::string> &, const std::vector<unsigned>& = std::vector<unsigned>(), unsigned = 0, const std::vector<std::pair<float,float> >& = std::vector<std::pair<float,float> >(), const std::vector<std::string> & = std::vector<std::string>());
  
  static std::pair<float,float> getCoordinate(const std::string &, const std::string &);
  
private:
  static void kmeans(unsigned, const std::vector<std::pair<float,float> >&);
  static void randomCentroids(unsigned, const std::vector<std::pair<float,float> >&);
  static void optimiseCentroids(const std::vector<std::pair<float,float> >&);
  static void optimiseClusters(const std::vector<std::pair<float,float> >&);
  static double chIndex(const std::vector<std::pair<float,float> >&, double*, double* = 0, double* = 0);
  
  std::pair<float,float> _location;
  std::multimap<std::string, std::pair<float,float> > _seqLocs;
  
  static std::vector<std::pair<float,float> > _centroids;
  static std::vector<unsigned> _clusters;
  const static unsigned ITERATIONS = 1000;
  const static double SMALL = 1E-6;
};

#endif