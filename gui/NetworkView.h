/*
 * NetworkView.h
 *
 *  Created on: Mar 20, 2012
 *      Author: jleigh
 */

#ifndef NETWORKVIEW_H_
#define NETWORKVIEW_H_

#include "NetworkLayout.h"
#include "NetworkScene.h"
#include "ColourTheme.h"
#include "Graph.h"
#include "BarchartItem.h"
#include "BorderRectItem.h"
#include "LabelItem.h"
#include "LegendItem.h"
#include "EdgeItem.h"
#include "TaxBoxItem.h"
#include "VertexItem.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QFont>
//#include <QFuture>
//#include <QFutureWatcher>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QKeyEvent>
#include <QPair>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QProgressDialog>
#include <QRectF>
#include <QString>
#include <QThread>
#include <QVector>
//#include <QPersistentModelIndex>

class NetworkView : public QAbstractItemView
{
  Q_OBJECT
 
public:
  NetworkView(QWidget * = 0 );
  virtual ~NetworkView();
  
  //typedef enum {Greyscale, Camo, Pastelle, Vibrant, Spring, Summer, Autumn, Winter}  ColourTheme;
  virtual QModelIndex indexAt(const QPoint &) const;
  virtual void scrollTo(const QModelIndex &, ScrollHint = EnsureVisible);
  virtual void setModel(QAbstractItemModel *, bool=false);
  void clearModel();

  unsigned defaultIterations() const { return _defaultIterations; };
  void setDefaultIterations(unsigned iterations) { _defaultIterations = iterations; };
  virtual QRect visualRect(const QModelIndex &) const;
  static ColourTheme::Theme defaultColourTheme() { return _defaultTheme; };
  
  void setColourTheme(ColourTheme::Theme = _defaultTheme);
  ColourTheme::Theme colourTheme() const { return _currentTheme; };
  const QColor & colour(unsigned) const;
  QList<QColor> traitColours() const;
  void setColour(unsigned, const QColor &);
  const QColor & backgroundColour() const {return _backgroundBrush.color(); };
  void setBackgroundColour(const QColor &);
  const QColor & edgeColour() const { return _edgePen.brush().color(); };
  void setEdgeColour(const QColor &);

  EdgeItem::MutationView edgeMutationView() const { return EdgeItem::mutationView(); };
  void setEdgeMutationView(EdgeItem::MutationView);// { EdgeItem::setMutationView(view); };
  const QColor & vertexColour() const { return _defaultVertBrush.color(); };
  void setVertexColour(const QColor &);
  double vertexSize() const { return _vertRadUnit; };
  void setVertexSize(double);
  QPointF vertexPosition(unsigned) const;
  void setVertexPosition(unsigned, const QPointF &);
  void setVertexPosition(unsigned, double, double);
  QPointF labelPosition(unsigned) const;
  void setLabelPosition(unsigned, const QPointF &);
  void setLabelPosition(unsigned, double, double);
  QRectF sceneRect() const;
  void setSceneRect(const QRectF &);
  void setSceneRect(double, double, double, double);
  QPointF legendPosition() const;
  void setLegendPosition(const QPointF &);
  void setLegendPosition(double, double);

  bool nodeLabelsVisible() const { return _nodeLabelsVisible; };
  void setNodeLabelsVisible(bool);
  
  const QFont & defaultFont() const { return _defaultFont; };
  void setDefaultFont(const QFont & font) { _defaultFont = font; };
  const QFont & smallFont() const { return _smallFont; }
  void setSmallFont(const QFont & font) { _smallFont = font; };
  const QFont & labelFont() const { return _labelFont; }
  void setLabelFont(const QFont &);// { _labelFont = font; };
  void changeLabelFont(const QFont & font);
  const QFont & legendFont() const { return _legendFont; }
  void setLegendFont(const QFont & font) { _legendFont = font; };
  void changeLegendFont(const QFont & font);
  
  // TODO Add a save snapshot function, e.g., http://stackoverflow.com/questions/7451183/how-to-create-image-file-from-qgraphicsscene-qgraphicsview
  void saveSVGFile(const QString &);
  void savePNGFile(const QString &);
  void savePDFFile(const QString &);
  
public slots:
  
