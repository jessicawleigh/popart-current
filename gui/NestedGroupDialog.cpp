#include "NestedGroupDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

using namespace std;

#include <QDebug>


NestedGroupDialog::NestedGroupDialog(const vector<Trait*> &populations, QWidget *parent, Qt::WindowFlags flags)
 : QDialog(parent, flags)
{
  
  _populations = QVector<Trait*>::fromStdVector(populations);
  
  _unassignedView = new QListWidget(this);
  setPopulations();
  
  
  _groupView = new QTreeWidget(this);
  _groupView->setHeaderHidden(true);
  _groupView->setColumnCount(2);
   
  QVBoxLayout *outerLayout = new QVBoxLayout(this);
  QHBoxLayout *mainLayout = new QHBoxLayout;
  
  QVBoxLayout *listLayout = new QVBoxLayout;
  listLayout->addWidget(new QLabel("<b>Unassigned populations</b>", this));
  listLayout->addWidget(_unassignedView);
  mainLayout->addLayout(listLayout);
  
  QVBoxLayout *treeLayout = new QVBoxLayout;
  treeLayout->addWidget(new QLabel("<b>Population groups</b>", this));
  treeLayout->addWidget(_groupView);
  
  QHBoxLayout *addGroupLayout = new QHBoxLayout;
  _addGroupEdit = new QLineEdit(this);
  connect(_addGroupEdit, SIGNAL(returnPressed()), this, SLOT(addGroup()));
  addGroupLayout->addWidget(_addGroupEdit);
  QPushButton *addGroupButton = new QPushButton("Add group", this);
  connect(addGroupButton, SIGNAL(clicked()), this, SLOT(addGroup()));
  addGroupLayout->addWidget(addGroupButton);
  
  treeLayout->addLayout(addGroupLayout);
  mainLayout->addLayout(treeLayout);
  outerLayout->addLayout(mainLayout);
  
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  
  buttonLayout->addStretch(1);

  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  buttonLayout->addWidget(okButton, 0, Qt::AlignRight);
  
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", this);
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  buttonLayout->addWidget(cancelButton, 0, Qt::AlignRight);
   
  outerLayout->addLayout(buttonLayout);
}

void NestedGroupDialog::addGroup()
{
  QString groupName = _addGroupEdit->text();
  _addGroupEdit->clear();
  
  if (groupName.isEmpty())
    qDebug() << "no group name given!";
  
  else
  {
    qDebug() << "new group: " << groupName;
    //_groupView->invisibleRootItem();
    _groupView->addTopLevelItem(new QTreeWidgetItem(QStringList(groupName))); 
  }
}

// When adding a population, set Qt::DisplayRole to Trait name (as a QString), and Qt::UserRole to trait's index

QMap<QString, QList<Trait *> > NestedGroupDialog::groups() const 
{
  QMap<QString, QList<Trait *> > popMap;
  //QTreeWidgetItem *root = _groupView->invisibleRootItem();
  
  for (int i = 0; i < _groupView->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *groupItem = _groupView->topLevelItem(i);
    QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
    QList<Trait*> pops;
    for (int j = 0; j < groupItem->childCount(); j++)
    {
      QTreeWidgetItem *popItem = groupItem->child(j);
      int traitIdx = popItem->data(1, Qt::UserRole).toInt();
      pops << _populations.at(traitIdx);
    }
    
    popMap.insert(groupName, pops);
  }
  
  return popMap;
}


void NestedGroupDialog::setPopulations()
{
  for (int i = 0; i < _populations.size(); i++)
  {
    Trait *t = _populations.at(i);
    QListWidgetItem *popItem = new QListWidgetItem(QString::fromStdString(t->name()));
    popItem->setData(Qt::UserRole, i);
    _unassignedView->addItem(popItem);
  }
}



