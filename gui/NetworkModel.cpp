/*
 * NetworkModel.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: jleigh
 */
#include <cmath>

#include "NetworkModel.h"

NetworkModel::NetworkModel(const HapNet *g, QObject * parent)
  : QAbstractItemModel(parent), _graph(g)
{  
  const Edge *e;
  NetworkItem *elem, *parentItem;
  
  for (unsigned i = 0; i < _graph->vertexCount(); i++)
  {
    elem = new NetworkItem(_graph->vertex(i), _graph->freq(i), _graph->traits(i), _graph->identicalTaxa(i));
    _vertexItems.push_back(elem);
  }
  
  for (unsigned i = 0; i < _graph->edgeCount(); i++)
  {
    e = _graph->edge(i);
    parentItem = _vertexItems.at(e->from()->index());
    elem = new NetworkItem(e, parentItem);
  }
  
  
  vector<string>::const_iterator trIt = _graph->traitNames().begin();
  
  while (trIt != _graph->traitNames().end())
  {
    _traitText.push_back(QString::fromStdString(*trIt));  
    ++trIt;
  }
}

NetworkModel::~NetworkModel()
{
  
  QVector<NetworkItem *>::iterator elemIt = _vertexItems.begin();
  while (elemIt != _vertexItems.end())
  {
    delete *elemIt;
    ++elemIt;
  }
    
  _vertexItems.clear();
}

QVariant NetworkModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid())  
  {
    
    NetworkItem *item = static_cast<NetworkItem *>(index.internalPointer());
    
    return item->data(role);
  }
  
  else
  {
    return QVariant();
  }
  
}


QVariant NetworkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (section < 0 || section >= columnCount())  return QVariant();
    
    if (_traitText.empty())  return "(no traits)";
    else  return _traitText.at(section);
    
  }
  
  return QVariant();
}


QModelIndex NetworkModel::index(int row, int column, const QModelIndex &parent) const
{
  if (! hasIndex(row, column, parent))
  {
    return QModelIndex();
  }
  
  if (parent.isValid())
  {
    NetworkItem* parentItem = _vertexItems.value(parent.row());
    if (! parentItem)  return QModelIndex();
    
    NetworkItem* childItem = parentItem->child(row);
    if (childItem)  
      return createIndex(row, column, childItem);
    
    else  
      return QModelIndex();

  }
  
  else
  {
    NetworkItem* item = _vertexItems.value(row);//parent.row());
    
    if (item)  return createIndex(row, column, item);
    else  return QModelIndex();
  }
  // Maybe just use a list model?
  // Maybe keep separate lists for edges and vertices? Separate parents?

}

QModelIndex NetworkModel::parent(const QModelIndex &index) const
{
  if (! index.isValid())  return QModelIndex();
  
  NetworkItem *childItem = static_cast<NetworkItem*>(index.internalPointer());
  NetworkItem *parentItem = childItem->parent();
  
  if (! parentItem)  
    return QModelIndex();
  
  return  createIndex(parentItem->row(), 0, parentItem);
  // TODO return a QModelIndex corresponding to the vertex to which an edge belongs
  // if this is already a vertex, return QModelIndex()
}

int NetworkModel::columnCount(const QModelIndex &parent) const
{  
  if (_traitText.empty())  return 1;
  
  else  return _traitText.size();
}

int NetworkModel::rowCount(const QModelIndex &parent) const
{  
  NetworkItem *parentItem = 0;
  
  if (parent.column() > 0)  return 0;
  
  if (! parent.isValid())  return _vertexItems.size();
  else  parentItem = static_cast<NetworkItem *>(parent.internalPointer());
  
  return parentItem->childCount();
}


