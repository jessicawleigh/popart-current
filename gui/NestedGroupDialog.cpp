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
  //_groupView->setSupportedDropActions(Qt::MoveAction);
  _groupView->setDragDropMode(QAbstractItemView::InternalMove); // Maybe DragDrop? InternalMove);
  
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
  
  if (! groupName.isEmpty())
  {
    //qDebug() << "new group: " << groupName;
    //_groupView->invisibleRootItem();
    if (_groupView->findItems(groupName, Qt::MatchExactly).isEmpty())
    {
      QTreeWidgetItem *groupItem = new QTreeWidgetItem(QStringList(groupName));
      //groupItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
      //qDebug() << "drag flag: " << ((groupItem->flags() & Qt::ItemIsDragEnabled) ? true : false) << " drop flag: " << ((groupItem->flags() & Qt::ItemIsDropEnabled) ? true : false);
      /*qDebug() << "testing logic. current flags: " << groupItem->flags();
      qDebug() << "Qt::ItemIsEnabled: " << Qt::ItemIsEnabled;
      qDebug() << "current flags & ~ itemIsEnabled: " << (groupItem->flags() & (~Qt::ItemIsEnabled));
      qDebug() << "desired flags (I think): " << (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
      qDebug() << "current flags & ~ itemIsDragEnabled: " << (groupItem->flags() & (~Qt::ItemIsDragEnabled));*/
      //groupItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
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
    /*QString itemName = item->data(0, Qt::DisplayRole).toString();
    qDebug() << "found item with data: " << itemName;*/
    
    foreach(QListWidgetItem *popItem, _unassignedView->selectedItems())
    {
      QStringList popData;
      popData << popItem->data(Qt::DisplayRole).toString();
      QTreeWidgetItem *popChild = new QTreeWidgetItem(groupItem, popData);
      
      // *** CHECK THIS ***
      // Note only change from default is that children are not selectable... maybe change this.
      //popChild->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
      //popChild->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
      popChild->setFlags(popChild->flags() & ~Qt::ItemIsDropEnabled);
      popChild->setData(0, Qt::UserRole, popItem->data(Qt::UserRole));
      
      //*** CHECK THIS ***
      //_unassignedView->takeItem(_unassignedView->row(popItem));
      
      delete popItem;
    }
    
    //groupItem->setExpanded(true);
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


void PopulationListWidget::removeGroup(QString groupName)
{
  
  // TODO add more error control here.
  int idx = _groupNames.indexOf(groupName);
  
  if (idx >= 0)
    _groupNames.removeAt(idx);
}

void PopulationListWidget::contextMenuEvent(QContextMenuEvent *event)
{
  /*qDebug() << "Selected items:";
  
  foreach (QListWidgetItem *item, selectedItems())
  {
    qDebug() << item->data(Qt::DisplayRole).toString();
  }*/
  

  
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
        //qDebug() << "got group name: " << groupName;
        
        QAction *groupAct = groupActions->addAction(groupName);
        addMenu->addAction(groupAct);

      }
      
      connect(groupActions, SIGNAL(triggered(QAction *)), this, SLOT(setSelectedGroup(QAction *)));
    }
  }
  
  menu.exec(event->globalPos());
}

/*void PopulationListWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
  qDebug() << "got a drag leave event.";
  
  // set mime data
  
  
}*/

void PopulationListWidget::mousePressEvent(QMouseEvent *event)
{
  
  _mousePressed = event->pos();
  
  QListWidget::mousePressEvent(event);
}

void PopulationListWidget::startDrag(Qt::DropActions)// supportedActions)
{
  // TODO  highlight group to be dropped on when dragging and dropping from list (see drag-and-drop from within tree)
  // don't let user hit OK until all pops assigned to groups
  // collect group structure to use with AMOVA
  // write nested AMOVA
  
    
  QByteArray popData;
  QDataStream dataStream(&popData, QIODevice::WriteOnly);
  
  QModelIndexList indices = selectedIndexes();
  QItemSelection selection(indices.first(), indices.last());

  QRegion selectedRegion = visualRegionForSelection(selection);
  selectedRegion.translate(3,3);

  QRect selectedRect = selectedRegion.boundingRect();
  QPixmap pixmap(selectedRect.size());
  pixmap.fill(Qt::transparent);

  
  foreach (QListWidgetItem *popItem, selectedItems())
  {
  //QListWidgetItem *item = currentItem();
    QString popName = popItem->data(Qt::DisplayRole).toString();
    int popIdx = popItem->data(Qt::UserRole).toInt();
    
    QRect rect(visualItemRect(popItem));
    rect.adjust(3, 3, 3, 3);
    render(&pixmap, rect.topLeft() - selectedRect.topLeft(), QRegion(rect));
  
    dataStream << popName << popIdx;
  }
  
  QMimeData *mimeData = new QMimeData;
  mimeData->setData("text/x-popart-pop", popData);
  
  
  
  //qDebug() << "horizontal offset: " << horizontalOffset() << " vertical offset: " << verticalOffset();

  
  //QPixmap pixmap = d->renderToPixmap(index, &rect);
  //rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);
  
  QDrag *drag = new QDrag(this);
  drag->setPixmap(pixmap);
  drag->setMimeData(mimeData);
  drag->setHotSpot(_mousePressed - selectedRect.topLeft());
    
  if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
  {
    foreach (QListWidgetItem *popItem, selectedItems())
      delete takeItem(row(popItem));
  }
}

