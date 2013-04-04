
#include "LabelItem.h"
#include "NetworkView.h"
#include "NetworkItem.h"

#include <QColor>
#include <QtConcurrentMap>
#include <QtCore>
#include <QFont>
#include <QFontMetricsF>
#include <QBrush>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QPointF>
#include <QPrinter>
#include <QPushButton>
#include <QRegExp>
#include <QSvgGenerator>
#include <QThread>
#include <QVBoxLayout>

#include <cmath>

NetworkView::ColourTheme NetworkView::_defaultTheme = NetworkView::Greyscale;

NetworkView::NetworkView(QWidget * parent)
  : QAbstractItemView(parent), 
  _backgroundBrush(Qt::transparent), 
  _foregroundBrush(Qt::black), 
  _defaultVertBrush(Qt::black),
  _vertexSections()
{  
  _layout = 0;
  _legend = 0;
  _legendRotation = 0;
  _sceneClear = true;
  _showBarcharts = false;
  _showTaxBox = false;
  setupDefaultPens();
  setColourTheme(defaultColourTheme());
  _currentTheme = defaultColourTheme();
  _layoutThread = new QThread(this);
  connect(_layoutThread, SIGNAL(finished()), this, SLOT(adjustAndDraw()));
  _progress = new QProgressDialog(this);
  QPushButton *cancelButton = new QPushButton("Can't Cancel", _progress);
  cancelButton->setEnabled(false);
  _progress->setCancelButton(cancelButton);
  _progress->setLabelText("Drawing network...");
  _progress->setMinimum(0);
  _progress->setMaximum(100);

  
  _theView.setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  //_theView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  //_theView.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  _theView.setDragMode(QGraphicsView::RubberBandDrag);
  _theView.setRenderHint(QPainter::Antialiasing);
  _theView.setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  _theScene.setSceneRect(0, 0, _theView.width(), _theView.height());  

  _theView.setScene(&_theScene); 
  
  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->addWidget(&_theView);  
  
  connect(&_theScene, SIGNAL(itemsMoved(QList<QPair<QGraphicsItem *, QPointF> >)), this, SLOT(handleItemMove(QList<QPair<QGraphicsItem *, QPointF> >)));
}

NetworkView::~NetworkView()
{

  clearScene();
  delete _layout;
  _layout = 0;
}

QModelIndex NetworkView::indexAt(const QPoint &point) const
{
  QGraphicsItem *theItem = _theView.itemAt(point);
  if (theItem)
  {

    int row = theItem->data(0).toInt();
    
    // TODO think about this... QAbstractListModel requires only row
    return model()->index(row, 0, QModelIndex());
  }
  
  return QModelIndex();
}

void NetworkView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
  // TODO What?
}

QRect NetworkView::visualRect(const QModelIndex &index) const
{
  // TODO check that this is a vertex, not an edge
  // Convert from QRectF to QRect
    
  if (index.row() >= 0 && index.row() < _vertexItems.size())
    return _vertexItems.at(index.row())->rect().toRect();
  else
    return QRect();
}

int NetworkView::horizontalOffset() const
{
  // TODO figure this out
  return 0;
}

bool NetworkView::isIndexHidden(const QModelIndex &index) const
{
  // TODO figure this out
  
  return false;
}

QModelIndex NetworkView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
  // TODO What should this do? Move to next vertex? "Nudge" vertex?
  return QModelIndex();
}

QItemSelectionModel::SelectionFlags NetworkView::selectionCommand(const QModelIndex &model, const QEvent *event) const
{
  // TODO fill this in
  
  return QItemSelectionModel::NoUpdate;
}

void NetworkView::setModel(QAbstractItemModel *themodel)
{
  QAbstractItemModel *oldmodel = model();
  clearModel();
  delete oldmodel;
  

  QAbstractItemView::setModel(themodel);
  
  _defaultIterations = 10 * themodel->rowCount();
    
  createLayout();
}

void NetworkView::createLayout(int iterations)
{
  clearScene();
  
  if (_layout)  delete _layout;
  _layout = 0;
  
  NetworkModel *nModel = dynamic_cast<NetworkModel *>(model());
  double wdth = _theScene.width();
  double hght = _theScene.height();
    
  _layout = new NetworkLayout(nModel, wdth, hght);
  
  unsigned maxiter;
  if (iterations > 0)
    maxiter = iterations;
  else
    maxiter = _defaultIterations;
  
  _layout->setMaxIter(maxiter);

  _progress->show();

  connect(_layout, SIGNAL(progressChanged(int)), _progress, SLOT(setValue(int)));
  connect(_layoutThread, SIGNAL(started()), _layout, SLOT(optimise()));

  _layout->moveToThread(_layoutThread);
  _layoutThread->start();

  // TODO check cancelled each iteration?
}

