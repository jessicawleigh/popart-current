#include "NetworkLayout.h"
#include "HapAppError.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>

#include <QApplication>
#include <QThread>
//#include <QTime>


const double NetworkLayout::CAP = numeric_limits<double>::max() / 1000;
const double NetworkLayout::MINVERTSIZE = 4.0 / 9;


NetworkLayout::NetworkLayout(NetworkModel *model, double width, double height, double depth)
  : _southEast(), _northWest()//, _bounds({QVector3D(-1,-1,-1),QVector3D(-1,-1,-1)})
{
  _model = model;
  _width = width;
  _height = height;
  _depth = depth;
  
  if (_depth == 0)
    _is3Dlayout = false;
  else
    _is3Dlayout = true;
  
  _stepSize = 0.1;
  _prevStepSize = 0;
  _howGood = numeric_limits<double>::max();
  
  mapEdges();
  shuffleVertices();
  _quadtree = 0;
  
}

NetworkLayout::~NetworkLayout()
{
  // TODO fill this in

  if (_quadtree)
    delete _quadtree;
}

void NetworkLayout::mapEdges()
{
  for (unsigned i = 0; i < _model->rowCount(); i++)
  {
    QModelIndex parent = _model->index(i, 0);   
    for (unsigned j = 0; j < _model->rowCount(parent); j++)
    {
      QModelIndex child = _model->index(j, 0, parent);
      EdgeData ed;
      ed.start = i;
      ed.end = child.data(NetworkItem::EdgeEndRole).toInt();
      ed.prefLength = NetworkItem::EDGELENGTH * child.data(NetworkItem::SizeRole).toDouble();//SCALEFACTOR * _model->data(child, NetworkItem::SizeRole).toInt();
      _edgeList.push_back(ed);
            
    }
  }
}

void NetworkLayout::shuffleVertices()
{
  
  // TODO check that vertex ID matches _vertexPositions index
  

  for (int i = 0; i < _model->rowCount(); i++)
  {
    double x = qrand() % (int)_width;
    double y = qrand() % (int)_height;
    double z = _is3Dlayout ? qrand() % (int)_depth : 0;

    
    //double vrad = 0.5 * NetworkItem::VERTRAD * sqrt(_model->index(0, 0).data(NetworkItem::SizeRole).toDouble());
    //QVector3D vmin = QVector3D(x, y, z) - vrad;
    //QVector3D vmax = QVector3D(x, y, z) + vrad;
    
    /*if (i == 0)
    {
      _bounds.min = vmin;
      _bounds.max = vmax;
    }
    
    else
    {
      _bounds.min.setX(qMin(_bounds.min.x(), vmin.x()));
      _bounds.min.setY(qMin(_bounds.min.y(), vmin.y()));
      _bounds.min.setZ(qMin(_bounds.min.z(), vmin.z()));

      _bounds.max.setX(qMax(_bounds.max.x(), vmax.x()));
      _bounds.max.setY(qMax(_bounds.max.y(), vmax.y()));
      _bounds.max.setZ(qMax(_bounds.max.z(), vmax.z()));
      
    }*/

    //_vertexPositions.push_back(QPointF(15 * (i + 10 + pow(-1., i)), 15 * (i + 10 + pow(-1., i + 1))));
    //x = i * 20; y = i * 20; z =_is3Dlayout ? i * 20 : 0;
    _vertexPositions.push_back(QVector3D(x, y, z));

  }  
  
  centreVertices();
}

void NetworkLayout::zeroVertices()
{
  for (unsigned i = 0; i < _vertexPositions.size(); i++)
  {
    
      _vertexPositions[i].setX(0);
      _vertexPositions[i].setY(0);
      _vertexPositions[i].setZ(0);
  }
  
  _northWest.setX(0);
  _northWest.setY(0);
  _northWest.setZ(0);
  _southEast.setX(0);
  _southEast.setY(0);
  _southEast.setZ(0);
}

