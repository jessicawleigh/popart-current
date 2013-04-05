#ifndef NETWORK_ITEM_H_
#define NETWORK_ITEM_H_

#include "../networks/Vertex.h"
#include "../networks/Edge.h"

#include <QList>
#include <QSize>
#include <QVariant>
#include <QVector>

#include <vector>

 
class NetworkItem
{
public:
  typedef enum {EmptyType, VertexType, EdgeType} ItemType;
  //Q_DECLARE_METATYPE(ItemType);

  static const double VERTWEIGHT = 1;
  static const double VERTRAD = 15;
  static const double EDGELENGTH = 50;
  
  static const int TypeRole = Qt::UserRole + 1;
  static const int LabelRole = Qt::UserRole + 2;
  static const int SizeRole = Qt::UserRole + 3;
  static const int TraitRole = Qt::UserRole + 4;
  static const int TaxaRole = Qt::UserRole + 5;
  static const int EdgeStartRole = Qt::UserRole + 6;
  static const int EdgeEndRole = Qt::UserRole + 7;
  
  NetworkItem();
  NetworkItem(const Edge *, NetworkItem*);
  NetworkItem(const Vertex *, unsigned, const std::vector<unsigned> &, const std::vector<std::string> & = std::vector<std::string>()); // parent
  virtual ~NetworkItem() {};
  QVariant data(int = Qt::DisplayRole) const;
  
  NetworkItem * child(int);
  
  int childCount() const;
  NetworkItem *parent();
  int row() const;
  
  //int row() const;
  //NetworkItem *parent();

private:
  
  void appendChild(NetworkItem *);
  
  QList<NetworkItem*> _children; // Edges, for vertices
  NetworkItem *_parentItem;

  ItemType _type;
  QString _label;
  unsigned _size;
  double _dsize;
  
  QList<QVariant> _traits;
  QList<QVariant> _taxa;
  // 
  const Vertex *_v;
  const Edge *_e;
};

Q_DECLARE_METATYPE(NetworkItem);
 #endif
