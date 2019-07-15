#ifndef ALIGNMENTDELEGATE_H
#define ALIGNMENTDELEGATE_H

#include <QObject>
#include <QAbstractItemDelegate>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QColor>
#include <QSize>
#include <QPainter>
#include <map>
using namespace std;

#include "../seqio/Sequence.h"

typedef std::map<char, QColor, std::less<char> > chcolourmap;

class AlignmentDelegate : public QAbstractItemDelegate
{
public:
  AlignmentDelegate(Sequence::CharType = Sequence::DNAType, QObject * = 0);
  virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
  virtual QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
  chcolourmap defaultColourMap(Sequence::CharType) const;
  chcolourmap colourMap() const;
  void setColourMap(chcolourmap);
  Sequence::CharType charType() const;
  void setCharType(Sequence::CharType);

private:
  chcolourmap _colourmap;
  Sequence::CharType _chartype;
};

#endif
