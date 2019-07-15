#include "ColourDialog.h"

#include "XPM.h"
#include "NetworkView.h"

#include <QCheckBox>
#include <QIcon>
#include <QLayout>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QSize>
#include <QToolButton>

#include <iostream>
using namespace std;


ColourDialog::ColourDialog(QWidget *parent, ColourTheme::Theme currentTheme, Qt::WindowFlags flags)
 : QDialog(parent, flags), _iconSize(64, 64)
{
  _theme = currentTheme;
  _changeMapTheme = true;
  _changeNetTheme = true;

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
  hlayout->setDirection(QBoxLayout::RightToLeft);
  //hlayout->addWidget(buttons);
  layout->addLayout(hlayout);
  
  QCheckBox *mapCheckBox = new QCheckBox("Apply to map", this);
  mapCheckBox->setCheckState(Qt::Checked);
  connect(mapCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleChangeMapTheme(int)));
  hlayout->addWidget(mapCheckBox, 0, Qt::AlignRight);

  QCheckBox *netCheckBox = new QCheckBox("Apply to network", this);
  netCheckBox->setCheckState(Qt::Checked);
  connect(netCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleChangeNetTheme(int)));
  hlayout->addWidget(netCheckBox, 0, Qt::AlignRight);

  hlayout->addStretch(1);

  hlayout = new QHBoxLayout();
  hlayout->setDirection(QBoxLayout::RightToLeft);
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
      _theme = ColourTheme::Camo;
      break;
    case 2:
      _theme = ColourTheme::Vibrant;
      break;
    case 3:
      _theme = ColourTheme::Pastelle;
      break;
    case 4:
      _theme = ColourTheme::Spring;
      break;
    case 5:
      _theme = ColourTheme::Summer;
      break;
    case 6:
      _theme = ColourTheme::Autumn;
      break;
    case 7:
      _theme = ColourTheme::Winter;
      break;
    case 0:
    default:
      _theme = ColourTheme::Greyscale;
      break;
  }
}

ColourTheme::Theme ColourDialog::getColour(QWidget *parent, ColourTheme::Theme currentTheme, Qt::WindowFlags flags, bool *changeNetTheme, bool *changeMapTheme)
{
  
  ColourDialog *colDlg = new ColourDialog(parent, currentTheme, flags);
  colDlg->setWindowTitle(tr("Select a new Colour Theme"));
  //qDlg->setLabelText(label);
  
  //ColourTheme oldTheme = _theme;
  
  int returnval = colDlg->exec();

  *changeNetTheme = false;
  *changeMapTheme = false;

  if (returnval != Accepted)  return currentTheme;
    //_theme = oldTheme;
  if (changeNetTheme)
    *changeNetTheme = colDlg->changeNetTheme();

  if (changeMapTheme)
    *changeMapTheme = colDlg->changeMapTheme();

  //return (returnval == Accepted);

  return colDlg->theme();
}
