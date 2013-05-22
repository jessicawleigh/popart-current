#ifndef _HAPNETWINDOW_H
#define _HAPNETWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QClipboard>
#include <QGraphicsItem>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QPair>
#include <QProgressDialog>
#include <QStackedWidget>
#include <QUndoStack>
#include <QTabWidget>
#include <QThread>
#include <QVariant>
#include <QWidget>

#include <vector>

#include "AlignmentView.h"
#include "AlignmentModel.h"
#include "MapView.h"
#include "NetworkView.h"
#include "NetworkModel.h"
#include "Statistics.h"
#include "TraitView.h"
#include "TraitModel.h"
#include "HapNet.h"
#include "Sequence.h"
#include "ParsimonyTree.h"

class HapnetWindow : public QMainWindow
{
  Q_OBJECT
public:
  HapnetWindow(QWidget * = 0, Qt::WindowFlags = 0);
  virtual ~HapnetWindow();

protected:
  //virtual void keyPressEvent(QKeyEvent *);
  
private:
  
  typedef enum {Msn, Mjn, Tcs, Tsw, Inj, Apn} HapnetType;
  typedef enum {Net, Map} ViewType;
  void setupActions();
  void setupMenus();
  void setupTools();
  void doStatsSetup();
  bool loadAlignmentFromFile(QString = QString());
  bool loadTreesFromParser(vector<ParsimonyTree *> &);
  bool loadTraitsFromParser();
  void inferNetwork(HapnetType, QVariant = QVariant());
  virtual void resizeEvent(QResizeEvent *);
  
  const bool _debug;
  QString _msgText;
  QString _msgDetail;
  bool _success;
  bool _hapnetRunning;
  ViewType _view;
  bool _mapTraitsSet;
  HapNet * _g;
  Sequence::CharType _datatype;
  QClipboard *_clipboard;
  QUndoStack *_history;
  QThread *_netThread;
  //QThread *_drawThread;
  QThread *_statThread;
  QProgressDialog *_progress;
  QStackedWidget *_centralContainer;
  NetworkView *_netView;
  MapView *_mapView;
  NetworkModel *_netModel;
  AlignmentModel *_alModel;
  AlignmentView *_alView;
  TraitModel *_tModel;
  TraitView *_tView;
  QTabWidget *_dataWidget;
  QMenu *_networkMenu;
  QMenu *_viewMenu;
  QMenu *_statsMenu;
  
  std::vector<Sequence*> _alignment;//*_alignment;
  std::vector<Sequence*> _goodSeqs;
  std::vector<bool> _mask;
  std::vector<unsigned> _badSeqs;
  std::vector<Trait *> _traitVect;
  std::vector<ParsimonyTree *> _treeVect;
  Statistics *_stats;
  
  QAction *_openAct;
  QAction *_closeAct;
  QAction *_importAlignAct;
  QAction *_importTraitsAct;
  QAction *_importGeoTagsAct;
  QAction *_exportAct;
  QAction *_saveGraphicsAct;
  QAction *_quitAct;
  
  QAction *_undoAct;
  QAction *_redoAct;
  QAction *_traitColourAct;
  QAction *_vertexColourAct;
  QAction *_vertexSizeAct;
  QAction *_edgeColourAct;
  QAction *_backgroundColourAct;
  QAction *_labelFontAct;
  QAction *_legendFontAct;
  QAction *_redrawAct;
  
  QAction *_msnAct;
  QAction *_mjnAct;
  QAction *_apnAct;
  QAction *_injAct;
  QAction *_tswAct;
  QAction *_tcsAct;
  QAction *_umpAct;
  
  QAction *_toggleViewAct;

  QAction *_dashViewAct;
  QAction *_nodeViewAct;
  QAction *_numViewAct;

  QAction *_identicalAct;
  QAction *_ntDiversityAct;
  QAction *_nSegSitesAct;
  QAction *_nParsimonyAct;
  QAction *_tajimaAct;
  QAction *_allStatsAct;

  QAction *_nexusToolAct;
  QAction *_exportToolAct;
  QAction *_colourAct;
  QAction *_zoomInAct;
  QAction *_zoomOutAct;
  QAction *_rotateLAct;
  QAction *_rotateRAct;
  QAction *_searchAct;
  QAction *_barchartAct;
  QAction *_taxBoxAct;
  
  QString _filename;
  
  
private slots:
  void printprog(int);
  void openAlignment();
  void importAlignment();
  void importTraits();
  void importGeoTags();
  void closeAlignment();
  void saveGraphics();
  void exportNetwork();
  void quit();
  void buildMSN();
  void buildMJN();
  void buildAPN();
  void buildIntNJ();
  void buildTCS();
  void buildTSW();
  void buildUMP();
  void displayNetwork();
  void finaliseDisplay();
  void showNetError(const QString &);
  void showIdenticalSeqs();
  void showNucleotideDiversity();
  void showSegSites();
  void showParsimonySites();
  void showTajimaD();
  void showAllStats();
  void search();
  void changeColourTheme(); 
  void changeColour(int);
  void setTraitColour();
  void changeVertexColour();
  void changeVertexSize();
  void changeEdgeColour();
  void changeEdgeMutationView(QAction*);
  void toggleView();
  void changeBackgroundColour();
  void changeLabelFont();
  void changeLegendFont();
  void redrawNetwork();
  void toggleNetActions(bool);
  void toggleAlignmentActions(bool);
  void fixBarchartButton(bool);
  void fixTaxBoxButton(bool);
  void graphicsMove(QList<QPair<QGraphicsItem *, QPointF> >);

  //QPlainTextEdit *_messageConsole;
  
signals:
  void sizeChanged(const QSize&);
  
};

#endif
