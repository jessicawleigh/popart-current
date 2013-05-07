#include "HapAppError.h"
#include "HapLocation.h"

#include <QList>

#include <vector>
#include <string>
using namespace std;

using namespace Marble;

HapLocation::HapLocation(const QString &name, QObject *parent)
  : QObject(parent), _location(0, 0, 0), _totalCount(0)
{
  _name = name;
  emit nameSet(_name);
}

HapLocation::HapLocation(const Trait &trait, QObject *parent)
  : QObject(parent), _location(0, 0, 0)
{
  _name = QString::fromStdString(trait.name());
  emit nameSet(_name);

  const vector<string> seqNames = trait.seqNames();
  for (unsigned i = 0; i < seqNames.size(); i++)
    addSeq(QString::fromStdString(seqNames.at(i)), trait.seqCount(seqNames.at(i)));
}

HapLocation::HapLocation(const HapLocation &loc)
  : QObject(loc.parent()), _name(loc._name), _location(loc._location), _seqCounts(loc._seqCounts), _totalCount(loc._totalCount)
{
}

HapLocation::~HapLocation()
{
  //TODO
}

void HapLocation::addSeq(const QString &seqname, unsigned seqcount)
{
  _seqCounts[seqname] = seqcount;
  _totalCount += seqcount;

  emit seqAdded(seqname, seqcount);
}

QVector<QString> HapLocation::seqNames() const
{
  return QVector<QString>::fromList(_seqCounts.keys());
}

unsigned HapLocation::seqCount(const QString &seqname) const
{
  QMap<QString, unsigned>::const_iterator locIt = _seqCounts.find(seqname);

  if (locIt == _seqCounts.end())  throw HapAppError("Sequence not associated with this location.");

  return locIt.value();
}


