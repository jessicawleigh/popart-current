
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QPixmap>
#include <QScrollArea>
#include <QSplashScreen>
#include <QString>
#include <QTimer>

#include "HapnetWindow.h"
#include "splash.h"

/*
 * TODO:
 * 
 * REALLY IMPORTANT, BEFORE SMBE:
 * 
 * - add traits editor and associate meaningful traits
 * - get ParsimonyNet doing something useful
 * - possibly add a slider to let users scale things like deleting edges,
 *   ancestral nodes etc. below a threshold
 * 
 * Integrate GUI and back end
 * Implement new hap net algos
 * Implement TCS
 * Condense sequences like SplitsTree does
 * Read more alignment formats
 * Time networks: view network between nodes with associated dates
 * Other cool visual things, like loading a map and associating nodes with coordinates
 * 
 * IMPORTANT: consider changing traits editor to a tree view (as below)
 * Current idea is table view, with sequence name in one col and trait in another
 *  - this is a one sample/one trait view, as opposed to one sequence/all associated traits view
 *  - could toggle between the two?
 *
 * Make dock a data viewer/editor:
 *  - alignment (obviously)
 *  - frequencies
 *  - traits: phylogeography/phenotype/whatever
 *  - use a tree view to show identical haplotypes merged (default) or expanded (to show traits, etc.)
 *     - when merged, show number of htypes and trait frequencies (tool tip? pie?)
 *     - when expaneded, show traits separately
 *  - hap seqs are given an ID that corresponds to actual sequence? or just use name with first occurrence?
 *     - possibly mouse over lists other names in tool tip/status, click expands to show all identical sequences
 *  - in traits viewer, mouse over will show beginning of actual sequence in status? tool tip?
 *  - a view menu to switch between data view (alignment/traits)
 * 
 * Make network viewer pretty:
 *  - pie charts showing trait frequencies associated with haplotypes
 * 
 * Make network viewer configurable:
 *  - different types of network views: rooted, etc.
 *  - change colours of edges
 *  - change colours of pie slices
 */


int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  #if defined(Q_OS_WIN)
  QString pluginpath = QCoreApplication::applicationDirPath() + QDir::separator() + QLatin1String("plugins");
  app.addLibraryPath(pluginpath);
  #endif

  QPixmap pixmap(splash::popartsplash);
  QSplashScreen *splash = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
  splash->show();
  QTimer::singleShot(2000, splash, SLOT(hide())); 
  HapnetWindow hapwin;
  //hapwin.resize(400, 600);
  hapwin.show();
  
  return app.exec();
}
