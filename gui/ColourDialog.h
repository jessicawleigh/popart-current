#ifndef COLOURDIALOG_H
#define COLOURDIALOG_H

#include "NetworkView.h"

#include <QButtonGroup>
#include <QDialog>
#include <QObject>
#include <QSize>
#include <QWidget>

class ColourDialog : public QDialog
{
Q_OBJECT

public:  
  static NetworkView::ColourTheme getColour(QWidget *, NetworkView::ColourTheme, Qt::WindowFlags = 0);
  
private:
  ColourDialog(QWidget *, NetworkView::ColourTheme, Qt::WindowFlags = 0);
  NetworkView::ColourTheme theme() { return _theme; };
  
  const QSize _iconSize;
  NetworkView::ColourTheme _theme;
  //static QButtonGroup *_buttons = 0;
  
private slots:
  void changeColour(int);
  
};

#endif