void NetworkView::adjustAndDraw()
{
  _progress->hide();
  adjustScene();
  drawLayout();
  _sceneClear = false;
  emit networkDrawn();

}
void NetworkView::optimiseLayout()//NetworkLayout* layout)
{
  //cout << "in optimiseLayotu" << endl;
  //_layout->optimise();
}

void NetworkView::adjustScene()
{
  _layout->centreVertices();

  
  // If vertices have negative coordinates, move them
  QPointF translation = _layout->northWest();
  if (translation.x() < MARGIN)
  {
    translation.setX(MARGIN - translation.x());;
    if (translation.y() < MARGIN)  
      translation.setY(MARGIN - translation.y());
    else
      translation.setY(0);
    
    _layout->translateVertices(translation);
  }
  
  else if (translation.y() < MARGIN)
  {
    translation.setX(0);
    translation.setY(MARGIN - translation.y());
    
    _layout->translateVertices(translation);
  }
  
  // If network takes too much space, make scene bigger
  QPointF newSize = _layout->southEast();
       
  double maxDiam = NetworkItem::VERTRAD * 2;
  
  // account for vertex diametres
  for (unsigned i = 0; i < _layout->vertexCount(); i++)
  {  
    double diametre = NetworkItem::VERTRAD * sqrt(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
    if (diametre > maxDiam)  maxDiam = diametre;
  }
  
  newSize.rx() += maxDiam + MARGIN;
  newSize.ry() += maxDiam + MARGIN;
  
  _graphRect = QRectF(0, 0, newSize.x(), newSize.y());
  _theScene.setSceneRect(_graphRect);
}

void NetworkView::handleItemMove(QList<QPair<QGraphicsItem *, QPointF> >  itemList)
{
  emit itemsMoved(itemList); 
}

void NetworkView::drawLayout()
{
  _theScene.setBackgroundBrush(_backgroundBrush);
     
  _barchart = new BarchartItem();
  _barchart->setFlags(QGraphicsItem::ItemIgnoresTransformations);
  _barchart->setZValue(10);
  _barchart->setPos(-10, -10);
  _barchart->hide();
  _theScene.addItem(_barchart);
  
  _taxbox = new TaxBoxItem();
  _taxbox->setFlags(QGraphicsItem::ItemIgnoresTransformations);
  _taxbox->setZValue(10);
  _taxbox->setPos(-10, -10);
  _taxbox->hide();
  _theScene.addItem(_taxbox);
  
  
  QGraphicsEllipseItem *vSection;
  LabelItem *labelItem;
  VertexItem *vItem;
  EdgeItem *eItem;
  
  bool addLegend = false;
  
  double minVertDiam = NetworkItem::VERTRAD * 2 / 3;
  
  _vertexSections.resize(_layout->vertexCount());
  for (unsigned i = 0; i < _layout->vertexCount(); i++)
  {
    QList<QVariant> traits = model()->index(i,0).data(NetworkItem::TraitRole).toList();
    
    
    double diametre = NetworkItem::VERTRAD * sqrt(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
    if (diametre < minVertDiam)  
      diametre = minVertDiam;

    const QPointF & vCentre = _layout->vertexCoords(i);
     

    
    if (traits.empty())
    {
      vItem = new VertexItem(vCentre.x(), vCentre.y(), diametre, diametre);
      vItem->setPen(outlinePen());
      vItem->setBrush(vertBrush());
      vItem->setData(0, i);
      vItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
      vItem->setAcceptHoverEvents(true);
      connect(vItem, SIGNAL(hoverEntered(QGraphicsItem *)), this, SLOT(showTaxBox(QGraphicsItem *)));
      connect(vItem, SIGNAL(hoverLeft()), this, SLOT(hideTaxBox()));
      _theScene.addItem(vItem);
      _vertexItems.push_back(vItem);
    }
    
    else
    {
      
      addLegend = true;
      unsigned counter = 0;
      unsigned total = 0;
      unsigned startAngle = 0;
      unsigned spanAngle;
      bool done = false;
      unsigned freq = model()->index(i, 0).data(NetworkItem::SizeRole).toUInt();
      
      QList<QVariant>::const_iterator trit = traits.constBegin();
    
      // vItem that's transparent to anchor sections
      vItem = new VertexItem(vCentre.x(), vCentre.y(), diametre, diametre);
      vItem->setPen(invisiblePen());
      vItem->setBrush(Qt::transparent);
      vItem->setData(0, i);
      vItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
      vItem->setAcceptHoverEvents(true);
      connect(vItem, SIGNAL(hoverEntered(QGraphicsItem *)), this, SLOT(showBarchart(QGraphicsItem *)));
      connect(vItem, SIGNAL(hoverEntered(QGraphicsItem *)), this, SLOT(showTaxBox(QGraphicsItem *)));
      connect(vItem, SIGNAL(hoverLeft()), this, SLOT(hideBarchart()));
      connect(vItem, SIGNAL(hoverLeft()), this, SLOT(hideTaxBox()));
      _theScene.addItem(vItem);
      _vertexItems.push_back(vItem);

      
      while (trit != traits.constEnd() && ! done)
      {
        total += (*trit).toUInt(); 
        // once this is the last trait, it should be set to pieSegments - startAngle

        if (total == freq)  
        {
          done = true; 
          spanAngle = PIESEGMENTS - startAngle;           
        } 
        
        else  spanAngle = PIESEGMENTS * (*trit).toUInt()/freq;
        
        vSection = new QGraphicsEllipseItem(vCentre.x(), vCentre.y(), diametre, diametre);

       
        // If there is a single trait associated with a node, no need to set angles
        if (startAngle > 0 || spanAngle < PIESEGMENTS)
        {
          vSection->setStartAngle(startAngle);
          vSection->setSpanAngle(spanAngle);
        }
        vSection->setPen(outlinePen());
        
        // This is a good idea, but makes a legend difficult:
        // Avoid making last and first pie section the same colour
        //if (total == freq && (counter % freq == 0))
        //  vSection->setBrush(vertBrush(counter + 1));
        //else
        vSection->setBrush(vertBrush(counter));
        
        vSection->setParentItem(vItem);
        vSection->setData(0, i);
        vSection->setData(1, counter);
        _vertexSections[i].push_back(vSection);
        
        startAngle += spanAngle;
        counter++;
        ++trit;
      }
    }
    
    // TODO background, font, text colour
    QString label = model()->index(i,0).data(NetworkItem::LabelRole).toString();
    labelItem = new LabelItem(label, vItem);
    labelItem->setFont(labelFont());
    QPointF newpos = vItem->boundingRect().topLeft();
    newpos.rx() -= labelItem->boundingRect().width();
    if (newpos.x() < 0)  newpos.setX(0);

    labelItem->setPos(newpos);
    labelItem->setFlags(QGraphicsItem::ItemIgnoresTransformations | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
    _labelItems.push_back(labelItem);
    /*connect(labelItem, SIGNAL(moved(const QPointF &)), this, SLOT(captureMoveEvent(const QPointF &)));
    connect(labelItem, SIGNAL(aboutToMove(QGraphicsItem *)), this, SLOT(prepareMoveEvent(QGraphicsItem *)));*/
  }
  
  for (unsigned i = 0; i < _layout->edgeCount(); i++)
  {
    eItem = new EdgeItem(_vertexItems.at(_layout->edgeStartIdx(i)), _vertexItems.at(_layout->edgeEndIdx(i)));

    eItem->setPen(edgePen());
    eItem->setZValue(-1);
    _theScene.addItem(eItem);
    _edgeItems.push_back(eItem);

  }  
  
  if (addLegend)
    drawLegend();
      
  _border = new BorderRectItem(_theScene.sceneRect());
  _border->setPen(borderPen());
  _border->setBrush(Qt::transparent);
  _border->setAcceptHoverEvents(true);
  //(_theScene.sceneRect(), borderPen(), Qt::transparent);
  _border->setZValue(-1);
  _theScene.addItem(_border);
}

void NetworkView::drawLegend()
{
  
  QFontMetricsF metric(legendFont());
  QFontMetricsF smallMetric(smallFont());
  double maxwidth = 0;
  for (unsigned i = 0; i < model()->columnCount(); i++)
  {
    double width = metric.width(model()->headerData(i, Qt::Vertical).toString());
    if (width > maxwidth)  maxwidth = width;
  }
  
  double minHeight = NetworkItem::VERTRAD + 5;
  double entryHeight = metric.height();
  if (minHeight > entryHeight)
    entryHeight = minHeight;
  
  double legendWidth = maxwidth + NetworkItem::VERTRAD + 3 * MARGIN;
  double legendHeight = entryHeight * model()->columnCount() + smallMetric.height() + sqrt(5) * NetworkItem::VERTRAD + 3 * MARGIN;
  
  QPointF legendStart(_graphRect.width(), _graphRect.height() - legendHeight);    
  

  _legend = new QGraphicsRectItem(0, 0, legendWidth, legendHeight);
  _legend->setPen(outlinePen());
  _legend->setData(0, -1);
  _legend->setBrush(Qt::white);
  //_legend->setPos(legendStart.x(), legendStart.y());
  _legend->setRotation(_legendRotation);
  _legend->setPos(computeLegendPos());
  _legend->setFlags(QGraphicsItem::ItemIsMovable);
  //_legendPos = _theView.mapFromScene(legendStart);
  
  //_theScene.setSceneRect(_graphRect.x(), _graphRect.y(), _graphRect.width() + legendWidth, _graphRect.height())
  
  
  
  _theScene.setSceneRect(computeSceneRect());
  _theScene.addItem(_legend);
  
  
  QGraphicsEllipseItem *key;
  double currentY = MARGIN; 
  double keyX = legendWidth/2 - sqrt(10)/2 * NetworkItem::VERTRAD;
  
  key = new QGraphicsEllipseItem(0, 0, sqrt(10) * NetworkItem::VERTRAD, sqrt(10) * NetworkItem::VERTRAD, _legend);
  key->setPos(keyX, currentY);
  key->setBrush(Qt::transparent);
  key->setData(0, -1);
  key->setPen(outlinePen());
  _sizeKeys.first = key;
  
  QGraphicsSimpleTextItem *legendLabel = new QGraphicsSimpleTextItem("10 samples", key);
  legendLabel->setData(0, -1);
  legendLabel->setFont(smallFont());
  double textX = key->boundingRect().center().x() - smallMetric.width(legendLabel->text())/2;
  currentY = key->boundingRect().center().y() - smallMetric.height()/2;
  legendLabel->setPos(QPointF(textX, currentY));
  _sizeLabels.first = legendLabel;
  
  currentY = key->boundingRect().bottom();
  keyX = legendWidth/2 - 0.5 * NetworkItem::VERTRAD;
  
  key = new QGraphicsEllipseItem(0, 0, NetworkItem::VERTRAD, NetworkItem::VERTRAD, _legend);
  key->setPos(keyX, currentY);
  key->setBrush(Qt::transparent);
  key->setData(0, -1);
  key->setPen(outlinePen());
  _sizeKeys.second = key;

  legendLabel = new QGraphicsSimpleTextItem("1 sample", key);
  legendLabel->setData(0, -1);
  legendLabel->setFont(smallFont());
  currentY = key->boundingRect().bottom();  
  textX = key->boundingRect().center().x() - smallMetric.width(legendLabel->text())/2;
  legendLabel->setPos(QPointF(textX, currentY));
  _sizeLabels.second = legendLabel;
  
  currentY =  2 * MARGIN + sqrt(10) * NetworkItem::VERTRAD + smallMetric.height();

  textX = NetworkItem::VERTRAD + 2 * MARGIN;
  keyX = MARGIN;
  
  for (unsigned i = 0; i < model()->columnCount(); i++)
  {
    LegendItem *legkey = new LegendItem(0, 0, NetworkItem::VERTRAD, NetworkItem::VERTRAD, _legend);
    legkey->setPos(keyX, currentY);
    legkey->setBrush(vertBrush(i));
    legkey->setData(0, -1);
    legkey->setData(1, i);
    connect(legkey, SIGNAL(clicked(LegendItem *)), this, SLOT(legendItemClicked(LegendItem *))); 
    _legendKeys.push_back(legkey);
    
    legendLabel = new QGraphicsSimpleTextItem(model()->headerData(i, Qt::Vertical).toString(), _legend);
    legendLabel->setData(0, -1);
    legendLabel->setData(1, i);
    legendLabel->setFont(legendFont());
    legendLabel->setPos(QPointF(textX, currentY));
    _legendLabels.push_back(legendLabel);
    
    currentY += entryHeight;
  }
}

void NetworkView::changeLabelFont(const QFont &font)
{
  setLabelFont(font);
  
  for (unsigned i = 0; i < _labelItems.size(); i++)
  {
    LabelItem *label = _labelItems.at(i);
    label->setFont(labelFont());
    label->update(label->boundingRect());
  }
}

void NetworkView::changeLegendFont(const QFont &font)
{
  //_legendFont = font;
  //_smallFont = font;
  setLegendFont(font);

  int smallerPointSize = _legendFont.pointSize() * 0.6 + 0.5; 
  QFont smaller(font);
  smaller.setPointSize(smallerPointSize);
  setSmallFont(smaller);
  
  QFontMetricsF metric(legendFont());
  QFontMetricsF smallMetric(smallFont());

  double maxwidth = 0;
  for (unsigned i = 0; i < model()->columnCount(); i++)
  {
    double width = metric.width(model()->headerData(i, Qt::Vertical).toString());
    if (width > maxwidth)  maxwidth = width;
  }
  
  double minHeight = NetworkItem::VERTRAD + 5;
  double entryHeight = metric.height();
  if (minHeight > entryHeight)
    entryHeight = minHeight;

  double legendWidth = maxwidth + NetworkItem::VERTRAD + 3 * MARGIN;
  double legendHeight = entryHeight * (1 + model()->columnCount()) + 5 * NetworkItem::VERTRAD + 3 * MARGIN;
 
  QPointF legendStart(_graphRect.width(), _graphRect.height() - legendHeight);    

  _legend->setRect(0, 0, legendWidth, legendHeight);
  _legend->setRotation(_legendRotation);
  _legend->setPos(computeLegendPos());
  _theScene.setSceneRect(computeSceneRect());
  _border->updateRect();
  
  QGraphicsEllipseItem *key = _sizeKeys.first;
  QGraphicsSimpleTextItem *label = _sizeLabels.first;
  
  double currentY = MARGIN; 
  double keyX = legendWidth/2 - 2.5 * NetworkItem::VERTRAD;  
  key->setPos(keyX, currentY);

  label->setFont(smallFont());
  label->update(label->boundingRect());
  double textX = key->boundingRect().center().x() - smallMetric.width(label->text())/2;
  currentY = key->boundingRect().center().y() - smallMetric.height()/2;
  label->setPos(textX, currentY);

  currentY = key->boundingRect().bottom();
  key = _sizeKeys.second;
  label = _sizeLabels.second;

  keyX = legendWidth/2 - 0.5 * NetworkItem::VERTRAD;
  key->setPos(keyX, currentY);
  
  label->setFont(smallFont());
  label->update(label->boundingRect());;
  currentY = key->boundingRect().bottom();  
  textX = key->boundingRect().center().x() - smallMetric.width(label->text())/2;
  label->setPos(textX, currentY);
  
  currentY =  2 * MARGIN + 5 * NetworkItem::VERTRAD + smallMetric.height();

  textX = NetworkItem::VERTRAD + 2 * MARGIN;
  keyX = MARGIN;
  
  for (unsigned i = 0; i < model()->columnCount(); i++)
  {
    key = _legendKeys.at(i);
    key->setPos(keyX, currentY);
    
    label = _legendLabels.at(i);
    label->setFont(legendFont());
    label->update(label->boundingRect());;
    label->setPos(textX, currentY);

    currentY += entryHeight;
  }
}

void NetworkView::clearModel()
{
  delete _layout;
  _layout = 0;
  clearScene();
}

void NetworkView::clearScene()
{
  _theScene.clear();
  _vertexSections.clear();
  _vertexItems.clear();
  _edgeItems.clear();
  _labelItems.clear();
  _theView.matrix().reset();
  _theScene.setSceneRect(0, 0, _theView.width(), _theView.height());
  _sceneClear = true;
  _legend = 0;
  _legendKeys.clear();
  _legendLabels.clear();

}

void NetworkView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
  // TODO fill this in
}

int NetworkView::verticalOffset() const
{
  // TODO figure this out
  return 0;
}

QRegion NetworkView::visualRegionForSelection(const QItemSelection &selection) const
{
  return QRegion();
}

void NetworkView::setupDefaultPens()
{
  _defaultFont.setFamily("Baskerville");
  _defaultFont.setPointSize(10);
  _legendFont = _defaultFont;
  _labelFont = _defaultFont;
  
  _smallFont = QFont(_defaultFont);
  _smallFont.setPointSize(6);

  _textPen.setWidth(2);
  _edgePen.setWidth(2);
  _edgePen.setCapStyle(Qt::RoundCap);
  
  _borderPen.setStyle(Qt::DashLine);
  _borderPen.setBrush(QColor(50, 0, 128));

  
  _transparentPen.setBrush(Qt::transparent);
  _transparentPen.setWidth(0);
  
  _defaultOutlinePen.setBrush(_foregroundBrush);
  _defaultOutlinePen.setWidth(1);
  
  _greyscale.push_back(Qt::white);
  _greyscale.push_back(QColor(204, 204, 204));
  _greyscale.push_back(Qt::black);
  _greyscale.push_back(QColor(249, 249, 249));
  _greyscale.push_back(QColor(26, 26, 26));
  _greyscale.push_back(QColor(242, 242, 242));
  _greyscale.push_back(QColor(179, 179, 179));
  _greyscale.push_back(QColor(51, 51, 51));
  _greyscale.push_back(QColor(230, 230, 230));
  _greyscale.push_back(QColor(102, 102, 102));
   
  
  _camo.push_back(QColor(80, 45, 22));
  _camo.push_back(QColor(51, 128, 0));
  _camo.push_back(QColor(77, 77, 77));
  _camo.push_back(QColor(188, 211, 95));
  _camo.push_back(QColor(22, 80, 22));
  _camo.push_back(QColor(200, 190, 183));
  _camo.push_back(QColor(31, 36, 28));
  _camo.push_back(QColor(136, 170, 0));
  _camo.push_back(QColor(172, 147, 147));
  _camo.push_back(QColor(83, 108, 83));
  
  _pastelle.push_back(QColor(175, 198, 233));
  _pastelle.push_back(QColor(170, 255, 170));
  _pastelle.push_back(QColor(229, 128, 255));
  _pastelle.push_back(QColor(255, 238, 170));
  _pastelle.push_back(QColor(255, 128, 229));
  _pastelle.push_back(QColor(255, 170, 170));
  _pastelle.push_back(QColor(198, 175, 233));
  _pastelle.push_back(QColor(233, 175, 198));
  _pastelle.push_back(QColor(230, 255, 180));
  _pastelle.push_back(QColor(100, 255, 255));
  
  _vibrant.push_back(QColor(255, 0, 0));
  _vibrant.push_back(QColor(55, 200, 55));
  _vibrant.push_back(QColor(102, 0, 128));
  _vibrant.push_back(QColor(255, 204, 0));
  _vibrant.push_back(QColor(255, 0, 204));
  _vibrant.push_back(QColor(170, 0, 0));
  _vibrant.push_back(QColor(85, 0, 212));
  _vibrant.push_back(QColor(255, 0, 102));
  _vibrant.push_back(QColor(215, 255, 42));
  _vibrant.push_back(QColor(0, 204, 255));
  
  _spring.push_back(QColor(255, 85, 85));
  _spring.push_back(QColor(221, 255, 85));
  _spring.push_back(QColor(229, 128, 255));
  _spring.push_back(QColor(128, 179, 255));
  _spring.push_back(QColor(255, 0, 204));
  _spring.push_back(QColor(0, 255, 102));
  _spring.push_back(QColor(198, 175, 233));
  _spring.push_back(QColor(255, 85, 153));
  _spring.push_back(QColor(55, 200, 171));
  _spring.push_back(QColor(102, 255, 255));
  
  _summer.push_back(QColor(0, 212, 0));
  _summer.push_back(QColor(255, 42, 127));
  _summer.push_back(QColor(0, 128, 102));
  _summer.push_back(QColor(255, 102, 0));
  _summer.push_back(QColor(0, 102, 255));
  _summer.push_back(QColor(212, 0, 170));
  _summer.push_back(QColor(255, 85, 85));
  _summer.push_back(QColor(255, 204, 0));
  _summer.push_back(QColor(171, 55, 200));
  _summer.push_back(QColor(192, 0, 0));

  _autumn.push_back(QColor(51, 51, 51));
  _autumn.push_back(QColor(128, 0, 0));
  _autumn.push_back(QColor(255, 104, 0));
  _autumn.push_back(Qt::red);
  _autumn.push_back(QColor(255, 102, 0));
  _autumn.push_back(QColor(160, 190, 44));
  _autumn.push_back(QColor(80, 45, 22));
  _autumn.push_back(QColor(22, 80, 22));
  _autumn.push_back(QColor(200, 55, 55));
  _autumn.push_back(QColor(200, 171, 55));

  _winter.push_back(QColor(128, 0, 0));
  _winter.push_back(QColor(0, 85, 68));
  _winter.push_back(QColor(51, 51, 51));
  _winter.push_back(QColor(198, 175, 233));
  _winter.push_back(QColor(22, 80, 22));
  _winter.push_back(Qt::black);
  _winter.push_back(QColor(102, 0, 128));
  _winter.push_back(Qt::white);
  _winter.push_back(QColor(51, 0, 128));
  _winter.push_back(QColor(100, 255, 255));
  
}

void  NetworkView::setColourTheme(ColourTheme theme)
{
  QVector<QColor> *colours;
  _currentTheme = theme;
  
  switch (theme)
  {
    case Camo:
      colours = &_camo;
      break;
      
    case Pastelle:
      colours = &_pastelle;
      break;
    
    case Vibrant:
      colours = &_vibrant;
      break;
    
    case Spring:
      colours = &_spring;
      break;
      
    case Summer:
      colours = &_summer;
      break;
      
    case Autumn:
      colours = &_autumn;
      break;
      
    case Winter:
      colours = &_winter;
      break;
      
    case Greyscale:
    default:
      colours = &_greyscale;
      break;
  }
  
  _colourTheme.clear();
  
  QVector<QColor>::const_iterator colIt = colours->constBegin();
  
  while (colIt != colours->constEnd())
  {
    _colourTheme.push_back(QBrush(*colIt));
    ++colIt;
  }
  
  if (_layout)  
  {
    /*clearScene();
    drawLayout();*/
    updateColours();
  }
}

void NetworkView::updateColours()
{
  for (unsigned i = 0; i < _vertexSections.size(); i++)
  {
    QList<QGraphicsEllipseItem*>::iterator sectionIt = _vertexSections[i].begin();
    
    while (sectionIt != _vertexSections[i].end())
    {
      (*sectionIt)->setBrush(vertBrush((*sectionIt)->data(1).toInt()));
      (*sectionIt)->update((*sectionIt)->boundingRect());
      ++sectionIt;
    }
  }
  
  if (! _legendKeys.empty())
  {
    for (unsigned i = 0; i < _legendKeys.size(); i++)
    {
      _legendKeys.at(i)->setBrush(vertBrush(i));
      _legendKeys.at(i)->update(_legendKeys.at(i)->boundingRect());
    }
  }

  //_theScene.update(0, 0, _theScene.width(), _theScene.height());

}

const QBrush & NetworkView::vertBrush(int traitID) const
{
  if (traitID < 0)  
    return _defaultVertBrush;
  
  else 
  {
    int npens = _colourTheme.size();
    return _colourTheme.at(traitID % npens);
  }
}

const QColor & NetworkView::colour(unsigned idx) const
{
  return vertBrush(idx).color();
}

void NetworkView::setColour(unsigned idx, const QColor & colour)
{
  for (unsigned i = 0; idx >= _colourTheme.size(); i++)
  {
    _colourTheme.push_back(vertBrush(i));
  }
  
  _colourTheme[idx] = QBrush(colour);
  
  updateColours();
}

void NetworkView::setBackgroundColour(const QColor &colour)
{
  _backgroundBrush.setColor(colour);
  
  _theScene.setBackgroundBrush(_backgroundBrush);
  _theScene.update();
}

void NetworkView::setEdgeColour(const QColor &colour)
{
  _edgePen.setBrush(colour);
  for (unsigned i = 0; i < _edgeItems.size(); i++)
  {
    _edgeItems.at(i)->setPen(_edgePen);
    _edgeItems.at(i)->update(_edgeItems.at(i)->boundingRect());
  }
}

void NetworkView::setVertexColour(const QColor &colour)
{
  _defaultVertBrush.setColor(colour);
  
  for (unsigned i = 0; i < _vertexItems.size(); i++)
  {
    QList<QVariant> traits = model()->index(i,0).data(NetworkItem::TraitRole).toList();
    
    // ignore vertices with traits data
    if (traits.empty())
    {
      _vertexItems.at(i)->setBrush(_defaultVertBrush);
      _vertexItems.at(i)->update(_vertexItems.at(i)->boundingRect());
    }
  }
}

void NetworkView::saveSVGFile(const QString &filename)
{
  double width = _theScene.width();
  double height = _theScene.height();
  _border->hide();
  _theScene.clearSelection();
  
  QSvgGenerator generator;
  generator.setFileName(filename);
  generator.setSize(QSize(width, height));
  generator.setViewBox(QRect(0, 0, width, height));
  generator.setTitle(tr("PopART SVG Image"));
  generator.setDescription(tr("A haplotype network image created by PopART."));
  QPainter painter;
  painter.begin(&generator);
  _theScene.render(&painter);
  painter.end();
  
  _border->show();
}

// TODO use QImageWriter for more advanced options
void NetworkView::savePNGFile(const QString &filename)
{  
  double width = _theScene.width();
  double height = _theScene.height();
  _border->hide();
  _theScene.clearSelection();
  
  QImage image(width, height, QImage::Format_ARGB32);
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  _theScene.render(&painter);
  image.save(filename);
  painter.end();
  _border->show();
  
}

void NetworkView::savePDFFile(const QString &filename)
{
  double width = _theScene.width();
  double height = _theScene.height();
  _border->hide();
  _theScene.clearSelection();
  
  QPrinter printer;
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(filename);
  
  if (width > height)
    printer.setOrientation(QPrinter::Landscape);
  else
    printer.setOrientation(QPrinter::Portrait);
  //printer.setWidth(width);
  //printer.setHeight(height);
  QPainter painter(&printer);
  painter.setRenderHint(QPainter::Antialiasing);
  _theScene.render(&painter);
  painter.end();
  _border->show();

}

void NetworkView::redraw(unsigned iterations)
{
  createLayout(iterations);
}

void NetworkView::zoomIn()
{
  _theView.scale(1.2, 1.2); 
}

void NetworkView::zoomOut()
{
  _theView.scale(1/1.2, 1/1.2);
}

// Rotate scene 90 anti-clockwise
void NetworkView::rotateL()
{
  _theView.rotate(-90);
  _legendRotation += 90;

  if (_legend) 
  {
    _legend->setRotation(_legendRotation);
    _legend->setPos(computeLegendPos());
    _theScene.setSceneRect(computeSceneRect());
    _border->updateRect();
  }
  
}

// Rotate scene 90 degrees clockwise
void NetworkView::rotateR()
{
  _theView.rotate(90);
  _legendRotation -= 90;
  if (_legendRotation < 0)  _legendRotation += 360;
  
  if (_legend) 
  {
    _legend->setRotation(_legendRotation);
    _legend->setPos(computeLegendPos());
    _theScene.setSceneRect(computeSceneRect());
    _border->updateRect();
  }  
}

QPointF NetworkView::computeLegendPos()
{
  if (! _legend)  return QPointF(0, 0);
  
  QPointF pos;
  
  double hght = _legend->boundingRect().height();
  //double wdth = _legend->boundingRect().width();
  
  double graphHeight = _graphRect.height();
  double graphWidth = _graphRect.width();
  
  int rotationCount = _legend->rotation() / 90;
  
  if (rotationCount % 4 == 1)
  {
    pos.setX(hght);
    pos.setY(graphHeight);
  }
  
  else if (rotationCount % 4 == 2)
  {
    pos.setX(0);
    pos.setY(hght);
  }
  
  else if (rotationCount % 4 == 3)
  {
    pos.setX(graphWidth - hght);
    pos.setY(0);
  }
  
  else
  {
    pos.setX(graphWidth);
    pos.setY(graphHeight - hght);
  }
  
  return pos;
}

QRectF NetworkView::computeSceneRect()
{  
  if (! _legend)  return _graphRect;
  
  QRectF newRect(_graphRect);
  
  double legendHeight = _legend->boundingRect().height();
  double legendWidth = _legend->boundingRect().width();
    
  int rotationCount = _legend->rotation() / 90;
  
  if (rotationCount % 4 == 1)
  {
    newRect.setHeight(_graphRect.height() + legendWidth);
    newRect.setY(_graphRect.y() - legendWidth);
  }
  
  else if (rotationCount % 4 == 2)
  {
    newRect.setWidth(_graphRect.width() + _legend->boundingRect().width());
    newRect.setX(_graphRect.x() - legendWidth);
  }
  
  else if (rotationCount % 4 == 3)
  {
    newRect.setHeight(_graphRect.height() + legendWidth);   
    newRect.setY(_graphRect.y() - legendWidth);
  }
  
  else
  {      
    newRect.setWidth(_graphRect.width() + legendWidth);
  }
 
  return newRect;
}

void NetworkView::selectNodes(const QString &label)
{
  QRegExp regex(label);
  regex.setPatternSyntax(QRegExp::Wildcard);
  
  for (unsigned i = 0; i < _labelItems.size(); i++)
  {
    
    if (regex.exactMatch(_labelItems.at(i)->text()))
      _labelItems.at(i)->parentItem()->setSelected(true);
  }
}

void NetworkView::toggleShowBarcharts(bool show)
{
  _showBarcharts = show;
  if (show)
    _showTaxBox = false;
}

void NetworkView::toggleShowTaxBox(bool show)
{
  _showTaxBox = show;
  if (show)
    _showBarcharts = false;
}

void NetworkView::showTaxBox(QGraphicsItem *item)
{
  if (! _showTaxBox)  return;
  //QGraphicsItem *item = _theScene.itemAt(pos, _theView.viewportTransform());
  if (item)
  {   
    QPointF pos = item->boundingRect().center();
    QVariant data = item->data(0);
    
    if (data == QVariant())  return;
    unsigned idx = data.toUInt();
    
    _taxbox->setPos(pos); 
    QString label = model()->index(idx, 0).data(NetworkItem::LabelRole).toString();
    QList<QVariant> taxaVariants = model()->index(idx, 0).data(NetworkItem::TaxaRole).toList();

    QVector<QString> taxa;
    QList<QVariant>::const_iterator taxIt = taxaVariants.constBegin();
    
    while (taxIt != taxaVariants.constEnd())
    {
      taxa.push_back((*taxIt).toString());
  
      ++taxIt;
    }
    
    if (taxa.size() > 1)
    {  
      _taxbox->setLabels(taxa);
      _taxbox->show();
    }   
  }  
}


void NetworkView::hideTaxBox()
{
  _taxbox->hide();
}

void NetworkView::showBarchart(QGraphicsItem *item)
{
  
  if (! _showBarcharts)  return;
  // _theScene.itemAt(
  //QGraphicsItem *item = _theScene.itemAt(pos, _theView.viewportTransform());
  if (item)
  {   
    QPointF pos = item->boundingRect().center();
    QVariant data = item->data(0);
   
    if (data == QVariant())  return; 
    unsigned idx = data.toUInt();
        
    _barchart->setPos(pos);
    
    double freq = model()->index(idx, 0).data(NetworkItem::SizeRole).toUInt();
    QVector<double> traitsD;
    
    QList<QVariant> traits = model()->index(idx,0).data(NetworkItem::TraitRole).toList();
    
    QList<QVariant>::const_iterator trit = traits.constBegin();
    
    while (trit != traits.constEnd())
    {
      traitsD.push_back((*trit).toUInt()/freq);
  
      ++trit;
    }
    
    _barchart->setTraits(traitsD, _colourTheme);
    _barchart->show();
  }
  
}

void NetworkView::hideBarchart()
{
  _barchart->hide();
}

void NetworkView::legendItemClicked(LegendItem *item)
{
  QVariant data = item->data(1);
  
  if (data == QVariant())  return;
  
  int idx = data.toInt();
  
  emit legendItemClicked(idx);
}

void NetworkView::keyPressEvent(QKeyEvent *event)
{

  if (_sceneClear)
  {
    QAbstractItemView::keyPressEvent(event);
  }

  else
  {
    if (event->key() == Qt::Key_Plus)
    {
      zoomIn();
    }

    else if (event->key() == Qt::Key_Minus)
    {
      zoomOut();
    }

    else
      QAbstractItemView::keyPressEvent(event);
  }
}


