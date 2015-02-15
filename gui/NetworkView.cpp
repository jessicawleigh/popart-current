#include "HapAppError.h"
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

#include <QDebug>

#include <cmath>

ColourTheme::Theme NetworkView::_defaultTheme = ColourTheme::Greyscale;

NetworkView::NetworkView(QWidget * parent)
  : QAbstractItemView(parent), 
  _vertexSections(),
  _backgroundBrush(Qt::transparent),
  _foregroundBrush(Qt::black), 
  _defaultVertBrush(Qt::black)
{  
  _layout = 0;
  _legend = 0;
  _legendRotation = 0;
  _sceneClear = true;
  _nodeLabelsVisible = true;
  _showBarcharts = false;
  _showTaxBox = false;
  _vertRadUnit = NetworkItem::VERTRAD;
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
  _graphRect = QRectF(0, 0, _theView.width(), _theView.height());
  _theScene.setSceneRect(_graphRect);  

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

void NetworkView::setModel(QAbstractItemModel *themodel, bool skipLayout)
{
  QAbstractItemModel *oldmodel = model();
  clearModel();
  delete oldmodel;
  

  QAbstractItemView::setModel(themodel);
  
  _defaultIterations = 10 * themodel->rowCount();

  if (skipLayout)
  {
    createEmptyLayout();
  }
  
  else
    createLayout();
  
  connect(themodel, SIGNAL(traitsUpdated()), this, SLOT(updateTraits()));
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

void NetworkView::createEmptyLayout()
{
  NetworkModel *nModel = dynamic_cast<NetworkModel *>(model());
  double wdth = _theScene.width();
  double hght = _theScene.height();
  
  _layout = new NetworkLayout(nModel, wdth, hght);
  _layout->zeroVertices();
  drawLayout();
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
       
  double maxDiam = _vertRadUnit * 2;
  
  // account for vertex diametres
  for (unsigned i = 0; i < _layout->vertexCount(); i++)
  {  
    double diametre = _vertRadUnit * sqrt(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
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

void NetworkView::setGrabbableCursor(bool grabbable)
{
  if (grabbable)
    setCursor(Qt::OpenHandCursor);
  else
    setCursor(Qt::ArrowCursor);
}

void NetworkView::setGrabbingCursor(bool grabbing)
{
  if (grabbing)
    setCursor(Qt::ClosedHandCursor); // DragMoveCursor ?
  else
    setCursor(Qt::OpenHandCursor);
}

void NetworkView::setClickableCursor(bool clickable)
{
  if (clickable)
    setCursor(Qt::PointingHandCursor);
  else
    setCursor(Qt::ArrowCursor);

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
  
  double minVertDiam = _vertRadUnit * 2 / 3;
  
  _vertexSections.resize(_layout->vertexCount());
  for (unsigned i = 0; i < _layout->vertexCount(); i++)
  {
    QList<QVariant> traits = model()->index(i,0).data(NetworkItem::TraitRole).toList();
    
    
    double diametre = _vertRadUnit * sqrt(model()->index(i, 0).data(NetworkItem::SizeRole).toUInt());
    if (diametre < minVertDiam)  
      diametre = minVertDiam;

    const QPointF & vCentre = _layout->vertexCoords(i);
     

    
    if (traits.empty())
    {
      vItem = new VertexItem(vCentre.x(), vCentre.y(), diametre, diametre);
      //vItem = new VertexItem(0, 0, diametre, diametre);
      //vItem->setPos(vCentre.x(), vCentre.y());
      
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
    
      //vItem that's transparent to anchor sections
      vItem = new VertexItem(vCentre.x(), vCentre.y(), diametre, diametre);
      //vItem = new VertexItem(0, 0, diametre, diametre);
      //vItem->setPos(vCentre.x(), vCentre.y());

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
    if (_nodeLabelsVisible)
    {
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
  }
  
  for (unsigned i = 0; i < _layout->edgeCount(); i++)
  {
    eItem = new EdgeItem(_vertexItems.at(_layout->edgeStartIdx(i)), _vertexItems.at(_layout->edgeEndIdx(i)), _layout->edgeWeight(i));

    eItem->setPen(edgePen());
    eItem->setZValue(-1);
    _theScene.addItem(eItem);
    _edgeItems.push_back(eItem);

  }  
  
  if (addLegend)
    drawLegend();
      
  _border = new BorderRectItem(_theScene.sceneRect());
  connect(_border, SIGNAL(grabbable(bool)), this, SLOT(setGrabbableCursor(bool)));
  connect(_border, SIGNAL(grabbing(bool)), this, SLOT(setGrabbingCursor(bool)));
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
  
  double minHeight = _vertRadUnit + 5;
  double entryHeight = metric.height();
  if (minHeight > entryHeight)
    entryHeight = minHeight;
  
  double legendWidth = maxwidth + _vertRadUnit + 3 * MARGIN;
  legendWidth = max(legendWidth,  _vertRadUnit * sqrt(10) + 2 * MARGIN);

  double legendHeight = entryHeight * model()->columnCount() + smallMetric.height() + sqrt(10) * _vertRadUnit + 3 * MARGIN;
  
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
  double keyX = legendWidth/2 - sqrt(10)/2 * _vertRadUnit;
  
  key = new QGraphicsEllipseItem(0, 0, sqrt(10) * _vertRadUnit, sqrt(10) * _vertRadUnit, _legend);
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
  
  //currentY = key->boundingRect().bottom();
  currentY = MARGIN + key->boundingRect().height() - _vertRadUnit;
  keyX = legendWidth/2 - 0.5 * _vertRadUnit;
  
  key = new QGraphicsEllipseItem(0, 0, _vertRadUnit, _vertRadUnit, _legend);
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
  
  currentY =  2 * MARGIN + sqrt(10) * _vertRadUnit + smallMetric.height();

  textX = _vertRadUnit + 2 * MARGIN;
  keyX = MARGIN;
  
  //_legendKeys.resize(model()->columnCount());
  
  // cout << "new size for legendLabels: " << _legendKeys.size() << endl;
  
  
  for (unsigned i = 0; i < model()->columnCount(); i++)
  {
    LegendItem *legkey = new LegendItem(0, 0, _vertRadUnit, _vertRadUnit, _legend);
    legkey->setPos(keyX, currentY);
    legkey->setBrush(vertBrush(i));
    legkey->setData(0, -1);
    legkey->setData(1, i);
    //connect(legkey, SIGNAL(clicked(LegendItem *)), this, SLOT(legendItemClicked(LegendItem *)));
    connect(legkey, SIGNAL(colourChangeTriggered(LegendItem *)), this, SLOT(legendItemClicked(LegendItem *)));
    connect(legkey, SIGNAL(clickable(bool)), this, SLOT(setClickableCursor(bool)));
    _legendKeys.push_back(legkey);
    //_legendKeys[i] = legkey;
    
    legendLabel = new QGraphicsSimpleTextItem(model()->headerData(i, Qt::Vertical).toString(), _legend);
    legendLabel->setData(0, -1);
    legendLabel->setData(1, i);
    legendLabel->setFont(legendFont());
    legendLabel->setPos(QPointF(textX, currentY));
    _legendLabels.push_back(legendLabel);
    
    currentY += entryHeight;
  }
}

void NetworkView::setNodeLabelsVisible(bool visible) 
{ 
  _nodeLabelsVisible = visible;

  //_theScene.update();
  clearScene();
  drawLayout();
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

void NetworkView::updateTraits()
{
  
  for (unsigned i = 0; i < _vertexSections.size(); i++)
  {
    QList<QGraphicsEllipseItem*>::iterator pieIt = _vertexSections[i].begin();
    
    while (pieIt != _vertexSections[i].end())
    {
      _theScene.removeItem(*pieIt);
      ++pieIt;
    }
    
    _vertexSections[i].clear();
    
    QPointF p = vertexPosition(i);
    p -= _vertexItems.at(i)->pos();
    
    
    QList<QVariant> traits = model()->index(i,0).data(NetworkItem::TraitRole).toList();
    
    
    if (! traits.empty())
    {
      double diametre = _vertexItems.at(i)->rect().width();
      unsigned counter = 0;
      unsigned total = 0;
      unsigned startAngle = 0;
      unsigned spanAngle;
      bool done = false;
      unsigned freq = model()->index(i, 0).data(NetworkItem::SizeRole).toUInt();
      
      QList<QVariant>::const_iterator trit = traits.constBegin();
      while (trit != traits.constEnd() && ! done)
      {
        total += (*trit).toUInt(); 
        
        if (total == freq)  
        {
          done = true; 
          spanAngle = PIESEGMENTS - startAngle;           
        } 
        
        else  spanAngle = PIESEGMENTS * (*trit).toUInt()/freq;
        
        QGraphicsEllipseItem *vSection = new QGraphicsEllipseItem(p.x(), p.y(), diametre, diametre);
        
        // If there is a single trait associated with a node, no need to set angles
        if (startAngle > 0 || spanAngle < PIESEGMENTS)
        {
          vSection->setStartAngle(startAngle);
          vSection->setSpanAngle(spanAngle);
        }
        
        vSection->setPen(outlinePen());
        vSection->setBrush(vertBrush(counter));
        
        vSection->setParentItem(_vertexItems.at(i));
        vSection->setData(0, i);
        vSection->setData(1, counter);
        _vertexSections[i].push_back(vSection);
        
        startAngle += spanAngle;
        counter++;
        ++trit;    
      }
    }
  }
  
  if (_legend)
  {
    _theScene.removeItem(_legend);
    delete _legend;
    _legend = 0;
    _legendKeys.clear();
    _legendLabels.clear();
  }
  
  drawLegend();
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
  
  _legendKeys.clear();
  _legendLabels.clear();
  if (_legend)
  {
    _theScene.removeItem(_legend);
    delete _legend;
    _legend = 0;
  }

  drawLegend();
  _border->updateRect();
  return;

  QFontMetricsF metric(legendFont());
  QFontMetricsF smallMetric(smallFont());

  double maxwidth = 0;
  for (unsigned i = 0; i < model()->columnCount(); i++)
  {
    double width = metric.width(model()->headerData(i, Qt::Vertical).toString());
    if (width > maxwidth)  maxwidth = width;
  }
  
  double minHeight = _vertRadUnit + 5;
  double entryHeight = metric.height();
  if (minHeight > entryHeight)
    entryHeight = minHeight;

  double legendWidth = maxwidth + _vertRadUnit + 3 * MARGIN;
  legendWidth = max(legendWidth,  _vertRadUnit * sqrt(10) + 2 * MARGIN);

  double legendHeight = entryHeight * model()->columnCount() + smallMetric.height() + sqrt(10) * _vertRadUnit + 3 * MARGIN;
 
  QPointF legendStart(_graphRect.width(), _graphRect.height() - legendHeight);    

  _legend->setRect(0, 0, legendWidth, legendHeight);
  _legend->setRotation(_legendRotation);
  _legend->setPos(computeLegendPos());
  _theScene.setSceneRect(computeSceneRect());
  _border->updateRect();
  
  QGraphicsEllipseItem *key = _sizeKeys.first;
  QGraphicsSimpleTextItem *label = _sizeLabels.first;
  
  double currentY = MARGIN; 
  double keyX = legendWidth/2 - 2.5 * _vertRadUnit;
  key->setPos(keyX, currentY);

  label->setFont(smallFont());
  label->update(label->boundingRect());
  double textX = key->boundingRect().center().x() - smallMetric.width(label->text())/2;
  currentY = key->boundingRect().center().y() - smallMetric.height()/2;
  label->setPos(textX, currentY);

  currentY = MARGIN + key->boundingRect().height() - _vertRadUnit;
  key = _sizeKeys.second;
  label = _sizeLabels.second;

  keyX = legendWidth/2 - 0.5 * _vertRadUnit;
  key->setPos(keyX, currentY);
  
  label->setFont(smallFont());
  label->update(label->boundingRect());;
  currentY = key->boundingRect().bottom();  
  textX = key->boundingRect().center().x() - smallMetric.width(label->text())/2;
  label->setPos(textX, currentY);
  
  currentY =  2 * MARGIN + 5 * _vertRadUnit + smallMetric.height();

  textX = _vertRadUnit + 2 * MARGIN;
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

QPointF NetworkView::vertexPosition(unsigned idx) const
{
  
  if (idx >= _vertexItems.size())
    throw HapAppError("Vertex index out of range");
  
  QRectF vertRect = _vertexItems.at(idx)->boundingRect();
  
  return _vertexItems.at(idx)->mapToScene(vertRect.topLeft());
}

void NetworkView::setVertexPosition(unsigned idx, const QPointF &p)
{
  if (idx >= _vertexItems.size())
    throw HapAppError("Vertex index out of range");
  
  // in case vertex has been moved, move back to its origin before mapping
  _vertexItems.at(idx)->setPos(0,0);
  
  QPointF newPos = _vertexItems.at(idx)->mapFromScene(p);  
  _vertexItems.at(idx)->setPos(newPos);
}

void NetworkView::setVertexPosition(unsigned idx, double x, double y)
{
  setVertexPosition(idx, QPointF(x,y));
}



QPointF NetworkView::labelPosition(unsigned idx) const
{
  
  if (idx >= _labelItems.size())
    throw HapAppError("Label index out of range");
  
  QRectF labelRect = _labelItems.at(idx)->boundingRect();
  
  QPointF vertPos = vertexPosition(idx);
  QPointF labelPos = _labelItems.at(idx)->pos();
  QPointF toParent = _labelItems.at(idx)->mapToParent(labelRect.topLeft());
  

  return _labelItems.at(idx)->mapToScene(labelRect.topLeft());

}

void NetworkView::setLabelPosition(unsigned idx, const QPointF &p)
{
  if (idx >= _labelItems.size())
    throw HapAppError("Label index out of range");
  
  // need to map from scene because it will have already moved with parent (vertex) position
  QPointF newPos = _labelItems.at(idx)->mapFromScene(p);
  _labelItems.at(idx)->setPos(newPos);
}

void NetworkView::setLabelPosition(unsigned idx, double x, double y)
{
  setLabelPosition(idx, QPointF(x,y));
}

QPointF NetworkView::legendPosition() const
{
  if (_legend)
  {
    QPointF brPoint = _legend->boundingRect().topLeft();

    return _legend->mapToScene(brPoint);
  }

  return QPointF(-1,-1);
}

void NetworkView::setLegendPosition(const QPointF &p)
{
  if (! _legend)
    throw HapAppError("No legend available");
  
  // In case legend has moved, move back to its origin before mapping
  _legend->setPos(0,0);
  
  QPointF newPos = _legend->mapFromScene(p);
  _legend->setPos(newPos);
}

void NetworkView::setLegendPosition(double x, double y)
{
  setLegendPosition(QPointF(x,y));
}


QRectF NetworkView::sceneRect() const
{
  return _theScene.sceneRect();
}

void NetworkView::setSceneRect(const QRectF &rect)
{
  _theScene.setSceneRect(rect);
  _border->updateRect();  
}

void NetworkView::setSceneRect(double x, double y, double width, double height)
{
  setSceneRect(QRectF(x, y, width, height));
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
  EdgeItem::setFont(_labelFont);
  
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
  
//  _greyscale.push_back(Qt::white);
//  _greyscale.push_back(QColor(204, 204, 204));
//  _greyscale.push_back(Qt::black);
//  _greyscale.push_back(QColor(249, 249, 249));
//  _greyscale.push_back(QColor(26, 26, 26));
//  _greyscale.push_back(QColor(242, 242, 242));
//  _greyscale.push_back(QColor(179, 179, 179));
//  _greyscale.push_back(QColor(51, 51, 51));
//  _greyscale.push_back(QColor(230, 230, 230));
//  _greyscale.push_back(QColor(102, 102, 102));
//
//
//  _camo.push_back(QColor(80, 45, 22));
//  _camo.push_back(QColor(51, 128, 0));
//  _camo.push_back(QColor(77, 77, 77));
//  _camo.push_back(QColor(188, 211, 95));
//  _camo.push_back(QColor(22, 80, 22));
//  _camo.push_back(QColor(200, 190, 183));
//  _camo.push_back(QColor(31, 36, 28));
//  _camo.push_back(QColor(136, 170, 0));
//  _camo.push_back(QColor(172, 147, 147));
//  _camo.push_back(QColor(83, 108, 83));
//
//  _pastelle.push_back(QColor(175, 198, 233));
//  _pastelle.push_back(QColor(170, 255, 170));
//  _pastelle.push_back(QColor(229, 128, 255));
//  _pastelle.push_back(QColor(255, 238, 170));
//  _pastelle.push_back(QColor(255, 128, 229));
//  _pastelle.push_back(QColor(255, 170, 170));
//  _pastelle.push_back(QColor(198, 175, 233));
//  _pastelle.push_back(QColor(233, 175, 198));
//  _pastelle.push_back(QColor(230, 255, 180));
//  _pastelle.push_back(QColor(100, 255, 255));
//
//  _vibrant.push_back(QColor(255, 0, 0));
//  _vibrant.push_back(QColor(55, 200, 55));
//  _vibrant.push_back(QColor(102, 0, 128));
//  _vibrant.push_back(QColor(255, 204, 0));
//  _vibrant.push_back(QColor(255, 0, 204));
//  _vibrant.push_back(QColor(170, 0, 0));
//  _vibrant.push_back(QColor(85, 0, 212));
//  _vibrant.push_back(QColor(255, 0, 102));
//  _vibrant.push_back(QColor(215, 255, 42));
//  _vibrant.push_back(QColor(0, 204, 255));
//
//  _spring.push_back(QColor(255, 85, 85));
//  _spring.push_back(QColor(221, 255, 85));
//  _spring.push_back(QColor(229, 128, 255));
//  _spring.push_back(QColor(128, 179, 255));
//  _spring.push_back(QColor(255, 0, 204));
//  _spring.push_back(QColor(0, 255, 102));
//  _spring.push_back(QColor(198, 175, 233));
//  _spring.push_back(QColor(255, 85, 153));
//  _spring.push_back(QColor(55, 200, 171));
//  _spring.push_back(QColor(102, 255, 255));
//
//  _summer.push_back(QColor(0, 212, 0));
//  _summer.push_back(QColor(255, 42, 127));
//  _summer.push_back(QColor(0, 128, 102));
//  _summer.push_back(QColor(255, 102, 0));
//  _summer.push_back(QColor(0, 102, 255));
//  _summer.push_back(QColor(212, 0, 170));
//  _summer.push_back(QColor(255, 85, 85));
//  _summer.push_back(QColor(255, 204, 0));
//  _summer.push_back(QColor(171, 55, 200));
//  _summer.push_back(QColor(192, 0, 0));
//
//  _autumn.push_back(QColor(51, 51, 51));
//  _autumn.push_back(QColor(128, 0, 0));
//  _autumn.push_back(QColor(255, 104, 0));
//  _autumn.push_back(Qt::red);
//  _autumn.push_back(QColor(255, 102, 0));
//  _autumn.push_back(QColor(160, 190, 44));
//  _autumn.push_back(QColor(80, 45, 22));
//  _autumn.push_back(QColor(22, 80, 22));
//  _autumn.push_back(QColor(200, 55, 55));
//  _autumn.push_back(QColor(200, 171, 55));
//
//  _winter.push_back(QColor(128, 0, 0));
//  _winter.push_back(QColor(0, 85, 68));
//  _winter.push_back(QColor(51, 51, 51));
//  _winter.push_back(QColor(198, 175, 233));
//  _winter.push_back(QColor(22, 80, 22));
//  _winter.push_back(Qt::black);
//  _winter.push_back(QColor(102, 0, 128));
//  _winter.push_back(Qt::white);
//  _winter.push_back(QColor(51, 0, 128));
//  _winter.push_back(QColor(100, 255, 255));
  
}

void  NetworkView::setColourTheme(ColourTheme::Theme theme)
{
  const QVector<QColor> *colours;
  _currentTheme = theme;
  
  switch (theme)
  {
    case ColourTheme::Camo:
      colours = &ColourTheme::camo();//_camo;
      break;
      
    case ColourTheme::Pastelle:
      colours = &ColourTheme::pastelle();//&_pastelle;
      break;
    
    case ColourTheme::Vibrant:
      colours = &ColourTheme::vibrant();//&_vibrant;
      break;
    
    case ColourTheme::Spring:
      colours = &ColourTheme::spring();//&_spring;
      break;
      
    case ColourTheme::Summer:
      colours = &ColourTheme::summer();//&_summer;
      break;
      
    case ColourTheme::Autumn:
      colours = &ColourTheme::autumn();//&_autumn;
      break;
      
    case ColourTheme::Winter:
      colours = &ColourTheme::winter();//&_winter;
      break;
      
    case ColourTheme::Greyscale:
    default:
      colours = &ColourTheme::greyscale();//&_greyscale;
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

QList<QColor> NetworkView::traitColours() const
{
  QList<QColor> cols;

  for (unsigned i = 0; i< _colourTheme.size(); i++)
    cols << _colourTheme.at(i).color();

  return cols;
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

void NetworkView::setEdgeMutationView(EdgeItem::MutationView view)
{
  EdgeItem::setMutationView(view);

  for (unsigned i = 0; i < _edgeItems.size(); i++)
  {
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

void NetworkView::setVertexSize(double rad)
{
  double factor = rad/_vertRadUnit; 
  _vertRadUnit = rad;
  EdgeItem::setVertexSize(rad);
  
  for (unsigned i = 0; i < _vertexItems.size(); i++)
  {
    QRectF rect = _vertexItems.at(i)->rect();
    double newDiam = rect.width() * factor;
    double extraRad = (newDiam - rect.width()) / 2;
    rect.setX(rect.x() - extraRad);
    rect.setY(rect.y() - extraRad);
    rect.setWidth(newDiam);
    rect.setHeight(newDiam);
    _vertexItems.at(i)->setRect(rect);
  }
  
  for (unsigned i = 0; i < _vertexSections.size(); i++)
  {
    QList<QGraphicsEllipseItem*>::iterator sectionIt = _vertexSections[i].begin();
    
    while (sectionIt != _vertexSections[i].end())
    {
      QRectF rect = (*sectionIt)->rect();
      double newDiam = rect.width() * factor;
      double extraRad = (newDiam - rect.width()) / 2;
      rect.setX(rect.x() - extraRad);
      rect.setY(rect.y() - extraRad);
      rect.setWidth(newDiam);
      rect.setHeight(newDiam);
      (*sectionIt)->setRect(rect);
      ++sectionIt;
    }
  }

  
  /*
   * Update sizes for vertex objects (maybe set signal to pie sections)
   * Delete legend/labels/keys
   * draw legend again
   */
  
  if (_legend)
  {
    _theScene.removeItem(_legend);
    delete _legend;
    _legend = 0;
    _legendKeys.clear();
    _legendLabels.clear();
  }
  
  drawLegend();
  

  //clearScene();
  //adjustAndDraw();
  _theScene.update(0, 0, _theScene.width(), _theScene.height());
}

void NetworkView::setLabelFont(const QFont & font)
{
  _labelFont = font;
  EdgeItem::setFont(_labelFont);
  //clearScene();
  //adjustAndDraw();
  
  _theScene.update(0, 0, _theScene.width(), _theScene.height());

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
    QPointF pos = item->mapToScene(item->boundingRect().center());;
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
    QPointF pos = item->mapToScene(item->boundingRect().center());
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