void NetworkLayout::centreVertices()
{
  //QPointF northWest(_vertexPositions.at(0));
  //QPointF southEast(_vertexPositions.at(0));
  
  _northWest.setX(_vertexPositions.at(0).x());
  _northWest.setY(_vertexPositions.at(0).y());
  _southEast.setX(_vertexPositions.at(0).x());
  _southEast.setY(_vertexPositions.at(0).y());
  
  if (_is3Dlayout)
  {
    _northWest.setZ(_vertexPositions.at(0).z());
    _southEast.setZ(_vertexPositions.at(0).z());
  }
  
  for (unsigned i = 1; i < _vertexPositions.size(); i++)
  {
    
      double x = _vertexPositions.at(i).x();
      double y = _vertexPositions.at(i).y();
      double z = _vertexPositions.at(i).z();
    
      if (x < _northWest.x())  _northWest.setX(x);
      else if (x > _southEast.x())  _southEast.setX(x);
      
      if (y < _northWest.y())  _northWest.setY(y);
      else if (y > _southEast.y())  _southEast.setY(y);
      
      if (_is3Dlayout)
      {
        if (z < _northWest.z())  _northWest.setZ(z);
        else if (z > _southEast.z()) _southEast.setZ(z);
      }
  }
  
  QVector3D mid = _northWest + _southEast / 2;
  mid.setX(_width/2 - mid.x());
  mid.setY(_height/2 - mid.y());
  
  if (_is3Dlayout)
    mid.setZ(_depth/2 - mid.z());
  
  else
    mid.setZ(0);
  
  for (unsigned i = 0; i < _vertexPositions.size(); i++)
    _vertexPositions[i] += mid;
 
  
  _northWest += mid;  
  _southEast += mid;
}

void NetworkLayout::translateVertices(const QPointF & translation2D)
{
  QVector3D translation(translation2D);
  
  for (unsigned i = 0; i < _vertexPositions.size(); i++)
  {
    _vertexPositions[i] += translation;
  }
  
  _southEast += translation;
  _northWest += translation;
}

void NetworkLayout::optimise()
{  

  //QTime timer;
  //cout << "Performing up to " << _maxiter << " iterations." << endl;
  //timer.start();
  //long lasttime = timer.elapsed();
  for (unsigned i = 0; i < _maxiter; i++)
  {
    //if (i % 10 == 0)
    //{
      //cout << "Time for iteration " << i << ": " << (timer.elapsed() - lasttime) << endl;
      //cerr << (timer.elapsed() - lasttime) << endl;
      //lasttime = timer.elapsed();
    //}

    // TODO eventually
    // implement penalties
    

    if (_negGrad.empty())
    {
      _negGrad.resize(_vertexPositions.size());
      //penaltyVector = new double [n] [d];
      getNegGrad();
    }
    
    computeDirection();
  
    // Line search
    
    double magDescDir = l2Norm(_descentDirection);
    if (magDescDir < SMALL)
    {
      _howGood = 0;

      // return _howGood;
    }

    else
    {
      step();//_stepSize);
      //_prevStepSize = _stepSize;
      getNegGrad();
      double magHi = l2Norm(_negGrad);

      // cos of angle of step
      double cosGamma = dot(_negGrad, _descentDirection) / (magDescDir * magHi);

      double lo = 0, hi = numeric_limits<double>::max();


      while (((cosGamma < 0) || (cosGamma > MAXCOS)) && ((hi - lo) > VERYSMALL))
      {

        if (cosGamma < 0)
        {
          hi = _stepSize;
          _stepSize = (lo + hi) / 2;
        }

        else  if (hi < numeric_limits<double>::max())
        {
          lo = _stepSize;
          _stepSize = (lo + hi) / 2;
        }

        else
        {
          lo = _stepSize;
          _stepSize *= 2;
        }

        step();//stepSize - prevStepSize);
        //prevStepSize = stepSize;
        getNegGrad();

        cosGamma = dot(_negGrad, _descentDirection) / (magDescDir * l2Norm(_negGrad));
      }

    
      _howGood = l2Norm(_negGrad);
      emit progressChanged(100. * (i + 1) / _maxiter);
    }
    //return _howGood;
    if (_howGood <= GOODENOUGH)
    {
      //cout << "stopping after " << (i + 1) << " iterations." << endl;
      break;
    }
  }

  emit progressChanged(100);
  //cout << "Time elapsed: " << timer.elapsed() << endl;
  if (thread() != QApplication::instance()->thread())
    thread()->exit();
}

void NetworkLayout::getNegGrad()
{
  
  for (unsigned i = 0; i < _negGrad.size(); i++)
  {  
    //_negGrad[i].rx() = _negGrad[i].ry() = 0;
    _negGrad[i].setX(0);
    _negGrad[i].setY(0);
    _negGrad[i].setZ(0);
  }
  
  applySprings();   
  applyCharges();
}

