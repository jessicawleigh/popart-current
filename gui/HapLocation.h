#ifndef HAPLOCATION_H_
#define HAPLOCATION_H_

#include "Trait.h"

#include <marble/GeoDataCoordinates.h>

#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

class HapLocation : public QObject
{
  Q_OBJECT
public:
  HapLocation(const QString &, QObject * = 0);
  HapLocation(const HapLocation &);
  HapLocation(const Trait &, QObject * = 0);
  ~HapLocation();

  const QString & name() const { return _name; };
  const Marble::GeoDataCoordinates & location() const { return _location; };
  void addSeq(const QString &, unsigned);

  QVector<QString> seqNames() const;
  unsigned seqCount(const QString &) const;
  unsigned totalCount() const { return _totalCount; };
  
  static const QMap<QString, unsigned> & seqIDs() { return _seqIDs; };
  static void associateSeqIDs(QSet<QString>);
  static int seqID(const QString & seqName);
  static void clearSeqIDs() { _seqIDs.clear(); };

public slots:
  void setLocation(const Marble::GeoDataCoordinates &coords);// { _location = new Marble::GeoDataCoordinates(coords); };//emit locationSet(_location); };

private:


  QString _name;
  Marble::GeoDataCoordinates _location;
  QMap<QString, unsigned> _seqCounts;
  static QMap<QString, unsigned> _seqIDs;
  unsigned _totalCount;
  //const std::vector<unsigned> & traits(unsigned) const;
signals:
  void nameSet(const QString &);
  void locationSet(const Marble::GeoDataCoordinates &);
  void seqAdded(const QString &, unsigned);
};


#endif
