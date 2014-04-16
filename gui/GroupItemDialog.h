#ifndef GROUPITEMDIALOG_H
#define GROUPITEMDIALOG_H

#include <QAction>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDialog>
#include <QIcon>
#include <QLineEdit>
#include <QListWidget>
#include <QList>
#include <QMap>
#include <QMouseEvent>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QVector>

#include <memory>

class UnsortedListWidget;
class GroupedTreeWidget;

class GroupItemDialog : public QDialog
{
  Q_OBJECT

public:
  // At some point make a static function  
  //static ColourTheme::Theme getColour(QWidget *, ColourTheme::Theme, Qt::WindowFlags = 0, bool * = 0, bool * = 0);
  // and make the constructor private
  
  GroupItemDialog(const QVector<QString> &, QMap<QString, QList<QString> > &, const QString & = "Unsorted Items", const QString & = "Grouped Items", QWidget * = 0, Qt::WindowFlags = 0);
  
  QMap<QString, QList<QString> > groups() const;
  
  
private:
  
  void setItemContent();
  
  UnsortedListWidget *_unassignedView;
  GroupedTreeWidget *_groupView;
  QLineEdit *_addGroupEdit;
  
  //QMap<QString, QList<Trait*> > _groups;
  //QSet<Trait*> _unassignedPops;
  QVector<QString> _items;
  QMap<QString, QList<QString> > &_groupedItems;

private slots:
  void checkAndAccept();
  void addGroup();
  void addSelectedItemsToGroup(const QString &);
  void deassignItems(const QList<QPair<QString,int> > &);
  //void checkAndRenameGroup(QTreeWidgetItem *, int);
  
};


// Would like this to be a nested or somehow private class; otherwise, dump into a new file?
class UnsortedListWidget : public QListWidget
{
  Q_OBJECT
public:
  UnsortedListWidget(QWidget * parent = 0) : QListWidget(parent) {};
  const QString & selectedGroup() const { return _selectedGroup; };
public slots:
  void addGroup(const QString &group) { _groupNames << group; };
  void removeGroup(QString);
  void renameGroup(QString, QString);
  
protected:
  virtual void contextMenuEvent(QContextMenuEvent *);
  //virtual void dragLeaveEvent(QDragLeaveEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void startDrag(Qt::DropActions);
  virtual QStringList mimeTypes() const;
  virtual QMimeData * mimeData(const QList<QListWidgetItem *>) const;
  
private:
  QPoint _mousePressed;
  QString _selectedGroup;
  QStringList _groupNames;
  
private slots:
  void setSelectedGroup(QAction *);
signals:
  void groupSelected(const QString &);
};

class GroupedTreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  GroupedTreeWidget(QWidget * = 0);
  
protected:
  virtual void contextMenuEvent(QContextMenuEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual QStringList mimeTypes() const;
  virtual QMimeData * mimeData(const QList<QTreeWidgetItem *>) const;
  virtual bool dropMimeData(QTreeWidgetItem *, int, const QMimeData *, Qt::DropAction);
  virtual void startDrag(Qt::DropActions);
  //virtual Qt::DropActions supportedDropActions() const { return Qt::MoveAction; };
  /*virtual void dragEnterEvent(QDragEnterEvent *);
  //virtual void dragLeaveEvent(QDragLeaveEvent *);
  virtual void dragMoveEvent(QDragMoveEvent *);
  virtual void dropEvent(QDropEvent *);*/
  
private:
  //bool isLocked(QTreeWidgetItem *) const;

  QList<QPair<QString,int> > _deassignedItems;
  QPoint _mousePressed;
  //QVector<bool> _lockedGroups;
  
private slots:
  void toggleLockSelectedGroup();
  void deassignSelectedItem();
  void deleteSelectedGroups();
  void renameGroup(QTreeWidgetItem *, int);
    
signals:
  void itemsRemoved(const QList<QPair<QString,int> > &);
  void groupDeleted(QString);
  void groupLocked(QString);
  void groupUnlocked(QString);
  void groupNameChanged(QString, QString);
};


class GroupedTreeWidgetItem : public QTreeWidgetItem
{
public:
  GroupedTreeWidgetItem(const QStringList &);
  GroupedTreeWidgetItem(GroupedTreeWidget *, const QStringList &);
  GroupedTreeWidgetItem(GroupedTreeWidgetItem *, const QStringList &);
  
  bool isLocked() const;
  void setLocked(bool);
  QString oldText() const;
  void setOldText(const QString &);
  void updateOldText();
  //void unlock();
  
private:
  //void constructItem();
  bool _locked;
  static std::auto_ptr<QIcon> _lockedIcon;// = 0; 
  QString _oldText;
};


#endif
