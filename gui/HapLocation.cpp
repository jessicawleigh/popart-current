#include "HapAppError.h"
#include "HapLocation.h"

#include <QDebug>
#include <QList>

#include <vector>
#include <string>
using namespace std;

using namespace Marble;

/*QMap<QString, unsigned> emptyMap()
{
  QMap<QString, unsigned> tmp;
  
  return tmp;
}*/

QMap<QString, unsigned> HapLocation::_seqIDs = QMap<QString, unsigned>();

HapLocation::HapLocation(const QString &name, QObject *parent)
  : QObject(parent), _location(0,0,0), _totalCount(0)
{
  _name = name;
  //_location = new GeoDataCoordinates(0,0,0);
  emit nameSet(_name);
}

HapLocation::HapLocation(const Trait &trait, QObject *parent)
  : QObject(parent), _location(0,0,0), _totalCount(0)
{
  _name = QString::fromStdString(trait.name());
 // _location = new GeoDataCoordinates(0,0,0);
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
  // delete _location
}

void HapLocation::addSeq(const QString &seqname, unsigned seqcount)
{
  _seqCounts[seqname] = seqcount;
  _totalCount += seqcount;
  
  int id = seqID(seqname);
  if (id < 0)
  {
    id = _seqIDs.size();
    _seqIDs[seqname] = id;//_seqIDs.size();
  }
    
  else
    _seqIDs[seqname] = id;


  emit seqAdded(seqname, seqcount);
}

void HapLocation::associateSeqIDs(QSet<QString> seqNames)
{
  _seqIDs.clear();
  
  unsigned count = 0;
  
  QSet<QString>::const_iterator nameIt = seqNames.constBegin();
  
  while(nameIt != seqNames.constEnd())
  {
    _seqIDs[*nameIt] = count;
    ++nameIt;
    count++;
  }
}


void HapLocation::setLocation(const Marble::GeoDataCoordinates &coords) 
{
  _location = coords;
  //qDebug() << "Latitude:" << _location->latToString() << "Longitude:" << _location->lonToString();
  emit locationSet(_location);
  
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

int HapLocation::seqID(const QString &seqname)
{
  QMap<QString, unsigned>::const_iterator locIt = _seqIDs.find(seqname);

  if (locIt == _seqIDs.end())  return -1;

  return locIt.value();
}