  void zoomIn();
  void zoomOut();
  void rotateL();
  void rotateR();
  void selectNodes(const QString &);
  void redraw(unsigned);
  void toggleShowBarcharts(bool);// show) {_showBarcharts = show;};
  void toggleShowTaxBox(bool);
  
protected:
  virtual int horizontalOffset() const;
  virtual bool isIndexHidden(const QModelIndex &) const;
  virtual QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
  virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &, const QEvent * = 0) const;
  virtual void setSelection(const QRect &, QItemSelectionModel::SelectionFlags);
  virtual int verticalOffset() const;
  virtual QRegion visualRegionForSelection(const QItemSelection &) const;
  virtual const QPen & textPen() const { return _textPen; };
  virtual const QBrush & backgroundBrush() const { return _backgroundBrush; } ;
  virtual const QPen & edgePen() const { return _edgePen; };
  virtual const QPen & borderPen() const { return _borderPen; };
  virtual const QPen & outlinePen() const { return _defaultOutlinePen; };
  virtual const QPen & invisiblePen() const { return _transparentPen; };
  virtual const QBrush & vertBrush(int = -1) const;
  virtual void keyPressEvent(QKeyEvent *);
  void clearScene();
  
private:
  
  typedef struct {QGraphicsItem * item; QPointF move;} UndoItem;

  const static unsigned MARGIN = 15;
  const static unsigned PIESEGMENTS = 5760;
  
  virtual void setupDefaultPens();
  void adjustScene();
  virtual void drawLayout();
  void drawLegend();
  virtual void createLayout(int = -1);
  void createEmptyLayout();
  void optimiseLayout();//NetworkLayout*);
  virtual void updateColours();
  QPointF computeLegendPos();
  QRectF computeSceneRect();

  QGraphicsView _theView;
  NetworkScene _theScene;
  NetworkLayout *_layout;
  bool _sceneClear;
    
  QVector<VertexItem *> _vertexItems;
  QVector<QList<QGraphicsEllipseItem *> > _vertexSections;
  QVector<EdgeItem *> _edgeItems;
  QVector<LabelItem *> _labelItems;
  QVector<QBrush> _colourTheme;
  
  /*QVector<QColor> _greyscale;
  QVector<QColor> _camo;
  QVector<QColor> _pastelle;
  QVector<QColor> _vibrant;
  QVector<QColor> _spring;
  QVector<QColor> _summer;
  QVector<QColor> _autumn;
  QVector<QColor> _winter;*/
  
  
  QPen _textPen;
  QBrush _backgroundBrush;
  QBrush _foregroundBrush;
  QBrush _defaultVertBrush;
  QPen _edgePen;
  QPen _borderPen;
  QPen _defaultOutlinePen;
  QPen _transparentPen;
  QFont _defaultFont;
  QFont _smallFont;
  QFont _labelFont;
  QFont _legendFont;
  ColourTheme::Theme _currentTheme;
  static ColourTheme::Theme _defaultTheme;// = Greyscale;
  //QVector<QPersistentModelIndex *> _modelIndices;
  bool _nodeLabelsVisible;
  bool _showBarcharts;
  bool _showTaxBox;
  double _vertRadUnit;
  BarchartItem *_barchart;
  TaxBoxItem *_taxbox;
  QGraphicsRectItem *_legend;
  BorderRectItem *_border;
  int _legendRotation;
  QRectF _graphRect;
  QPair<QGraphicsEllipseItem *, QGraphicsEllipseItem *> _sizeKeys;
  QPair<QGraphicsSimpleTextItem *, QGraphicsSimpleTextItem *> _sizeLabels;
  QVector<QGraphicsEllipseItem *> _legendKeys;
  QVector<QGraphicsSimpleTextItem *> _legendLabels;
  
  bool _mouseDragging;
  QPointF _itemStart;
  QPointF _itemEnd;
  QGraphicsItem *_item;
  //QFuture<void> *_future;
  //QFutureWatcher<void> *_watcher;
  QThread *_layoutThread;

  QProgressDialog *_progress;
  unsigned _defaultIterations;

  
  
  
  
private slots:
  void adjustAndDraw();
  void showTaxBox(QGraphicsItem *);
  void hideTaxBox();
  void showBarchart(QGraphicsItem *);
  void hideBarchart();
  void legendItemClicked(LegendItem *);
  /*void prepareMoveEvent(QGraphicsItem *);
  void captureMoveEvent(const QPointF &);*/
  void handleItemMove(QList<QPair<QGraphicsItem *, QPointF> >);
  void setGrabbableCursor(bool);
  void setGrabbingCursor(bool);
  void setClickableCursor(bool);
  void updateTraits();
  
signals:
  void networkDrawn();
  void itemsMoved(QList<QPair<QGraphicsItem *, QPointF> >);
  void legendItemClicked(int);
  void caughtException(const QString &);
  
};

//void optimiseLayout(NetworkLayout*);
#endif /* NETWORKVIEW_H_ */
