#ifndef NESTEDGROUPDIALOG_H
#define NESTEDGROUPDIALOG_H

#include <QAction>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDialog>
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

#include <vector>

#include "Trait.h"

class PopulationListWidget;
class PopGroupWidget;

class NestedGroupDialog : public QDialog
{
  Q_OBJECT

public:
  // At some point make a static function  
  //static ColourTheme::Theme getColour(QWidget *, ColourTheme::Theme, Qt::WindowFlags = 0, bool * = 0, bool * = 0);
  // and make the constructor private
  
  NestedGroupDialog(const std::vector<Trait*> &, QWidget * = 0, Qt::WindowFlags = 0);
  
  QMap<QString, QList<Trait*> > groups() const;
  
  
private:
  
  void setPopulations();
  
  PopulationListWidget *_unassignedView;
  PopGroupWidget *_groupView;
  QLineEdit *_addGroupEdit;
  
  //QMap<QString, QList<Trait*> > _groups;
  //QSet<Trait*> _unassignedPops;
  QVector<Trait*> _populations;

private slots:
  void addGroup();
  void addSelectedPopsToGroup(const QString &);
  void deassignPopulations(const QList<QPair<QString,int> > &);
  
};


// Would like this to be a nested or somehow private class; otherwise, dump into a new file?
class PopulationListWidget : public QListWidget
{
  Q_OBJECT
public:
  PopulationListWidget(QWidget * parent = 0) : QListWidget(parent) {};
  const QString & selectedGroup() const { return _selectedGroup; };
public slots:
  void addGroup(const QString &group) { _groupNames << group; };

protected:
  virtual void contextMenuEvent(QContextMenuEvent *);
  //virtual void dragLeaveEvent(QDragLeaveEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void startDrag(Qt::DropActions);
  
private:
  QPoint _mousePressed;
  QString _selectedGroup;
  QStringList _groupNames;
  
private slots:
  void setSelectedGroup(QAction *);
signals:
  void groupSelected(const QString &);
};

class PopGroupWidget : public QTreeWidget
{
  Q_OBJECT
public:
  PopGroupWidget(QWidget * parent = 0) : QTreeWidget(parent) {};
  
protected:
  virtual void contextMenuEvent(QContextMenuEvent *);
  virtual void dragEnterEvent(QDragEnterEvent *);
  //virtual void dragLeaveEvent(QDragLeaveEvent *);
  virtual void dragMoveEvent(QDragMoveEvent *);
  virtual void dropEvent(QDropEvent *);
  
private:
  QList<QPair<QString,int> > _deassignedPops;
  
private slots:
  void deassignSelectedPop();
  void deleteSelectedGroups();
    
signals:
  void popsRemoved(const QList<QPair<QString,int> > &);
};


#endif
