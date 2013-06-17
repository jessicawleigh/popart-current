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
    double prob;
  } anovatab;
  
  Statistics(const std::vector<Sequence*> & = std::vector<Sequence*>(), const std::vector<bool> & = std::vector<bool>(), Sequence::CharType = Sequence::DNAType);
  const std::map<Sequence, std::list<Sequence > > & mapIdenticalSeqs() { return _identicalSeqMap; };
  void setFreqsFromTraits(const std::vector<Trait *> &);
  double nucleotideDiversity() const;
  unsigned nSegSites() const { return _nSegSites; };
  unsigned nParsimonyInformative() const { return _nParsimonyInformative; };
  stat TajimaD() const ;
  anovatab amova() const;

#ifdef NET_QT
public slots:
#endif
  void setupStats();


private:  
  
  void condenseSitePats();
  void assessSites();
  void computeDistances();
  unsigned pairwiseDistance(unsigned, unsigned) const;
  
  // these functions are straight out of numerical recipes 6.1 and 6.4
  static double gammaLn(double);
  static double betaI(double, double, double);
  static double betaCF(double, double, double);
  const static unsigned MAXIT = 100;
  const static double EPS = 3.0e-7;
  const static double FPMIN = 1.0e-30;
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
#ifdef NET_QT
  QTime _executionTimer;
signals:
  void progressUpdated(int);
#endif
};
