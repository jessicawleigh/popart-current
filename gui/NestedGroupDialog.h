#ifndef NESTEDGROUPDIALOG_H
#define NESTEDGROUPDIALOG_H

#include <QAction>
#include <QContextMenuEvent>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QVector>

#include <vector>

#include "Trait.h"

class PopulationListWidget;

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
  QTreeWidget *_groupView;
  QLineEdit *_addGroupEdit;
  
  //QMap<QString, QList<Trait*> > _groups;
  //QSet<Trait*> _unassignedPops;
  QVector<Trait*> _populations;

private slots:
  void addGroup();
  void addSelectedPopsToGroup(const QString &);
  
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
private:
  QString _selectedGroup;
  QStringList _groupNames;
  
private slots:
  void setSelectedGroup(QAction *);
signals:
  void groupSelected(const QString &);
};


#endif
