#ifndef MAPLEGENDDIALOG_H_
#define MAPLEGENDDIALOG_H_

#include "MapLegendWidget.h"
#include "HapLocation.h"

#include <QDialog>
#include <QVector>

class MapLegendDialog : public QDialog
{
  Q_OBJECT
public:
  MapLegendDialog(QVector<HapLocation*>, QWidget * = 0, Qt::WindowFlags = 0 );
  void setColours(const QVector<QBrush> &colours) { _legend->setColours(colours); };
  const QBrush & hapBrush(int i) const { return _legend->hapBrush(i); };  
  const QFont & smallFont() const { return _legend->smallFont(); };
  void setSmallFont(const QFont & font) { _legend->setSmallFont(font); };
  const QFont & legendFont() const { return _legend->legendFont(); };
  void setLegendFont(const QFont & font) { _legend->setLegendFont(font); };
  void changeLegendFont(const QFont & font) {_legend->changeLegendFont(font); };

private:
  
  MapLegendWidget *_legend;
  
private slots:
  void changeColour(int);
  
signals:
  void colourChangeRequested(int);
};


#endif