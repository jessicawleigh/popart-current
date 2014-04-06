#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QApplication>
#include <QButtonGroup>
#include <QChar>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QSizePolicy>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QStringList>
#include <QStyle>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QTime>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUndoCommand>
#include <QtCore/qmath.h>
#include <QtCore>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
using namespace std;


#include "HapnetWindow.h"
#include "ColourDialog.h"
#include "HapAppError.h"
#include "MoveCommand.h"
#include "GeoTrait.h"
#include "GroupItemDialog.h"
#include "XPM.h"
#include "ConcreteHapNet.h"
#include "Edge.h"
#include "EdgeItem.h"
#include "HapNet.h"
#include "MinSpanNet.h"
#include "MedJoinNet.h"
#include "MonospaceMessageBox.h"
#include "ParsimonyNet.h"
#include "IntNJ.h"
#include "TCS.h"
#include "TightSpanWalker.h"
#include "NetworkError.h"
#include "SeqParser.h"
#include "NexusParser.h"
#include "SeqParseError.h"
#include "SequenceError.h"
#include "StatsError.h"
#include "TreeError.h"

// TODO Features to add for user input:
// AMP: add user cutoff for how frequently an ancestral sequence is sampled
// TODO use QMessageBox instead of QErrorMessage
//   this would allow exception details to be written to "detailedText"

HapnetWindow::HapnetWindow(QWidget *parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags), _debug(true)
{
  _clipboard = QApplication::clipboard();
  _history = new QUndoStack(this);
  _netThread = new QThread(this);
  connect(_netThread, SIGNAL(finished()), this, SLOT(displayNetwork()));

  _statThread = new QThread(this);
  
  _clusterThread = new QThread(this);
  connect(_clusterThread, SIGNAL(finished()), this, SLOT(finaliseClustering()));

  //_drawThread = new QThread(this);
  //connect(_drawThread, SIGNAL(finished()), this, SLOT(finaliseDisplay()));
  //connect(_drawThread, SIGNAL(started()), this, SLOT(setModel()));

  _progress = new QProgressDialog(this);
  _progress->setMinimum(0);
  _progress->setMaximum(100);
  QPushButton *cancelButton = new QPushButton("Can't Cancel", _progress);
  cancelButton->setEnabled(false);
  _progress->setCancelButton(cancelButton);

  int seed = QDateTime::currentDateTime().toTime_t();
  qsrand(seed);
  QStatusBar *sbar = statusBar();
  
  resize(800, 600);
  
  _centralContainer = new QStackedWidget(this);
  setCentralWidget(_centralContainer);

  _netModel = 0;
  _netView = new NetworkView(this);
  _netView->resize(width(), height() * 2/3);
  _view = Net;
  _externalLegend = false;
  //setCentralWidget(_netView);
  _centralContainer->addWidget(_netView);
  connect(_netView, SIGNAL(itemsMoved(QList<QPair<QGraphicsItem *, QPointF> >)), this, SLOT(graphicsMove(QList<QPair<QGraphicsItem *, QPointF> > )));
  connect(_netView, SIGNAL(legendItemClicked(int)), this, SLOT(changeColour(int)));
  connect(_netView, SIGNAL(networkDrawn()), this, SLOT(finaliseDisplay()));
  //connect(_netView, SIGNAL(progressUpdated(int)), _progress, SLOT(setValue(int)));
  //connect(_netView, SIGNAL(caughtException(const QString &)), this, SLOT(showNetError(const QString &)));

  /*_messageConsole = new QPlainTextEdit(this);
  _messageConsole->setTextInteractionFlags(Qt::TextSelectableByMouse);
  _messageConsole->resize(400, 200);
  _messageConsole->setMinimumHeight(0);*/

   _mapView = new MapView(this);
   //_mapView->setVisible(false);
   _centralContainer->addWidget(_mapView);
   _mapTraitsSet = false;
   connect(_mapView, SIGNAL(positionChanged(const QString &)), sbar, SLOT(showMessage(const QString &)));
   connect(_mapView, SIGNAL(seqColourChangeRequested(int)), this, SLOT(changeColour(int)));
   connect(_mapView, SIGNAL(locationSet(unsigned, std::pair<float,float>)), this, SLOT(updateTraitLocation(unsigned, std::pair<float,float>)));

  _stats = 0;
  _alModel = 0;
  _alView = new AlignmentView(this);
  _tModel = 0;
  _tView = new TraitView(this);
  _traitGroupsSet = false;
  _tView->setSelectionBehavior(QAbstractItemView::SelectRows);
  _tView->setSelectionMode(QAbstractItemView::ContiguousSelection);
  _dataWidget = new QTabWidget(this);
  _dataWidget->addTab(_tView, "Traits");
  _dataWidget->addTab(_alView, "Alignment");

  QDockWidget *dockWidget = new QDockWidget("Data Viewer", this);
  dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
  dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::LeftDockWidgetArea);
  dockWidget->setWidget(_dataWidget);//new QLabel("Placeholder", this));
  addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
  
  _assistant = new Assistant;
  
  
  setupActions();
  setupMenus();
  setupTools();
  toggleNetActions(false);
  toggleAlignmentActions(false);

  setWindowTitle("PopART");
  setWindowIcon(QIcon(QPixmap(xpm::popart2)));
  
  // TODO add console
  //_networkArea->writeToConsole("Console text here.");
  _g = 0;
  
  //_messageConsole->setPlainText("Message output here.");
}

HapnetWindow::~HapnetWindow()
{
  // unnecessary? window's destruction takes care of this
  /*delete _networkArea;
  delete _messageConsole;*/
  
  /*if (_netThread->isRunning())
  {
    _netThread->terminate();
    
    cout << "starting timer..." << endl;
    sleep(2);    
    
    cout << "still running? " << _netThread->isRunning() << endl;
    //_netThread->wait();
  }*/
  
  closeTraits();
  closeAlignment();
}

