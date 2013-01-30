#ifndef TRAITMODEL_H
#define TRAITMODEL_H

#include "TraitItem.h"
#include "../seqio/Trait.h"

#include <vector>


#include <QList>
#include <QObject>
#include <QVariant>
#include <QAbstractItemModel>

class TraitModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  TraitModel (const std::vector<Trait *> &, QObject * = 0);
  virtual ~TraitModel() {};
  
  virtual int columnCount (const QModelIndex &  = QModelIndex()) const;
  virtual int rowCount (const QModelIndex &  = QModelIndex()) const;
  virtual QVariant data (const QModelIndex &, int = Qt::DisplayRole) const;
  virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
  virtual bool setData (const QModelIndex &, const QVariant &, int = Qt::EditRole);
  virtual Qt::ItemFlags flags (const QModelIndex &) const;
  virtual bool insertRows (int, int, const QModelIndex & = QModelIndex());
  virtual bool removeRows (int, int, const QModelIndex & = QModelIndex());
  virtual QModelIndex index (int, int, const QModelIndex & = QModelIndex()) const;
  virtual QModelIndex parent (const QModelIndex & ) const;
  
  
private:
  //const static int NCOLS = 2;
  TraitItem *_rootItem;
  const std::vector <Trait *> _traits;
  
  
  
  
  // Probably don't need to do this:
  //insertColumns
  //removeColumns
};

#endif