// Quadratic spring law from Tunkelang
void NetworkLayout::applySprings()
{
  
  QVector<EdgeData>::const_iterator edIt = _edgeList.constBegin();
  
  //unsigned eidx = 0;
  while (edIt != _edgeList.constEnd())
  {
    const QVector3D & startPoint = _vertexPositions.at(edIt->start);
    const QVector3D & endPoint = _vertexPositions.at(edIt->end);
    
    double dx = endPoint.x() - startPoint.x();
    double dy = endPoint.y() - startPoint.y();
    double dz = endPoint.z() - startPoint.z();
    double length = sqrt(dx * dx + dy * dy + dz * dz);
    
    // Comes from the weird radius function of Cell.java in Jiggle, might be for square vertices. Maybe replace with just:
    // 0.5 * (NetworkItem::VERTRAD * sqrt(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt()) + NetworkItem::VERTRAD * sqrt(model()->index(j, 0).data(NetworkItem::SizeRole).toUInt()));
    //double t = abs(NetworkItem::VERTRAD/dx);
    //t = min(t, abs(NetworkItem::VERTRAD/dy));
    //double radsum = 2 * sqrt(pow(t * dx, 2) + pow(t * dy, 2));
    double sizeS = _model->index(edIt->start, 0).data(NetworkItem::SizeRole).toUInt();
    sizeS = max(sizeS, MINVERTSIZE);
    double sizeE = _model->index(edIt->end, 0).data(NetworkItem::SizeRole).toUInt();
    sizeE = max(sizeE, MINVERTSIZE);
    double radsum = 0.5 * NetworkItem::VERTRAD * (sqrt(sizeS) + sqrt(sizeE));
    
    double prefLength = max((double)(edIt->prefLength), radsum);
    double attraction = (length - radsum)/prefLength;
    
    // Do I need this?
    
    attraction = min(attraction, CAP / length);
    
    
    QVector3D force(dx, dy, dz);
    force *= attraction;
    
    if (! _is3Dlayout)
      force.setZ(0);
    
    _negGrad[edIt->start] += force * NetworkItem::VERTWEIGHT;
    _negGrad[edIt->end] -= force * NetworkItem::VERTWEIGHT;
    
    // X component of force
   /* double force = attraction * dx;
    //_negGrad[edIt->start].rx() += NetworkItem::VERTWEIGHT * force;
   // _negGrad[edIt->end].rx() -= NetworkItem::VERTWEIGHT * force;
    _negGrad[edIt->start].setX(_negGrad[edIt->start].x() + NetworkItem::VERTWEIGHT * force);
    _negGrad[edIt->end].setX(_negGrad[edIt->end].x() - NetworkItem::VERTWEIGHT * force);
    
    // Y component
    force = attraction * dy;
    //_negGrad[edIt->start].ry() += NetworkItem::VERTWEIGHT * force;
    //_negGrad[edIt->end].ry() -= NetworkItem::VERTWEIGHT * force;
    _negGrad[edIt->start].setY(_negGrad[edIt->start].y() + NetworkItem::VERTWEIGHT * force);
    _negGrad[edIt->end].setY(_negGrad[edIt->end].y() -NetworkItem::VERTWEIGHT * force);
    
    // Z component
    force = attraction * dz;
    _negGrad[edIt->start].setZ(_negGrad[edIt->start].z() + NetworkItem::VERTWEIGHT * force);
    _negGrad[edIt->end].setZ(_negGrad[edIt->end].z() -NetworkItem::VERTWEIGHT * force);*/
    
    
    ++edIt;
  }
}

