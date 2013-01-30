#include "TraitItem.h"

#include <iostream>
using namespace std;

TraitItem::TraitItem(const QList<QVariant> &data, TraitItem *parent)
{
  _data = data;
  _parentItem = parent;
  
}

TraitItem::~TraitItem()
{
  qDeleteAll(_childItems);
}

void TraitItem::appendChild(TraitItem *child)
{
  _childItems.append(child);
}

TraitItem * TraitItem::child(int row)
{
  return _childItems.value(row);
}

int TraitItem::childCount() const
{
  
  return _childItems.count();
}

int TraitItem::columnCount() const
{
  return _data.count();
}

QVariant TraitItem::data(int column) const
{  
  return _data.value(column); 
}

int TraitItem::row() const
{
  if (_parentItem)
    return _parentItem->_childItems.indexOf(const_cast<TraitItem*>(this));

  return 0;
}

TraitItem * TraitItem::parent()
{
  
  return _parentItem;
}

