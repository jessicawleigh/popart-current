/*
 * HapNet.h
 *
 *  Created on: Feb 21, 2012
 *      Author: jleigh
 */

// todo This class should know how to draw itself, above and beyond how a graph would do it (node diameters proportional to number of samples, etc)

#ifndef HAPNET_H_
#define HAPNET_H_

#include <list>
#include <queue>
#include <string>
#include <vector>

#include "Graph.h"
#include "../seqio/Sequence.h"
#include "../seqio/Trait.h"

#ifdef NET_QT
#include <QObject>
#include <QString>
class HapNet : public QObject, public Graph
{
  Q_OBJECT
#else
class HapNet : public Graph
{
#endif
public:
  HapNet(const std::vector <Sequence*> &, const std::vector<bool> & = std::vector<bool>());
  virtual ~HapNet();
#ifndef NET_QT
  void setupGraph();
#endif

  virtual const Sequence * seq(unsigned) const;
  virtual const std::string & seqName(unsigned, bool=false) const;
  virtual const std::string & seqSeq(unsigned, bool=false) const;
  //virtual const std::string & cSeq(unsigned) const;
  //size_t nTraits() const;
  unsigned weight(unsigned) const;

  unsigned freq(unsigned) const;
  const std::vector<std::string> & traitNames() const { return _traitNames; } ;
  void associateTraits(const std::vector<Trait *> &);

  //const unsigned * traits(unsigned) const;
  const std::vector<unsigned> & traits(unsigned) const;
  std::vector<std::string> identicalTaxa(unsigned) const;
#ifdef NET_QT
public slots:
  void setupGraph();
#endif


protected:
  class VertContainer
  {
  public:
    VertContainer(unsigned);//, const Node * = NULL);
    virtual ~VertContainer();

    class Iterator
    {
    public:
      Iterator(VertContainer*, bool = false);
      Iterator & operator++();
      //Iterator operator++(int);
      const Vertex ** operator*() const;
      
      const Vertex** removePair();
      void insertPair(const Vertex **);
      bool operator==(const Iterator &) const;
      bool operator!=(const Iterator &) const;
    protected:
      bool isEnd() const;
    private:
      std::list<const Vertex**>::iterator _pairIt;
      std::list<const Vertex**> _pairs;
      bool _isEnd;
    };



    void addPair(const Vertex *, const Vertex*);
    void removePair(Iterator &position);
    void insertPair(Iterator &position, const Vertex *, const Vertex*);
    //const std::vector<const Node**> & pairs() const;
    unsigned distance() const;
    bool operator<(const VertContainer &) const;
    bool operator>(const VertContainer &) const;
    
    size_t size() const { return _npairs; };
    const Vertex ** at(unsigned) const;

    Iterator begin();
    Iterator end();

  private:

    const unsigned _distance;
    std::list<const Vertex**> _pairs;
    unsigned _npairs;
  };

  class VCPtrComparitor
  {
  public:
    VCPtrComparitor(const bool & reverse = false)
      : _reverse(reverse)  {}
    VCPtrComparitor(const VCPtrComparitor & other)
      : _reverse(other._reverse)  {}

    bool operator() (const VertContainer *lhs, const VertContainer *rhs) const;

  private:
    const bool _reverse;
  };
  
    
  typedef std::priority_queue<VertContainer*, std::vector<VertContainer*>, VCPtrComparitor> VCPQ;
  
  virtual void setDistance(unsigned, unsigned, unsigned);
  virtual unsigned distance(unsigned, unsigned) const;
  virtual size_t nseqs() const;
  size_t nsites(bool =false) const;
  const unsigned * freqs() const;
  bool isGraphSetup() const;
  virtual unsigned pairwiseDistance(const std::string &, const std::string &) const;

  const std::string & condensedSeqSeq(unsigned) const;
  const std::vector<std::string> & condensedSeqs() const;
  const std::vector<unsigned> & weights() const;
  void updateProgress(int);
  
private:
  size_t _nseqs; // unique sequences!
  size_t _nsites;
  size_t _nCsites;
  //size_t _nTraits;
  Sequence::CharType _datatype;
  std::vector<Sequence *>  _origSeqs; 
  std::vector<std::string> _condensedSeqs;
  unsigned *_orig2cond;
  std::vector<std::vector<unsigned> > _cond2orig;
  std::vector<unsigned> _weights;
  unsigned *_oPos2CPos;
  unsigned *_freqs;
  unsigned *_distances;
  //unsigned **_traits;
  std::vector<unsigned> *_traits;
  std::vector<std::string> _traitNames;
  bool _isGraphSetup;
  
  const static std::vector<unsigned> _emptyTraits;

  virtual void computeGraph() = 0;
  void condenseSeqs();
  void condenseSitePats();
  virtual void computeDistances();

  friend std::ostream &operator<<(std::ostream &, const HapNet &);
#ifdef NET_QT
  QString _errorMsg;
signals:
  void progressUpdated(int);
  void caughtException(const QString &);
  void traitsChanged();
#endif
};

#endif /* HAPNET_H_ */