void NetworkLayout::applyCharges()
{
  /*QPointF lowerLeft(0, 0);
  QPointF upperRight(_width, _height);*/
  
  // TODO test if 3D code works with useBH = true
  bool useBH = false;
  if (useBH)
  {
    QVector3D lowerLeft(_vertexPositions.at(0));
    QVector3D upperRight(_vertexPositions.at(0));

    for (unsigned i = 1 ; i < _vertexPositions.size(); i++)
    {
      double x = _vertexPositions.at(i).x();
      double y = _vertexPositions.at(i).y();
      double z = _vertexPositions.at(i).z();

      if (x < lowerLeft.x())  lowerLeft.setX(x);
      else if (x > upperRight.x())  upperRight.setX(x);

      if (y < lowerLeft.y())  lowerLeft.setY(y);
      else if (y > upperRight.y())  upperRight.setY(y);

      if (_is3Dlayout)
      {
        if (z < lowerLeft.z())  lowerLeft.setZ(z);
        else if (z > upperRight.z())  upperRight.setZ(z);
      }
    }

    if (_quadtree)
      _quadtree->resetTree(lowerLeft.toPointF(), upperRight.toPointF());
    else
      _quadtree = new QuadTree(lowerLeft.toPointF(), upperRight.toPointF());
    //QuadTree qt(lowerLeft, upperRight);
    //QPointF &coordsI, &coordsJ;

    for (unsigned i = 0; i < _vertexPositions.size(); i++)
    {
      double size = _model->index(i, 0).data(NetworkItem::SizeRole).toUInt();
      size = max(size, MINVERTSIZE);
      double rad = 0.5 * NetworkItem::VERTRAD * sqrt(size);
      _quadtree->insertVertex(i, rad, _vertexPositions.at(i).toPointF());
    }
    
    
    
    QVector<QuadTreeNode *> leavesByID;
    
    QVector<QPointF> negGrad2D;
    foreach (QVector3D point3D, _negGrad)
      negGrad2D.push_back(point3D.toPointF());
    
    for (unsigned i = 0; i < _vertexPositions.size(); i++)
    {
      QuadTreeNode *leaf = _quadtree->lookup(_vertexPositions.at(i).toPointF());
      leavesByID.push_back(leaf);

      QuadTreeNode *p = leaf;
      QuadTreeNode *parent;// = p->parent();

      
      // TODO need to apply this in 3D quadrants... so 8 children (front and back): nwf, nwb, nef, neb, swf, swb, sef, seb
      // or maybe just use 0, 1, for each dimension: parent->child(0,0,0) is nwchild?
      while (p != _quadtree->root())
      {
        parent = p->parent();
        QuadTreeNode *sibling = parent->nwChild();
        if (sibling != p && sibling->vertexCount() > 0)  _quadtree->computeRepulsion(leaf, sibling, negGrad2D);

        sibling = parent->neChild();
        if (sibling != p && sibling->vertexCount() > 0)  _quadtree->computeRepulsion(leaf, sibling, negGrad2D);
  
        sibling = parent->swChild();
        if (sibling != p && sibling->vertexCount() > 0)  _quadtree->computeRepulsion(leaf, sibling, negGrad2D);

        sibling = parent->seChild();
        if (sibling != p && sibling->vertexCount() > 0)  _quadtree->computeRepulsion(leaf, sibling, negGrad2D);

        p = parent;
      }
    }

    _quadtree->updateLeafForces();


    for (unsigned i = 0; i < leavesByID.size(); i++)
      _negGrad[i] += QVector3D(leavesByID.at(i)->force());
  }
  
  else
  {

    for (unsigned i = 0; i < _vertexPositions.size(); i++)
    {
      const QVector3D &coordsI = _vertexPositions.at(i);
      double sizeI = _model->index(i, 0).data(NetworkItem::SizeRole).toUInt();
      sizeI = max(sizeI, MINVERTSIZE);
      
      for (unsigned j = i + 1; j < _vertexPositions.size(); j++)
      {
        const QVector3D &coordsJ = _vertexPositions.at(j);
        double dx = coordsI.x() - coordsJ.x();
        double dy = coordsI.y() - coordsJ.y();
        double dz = coordsI.z() - coordsJ.z();

        // Sum of radii
        //double t = abs(NetworkItem::VERTRAD/dx);
        //t = min(t, abs(NetworkItem::VERTRAD/dy));
        //double radsum = 2 * sqrt(pow(t * dx, 2) + pow(t * dy, 2));


        double sizeJ = _model->index(j, 0).data(NetworkItem::SizeRole).toUInt();
        sizeJ = max(sizeJ, MINVERTSIZE);
        double radsum = 0.5 * NetworkItem::VERTRAD * (sqrt(sizeI) + sqrt(sizeJ));

        // Distance squared
        double dist2 = pow(dx, 2) + pow(dy, 2) + pow(dz, 2);
        double dist = sqrt(dist2);

        // minimum preferred distance
        double k = NetworkItem::EDGELENGTH + radsum; // EDGELENGTH is preferred edge length (for one-step edges)
        double k2 = pow(k, 2);

        double repulsion;

        if (dist2 < k2)  repulsion = k2 / dist2;
        else  repulsion = k2 * k / (dist2 * dist);
        //double repulsion = (dist2 < (k * k) ?  k * k / dist2 : k * k * k / (dist2 * dist));

        repulsion = min(repulsion, CAP / dist);
        
        QVector3D force(dx, dy, dz);
        force *= repulsion;
        
        if (! _is3Dlayout)
          force.setZ(0);
        
        _negGrad[i] += force;
        _negGrad[j] -= force;

        /*
        // X component of force
        double force = repulsion * dx;
        _negGrad[i].rx() += force * NetworkItem::VERTWEIGHT;
        _negGrad[j].rx() -= force * NetworkItem::VERTWEIGHT;

        // Y component of force
        force = repulsion * dy;
        _negGrad[i].ry() += force * NetworkItem::VERTWEIGHT;
        _negGrad[j].ry() -= force * NetworkItem::VERTWEIGHT;
        */
        
      }
    }
  }
}

