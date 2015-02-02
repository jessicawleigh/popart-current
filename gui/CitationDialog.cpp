#include "CitationDialog.h"

#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>

#include <QDebug>

CitationDialog::CitationDialog(QWidget * parent, Qt::WindowFlags flags)
 : QDialog(parent, flags)//, _citationDisplay(this)
{
  setupCitations();

  _checkboxes.setExclusive(false);

  _citationDisplay = new QPlainTextEdit(this);

  _citationDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
  _citationDisplay->setLineWrapMode(QPlainTextEdit::WidgetWidth);

  setWindowTitle("Useful Citations");
  
  QVBoxLayout *layout = new QVBoxLayout(this);

  layout->addWidget(new QLabel("If you use PopART's network inference methods, please cite:", this));

  QMap<QString, CitationRecord>::const_iterator citeIt = _citations.constBegin();

  while (citeIt != _citations.constEnd())
  {
    QCheckBox *item = new QCheckBox(citeIt.key(), this);
    item->setChecked(true);

    connect(item, SIGNAL(toggled(bool)), this, SLOT(updateCitationDisplay(bool)));
    _checkboxes.addButton(item);

    layout->addWidget(item);

  	++citeIt;
  }

  layout->addWidget(_citationDisplay);
  updateCitationDisplay(true);

  //connect(&_checkboxes, SIGNAL(buttonReleased(QAbstractButton *)), this, SLOT(updateCitationDisplay(QAbstractButton *)));
  
  // checkboxes to select methods that need to be cited
  // show/hide selectable (not editable) text area
  // select all, copy
  // button to copy all (selected) citations
  // allow output of different formats, similar to Google Scholar
}

void CitationDialog::setupCitations()
{
  CitationRecord mjcr("Bandelt, Hans-Jurgen", "Median-joining networks for inferring intraspecific phylogenies", "Molecular Biology and Evolution", 1999, "Mol Biol Evol", 16, 1, 37, 48);

  mjcr.addAuthor("Forster, Peter");
  mjcr.addAuthor(QString("R%1hl, Arne").arg(QChar(0xf6)));

  _citations.insert("Median-Joining and Minimum spanning networks", mjcr);
	

  CitationRecord tcscr("Clement, Mark", "TCS: Estimating gene genealogies", "Parallel and Distributed Processing Symposium", 2002, "", 2, 184, 184);

  tcscr.addAuthor("Snell, Quinn");
  tcscr.addAuthor("Walker, Peter");
  tcscr.addAuthor("Posada, David");
  tcscr.addAuthor("Crandall, Keith");

  _citations.insert("TCS networks", tcscr);
}

void CitationDialog::updateCitationDisplay(bool checked)//QAbstractButton *button)
{
	//Q_UNUSED(button);
	Q_UNUSED(checked);

	QStringList displayText;

	foreach (QAbstractButton *item, _checkboxes.buttons())
	{
     if (item->isChecked())
     {
     	 QMap<QString, CitationRecord>::const_iterator citeIt = _citations.find(item->text());

     	 if (citeIt != _citations.end())
     	 {
     	 	 qDebug() << citeIt.value().title();
         const CitationRecord &cr = citeIt.value();
  	     displayText << cr.formatCitation(CitationRecord::Apa);
  	   }
  	 }

	}
  
  // TODO figure out whether unicode (used for MJN reference) or html works for QPlainTextEdit
  _citationDisplay->setPlainText(displayText.join("\n"));

}


CitationDialog::CitationRecord::CitationRecord(const QString & author, const QString & title, const QString & journal, unsigned year, const QString & abbrevJ, int vol, int issue, int startP, int endP)
 : _leadAuthor(author), _title(title), _journal(journal), _year(year), _abbrevJ(abbrevJ), _vol(vol), _issue(issue), _startP(startP), _endP(endP)
{}

void CitationDialog::CitationRecord::addAuthor(const QString & author)
{
  _authors.push_back(author);
}

void CitationDialog::CitationRecord::setAbbrevJ(const QString &abbrev)
{
  _abbrevJ = abbrev;
}

void CitationDialog::CitationRecord::setVol(unsigned vol)
{
  _vol = vol;
}

void CitationDialog::CitationRecord::setIssue(unsigned issue)
{
  _issue = issue;
}

void CitationDialog::CitationRecord::setPages(unsigned start, int end)
{
  _startP = start;
  _endP = end;
}

QString CitationDialog::CitationRecord::formatAuthor(const QString & author, CitationDialog::CitationRecord::CitationFormat format)
{

  switch (format)
  {
  	case (Apa):
  	{

  		int idx = author.indexOf(',');
  		if (idx < 0)
  		{
  			qDebug() << "comma not found"; // TODO make this a warning?
  			return author;
  		}

  		QString last = author.left(idx);

  		int remainder = author.length() - idx; // just use section, since we can't use refs anyway.

  		QStringList otherNames = author.right(remainder - 1).split(' ', QString::SkipEmptyParts);

      QString initials;

      foreach (QString name, otherNames)
      {
      	if (initials.isEmpty())
      		initials += QString("%1.").arg(name.at(0));
      	else
      		initials += QString(" %1.").arg(name.at(0));
      }

      return QString("%1, %2").arg(last).arg(initials);
    }

  	default:
  	{
  	 return author;
  	  break;
  	}
  }	

  return author;
}

QString CitationDialog::CitationRecord::formatCitation(CitationDialog::CitationRecord::CitationFormat format) const
{
	QStringList citation;

	// TODO switch-case here
	foreach (QString author, _authors)
	{
    if (citation.isEmpty())
    	citation << formatAuthor(author, format);
    else
    	citation << ", " << formatAuthor(author, format);
	}

  // TODO check that optional args aren't negative values
  // TODO use N-dash
	citation << QString(" (%1). %2. %3, %4(%5), %6--%7").arg(_year).arg(_title).arg(_abbrevJ).arg(_vol).arg(_issue).arg(_startP).arg(_endP);


	return citation.join(QChar());
}