void PopulationListWidget::setSelectedGroup(QAction *action) 
{
  _selectedGroup = action->text(); 
  
  emit groupSelected(_selectedGroup);
}

/*PopGroupWidget::PopGroupWidget(QWidget *parent)
 : QTreeWidget(parent)
{
  qDebug() << "dragdropmode: " << dragDropMode() << " acceptdrops: " << acceptDrops() << " defaultdropaction: " << defaultDropAction() << " showdropindicator: " << showDropIndicator() << " supportedDropActions: " << supportedDropActions() << " dragEnabled: " << dragEnabled(); 
}*/

void PopGroupWidget::contextMenuEvent(QContextMenuEvent *event)
{
  //qDebug() << "got context menu event from PopGroupWidget";
  

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

void PopGroupWidget::dragEnterEvent(QDragEnterEvent *event)
{
  
  if (event->mimeData()->hasFormat("text/x-popart-pop"))
  {
    event->accept();
    //qDebug() << "drag enter from list";
  }
  else
  {
    QTreeWidget::dragEnterEvent(event);
    
    //qDebug() << "drag enter from within tree.";
  }
    //event->ignore();
    
  
}

void PopGroupWidget::dragMoveEvent(QDragMoveEvent *event)
{
  
  if (event->mimeData()->hasFormat("text/x-popart-pop"))
  {
    event->setDropAction(Qt::MoveAction);
    event->accept();
    //QTreeWidget::dragMoveEvent(event);
    //qDebug() << "drag move from list.";
  }
    
  else
  {
    QTreeWidget::dragMoveEvent(event);
    //qDebug() << "drag move from within tree.";
  }
  
}

/*void PopGroupWidget::dragLeaveEvent(QDragLeaveEvent *event)
{}*/

void PopGroupWidget::dropEvent(QDropEvent *event)
{
  
  QTreeWidget::dropEvent(event);
  //qDebug() << "got a drop event.";
  
  if (event->mimeData()->hasFormat("text/x-popart-pop"))
  {
    //qDebug() << "dropping from list.";
    
    QTreeWidgetItem *targetItem = itemAt(event->pos());
    if (targetItem)
    {
      if (targetItem->parent())
        targetItem = targetItem->parent();
      
      //targetItem->setExpanded(true);

      
      QByteArray itemData = event->mimeData()->data("text/x-popart-pop");
      QDataStream dataStream(&itemData, QIODevice::ReadOnly);
      
      while (! dataStream.atEnd())
      { 
        QString popName;
        int popIdx;
        dataStream >> popName;
        dataStream >> popIdx;
        
        QTreeWidgetItem *popChild = new QTreeWidgetItem(targetItem, QStringList(popName));
        popChild->setFlags(popChild->flags() & ~Qt::ItemIsDropEnabled);
        popChild->setData(0, Qt::UserRole, popIdx);
        
        //qDebug() << "got pop name: " << popName << " and index: " << popIdx; 
      }
      
      event->setDropAction(Qt::MoveAction);
      event->accept();
    }
    
    else // no target item
      QTreeWidget::dropEvent(event);
  }
  
  else
  {
    //qDebug() << "dropping from within tree.";
    QTreeWidget::dropEvent(event);
  }

  //qDebug() << "data: " << event->mimeData()->data();
  //qDebug() << "text: " << event->mimeData()->formats();
  
  //foreach (QString format, event->mimeData()->formats())
  //{
    //qDebug() << "format: " << format;
    //qDebug() << "data: " << event->mimeData()->data(format);
  //}
  
}

void PopGroupWidget::deleteSelectedGroups()
{
  _deassignedPops.clear();
  
  foreach (QTreeWidgetItem *groupItem, selectedItems())
  {
    //qDebug() << "child count: " << groupItem->childCount();
    for (int j = 0; j < groupItem->childCount(); j++)
    {
      QTreeWidgetItem *popItem = groupItem->child(j);
      QString popName = popItem->data(0, Qt::DisplayRole).toString();
      int traitIdx = popItem->data(0, Qt::UserRole).toInt();
      QPair<QString,int> popData(popName, traitIdx);
      
      //qDebug() << "deleting pop " << popName << " from group " << groupItem->data(0, Qt::DisplayRole);
      
      _deassignedPops << popData;
      
      //delete popItem;
    }
    
    //takeTopLevelItem(
    QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
    delete groupItem;
    
    emit groupDeleted(groupName);
  }
  
  //qDebug() << "populations to deassign:";
  
  /*QList<QPair<QString,int> >::iterator dataIter = _deassignedPops.begin();
  
  while (dataIter != _deassignedPops.end())
  {
    qDebug() << (*dataIter).first;
    ++dataIter;
  }*/
  
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