void NetworkLayout::computeDirection()
{
  int n = _vertexPositions.size();
  
  // magnitude of current gradient
  double gradMag2 = 0;
  
  // First pass, just use steepest descent
  if (_descentDirection.empty())
  {    
    for (unsigned i = 0; i < n; i++)
    {
      
      _descentDirection.push_back(_negGrad.at(i));
      gradMag2 += pow(_negGrad.at(i).x(), 2) + pow(_negGrad.at(i).y(), 2) + pow(_negGrad.at(i).z(), 2);
    }
  }
  
  else
  {

    for (unsigned i = 0; i < n; i++)
    {
      gradMag2 += pow(_negGrad.at(i).x(), 2) + pow(_negGrad.at(i).y(), 2) + pow(_negGrad.at(i).z(), 2);
      
    }
    
    if (gradMag2 < FAIRLYSMALL)
    {
      for (unsigned i = 0; i < n; i++)
      {
        _previousDirection[i].setX(0); _previousDirection[i].setY(0); _previousDirection[i].setZ(0);
        _descentDirection[i].setX(0); _descentDirection[i].setY(0); _descentDirection[i].setZ(0);
      }
      
      return;
    }
    
    double ratio = gradMag2 / _prevGradMag2;
    
    double dotProd = 0;
    double magDescDir2 = 0;
    
    
    // Check that search direction dot gradient is actually a descent 
    for (unsigned i = 0; i < n; i++)
    {
      _descentDirection[i].setX(_negGrad.at(i).x() + ratio * _previousDirection[i].x());
      _descentDirection[i].setY(_negGrad.at(i).y() + ratio * _previousDirection[i].y());
      
      if (_is3Dlayout)
        _descentDirection[i].setZ(_negGrad.at(i).z() + ratio * _previousDirection[i].z());
      
      dotProd += _descentDirection.at(i).x() * _negGrad.at(i).x() + _descentDirection.at(i).y() * _negGrad.at(i).y() + _descentDirection.at(i).z() * _negGrad.at(i).z();
      
      magDescDir2 += _descentDirection.at(i).x() * _descentDirection.at(i).x() + _descentDirection.at(i).y() * _descentDirection.at(i).y() + _descentDirection.at(i).z() * _descentDirection.at(i).z();
      
    }
    
    // If dot product is not negative or is just too small, restart
    // Note that this case will never be hit on the first pass through the function
    if ( dotProd / sqrt(gradMag2 * magDescDir2) < RESTARTTHRESHOLD) 
    {
      _descentDirection.clear();
      computeDirection();
      return;
    }
    
  }
  
  _prevGradMag2 = gradMag2;
  
  if (_previousDirection.empty())
  {
    QVector<QVector3D>::const_iterator dirIt = _descentDirection.constBegin();
    
    while (dirIt != _descentDirection.constEnd())
    {
      _previousDirection.push_back(QVector3D(*dirIt));
      ++dirIt;
    }
  }
  
  else
  {
    for (unsigned i = 0; i < n; i++)
      _previousDirection[i] = _descentDirection.at(i);
    //{
      //_previousDirection[i].setX(_descentDirection.at(i).x());
      //_previousDirection[i].setY(_descentDirection.at(i).y());
    //}
  }
}

void NetworkLayout::step()//double size)
{
  double s = _stepSize - _prevStepSize;
  for (unsigned i = 0; i < _vertexPositions.size(); i++)
  {
    //_vertexPositions[i].rx() += _descentDirection.at(i).x() * s;
    //_vertexPositions[i].ry() += _descentDirection.at(i).y() * s;
    _vertexPositions[i] += _descentDirection.at(i) * s;
    
  }
  _prevStepSize = _stepSize;
    //_vertexPositions[i] += _descentDirection.at(i) * size;
}

double NetworkLayout::l2Norm(const QVector<QVector3D> & points) const
{  
  return sqrt(dot(points, points));
}

double NetworkLayout::dot(const QVector<QVector3D> & pointsA, const QVector<QVector3D> & pointsB) const
{
  if (pointsA.size() != pointsB.size())  throw HapAppError("Vectors are different sizes, can't compute dot product.");
  
  double dotProd = 0;
  
  for (unsigned i = 0; i < pointsA.size(); i++)
    dotProd += pointsA.at(i).x() * pointsB.at(i).x() + pointsA.at(i).y() * pointsB.at(i).y() + pointsA.at(i).z() * pointsB.at(i).z();
  
  
  return dotProd;
}

unsigned NetworkLayout::edgeCount() const
{
  return _edgeList.size();
}

const QVector3D & NetworkLayout::edgeStart3D(unsigned edgeId) const
{
  if (edgeId >= _edgeList.size())  
  {
    throw HapAppError("Edge index out of range.");
  }
  
  unsigned idx = _edgeList.at(edgeId).start;
  //double vrad = 0.5 * NetworkItem::VERTRAD * sqrt(_model->index(idx, 0).data(NetworkItem::SizeRole).toDouble());
  //vrad = max(vrad, NetworkItem::VERTRAD / 3.0);

  //QVector3D startPoint = _vertexPositions.at(idx) + QVector3D(vrad, vrad, vrad);
  //startPoint.rx() += vrad;
  //startPoint.ry() += vrad;
    
  //return startPoint;//.toPointF();
  return _vertexPositions.at(idx);
}

