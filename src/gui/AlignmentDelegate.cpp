#include <QString>
#include <QStyle>
#include <QChar>
#include <iostream>
using namespace std;

#include "AlignmentDelegate.h"

AlignmentDelegate::AlignmentDelegate(Sequence::CharType chartype, QObject *parent)
  : QAbstractItemDelegate(parent)
{
  _chartype = chartype;
  setColourMap(defaultColourMap(_chartype));
}

void AlignmentDelegate::paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  /*  Wrong: should first check if can convert */
  QString data = index.data().toString().toUpper();
  bool maskstate = index.data(Qt::UserRole).toBool();

  char seqchar = data[0].toLatin1();
  if (option.state & QStyle::State_Selected)
  {
    if (!maskstate)  painter->fillRect(option.rect, option.palette.dark());
    else
      painter->fillRect(option.rect, option.palette.highlight());
    painter->setPen(option.palette.highlightedText().color()); //Qt::white
  }
  
  else
  {
    if (!maskstate)  
    {
      painter->fillRect(option.rect, option.palette.alternateBase());
      painter->setPen(Qt::darkGray);
      
    }
    else
    {
      chcolourmap::const_iterator result = _colourmap.find(seqchar);
      if (result != _colourmap.end())
        painter->setPen(result->second);
      else
        painter->setPen(Qt::black); 
    }
  }
  
  painter->drawText(option.rect, Qt::AlignCenter, data);
}

QSize AlignmentDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QString data = index.data().toString();

  return QSize(option.fontMetrics.width(data), option.fontMetrics.height());
}

chcolourmap AlignmentDelegate::defaultColourMap(Sequence::CharType chartype) const
{
  chcolourmap colourmap;
  if (chartype == Sequence::AAType)
  {
    QColor purple(142, 13, 225);
    QColor red(213, 41, 32);
    QColor green(25, 146, 32);
    //QColour green(134, 211, 57);
    //QColor orange(255, 128, 0);
    QColor darkyellow(249, 206, 18);
    QColor pink(225, 13, 187);
    QColor blue(13, 139, 225);

    colourmap.insert(chcolourmap::value_type('M', red));
    colourmap.insert(chcolourmap::value_type('V', red));
    colourmap.insert(chcolourmap::value_type('I', red));
    colourmap.insert(chcolourmap::value_type('L', red));
    
    colourmap.insert(chcolourmap::value_type('F', pink));
    colourmap.insert(chcolourmap::value_type('Y', pink));
    colourmap.insert(chcolourmap::value_type('W', pink));
    
    colourmap.insert(chcolourmap::value_type('C', darkyellow));
    
    colourmap.insert(chcolourmap::value_type('D', purple));
    colourmap.insert(chcolourmap::value_type('N', purple));
    colourmap.insert(chcolourmap::value_type('E', purple));
    colourmap.insert(chcolourmap::value_type('Q', purple));
    
    colourmap.insert(chcolourmap::value_type('R', green));
    colourmap.insert(chcolourmap::value_type('K', green));
    colourmap.insert(chcolourmap::value_type('H', green));
    
    colourmap.insert(chcolourmap::value_type('A', blue));
    colourmap.insert(chcolourmap::value_type('S', blue));
    colourmap.insert(chcolourmap::value_type('T', blue));
    colourmap.insert(chcolourmap::value_type('G', blue));
    colourmap.insert(chcolourmap::value_type('P', blue));
  }
  
  else if (chartype == Sequence::DNAType)
  {
    QColor red(213, 41, 32);
    QColor green(134, 211, 57);
    QColor blue(13, 139, 225);
    QColor darkyellow(249, 206, 18);
    
    colourmap.insert(chcolourmap::value_type('A', blue));
    colourmap.insert(chcolourmap::value_type('T', darkyellow));
    colourmap.insert(chcolourmap::value_type('U', darkyellow));
    colourmap.insert(chcolourmap::value_type('G', red));
    colourmap.insert(chcolourmap::value_type('C', green));
  }
  
  else
  {
    QColor red(213, 41, 32);
    QColor blue(13, 139, 225);
    
    colourmap.insert(chcolourmap::value_type('0', blue));
    colourmap.insert(chcolourmap::value_type('1', red)); 
  }
  
  return colourmap;
}

chcolourmap AlignmentDelegate::colourMap() const
{
  return _colourmap;
}

// Print warnings if this appears to be neither amino acid nor DNA colourmap
void AlignmentDelegate::setColourMap(chcolourmap map)
{
  _colourmap = map;
}

Sequence::CharType AlignmentDelegate::charType() const
{
  return _chartype; 
}

void AlignmentDelegate::setCharType(Sequence::CharType chartype)
{
  _chartype = chartype;
  _colourmap = defaultColourMap(chartype);
}

/*
QFont AlignmentDelegate::defaultFont() const
{
  return QFont("Helvetica");
}

QFont AlignmentDelegate::font() const
{
  return _font;
}

void AlignmentDelegate::setFont(QFont font)
{
  _font = font;
} */
