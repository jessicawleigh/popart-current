#ifndef GLNETWORKWIDGET_H_
#define GLNETWORKWIDGET_H_
#include <QScrollArea>
//#include <QAbstractItemView>
#include <QModelIndex>
//#include <QPoint>
#include <QProgressDialog>
#include <QThread>

#include "GLNetworkWindow.h"
#include "NetworkLayout.h"
#include "NetworkModel.h"


class GLNetworkWidget : public QScrollArea//QAbstractItemView, private QScrollArea
{
  Q_OBJECT
public:
  GLNetworkWidget(QWidget * = 0);
  ~GLNetworkWidget();
  
  virtual void setModel(NetworkModel *, bool = false);
  
  // Functions reimplemented from QAbstractItemView
  /*virtual QModelIndex indexAt (const QPoint &point) const;
  virtual void    scrollTo ( const QModelIndex & index, ScrollHint hint = EnsureVisible );
  virtual QRect   visualRect ( const QModelIndex & index ) const;*/
  
protected:

  // Functions reimplemented from QAbstractItemView
  /*virtual int    horizontalOffset () const;
  virtual QModelIndex     moveCursor ( CursorAction cursorAction, Qt::KeyboardModifiers modifiers );
  virtual QItemSelectionModel::SelectionFlags   selectionCommand ( const QModelIndex & index, const QEvent * event = 0 );
  virtual void  setSelection ( const QRect & rect, QItemSelectionModel::SelectionFlags flags );
  virtual int   verticalOffset () const;
  virtual QRegion       visualRegionForSelection ( const QItemSelection & selection ) const;*/
private:
  virtual void createLayout(int = -1);
  void createEmptyLayout(); 
  
  double _aspectRatio;
  GLNetworkWindow *_window;
  NetworkModel *_model;
  NetworkLayout *_layout;
  QThread *_layoutThread;
  QProgressDialog *_progress;
  
  unsigned _defaultIterations;
  
private slots:
  void adjustAndDraw();
  void updateTraits();
  void setWidgetMinHeight(int);
  void setWidgetMinWidth(int);
};

#endif