void HapnetWindow::setupActions()
{
  
  _openAct = new QAction(tr("&Open..."), this);
  _openAct->setShortcut(QKeySequence::Open);
  _openAct->setStatusTip(tr("Open an existing alignment"));
  connect(_openAct, SIGNAL(triggered()), this, SLOT(openAlignment()));
  
  _saveAsAct = new QAction(tr("Save& As..."), this);
  _saveAsAct->setShortcut(QKeySequence::Save); // There's a SaveAs key sequence as well, but since I'm not using save...
  _saveAsAct->setStatusTip(tr("Save network as a Nexus file."));
  connect(_saveAsAct, SIGNAL(triggered()), this, SLOT(saveNexusFile()));

  _closeAct = new QAction(tr("&Close"), this);
  _closeAct->setShortcut(QKeySequence::Close);
  _closeAct->setStatusTip(tr("Close current alignment"));
  connect(_closeAct, SIGNAL(triggered()), this, SLOT(closeAlignment()));
  connect(_closeAct, SIGNAL(triggered()), this, SLOT(closeTraits()));

  _importAlignAct = new QAction(tr("&Alignment"), this);
  _importAlignAct->setStatusTip(tr("Import Phylip-format alignment"));
  connect(_importAlignAct, SIGNAL(triggered()), this, SLOT(importAlignment()));

  _importTraitsAct = new QAction(tr("&Traits"), this);
  _importTraitsAct->setStatusTip(tr("Import traits from table"));
  connect(_importTraitsAct, SIGNAL(triggered()), this, SLOT(importTraits()));

  _importGeoTagsAct = new QAction(tr("&Geo Tags"), this);
  _importGeoTagsAct->setStatusTip(tr("Import geo tags from table"));
  connect(_importGeoTagsAct, SIGNAL(triggered()), this, SLOT(importGeoTags()));

  _saveGraphicsAct = new QAction(tr("Export &graphics"), this);
  _saveGraphicsAct->setStatusTip(tr("Export network as an image"));
  _saveGraphicsAct->setEnabled(false);
  connect(_saveGraphicsAct, SIGNAL(triggered()), this, SLOT(saveGraphics()));
  
  _exportAct = new QAction(tr("E&xport network as table"), this);
  _exportAct->setStatusTip(tr("Export network as a tab-delimited table"));
  _exportAct->setEnabled(false);
  connect(_exportAct, SIGNAL(triggered()), this, SLOT(exportNetwork()));
  
  _quitAct = new QAction(tr("&Quit"), this);
  _quitAct->setShortcut(QKeySequence::Quit);
  _quitAct->setStatusTip(tr("Quit HapApp"));
  connect(_quitAct, SIGNAL(triggered()), this, SLOT(quit()));
  
  _undoAct = _history->createUndoAction(this, tr("&Undo"));
  _undoAct->setShortcuts(QKeySequence::Undo);
  
  _redoAct = _history->createRedoAction(this, tr("&Redo"));
  _redoAct->setShortcuts(QKeySequence::Redo);
  
  _traitGroupAct = new QAction(tr("Create &trait groups"), this);
  _traitGroupAct->setStatusTip(tr("Create or edit groups of traits (for AMOVA)"));
  _traitGroupAct->setEnabled(false);
  connect(_traitGroupAct, SIGNAL(triggered()), this, SLOT(setTraitGroups()));
  
  _traitColourAct = new QAction(tr("Set trait &colour"), this);
  _traitColourAct->setStatusTip(tr("Change a colour associated with a trait"));
  _traitColourAct->setEnabled(false);
  connect(_traitColourAct, SIGNAL(triggered()), this, SLOT(setTraitColour()));
  
  _vertexColourAct = new QAction(tr("Set default vertex &colour"), this);
  _vertexColourAct->setStatusTip(tr("Change the default vertex colour"));
  _vertexColourAct->setEnabled(false);
  connect(_vertexColourAct, SIGNAL(triggered()), this, SLOT(changeVertexColour()));

  _vertexSizeAct = new QAction(tr("Set default vertex &size"), this);
  _vertexSizeAct->setStatusTip(tr("Change the default vertex size"));
  _vertexSizeAct->setEnabled(false);
  connect(_vertexSizeAct, SIGNAL(triggered()), this, SLOT(changeVertexSize()));

  _edgeColourAct = new QAction(tr("Set edge &colour"), this);
  _edgeColourAct->setStatusTip(tr("Change edge colour"));
  _edgeColourAct->setEnabled(false);
  connect(_edgeColourAct, SIGNAL(triggered()), this, SLOT(changeEdgeColour()));
  
  _backgroundColourAct = new QAction(tr("Set background &colour"), this);
  _backgroundColourAct->setStatusTip(tr("Change network background colour"));
  _backgroundColourAct->setEnabled(false);
  connect(_backgroundColourAct, SIGNAL(triggered()), this, SLOT(changeBackgroundColour()));

  _legendFontAct = new QAction(tr("Set &legend font"), this);
  _legendFontAct->setStatusTip(tr("Change font used in legend"));
  _legendFontAct->setEnabled(false);
  connect(_legendFontAct, SIGNAL(triggered()), this, SLOT(changeLegendFont()));

  _labelFontAct = new QAction(tr("Set &label font"), this);
  _labelFontAct->setStatusTip(tr("Change font used for node labels"));
  _labelFontAct->setEnabled(false);
  connect(_labelFontAct, SIGNAL(triggered()), this, SLOT(changeLabelFont()));
  
  QActionGroup *mapThemeActions = new QActionGroup(this);
  _plainMapAct = new QAction(tr("Plain"), mapThemeActions);
  _plainMapAct->setCheckable(true);
  _plainMapAct->setChecked(true);
  _blueMarbleMapAct = new QAction(tr("Blue marble"), mapThemeActions);
  _blueMarbleMapAct->setCheckable(true);
  _atlasMapAct = new QAction(tr("Atlas"), mapThemeActions);
  _atlasMapAct->setCheckable(true);
  _osvMapAct = new QAction(tr("Open Streetmap"), mapThemeActions);
  _osvMapAct->setCheckable(true);
  _cityLightsMapAct = new QAction(tr("Nighttime"), mapThemeActions);
  _cityLightsMapAct->setCheckable(true);
  _oldMapAct = new QAction(tr("Old map (1689)"), mapThemeActions);
  _oldMapAct->setCheckable(true);
  connect(mapThemeActions, SIGNAL(triggered(QAction*)), this, SLOT(changeMapTheme(QAction*)));
  
  _redrawAct = new QAction(tr("Re&draw network"), this);
  _redrawAct->setStatusTip(tr("Redraw the current network"));
  _redrawAct->setEnabled(false);
  connect(_redrawAct, SIGNAL(triggered()), this, SLOT(redrawNetwork()));//_netView, SLOT(redraw()));
  
  _msnAct = new QAction(tr("&Minimum Spanning Network"), this);
  _msnAct->setStatusTip(tr("Build minimum spanning network"));
  connect(_msnAct, SIGNAL(triggered()), this, SLOT(buildMSN()));
  
  _mjnAct = new QAction(tr("Median &Joining Network"), this);
  _mjnAct->setStatusTip(tr("Build median joining network"));
  connect(_mjnAct, SIGNAL(triggered()), this, SLOT(buildMJN()));
  
  _apnAct = new QAction(tr("Ancestral M&P Network"), this);
  _apnAct->setStatusTip(tr("Build ancestral maximum parsimony network"));
  connect(_apnAct, SIGNAL(triggered()), this, SLOT(buildAPN()));
  
  _injAct = new QAction(tr("&Integer NJ Net"), this);
  _injAct->setStatusTip(tr("Build integer neightbour-joining network"));
  connect(_injAct, SIGNAL(triggered()), this, SLOT(buildIntNJ()));

  _tswAct = new QAction(tr("Tight Span &Walker"), this);
  _tswAct->setStatusTip(tr("Build Tight Span Walker network"));
  connect(_tswAct, SIGNAL(triggered()), this, SLOT(buildTSW()));
  
  _tcsAct = new QAction(tr("&TCS Network"), this);
  _tcsAct->setStatusTip(tr("Build TCS network"));
  connect(_tcsAct, SIGNAL(triggered()), this, SLOT(buildTCS()));

  /*_umpAct = new QAction(tr("UMP Network"), this);
  _umpAct->setShortcut(tr("Ctrl+U"));
  _umpAct->setStatusTip(tr("Build UMP network"));
  connect(_umpAct, SIGNAL(triggered()), this, SLOT(buildUMP()));*/

  _toggleViewAct = new QAction(tr("Switch to map &view"), this);
  _toggleViewAct->setStatusTip(tr("Toggle between network and map view"));
  connect(_toggleViewAct, SIGNAL(triggered()), this, SLOT(toggleView()));

  _toggleTraitAct = new QAction(tr("Use GeoTags &groups"), this);
  _toggleTraitAct->setStatusTip(tr("Toggle between groups defined by Traits and GeoTags"));
  connect(_toggleTraitAct, SIGNAL(triggered()), this, SLOT(toggleActiveTraits()));
  _toggleTraitAct->setEnabled(false);
  
  _toggleExternalLegendAct = new QAction(tr("Show &legend window"), this);
  _toggleExternalLegendAct->setStatusTip(tr("Show legend in external window"));
  connect(_toggleExternalLegendAct, SIGNAL(triggered()), this, SLOT(toggleExternalLegend()));
  _toggleExternalLegendAct->setEnabled(false);

  QActionGroup *viewActions = new QActionGroup(this);
  _dashViewAct = new QAction(tr("&Hatch marks"), viewActions);
  _dashViewAct->setStatusTip(tr("Show mutations as hatch marks along edges"));
  _dashViewAct->setCheckable(true);
  _dashViewAct->setChecked(true);
  _nodeViewAct = new QAction(tr("&1-step edges"), viewActions);
  _nodeViewAct->setStatusTip(tr("Show single-mutation edges with intermediate vertices"));
  _nodeViewAct->setCheckable(true);
  _numViewAct = new QAction(tr("&Numbers"), viewActions);
  _numViewAct->setStatusTip(tr("Show numbers of mutations along edges"));
  _numViewAct->setCheckable(true);
  connect(viewActions, SIGNAL(triggered(QAction*)), this, SLOT(changeEdgeMutationView(QAction*)));
  
  _identicalAct = new QAction(tr("&Identical sequences"), this);
  _identicalAct->setStatusTip(tr("Show identical sequences"));
  connect(_identicalAct, SIGNAL(triggered()), this, SLOT(showIdenticalSeqs()));
  
  _ntDiversityAct = new QAction(tr("Nucleotide &diversity"), this);
  _ntDiversityAct->setStatusTip(tr("Compute nucleotide diversity"));
  connect(_ntDiversityAct, SIGNAL(triggered()), this, SLOT(showNucleotideDiversity()));
  
  _nSegSitesAct = new QAction(tr("&Segregating sites"), this);
  _nSegSitesAct->setStatusTip(tr("Count segregating sites"));
  connect(_nSegSitesAct, SIGNAL(triggered()), this, SLOT(showSegSites()));
  
  _nParsimonyAct = new QAction(tr("&Parsimony-informative sites"), this);
  _nParsimonyAct->setStatusTip(tr("Count parsimony-informative sites"));
  connect(_nParsimonyAct, SIGNAL(triggered()), this, SLOT(showParsimonySites()));
  
  _tajimaAct = new QAction(tr("&Tajima's D"), this);
  _tajimaAct->setStatusTip(tr("Compute Tajima's D statsitic"));
  connect(_tajimaAct, SIGNAL(triggered()), this, SLOT(showTajimaD()));
  
  _amovaAct = new QAction(tr("A&MOVA"), this);
  _amovaAct->setStatusTip(tr("Perform analysis of molecular variance"));
  connect(_amovaAct, SIGNAL(triggered()), this, SLOT(showAmova()));
  
  _allStatsAct = new QAction(tr("&All stats"), this);
  _allStatsAct->setStatusTip(tr("Compute all statistics"));
  connect(_allStatsAct, SIGNAL(triggered()), this, SLOT(showAllStats()));
  
  _colourAct = new QAction(QIcon(QPixmap(xpm::colourTheme)), tr("Change colour theme"), this);
  _colourAct->setStatusTip(tr("Change colour theme"));
  connect(_colourAct, SIGNAL(triggered()), this, SLOT(changeColourTheme()));
  
   _zoomInAct = new QAction(QIcon(QPixmap(xpm::zoomin)), tr("Zoom in"), this);
  _zoomInAct->setEnabled(false);
  connect(_zoomInAct, SIGNAL(triggered()), _netView, SLOT(zoomIn()));
  
  _zoomOutAct = new QAction(QIcon(QPixmap(xpm::zoomout)), tr("Zoom out"), this);
  _zoomOutAct->setEnabled(false);
  connect(_zoomOutAct, SIGNAL(triggered()), _netView, SLOT(zoomOut()));
 
  _rotateLAct = new QAction(QIcon(QPixmap(xpm::rotateL)), tr("Rotate left"), this);
  _rotateLAct->setEnabled(false);
  connect(_rotateLAct, SIGNAL(triggered()), _netView, SLOT(rotateL()));

  _rotateRAct = new QAction(QIcon(QPixmap(xpm::rotateR)), tr("Rotate right"), this);
  _rotateRAct->setEnabled(false);
  connect(_rotateRAct, SIGNAL(triggered()), _netView, SLOT(rotateR()));
  
  _searchAct = new QAction(QIcon(QPixmap(xpm::search)), tr("Search for a node"), this);
  _searchAct->setShortcut(QKeySequence::Find);//tr("Ctrl+F"));
  _searchAct->setEnabled(false);
  connect(_searchAct, SIGNAL(triggered()), this, SLOT(search()));

  _barchartAct = new QAction(QIcon(QPixmap(xpm::barchart)), tr("Toggle show barcharts"), this);
  _barchartAct->setEnabled(false);
  _barchartAct->setCheckable(true);
  connect(_barchartAct, SIGNAL(toggled(bool)), _netView, SLOT(toggleShowBarcharts(bool)));
  connect(_barchartAct,  SIGNAL(toggled(bool)), this, SLOT(fixTaxBoxButton(bool)));
  
  _taxBoxAct = new QAction(QIcon(QPixmap(xpm::taxbox)), tr("Toggle show identical taxa"), this);
  _taxBoxAct->setEnabled(false);
  _taxBoxAct->setCheckable(true);
  connect(_taxBoxAct, SIGNAL(toggled(bool)), _netView, SLOT(toggleShowTaxBox(bool)));
  connect(_taxBoxAct, SIGNAL(toggled(bool)), this, SLOT(fixBarchartButton(bool)));
  
  _assistantAct = new QAction(tr("Help Contents"), this);
  _assistantAct->setShortcut(QKeySequence::HelpContents);
  connect(_assistantAct, SIGNAL(triggered()), this, SLOT(showDocumentation()));
  
  _aboutAct = new QAction(tr("&About"), this);
  connect(_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  _aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

}

void HapnetWindow::setupMenus()
{  
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(_openAct);
  fileMenu->addAction(_closeAct);
  fileMenu->addAction(_saveAsAct);
  fileMenu->addSeparator();
  
  QMenu *importMenu = fileMenu->addMenu(tr("Import..."));
  importMenu->addAction(_importAlignAct);
  importMenu->addAction(_importTraitsAct);
  importMenu->addAction(_importGeoTagsAct);

  fileMenu->addAction(_exportAct);
  fileMenu->addAction(_saveGraphicsAct);
  fileMenu->addSeparator();

  fileMenu->addAction(_quitAct);
  
  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    
  editMenu->addAction(_undoAct);
  editMenu->addAction(_redoAct);
  editMenu->addSeparator();

  //QAction *dataEditAction = new QAction("Data editing disabled", this);
  //dataEditAction->setEnabled(false);
  //editMenu->addAction(dataEditAction);
  //editMenu->addSeparator();
  editMenu->addAction(_traitGroupAct);
  editMenu->addAction(_traitColourAct);
  editMenu->addAction(_vertexColourAct);
  editMenu->addAction(_vertexSizeAct);
  editMenu->addAction(_edgeColourAct);
  editMenu->addAction(_backgroundColourAct);
  editMenu->addSeparator();
  editMenu->addAction(_labelFontAct);
  editMenu->addAction(_legendFontAct);
  
  editMenu->addSeparator();
  editMenu->addAction(_redrawAct);
  
  
  _networkMenu = menuBar()->addMenu(tr("&Network"));
  _networkMenu->addAction(_msnAct);
  _networkMenu->addAction(_mjnAct);
  _networkMenu->addAction(_apnAct);
  _networkMenu->addAction(_injAct);
  _networkMenu->addAction(_tswAct);
  _networkMenu->addAction(_tcsAct);
  //_networkMenu->addAction(_umpAct);
  //_networkMenu->setEnabled(false);
  
  _viewMenu = menuBar()->addMenu(tr("&View"));
  _viewMenu->addAction(_toggleViewAct);
  _viewMenu->addAction(_toggleExternalLegendAct);
  _viewMenu->addAction(_toggleTraitAct);

  _viewMenu->addSeparator();//->setText(tr("Network"));
  // Add maps: make a group for network vs. map view, and a setSeparator(true) for both, maybe
  // Disable mutation view options when map view selected
  _mutationMenu = _viewMenu->addMenu(tr("Show mutations as..."));
  _mutationMenu->addAction(_dashViewAct);
  _mutationMenu->addAction(_nodeViewAct);
  _mutationMenu->addAction(_numViewAct);

  _viewMenu->addSeparator();
  QMenu *mapThemeMenu = _viewMenu->addMenu(tr("Map theme..."));
  mapThemeMenu->addAction(_plainMapAct); 
  mapThemeMenu->addAction(_blueMarbleMapAct);
  mapThemeMenu->addAction(_atlasMapAct); // strm
  mapThemeMenu->addAction(_osvMapAct);
  mapThemeMenu->addAction(_cityLightsMapAct);
  mapThemeMenu->addAction(_oldMapAct);


  _statsMenu =  menuBar()->addMenu(tr("&Statistics"));
  _statsMenu->addAction(_identicalAct);
  _statsMenu->addSeparator();
  _statsMenu->addAction(_ntDiversityAct);
  _statsMenu->addAction(_nSegSitesAct);
  _statsMenu->addAction(_nParsimonyAct);
  _statsMenu->addAction(_tajimaAct);
  _statsMenu->addAction(_amovaAct);
  _statsMenu->addSeparator();  
  _statsMenu->addAction(_allStatsAct);
  
  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(_assistantAct);
  helpMenu->addAction(_aboutAct);
  helpMenu->addAction(_aboutQtAct);
  
}

void HapnetWindow::setupTools()
{
  QToolBar *toolbar = addToolBar(tr("Toolbar"));
  //toolbar->addAction(tr("Tools here"));
  QStyle *toolstyle = toolbar->style();
  
  _nexusToolAct = toolbar->addAction(QIcon(QPixmap(xpm::nexus)), "Open Nexus file");
  connect(_nexusToolAct, SIGNAL(triggered()), this, SLOT(openAlignment()));
  
  /*_exportToolAct = toolbar->addAction(toolstyle->standardIcon(QStyle::SP_DialogSaveButton), "Export network");
  connect(_exportToolAct, SIGNAL(triggered()), this, SLOT(exportNetwork()));
  _exportToolAct->setEnabled(false);*/

  _saveToolAct = toolbar->addAction(toolstyle->standardIcon(QStyle::SP_DialogSaveButton), "Save network as Nexus file");
  connect(_saveToolAct, SIGNAL(triggered()), this, SLOT(saveNexusFile()));
  _saveToolAct->setEnabled(false);
  
  toolbar->addSeparator();
  
  QSlider *timeSlider = new QSlider(Qt::Horizontal, toolbar);
  timeSlider->setMinimum(-50);
  timeSlider->setMaximum(0);
  timeSlider->setTickInterval(10);
  
  timeSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QLabel *timeLabel = new QLabel(tr("Time (kYA)"), toolbar);
  
  timeSlider->setEnabled(false);
  timeLabel->setEnabled(false);
  timeSlider->setVisible(false);
  timeLabel->setVisible(false);
  //connect(timeSlider, SIGNAL(isEnabled(bool)), timeLabel, SLOT(enable(bool)));

  toolbar->addWidget(timeLabel);
  toolbar->addWidget(timeSlider);
  
  // important for time slider, if I add it again
  //toolbar->addSeparator();
  
  toolbar->addAction(_colourAct);
  toolbar->addAction(_zoomInAct);
  toolbar->addAction(_zoomOutAct);
  toolbar->addAction(_rotateLAct);
  toolbar->addAction(_rotateRAct);
  toolbar->addAction(_searchAct);

  toolbar->addSeparator();

  toolbar->addAction(_taxBoxAct);
  toolbar->addAction(_barchartAct);
}

void HapnetWindow::showErrorDlg(const QString &text, const QString &detailedText, const QString &informativeText)
{
  QMessageBox errorBox;
  errorBox.setIcon(QMessageBox::Critical);
  errorBox.setText(text);
  if (! detailedText.isEmpty())
    errorBox.setDetailedText(detailedText);
  if (! informativeText.isEmpty())
    errorBox.setInformativeText(informativeText);
  errorBox.setStandardButtons(QMessageBox::Ok);
  errorBox.setDefaultButton(QMessageBox::Ok);
  errorBox.exec();

}

void HapnetWindow::showWarnDlg(const QString &text, const QString &detailedText, const QString &informativeText)
{
  QMessageBox warnBox;
  warnBox.setIcon(QMessageBox::Warning);
  warnBox.setText(text);
  if (! detailedText.isEmpty())
    warnBox.setDetailedText(detailedText);
  if (! informativeText.isEmpty())
    warnBox.setInformativeText(informativeText);
  warnBox.setStandardButtons(QMessageBox::Ok);
  warnBox.setDefaultButton(QMessageBox::Ok);
  warnBox.exec();

}


void HapnetWindow::openAlignment()
{
  QString oldname(_filename);

  //_filename = QFileDialog::getOpenFileName(this, "Open alignment file", ".", "Fasta Files (*.fas *.fa *.fasta);;Phylip Files (*.seq *.phy *.phylip);;Nexus files (*.nex *.mac);;All Files(*)").toStdString();
  _filename = QFileDialog::getOpenFileName(this, "Open alignment file", tr("."), "Nexus files (*.nex *.mac);;All Files(*)");
  
  if (_filename != "")
  {  
    if (! _alignment.empty())
    {
       closeAlignment();
       closeTraits();
    }

    // TODO check whether these functions can produce traits exceptions that aren't caught
    bool success = loadAlignmentFromFile();
    bool traitsuccess = loadTraitsFromParser();
    bool netsuccess = loadNetFromParser();
    bool treesuccess = loadTreesFromParser(_treeVect);

    if (! treesuccess)
      closeTrees();

    if (success)
    {
      statusBar()->showMessage(tr("Loaded file %1").arg(_filename));
      //_networkMenu->setEnabled(true);
      
      //if (traitsuccess)  _stats->setFreqsFromTraits(_traitVect);
      QFileInfo fileInfo(_filename);
      setWindowTitle(tr("PopART: %1").arg(fileInfo.fileName()));
      toggleAlignmentActions(true);

      if (! _alModel)
      {
        _alModel = new AlignmentModel(_alignment);
        _alView->setModel(_alModel);

        //_alModel->setCharType(_datatype);
        _alModel->setCharType(_datatype);
        
        if (! _mask.empty())
        {
          for (unsigned i = 0; i < _mask.size(); i++)
            if (! _mask.at(i))
              _alModel->maskColumns(i, i);
        }
        
        if (! _badSeqs.empty())
        {
          for (unsigned i = 0; i < _badSeqs.size(); i++)
            _alModel->maskRows(_badSeqs.at(i), _badSeqs.at(i));
        }
        
        if (traitsuccess)
        {
          _tModel = new TraitModel(_traitVect);
          _tView->setModel(_tModel);

          if (_view == Map)
          {
            _mapView->addHapLocations(_traitVect);
            _mapTraitsSet = true;
          }

          else
            _mapTraitsSet = false;
          
          toggleTraitActions(true);
        }
        
        // this will turn off AMOVA, which requires traits
        else
          toggleTraitActions(false);

        if (netsuccess)
        {
          //displayNetwork();
          
          try
          {
            _g->associateTraits(_traitVect);
            _netModel = new NetworkModel(_g); 
            _netView->setModel(_netModel, true);
            loadNetAttributes();
            toggleNetActions(true);      
          }
          catch (NetworkError &ne)
          {
            toggleNetActions(false);
            showErrorDlg("<b>Error finalising network</b>", ne.what());
          }

        }
      }


      else
      {
        _netView->clearModel();
        toggleNetActions(false);

        QAbstractItemModel *tmpModel = _tView->model();
        if (traitsuccess)
        {
          _tModel = new TraitModel(_traitVect);
          _tView->setModel(_tModel);
          if (_view == Map)
          {
            _mapView->addHapLocations(_traitVect);
            _mapTraitsSet = true;
          }

          else
            _mapTraitsSet = false;
        }
        delete tmpModel;

        tmpModel = _alView->model();
        _alModel = new AlignmentModel(_alignment);
        _alView->setModel(_alModel);
        _alModel->setCharType(_datatype);
        delete tmpModel;
        //_alView->resizeColumnsToContents();
      }
    }
  }

  else
  {
    _filename = oldname;
    statusBar()->showMessage(tr("No file selected"));
  }  
}

bool HapnetWindow::loadAlignmentFromFile(QString filename)
{

  //int index = 0;
  if (filename.isEmpty())  filename = _filename;

  const char *cstr = filename.toLatin1().constData();


  // need a std::ifstream to deal with Sequence input
  ifstream seqfile(cstr);

 // _networkArea->writeToConsole("Opened seqfile.");

  // Set null Parser so that appropriate parser will be chosen
  Sequence::setParser(0);

  if (!seqfile)  return false;

  //_alignment = new vector<Sequence *>;
  Sequence *seqptr;
  int nseq = -1, seqcount = 0;
  _progress->setValue(0);
  _progress->setLabelText("Loading alignment...");
  _progress->show();

  try
  {
    while (seqfile.good())
    {
      seqptr = new Sequence();
      seqfile >> (*seqptr);
      QString qseq = QString::fromStdString(seqptr->seq()).toUpper();
      seqptr->setSeq(qseq.toStdString());
      _alignment.push_back(seqptr);
      //cout << "nseq: " << seqptr->parser()->nSeq() << endl;
      if (nseq < 0)  nseq = seqptr->parser()->nSeq();
      _progress->setValue((int)(50.0 * seqcount++/ nseq + 0.5));
      //cout << "set value to: " << _progress->value() << endl;
    }
    //_progress->hide();
  }

  catch (SeqParseError &spe)
  {
    _progress->hide();
    //cerr << spe.what() << endl;
    //_errorMessage.showMessage("Error parsing sequence data.");
    
    showErrorDlg(tr("<b>Error parsing sequence data</b>"), spe.what());
   /* QMessageBox error;
    error.setIcon(QMessageBox::Critical);
    error.setText(tr("<b>Error parsing sequence data</b>"));
    error.setDetailedText(spe.what());
    error.setStandardButtons(QMessageBox::Ok);
    error.setDefaultButton(QMessageBox::Ok);
    error.exec();*/

    return false;
  }
  
  catch (SequenceError &se)
  {
    showErrorDlg(tr("<b>Error parsing sequence data</b>"), se.what());
  }

  seqfile.close();

  SeqParser *parser = Sequence::parser();

  QString msg = QString::fromStdString(parser->getWarning());

  while (! msg.isEmpty())
  {
    QMessageBox warnBox;
    warnBox.setIcon(QMessageBox::Warning);
    warnBox.setText(tr("<b>Warning from Nexus Parser</b>"));
    warnBox.setInformativeText(msg);
    warnBox.setStandardButtons(QMessageBox::Ok);
    warnBox.setDefaultButton(QMessageBox::Ok);
    warnBox.exec();

    msg = QString::fromStdString(parser->getWarning());
  }


  if (parser->charType() == SeqParser::AAType)
    _datatype = Sequence::AAType;
  else if (parser->charType() == SeqParser::DNAType)
    _datatype = Sequence::DNAType;
  else if (parser->charType() == SeqParser::StandardType)
    _datatype = Sequence::BinaryType;

  /*if (_datatype == Sequence::BinaryType)
  else if (_datatype == Sequence::DNAType)
  else*/
  _progress->setLabelText("Detecting problem sites...");
  vector<unsigned> ambiguousCounts;

  for (unsigned i = 0; i < _alignment.size(); i++)
  {
    _alignment.at(i)->setCharType(_datatype);

    ambiguousCounts.push_back(0);

    if (i == 0)
      _mask.resize(_alignment.at(i)->length(), true);

    else  if (_mask.size() != _alignment.at(i)->length())
    {
      QMessageBox warnBox;
      warnBox.setIcon(QMessageBox::Warning);
      warnBox.setText(tr("<b>Sequence lengths differ</b>"));
      warnBox.setStandardButtons(QMessageBox::Ok);
      warnBox.setDefaultButton(QMessageBox::Ok);
      warnBox.exec();

      if (_mask.size() < _alignment.at(i)->length())
        _mask.resize(_alignment.at(i)->length(), false);
      else
        for (unsigned j = _alignment.at(i)->length(); j < _mask.size(); j++)
          _mask.at(j) = false;
    }

    for (unsigned j = 0; j < _mask.size(); j++)
    {
      if (Sequence::isAmbiguousChar(_alignment.at(i)->at(j), _datatype))
      {
        _mask.at(j) = false;
        ambiguousCounts[i]++;
      }
    }
  }

  unsigned totalAmbiguous = 0;
  for (unsigned i = 0; i < _mask.size(); i++)
    if (! _mask.at(i))  totalAmbiguous++;

  if (totalAmbiguous * 100. / _mask.size() > 5)
  {

    //_errorMessage.showMessage("Warning: more than 5% sites contain undefined states and will be masked.");
    //_errorMessage.
    QMessageBox warnBox;
    warnBox.setIcon(QMessageBox::Warning);
    warnBox.setText(tr("<b>More than 5% sites contain undefined states and will be masked</b>"));
    warnBox.setStandardButtons(QMessageBox::Ok);
    warnBox.setDefaultButton(QMessageBox::Ok);
    warnBox.exec();

    double mean = 0;
    for (unsigned i = 0; i < ambiguousCounts.size(); i++)
      mean += ambiguousCounts.at(i);

    mean /= ambiguousCounts.size();

    double stdev = 0;

    for (unsigned i = 0; i < ambiguousCounts.size(); i++)
      stdev += qPow(ambiguousCounts.at(i) - mean, 2);

    stdev = qSqrt(stdev / ambiguousCounts.size());

    double outlierThreshold = mean + 0.5 * stdev;

    QStringList problemSeqs;

    for (unsigned i = 0; i < ambiguousCounts.size(); i++)
    {
      if (ambiguousCounts.at(i) > outlierThreshold)
      {
        problemSeqs.append(QString::fromStdString(_alignment.at(i)->name()));
        _badSeqs.push_back(i);
      }
    }

    _progress->setValue(100);

    if (! problemSeqs.isEmpty())
    {

      QMessageBox message;
      message.setIcon(QMessageBox::Question);
      message.setText(tr("<b>Some sequences contain significantly more undefined states than others</b>"));
      message.setInformativeText(tr("Remove these sequences?"));
      message.setDetailedText(problemSeqs.join(tr("\n")));
      message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      message.setDefaultButton(QMessageBox::No);
      int result = message.exec();

      if (result == QMessageBox::No)
        _badSeqs.clear();

      else
      {

        unsigned badSeqIdx = 0;
        for (unsigned i = 0; i < _alignment.size(); i++)
        {
          if (i == 0)
            _mask.assign(_alignment.at(i)->length(), true);

          if (badSeqIdx < _badSeqs.size() && _badSeqs.at(badSeqIdx) == i)
          {
            badSeqIdx++;
            continue;
          }

          else
            _goodSeqs.push_back(_alignment.at(i));


          if (_mask.size() < _alignment.at(i)->length())
            _mask.resize(_alignment.at(i)->length(), false);
          else  if (_mask.size() > _alignment.at(i)->length())
            for (unsigned j = _alignment.at(i)->length(); j < _mask.size(); i++)
              _mask.at(j) = false;


          for (unsigned j = 0; j < _mask.size(); j++)
          {
            if (Sequence::isAmbiguousChar(_alignment.at(i)->at(j), _datatype))
            {
              _mask.at(j) = false;
            }
          }
        }
      }
    }
  }

  if (_goodSeqs.empty())
    _goodSeqs.assign(_alignment.begin(), _alignment.end());

  //doStatsSetup();
  _progress->hide();
  return true;
}

bool HapnetWindow::loadNetFromParser()
{
  
  NexusParser *nexParser = dynamic_cast<NexusParser *>(Sequence::parser());

  if (! nexParser)  return false;

  // Do this in loadNetFromFile
  Graph g;

  const vector<pair<double,double> > & vertices = nexParser->netVertices();
  const vector<pair<unsigned,unsigned> >  & edges = nexParser->netEdges();

  if (vertices.empty())
    return false;

  if (edges.empty())
    return false;

  for (unsigned i = 0; i < vertices.size(); i++)
    g.newVertex("");


  for (unsigned i = 0; i < edges.size(); i++)
    g.newEdge(g.vertex(edges.at(i).first), g.vertex(edges.at(i).second));

  if (_g)  delete _g;
  try
  {
    _g = new ConcreteHapNet(g, _goodSeqs, _mask);
    _g->setupGraph();
  }
  
  catch (NetworkError &ne)
  {
    showErrorDlg("<b>Error rebuilding network</b>", ne.what());
    return false;
  }

  return true;

}

bool HapnetWindow::loadNetAttributes()
{
  NexusParser *nexParser = dynamic_cast<NexusParser *>(Sequence::parser());
  
  const vector<pair<double,double> > & vertexPositions = nexParser->netVertices();
  const vector<pair<double,double> > & labelPositions = nexParser->netVLabels();

  bool allgood = true;

  for (unsigned i = 0; i < vertexPositions.size(); i++)
  {
    try
    {
      _netView->setVertexPosition(i, vertexPositions.at(i).first, vertexPositions.at(i).second);
    }
    catch (HapAppError &)
    {
      allgood = false;
    }
  }

  if (! allgood)
    showWarnDlg("<b>Error setting one or more vertex positions.</b>");

  allgood = true;

  for (unsigned i = 0; i < labelPositions.size(); i++)
  {
    try
    {
      _netView->setLabelPosition(i,labelPositions.at(i).first, labelPositions.at(i).second);
    }
    catch (HapAppError &)
    {
      allgood = false;
    }
  }

  if (! allgood)
    showWarnDlg("<b>Error setting one or more label positions.</b>");

  QFont font;
  QString fontstr = QString::fromStdString(nexParser->netFont());

  if (font.fromString(fontstr))
    _netView->changeLabelFont(font);

  else
    showWarnDlg("<b>Error setting label font.</b>", tr("Font string %1 could not be parsed").arg(fontstr));

  fontstr = QString::fromStdString(nexParser->netLegendFont());

  if (font.fromString(fontstr))
    _netView->changeLegendFont(font);

  else
    showWarnDlg("<b>Error setting legend font.</b>", tr("Font string %1 could not be parsed").arg(fontstr));


  QColor col;
  int alpha;
  bool hasalpha;
  bool ok = true;
  QString colstr = QString::fromStdString(nexParser->netVColour());

  if (colstr.length() == 9)
    hasalpha = true;
  else if (colstr.length() == 7)
    hasalpha = false;
  else
    ok = false;

  if (ok)
  {

    if (hasalpha)
    {
      col = QColor(colstr.left(7));
      alpha = colstr.right(2).toInt(&ok, 16);
    }
    
    else
    {
      col = QColor(colstr);
      alpha = 255;
    }

    if (ok)
    {
      col.setAlpha(alpha);
      _netView->setVertexColour(col);
    }
  }

  if (! ok)
    showWarnDlg("<b>Error setting vertex colour.</b>", tr("Colour string %1 could not be parsed").arg(colstr));

  colstr = QString::fromStdString(nexParser->netEColour());
  ok = true;

  if (colstr.length() == 9)
    hasalpha = true;
  else if (colstr.length() == 7)
    hasalpha = false;
  else
    ok = false;

  if (ok)
  {

    if (hasalpha)
    {
      col = QColor(colstr.left(7));
      alpha = colstr.right(2).toInt(&ok, 16);
    }
    
    else
    {
      col = QColor(colstr);
      alpha = 255;
    }

    if (ok)
    {
      col.setAlpha(alpha);
      _netView->setEdgeColour(col);
    }
  }

  if (! ok)
    showWarnDlg("<b>Error setting edge colour.</b>", tr("Colour string %1 could not be parsed").arg(colstr));

  colstr = QString::fromStdString(nexParser->netBGColour());
  ok = true;

  if (colstr.length() == 9)
    hasalpha = true;
  else if (colstr.length() == 7)
    hasalpha = false;
  else
    ok = false;

  if (ok)
  {

    if (hasalpha)
    {
      col = QColor(colstr.left(7));
      alpha = colstr.right(2).toInt(&ok, 16);
    }
    
    else
    {
      col = QColor(colstr);
      alpha = 255;
    }

    if (ok)
    {
      col.setAlpha(alpha);
      _netView->setBackgroundColour(col);
    }
  }

  if (! ok)
    showWarnDlg("<b>Error setting background colour.</b>", tr("Colour string %1 could not be parsed").arg(colstr));

  string eview = nexParser->netEView();


  if (eview == "dashes")
  {
    _netView->setEdgeMutationView(EdgeItem::ShowDashes);
    _dashViewAct->setChecked(true);
  }

  else if (eview == "ellipses")
  {
    _netView->setEdgeMutationView(EdgeItem::ShowEllipses);
    _nodeViewAct->setChecked(true);
  }

  else if (eview == "numbers")
  {
    _netView->setEdgeMutationView(EdgeItem::ShowNums);
    _dashViewAct->setChecked(true);
  }

  else
    showWarnDlg("<b>Error setting edge view.</b>", tr("Edge view string %1 could not be parsed").arg(QString::fromStdString(eview)));


  _netView->setVertexSize(nexParser->netVSize());

  QPointF legPos(nexParser->netLPos().first, nexParser->netLPos().second);
  
  // I'm using -1,-1 to indicate no legend, dumb
  if (legPos.x() >= 0 || legPos.y() >= 0)
    _netView->setLegendPosition(legPos);


  list<string>::const_iterator colIt = nexParser->netLColours().begin();
  unsigned count = 0;
  while (colIt != nexParser->netLColours().end())
  {
    colstr = QString::fromStdString(*colIt);
    
    ok = true;

    if (colstr.length() == 9)
      hasalpha = true;
    else if (colstr.length() == 7)
      hasalpha = false;
    else
      ok = false;

    if (ok)
    {

      if (hasalpha)
      {
        col = QColor(colstr.left(7));
        string colstdstr = colstr.toStdString();
        alpha = colstr.right(2).toInt(&ok, 16);
      }
      
      else
      {
        col = QColor(colstr);
      string colstdstr = colstr.toStdString();
        alpha = 255;
      }
      
      if (ok)
      {
        col.setAlpha(alpha);
        _netView->setColour(count, col);
      }
    }

    if (! ok)
      showWarnDlg(tr("<b>Error setting colour for trait %1.</b>").arg(count), tr("Colour string %1 could not be parsed").arg(colstr));



    ++colIt;
    count++;
  }
  
  const vector<double> & sceneRect = nexParser->netPlotDim();

  if (sceneRect.size() != 4)
    showWarnDlg("<b>Error setting network plot dimensions.</b>");
  else
    _netView->setSceneRect(sceneRect.at(0), sceneRect.at(1), sceneRect.at(2), sceneRect.at(3));


  return true;
}

bool HapnetWindow::loadTraitsFromParser()
{
  NexusParser *parser = dynamic_cast<NexusParser *>(Sequence::parser());
  
  if (! parser)  return false;
  
  try
  {
    if (parser->hasTraits() && parser->hasGeoTags())
    {
      QDialog dlg(this);
      
      QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
      
      vlayout->addWidget(new QLabel("Nexus file has both Traits and GeoTags data.<br>Which do you want to use by default to colour networks?", this));
      
      QButtonGroup *buttonGroup = new QButtonGroup(this);
      int idCount = 0;
      QRadioButton *button = new QRadioButton("Use Traits", &dlg);
      button->setChecked(true);
      buttonGroup->addButton(button, idCount++);
      vlayout->addWidget(button);
      
      button = new QRadioButton("Use GeoTags", &dlg);
      buttonGroup->addButton(button, idCount++);
      vlayout->addWidget(button);
      
      QHBoxLayout *hlayout = new QHBoxLayout;
      
      QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
      connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
      hlayout->addWidget(okButton, 0, Qt::AlignRight);
      QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
      connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
      hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
      
      vlayout->addLayout(hlayout);      
      
      int result = dlg.exec();
      
      if (result == QDialog::Rejected)
        return false;
      
      if (buttonGroup->checkedId() == 0)
      {

        const vector<Trait*> &traits = parser->traitVector();
        for (unsigned i = 0; i < traits.size(); i++)
        {
          GeoTrait *gt = dynamic_cast<GeoTrait *>(traits.at(i));
          if (gt)
            _traitVect.push_back(new GeoTrait(*gt));
          else
            _traitVect.push_back(new Trait(*(traits.at(i))));
        }
        
        _activeTraits = Traits;
        _toggleTraitAct->setText(tr("Use GeoTags &groups"));
        _traitGroups = &_groupVect;
      }

      else
      {
        const vector<GeoTrait*> & gtv = parser->geoTraitVector();

        for (unsigned i = 0; i < gtv.size(); i++)
          _traitVect.push_back(new GeoTrait(*(gtv.at(i))));

        _activeTraits = GeoTags;
        _toggleTraitAct->setText(tr("Use Traits &groups"));
        _traitGroups = &_geoGroupVect;

      }

      _toggleTraitAct->setEnabled(true);
    }
    
    else if (parser->hasTraits())
    {

      const vector<Trait*> &traits = parser->traitVector();
      for (unsigned i = 0; i < traits.size(); i++)
      {
        GeoTrait *gt = dynamic_cast<GeoTrait *>(traits.at(i));
        if (gt)
          _traitVect.push_back(new GeoTrait(*gt));
        else
          _traitVect.push_back(new Trait(*(traits.at(i))));
      }
      
      _activeTraits = Traits;
      _toggleTraitAct->setText(tr("Use GeoTags &groups"));
      _toggleTraitAct->setEnabled(false);
        _traitGroups = &_groupVect;
    }
    
    else if (parser->hasGeoTags())
    {
      const vector<GeoTrait*> & gtv = parser->geoTraitVector();
      for (unsigned i = 0; i < gtv.size(); i++)
        _traitVect.push_back(new GeoTrait(*(gtv.at(i))));

      _activeTraits = GeoTags;
      _toggleTraitAct->setText(tr("Use Traits &groups"));
      _toggleTraitAct->setEnabled(false);
        _traitGroups = &_geoGroupVect;
    }
    
    else
      return false;
  }
  
  catch (SeqParseError &spe)
  {
    _traitVect.clear();
    QMessageBox error;
    error.setIcon(QMessageBox::Critical);
    error.setText(tr("<b>Error parsing traits</b>"));
    error.setDetailedText(spe.what());
    error.setStandardButtons(QMessageBox::Ok);
    error.setDefaultButton(QMessageBox::Ok);
    error.exec();

    //_errorMessage.showMessage("Error parsing traits.");
    return false;
  }
  
  return true;
}

bool HapnetWindow::loadTreesFromParser(vector<ParsimonyTree *> & treevect)
{
  
  NexusParser *parser = dynamic_cast<NexusParser *>(Sequence::parser());
  
  if (! parser)  return false;

  if (parser->treeVector().empty())  return false;
  vector<Tree *>::const_iterator treeit = parser->treeVector().begin();
  

  try
  {
    while (treeit != parser->treeVector().end())
    {
      treevect.push_back(new ParsimonyTree(**treeit));
      ++treeit;
    }
  }
  
  catch (TreeError te)
  {
    QMessageBox error;
    error.setIcon(QMessageBox::Critical);
    error.setText(tr("<b>Error in an input tree</b>"));
    error.setDetailedText(te.what());
    error.setStandardButtons(QMessageBox::Ok);
    error.setDefaultButton(QMessageBox::Ok);
    error.exec();

    //_errorMessage.showMessage("Error in an input tree.");
    return false;
  }
  
  catch (SeqParseError spe)
  {
    QMessageBox error;
    error.setIcon(QMessageBox::Critical);
    error.setText(tr("<b>Error parsing trees</b>"));
    error.setDetailedText(spe.what());
    error.setStandardButtons(QMessageBox::Ok);
    error.setDefaultButton(QMessageBox::Ok);
    error.exec();

    //_errorMessage.showMessage("Error parsing trees.");
    return false;
  }
  
  return true;
}

void HapnetWindow::closeAlignment()
{
  if (! _alignment.empty())
  {
    /*_alModel->removeColumns(0, _alModel->columnCount());
    _alModel->removeRows(0, _alModel->rowCount());*/
    setWindowTitle("PopART");
    
    for (unsigned i = 0; i < _alignment.size(); i++)
      delete _alignment.at(i);
    
    _alignment.clear();
    _goodSeqs.clear();
    _mask.clear();
    _badSeqs.clear();
    
    for (unsigned i = 0; i < _treeVect.size(); i++)
      delete _treeVect.at(i);
    _treeVect.clear();
    

    QAbstractItemModel *tmpModel = _alView->model();
    _alView->setModel(0);
    delete tmpModel;
    _alModel = 0;
    
    /*tmpModel = _tView->model();
    //_tModel = new TraitModel(_traitVect);
    _tView->setModel(0);//_tModel);
    delete tmpModel;
    _tModel = 0;*/

    toggleAlignmentActions(false);
    
    _netView->clearModel();
    toggleNetActions(false);
    delete _netModel;
    _netModel = 0;
    
    _history->clear();

  }
  
  if (_stats) 
  {
    delete _stats;
    _stats = 0;
  }
  
  if (! _treeVect.empty())
    closeTrees();

}

void HapnetWindow::closeTrees()
{
  for (unsigned i = 0; i < _treeVect.size(); i++)
    delete _treeVect.at(i);
  _treeVect.clear();
}


void HapnetWindow::closeTraits()
{
  toggleTraitActions(false);
  _traitGroupsSet = false;
  _geoGroupVect.clear();
  _groupVect.clear();

  if (! _traitVect.empty())
  {
    for (unsigned i = 0; i < _traitVect.size(); i++)
      delete _traitVect.at(i);
    _traitVect.clear();
  }

  QAbstractItemModel *tmpModel = _tView->model();
    //_tModel = new TraitModel(_traitVect);
    _tView->setModel(0);//_tModel);
    delete tmpModel;
    _tModel = 0;
}

void HapnetWindow::askAndCloseAlignment()
{  
  QDialog dlg(this);
  
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  
  vlayout->addWidget(new QLabel("Clear alignment data?", this));

  QHBoxLayout *hlayout = new QHBoxLayout;
  
  hlayout->addStretch(1);
  QPushButton *yesButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogYesButton), "Yes", &dlg);
  connect(yesButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(yesButton, 0, Qt::AlignRight);
  QPushButton *noButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogNoButton), "No", &dlg);
  connect(noButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(noButton, 0, Qt::AlignRight);
  vlayout->addLayout(hlayout);

  
  int result = dlg.exec();
  
  if (result == QDialog::Accepted)
    closeAlignment();
}

