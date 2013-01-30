#include "NetworkItem.h"

//#include <cmath>
#include <iostream>
using namespace std;

NetworkItem::NetworkItem()
{
  _v = 0;
  _e = 0;
  _parentItem = 0;
  _type = EmptyType;
}

NetworkItem::NetworkItem(const Vertex *v, unsigned freq, const vector<unsigned> &traits, const vector<string> &otherTaxa)
{
  _parentItem = 0;
  _v = v;
  _type = VertexType;
  _e = 0;
  _size = freq;
  
  //_traits = QList<QVariant>::fromVector(QVector<unsigned>::fromStdVector(traits));
  vector<unsigned>::const_iterator trit = traits.begin();
  
  while (trit != traits.end())
  {
    _traits.append(*trit);
    ++trit;
  }
  
  if (! _v->label().empty()) 
    _label = QString::fromStdString(_v->label());
  
  if (! otherTaxa.empty())
  {
    vector<string>::const_iterator taxit = otherTaxa.begin();
    
    while (taxit != otherTaxa.end())
    {
      _taxa.append(QString::fromStdString(*taxit));
      ++taxit;
    }
  }
 
}

NetworkItem::NetworkItem(const Edge *e, NetworkItem *parent)
{
  
  //cout << "edge type constructor" << endl;
  _e = e;
  _size = e->weight();
  _type = EdgeType;
  _v = 0;
  
  //cout << "type: " << _type << " from: " << _e->from()->index() << " to: " << _e->to()->index() << endl;
  
  parent->appendChild(this);
  _parentItem = parent;
}

/*NetworkItem::~NetworkItem()
{
  TODO fill this in
}*/


// child should be an edge; parent ("this") should be a vertex
void NetworkItem::appendChild(NetworkItem *child)
{
  _children.append(child);
}

NetworkItem * NetworkItem::child(int row)
{
  return _children.value(row);
}

int NetworkItem::childCount() const
{
  return _children.count();
}

QVariant NetworkItem::data(int role) const
{
  
  //cout << "in data. Role:" << role << " type: " << _type << endl;
  if (_type == VertexType)
  {
    //cout << "VertexType" << endl;
    if (role == Qt::DisplayRole || role == SizeRole)
    {
      //cout << "returning size: " << _size << endl;
      return _size;
    }
    
    else if (role == LabelRole)
      return _label;
    
    else if (role == TraitRole)
      return _traits;
    
    else if (role == TaxaRole)
      return _taxa;
      
    else
    {
      //cout << "Other roles not yet implemented. Sorry!" << endl;
      return QVariant();
    }
  }
  
  else if (_type == EdgeType)
  {
    //cout << "returning from/to data for edge: " << _e << endl;
    if (role == Qt::DisplayRole || role == SizeRole)
      return _size;
    else if (role == EdgeStartRole)
      return _e->from()->index();
    else if (role == EdgeEndRole)
      return _e->to()->index();
     else
    {
      //cout << "Other edge roles not yet implemented. Sorry!" << endl;
      return QVariant();
    }
   
  }
  
  else
  {
    //cout << "Empty types have no associated data!" << endl;
    return QVariant();
  }
  
  // TODO remember to deal with EmptyType
}

NetworkItem * NetworkItem::parent()
{
  return _parentItem;
}

int NetworkItem::row() const
{
  if (_parentItem)
    return _parentItem->_children.indexOf(const_cast<NetworkItem*>(this));

  return 0;
}

