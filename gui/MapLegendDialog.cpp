#include "MapLegendDialog.h"

#include <QLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPalette>
#include <QTextEdit>

#include <QDebug>

MapLegendDialog::MapLegendDialog(QVector<HapLocation*> locations, QWidget * parent, Qt::WindowFlags flags)
 : QDialog(parent, flags)
{
  
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  
  QWidget *viewport = new QWidget;  
  QVBoxLayout *layout = new QVBoxLayout(viewport);

  QScrollArea *scrollArea = new QScrollArea;//(this);
  scrollArea->setWidget(viewport);
  scrollArea->setWidgetResizable(true);
  
  //QTextEdit *text = new QTextEdit("This is some text", this);
  //layout->addWidget(text);
  _legend = new MapLegendWidget(locations, this);
  layout->addWidget(_legend);
  
  
  mainLayout->addWidget(scrollArea);
  
  
  //scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  //scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  //resize(scrollArea->width(), scrollArea->height());
  //setGeometry(QRect(0,0,400,400));
  
  //resize(legend->width(), legend->height());
  
  qDebug() << "scroll area size:" << scrollArea->width() << "," << scrollArea->height();
  qDebug() << "dialog size:" << width() << "," << height();
}