void HapnetWindow::askAndCloseTraits()
{  
  QDialog dlg(this);
  
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  
  vlayout->addWidget(new QLabel("Clear traits data?", this));

  QHBoxLayout *hlayout = new QHBoxLayout;
  
  hlayout->addStretch(1);
  QPushButton *yesButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogYesButton), "Yes", &dlg);
  connect(yesButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(yesButton, 0, Qt::AlignRight);
  QPushButton *noButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogNoButton), "No", &dlg);
  connect(noButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(noButton, 0, Qt::AlignRight);
  vlayout->addLayout(hlayout);

  
  int result = dlg.exec();
  
  if (result == QDialog::Accepted)
    closeTraits();
}

void HapnetWindow::importAlignment()
{
  QString phylipname = QFileDialog::getOpenFileName(this, "Open alignment file", tr("."), "Phylip files (*.phy *.seq *.phylip);;All Files(*)");

  if (phylipname != "")
  {
    if (! _alignment.empty())
       closeAlignment();
    
    if (! _traitVect.empty())
      askAndCloseTraits();

    bool success = loadAlignmentFromFile(phylipname);

    if (success)
    {
      _filename = QString("%1.%2").arg(phylipname).arg("nex");
      statusBar()->showMessage(tr("Imported file %1").arg(phylipname));
      //_networkMenu->setEnabled(true);

      QFileInfo fileInfo(phylipname);
      setWindowTitle(tr("PopART: %1").arg(fileInfo.fileName()));
      toggleAlignmentActions(true);

      if (! _alModel)
      {
        _alModel = new AlignmentModel(_alignment);
        _alView->setModel(_alModel);

        _alModel->setCharType(_datatype);

        if (! _mask.empty())
        {
          for (unsigned i = 0; i < _mask.size(); i++)
            if (! _mask.at(i))
              _alModel->maskColumns(i, i);
        }

        if (! _badSeqs.empty())
        {
          for (unsigned i = 0; i < _badSeqs.size(); i++)
            _alModel->maskRows(_badSeqs.at(i), _badSeqs.at(i));
        }


      }

      else
      {
        _netView->clearModel();
        toggleNetActions(false);
        QAbstractItemModel *tmpModel = _alView->model();
        _alModel = new AlignmentModel(_alignment);
        _alView->setModel(_alModel);
        _alModel->setCharType(_datatype);
        delete tmpModel;
        //_alView->resizeColumnsToContents();
      }
    _dataWidget->setCurrentWidget(_alView);
    }
  }

  else
    statusBar()->showMessage(tr("No file selected"));
}