QPointF NetworkLayout::edgeStart(unsigned edgeId) const
{
  return edgeStart3D(edgeId).toPointF();
}

const QVector3D & NetworkLayout::edgeEnd3D(unsigned edgeId) const
{
  if (edgeId >= _edgeList.size())
  {
    throw HapAppError("Edge index out of range.");
  }
  
    
  unsigned idx = _edgeList.at(edgeId).end;
  //double vrad = 0.5 * NetworkItem::VERTRAD * sqrt(_model->index(idx, 0).data(NetworkItem::SizeRole).toDouble());
  //vrad = max(vrad, NetworkItem::VERTRAD / 3);
  
  //QVector3D endPoint = _vertexPositions.at(idx) + QVector3D(vrad, vrad, vrad);

  //endPoint.rx() += vrad;
  //endPoint.ry() += vrad;
    
  //return endPoint;//.toPointF();
  return _vertexPositions.at(idx);

}

QPointF NetworkLayout::edgeEnd(unsigned edgeId) const
{
  return edgeEnd3D(edgeId).toPointF();
}


unsigned NetworkLayout::vertexCount() const
{
  return _vertexPositions.size();
}

const QVector3D & NetworkLayout::vertexCoords3D(unsigned vertId) const
{
  if (vertId >= _vertexPositions.size())
  {
    throw HapAppError("Vertex index out of range");
  }
  
  return _vertexPositions.at(vertId);//.toPointF();
}

QPointF NetworkLayout::vertexCoords(unsigned vertId) const
{
  return vertexCoords3D(vertId).toPointF();
}

/*double NetworkLayout::vertexSize(unsigned vertId)
{
  // TODO something useful here
  return 15;
}*/


/**
 * NetworkLayout::QuadTreeNode starts here
 */

NetworkLayout::QuadTreeNode::QuadTreeNode()
{
  resetNode();
}

NetworkLayout::QuadTreeNode::~QuadTreeNode()
{
  _children.clear();
}

void NetworkLayout::QuadTreeNode::resetNode(QStack<QuadTreeNode *> *spareNodes)
{
  
  if (! _children.isEmpty())
  {
    for (unsigned i = 0; i < _children.size(); i++)
      _children.at(i)->resetNode(spareNodes);
    
  _children.clear();
  }
  
  _min.rx() = _min.ry() = _max.rx() = _max.ry() = 0;
  _centroid.rx() = _centroid.ry() = 0;
  _force.rx() = _force.ry() = 0;
  _meanRad = 0;
  _vCount = 0;
  _vID = -1;
  _vCoords.rx() = _vCoords.ry() = 0;
  //if (_vCoords)  delete _vCoords;
  //_vCoords = 0;
  _quad = NoQuad;
  // don't add root to the stack
  if (spareNodes && _parent)
    spareNodes->push_back(this);
  _parent = 0;
}

void NetworkLayout::QuadTreeNode::insertVertex(int vID, double rad, const QPointF & coords, QStack<NetworkLayout::QuadTreeNode *> *spareNodes)
{
  if (isLeaf() && _vCount == 0)  setVertex(vID, rad, coords);
    
  else
  {
    if (isLeaf())  splitQuad(spareNodes);
    adjustCentroid(coords);
    adjustMeanRad(rad);
    incrementVCount();
    
    QPointF mid = (min() + max()) / 2;
    if (coords.x() < mid.x())
    {
      if (coords.y() < mid.y())
        nwChild()->insertVertex(vID, rad, coords, spareNodes);
      else swChild()->insertVertex(vID, rad, coords, spareNodes);
    }
    
    else
    {
      if (coords.y() < mid.y())
        neChild()->insertVertex(vID, rad, coords, spareNodes);
      else seChild()->insertVertex(vID, rad, coords, spareNodes);
    }
    
  }
}

