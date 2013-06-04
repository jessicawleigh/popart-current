#ifndef GEOTRAIT_H_
#define GEOTRAIT_H_

#include "Trait.h"

#include <string>
#include <utility>
#include <vector>
using namespace std;


#ifdef NET_QT
#include <QObject>
class GeoTrait : public QObject, public Trait
{
  Q_OBJECT
public:
  GeoTrait(const std::pair<float,float> &, const std::string &, QObject * = 0);
  GeoTrait(const std::pair<float,float> &, const Trait &, QObject * = 0);
  GeoTrait(const GeoTrait &, QObject * = 0);
#else
class GeoTrait : public Trait
{
public:
  GeoTrait(const std::pair<float,float> &, const std::string &);
  GeoTrait(const std::pair<float,float> &, const Trait &);
#endif

  const std::pair<float,float> & location() const { return _location; };
  float latitude() const { return _location.first; };
  float longitude() const { return _location.second; };
  void setLocation(const std::pair<float,float> &location) { _location = location; };
  void setLatitude(float lat) { _location.first = lat; };
  void setLongitude(float lon) { _location.second = lon; };
  
  virtual void addSeq(const std::string &, unsigned);
  virtual void addSeq(const std::pair<float,float> &, const std::string &, unsigned);
  std::vector<std::pair<float,float> > seqLocations(const std::string &) const;
  std::vector<unsigned> seqCounts(const std::string &) const;
  
  static std::vector<GeoTrait*> clusterSeqs(const std::vector<std::pair<float,float> >&,
      const std::vector<std::string> &,
      const std::vector<unsigned>& = std::vector<unsigned>(),
      unsigned = 0,
      const std::vector<std::pair<float,float> >&
      = (std::vector<std::pair<float,float> >()),
      const std::vector<std::string> & = std::vector<std::string>());
  
  static void setupStaticData(const std::vector<std::pair<float,float> >&,
      const std::vector<std::string> &,
      const std::vector<unsigned>& = std::vector<unsigned>(),
      unsigned = 0,
      const std::vector<std::pair<float,float> >&
      = (std::vector<std::pair<float,float> >()),
      const std::vector<std::string> & = std::vector<std::string>());
  const std::vector<GeoTrait*> & getClusterResult() const;
  
  static std::pair<float,float> getCoordinate(const std::string &, const std::string &);
  
  // Used to manage signals
  static GeoTrait *statTrait;

  
private:
  static double iterativeKmeans(unsigned, const std::vector<std::pair<float,float> >&, double *distances, unsigned, bool = false);
  static void kmeans(unsigned, const std::vector<std::pair<float,float> >&);
  static void randomCentroids(unsigned, const std::vector<std::pair<float,float> >&);
  static void optimiseCentroids(const std::vector<std::pair<float,float> >&);
  static void optimiseClusters(const std::vector<std::pair<float,float> >&);
  static double chIndex(const std::vector<std::pair<float,float> >&, double*, double* = 0, double* = 0);
  void updateProgress(int);
  
  
  std::pair<float,float> _location;
  std::multimap<std::string, std::pair<float,float> > _seqLocs;
  std::multimap<std::string, unsigned> _seqCounts;
  
  static std::vector<std::pair<float,float> > _centroids;
  static std::vector<unsigned> _clusters;
  const static unsigned ITERATIONS = 1000;
  const static double SMALL = 1E-6;
  
  static std::vector<std::pair<float,float> > _statCoords;
  static std::vector<std::string> _statSeqNames;
  static std::vector<unsigned> _statSeqCounts;
  static unsigned _statNClusts;
  static std::vector<std::pair<float,float> > _statClustCoords;
  static std::vector<std::string> _statClustNames;
  
  static vector<GeoTrait *> _statGeoTraits;
  
#ifdef NET_QT
public slots:
  void processClustering();
signals:
  void progressUpdated(int);
#endif

};

#endif