void HapnetWindow::importTraits()
{
  QString filename = QFileDialog::getOpenFileName(this, "Open traits table", tr("."), "Table files (*.csv *.txt);;All Files(*)");
    
  if (filename != "")
  {      
    if (! _alignment.empty())
      askAndCloseAlignment();
    
    if (! _traitVect.empty())
      closeTraits();

    _toggleTraitAct->setEnabled(false);

    _tp = new TableParser();
    bool success = loadTableFromFile(filename);
    if (success)
    {    
      bool warned = false;
      set<string> seqnames;
      set<string> alseqnames;
      if (! _alignment.empty())
      {
        for (unsigned i = 0; i < _alignment.size(); i++)
          alseqnames.insert(_alignment.at(i)->name());
      }

      for (unsigned i = 0; i < _tp->columns(); i++)
      {
        if(_tp->headerData().empty())
        {
          ostringstream oss;
          oss << "trait" << (i+1);
          _traitVect.push_back(new Trait(oss.str()));
        }
        else
          _traitVect.push_back(new Trait(_tp->headerData().at(i)));
        
        // set data type to integer for trait data
        _tp->setDataType(i, 'd');
      }

      for (unsigned i = 0; i < _tp->rows(); i++)
      {
        pair<set<string>::iterator,bool> result = seqnames.insert(_tp->vHeaderData(i));
        if (! warned && ! alseqnames.empty())
        {
           set<string>::iterator sit = alseqnames.find(*(result.first));
           
           if (sit == alseqnames.end())
           {
             QString text(tr("<b>Sequence %1 appears in traits file but not alignment</b>").arg(QString::fromStdString(*(result.first))));
             QString infText("If traits file does not correspond to alignment, network inference will produce errors.");
             showWarnDlg(text, QString(), infText);
            
             warned = true;

           }
        }
        
        if (! result.second) // sequence name repeated in file
        {
          QString text(tr("<b>Sequence %1 appears more than once!</b>").arg(QString::fromStdString(*(result.first))));
          QString detText("Duplicate sequence names should be merged into one line");
          showErrorDlg(text, detText);
          
          closeTraits();
          delete _tp;
          
          return;
          
        }
        
        for (unsigned j = 0; j < _tp->columns(); j++)
        {
          // TODO remove whitespace from entries after reading!!!
          try
          {
            unsigned count = (unsigned)(_tp->dataInt(i, j));
            if (count > 0)
              _traitVect.at(j)->addSeq(*(result.first), count);
          } 
          
          catch (SeqParseError &spe)
          {
            QString text("<b>Error importing traits</b>");
            QString detText(spe.what());
            showErrorDlg(text, detText);
            
            closeTraits();
            delete _tp;
            
            return;
          }

        }
      }
      
      _activeTraits = Traits;
      _traitGroups = &_groupVect;

      QAbstractItemModel *tmpModel = _tView->model();
      _tModel = new TraitModel(_traitVect);
      _tView->setModel(_tModel);
      delete tmpModel;

      if (_view == Map)
      {
        _mapView->addHapLocations(_traitVect);
        _mapTraitsSet = true;
      }

      else
        _mapTraitsSet = false;

    _dataWidget->setCurrentWidget(_tView);
    }

    delete _tp;
    
  }
  
  else 
  {
    statusBar()->showMessage(tr("No file selected"));
  }
}

void HapnetWindow::importGeoTags()
{
  QString filename = QFileDialog::getOpenFileName(this, "Open geotag table", tr("."), "Table files (*.csv *.txt);;All Files(*)");
    
  // TODO ask user for number, location of clusters?
  if (filename != "")
  {      
    if (! _alignment.empty())
      askAndCloseAlignment();
    
    if (! _traitVect.empty())
      closeTraits();

    _toggleTraitAct->setEnabled(false);
    _tp = new TableParser();
    bool success = loadTableFromFile(filename);
    
    if (success)
    { 
      //cout << "columns: " << _tp->columns() << endl;
      if (_tp->columns() == 3)
        _tp->setDataType(2, 'd');

      else if (_tp->columns() != 2)
      {
        QString text("<b>Wrong number of columns in geotags table</b>");
        QString detText("Each entry in the geotags file should consist of a sequence name, latitude, longitude, and (optionally) count");
        showErrorDlg(text, detText);
        
        delete _tp;
        return;
      }  
      
      vector<string> seqnames;
      vector<pair<float,float> > coordinates;
      vector<unsigned> seqcounts;
     
      set<string> alseqnames;
      bool warned = false;
      if (! _alignment.empty())
      {
        for (unsigned i = 0; i < _alignment.size(); i++)
          alseqnames.insert(_alignment.at(i)->name());
      }
            
      for (unsigned i = 0; i < _tp->rows(); i++)
      {
        string name = _tp->vHeaderData(i);
        if (! warned && ! alseqnames.empty())
        {
           set<string>::iterator sit = alseqnames.find(name);
           
           if (sit == alseqnames.end())
           {
             QString text(tr("<b>Sequence %1 appears in traits file but not alignment</b>").arg(QString::fromStdString(name)));
             QString infText("If traits file does not correspond to alignment, network inference will produce errors.");
             showWarnDlg(text, QString(), infText);
            
             warned = true;
           }
        }
        
        seqnames.push_back(name);
        const string & latStr = _tp->data(i,0);
        const string & lonStr = _tp->data(i,1);
        coordinates.push_back(GeoTrait::getCoordinate(latStr, lonStr));
        
        unsigned count = 1;
        if (_tp->columns() == 3)
          count = (unsigned)(_tp->dataInt(i, 2));
        if (count == 0)
          count = 1;
        
        seqcounts.push_back(count);
          
      }
      
      QDialog dlg(this);
      QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
      QHBoxLayout *hlayout = new QHBoxLayout;
      
      QLabel *label = new QLabel("Number of location clusters (0 to estimate, very slow!):", &dlg);
      hlayout->addWidget(label);
      
      QSpinBox *spinBox = new QSpinBox(&dlg);
      //spinBox->setRange(0, 10);
      spinBox->setMinimum(0);
      spinBox->setValue(5);
      hlayout->addWidget(spinBox);
      
      vlayout->addLayout(hlayout);
      
      hlayout = new QHBoxLayout;
      
      hlayout->addStretch(1);
      QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
      connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
      hlayout->addWidget(okButton, 0, Qt::AlignRight);
      QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
      connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
      hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
      
      vlayout->addLayout(hlayout);
      
      int result = dlg.exec();
      
      if (result == QDialog::Rejected)
      {
        delete _tp;
        return;
      }
      
      unsigned nclusts = spinBox->value();
      
      //vector<GeoTrait*> geoTraits = GeoTrait::clusterSeqs(coordinates, seqnames, seqcounts, nclusts);
      
      GeoTrait::statTrait->setupStaticData(coordinates, seqnames, seqcounts, nclusts);
      
      
      connect(GeoTrait::statTrait, SIGNAL(progressUpdated(int)), _progress, SLOT(setValue(int)));
      connect(_clusterThread, SIGNAL(started()), GeoTrait::statTrait, SLOT(processClustering()));
      _progress->setValue(0);
      _progress->setLabelText("Clustering...");
      _progress->show();
      
      GeoTrait::statTrait->moveToThread(_clusterThread);
      _clusterThread->start();
    }

  }
  
  else 
  {
    statusBar()->showMessage(tr("No file selected"));
  }
}

void HapnetWindow::updateTraitLocation(unsigned traitIdx, pair<float,float> location)
{
  GeoTrait *gt = dynamic_cast<GeoTrait *>(_traitVect.at(traitIdx));
  
  if (gt)
    gt->setLocation(location);
  else
  {
    gt = new GeoTrait(location, *(_traitVect.at(traitIdx)));
    delete _traitVect.at(traitIdx);
    _traitVect.at(traitIdx) = gt;
  }
  
  // update location in parser
  NexusParser * parser = dynamic_cast<NexusParser *>(Sequence::parser());
  
  if (parser)
  {
    if (_activeTraits == Traits)
      parser->setTraitLocation(traitIdx, location);
    
    else
      parser->setGeoTraitLocation(traitIdx, location);
  }
}

void HapnetWindow::finaliseClustering()
{   
  const vector<GeoTrait*> & geoTraits = GeoTrait::statTrait->getClusterResult();
  
  _traitVect.assign(geoTraits.begin(), geoTraits.end());
  
  _activeTraits = GeoTags;
  _traitGroups = &_geoGroupVect;
  
  QAbstractItemModel *tmpModel = _tView->model();
  _tModel = new TraitModel(_traitVect);
  _tView->setModel(_tModel);
  delete tmpModel;
  
  if (_view == Map)
  {
    _mapView->addHapLocations(_traitVect);
    _mapTraitsSet = true;
  }
  
  else
    _mapTraitsSet = false;
  
  _dataWidget->setCurrentWidget(_tView);
  
  delete _tp;
}


