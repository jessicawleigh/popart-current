#ifndef COLOURDIALOG_H
#define COLOURDIALOG_H

#include "ColourTheme.h"

#include <QButtonGroup>
#include <QDialog>
#include <QObject>
#include <QSize>
#include <QWidget>

class ColourDialog : public QDialog
{
Q_OBJECT

public:  
  static ColourTheme::Theme getColour(QWidget *, ColourTheme::Theme, Qt::WindowFlags = 0, bool * = 0, bool * = 0);
  
private:
  ColourDialog(QWidget *, ColourTheme::Theme, Qt::WindowFlags = 0);
  ColourTheme::Theme theme() { return _theme; };
  bool changeMapTheme() { return _changeMapTheme; };
  bool changeNetTheme() { return _changeNetTheme; };
  
  const QSize _iconSize;
  ColourTheme::Theme _theme;
  bool _changeMapTheme;
  bool _changeNetTheme;
  //static QButtonGroup *_buttons = 0;
  
private slots:
  void changeColour(int);
  void toggleChangeMapTheme(int state) {_changeMapTheme = (state == Qt::Unchecked ? false : true); };
  void toggleChangeNetTheme(int state) {_changeNetTheme = (state == Qt::Unchecked ? false : true); };
  
};

#endif
