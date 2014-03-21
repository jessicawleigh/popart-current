#include "MonospaceMessageBox.h"


#include <QAbstractButton>
#include <QBoxLayout>
#include <QFont>
#include <QFrame>
#include <QList>
#include <QStyle>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QTextDocument>

#include <QDebug>


MonospaceMessageBox::MonospaceMessageBox(QWidget *parent, Qt::WindowFlags f)
 : QDialog(parent, f) //, _detailedTextArea(this), _detailedTextButton("Show Details...", this)
{
  _detailedTextArea = 0;
  _line = 0;
  _textLabel = new QLabel("", this);
  _infTextLabel = new QLabel("", this);
  _detailedTextButton = 0;
 
  //QHBoxLayout *outerLayout = new QHBoxLayout(this);
  
  _outerLayout = new QVBoxLayout(this);
  _topLayout = new QHBoxLayout; // icon, labels, buttons
  _mainLayout = new QVBoxLayout; // labels and buttons
  _buttonLayout = new QHBoxLayout;
  
  QLabel *iconLabel = new QLabel("icon", this);
  QIcon infoIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
  //foreach (QSize s, infoIcon.availableSizes()) qDebug() << s;
  iconLabel->setPixmap(infoIcon.pixmap(64, 64));  
  _topLayout->addWidget(iconLabel);
  
  QPushButton *okButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton), "OK", this);
  okButton->setDefault(true);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  
  _buttonLayout->setDirection(QBoxLayout::RightToLeft);
  _buttonLayout->addWidget(okButton, 0, Qt::AlignRight);
  
  _mainLayout->addWidget(_textLabel);
  _mainLayout->addWidget(_infTextLabel);
  _mainLayout->addLayout(_buttonLayout);
  _topLayout->addLayout(_mainLayout);

  _outerLayout->addLayout(_topLayout);   
}


void MonospaceMessageBox::setText(const QString &text)
{
  _text = text;
  _textLabel->setText(_text);
}

void MonospaceMessageBox::setInformativeText(const QString &text)
{
  _informativeText = text;
  _infTextLabel->setText(_informativeText);
}

void MonospaceMessageBox::setDetailedText(const QString &text)
{
  _detailedText = text;  
    
  if (! _detailedTextButton)
  {
    _detailedTextButton = new QPushButton("Show Details...", this);
      _detailedTextButton->setCheckable(true);
    _detailedTextButton->setChecked(false);
    connect(_detailedTextButton, SIGNAL(toggled(bool)), this, SLOT(toggleShowDetailedText(bool)));
    _buttonLayout->addWidget(_detailedTextButton, 0, Qt::AlignRight); 
    _buttonLayout->addStretch(1);
    
  }
  
  if (! _detailedTextArea)
  {
    _line = new QFrame(this);
    _line->setFrameShape(QFrame::HLine);
    _line->setFrameShadow(QFrame::Sunken);
    _outerLayout->addWidget(_line);
    
    _detailedTextArea = new QPlainTextEdit(text, this);
    QTextDocument* doc = _detailedTextArea->document();
    QFont font(doc->defaultFont());
    font.setFamily("monospace");
    doc->setDefaultFont(font);
    
    _detailedTextArea->setReadOnly(true);
    _outerLayout->addWidget(_detailedTextArea);
    _line->hide();
    _detailedTextArea->hide();
    _detailedTextArea->setLineWrapMode(QPlainTextEdit::NoWrap);
  }
  
  else
    _detailedTextArea->setPlainText(_detailedText);

}

void MonospaceMessageBox::toggleShowDetailedText(bool show)
{
  if (show)
  {
    _detailedTextButton->setText("Hide Details...");
    _detailedTextArea->show();
    _line->show();
  }
  
  else
  {
    _detailedTextButton->setText("Show Details...");
    _detailedTextArea->hide();
    _line->hide();
  }
  
  adjustSize();
}

/*MonospaceMessageBox::~MonospaceMessageBox()
{
  qDebug() << "in MonospaceMessageBox destructor";
  foreach (QAbstractButton *button, buttons())
  {
    qDebug() << button->text();
  }
}*/


//MonospaceMessageBox::MonospaceMessageBox(Icon icon, const QString & title, const QString & text, StandardButtons buttons = NoButton, QWidget * parent = 0, Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);