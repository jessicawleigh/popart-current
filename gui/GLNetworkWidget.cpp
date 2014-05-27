#include "GLNetworkWidget.h"

#include <QPushButton>
#include <QDebug>
#include <QSize>

/*
 * Make this a derived QAbstractItemView, can reuse NetworkModel class
 * Connect the same signals to NetworkModel's slots
 * Needs to have the GLNetworkWindow as an instance variable
 * Derive this class to make a TempnetWidget: just reimplement the createLayout function,
 * add some functions to actually build the tempnet. Create a series of 2D network layouts, then add inter-network edges
 * and explicitly set Z as a function of time. 
 * THINK ABOUT INITIAL ROTATION
 * Think also about ghost nodes... no frequency data, but these should be bigger than unsampled nodes. Maybe
 * use existing trait framework, or maybe set up something in GLNetworkWindow that's aware of ghosts
 */



GLNetworkWidget::GLNetworkWidget(QWidget *parent)
 : QScrollArea(parent)
 , _aspectRatio(16.0/9)
 , _window(0)
 , _model(0)
 , _layout(0)
{
  _window = new GLNetworkWindow;
  QWidget *widget = QWidget::createWindowContainer(_window, this);
  //widget->setMinimumHeight(600);
  
  setWidget(widget);
  setWidgetResizable(true);
  //qDebug() << "in scroll area, size: " << size() << "widget's size:" << widget->size() << "window's size:" << window->size();

  connect(_window, SIGNAL(minimumHeightChanged(int)), this, SLOT(setWidgetMinHeight(int)));
  connect(_window, SIGNAL(minimumWidthChanged(int)), this, SLOT(setWidgetMinWidth(int)));
  _window->setMinimumHeight(600);
  //window->resize(600, 600);
  
  _layoutThread = new QThread(this);
  connect(_layoutThread, SIGNAL(finished()), this, SLOT(adjustAndDraw()));
  _progress = new QProgressDialog(this);
  QPushButton *cancelButton = new QPushButton("Can't Cancel", _progress);
  cancelButton->setEnabled(false);
  _progress->setCancelButton(cancelButton);
  _progress->setLabelText("Drawing network...");
  _progress->setMinimum(0);
  _progress->setMaximum(100);
  
  
}

GLNetworkWidget::~GLNetworkWidget()
{
  if (_layout)
    delete (_layout);
}

// TODO fill this in? If inheriting from QAbstractItemView
/*QModelIndex indexAt (const QPoint &point) const
{
}

void GLNetworkWidget::scrollTo(const QModelIndex & index, ScrollHint hint);
QRect GLNetworkWidget::visualRect(const QModelIndex & index) const
int GLNetworkWidget::horizontalOffset () const
QModelIndex GLNetworkWidget::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
QItemSelectionModel::SelectionFlags   GLNetworkWidget::selectionCommand(const QModelIndex & index, const QEvent * event)
void GLNetworkWidget::setSelection(const QRect & rect, QItemSelectionModel::SelectionFlags flags)
int GLNetworkWidget::verticalOffset () const
QRegion GLNetworkWidget::visualRegionForSelection(const QItemSelection & selection) const  */

void GLNetworkWidget::setModel(NetworkModel *model, bool skipLayout)
{
  _model = model;
  
  _defaultIterations = 10 * model->rowCount();
  
  if (skipLayout)
    createEmptyLayout();
  
  else
    createLayout();
  
  connect(model, SIGNAL(traitsUpdated()), this, SLOT(updateTraits()));
}

void GLNetworkWidget::updateTraits()
{
  // TODO update traits in _window
}

void GLNetworkWidget::createLayout(int iterations)
{
  if (_layout)  delete _layout;
  
  double wdth = widget()->width();
  double hght = widget()->height();
    
  _layout = new NetworkLayout(_model, wdth, hght, qMax(wdth, hght));
  
  unsigned maxiter;
  if (iterations > 0)
    maxiter = iterations;
  else
    maxiter = _defaultIterations;
  
  _layout->setMaxIter(maxiter);
  //_layout->optimise();
  //adjustAndDraw();
  

  _progress->show();

  connect(_layout, SIGNAL(progressChanged(int)), _progress, SLOT(setValue(int)));
  connect(_layoutThread, SIGNAL(started()), _layout, SLOT(optimise()));

  _layout->moveToThread(_layoutThread);
  _layoutThread->start();
}

void GLNetworkWidget::createEmptyLayout()
{
  double wdth = widget()->width();
  double hght = widget()->height();
  
  _layout = new NetworkLayout(_model, wdth, hght, qMax(wdth, hght));
  _layout->zeroVertices();
  _window->drawGL();
}

void GLNetworkWidget::adjustAndDraw()
{
  _progress->hide();
  _layout->centreVertices();
  
  // Add margins?
  
  const QVector3D & layoutDim = _layout->southEast3D();
  double maxDim = qMax(qMax(layoutDim.x(), layoutDim.y()), layoutDim.z());
  
  _window->setMinimumHeight(maxDim / _aspectRatio); 
  _window->setMinimumWidth(maxDim);

  _window->setNetworkData((Network::layoutData){.model=_model, .layout=_layout});
}

void GLNetworkWidget::setWidgetMinHeight(int newHeight)
{
  //widget()->resize(widget()->width(), newHeight);
  widget()->setMinimumHeight(newHeight);
  qDebug() << "set height to:" << newHeight;
}

void GLNetworkWidget::setWidgetMinWidth(int newWidth)
{
  
  //widget()->resize(newWidth, widget()->height());
  widget()->setMinimumWidth(newWidth);
  qDebug() << "set width to:" << newWidth;
  
}

