#ifndef HAPDATAPLACEMARK_H_
#define HAPDATAPLACEMARK_H_

#include <QString>

#include <marble/GeoDataPlacemark.h>


class HapDataPlacemark : public Marble::GeoDataPlacemark
{
public:
  HapDataPlacemark() : Marble::GeoDataPlacemark() {};
  HapDataPlacemark(const QString &);
  virtual ~HapDataPlacemark() {/*TODO*/};
};

#endif;
