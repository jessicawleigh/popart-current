#include "NestedGroupDialog.h"

#include <QAction>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QItemSelection>
#include <QListWidgetItem>
#include <QMenu>
#include <QModelIndexList>
#include <QPoint>
#include <QPushButton>
#include <QRect>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

using namespace std;

#include <QDebug>


NestedGroupDialog::NestedGroupDialog(const vector<Trait*> &populations, QWidget *parent, Qt::WindowFlags flags)
 : QDialog(parent, flags)
{
  
  _populations = QVector<Trait*>::fromStdVector(populations);
  
  _unassignedView = new PopulationListWidget(this);
  _unassignedView->setDragEnabled(true);
  setPopulations();
  _unassignedView->setSelectionMode(QAbstractItemView::ExtendedSelection); // Maybe MultiSelection?
  connect(_unassignedView, SIGNAL(groupSelected(const QString &)), this, SLOT(addSelectedPopsToGroup(const QString &)));
  
  _groupView = new PopGroupWidget(this);
  _groupView->setHeaderHidden(true);
  _groupView->setItemsExpandable(false);
  _groupView->setColumnCount(1);
  _groupView->setDragEnabled(true);
  _groupView->setAcceptDrops(true);
  //_groupView->viewport()->setAcceptDrops(true);
  //_groupView->setDropIndicatorShown(true);
  //_groupView->setSupportedDropActions(Qt::MoveAction);
  _groupView->setDragDropMode(QAbstractItemView::DragDrop); // Maybe DragDrop? InternalMove);
  
  connect(_groupView, SIGNAL(popsRemoved(const QList<QPair<QString, int> > &)), this, SLOT(deassignPopulations(const QList<QPair<QString, int> > &)));
  connect(_groupView, SIGNAL(groupDeleted(QString)), _unassignedView, SLOT(removeGroup(QString)));
  
   
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
  connect(okButton, SIGNAL(clicked()), this, SLOT(checkAndAccept()));
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
  
  if (! groupName.isEmpty())
  {
    if (_groupView->findItems(groupName, Qt::MatchExactly).isEmpty())
    {
      QTreeWidgetItem *groupItem = new QTreeWidgetItem(QStringList(groupName));
      groupItem->setFlags(groupItem->flags() & ~Qt::ItemIsDragEnabled);
      _groupView->addTopLevelItem(groupItem); 
      
      _unassignedView->addGroup(groupName);
      groupItem->setExpanded(true);      
    }
    
    else 
    {
      QMessageBox::warning(this, "<b>Group Exists</b>", QString("A group named %1 has already been defined").arg(groupName));
    }
  }
}


