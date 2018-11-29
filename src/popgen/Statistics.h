#include "Sequence.h"
#include "Trait.h"

#include <list>
#include <map>
#include <vector>

#ifdef NET_QT
#include <QObject>
#include <QTime>
class Statistics : public QObject
{
  Q_OBJECT
#else
class Statistics
{
#endif
public:
  typedef struct {double value; double prob; } stat;
  typedef struct 
  {
    double ssb; 
    double ssw; 
    unsigned dfFac;
    unsigned dfRes;
    double msb; 
    double msw; 
    double F; 
    double phiST;
    double prob;
  } anovatab;
  
  typedef struct
  {
    double ss_ag; // among groups
    double ss_ap; // among populations/within groups
    double ss_wp; // within populations (among individuals)
    unsigned df_ag;
    unsigned df_ap;
    unsigned df_wp;
    double ms_ag;
    double ms_ap;
    double ms_wp;
    double sigma2_a;
    double sigma2_b;
    double sigma2_c;
    stat phiST;
    stat phiSC;
    stat phiCT;
    
    bool nested;
  } amovatab;
  
  Statistics(const std::vector<Sequence*> & = std::vector<Sequence*>(), const std::vector<bool> & = std::vector<bool>(), Sequence::CharType = Sequence::DNAType);
    
  const std::map<Sequence, std::list<Sequence > > & mapIdenticalSeqs() { return _identicalSeqMap; };
  void setFreqsFromTraits(const std::vector<Trait *> &);
  void setAmovaPointer(amovatab &);
  double nucleotideDiversity() const;
  unsigned nSegSites() const { return _nSegSites; };
  unsigned nParsimonyInformative() const { return _nParsimonyInformative; };
  stat TajimaD() const ;
  
  
  const static unsigned Iterations = 1000;

#ifdef NET_QT
public slots:
#endif
  void setupStats();
  void nestedAmova();
  void amova();


private:  
  
  class DiscreteDistribution
  {
  public:
    DiscreteDistribution(const vector<double> &);
    DiscreteDistribution(const vector<unsigned> &);
    const vector<double> & probabilities() const { return _probs; };
    unsigned sample() const;
    
  private:
    void setupTables();
    
    vector<double> _weights;
    unsigned _nitems;
    vector<double> _probs;
    vector<unsigned> _alias;
    
  };
  
  void condenseSitePats();
  void assessSites();
  void computeDistances();
  unsigned pairwiseDistance(unsigned, unsigned) const;
  void amovaPrivate(const std::vector<std::vector<unsigned> > &, amovatab &) const;
  void nestedAmovaPrivate(const std::vector<std::vector<unsigned > > &, const std::vector<unsigned> &, amovatab &) const;
  void permuteAll(std::vector<std::vector<unsigned> > &, const std::vector<unsigned> &) const; 
  void permuteInGroups(std::vector<std::vector<unsigned> > &, const std::vector<unsigned> &, const std::vector<vector<unsigned> >&) const; 
  
  // these functions are straight out of numerical recipes 6.1 and 6.4
  static double gammaLn(double);
  static double betaI(double, double, double);
  static double betaCF(double, double, double);
  static const unsigned MAXIT = 100;
  static constexpr double EPS = 3.0e-7;
  static constexpr double FPMIN = 1.0e-30;
  const static unsigned LARGE = 1E6;
  void updateProgress(int);

  const vector<Sequence*> *_tmpAlignment;
  const vector<bool> *_tmpMask;
 
  Sequence::CharType _datatype;
  unsigned _nsites;
  unsigned _nseqs;
  unsigned _nSegSites;
  unsigned _nParsimonyInformative;
  std::vector<Sequence> _alignment;
  std::map<Sequence, std::list<Sequence > > _identicalSeqMap;
  std::vector<unsigned> _weights;
  std::map<std::string, unsigned> _seqCounts;
  std::map<std::string, unsigned> _name2idx;
  std::vector<std::string> _idx2name;
  std::vector<unsigned> _orig2condidx;
  std::vector<std::vector<unsigned> > _distances;
  std::vector<std::vector<unsigned> > _traitMat;
  amovatab *_amovaresult;
  std::vector<unsigned> _traitGroups;
  
  static unsigned permuteCount;
#ifdef NET_QT
  QTime _executionTimer;
signals:
  void progressUpdated(int);
#endif
};