bool HapnetWindow::loadTableFromFile(const QString &filename)
{
  const char *cstr = filename.toLatin1().constData();

  ifstream tabfile(cstr);
  _tabfile = &tabfile;

  //_tp = new TableParser();

  _tp->setHasHeader(true);
  _tp->setHasVHeader(true);
  _tp->setMergeDelims(true);
  // Do this in another function so that we can connect table interaction checkboxes to parser


  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);

  QHBoxLayout *hlayout = new QHBoxLayout;

  QButtonGroup *radioButtons = new QButtonGroup(this);
  int idCount = 0;

  QVBoxLayout *vlayout2 = new QVBoxLayout;
  hlayout->addWidget(new QLabel("Delimiter:", &dlg));

  QAbstractButton *button = new QRadioButton("Comma", &dlg);
  button->setChecked(true);
  radioButtons->addButton(button, idCount++);
  vlayout2->addWidget(button);

  button = new QRadioButton("Space", &dlg);
  radioButtons->addButton(button, idCount++);
  vlayout2->addWidget(button);

  button = new QRadioButton("Tab", &dlg);
  radioButtons->addButton(button, idCount++);
  vlayout2->addWidget(button);

  connect(radioButtons, SIGNAL(buttonClicked(int)), this, SLOT(changeDelimiter(int)));
  hlayout->addLayout(vlayout2);

  vlayout2 = new QVBoxLayout;

  button = new QCheckBox("Merge consecutive delimiters", &dlg);
  button->setChecked(true);
  connect(button, SIGNAL(toggled(bool)), this, SLOT(setMergeDelims(bool)));
  vlayout2->addWidget(button);

  button = new QCheckBox("Table has header row", &dlg);
  button->setChecked(true);
  connect(button, SIGNAL(toggled(bool)), this, SLOT(setHasHeader(bool)));
  //button->setEnabled(false);
  vlayout2->addWidget(button);

  /*button = new QCheckBox("Table has header column", &dlg);
  button->setChecked(true);
  connect(button, SIGNAL(toggled(bool)), this, SLOT(setHasVHeader(bool)));
  button->setEnabled(false);
  vlayout2->addWidget(button);*/

  hlayout->addLayout(vlayout2);
  vlayout->addLayout(hlayout);

  // TODO check for vertical header
  // Use a delegate to colour columns by data type?
  // Allow user to set data type?
  // Fix column widths (too wide by default)
  // Add checkboxes to change delimiters, etc.
  _table = new QTableWidget(&dlg);
  updateTable();
  //table->setRowCount(tp.rows());
  //table->setColumnCount(tp.columns());

  vlayout->addWidget(_table);


  hlayout = new QHBoxLayout;


  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);

  vlayout->addLayout(hlayout);

  int result = dlg.exec();


 bool cancelled = false;
 if (result == QDialog::Rejected)
   cancelled = true;

  // return parse result


  /*tp.readTable(tabfile);

  const vector<vector<string> > & data = tp.data();

  for (unsigned i = 0; i < data.size(); i++)
  {
    for (unsigned j = 0; j < data.at(i).size(); j++)
      cout << data.at(i).at(j) << " ";
    cout << "\n";
  }*/

  tabfile.close();

  delete _table;

  return ! cancelled;
}

void HapnetWindow::updateTable()
{
  _tabfile->clear();
  _tabfile->seekg(ios::beg);
  
  _table->clear();
  _tp->readTable(*_tabfile);

  const vector<vector<string> > & data = _tp->data();

  QStringList headerList;

  //qDebug() << "header list:" << headerList;

  _table->setRowCount(_tp->rows());
  _table->setColumnCount(_tp->columns());

  if (_tp->hasHeader())
  {
    const vector<string> & header = _tp->headerData();
    for (unsigned i = 0; i < header.size(); i++)
      headerList << QString::fromStdString(header.at(i));
    _table->setHorizontalHeaderLabels(headerList);
  }

  if (_tp->hasVHeader())
  {
    headerList.clear();
    const vector<string> & header = _tp->vHeaderData();
    for (unsigned i = 0; i < header.size(); i++)
      headerList << QString::fromStdString(header.at(i));
    _table->setVerticalHeaderLabels(headerList);
  }

  for (unsigned i = 0; i < data.size(); i++)
  {
    for (unsigned j = 0; j < data.at(i).size(); j++)
    {
      /*if (hasVheader)
      {
        if (j == 0)
          _table->setVerticalHeaderItem(i, new QTableWidgetItem(QString::fromStdString(data.at(i).at(j))));
        else
        {
          QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(data.at(i).at(j)));
          item->setFlags(Qt::NoItemFlags);
          _table->setItem(i, j - 1, item);
        }
      }

      else
      {*/
        QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(data.at(i).at(j)));
        item->setFlags(Qt::NoItemFlags);
        _table->setItem(i, j, item);
      //}
    }
  }

  //qDebug() << "table dimensions:" << _table->rowCount() << _table->columnCount();


  _table->resizeColumnsToContents();


}

void HapnetWindow::changeDelimiter(int buttonIdx)
{
  switch(buttonIdx)
  {
    case 0:
      _tp->setDelimChar(',');
      break;
    case 1:
      _tp->setDelimChar(' ');
      break;
    case 2:
      _tp->setDelimChar('\t');
      break;
    default:
      qDebug() << "this shouldn't happen";
      break;
  }

  updateTable();
}

void HapnetWindow::setMergeDelims(bool merge)
{
  _tp->setMergeDelims(merge);
  updateTable();
}

void HapnetWindow::setHasHeader(bool hasHeader)
{
  _tp->setHasHeader(hasHeader);
  updateTable();
}

void HapnetWindow::setHasVHeader(bool hasVHeader)
{
  _tp->setHasVHeader(hasVHeader);
  updateTable();
}

void HapnetWindow::saveNexusFile()
{

  QString defaultName(_filename);

  if (! defaultName.endsWith(".nex", Qt::CaseInsensitive))
    defaultName.append(".nex");

  QString filename = QFileDialog::getSaveFileName(this, tr("Save Network"), tr("./%1").arg(defaultName), tr("Nexus files (*.nex *.mac)"));

  if (! filename.isEmpty())
  {
    const char *cstr = filename.toLatin1().constData();

    ofstream nexfile(cstr);

    if (! writeNexusFile(nexfile))
    {
      statusBar()->showMessage("File not saved.");
      showErrorDlg("Error saving Nexus file.");
    }

    nexfile.close();
  }
}

bool HapnetWindow::writeNexusFile(ostream &nexfile)
{
  if (_alignment.size() <  1)  return false;

  if (! writeNexusAlignment(nexfile))
    return false;

  if (! writeNexusTraits(nexfile))
    return false;

  if (! writeNexusTrees(nexfile))
    return false;

  if (! writeNexusNetwork(nexfile))
    return false;

  return true;
}

bool HapnetWindow::writeNexusAlignment(ostream &nexfile)
{

  SeqParser *sp = Sequence::parser();

  NexusParser *nexParser = dynamic_cast<NexusParser*>(sp);
  if (! nexParser)
  {
    nexParser = new NexusParser();
    nexParser->setNchar(_alignment.at(0)->length());
    nexParser->setNseq(_alignment.size());
    switch (_datatype)
    {
      case Sequence::AAType:
        nexParser->setCharType(SeqParser::AAType);
        break;
      case Sequence::DNAType:
        nexParser->setCharType(SeqParser::DNAType);
        break;
      case Sequence::BinaryType:
        nexParser->setCharType(SeqParser::StandardType);
        break;
      default:
        return false;
        break;
    }

    if (sp)
    {
      delete sp;
      Sequence::setParser(nexParser);
    }
  }

  try
  {
    for (unsigned i = 0; i < _alignment.size(); i++)
      nexfile << *(_alignment.at(i));
  }
  catch (SequenceError &)
  {
    return false;
  }

  nexfile << endl;

  return true;
}

bool HapnetWindow::writeNexusTraits(ostream &nexfile)
{
  if (_traitVect.empty())
    return true;

  NexusParser *nexParser = dynamic_cast<NexusParser*>(Sequence::parser());

  if (! nexParser)
  {
    //return false;
    GeoTrait *gt = dynamic_cast<GeoTrait*>(_traitVect.at(0));
    
    if (gt)
    {
      vector<GeoTrait *> gtv;
      
      for (unsigned i = 0; i < _traitVect.size(); i++)
        gtv.push_back(dynamic_cast<GeoTrait *>(_traitVect.at(i)));
      
      writeGeoData(nexfile, gtv);
    }
    
    else
      writeTraitData(nexfile, _traitVect);
    
  }

  if (_activeTraits == Traits)
  {

    if (! writeTraitData(nexfile, _traitVect))
      return false;


    if (nexParser && nexParser->hasGeoTags())
    {
      const vector<GeoTrait*> & gtv = nexParser->geoTraitVector();
      if (! writeGeoData(nexfile, gtv))
        return false;
    }
  }

  else
  {

    vector<GeoTrait*> gtv;

    for (unsigned i = 0; i < _traitVect.size(); i++)
    {
      GeoTrait *gt = dynamic_cast<GeoTrait *>(_traitVect.at(i));
      if (! gt)
        return false;

      gtv.push_back(gt);
    }

    if (! writeGeoData(nexfile, gtv))
      return false;

    if (nexParser && nexParser->hasTraits())
      if (! writeTraitData(nexfile, nexParser->traitVector()))
        return false;
  }


  return true;
}

bool HapnetWindow::writeTraitData(ostream &nexfile, const vector<Trait *> &traits)
{
  /*NexusParser *nexParser = dynamic_cast<NexusParser*>(Sequence::parser());

  if (! nexParser)
    return false;*/

  nexfile << "Begin Traits;" << endl;
  nexfile << "Dimensions NTraits=" << traits.size() << ';' << endl;
  nexfile << "Format labels=yes missing=? separator=Comma;" << endl;

  vector<GeoTrait*> gtvect;

  for (unsigned i = 0; i < traits.size(); i++)
  {
    GeoTrait *gt = dynamic_cast<GeoTrait*>(traits.at(i));
    if (gt)
      gtvect.push_back(new GeoTrait(*gt));

    else
    {
      gtvect.clear();
      break;
    }
  }

  if (! gtvect.empty())
  {

    nexfile << "TraitLatitude";
    for (unsigned i = 0; i < gtvect.size(); i++)
    {
      nexfile << ' ' << gtvect.at(i)->latitude();
    }
    nexfile << ';' << endl;
    nexfile << "TraitLongitude";
    for (unsigned i = 0; i < gtvect.size(); i++)
    {
      nexfile << ' ' << gtvect.at(i)->longitude();
    }
    nexfile << ';' << endl;

  }

  // geo data
  nexfile << "TraitLabels";

  for (unsigned i = 0; i < traits.size(); i++)
    nexfile << ' ' << traits.at(i)->name();

  nexfile << ";\nMatrix" << endl;

  for (unsigned i = 0; i < _alignment.size(); i++)
  {
    string seqname = _alignment.at(i)->name();
    nexfile << seqname << ' ';
    for (unsigned j = 0; j < traits.size(); j++)
    {

      if (j > 0)
        nexfile << ',';

      try
      {
        nexfile << traits.at(j)->seqCount(seqname);
      }

      catch (SequenceError &)
      {
        nexfile << 0;
      }
    }
    nexfile << endl;
  }
  nexfile << ";\nEnd;\n" << endl;

  return true;
}


bool HapnetWindow::writeGeoData(ostream &nexfile, const vector<GeoTrait *> &geotraits)
{
  /*NexusParser *nexParser = dynamic_cast<NexusParser*>(Sequence::parser());

  if (! nexParser)
    return false;*/

  nexfile << "Begin GeoTags;" << endl;
  nexfile << "Dimensions NClusts=" << geotraits.size() << ';' << endl;
  nexfile << "Format labels=yes separator=Comma;" << endl;

  if (! geotraits.empty())
  {

    nexfile << "ClustLatitude";
    for (unsigned i = 0; i < geotraits.size(); i++)
    {
      nexfile << ' ' << geotraits.at(i)->latitude();
    }
    nexfile << ';' << endl;
    nexfile << "ClustLongitude";
    for (unsigned i = 0; i < geotraits.size(); i++)
    {
      nexfile << ' ' << geotraits.at(i)->longitude();
    }
    nexfile << ';' << endl;

  }

  // geo data
  nexfile << "ClustLabels";

  for (unsigned i = 0; i < geotraits.size(); i++)
    nexfile << ' ' << geotraits.at(i)->name();

  nexfile << ";\nMatrix" << endl;



  for (unsigned i = 0; i < _alignment.size(); i++)
  {
    string seqname = _alignment.at(i)->name();
    for (unsigned j = 0; j < geotraits.size(); j++)
    {
      try
      {
        vector<unsigned> counts = geotraits.at(j)->seqCounts(seqname);
        vector<pair<float,float> > locations = geotraits.at(j)->seqLocations(seqname);

        if (counts.size() != locations.size())
          return false;

        for (unsigned k = 0; k < counts.size(); k++)
        {
          nexfile << seqname << '\t' << locations.at(k).first << ',' << locations.at(k).second << ',';
          nexfile << counts.at(k) << ',' << (j + 1) << endl;
        }
      }

      catch (SequenceError &) // do nothing, but ignore trait
      { }
    }
  }
  nexfile << ";\nEnd;\n" << endl;

  return true;
}

bool HapnetWindow::writeNexusTrees(ostream &nexfile)
{

  if (_treeVect.empty())  return true;

  nexfile << "Begin Trees;" << endl;
  for (unsigned i = 0; i < _treeVect.size(); i++)
    nexfile << "tree POPART_" << (i+1) << " = " << *(_treeVect.at(i)) << endl;

  nexfile << "End;\n" << endl;

  return true;
}
  
bool HapnetWindow::writeNexusNetwork(ostream &nexfile)
{
  vector<string> vertLabels;
  
  
  // TODO get vertex labels from _netView
  for (unsigned i = 0; i < _g->vertexCount(); i++)
  {
    vector<string> identical = _g->identicalTaxa(i);
    if (identical.size() > 0)
      vertLabels.push_back(identical.at(0));
  }
  

  nexfile << "Begin Network;" << endl;
  nexfile << "Dimensions ntax=" << vertLabels.size() << " nvertices=" << _g-> vertexCount() << " nedges=" << _g->edgeCount() << ' ';

  QRectF plotRect = _netView->sceneRect();
  nexfile << "plotDim=" << plotRect.x() << ',' << plotRect.y() << ',' << plotRect.width() << ',' << plotRect.height();
  nexfile << ';' << endl;
  //nexfile << "DRAW to_scale;" << endl;
  nexfile << "Format ";
  
  nexfile << "Font=" << _netView->labelFont().toString().toStdString() << ' ';
  nexfile << "LegendFont=" << _netView->legendFont().toString().toStdString() << ' ';
  QColor col = _netView->vertexColour();
  nexfile << setfill('0');
  nexfile << setbase(16);
  nexfile << "VColour=" << col.name().toStdString() << setw(2) << col.alpha() << ' ';
  col = _netView->edgeColour();
  nexfile << "EColour=" << col.name().toStdString() << setw(2) << col.alpha() << ' ';
  col = _netView->backgroundColour();
  nexfile << "BGColour=" << col.name().toStdString() << setw(2) << col.alpha() << ' ';
  
  nexfile << setfill(' ');
  nexfile << setbase(10);
  
  nexfile << "VSize=" << _netView->vertexSize() << ' ';
  nexfile << "EView=";
  
  switch (_netView->edgeMutationView())
  {
    case EdgeItem::ShowDashes:
      nexfile << "Dashes ";
      break;
    case EdgeItem::ShowEllipses:
      nexfile << "Ellipses ";
      break;
    case EdgeItem::ShowNums:
    default:
      nexfile << "Numbers ";
      break;
  }
  
   
  nexfile << setfill('0');
  nexfile << setbase(16);
  
  QPointF legendPos = _netView->legendPosition();
  nexfile << "LPos=" << legendPos.x() << ',' << legendPos.y() << ' ';
  nexfile << "LColours=";
  bool first = true;
  for (QList<QColor> legendCols = _netView->traitColours(); ! legendCols.isEmpty(); legendCols.pop_front())
  {
    if (first)
      first = false;
    else
      nexfile << ',';
    nexfile << legendCols.front().name().toStdString();
    nexfile << setw(2) << legendCols.front().alpha();
  }
  
  nexfile << ';' << endl;
   
  nexfile << setfill(' ');
  nexfile << setbase(10);

  
  nexfile << "Translate" << endl;
  
  for (unsigned i = 0; i < vertLabels.size(); i++)
  {
    nexfile << (i + 1) << ' ' << vertLabels.at(i) << ',' << endl;
  }
  
  nexfile << ';' << endl;
  
  nexfile << "Vertices" << endl;
  for (unsigned i = 0; i < _g->vertexCount(); i++)
  {
    QPointF vertPos = _netView->vertexPosition(i);
    nexfile << (i + 1) << ' ' << vertPos.x() << ' ' << vertPos.y() << ',' << endl;
  }
  
  nexfile << ';' << endl;
  
  nexfile << "VLabels" << endl;
  for (unsigned i = 0; i < _g->vertexCount(); i++)
  {
    QPointF labPos = _netView->labelPosition(i);
    nexfile << (i + 1) << ' ' << labPos.x() << ' ' << labPos.y() << ',' << endl;
  }
  
  nexfile << ';' << endl;
    
  nexfile << "Edges" << endl;
  for (unsigned i = 0; i < _g->edgeCount(); i++)
  {
    const Edge *e = _g->edge(i);
    nexfile << (i + 1) << ' ' << e->from()->index() << ' ' << e->to()->index() << ',' << endl;
  } 
  
  nexfile << ';' << endl;
  
  nexfile << "End;" << endl;

  return true;
}

