#include "MapLegendDialog.h"

#include <QLayout>
#include <QScrollArea>

MapLegendDialog::MapLegendDialog(QVector<HapLocation*> locations, QWidget * parent, Qt::WindowFlags flags)
 : QDialog(parent, flags)
{
  
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  
  QWidget *viewport = new QWidget;  
  QVBoxLayout *layout = new QVBoxLayout(viewport);

  QScrollArea *scrollArea = new QScrollArea;//(this);
  scrollArea->setWidget(viewport);
  scrollArea->setWidgetResizable(true);

  _legend = new MapLegendWidget(locations, this);
  
  connect(_legend, SIGNAL(colourChangeTriggered(int)), this, SLOT(changeColour(int)));
  layout->addWidget(_legend);
  
  
  mainLayout->addWidget(scrollArea);
  

}

void MapLegendDialog::changeColour(int key)
{
  emit colourChangeRequested(key);
}