void NetworkLayout::QuadTreeNode::splitQuad(QStack<NetworkLayout::QuadTreeNode*> * spareNodes)
{
  QPointF mid = (max() + min()) / 2;
  
  QuadTreeNode * child;
  
  if (spareNodes && ! spareNodes->isEmpty())
    child = spareNodes->pop();
  else 
    child = new QuadTreeNode();
  child->setMin(QPointF(min().x(), min().y()));
  child->setMax(mid);
  child->_quad = NorthWest;
  child->_parent = this;
  _children.push_back(child);
  
  if (spareNodes && ! spareNodes->isEmpty())
    child = spareNodes->pop();
  else 
    child = new QuadTreeNode();
  child->setMin(QPointF(mid.x(), min().y()));
  child->setMax(QPointF(max().x(), mid.y()));
  child->_quad = NorthEast;
  child->_parent = this;
  _children.push_back(child);
  
  if (spareNodes && ! spareNodes->isEmpty())
    child = spareNodes->pop();
  else 
    child = new QuadTreeNode();
  child->setMin(QPointF(min().x(), mid.y()));
  child->setMax(QPointF(mid.x(), max().y()));
  child->_quad = SouthWest;
  child->_parent = this;
  _children.push_back(child);
  
  if (spareNodes && ! spareNodes->isEmpty())
    child = spareNodes->pop();
  else 
    child = new QuadTreeNode();
  child->setMin(QPointF(mid.x(), mid.y()));
  child->setMax(QPointF(max().x(), max().y()));
  child->_quad = SouthEast;
  child->_parent = this;
  _children.push_back(child);
  
  if (_vCoords.x() < mid.x())
  {
    if (_vCoords.y() < mid.y())  nwChild()->setVertex(_vID, _meanRad, _vCoords);
    else  swChild()->setVertex(_vID, _meanRad, _vCoords);
  }
  
  else
  {
    if (_vCoords.y() < mid.y())  neChild()->setVertex(_vID, _meanRad, _vCoords);
    else seChild()->setVertex(_vID, _meanRad, _vCoords);
  }
  
  //_isParent = true;
  _vID = -1;
  _vCoords.rx() = _vCoords.ry() = 0;
}


// TODO more error checking to make sure tese are the right children?
NetworkLayout::QuadTreeNode * NetworkLayout::QuadTreeNode::nwChild()
{
  if (isLeaf())  return 0;
  
  QuadTreeNode *child = _children.at(0);
  
  if (child->_quad == NorthWest)  return child;

  return 0;
}

NetworkLayout::QuadTreeNode * NetworkLayout::QuadTreeNode::neChild()
{
  if (isLeaf())  return 0;
  
  QuadTreeNode *child = _children.at(1);
  
  if (child->_quad == NorthEast)  return child;
  return 0;
}

NetworkLayout::QuadTreeNode * NetworkLayout::QuadTreeNode::swChild()
{
  if (isLeaf())  return 0;
  
  QuadTreeNode *child = _children.at(2);
  
  if (child->_quad == SouthWest)  return child;
  
  return 0;
}

NetworkLayout::QuadTreeNode * NetworkLayout::QuadTreeNode::seChild()
{
  if (isLeaf())  return 0;
  
  QuadTreeNode *child = _children.at(3);
  
  if (child->_quad == SouthEast)  return child;

  return 0;
}

NetworkLayout::QuadTreeNode * NetworkLayout::QuadTreeNode::parent()
{
  return _parent;  
}


void NetworkLayout::QuadTreeNode::adjustCentroid (const QPointF & newCoords)
{
  _centroid *= _vCount;
  _centroid += newCoords;
  _centroid /= (_vCount + 1);
}

void NetworkLayout::QuadTreeNode::adjustMeanRad(double newRad)
{
  _meanRad *= _vCount;
  _meanRad += newRad;
  _meanRad /= (_vCount + 1);
}

void NetworkLayout::QuadTreeNode::incrementVCount()
{
   _vCount++;
}

void NetworkLayout::QuadTreeNode::setVertex(int vID, double rad, const QPointF & coords)
{
  

  if (vID < 0)  throw HapAppError("Invalid vertex ID." );
  if (! isLeaf())  throw HapAppError("Can't insert a vertex at an internal node.");
  
  _vID = vID;
  _meanRad = rad;
  _vCoords.setX(coords.x());
  _vCoords.setY(coords.y());
  _vCount = 1;
  _centroid.setX(coords.x());
  _centroid.setY(coords.y());
}

NetworkLayout::QuadTreeNode * NetworkLayout::QuadTreeNode::lookup(const QPointF & coords)
{
  if (_vCount == 1 && coords == vertexCoords())  return this;
  
  if (isLeaf())  throw HapAppError("Reached a leaf without finding vID!");

  
  QPointF mid = (min() + max()) / 2;
  
  if (coords.x() < mid.x())
  {
    if (coords.y() < mid.y())
      return nwChild()->lookup(coords);
    else return swChild()->lookup(coords);
  }
  
  else
  {
    if (coords.y() < mid.y())
      return neChild()->lookup(coords);
    else return seChild()->lookup(coords);
  }
  
  return 0;
  
}

