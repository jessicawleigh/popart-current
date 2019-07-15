/*
 * NetworkModel.h
 *
 *  Created on: Mar 20, 2012
 *      Author: jleigh
 */

#ifndef NETWORKMODEL_H_
#define NETWORKMODEL_H_

#include <QAbstractItemModel>
#include <QVariant>
#include <QModelIndex>
#include <QVector>
#include <QString>

#include <map>
using namespace std;

#include "NetworkItem.h"
#include "../networks/HapNet.h"
#include "../networks/Vertex.h"
#include "../networks/Edge.h"

class NetworkModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  NetworkModel(const HapNet*, QObject * = 0);
  virtual ~NetworkModel();

  virtual int columnCount(const QModelIndex & = QModelIndex()) const;// { return 1};
  virtual QVariant data(const QModelIndex &, int = Qt::DisplayRole) const;
  virtual QModelIndex index(int, int = 0, const QModelIndex & = QModelIndex() ) const;
  virtual QModelIndex parent(const QModelIndex &) const;
  virtual int rowCount(const QModelIndex & = QModelIndex()) const;
  virtual QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
  
private:
  const HapNet *_graph;
  QVector<NetworkItem *> _vertexItems;
  QVector<QString> _traitText;
  /*QVector<const Vertex*> _vertices;
  QVector<const Edge*> _edges;*/
private slots:
  void updateTraits();
  
signals:
  void traitsUpdated();
};

#endif /* NETWORKMODEL_H_ */
