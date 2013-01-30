#ifndef NETWORK_LAYOUT_H_
#define NETWORK_LAYOUT_H_

#include "NetworkModel.h"


#include <limits>
using namespace std;

#include <QObject>
#include <QPointF>
#include <QStack>
#include <QString>
#include <QVector>

// Algorithm taken from Tunkelang 1999

class NetworkLayout : public QObject
{
Q_OBJECT

public:
  NetworkLayout(NetworkModel *, double, double);
  virtual ~NetworkLayout();

  static const double GOODENOUGH = 1E-4;
  unsigned maxIter() const { return _maxiter; };
  void setMaxIter(unsigned maxiter) { _maxiter = maxiter; };
  
  
  unsigned edgeCount() const;
  QPointF edgeStart(unsigned) const;
  QPointF edgeEnd(unsigned) const;
  unsigned edgeStartIdx(unsigned edgeId) const { return _edgeList.at(edgeId).start; };
  unsigned edgeEndIdx(unsigned edgeId) const { return _edgeList.at(edgeId).end; };
  unsigned vertexCount() const;
  const QPointF & vertexCoords(unsigned vertId) const;
  const QPointF & northWest() const { return _northWest; };
  const QPointF & southEast() const { return _southEast; };
  void centreVertices();
  void translateVertices(const QPointF & );
  
  double howGood() { return _howGood; };
  
  static const double MINVERTSIZE;

public slots:
  void optimise();
  
private:
    


  
  class QuadTreeNode
  {
  public:
    QuadTreeNode();
    virtual ~QuadTreeNode();
    //virtual TreeNode* newNodeVirtual(const std::string & = "", double = -1) const { return  new QuadTreeNode(); };
    void resetNode(QStack<QuadTreeNode*> * = 0);
    
    const QPointF & min() const { return _min; };
    const QPointF & max() const { return _max; };
    const QPointF & centroid() const { return _centroid; };
    const double meanRad() const { return _meanRad; };
    void setMin(QPointF minVal) { _min = minVal; };
    void setMax(QPointF maxVal) { _max = maxVal; };
    void adjustCentroid(const QPointF &);
    void adjustMeanRad(double);
    bool isLeaf() const { return _children.isEmpty(); };

    void insertVertex(int, double, const QPointF &, QStack<QuadTreeNode*> * = 0);
    void splitQuad(QStack<QuadTreeNode*> * = 0);
    void setVertex(int, double, const QPointF &);
    int vertexID() const { return _vID; };
    unsigned vertexCount() const { return _vCount; };
    const QPointF & vertexCoords() const { return _vCoords; };
    void resetForce() { _force.setX(0); _force.setY(0); };
    QPointF & force() { return _force; };
    
    void incrementVCount();
    
    QuadTreeNode * nwChild();
    QuadTreeNode * neChild();
    QuadTreeNode * swChild();
    QuadTreeNode * seChild();
    QuadTreeNode * parent();
    
    QuadTreeNode * lookup(const QPointF &);
    bool wellSep(const QuadTreeNode &) const;
    void updateLeafForces();
    
  private:
    
    typedef enum {NoQuad, NorthWest, NorthEast, SouthWest, SouthEast} Quadrant;
    
    /*void setQuad(Quadrant q) { _quad = q; };
    void setIsParent(bool isP)  { _isParent = isP; };*/
    
    QPointF _min;
    QPointF _max;
    QPointF _centroid;
    QPointF _force;
    double _meanRad;
    unsigned _vCount;
    int _vID;
    //const QPointF * _vCoords;
    QPointF _vCoords;
    Quadrant _quad;// = NoQuad;
    //bool _isParent;// = false;
    QVector<QuadTreeNode *> _children;
    QuadTreeNode *_parent;
  };
  
  class QuadTree
  {
  public:
    QuadTree(const QPointF &, const QPointF &);
    virtual ~QuadTree();
    void insertVertex(int, double, const QPointF &);
    void resetTree(const QPointF &, const QPointF &);
    
    const QuadTreeNode * root() const { return _root; };

    const QPointF & min() const { return _min; };
    const QPointF & max() const { return _max; };
    
    QuadTreeNode * lookup(const QPointF &);  
    void computeRepulsion(QuadTreeNode *, QuadTreeNode *, QVector<QPointF> &);
    void updateLeafForces();
    
  protected:

  private:  

    const QPointF & _min;
    const QPointF & _max;
    QuadTreeNode *_root;
    QStack<QuadTreeNode *> _spareNodes;
  };

  typedef struct {unsigned start; unsigned end; unsigned prefLength; } EdgeData;


  // These are different constants because Jiggle uses different values
  static const unsigned MARGIN = 20;
  static const double VERYSMALL = 1E-8;
  static const double FAIRLYSMALL = 1E-6;
  static const double SMALL = 1E-4;

  // Some of these shouldn't be constants
  //static const unsigned SCALEFACTOR = 50;
  static const double CAP;// = numeric_limits<double>::max() / 1000;
  static const double BARNESHUTTHETA = 0.7;
  static const double RESTARTTHRESHOLD = 0.2;
  static const double MAXCOS = 0.5;



  void mapEdges();
  void shuffleVertices();

  void getNegGrad();
  void computeDirection();

  void applySprings();
  void applyCharges();

  void step();//double);

  double l2Norm(const QVector<QPointF> &) const;
  double dot(const QVector<QPointF> &,  const QVector<QPointF> &) const;

  NetworkModel *_model;
  double _width;
  double _height;
  unsigned _maxiter;

  QVector<QPointF> _vertexPositions;
  QVector<EdgeData> _edgeList;
  QVector<QPointF> _negGrad;
  QVector<QPointF> _descentDirection;
  QVector<QPointF> _previousDirection;

  QPointF _southEast;
  QPointF _northWest;

  double _prevGradMag2;
  double _stepSize;
  double _prevStepSize;
  double _howGood;

  QuadTree *_quadtree;

signals:
  void progressChanged(int);
};


#endif