bool NetworkLayout::QuadTreeNode::wellSep(const NetworkLayout::QuadTreeNode &other) const
{
  // TODO figure this out... possibly just because this should never be called with a leaf as other?
  if (other.isLeaf())  return true;
  
  const QPointF & lo = other.min();
  const QPointF & hi = other.max();
  
  
  double wdth = hi.x() - lo.x();
  double hght = hi.y() - lo.y();
  
  double dx = other.centroid().x() - centroid().x();
  double dy = other.centroid().y() - centroid().y();
  
  double dist = sqrt(dx * dx + dy * dy);
  
  return (std::min(wdth, hght) / dist) < BARNESHUTTHETA;
}

void NetworkLayout::QuadTreeNode::updateLeafForces()
{
  QuadTreeNode *child = nwChild();
  child->force() += force();
  if ( ! child->isLeaf())  child->updateLeafForces();
  
  child = neChild();
  child->force() += force();  
  if ( ! child->isLeaf())  child->updateLeafForces();
  
  child = swChild();
  child->force() += force();  
  if ( ! child->isLeaf())  child->updateLeafForces();
  
  child = seChild();
  child->force() += force();  
  if ( ! child->isLeaf())  child->updateLeafForces();
  
  resetForce();
}


/**
 * NetworkLayout::QuadTree starts here
 */

NetworkLayout::QuadTree::QuadTree(const QPointF &minVal, const QPointF &maxVal)
  : _min(minVal), _max(maxVal)
{

  //if (_spareNodes.isEmpty())
  _root = new QuadTreeNode();
  //else
  //  _root = _spareNodes.pop();

  _root->setMin(minVal);
  _root->setMax(maxVal);
}

NetworkLayout::QuadTree::~QuadTree()
{
  //resetTree();
  _root->resetNode(&_spareNodes);
  for (unsigned i = 0; i < _spareNodes.size(); i++)
    delete _spareNodes.at(i);
  
  _spareNodes.clear();
}

void NetworkLayout::QuadTree::resetTree(const QPointF &minVal, const QPointF &maxVal)
{
  _root->resetNode(&_spareNodes);
  _root->setMin(minVal);
  _root->setMax(maxVal);
}


void NetworkLayout::QuadTree::insertVertex(int vID, double rad, const QPointF & coords)
{
  
  _root->insertVertex(vID, rad, coords, &_spareNodes);
  
}
// TODO recycle nodes by storing in sparenodes
// write destructor to free all nodes by preorder traversal
// maybe reuse recycler? Maybe not.

NetworkLayout::QuadTreeNode * NetworkLayout::QuadTree::lookup(const QPointF &coords)
{  
  return _root->lookup(coords);
}

void NetworkLayout::QuadTree::computeRepulsion(QuadTreeNode *ref, QuadTreeNode *other, QVector<QPointF> &negGrad)
{
  if (ref->vertexID() < 0)  throw HapAppError("Can't calculate force in reference to an internal node!");

  // if other is a leaf, its centroid will be it's vertex's coordinates
  // if the two are well-separated, the centroid is all we'll use
  if (other->isLeaf() || ref->wellSep(*other))
  {
  
    double dx = ref->centroid().x() - other->centroid().x();
    double dy = ref->centroid().y() - other->centroid().y();
    
    // Distance squared
    double dist2 = dx * dx + dy * dy;
    double dist = sqrt(dist2);
    
    // minimum preferred distance
    
    /*double t = abs(NetworkItem::VERTRAD/dx);
    t = std::min(t, abs(NetworkItem::VERTRAD/dy));
    double radsum = 2 * sqrt(pow(t * dx, 2) + pow(t * dy, 2)); */
    
    double radsum = other->meanRad() + ref->meanRad();
    

    double k = NetworkItem::EDGELENGTH + radsum;
    double repulsion = (dist2 < (k * k) ?  k * k / dist2 : k * k * k / (dist2 * dist));
    
    repulsion = std::min(repulsion, CAP/dist);
    
    // X component of force
    double force = 0.5 * repulsion * dx;
    negGrad[ref->vertexID()].rx() += force;
    other->force().rx() -= force;
    
    force = 0.5 * repulsion * dy;
    negGrad[ref->vertexID()].ry() += force;
    other->force().ry() -= force;    
  }
  
  else
  {
    QuadTreeNode *child = other->nwChild();
    computeRepulsion(ref, child, negGrad);
    
    child = other->neChild();
    computeRepulsion(ref, child, negGrad);
    
    child = other->swChild();
    computeRepulsion(ref, child, negGrad);
    
    child = other->seChild();
    computeRepulsion(ref, child, negGrad);
    
  }  
}

void NetworkLayout::QuadTree::updateLeafForces()
{
  //QuadTreeNode *rootNode = dynamic_cast<QuadTreeNode *>(root());
  
  
  if (! _root->isLeaf())  _root->updateLeafForces();
}



