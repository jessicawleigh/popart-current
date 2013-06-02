#include "MapLegendWidget.h"
#include "NetworkItem.h"

#include <QBrush>
#include <QColor>
#include <QFontMetrics>
#include <QMap>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QSize>
#include <QString>

MapLegendWidget::MapLegendWidget(QVector<HapLocation*> locations, QWidget *parent)
 : QWidget(parent), _hapLocations(locations), _defaultBrush(Qt::black)
 {
   
   setMinimumSize(400, 400);
   setGeometry(QRect(0,0,400,400)); // Get rid of this?

  _defaultFont.setFamily("Baskerville");
  _defaultFont.setPointSize(10);
  _legendFont = _defaultFont;
  
  _smallFont = QFont(_defaultFont);
  _smallFont.setPointSize(6);
  
  _colours = QVector<QBrush>();

   
}
 
 void MapLegendWidget::paintEvent(QPaintEvent *event)
 {
   
   QPainter painter(this);
   painter.setBackground(Qt::white);
   
   
   const QMap<QString, unsigned> & seqIDs = HapLocation::seqIDs();
   QFontMetrics painterMetric = painter.fontMetrics();
   double vertRadUnit = NetworkItem::VERTRAD;
   painter.setPen(Qt::SolidLine); 
   double maxwidth = 0;

   QMap<QString, unsigned>::const_iterator seqNameIt = seqIDs.constBegin();
   painter.setFont(legendFont());
  
   while (seqNameIt != seqIDs.constEnd())
  {
    double width = painterMetric.width(seqNameIt.key());
    if (width > maxwidth)  maxwidth = width;

    ++seqNameIt;
  }
  
  unsigned margin = 15;
  double minHeight = vertRadUnit + 5;
  double entryHeight = painterMetric.height();
  if (minHeight > entryHeight)
    entryHeight = minHeight;

  double diam5seq = sqrt(10) * vertRadUnit;
  double minLegendWidth = diam5seq + 2 * margin;
  double legendWidth = maxwidth + vertRadUnit + 3 * margin;

  if (legendWidth < minLegendWidth)  legendWidth = minLegendWidth;

  painter.setFont(smallFont());
  painterMetric = painter.fontMetrics();

  double legendHeight = entryHeight * seqIDs.size() + painterMetric.height() + diam5seq + 3 * margin;
  
  setMinimumSize(legendWidth, legendHeight);
  painter.eraseRect(QRect(0, 0, width(), height()));

  
  QPointF legendStart(0,0);

  double currentY = legendStart.y() + margin;
  double keyX = legendStart.x() + legendWidth / 2 - diam5seq/2;

  painter.setBrush(Qt::transparent);
  painter.drawEllipse(keyX, currentY, diam5seq, diam5seq);
  QString key("10 samples");

  double textX = legendStart.x() + legendWidth/2 - painterMetric.width(key)/2;//smallMetric.width(key)/2;
  painter.drawText(textX, currentY + diam5seq/2, key);

  currentY += (diam5seq - vertRadUnit);
  keyX = legendStart.x() + legendWidth / 2 - vertRadUnit/2;
  painter.drawEllipse(keyX, currentY, vertRadUnit, vertRadUnit);

  currentY += vertRadUnit + margin; //smallMetric.ascent();
  key = QString("1 sample");
  textX = legendStart.x() + legendWidth/2 - painterMetric.width(key)/2;//smallMetric.width(key)/2;
  painter.drawText(textX, currentY , key);

  currentY += margin;
  keyX = legendStart.x() + margin;
  textX = keyX + vertRadUnit + margin;
  painter.setFont(legendFont());
  painterMetric = painter.fontMetrics();

  _legendKeys = QVector<QRegion>(seqIDs.size());
  seqNameIt = seqIDs.constBegin();
  
  while (seqNameIt != seqIDs.constEnd())
  {
    painter.setBrush(hapBrush(seqNameIt.value()));
    
    painter.drawEllipse(keyX, currentY, vertRadUnit, vertRadUnit);
    painter.drawText(textX, currentY + (painterMetric.ascent() + vertRadUnit)/2, seqNameIt.key());
    _legendKeys[seqNameIt.value()] = QRegion(keyX, currentY, vertRadUnit, vertRadUnit, QRegion::Ellipse);
    
    currentY += entryHeight;
    ++seqNameIt;
  } 
   
 }
 
 QSize MapLegendWidget::minimumSizeHint()
 {
   // TODO set this to something sensible using legend width
   return QSize(400,400);
 }
 
 void MapLegendWidget::changeLegendFont(const QFont &font)
{
  //_legendFont = font;
  //_smallFont = font;
  setLegendFont(font);

  int smallerPointSize = _legendFont.pointSize() * 0.6 + 0.5; 
  QFont smaller(font);
  smaller.setPointSize(smallerPointSize);
  setSmallFont(smaller);
}



const QBrush & MapLegendWidget::hapBrush(int hapID) const
{
  if (_colours.empty() || hapID < 0)  return _defaultBrush;
  return _colours.at(hapID % _colours.size());
}

void MapLegendWidget::setColours(const QVector<QBrush> &colours) 
{ 
  _colours = colours; 
  
}
