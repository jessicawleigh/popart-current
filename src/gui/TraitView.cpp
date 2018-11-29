#include "TraitView.h"

#include <QColor>
#include <QPalette>

TraitView::TraitView(QWidget *parent)
  : QTreeView(parent)
{
  
  //painter->setPen(option.palette.highlightedText().color());
  
  /*QPalette * newpal = new QPalette(palette());
  
  newpal->setColor(QPalette::Active, QPalette::Foreground, QColor("red"));
  
  setPalette(*newpal);*/
  //setData() and Qt::BackgroundColorRole or Qt::TextColorRole
  
}


