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
#include <QTableWidget>
#include <QTabWidget>
#include <QThread>
#include <QVariant>
#include <QWidget>

#include <istream>
#include <vector>

#include "AlignmentView.h"
#include "AlignmentModel.h"
#include "Assistant.h"
#include "CitationDialog.h"
#include "GeoTrait.h"
#include "MapView.h"
#include "NetworkView.h"
#include "NetworkModel.h"
#include "Statistics.h"
#include "TableParser.h"
#include "TraitView.h"
#include "TraitModel.h"
#include "HapNet.h"
#include "Sequence.h"
#include "ParsimonyTree.h"


typedef struct { double alpha; int epsilon; bool useRetTol; } IntNJArg;//QPair<double,int> IntNJArg;
Q_DECLARE_METATYPE(IntNJArg);  

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
  typedef enum {Traits, GeoTags} TraitType;
  void setupActions();
  void setupMenus();
  void setupTools();
  void showErrorDlg(const QString &, const QString & = "", const QString & = "");
  void showWarnDlg(const QString &, const QString & = "", const QString & = "");
  void doStatsSetup();
  bool loadAlignmentFromFile(QString = QString());
  bool loadNetFromParser();
  bool loadNetAttributes();
  bool loadTreesFromParser(vector<ParsimonyTree *> &);
  bool loadTraitsFromParser();
  bool writeNexusFile(ostream &);
  bool writeNexusAlignment(ostream &);
  bool writeNexusTrees(ostream &);
  bool writeNexusNetwork(ostream &);
  bool writeNexusTraits(ostream &);
  bool writeGeoData(ostream &, const vector<Trait *> &);
  bool writeTraitData(ostream &, const vector<Trait *> &);
  bool loadTableFromFile(const QString &);
  void askAndCloseAlignment();
  void askAndCloseTraits();
  void updateTable();
  void inferNetwork(HapnetType, QVariant = QVariant());
  virtual void resizeEvent(QResizeEvent *);
  
  const bool _debug;
  QString _msgText;
  QString _msgDetail;
  bool _success;
  bool _hapnetRunning;
  ViewType _view;
  TraitType _activeTraits;
  bool _mapTraitsSet;
  bool _traitGroupsSet;
  bool _externalLegend;
  HapNet * _g;
  Sequence::CharType _datatype;
  QClipboard *_clipboard;
  QUndoStack *_history;
  QThread *_netThread;
  //QThread *_drawThread;
  QThread *_amovaThread;
  QThread *_clusterThread;
  CitationDialog *_citationDlg;
  QProgressDialog *_progress;
  QStackedWidget *_centralContainer;
  NetworkView *_netView;
  MapView *_mapView;
  NetworkModel *_netModel;
  AlignmentModel *_alModel;
  AlignmentView *_alView;
  Assistant *_assistant;
  TraitModel *_tModel;
  TraitView *_tView;
  QTabWidget *_dataWidget;
  QMenu *_networkMenu;
  QMenu *_viewMenu;
  QMenu *_mutationMenu;
  QMenu *_statsMenu;
  TableParser *_tp;
  QTableWidget *_table;
  
  std::vector<Sequence*> _alignment;//*_alignment;
  std::vector<Sequence*> _goodSeqs;
  std::vector<bool> _mask;
  std::vector<unsigned> _badSeqs;
  std::vector<Trait *> _traitVect;
  std::vector<Trait *> _geoTagVect;
  std::vector<Trait *> * _activeTraitVect;
  std::vector<ParsimonyTree *> _treeVect;
  std::vector<std::string> _groupVect;
  std::vector<std::string> _geoGroupVect;
  std::vector<std::string> *_traitGroups;
  Statistics *_stats;
  Statistics::amovatab _amovaStat;
  
  std::istream *_tabfile;

  QAction *_openAct;
  QAction *_saveAsAct;
  QAction *_closeAct;
  QAction *_importAlignAct;
  QAction *_importTraitsAct;
  QAction *_importGeoTagsAct;
  QAction *_exportAct;

  QAction *_saveGraphicsAct;
  QAction *_quitAct;
  
  QAction *_undoAct;
  QAction *_redoAct;
  QAction *_traitGroupAct;
  QAction *_traitColourAct;
  QAction *_vertexColourAct;
  QAction *_vertexSizeAct;
  QAction *_edgeColourAct;
  QAction *_backgroundColourAct;
  QAction *_labelFontAct;
  QAction *_legendFontAct;
  
  QAction *_plainMapAct;
  QAction *_blueMarbleMapAct;
  QAction *_atlasMapAct;
  QAction *_osvMapAct;
  QAction *_cityLightsMapAct;
  QAction *_oldMapAct;
  
  QAction *_redrawAct;
  
  QAction *_msnAct;
  QAction *_mjnAct;
  QAction *_apnAct;
  QAction *_injAct;
  QAction *_tswAct;
  QAction *_tcsAct;
  QAction *_umpAct;
  
  QAction *_toggleViewAct;
  QAction *_toggleTraitAct;
  QAction *_toggleExternalLegendAct;

  QAction *_dashViewAct;
  QAction *_nodeViewAct;
  QAction *_numViewAct;

  QAction *_identicalAct;
  QAction *_ntDiversityAct;
  QAction *_nSegSitesAct;
  QAction *_nParsimonyAct;
  QAction *_tajimaAct;
  QAction *_amovaAct;
  QAction *_allStatsAct;

  QAction *_nexusToolAct;
  //QAction *_exportToolAct;
  QAction *_saveToolAct;
  QAction *_colourAct;
  QAction *_zoomInAct;
  QAction *_zoomOutAct;
  QAction *_rotateLAct;
  QAction *_rotateRAct;
  QAction *_searchAct;
  QAction *_barchartAct;
  QAction *_taxBoxAct;
  
  QAction *_assistantAct;
  QAction *_citationAct;
  QAction *_aboutAct;
  QAction *_aboutQtAct;
  
  QString _filename;
  
  
private slots:

  void printprog(int);
  void openAlignment();
  void saveNexusFile();
  void importAlignment();
  void importTraits();
  void importGeoTags();
  void changeDelimiter(int);
  void setMergeDelims(bool);
  void setHasHeader(bool);
  void setHasVHeader(bool);
  void closeAlignment();
  void closeTrees();
  void closeTraits();
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
  void finaliseClustering();
  void showNetError(const QString &);
  void showIdenticalSeqs();
  void showNucleotideDiversity();
  void showSegSites();
  void showParsimonySites();
  void showTajimaD();
  void computeAmova();
  void showAmova();
  void showAllStats();
  void search();
  void changeColourTheme(); 
  void changeColour(int);
  void setTraitGroups();
  void setTraitColour();
  void changeVertexColour();
  void changeVertexSize();
  void changeEdgeColour();
  void changeEdgeMutationView(QAction*);
  void toggleView();
  void toggleActiveTraits();
  void toggleExternalLegend();
  void updateTraitLocation(unsigned, std::pair<float,float>);
  void changeBackgroundColour();
  void changeLabelFont();
  void changeLegendFont();
  void changeMapTheme(QAction *);
  void redrawNetwork();
  void toggleNetActions(bool);
  void toggleAlignmentActions(bool);
  void toggleTraitActions(bool);
  void fixBarchartButton(bool);
  void fixTaxBoxButton(bool);
  void graphicsMove(QList<QPair<QGraphicsItem *, QPointF> >);
  void showDocumentation();
  void about();

  //void tmpSlot();

  //QPlainTextEdit *_messageConsole;
  
signals:
  void sizeChanged(const QSize&);
  
};

#endif