void HapnetWindow::exportNetwork()
{
  if (! _g)
  {
    QMessageBox warnBox;
    warnBox.setIcon(QMessageBox::Warning);
    warnBox.setText(tr("<b>No network to export</b>"));
    warnBox.setStandardButtons(QMessageBox::Ok);
    warnBox.setDefaultButton(QMessageBox::Ok);
    warnBox.exec();

    //_errorMessage.showMessage("Error, no network to export.");
  }
  
  QString defaultName(_filename);
  
  if (defaultName.endsWith(".nex", Qt::CaseInsensitive))
    defaultName.replace(defaultName.length() - 3, 3, "txt"); 
  else
    defaultName.append(".txt");
  
  QString filename = QFileDialog::getSaveFileName(this, tr("Save Network"), tr("./%1").arg(defaultName), tr("Tab-delimited text files (*.txt);;Comma-separated value files (*.csv)"));

  if (filename.isEmpty())  return;
  QFile file(filename);
  if (! file.open(QIODevice::WriteOnly | QIODevice::Text))
     return;

  char separator = '\t';
  QTextStream out(&file);
  if (filename.endsWith(".csv", Qt::CaseInsensitive))
    separator = ',';
  
  Edge *e;
  for (unsigned i = 0; i < _g->edgeCount(); i++)
  {
    e = _g->edge(i);
    QString label = QString::fromStdString(e->from()->label());
    if (label.isEmpty())
      out << e->from()->index();
    else
      out << label;
    
    out << separator << e->weight() << separator;
    
    label = QString::fromStdString(e->to()->label());
    if (label.isEmpty())
      out << e->to()->index();
    else
      out << label;
    
    out << "\n";
  }
  
  file.close();
}

void HapnetWindow::saveGraphics()
{
  QString defaultName(_filename);
  
  if (defaultName.endsWith(".nex", Qt::CaseInsensitive))
    defaultName.replace(defaultName.length() - 3, 3, "svg"); 
  else
    defaultName.append(".svg");
  
  QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), tr("./%1").arg(defaultName), tr("Scalable Vector Graphics files (*.svg);;Portable Network Graphics files (*.png);;Portable Document Format (*.pdf)"));

  if (filename.isEmpty())  return;
  
  if (filename.endsWith(".svg", Qt::CaseInsensitive))
  {
    if (_view == Net)
      _netView->saveSVGFile(filename);
    else if (_view == Map)
      _mapView->saveSVGFile(filename);
  }
    
  else if (filename.endsWith(".png", Qt::CaseInsensitive))
  {
    if (_view == Net)
      _netView->savePNGFile(filename);
    else if (_view == Map)
      _mapView->savePNGFile(filename);
  }
  
  else if (filename.endsWith(".pdf", Qt::CaseInsensitive))
  {
    if (_view == Net)
      _netView->savePDFFile(filename);
    else if (_view == Map)
      _mapView->savePDFFile(filename);
  }
  
  else
  {
     //_errorMessage.showMessage("Unsupported graphics file type.");
    QMessageBox warnBox;
    warnBox.setIcon(QMessageBox::Warning);
    warnBox.setText(tr("<b>Unsupported graphics format</b>"));
    warnBox.setStandardButtons(QMessageBox::Ok);
    warnBox.setDefaultButton(QMessageBox::Ok);
    warnBox.exec();

  }
  
}

void HapnetWindow::quit()
{
  closeAlignment();
  closeTraits();
  qApp->quit();
}

void HapnetWindow::buildMSN()
{  
  
  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  QHBoxLayout *hlayout = new QHBoxLayout;
  
  QLabel *label = new QLabel("Epsilon", &dlg);
  hlayout->addWidget(label);
  
  QSpinBox *spinBox = new QSpinBox(&dlg);
  spinBox->setRange(0, 10);
  hlayout->addWidget(spinBox);
  
  vlayout->addLayout(hlayout);
  
  hlayout = new QHBoxLayout;
  
  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);
  
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;
  
  unsigned epsilon = spinBox->value();
  
  toggleNetActions(false);
  toggleAlignmentActions(false);
  _closeAct->setEnabled(false);
  _openAct->setEnabled(false);
  
  _netView->clearModel();

  _progress->setLabelText("Inferring Minimum Spanning Network...");

  inferNetwork(Msn, epsilon);
}

void HapnetWindow::buildMJN()
{
  
  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  QHBoxLayout *hlayout = new QHBoxLayout;
  
  QLabel *label = new QLabel("Epsilon", &dlg);
  hlayout->addWidget(label);
  
  QSpinBox *spinBox = new QSpinBox(&dlg);
  spinBox->setRange(0, 10);
  hlayout->addWidget(spinBox);
  
  vlayout->addLayout(hlayout);
  
  hlayout = new QHBoxLayout;
  
  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);
  
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;

  unsigned epsilon = spinBox->value();
  
  toggleNetActions(false);
  toggleAlignmentActions(false);
  _closeAct->setEnabled(false);
  _openAct->setEnabled(false);
  
  _netView->clearModel();
  
  _progress->setLabelText("Inferring Median Joining Network...");
  inferNetwork(Mjn, epsilon);
}


void HapnetWindow::buildAPN()
{
  
   if (_alignment.size() != _goodSeqs.size())
  {
    QMessageBox message;
    message.setIcon(QMessageBox::Question);
    message.setText(tr("<b>Some input sequences will be ignored</b>"));
    message.setInformativeText(tr("If these sequences appear in the input trees, an error will occur. Continue?"));
    message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    message.setDefaultButton(QMessageBox::No);
    int result = message.exec();
    
    if (result == QMessageBox::No)
      return;
  }
    
  if (! _treeVect.empty())
  {
    QDialog dlg(this);
    QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
    QHBoxLayout *hlayout = new QHBoxLayout;
    
    QLabel *label = new QLabel("Minimum edge frequency:", &dlg);
    hlayout->addWidget(label);
    
    QDoubleSpinBox *spinBox = new QDoubleSpinBox(&dlg);
    spinBox->setRange(0, 1);
    spinBox->setValue(0.05);
    spinBox->setSingleStep(0.1);
    hlayout->addWidget(spinBox);
    
    vlayout->addLayout(hlayout);
    
    hlayout = new QHBoxLayout;
    
    hlayout->addStretch(1);
    QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
    connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
    hlayout->addWidget(okButton, 0, Qt::AlignRight);
    QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
    connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
    hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
    
    vlayout->addLayout(hlayout);
    
    int result = dlg.exec();
    
    if (result == QDialog::Rejected)
      return;
    
    double ancestorFreq = spinBox->value();
    
    toggleNetActions(false);
    toggleAlignmentActions(false);
    _closeAct->setEnabled(false);
    _openAct->setEnabled(false);
    
    _netView->clearModel();

    _progress->setLabelText("Inferring Ancestral Parsimony network...");

    inferNetwork(Apn, ancestorFreq);
   }
  
  else  
  {
    QMessageBox error;
    error.setIcon(QMessageBox::Critical);
    error.setText("<b>Network inference error</b>");
    error.setInformativeText("Ancestral parsimony networks require a trees block in your Nexus file");
    error.setStandardButtons(QMessageBox::Ok);
    error.setDefaultButton(QMessageBox::Ok);
    
    error.exec();
  }
}

void HapnetWindow::buildIntNJ()
{
  
  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  QHBoxLayout *hlayout = new QHBoxLayout;
  
  QLabel *label = new QLabel("Reticulation tolerance", &dlg);
  hlayout->addWidget(label);
  
  QSpinBox *spinBox = new QSpinBox(&dlg);
  spinBox->setRange(0, 10);
  spinBox->setValue(1);
  hlayout->addWidget(spinBox);
  
  vlayout->addLayout(hlayout);
  
  hlayout = new QHBoxLayout;
  
  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);
  
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;

  unsigned epsilon = spinBox->value();
    
  toggleNetActions(false);
  toggleAlignmentActions(false);
  _closeAct->setEnabled(false);
  _openAct->setEnabled(false);
  
  _netView->clearModel();

  _progress->setLabelText("Inferring Integer Neighbor-Joining network...");
  inferNetwork(Inj, epsilon);

}

void HapnetWindow::buildTSW()
{
  toggleNetActions(false);
  toggleAlignmentActions(false);
  _closeAct->setEnabled(false);
  _openAct->setEnabled(false);
  
  _netView->clearModel();

  _progress->setLabelText("Inferring Tight Span Walker network...");

  inferNetwork(Tsw);
}

void HapnetWindow::buildTCS()
{
  toggleNetActions(false);
  toggleAlignmentActions(false);
  _closeAct->setEnabled(false);
  _openAct->setEnabled(false);
  
  _netView->clearModel();

  _progress->setLabelText("Inferring TCS network...");
  inferNetwork(Tcs);
}

void HapnetWindow::buildUMP()
{
  statusBar()->showMessage("UMP network not yet available.");
  //_networkArea->writeToConsole("UMP network not yet available.");
}

void HapnetWindow::graphicsMove(QList<QPair<QGraphicsItem *, QPointF> > itemList)
{
  QUndoCommand *moveCommand = new MoveCommand(itemList);
  _history->push(moveCommand);
}

void HapnetWindow::inferNetwork(HapnetWindow::HapnetType netType, QVariant argument)
{
  if (_g)  delete _g;

  //_success = false;

  QString errorText;
  
  switch(netType)
  {
    case Msn:
      errorText = "Error inferring Minimum Spanning Network!";
      _g = new MinSpanNet(_goodSeqs, _mask, argument.toUInt());
      break;
    case Mjn:
      errorText = "Error inferring Median Joining Network!";
      _g = new MedJoinNet(_goodSeqs, _mask, argument.toUInt());
      break;
    case Tcs:
      errorText = "Error inferring TCS Network!";
      _g = new TCS(_goodSeqs, _mask);
      break;
    case Apn:
      errorText = "Error inferring Ancestral Parsimony Network!";
      _g = new ParsimonyNet(_goodSeqs, _mask, _treeVect, 0, argument.toDouble());
      break;
    case Inj:
      errorText = "Error inferring Integer Neighbor-Joining Network!";
      _g = new IntNJ(_goodSeqs, _mask, argument.toUInt());
      break;
    case Tsw:
      errorText = "Error inferring Tight Span Walker network!";
      _g = new TightSpanWalker(_goodSeqs, _mask);
      break;
    default:
      break;

  }

  _success = true;
  // set error message in case of exception
  _msgText = tr("<b>%1</b>").arg(errorText);
  connect(_g, SIGNAL(progressUpdated(int)), _progress, SLOT(setValue(int)));
  connect(_g, SIGNAL(caughtException(const QString &)), this, SLOT(showNetError(const QString&)));
  connect(_netThread, SIGNAL(started()), _g, SLOT(setupGraph()));
  _progress->show();


  _g->moveToThread(_netThread);
  _netThread->start();


}

void HapnetWindow::printprog(int progress)
{
  cout << "progress: " << progress << endl;
}

void HapnetWindow::showNetError(const QString &msg)
{
  _success = false;
  //cout << "will display message " << msg.toStdString() << endl;
  //_msgDetail = msg;
 
  QMessageBox error;
  error.setIcon(QMessageBox::Critical);
  error.setText(_msgText);
  error.setDetailedText(msg);
  error.setStandardButtons(QMessageBox::Ok);
  error.setDefaultButton(QMessageBox::Ok);
  
  error.exec();

}

void HapnetWindow::displayNetwork()
{
  _progress->hide();

  if (_success)
  {
    //cout <<  *_g << endl;
    try
    {
      _g->associateTraits(_traitVect);
      _netModel = new NetworkModel(_g);
      /*_msgText = tr("<b>Error drawing network.</b>");
      _progress->show();

      _netView->moveToThread(_drawThread);
      _drawThread->start();*/

      _netView->setModel(_netModel);
      
      //connect(_netView, SIGNAL(networkDrawn()), this, SLOT(saveNexusFile()));
    }

    catch (NetworkError &e)
    {
      showNetError(tr(e.what()));
    }
  }
  
  //toggleNetActions(true);
  //toggleAlignmentActions(true);
  //_closeAct->setEnabled(true);
  //_openAct->setEnabled(true);
  //cout << "returning from displayNetwork" << endl;
  
}

/*void HapnetWindow::setModel()
{
  _netView->setModel(_netModel);
}*/

void HapnetWindow::finaliseDisplay()
{
  toggleNetActions(true);
  toggleAlignmentActions(true);
  _closeAct->setEnabled(true);
  _openAct->setEnabled(true);
}


void HapnetWindow::changeColourTheme()
{
  bool changeMap, changeNet;

  // TODO maybe change this? Store colour theme elsewhere, but which one (map or net)?
  ColourTheme::Theme theme = ColourDialog::getColour(this, _netView->colourTheme(), 0, &changeNet, &changeMap);
  

  if (changeNet)
    _netView->setColourTheme(theme);
  if (changeMap)
    _mapView->setColourTheme(theme);
}

void HapnetWindow::changeColour(int colourIdx)
{
  QColor oldColour;
  QString dlgStr;

  if (_view == Net)
  {
    oldColour = _netView->colour(colourIdx);
    dlgStr = tr("Choose a new trait colour");
  }
  else
  {
    oldColour = _mapView->colour(colourIdx);
    dlgStr = tr("Choose a new sequence colour");
  }
  
  QColor newColour = QColorDialog::getColor(oldColour, this, dlgStr);
  
  if (newColour.isValid())
  {
    if (_view == Net)
      _netView->setColour(colourIdx, newColour);
    else
      _mapView->setColour(colourIdx, newColour);
  }
}

void HapnetWindow::changeVertexColour()
{
  const QColor & oldColour = _netView->vertexColour();
  
  QColor newColour = QColorDialog::getColor(oldColour, this, tr("Choose a new default vertex colour"));
  
  if (newColour.isValid())
  _netView->setVertexColour(newColour);
}

void HapnetWindow::changeVertexSize()
{
  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  QHBoxLayout *hlayout = new QHBoxLayout;

  QLabel *label = new QLabel("Choose a new vertex size", &dlg);
  hlayout->addWidget(label);

  QDoubleSpinBox *spinBox = new QDoubleSpinBox(&dlg);
  spinBox->setRange(1, 100);
  spinBox->setSingleStep(1);
  spinBox->setDecimals(2);
  spinBox->setValue(_netView->vertexSize());
  hlayout->addWidget(spinBox);

  vlayout->addLayout(hlayout);

  hlayout = new QHBoxLayout;

  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);

  vlayout->addLayout(hlayout);

  int result = dlg.exec();

  if (result == QDialog::Rejected)
    return;

  _netView->setVertexSize(spinBox->value());
}

void HapnetWindow::changeEdgeColour()
{
  const QColor & oldColour = _netView->edgeColour();
  
  QColor newColour = QColorDialog::getColor(oldColour, this, tr("Choose a new edge colour"));

  if (newColour.isValid())
    _netView->setEdgeColour(newColour);
}

void HapnetWindow::changeEdgeMutationView(QAction *viewAction)
{
  if (viewAction == _dashViewAct)
    _netView->setEdgeMutationView(EdgeItem::ShowDashes);

  else if (viewAction == _nodeViewAct)
    _netView->setEdgeMutationView(EdgeItem::ShowEllipses);

  else if (viewAction == _numViewAct)
    _netView->setEdgeMutationView(EdgeItem::ShowNums);
}


