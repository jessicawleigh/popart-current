#include <QHeaderView>
#include <QModelIndex>
#include <QScrollBar>
#include <QRegion>
#include <QRect>
#include <QList>
#include <iostream>
using namespace std;

#include "AlignmentView.h"


AlignmentView::AlignmentView(QWidget *parent)
  : QTableView(parent)
{
  _delegate = new AlignmentDelegate();
  _maskHidden = false;
  //_startPos = QPointF(-1, -1);
  setItemDelegate(_delegate);
  setShowGrid(false);

  QHeaderView *vheader = verticalHeader();
  vheader->setSectionResizeMode(QHeaderView::Fixed);
  vheader->setDefaultSectionSize(19);
  vheader->setSectionsMovable(true);
  //connect(vheader, SIGNAL(sectionMoved(int, int, int)), this, SLOT(moveSequence(int, int, int)));

  QHeaderView *hheader = horizontalHeader();
  hheader->setSectionResizeMode(QHeaderView::Fixed);
  hheader->setDefaultSectionSize(12);
  
  setSelectionMode(QAbstractItemView::ContiguousSelection);
  
}

void AlignmentView::setModel(QAbstractItemModel *model)
{
  QTableView::setModel(model);
  //QHeaderView *vheader = verticalHeader();
  //QSize size;
  
  /*for (unsigned i = 0; i < model->rowCount(); i++)
  {
    for (unsigned j = 0; j < model->columnCount(); j++)
    {
      size = _delegate.sizeHint(viewOptions(), model->index(i, j));
      if (j == 0)  
        setRowHeight(i, size.height());
      if (i == 0 || columnWidth(j) < size.width())
        setColumnWidth(j, size.width());
    }
  }*/
  if (model)
  {
    connect(model, SIGNAL(columnsInserted(const QModelIndex &, int, int)), this, SLOT(columnsAdded(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(rowsAdded(const QModelIndex &, int, int)));
    connect(model, SIGNAL(characterDeleted(const QModelIndex&)), this, SLOT(characterDelete(const QModelIndex &)));
    connect(model, SIGNAL(characterInserted(const QModelIndex &)), this, SLOT(characterInsert(const QModelIndex &)));
    connect(model, SIGNAL(fetchedTo(const QModelIndex &, const QModelIndex &)), this, SLOT(moveTo(const QModelIndex &, const QModelIndex &)));
    connect(model, SIGNAL(pushedTo(const QModelIndex &, const QModelIndex &)), this, SLOT(moveTo(const QModelIndex &, const QModelIndex &)));
    connect(model, SIGNAL(charTypeChanged(Sequence::CharType)), this, SLOT(changeCharType(Sequence::CharType)));
    connect(model, SIGNAL(maskChanged(int, int)), this, SLOT(changeMask(int, int)));
    connect(model, SIGNAL(rowMaskChanged(int, int)), this, SLOT(changeRowMask(int, int)));
  }
}

void AlignmentView::resizeColumnsToContents()
{
  for (unsigned i = 0; i < model()->columnCount(); i++)
    resizeColumnToContents(i);
}

void AlignmentView::resizeColumnToContents(int index)
{
  QTableView::resizeColumnToContents(index);
  
  setColumnWidth(index, columnWidth(index) - 10);
}

void AlignmentView::resizeRowsToContents()
{
  for (unsigned i = 0; i < model()->rowCount(); i++)
    resizeRowToContents(i);
}

void AlignmentView::resizeRowToContents(int index)
{
  QTableView::resizeRowToContents(index);
  setRowHeight(index, rowHeight(index) - 2);
}

void AlignmentView::columnsAdded(const QModelIndex &parent, int start, int end)
{
  for (int i = start; i <= end; i++)  resizeColumnToContents(i);
}

void AlignmentView::rowsAdded(const QModelIndex &parent, int start, int end)
{
  for (int i = start; i <= end; i++)  resizeRowToContents(i);
}

void AlignmentView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
  QTableView::selectionChanged(selected, deselected);
  updateSelectedRange();
  
  emit newSelection();
}

void AlignmentView::updateSelectedRange()
{
  _range.top() = _range.left() = _range.bottom() = _range.right() = -1;
  int row, col;
    
  QModelIndexList indexes = selectedIndexes();
  
  QModelIndexList::const_iterator iter = indexes.constBegin();
  
  while (iter != indexes.constEnd())
  {
    row = (*iter).row();
    col = (*iter).column();

    if (_range.left() < 0)
    {
      _range.left() = _range.right() = col;
      _range.top() = _range.bottom() = row; 
    }
    
    else
    {
      if (_range.top() > row)  _range.top() = row;
      else if (_range.bottom() < row) _range.bottom() = row;
      
      if (_range.left() > col)  _range.left() = col;
      else if (_range.right() < col)  _range.right() = col;
    }
    
    iter++;
  }
}

SelectionRange AlignmentView::selectedRange() const
{
  return _range;
}

/*void AlignmentView::keyPressEvent(QKeyEvent *event)
{
  QString text = event->text();
  bool ignoreEvent = false;
  
  if (text.length() > 0)
  {
    if (event->key() == Qt::Key_Backspace || text.at(0).isLetter() || text.at(0) == '-')
    {
      ignoreEvent = true;
    }
  }
  
  if (ignoreEvent)  
  {
    event->ignore();
  }
  
  else 
  {
    QTableView::keyPressEvent(event);
  }
}*/

void AlignmentView::mousePressEvent(QMouseEvent *event)
{
  bool dealtWith = false;
  if (event->button() == Qt::LeftButton)
  {
    _startPos = event->pos();
    
    QModelIndex idx = indexAt(_startPos);
    SelectionRange range = selectedRange();
    
    if (idx.column() <= range.right()  && idx.column() >= range.left() && idx.row() <= range.bottom() && idx.row() >= range.top())
    {
      _lastIdx = idx;
      _clickedSelection = true;
    }
    
    else _clickedSelection = false;
    
  }
    
  if (! dealtWith)
    QTableView::mousePressEvent(event);
}

void AlignmentView::mouseMoveEvent(QMouseEvent *event)
{
  if (_clickedSelection)
  {
    QPoint currentPos = event->pos();
    QModelIndex currentIdx = indexAt(currentPos);
    
    if (_lastIdx.column() < currentIdx.column())
    {
      QRect visRect = viewport()->contentsRect();      
      int minRightVis = columnViewportPosition(currentIdx.column()) + 2 * (columnWidth(currentIdx.column()));
      
      if (minRightVis > visRect.x() + visRect.width()) 
      {
        QScrollBar *hbar = horizontalScrollBar();
        int newscrollval = hbar->value() + hbar->singleStep();
        
        if (newscrollval > hbar->maximum())
          hbar->setValue(hbar->maximum());
        else
          hbar->setValue(newscrollval);
      }
      
      emit draggedRight();
      
      _lastIdx = currentIdx;
    }
    
    else if (_lastIdx.column() > currentIdx.column())
    {
      QRect visRect = viewport()->contentsRect();
      int minLeftVis = columnViewportPosition(currentIdx.column()) - 2 * (columnWidth(currentIdx.column()));
      
      if (minLeftVis < 0) 
      {
        QScrollBar *hbar = horizontalScrollBar();
        int newscrollval = hbar->value() - hbar->singleStep();
        
        if (newscrollval < hbar->minimum())
          hbar->setValue(hbar->minimum());
        else
          hbar->setValue(newscrollval);
      }
      
      emit draggedLeft();
      _lastIdx = currentIdx;
    }
    
  }

  else  QTableView::mouseMoveEvent(event);
}

void AlignmentView::mouseReleaseEvent(QMouseEvent *event)
{  
  if (event->pos() == _startPos && _clickedSelection)
    clearSelection();
  
  QTableView::mouseReleaseEvent(event);
}


void AlignmentView::changeCharType(Sequence::CharType chartype)
{
  _delegate->setCharType(chartype);
  
  setDirtyRegion(viewport()->visibleRegion());
}

void AlignmentView::changeMask(int left, int right)
{ 
  bool maskstate;
  
  if (_maskHidden)
  {
    for (int i = left; i <= right; i++)
    {
      maskstate = model()->index(0, i).data(Qt::UserRole).toBool();
      if (maskstate)
        showColumn(i);
      else
        hideColumn(i);
    }
  }
    
  setDirtyRegion(viewport()->visibleRegion());
}

void AlignmentView::changeRowMask(int top, int bottom)
{
  bool maskstate;
  
  if (_maskHidden)
  {
    for (int i = top; i <= bottom; i++)
    {
      maskstate = model()->index(i, 0).data(Qt::UserRole).toBool();
      if (maskstate)
        showRow(i);
      else
        hideRow(i);
    }
  }
    
  setDirtyRegion(viewport()->visibleRegion());
}

void AlignmentView::hideMaskedColumns(bool hide)
{
  if (hide == _maskHidden)
  {
    cerr << "Non-fatal error: attempting to hide masked columns when already hidden." << endl;
  }
  
  bool maskstate;
  
  if (hide)
  {
    for (int i = 0; i < model()->columnCount(); i++)
    {
      maskstate = model()->index(0, i).data(Qt::UserRole).toBool();
      if (!maskstate)
        hideColumn(i);
    }
  }
  
  else
  {
    for (int i = 0; i < model()->columnCount(); i++)
      if (isColumnHidden(i))  showColumn(i);
  }
  
  _maskHidden = hide;
}

void AlignmentView::characterDelete(const QModelIndex &index)
{
  setCurrentIndex(index);
}

void AlignmentView::characterInsert(const QModelIndex &index)
{  
  QModelIndex newidx = model()->index(index.row(), index.column() + 1, index.parent());
  setCurrentIndex(newidx);

  //updateSelectedRange();
  //emit newSelection();
}

void AlignmentView::moveTo(const QModelIndex & topIndex, const QModelIndex & bottomIndex)
{
  setCurrentIndex(model()->index(currentIndex().row(), topIndex.column()));
  
  QItemSelectionModel *selModel = selectionModel();
  QItemSelection sel(topIndex, bottomIndex);
  
  selModel->select(sel, QItemSelectionModel::ClearAndSelect);  
}

void AlignmentView::setAllowNonGapEdits(bool allow)
{
  _allowNonGapEdits = allow;
}


/*void AlignmentView::moveSequence(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
  cout << "in alignmentview::movesequence" << endl;
  emit sequenceMoved(logicalIndex, oldVisualIndex, newVisualIndex);
}*/





