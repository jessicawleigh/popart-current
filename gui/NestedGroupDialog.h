#ifndef NESTEDGROUPDIALOG_H
#define NESTEDGROUPDIALOG_H


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
  
  QListWidget *_unassignedView;
  QTreeWidget *_groupView;
  QLineEdit *_addGroupEdit;
  
  //QMap<QString, QList<Trait*> > _groups;
  //QSet<Trait*> _unassignedPops;
  QVector<Trait*> _populations;

private slots:
  void addGroup();
  
};

#endif
