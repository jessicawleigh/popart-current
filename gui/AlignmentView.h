#ifndef ALIGNMENTVIEW_H
#define ALIGNMENTVIEW_H

#include <QObject>
#include <QWidget>
#include <QTableView>
#include <QKeyEvent>
#include <QMouseEvent>

#include "AlignmentDelegate.h"
#include "SelectionRange.h"

#define NUMCOORDS 4

class AlignmentView : public QTableView
{
  Q_OBJECT
public:
  AlignmentView(QWidget * = 0);
  virtual ~AlignmentView() {};
  virtual void setModel(QAbstractItemModel *);
  virtual void resizeColumnsToContents();
  virtual void resizeColumnToContents(int);
  virtual void resizeRowsToContents();
  virtual void resizeRowToContents(int);
  SelectionRange selectedRange() const;
  void setAllowNonGapEdits(bool);
public slots:
  void hideMaskedColumns(bool);
  
protected slots:
  void selectionChanged (const QItemSelection &, const QItemSelection &);
  void columnsAdded(const QModelIndex &, int, int);
  void rowsAdded(const QModelIndex &, int, int);
  //virtual void keyPressEvent(QKeyEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);
  //virtual void mouseClickEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);
  //virtual void dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
private:
  void updateSelectedRange();
  
  AlignmentDelegate *_delegate;  
  SelectionRange _range;
  bool _maskHidden;
  bool _clickedSelection;
  bool _allowNonGapEdits;
  QPoint _startPos;
  QModelIndex _lastIdx;
  

private slots:
  void characterInsert(const QModelIndex &);
  void characterDelete(const QModelIndex &);
  void moveTo(const QModelIndex &, const QModelIndex &);
  void changeCharType(Sequence::CharType);
  void changeMask(int, int);
  void changeRowMask(int, int);
  //void moveSequence(int, int, int);
  
  //void toto(const QModelIndex &, int, int);
signals:
  void newSelection(); 
  void draggedLeft();
  void draggedRight();
  //void sequenceMoved(int, int, int);
};

#endif