void HapnetWindow::toggleView()
{
  if (_view == Net)
  {
    //_mapView->show();//setVisible(true);
    //setCentralWidget(_mapView);
    //_netView->hide();//setVisible(false);
    _centralContainer->setCurrentIndex(1);

    if (! _mapTraitsSet && _traitVect.size() > 0)
    {
      _mapView->addHapLocations(_traitVect);
      _mapTraitsSet = true;
    }
    _toggleViewAct->setText(tr("Switch to network view"));
    _view = Map;
    _toggleExternalLegendAct->setEnabled(true);
  }

  else
  {
    //_netView->show();//setVisible(true);
    //setCentralWidget(_netView);
    //_mapView->hide(); //setVisible(false);
    _centralContainer->setCurrentIndex(0);

    _toggleViewAct->setText(tr("Switch to map view"));
    _view = Net;
    _toggleExternalLegendAct->setEnabled(false);
  }
}

void HapnetWindow::toggleExternalLegend()
{
  if (_externalLegend)
  {
    _externalLegend = false;    
    _mapView->setExternalLegend(false);
    _toggleExternalLegendAct->setText(tr("Show &legend window"));
  }
  
  else
  {
    _externalLegend = true;
    _mapView->setExternalLegend(true);
    _toggleExternalLegendAct->setText(tr("Show &legend on map"));
  }
}

void HapnetWindow::toggleActiveTraits()
{

  NexusParser *np = dynamic_cast<NexusParser *>(Sequence::parser());

  if (! np)
    showErrorDlg("Both Traits and GeoTags should be loaded from a Nexus file to enable toggling", "Please report this as a bug.");

  if (_activeTraits == Traits)
  {
    _activeTraits = GeoTags;
    _toggleTraitAct->setText(tr("Use Traits &populations"));

    closeTraits();

    const vector<GeoTrait*> & gtv = np->geoTraitVector();
    
    for (unsigned i = 0; i < gtv.size(); i++)
      _traitVect.push_back(new GeoTrait(*(gtv.at(i))));
    
    if (_geoGroupVect.empty())
      _traitGroupsSet = false;
    
    else
      _traitGroupsSet = true;

    _traitGroups = &_geoGroupVect;

  }

  else
  {
    _activeTraits = Traits;
    _toggleTraitAct->setText(tr("Use GeoTags &populations"));
    
    closeTraits();

    const vector<Trait*> &traits = np->traitVector();
    for (unsigned i = 0; i < traits.size(); i++)
    {
      GeoTrait *gt = dynamic_cast<GeoTrait *>(traits.at(i));
      if (gt)
        _traitVect.push_back(new GeoTrait(*gt));
      else
        _traitVect.push_back(new Trait(*(traits.at(i))));
    }
    
    if (_groupVect.empty())
      _traitGroupsSet = false;
    
    else
      _traitGroupsSet = true;

    _traitGroups = &_groupVect;
      
  }
  
  if (_g)
    _g->associateTraits(_traitVect);
  
  QAbstractItemModel *tmpModel = _tView->model();
  _tModel = new TraitModel(_traitVect);
  _tView->setModel(_tModel); 
  delete tmpModel;
  
  if (_view == Map)
  {
    _mapView->addHapLocations(_traitVect);
    _mapTraitsSet = true;
  }
  
  else
    _mapTraitsSet = false;
  
  
}

void HapnetWindow::changeBackgroundColour()
{
  const QColor & oldColour = _netView->backgroundColour();
  
  QColor newColour = QColorDialog::getColor(oldColour, this, tr("Choose a new background colour"));
  
  if (newColour.isValid())
    _netView->setBackgroundColour(newColour);
}

void HapnetWindow::changeLabelFont()
{
  const QFont & oldFont = _netView->labelFont();
  
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, oldFont, this, tr("Choose a new legend font"));
  
  _netView->changeLabelFont(newFont);
}

void HapnetWindow::changeLegendFont()
{
  QFont oldFont;
  
  if (_view == Net)
    oldFont = _netView->legendFont();
  else
    oldFont = _mapView->legendFont();
  
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, oldFont, this, tr("Choose a new legend font"));
  
  if (_view == Net)
    _netView->changeLegendFont(newFont);
  
  else
    _mapView->changeLegendFont(newFont);
}

void HapnetWindow::changeMapTheme(QAction *mapAction)
{
  
  if (mapAction == _plainMapAct)
    _mapView->setTheme("plain");
  
  else if (mapAction == _blueMarbleMapAct)
    _mapView->setTheme("bluemarble");
  
  else if (mapAction == _atlasMapAct)
    _mapView->setTheme("srtm");
  
  else if (mapAction == _osvMapAct)
    _mapView->setTheme("openstreetmap");
  
  else if (mapAction == _cityLightsMapAct)
    _mapView->setTheme("citylights");
  
  else if (mapAction == _oldMapAct)
    _mapView->setTheme("schagen1689");
}

void HapnetWindow::redrawNetwork()
{
  toggleNetActions(false);
  toggleAlignmentActions(false);
  _closeAct->setEnabled(false);
  _openAct->setEnabled(false);


  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  QHBoxLayout *hlayout = new QHBoxLayout;

  QLabel *label = new QLabel("Iterations", &dlg);
  hlayout->addWidget(label);

  QSpinBox *spinBox = new QSpinBox(&dlg);
  spinBox->setRange(100, 10 * _netView->defaultIterations());
  spinBox->setValue(_netView->defaultIterations());
  hlayout->addWidget(spinBox);

  vlayout->addLayout(hlayout);

  hlayout = new QHBoxLayout;

  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);

  vlayout->addLayout(hlayout);

  int result = dlg.exec();

  if (result == QDialog::Rejected)
    return;

  unsigned iterations = spinBox->value();

  _netView->redraw(iterations);
}

void HapnetWindow::setTraitGroups()
{
  if (_traitVect.empty()) 
  {
    statusBar()->showMessage(tr("No traits read."));
    return;
  }
  
  QVector<QString> traitStrings;
  map<QString, Trait*> nameToTrait;
  QMap<QString, QList<QString> > traitGroupMap;
  
  if (_traitGroupsSet)
  {
    for (unsigned i = 0; i < _traitGroups->size(); i++)
      traitGroupMap[QString::fromStdString(_traitGroups->at(i))] = QList<QString>();
  }
  
  for (unsigned i = 0; i < _traitVect.size(); i++)
  {
    traitStrings << QString::fromStdString(_traitVect.at(i)->name());
    nameToTrait[traitStrings.last()] = _traitVect.at(i);
    
    if (_traitGroupsSet)
    {
      string groupName = _traitGroups->at(_traitVect.at(i)->group());
      traitGroupMap[QString::fromStdString(groupName)] << traitStrings.last();
    }
  }
  
  GroupItemDialog groupDlg(traitStrings, traitGroupMap);
  bool groupsSuccess = groupDlg.exec();
  
  if (groupsSuccess)
  {
    //QMap<QString, QList<QString> > traitGroups = groupDlg.groups();
    _traitGroups->clear();
    
    QMap<QString, QList<QString> >::const_iterator mapIt = traitGroupMap.constBegin();
    unsigned groupIdx = 0;
    
    while (mapIt != traitGroupMap.constEnd())
    {
      _traitGroups->push_back(mapIt.key().toStdString());
      
      //cout << "group " << mapIt.key().toStdString() << ":";
      
      foreach (QString str, mapIt.value())
      {
        //cout << ' ' << str.toStdString();
        Trait *t = nameToTrait[str];
        t->setGroup(groupIdx);
        //cout << '=' << t->name();
      }
      
      ++mapIt;
      groupIdx++;
      //cout << endl;
    }
    
    if (_stats)
      _stats->setFreqsFromTraits(_traitVect);
    
    _traitGroupsSet = true;
  }
}

void HapnetWindow::setTraitColour()
{
  
  if (_traitVect.empty()) 
  {
    statusBar()->showMessage(tr("No traits read."));
    return;
  }
  
  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  
  QComboBox *comboBox = new QComboBox(&dlg);
  
  for (unsigned i = 0; i < _traitVect.size(); i++)
    comboBox->addItem(QString::fromStdString(_traitVect.at(i)->name()));
  
  vlayout->addWidget(comboBox);
  
  QHBoxLayout *hlayout = new QHBoxLayout;
  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;
  
  int traitIdx = comboBox->currentIndex();
  changeColour(traitIdx);
  
}


void HapnetWindow::resizeEvent(QResizeEvent *event)
{
  emit sizeChanged(event->size());
  QMainWindow::resizeEvent(event);
}

void HapnetWindow::showIdenticalSeqs()
{
  if (! _stats)
    doStatsSetup();
  
  if (! _stats)
    return;

  const map<Sequence, list<Sequence> > & identical = _stats->mapIdenticalSeqs();//_goodSeqs, _mask);
    
  map<Sequence, list<Sequence> >::const_iterator identicalIt = identical.begin();
   
  QStringList header("Node label");
  header.push_back("Matching sequences");
    
  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  
  vlayout->addWidget(new QLabel("Identical Sequences", this));
  
  QTreeWidget *tree = new QTreeWidget(&dlg);
  tree->setColumnCount(2);
  
  tree->setHeaderItem(new QTreeWidgetItem(header));
  
  while (identicalIt != identical.end())
  {
    if (!identicalIt->second.empty())
    {
      QStringList parentList(QString::fromStdString(identicalIt->first.name()));
      parentList.push_back(QString::fromStdString(identicalIt->first.name()));
      
      QTreeWidgetItem *parentItem = new QTreeWidgetItem(parentList);

      tree->addTopLevelItem(parentItem);
      
      list<Sequence>::const_iterator othersIt = identicalIt->second.begin();
      
      while (othersIt != identicalIt->second.end())
      {
        QStringList child(tr(""));
        child.push_back(QString::fromStdString(othersIt->name()));
        parentItem->addChild(new QTreeWidgetItem(child));
        
        ++othersIt;
      }
      
    }
    
    ++identicalIt;
  }

  tree->expandAll();
  tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
  vlayout->addWidget(tree);
  vlayout->addWidget(new QLabel("Log to file?", this));

  QHBoxLayout *hlayout = new QHBoxLayout;
  hlayout->addStretch(1);
  QPushButton *yesButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogYesButton), "Yes", &dlg);
  connect(yesButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(yesButton, 0, Qt::AlignRight);
  QPushButton *noButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogNoButton), "No", &dlg);
  connect(noButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(noButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);
  
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;

  QString defaultName(_filename);

  if (defaultName.endsWith(".nex", Qt::CaseInsensitive))
    defaultName.replace(defaultName.length() - 3, 3, "log"); 
  else
    defaultName.append(".log");
  
  QString filename = QFileDialog::getSaveFileName(this, "Log file", defaultName, "Log files (*.log);;All Files(*)");

  if (filename.isEmpty()) 
    return;
  QFile file(filename);
  if (!file.open(QIODevice::Append | QIODevice::Text))
    return;
  QTextStream out(&file);
  out << "Node label\tMatching Sequences\n";
  
  identicalIt = identical.begin();
  while (identicalIt != identical.end())
  {
    if (! identicalIt->second.empty())
    {
      out << QString::fromStdString(identicalIt->first.name()) << "\t" << QString::fromStdString(identicalIt->first.name()) << endl;
      list<Sequence>::const_iterator othersIt = identicalIt->second.begin();
      
      while (othersIt != identicalIt->second.end())
      {
        out << "\t" << QString::fromStdString(othersIt->name()) << "\n";
        
        ++othersIt;
      }     
    }   
    ++identicalIt;
  }
  
  file.close();
  
}

void HapnetWindow::doStatsSetup()
{
  try
  {
    _stats = new Statistics(_goodSeqs, _mask, _datatype);
    
    /*_progress->setLabelText("Setting up stats...");
    connect(_statThread, SIGNAL(started()), _stats, SLOT(setupStats()));
    connect(_stats, SIGNAL(progressUpdated(int)), _progress, SLOT(setValue(int)));
    _progress->show();
    
    _stats->moveToThread(_statThread);
    _statThread->start();
    
    _statThread->wait();*/
    _stats->setupStats();
    if (! _traitVect.empty())  _stats->setFreqsFromTraits(_traitVect); 
  }
  
  catch (StatsError &ste)
  {
    showErrorDlg("<b>Error setting up statistics</b>", ste.what());
    delete _stats;
    _stats = 0;
  }
  
  catch (exception &e)
  {
    showErrorDlg("<b>Unknown error in statistics</b>", e.what());
    delete _stats;
    _stats = 0;
  }
}

void HapnetWindow::showNucleotideDiversity()
{
  if (! _stats)
    doStatsSetup();
        
  if (! _stats)
    return;
  double diversity = _stats->nucleotideDiversity();
  
  QMessageBox message;
  message.setIcon(QMessageBox::Information);
  message.setText(tr("<b>Nucleotide diversity</b>"));
  QString infText(tr("%1 = %2").arg(QChar(0x03C0)).arg(diversity));
  message.setInformativeText(infText);
  message.setStandardButtons(QMessageBox::Ok); 
  message.setDefaultButton(QMessageBox::Ok);

  message.exec();

}

void HapnetWindow::showSegSites()
{
  if (! _stats)
    doStatsSetup();
  
  if (! _stats)
    return;
  
  unsigned segsites = _stats->nSegSites();
  
  QMessageBox message;
  message.setIcon(QMessageBox::Information);
  message.setText(tr("<b>Segregating sites</b>"));
  message.setInformativeText(tr("Number of sites: %1").arg(segsites));
  message.setStandardButtons(QMessageBox::Ok); 
  message.setDefaultButton(QMessageBox::Ok);

  message.exec(); 
}

void HapnetWindow::showParsimonySites()
{
  if (! _stats)
    doStatsSetup();
  
  if (! _stats)
    return;
  
  unsigned psites = _stats->nParsimonyInformative();
  
  QMessageBox message;
  message.setIcon(QMessageBox::Information);
  message.setText(tr("<b>Parsimony-informative sites</b>"));
  message.setInformativeText(tr("Number of sites: %1").arg(psites));
  message.setStandardButtons(QMessageBox::Ok); 
  message.setDefaultButton(QMessageBox::Ok);

  message.exec(); 
}

void HapnetWindow::showTajimaD()
{
  if (! _stats)
    doStatsSetup();

  if (! _stats)
    return;
  
  Statistics::stat tajimaStat = _stats->TajimaD();
  
  QMessageBox message;
  message.setIcon(QMessageBox::Information);
  message.setText(tr("<b>Tajima's D statistic</b>"));
  QString infText(tr("D = %1<br>p(D %2 %1) = %3").arg(tajimaStat.value).arg(QChar(0x2265)).arg(tajimaStat.prob));
  message.setInformativeText(infText);
  message.setStandardButtons(QMessageBox::Ok); 
  message.setDefaultButton(QMessageBox::Ok);

  message.exec(); 
}

