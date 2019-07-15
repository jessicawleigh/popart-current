#include "TraitModel.h"

#include <iostream>
using namespace std;

#include <QString>

TraitModel::TraitModel(const vector<Trait *> &traitvect, QObject *parent)
  : QAbstractItemModel(parent), _traits(traitvect)
{

  QList<QVariant> rootData;
  
  rootData << "Trait" << "Sequence" << "Samples";
  _rootItem = new TraitItem(rootData);
  
  vector<Trait *>::const_iterator traitIt = _traits.begin();
  
  while (traitIt != _traits.end())
  {
    QList<QVariant> traitData;
    
    traitData << QString::fromStdString((*traitIt)->name());
    
    
    unsigned seqCount = 0;
    unsigned sampCount = 0;

    vector<string> seqNames = (*traitIt)->seqNames();
    vector<string>::iterator seqIt = seqNames.begin();
    
    
    while (seqIt != seqNames.end())
    {
      unsigned nsamps = (*traitIt)->seqCount(*seqIt);
      sampCount += nsamps;
      seqCount++;
      ++seqIt;
    }
    
    QString seqCountStr = QString("(%1 sequences)").arg(seqCount);
    
    QString sampCountStr = QString("(%1 samples)").arg(sampCount);
    traitData << seqCountStr << sampCountStr;// number of sequences
    
    TraitItem *aTrait = new TraitItem(traitData, _rootItem); 
    _rootItem->appendChild(aTrait);
    
    seqIt = seqNames.begin();
    
    while (seqIt != seqNames.end())
    {
      QList<QVariant> seqData;
      seqData << "" << QString::fromStdString(*seqIt);
      
      unsigned nsamps = (*traitIt)->seqCount(*seqIt);
      QString seqSampStr;
      seqSampStr.setNum(nsamps);
      
      seqData << seqSampStr;
      
      TraitItem *seqTrait = new TraitItem(seqData, aTrait);
      aTrait->appendChild(seqTrait);
      
      ++seqIt;
    }
    
    ++traitIt;
    
  }   
}

int TraitModel::columnCount (const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<TraitItem*>(parent.internalPointer())->columnCount();

  else
    return _rootItem->columnCount();  
}

int TraitModel::rowCount (const QModelIndex &parent) const
{
  TraitItem *parentItem = 0;
  
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())  parentItem = _rootItem;
  
  else  parentItem = static_cast<TraitItem*>(parent.internalPointer());

  return parentItem->childCount();
}

QVariant TraitModel::data (const QModelIndex &index, int role) const
{
  if (index.isValid()  && role == Qt::DisplayRole)
  {
    TraitItem *item = static_cast<TraitItem*>(index.internalPointer());

    return item->data(index.column());
  }
  
  else  return QVariant();
}

QVariant TraitModel::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole || orientation == Qt::Vertical)
    return QVariant();

  else  
    return _rootItem->data(section);

}

QModelIndex TraitModel::index (int row, int column, const QModelIndex & parent) const
{
  if (! hasIndex(row, column, parent))
    return QModelIndex();

  TraitItem *parentItem;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<TraitItem*>(parent.internalPointer());

  TraitItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex TraitModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  TraitItem *childItem = static_cast<TraitItem*>(index.internalPointer());
  TraitItem *parentItem = childItem->parent();

  if (parentItem == _rootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

bool TraitModel::setData (const QModelIndex & index, const QVariant & value, int role)
{return false;}

Qt::ItemFlags TraitModel::flags (const QModelIndex & index) const
{return Qt::ItemIsEnabled |Qt::ItemIsSelectable;}

/* Qt::NoItemFlags, ItemIsEditable, ItemIsDragEnabled, ItemIsDropEnabled, ItemIsUserCheckable, ItemIsEnabled, ItemIsTristate*/

bool TraitModel::insertRows (int row, int count, const QModelIndex & parent)
{return false;}

bool TraitModel::removeRows (int row, int count, const QModelIndex & parent)
{return false;}
