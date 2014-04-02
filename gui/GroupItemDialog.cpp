#include "GroupItemDialog.h"
#include "XPM.h"

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

#include <QDebug>

using namespace std;
//const QIcon GroupedTreeWidgetItem::_lockedIcon = QIcon(QPixmap(xpm::lock));
auto_ptr<QIcon> GroupedTreeWidgetItem::_lockedIcon(new QIcon);

GroupItemDialog::GroupItemDialog(const QVector<QString> &items, QMap<QString,QList<QString> > &groupedItems, const QString & listText, const QString & treeText, QWidget *parent, Qt::WindowFlags flags)
 : QDialog(parent, flags), _items(items), _groupedItems(groupedItems)
{
   
  //_groupedItems = groupedItems;
    
  _unassignedView = new UnsortedListWidget(this);
  _unassignedView->setDragEnabled(true);
  //setPopulations();
  _unassignedView->setSelectionMode(QAbstractItemView::ExtendedSelection); // Maybe MultiSelection?
  connect(_unassignedView, SIGNAL(groupSelected(const QString &)), this, SLOT(addSelectedItemsToGroup(const QString &)));
  
  _groupView = new GroupedTreeWidget(this);
  _groupView->setHeaderHidden(true);
  _groupView->setItemsExpandable(false);
  _groupView->setColumnCount(1);
  _groupView->setDragEnabled(true);
  _groupView->setAcceptDrops(true);
  _groupView->setDragDropMode(QAbstractItemView::DragDrop);
  
  QTreeWidgetItem *root =  _groupView->invisibleRootItem();
  root->setFlags(root->flags() & ~Qt::ItemIsDropEnabled);
  
  setItemContent();
  
  connect(_groupView, SIGNAL(itemsRemoved(const QList<QPair<QString, int> > &)), this, SLOT(deassignItems(const QList<QPair<QString, int> > &)));
  connect(_groupView, SIGNAL(groupDeleted(QString)), _unassignedView, SLOT(removeGroup(QString)));
  connect(_groupView, SIGNAL(groupLocked(QString)), _unassignedView, SLOT(removeGroup(QString)));
  connect(_groupView, SIGNAL(groupUnlocked(QString)), _unassignedView, SLOT(addGroup(QString)));
  
   
  QVBoxLayout *outerLayout = new QVBoxLayout(this);
  QHBoxLayout *mainLayout = new QHBoxLayout;
  
  QVBoxLayout *listLayout = new QVBoxLayout;
  listLayout->addWidget(new QLabel(QString("<b>%1</b>").arg(listText), this));
  listLayout->addWidget(_unassignedView);
  mainLayout->addLayout(listLayout);
  
  QVBoxLayout *treeLayout = new QVBoxLayout;
  treeLayout->addWidget(new QLabel(QString("<b>%1</b>").arg(treeText), this));
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

void GroupItemDialog::addGroup()
{
  QString groupName = _addGroupEdit->text();
  _addGroupEdit->clear();
  
  if (! groupName.isEmpty())
  {
    if (_groupView->findItems(groupName, Qt::MatchExactly).isEmpty())
    {
      QTreeWidgetItem *groupItem = new GroupedTreeWidgetItem(QStringList(groupName));
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


QMap<QString, QList<QString> > GroupItemDialog::groups() const 
{
  /*QMap<QString, QList<Trait *> > popMap;
  
  for (int i = 0; i < _groupView->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *groupItem = _groupView->topLevelItem(i);
    QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
    QList<Trait*> pops;
    for (int j = 0; j < groupItem->childCount(); j++)
    {
      QTreeWidgetItem *popItem = groupItem->child(j);
      int traitIdx = popItem->data(0, Qt::UserRole).toInt();
      pops << _items.at(traitIdx);
    }
    
    popMap.insert(groupName, pops);
  }
  
  return popMap;*/
  
  return _groupedItems;
}


void GroupItemDialog::setItemContent()
{
  // check that items in groups appear in items list
  QMap<QString, int> unassignedItems;
  for (int i = 0; i < _items.size(); i++)
    unassignedItems.insert(_items.at(i), i);
  
  QMap<QString, QList<QString> >::const_iterator groupIt = _groupedItems.constBegin();
  
  while (groupIt != _groupedItems.constEnd())
  {  
    QString groupName = groupIt.key();

    GroupedTreeWidgetItem *groupItem = new GroupedTreeWidgetItem(QStringList(groupName));
    groupItem->setFlags(groupItem->flags() & ~Qt::ItemIsDragEnabled);
    _groupView->addTopLevelItem(groupItem); 
    
    _unassignedView->addGroup(groupName);
    groupItem->setExpanded(true);
    
    foreach (QString itemName, groupIt.value())
    {
      //itemIdx = unassignedItems.value(itemName);
      QMap<QString,int>::iterator itemIt = unassignedItems.find(itemName);
      
      if (itemIt == unassignedItems.end())
      {
        QMessageBox::critical(this, "<b>Unknown Item</b>", QString("Item %1 in group %2 does not exist, or has already been assigned to another group.").arg(itemName).arg(groupName));
        reject();
      }
      
      else
      {
        int itemIdx = itemIt.value();
        unassignedItems.erase(itemIt);

        QStringList itemData(itemName);
      
        GroupedTreeWidgetItem *childItem = new GroupedTreeWidgetItem(groupItem, itemData);
        childItem->setFlags(childItem->flags() & ~Qt::ItemIsDropEnabled);
        childItem->setData(0, Qt::UserRole, itemIdx);
      }
    }

    
    ++groupIt;
  }
  
  QMap<QString,int>::const_iterator itemIt = unassignedItems.constBegin();
  
  while (itemIt != unassignedItems.constEnd())
  {
    QListWidgetItem *unassignedItem = new QListWidgetItem(itemIt.key());
    unassignedItem->setData(Qt::UserRole, itemIt.value());
    _unassignedView->addItem(unassignedItem);
    
    ++itemIt;
  }
  
  /*for (int i = 0; i < _items.size(); i++)
  {
    Trait *t = _items.at(i);
    QListWidgetItem *popItem = new QListWidgetItem(QString::fromStdString(t->name()));
    popItem->setData(Qt::UserRole, i);
    _unassignedView->addItem(popItem);
  }*/
}

void GroupItemDialog::addSelectedItemsToGroup(const QString &group)
{
  // There should be only one item
  foreach (QTreeWidgetItem *qItem, _groupView->findItems(group, Qt::MatchExactly))
  {    
    GroupedTreeWidgetItem *groupItem = dynamic_cast<GroupedTreeWidgetItem *>(qItem);
    
    foreach(QListWidgetItem *listItem, _unassignedView->selectedItems())
    {
      QStringList itemData;
      itemData << listItem->data(Qt::DisplayRole).toString();
      GroupedTreeWidgetItem *childItem = new GroupedTreeWidgetItem(groupItem, itemData);
      childItem->setFlags(listItem->flags() & ~Qt::ItemIsDropEnabled);
      childItem->setData(0, Qt::UserRole, listItem->data(Qt::UserRole));
      
      delete listItem;
    }  
  }
}

void GroupItemDialog::deassignItems(const QList<QPair<QString,int> > & itemData)
{
  QList<QPair<QString,int> >::const_iterator dataIter = itemData.constBegin();
  
  while (dataIter != itemData.constEnd())
  {
    QPair<QString,int> datum = *dataIter;
    QListWidgetItem *item = new QListWidgetItem(datum.first);
    item->setData(Qt::UserRole, datum.second);
    _unassignedView->addItem(item);
    
    ++dataIter;
  }
}

void GroupItemDialog::checkAndAccept()
{
  
  // TODO allow unassigned items, but ask if they should be assigned to an "Unlabeled" group (or unlabeled_X if "Unlabeled" exists
  if (_unassignedView->count() > 0)
    QMessageBox::warning(this, "<b>Items Not Assigned</b>", "All items must be assigned to groups before clicking OK.");
  else
  {
    _groupedItems.clear();
    
    for (int i = 0; i < _groupView->topLevelItemCount(); i++)
    {
      QTreeWidgetItem *groupItem = _groupView->topLevelItem(i);
      QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
      QList<QString> itemList;
      for (int j = 0; j < groupItem->childCount(); j++)
      {
        QTreeWidgetItem *childItem = groupItem->child(j);
        int idx = childItem->data(0, Qt::UserRole).toInt();
        itemList << _items.at(idx);
      }
      
      _groupedItems.insert(groupName, itemList);
    }
    
    accept();
  }
}


void UnsortedListWidget::removeGroup(QString groupName)
{
  
  // TODO add more error control here.
  int idx = _groupNames.indexOf(groupName);
  
  if (idx >= 0)
    _groupNames.removeAt(idx);
}

void UnsortedListWidget::contextMenuEvent(QContextMenuEvent *event)
{ 
  QMenu menu;
  
  if (selectedItems().isEmpty())
  {
    QAction *emptyAct = menu.addAction("(No items selected)");
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

void UnsortedListWidget::mousePressEvent(QMouseEvent *event)
{
  
  _mousePressed = event->pos();
  
  QListWidget::mousePressEvent(event);
}

void UnsortedListWidget::startDrag(Qt::DropActions)// supportedActions)
{
  QModelIndexList indices = selectedIndexes();
  QItemSelection selection(indices.first(), indices.last());

  QRegion selectedRegion = visualRegionForSelection(selection);
  selectedRegion.translate(3,3);

  QRect selectedRect = selectedRegion.boundingRect();
  QPixmap pixmap(selectedRect.size());
  pixmap.fill(Qt::transparent);

  
  foreach (QListWidgetItem *listItem, selectedItems())
  {
    QRect rect(visualItemRect(listItem));
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
    foreach (QListWidgetItem *item, selectedItems())
      delete takeItem(row(item));
  }
}

QStringList UnsortedListWidget::mimeTypes() const
{  
  QStringList types = QListWidget::mimeTypes();
  types << "text/x-popart-pop";
  
  return types;
}

QMimeData* UnsortedListWidget::mimeData(const QList<QListWidgetItem *> items) const
{
  QByteArray itemData;
  QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  
  foreach (QListWidgetItem *listItem, items)
  {
    QString itemName = listItem->data(Qt::DisplayRole).toString();
    int itemIdx = listItem->data(Qt::UserRole).toInt();
      
    dataStream << itemName << itemIdx;
  }
  
  QMimeData *mimeData = new QMimeData;
  mimeData->setData("text/x-popart-pop", itemData);
  
  return mimeData;
}


void UnsortedListWidget::setSelectedGroup(QAction *action) 
{
  _selectedGroup = action->text(); 
  
  emit groupSelected(_selectedGroup);
}

GroupedTreeWidget::GroupedTreeWidget(QWidget *parent)
 : QTreeWidget(parent)
{
  setDropIndicatorShown(true);
    /*QStyledItemDelegate *delegate = dynamic_cast<QStyledItemDelegate *>(_groupView->itemDelegate());
  if (! delegate)
    qDebug() << "not a QStyledItemDelegate!";*/
}

void GroupedTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{  
  QMenu menu;
  
  if (selectedItems().isEmpty())
  {
    QAction *emptyAct = menu.addAction("(No groups selected)");
    emptyAct->setEnabled(false);    
  }
  

  else if (selectedItems().first()->parent())
  {
    GroupedTreeWidgetItem *parent = dynamic_cast<GroupedTreeWidgetItem *>(selectedItems().first()->parent());

    QAction *deassignItemAct = menu.addAction("Remove item from group");
    connect(deassignItemAct, SIGNAL(triggered()), this, SLOT(deassignSelectedItem()));
    
    if (parent->isLocked())
    {
      //deassignItemAct->setText("(Group is locked)");
      deassignItemAct->setEnabled(false);
    }
  }
  
  else
  {
    QAction *deleteGroupsAct = menu.addAction("Delete group");
    connect(deleteGroupsAct, SIGNAL(triggered()), this, SLOT(deleteSelectedGroups()));
    
    QAction *lockGroupAct = menu.addAction("Lock group");
    connect(lockGroupAct, SIGNAL(triggered()), this, SLOT(toggleLockSelectedGroup()));

    GroupedTreeWidgetItem *item = dynamic_cast<GroupedTreeWidgetItem *>(selectedItems().first());
    
    if (item->isLocked())//->isDisabled())
    {
      //deleteGroupsAct->setText("(Group is locked)");
      deleteGroupsAct->setEnabled(false);
      lockGroupAct->setText("Unlock group");
    }
    
    
  }
  
  menu.exec(event->globalPos());
}

void GroupedTreeWidget::mousePressEvent(QMouseEvent *event)
{
  _mousePressed = event->pos();
  
  QTreeWidget::mousePressEvent(event);
}

QStringList GroupedTreeWidget::mimeTypes() const
{  
  QStringList types = QTreeWidget::mimeTypes();
  types << "text/x-popart-pop";
  
  return types;
}

QMimeData * GroupedTreeWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{
  QByteArray itemData;
  QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  
  foreach (QTreeWidgetItem *treeItem, items)
  {
    QString itemName = treeItem->data(0, Qt::DisplayRole).toString();
    int itemIdx = treeItem->data(0, Qt::UserRole).toInt();
      
    dataStream << itemName << itemIdx;
  }
  
  QMimeData *mimeData = new QMimeData;
  mimeData->setData("text/x-popart-pop", itemData);
  
  return mimeData;  
}

bool GroupedTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
  bool dropSuccess = false;
  
  qDebug() << "in GroupTreeWidget::dropMimeData";
  
  if (action == Qt::MoveAction && data->hasFormat("text/x-popart-pop"))
  {
    if (parent && (parent->flags() & Qt::ItemIsDropEnabled))
    {
      QByteArray itemData = data->data("text/x-popart-pop");
      QDataStream dataStream(&itemData, QIODevice::ReadOnly);
      
      while (! dataStream.atEnd())
      { 
        QString itemName;
        int itemIdx;
        dataStream >> itemName;
        dataStream >> itemIdx;
        
        GroupedTreeWidgetItem *groupItem = dynamic_cast<GroupedTreeWidgetItem *>(parent);
        
        QTreeWidgetItem *childItem = new GroupedTreeWidgetItem(groupItem, QStringList(itemName));
        childItem->setFlags(childItem->flags() & ~Qt::ItemIsDropEnabled);
        childItem->setData(0, Qt::UserRole, itemIdx); 
        
      } 
      
      dropSuccess = true;
    }
  }
  
  else
    dropSuccess = QTreeWidget::dropMimeData(parent, index, data, action);
  
  return dropSuccess;
}

void GroupedTreeWidget::startDrag(Qt::DropActions)
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

void GroupedTreeWidget::deleteSelectedGroups()
{
  _deassignedItems.clear();
  
  foreach (QTreeWidgetItem *groupItem, selectedItems())
  {
    for (int j = 0; j < groupItem->childCount(); j++)
    {
      QTreeWidgetItem *childItem = groupItem->child(j);
      QString itemName = childItem->data(0, Qt::DisplayRole).toString();
      int itemIdx = childItem->data(0, Qt::UserRole).toInt();
      QPair<QString,int> itemData(itemName, itemIdx);      
      
      _deassignedItems << itemData;
      
    }
    
    QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
    delete groupItem;
    
    emit groupDeleted(groupName);
  }
    
  emit itemsRemoved(_deassignedItems);
}

/*bool GroupedTreeWidget::isLocked(QTreeWidgetItem *groupItem) const
{
  return groupItem->checkState(0) == Qt::Checked;
}*/

void GroupedTreeWidget::toggleLockSelectedGroup()
{
  GroupedTreeWidgetItem *groupItem = dynamic_cast<GroupedTreeWidgetItem *>(selectedItems().first());  
  
  QString groupName = groupItem->data(0, Qt::DisplayRole).toString();
  
  if (groupItem->isLocked())
  {
    for (int i = 0; i < groupItem->childCount(); i++)
    {
      QTreeWidgetItem *childItem = groupItem->child(i);
      childItem->setFlags(childItem->flags() | Qt::ItemIsDragEnabled);
    }
    groupItem->setFlags((groupItem->flags() | Qt::ItemIsDropEnabled) & ~Qt::ItemIsUserCheckable);
    groupItem->setLocked(false);//setDisabled(false);
    
    emit(groupUnlocked(groupName));
  }
  
  else
  {
    for (int i = 0; i < groupItem->childCount(); i++)
    {
      QTreeWidgetItem *childItem = groupItem->child(i);
      childItem->setFlags(childItem->flags() & ~Qt::ItemIsDragEnabled);
    }
    groupItem->setFlags(groupItem->flags() & ~Qt::ItemIsDropEnabled & ~Qt::ItemIsUserCheckable);
    groupItem->setLocked(true);//setDisabled(true);
    
    emit(groupLocked(groupName));
  }
}

void GroupedTreeWidget::deassignSelectedItem()
{  
  _deassignedItems.clear();
  
  QTreeWidgetItem *treeItem = selectedItems().first();
  QString itemName = treeItem->data(0, Qt::DisplayRole).toString();
  int itemIdx = treeItem->data(0, Qt::UserRole).toInt();
  QPair<QString,int> itemData(itemName, itemIdx);
  _deassignedItems << itemData;

  delete treeItem;
  
  emit itemsRemoved(_deassignedItems);

}

GroupedTreeWidgetItem::GroupedTreeWidgetItem(const QStringList &strings)
 : QTreeWidgetItem(strings, QTreeWidgetItem::UserType), _locked(false)
{}

GroupedTreeWidgetItem::GroupedTreeWidgetItem(GroupedTreeWidget *parent, const QStringList &strings)
 : QTreeWidgetItem(parent, strings, QTreeWidgetItem::UserType), _locked(false)
{}

GroupedTreeWidgetItem::GroupedTreeWidgetItem(GroupedTreeWidgetItem *parent, const QStringList &strings)
 : QTreeWidgetItem(parent, strings, QTreeWidgetItem::UserType), _locked(false)
{}

bool GroupedTreeWidgetItem::isLocked() const
{
  return _locked;
}

void GroupedTreeWidgetItem::setLocked(bool lock)
{
  //QIcon *ptr = _lockedIcon.get();
  // create the static lock icon the first time it's used
  if (_lockedIcon.get()->isNull())
    *_lockedIcon.get() = QIcon(QPixmap(xpm::lock));

  _locked = lock;
  
  if (lock)
  {
    setIcon(0, *_lockedIcon.get());
  }
  
  else
    setIcon(0, QIcon());
}