void HapnetWindow::showAmova()
{
  if (_traitVect.empty())
  {
    showErrorDlg("<b>Sequences must have associated traits to perform analysis of molecular variance.<b>");
    return;
  }

  if (! _stats)
    doStatsSetup();
  
  if (! _stats)
    return;
  
  Statistics::amovatab amovaStat;
  bool nestedAmovaPerformed = false;
  
  if (_traitGroupsSet)
  {
    amovaStat = _stats->nestedAmova();
    nestedAmovaPerformed = true;
  }
    
  else
  {
    
    QMessageBox groupPromptDlg(this);
    groupPromptDlg.setIcon(QMessageBox::Warning);
    groupPromptDlg.setText("<b>Trait groups not set</b>");
    groupPromptDlg.setInformativeText("Groups must be defined for nested AMOVA. Define groups now or perform simple (non-nested) AMOVA?");
    
    
    QPushButton *defineGroupsButton = groupPromptDlg.addButton("Define groups", QMessageBox::AcceptRole);
    groupPromptDlg.setDefaultButton(defineGroupsButton);
    //QAbstractButton *simpleAmovaButton = 
    groupPromptDlg.addButton("Simple AMOVA", QMessageBox::DestructiveRole);
    groupPromptDlg.setStandardButtons(QMessageBox::Cancel);
    
    
    // OK, Cancel, "Simple AMOVA"
    
    //int result = 
    groupPromptDlg.exec();
    
    QMessageBox::ButtonRole result = groupPromptDlg.buttonRole(groupPromptDlg.clickedButton());
    
    cout << "result: " << result << " destructive role: " << QMessageBox::DestructiveRole << " accept role: " << QMessageBox::AcceptRole << " reject role: " << QMessageBox::RejectRole << endl;
    
    if (result == QMessageBox::RejectRole)
      return;
    
    else if (result == QMessageBox::DestructiveRole)  // perform simple amova
      amovaStat = _stats->amova();
      
    else 
    {
      setTraitGroups();
      
      // if trait groups still not set, user must have cancelled, don't perform AMOVA
      if (! _traitGroupsSet)
        return;
      amovaStat = _stats->nestedAmova();
      nestedAmovaPerformed = true;
    }
  }
  
  double sigmaTotal = amovaStat.sigma2_a + amovaStat.sigma2_b;
  double smallestP = 1.0 / Statistics::Iterations;
  
  //QMessageBox 
  MonospaceMessageBox message(this);
  //message.setIcon(QMessageBox::Information);
  message.setText(tr("<b>Analysis of molecular variance</b>"));

  //QString infText(tr("F = %1<br>p(F %2 %1) = %3<br>&Phi;<sub>ST</sub> = %4").arg(QString::number(amovaStat.F, 'f', 3)).arg(QChar(0x2265)).arg(QString::number(amovaStat.prob, 'g', 3)).arg(amovaStat.phiST));
  QString infText(QString("&Phi;<sub>ST</sub> = %1 Significance: p ").arg(QString::number(amovaStat.phiST.value, 'f', 5))); 
  //%2").arg(QString::number(amovaStat.phiST.value, 'f', 5)).arg(QString::number(amovaStat.phiST.prob, 'f', 3)));
  
  if (amovaStat.phiST.prob < smallestP)
    infText += QString("&lt; %1").arg(smallestP);
    
  else
    infText += QString("= %1").arg(QString::number(amovaStat.phiST.prob, 'g', 3));
    
  message.setInformativeText(infText);
  
  QString detText(QString("                  Sum of\nVariation    df  squares sigma^2 %variation\n\n"));
  
  if (nestedAmovaPerformed)
  { 
    sigmaTotal += amovaStat.sigma2_c;
    
    detText += QString("Among\ngroups%1").arg(amovaStat.df_ag, 9); 
    detText += QString("%1").arg(QString::number(amovaStat.ss_ag, 'f', 3), 9);
    detText += QString("%1").arg(QString::number(amovaStat.sigma2_a, 'f', 3), 8);
    detText += QString("%1\n\n").arg(QString::number(amovaStat.sigma2_a / sigmaTotal, 'f', 5), 11);

    detText += QString("Among\npopulations%1").arg(amovaStat.df_ap, 4);
    detText += QString("%1").arg(QString::number(amovaStat.ss_ap, 'f', 3), 9);
    detText += QString("%1").arg(QString::number(amovaStat.sigma2_b, 'f', 3), 8);
    detText += QString("%1\n\n").arg(QString::number(amovaStat.sigma2_b / sigmaTotal, 'f', 5), 11);
    
    detText += QString("Within\npopulations%1").arg(amovaStat.df_wp, 4);
    detText += QString("%1").arg(QString::number(amovaStat.ss_wp, 'f', 3), 9);
    detText += QString("%1").arg(QString::number(amovaStat.sigma2_c, 'f', 3), 8);
    detText += QString("%1\n\n").arg(QString::number(amovaStat.sigma2_c / sigmaTotal, 'f', 5), 11);
    
    detText += QString("TOTAL%1").arg(amovaStat.df_ag + amovaStat.df_ap + amovaStat.df_wp, 10);
    detText += QString("%1").arg(QString::number(amovaStat.ss_ag + amovaStat.ss_ap + amovaStat.ss_wp, 'f', 3), 9);
    detText += QString("%1\n\n").arg(QString::number(sigmaTotal, 'f', 3), 8);
    
    detText += QString("Fixation indices\n    Phi_ST: %1\n\n").arg(QString::number(amovaStat.phiST.value, 'f', 5));
    detText += QString("    Phi_SC: %1\n\n").arg(QString::number(amovaStat.phiSC.value, 'f', 5));
    detText += QString("    Phi_CT: %1\n\n").arg(QString::number(amovaStat.phiCT.value, 'f', 5));
    detText += QString("Significance (%1 permutations):\n").arg(Statistics::Iterations);
    detText += QString("    Phi_ST: Pr(random value > observed Phi_ST) ");
    
    if (amovaStat.phiST.prob < smallestP)
      detText += QString("< %1").arg(smallestP);
      
    else
      detText += QString("= %1").arg(QString::number(amovaStat.phiST.prob, 'g', 3));

    detText += QString("    Phi_SC: Pr(random value > observed Phi_SC) ");
    
    if (amovaStat.phiSC.prob < smallestP)
      detText += QString("< %1").arg(smallestP);
      
    else
      detText += QString("= %1").arg(QString::number(amovaStat.phiSC.prob, 'g', 3));

    detText += QString("    Phi_CT: Pr(random value > observed Phi_CT) ");

    if (amovaStat.phiCT.prob < smallestP)
      detText += QString("< %1").arg(smallestP);
      
    else
      detText += QString("= %1").arg(QString::number(amovaStat.phiCT.prob, 'g', 3));
    
  }
  
  else
  {
    
    detText += QString("Among\npopulations%1").arg(amovaStat.df_ap, 4); // .arg((uint)amovaStat.df_ap, (int)4, (int)10, QChar(' '));
    detText += QString("%1").arg(QString::number(amovaStat.ss_ap, 'f', 3), 9);
    detText += QString("%1").arg(QString::number(amovaStat.sigma2_a, 'f', 3), 8);
    detText += QString("%1\n\n").arg(QString::number(amovaStat.sigma2_a / sigmaTotal, 'f', 5), 11);
    
    detText += QString("Within\npopulations%1").arg(amovaStat.df_wp, 4);
    detText += QString("%1").arg(QString::number(amovaStat.ss_wp, 'f', 3), 9);
    detText += QString("%1").arg(QString::number(amovaStat.sigma2_b, 'f', 3), 8);
    detText += QString("%1\n\n").arg(QString::number(amovaStat.sigma2_b / sigmaTotal, 'f', 5), 11);
    
    detText += QString("TOTAL%1").arg(amovaStat.df_ap + amovaStat.df_wp, 10);
    detText += QString("%1").arg(QString::number(amovaStat.ss_ap + amovaStat.ss_wp, 'f', 3), 9);
    detText += QString("%1\n\n").arg(QString::number(sigmaTotal, 'f', 3), 8);
    
    detText += QString("Fixation index\nPhi_ST: %1\n\n").arg(QString::number(amovaStat.phiST.value, 'f', 5));
    detText += QString("Significance (%1 permutations):\n").arg(Statistics::Iterations);
    detText += QString("Phi_ST: Pr(random value > observed Phi_ST) ");//%1 %2").arg(QString::number(amovaStat.phiST.prob, 'g', 3));
    
    if (amovaStat.phiST.prob < smallestP)
      detText += QString("< %1").arg(smallestP);
      
    else
      detText += QString("= %1").arg(QString::number(amovaStat.phiST.prob, 'g', 3));
  }
  

  /*QString detText(tr("%2%1%1Sum_Sq%1Mean_Sq%1F_value%1%1%1%1Pr(>F)\n").arg('\xa0').arg("df", 14, '\xa0'));
  detText += QString("Population%1").arg((uint)(amovaStat.dfFac), (int)4, (int)10, QChar('\xa0'));
  //arg(numstr, fieldwidth, fillchar)
  detText +=QString("%1").arg(QString::number(amovaStat.ssb, 'f', 2), 8, '\xa0');
  detText +=QString("%1").arg(QString::number(amovaStat.msb, 'f', 2), 8, '\xa0');
  detText +=QString("%1").arg(QString::number(amovaStat.F, 'f', 2), 8, '\xa0');
  detText += QString("%1\n").arg(QString::number(amovaStat.prob, 'g', 2), 10, '\xa0');
  
  detText += QString("Residuals%1").arg((uint)(amovaStat.dfRes), (int)5, (int)10, QChar('\xa0'));
  detText += QString("%1").arg(QString::number(amovaStat.ssw, 'f', 2), 8, '\xa0');
  detText += QString("%1\n").arg(QString::number(amovaStat.msw, 'f', 2), 8, '\xa0');  */
  
  message.setDetailedText(detText);
  //message.setStandardButtons(QMessageBox::Ok); 
  //message.setDefaultButton(QMessageBox::Ok);
  //message.setFixedWidth(1000);
  //qDebug() << "message width: " << message.width() << "height: " << message.height();

  message.exec(); 
}

// TODO add AMOVA here
void HapnetWindow::showAllStats()
{
  if (! _stats)
    doStatsSetup();
  
  if (! _stats)
    return;
  
  double diversity = _stats->nucleotideDiversity();
  unsigned segsites = _stats->nSegSites();
  unsigned psites = _stats->nParsimonyInformative();
  Statistics::stat tajimaStat = _stats->TajimaD();
  //Statistics::amovatab amovaStat = _stats->amova();
  
  QDialog dlg(this);
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);  
 
  QString divText(tr("<b>Nucleotide diversity:</b> %1 = %2").arg(QChar(0x03C0)).arg(diversity));
  vlayout->addWidget(new QLabel(divText, &dlg));
  
  QString ssText(tr("<b>Number of segregating sites:</b> %1").arg(segsites));
  vlayout->addWidget(new QLabel(ssText, &dlg));

  QString psText(tr("<b>Number of parsimony-informative sites:</b> %1").arg(psites));
  vlayout->addWidget(new QLabel(psText, &dlg));
  
  
  double tajimaPval;
  QChar inequal;
  if (tajimaStat.prob < 0.5)
  {
    tajimaPval = 2 * tajimaStat.prob;
    inequal = QChar(0x2265);
  }
  
  else
  {
    tajimaPval = 2 * (1 - tajimaStat.prob);
    inequal = QChar(0x2264);
  }
  
  QString tajimaText = (tr("<b>Tajima's D:</b> %1<br>p(D %2 %1) = %3").arg(tajimaStat.value).arg(inequal).arg(tajimaPval));
  vlayout->addWidget(new QLabel(tajimaText, &dlg));
  
  //QString amovaText(tr("<b>AMOVA F:</b> %1<br>p(F %2 %1) = %3").arg(amovaStat.F).arg(QChar(0x2265)).arg(amovaStat.prob));
  //vlayout->addWidget(new QLabel(amovaText, &dlg));
  
  vlayout->addWidget(new QLabel("Log to file?", this));

  QHBoxLayout *hlayout = new QHBoxLayout;
  hlayout->addStretch(1);
  QPushButton *yesButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogYesButton), "Yes", &dlg);
  connect(yesButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(yesButton, 0, Qt::AlignRight);
  QPushButton *noButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogNoButton), "No", &dlg);
  connect(noButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(noButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);
  
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;  
  

  QString defaultName(_filename);

  if (defaultName.endsWith(".nex", Qt::CaseInsensitive))
    defaultName.replace(defaultName.length() - 3, 3, "log"); 
  else
    defaultName.append(".log");
  
  QString filename = QFileDialog::getSaveFileName(this, "Log file", defaultName, "Log files (*.log);;All Files(*)");

  if (filename.isEmpty()) 
    return;
  
  QFile file(filename);
  if (!file.open(QIODevice::Append | QIODevice::Text))
    return;
  QTextStream out(&file);
  
  out << "Nucleotide diversity:\tpi = " << diversity << endl;
  out << "Number of segregating sites:\t" << segsites << endl;
  out << "Number of parsimony-informative sites:\t" << psites << endl;
  out << "Tajima's D statistic:\tD = " << tajimaStat.value << endl;
  out << "\tp (D >= " << tajimaStat.value << ") = " << tajimaStat.prob << endl;
  /*out << "Analysis of molecular variance:\tF = " << amovaStat.F << endl;
  out << "\tp (F >= " << amovaStat.F << ") = " << amovaStat.prob << endl;
  
  out << QString("%1  Sum Sq Mean Sq F value    Pr(>F)\n").arg(QString("df"), 14);
  out << QString("Population%1").arg(amovaStat.dfFac, 4);
  out << QString("%1").arg(QString::number(amovaStat.ssb, 'f', 2), 8);
  out << QString("%1").arg(QString::number(amovaStat.msb, 'f', 2), 8);
  out << QString("%1").arg(QString::number(amovaStat.F, 'f', 2), 8);
  out <<  QString("%1\n").arg(QString::number(amovaStat.prob, 'g', 2), 10);
  
  out <<  QString("Residuals %1").arg(amovaStat.dfRes, 4);
  out <<  QString("%1").arg(QString::number(amovaStat.ssw, 'f', 2), 8);
  out <<  QString("%1").arg(QString::number(amovaStat.msw, 'f', 2), 8) << endl;  */
  
  file.close();

}

void HapnetWindow::search()
{
  QDialog dlg(this);
  
  QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
  QHBoxLayout *hlayout = new QHBoxLayout;
  
  QLabel *label = new QLabel("Node label", &dlg);
  hlayout->addWidget(label);
  
  QLineEdit *edit = new QLineEdit(&dlg);
  hlayout->addWidget(edit);
  vlayout->addLayout(hlayout);
  
  hlayout = new QHBoxLayout;
  
  hlayout->addStretch(1);
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", &dlg);
  connect(okButton, SIGNAL(clicked()), &dlg, SLOT(accept()));
  hlayout->addWidget(okButton, 0, Qt::AlignRight);
  QPushButton *cancelButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", &dlg);
  connect(cancelButton, SIGNAL(clicked()), &dlg, SLOT(reject()));
  hlayout->addWidget(cancelButton, 0, Qt::AlignRight);
  
  vlayout->addLayout(hlayout);
  
  int result = dlg.exec();
  
  if (result == QDialog::Rejected)
    return;
  
  _netView->selectNodes(edit->text());
  
}



void HapnetWindow::toggleAlignmentActions(bool enable)
{
  
  QList<QAction *> netActions = _networkMenu->actions();
  QList<QAction *>::iterator netIt = netActions.begin();
  
  while (netIt != netActions.end())
  {
    (*netIt)->setEnabled(enable);
    ++netIt;
  }
  
  _networkMenu->setEnabled(enable);

  QList<QAction *> statsActions = _statsMenu->actions();
  QList<QAction *>::iterator statsIt = statsActions.begin();
  
  while (statsIt != statsActions.end())
  {
    (*statsIt)->setEnabled(enable);
    ++statsIt;
  }
  
  _statsMenu->setEnabled(enable);

  _closeAct->setEnabled(enable);
  
}

void HapnetWindow::toggleTraitActions(bool enable)
{
  _traitGroupAct->setEnabled(enable);
  _amovaAct->setEnabled(enable);
}

void HapnetWindow::toggleNetActions(bool enable)
{
  _saveAsAct->setEnabled(enable);
  _exportAct->setEnabled(enable);
  _saveGraphicsAct->setEnabled(enable);
  //_exportToolAct->setEnabled(enable);
  _saveToolAct->setEnabled(enable);
  _zoomInAct->setEnabled(enable);
  _zoomOutAct->setEnabled(enable);
  _rotateLAct->setEnabled(enable);
  _rotateRAct->setEnabled(enable);
  _searchAct->setEnabled(enable);
  _taxBoxAct->setEnabled(enable);
  _barchartAct->setEnabled(enable);
  
  _traitColourAct->setEnabled(enable);
  _vertexColourAct->setEnabled(enable);
  _vertexSizeAct->setEnabled(enable);
  _edgeColourAct->setEnabled(enable);
  _backgroundColourAct->setEnabled(enable);
  _labelFontAct->setEnabled(enable);
  _legendFontAct->setEnabled(enable);
  _redrawAct->setEnabled(enable);
  //_viewMenu->setEnabled(enable);
  _mutationMenu->setEnabled(enable);
  _dashViewAct->setEnabled(enable);
  _nodeViewAct->setEnabled(enable);
  _numViewAct->setEnabled(enable);
}

void HapnetWindow::fixBarchartButton(bool taxBoxChecked)
{
  if (taxBoxChecked)
    _barchartAct->setChecked(false);
}

void HapnetWindow::fixTaxBoxButton(bool barchartChecked)
{
  if (barchartChecked)
    _taxBoxAct->setChecked(false);
}

void HapnetWindow::showDocumentation()
{
  _assistant->showDocumentation("overview.html");
}

void HapnetWindow::about()
{
     QMessageBox::about(this, tr("About PopART"),
                        tr("PopART (Population Analysis with Reticulate Trees) is free, open source population genetics software that was developed as part of the Allan Wilson Centre Imaging Evolution Initiative.\n\nIt is used for inferring and visualising genealogical relationships among populations."));

}