QMap<QString, QList<Trait *> > NestedGroupDialog::groups() const 
{
  QMap<QString, QList<Trait *> > popMap;
  
  for (int i = 0; i < _groupView->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *groupItem = _groupView->topLevelItem(i);
    QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
    QList<Trait*> pops;
    for (int j = 0; j < groupItem->childCount(); j++)
    {
      QTreeWidgetItem *popItem = groupItem->child(j);
      int traitIdx = popItem->data(0, Qt::UserRole).toInt();
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

void NestedGroupDialog::addSelectedPopsToGroup(const QString &group)
{
  // There should be only one item
  foreach (QTreeWidgetItem *groupItem, _groupView->findItems(group, Qt::MatchExactly))
  {    
    foreach(QListWidgetItem *popItem, _unassignedView->selectedItems())
    {
      QStringList popData;
      popData << popItem->data(Qt::DisplayRole).toString();
      QTreeWidgetItem *popChild = new QTreeWidgetItem(groupItem, popData);
      popChild->setFlags(popChild->flags() & ~Qt::ItemIsDropEnabled);
      popChild->setData(0, Qt::UserRole, popItem->data(Qt::UserRole));
      
      delete popItem;
    }  
  }
}

void NestedGroupDialog::deassignPopulations(const QList<QPair<QString,int> > & popData)
{
  QList<QPair<QString,int> >::const_iterator dataIter = popData.constBegin();
  
  while (dataIter != popData.constEnd())
  {
    QPair<QString,int> datum = *dataIter;
    QListWidgetItem *popItem = new QListWidgetItem(datum.first);
    popItem->setData(Qt::UserRole, datum.second);
    _unassignedView->addItem(popItem);
    
    ++dataIter;
  }
}

void NestedGroupDialog::checkAndAccept()
{
  if (_unassignedView->count() > 0)
    QMessageBox::warning(this, "<b>Populations Not Assigned</b>", "All populations must be assigned to groups before clicking OK.");
  else
    accept();
}


void PopulationListWidget::removeGroup(QString groupName)
{
  
  // TODO add more error control here.
  int idx = _groupNames.indexOf(groupName);
  
  if (idx >= 0)
    _groupNames.removeAt(idx);
}

void PopulationListWidget::contextMenuEvent(QContextMenuEvent *event)
{ 
  QMenu menu;
  
  if (selectedItems().isEmpty())
  {
    QAction *emptyAct = menu.addAction("(No populations selected)");
    emptyAct->setEnabled(false);
  }
  
  else
  {
    QMenu *addMenu = menu.addMenu("Add to group...");
    
    
    if (_groupNames.isEmpty())
    {
      QAction *noGroupsAct = addMenu->addAction("(No groups defined)");
      noGroupsAct->setEnabled(false);
    }
    
    else
    {
      QActionGroup *groupActions = new QActionGroup(addMenu);
     
      foreach (QString groupName, _groupNames)
      {
        
        QAction *groupAct = groupActions->addAction(groupName);
        addMenu->addAction(groupAct);

      }
      
      connect(groupActions, SIGNAL(triggered(QAction *)), this, SLOT(setSelectedGroup(QAction *)));
    }
  }
  
  menu.exec(event->globalPos());
}

void PopulationListWidget::mousePressEvent(QMouseEvent *event)
{
  
  _mousePressed = event->pos();
  
  QListWidget::mousePressEvent(event);
}

void PopulationListWidget::startDrag(Qt::DropActions)// supportedActions)
{
  QModelIndexList indices = selectedIndexes();
  QItemSelection selection(indices.first(), indices.last());

  QRegion selectedRegion = visualRegionForSelection(selection);
  selectedRegion.translate(3,3);

  QRect selectedRect = selectedRegion.boundingRect();
  QPixmap pixmap(selectedRect.size());
  pixmap.fill(Qt::transparent);

  
  foreach (QListWidgetItem *popItem, selectedItems())
  {
    QRect rect(visualItemRect(popItem));
    rect.adjust(3, 3, 3, 3);
    render(&pixmap, rect.topLeft() - selectedRect.topLeft(), QRegion(rect));  
  }
  
  //qDebug() << "horizontal offset: " << horizontalOffset() << " vertical offset: " << verticalOffset();
  
  QDrag *drag = new QDrag(this);
  drag->setPixmap(pixmap);
  drag->setMimeData(mimeData(selectedItems()));
  drag->setHotSpot(_mousePressed - selectedRect.topLeft());
    
  if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
  {
    foreach (QListWidgetItem *popItem, selectedItems())
      delete takeItem(row(popItem));
  }
}

QStringList PopulationListWidget::mimeTypes() const
{  
  QStringList types = QListWidget::mimeTypes();
  types << "text/x-popart-pop";
  
  return types;
}

QMimeData* PopulationListWidget::mimeData(const QList<QListWidgetItem *> items) const
{
  QByteArray popData;
  QDataStream dataStream(&popData, QIODevice::WriteOnly);
  
  foreach (QListWidgetItem *popItem, items)
  {
    QString popName = popItem->data(Qt::DisplayRole).toString();
    int popIdx = popItem->data(Qt::UserRole).toInt();
      
    dataStream << popName << popIdx;
  }
  
  QMimeData *mimeData = new QMimeData;
  mimeData->setData("text/x-popart-pop", popData);
  
  return mimeData;
}


void PopulationListWidget::setSelectedGroup(QAction *action) 
{
  _selectedGroup = action->text(); 
  
  emit groupSelected(_selectedGroup);
}

void PopGroupWidget::contextMenuEvent(QContextMenuEvent *event)
{  
  QMenu menu;
  
  if (selectedItems().isEmpty())
  {
    QAction *emptyAct = menu.addAction("(No groups selected)");
    emptyAct->setEnabled(false);    
  }
  
  else if (selectedItems().first()->parent())
  {
    QAction *deassignPopAct = menu.addAction("Remove population from group");
    connect(deassignPopAct, SIGNAL(triggered()), this, SLOT(deassignSelectedPop()));
  }
  
  else
  {
    QAction *deleteGroupsAct = menu.addAction("Delete selected group");
    connect(deleteGroupsAct, SIGNAL(triggered()), this, SLOT(deleteSelectedGroups()));
  }
  
  menu.exec(event->globalPos());
}

void PopGroupWidget::mousePressEvent(QMouseEvent *event)
{
  _mousePressed = event->pos();
  
  QTreeWidget::mousePressEvent(event);
}

QStringList PopGroupWidget::mimeTypes() const
{  
  QStringList types = QTreeWidget::mimeTypes();
  types << "text/x-popart-pop";
  
  return types;
}

QMimeData * PopGroupWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{
  QByteArray popData;
  QDataStream dataStream(&popData, QIODevice::WriteOnly);
  
  foreach (QTreeWidgetItem *popItem, items)
  {
    QString popName = popItem->data(0, Qt::DisplayRole).toString();
    int popIdx = popItem->data(0, Qt::UserRole).toInt();
      
    dataStream << popName << popIdx;
  }
  
  QMimeData *mimeData = new QMimeData;
  mimeData->setData("text/x-popart-pop", popData);
  
  return mimeData;  
}

bool PopGroupWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
  bool dropSuccess = false;
  
  
  if (action == Qt::MoveAction && data->hasFormat("text/x-popart-pop"))
  {
    if (parent && (parent->flags() & Qt::ItemIsDropEnabled))
    {
      QByteArray itemData = data->data("text/x-popart-pop");
      QDataStream dataStream(&itemData, QIODevice::ReadOnly);
      
      while (! dataStream.atEnd())
      { 
        QString popName;
        int popIdx;
        dataStream >> popName;
        dataStream >> popIdx;
        
        QTreeWidgetItem *popChild = new QTreeWidgetItem(parent, QStringList(popName));
        popChild->setFlags(popChild->flags() & ~Qt::ItemIsDropEnabled);
        popChild->setData(0, Qt::UserRole, popIdx);      
      } 
      
      dropSuccess = true;
    }
  }
  
  else
    dropSuccess = QTreeWidget::dropMimeData(parent, index, data, action);
  
  return dropSuccess;
}

void PopGroupWidget::startDrag(Qt::DropActions)
{  
  
  QRect rect = visualItemRect(currentItem());
  rect.adjust(3, 3, 3, 3);
  QPixmap pixmap(rect.size());
  render(&pixmap, QPoint(), QRegion(rect));
  
  QDrag *drag = new QDrag(this);
  drag->setPixmap(pixmap);
  drag->setMimeData(mimeData(selectedItems()));
  drag->setHotSpot(_mousePressed - rect.topLeft());
    
  drag->exec(Qt::MoveAction);
}

void PopGroupWidget::deleteSelectedGroups()
{
  _deassignedPops.clear();
  
  foreach (QTreeWidgetItem *groupItem, selectedItems())
  {
    for (int j = 0; j < groupItem->childCount(); j++)
    {
      QTreeWidgetItem *popItem = groupItem->child(j);
      QString popName = popItem->data(0, Qt::DisplayRole).toString();
      int traitIdx = popItem->data(0, Qt::UserRole).toInt();
      QPair<QString,int> popData(popName, traitIdx);      
      
      _deassignedPops << popData;
      
    }
    
    QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
    delete groupItem;
    
    emit groupDeleted(groupName);
  }
    
  emit popsRemoved(_deassignedPops);
}

void PopGroupWidget::deassignSelectedPop()
{  
  _deassignedPops.clear();
  
  QTreeWidgetItem *popItem = selectedItems().first();
  QString popName = popItem->data(0, Qt::DisplayRole).toString();
  int traitIdx = popItem->data(0, Qt::UserRole).toInt();
  QPair<QString,int> popData(popName, traitIdx);
  _deassignedPops << popData;

  delete popItem;
  
  emit popsRemoved(_deassignedPops);

}


