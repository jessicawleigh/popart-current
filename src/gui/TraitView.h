#ifndef TRAITVIEW_H
#define TRAITVIEW_H

#include <QObject>
#include <QWidget>
#include <QTreeView>
#include <QKeyEvent>
#include <QMouseEvent>

#include "TraitModel.h"

class TraitView : public QTreeView
{
  Q_OBJECT
public:
  TraitView (QWidget * = 0);
  virtual ~TraitView() {};
  
};

#endif
