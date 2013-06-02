#ifndef MAPLEGENDWIDGET_H_
#define MAPLEGENDWIDGET_H_

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QPaintEvent>
#include <QRegion>
#include <QVector>
#include <QWidget>

#include "HapLocation.h"

class MapLegendWidget : public QWidget
{
  Q_OBJECT
public:
  MapLegendWidget(QVector<HapLocation*> locations, QWidget * = 0);
  virtual QSize minimumSizeHint();
  void setColours(const QVector<QBrush> &colours);// { _colours = colours; };
  const QBrush & hapBrush(int) const;  
  const QFont & smallFont() const { return _smallFont; };
  void setSmallFont(const QFont & font) { _smallFont = font; };
  const QFont & legendFont() const { return _legendFont; };
  void setLegendFont(const QFont & font) { _legendFont = font; };
  void changeLegendFont(const QFont & font);
  
protected:
  virtual void paintEvent(QPaintEvent *);

private:
  QVector<HapLocation*> _hapLocations;
  QVector<QBrush> _colours;
  QBrush _defaultBrush;
  QFont _defaultFont;
  QFont _smallFont;
  QFont _legendFont;
  QVector<QRegion> _legendKeys;
 
};


#endif