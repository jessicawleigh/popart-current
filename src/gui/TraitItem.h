#ifndef TRAITITEM_H_
#define TRAITITEM_H_

#include <QList>
#include <QVariant>

 
// Shamelessly stollen from Qt simple tree model
class TraitItem
{
public:
  TraitItem(const QList<QVariant> &, TraitItem * = 0);
  virtual ~TraitItem();

  void appendChild(TraitItem *child);

  TraitItem *child(int);
  int childCount() const;
  int columnCount() const;
  QVariant data(int) const;
  int row() const;
  TraitItem *parent();

private:
  QList<TraitItem*> _childItems;
  QList<QVariant> _data;
  TraitItem *_parentItem;
};

 #endif