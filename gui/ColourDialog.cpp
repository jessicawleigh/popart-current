#include "ColourDialog.h"

#include "XPM.h"
#include "NetworkView.h"

#include <QIcon>
#include <QLayout>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QSize>
#include <QToolButton>

#include <iostream>
using namespace std;


//NetworkView::ColourTheme ColourDialog::_theme = NetworkView::defaultColourTheme();

ColourDialog::ColourDialog(QWidget *parent, NetworkView::ColourTheme currentTheme, Qt::WindowFlags flags)
 : QDialog(parent, flags), _iconSize(64, 64)
{
  _theme = currentTheme;

  QVBoxLayout *layout = new QVBoxLayout(this);
  
  QGridLayout *glayout = new QGridLayout();
  glayout->setSpacing(0);
  layout->addLayout(glayout);
  
  QButtonGroup *buttons = new QButtonGroup(this);
  int idCount = 0;
  
  QToolButton *themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::greyscale)));//, "", this);
  themeButton->setToolTip("Greyscale");
  themeButton->setCheckable(true);  
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 0, 0);
  
  themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::camo)));
  themeButton->setToolTip("Camo");
  themeButton->setCheckable(true);
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 0, 1);

  themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::vibrant)));
  themeButton->setToolTip("Vibrant");
  themeButton->setCheckable(true);
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 0, 2);

  themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::pastelle)));
  themeButton->setToolTip("Pastelle");
  themeButton->setCheckable(true);
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 0, 3);

  themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::spring)));
  themeButton->setToolTip("Spring");
  themeButton->setCheckable(true);
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 1, 0);

  themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::summer)));
  themeButton->setToolTip("Summer");
  themeButton->setCheckable(true);
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 1, 1);

  themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::autumn)));
  themeButton->setToolTip("Autumn");
  themeButton->setCheckable(true);
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 1, 2);

  themeButton = new QToolButton(this);
  themeButton->setIcon(QIcon(QPixmap(xpm::winter)));
  themeButton->setToolTip("Winter");
  themeButton->setCheckable(true);
  themeButton->setIconSize(_iconSize);
  themeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttons->addButton(themeButton, idCount++);
  glayout->addWidget(themeButton, 1, 3);

  
  buttons->button(static_cast<int>(currentTheme))->setChecked(true);
  connect(buttons, SIGNAL(buttonClicked(int)), this, SLOT(changeColour(int)));
  
  
  QHBoxLayout *hlayout = new QHBoxLayout();
  //hlayout->addWidget(buttons);
  layout->addLayout(hlayout);
  
  QPushButton *okbutton = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton), "Apply", this);
  connect(okbutton, SIGNAL(clicked()), this, SLOT(accept()));
  hlayout->addWidget(okbutton, 0, Qt::AlignRight);
  
  QPushButton *cancelbutton = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel", this);
  connect(cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
  hlayout->addWidget(cancelbutton, 0, Qt::AlignRight);

  hlayout->addStretch(1);
  setSizeGripEnabled(false);
  
  //return _theme;
}

void ColourDialog::changeColour(int ID)
{
  switch (ID)
  {
    case 1:
      _theme = NetworkView::Camo;
      break;
    case 2:
      _theme = NetworkView::Vibrant;
      break;
    case 3:
      _theme = NetworkView::Pastelle;
      break;
    case 4:
      _theme = NetworkView::Spring;
      break;
    case 5:
      _theme = NetworkView::Summer;
      break;
    case 6:
      _theme = NetworkView::Autumn;
      break;
    case 7:
      _theme = NetworkView::Winter;
      break;
    case 0:
    default:
      _theme = NetworkView::Greyscale;
      break;
  }
}

NetworkView::ColourTheme ColourDialog::getColour(QWidget *parent, NetworkView::ColourTheme currentTheme, Qt::WindowFlags flags)
{
  
  ColourDialog *colDlg = new ColourDialog(parent, currentTheme, flags);
  colDlg->setWindowTitle(tr("Select a new Colour Theme"));
  //qDlg->setLabelText(label);
  
  //ColourTheme oldTheme = _theme;
  
  int returnval = colDlg->exec();
  
  if (returnval != Accepted)  return currentTheme;
    //_theme = oldTheme;
  
  //return (returnval == Accepted);

  return colDlg->theme();
}