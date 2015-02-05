#include "CitationDialog.h"

#include <QCheckBox>
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>

#include <QDebug>

CitationDialog::CitationDialog(QWidget * parent, Qt::WindowFlags flags)
 : QDialog(parent, flags), _currentFormat(CitationDialog::CitationRecord::Apa) 
{
  setupCitations();

  _checkboxes.setExclusive(false);
  _radioButtons.setExclusive(true);

  _citationDisplay = new QPlainTextEdit(this);

  _citationDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
  _citationDisplay->setLineWrapMode(QPlainTextEdit::WidgetWidth);

  setWindowTitle("Useful Citations");
  
  QVBoxLayout *mainlayout = new QVBoxLayout(this);
  QHBoxLayout *hlayout = new QHBoxLayout;
  QVBoxLayout *vlayout = new QVBoxLayout;

  mainlayout->addWidget(new QLabel("<b>If you use PopART's network inference methods, please cite:</b>", this));

  QMap<QString, CitationRecord>::const_iterator citeIt = _citations.constBegin();

  while (citeIt != _citations.constEnd())
  {
    QCheckBox *item = new QCheckBox(citeIt.key(), this);
    item->setChecked(true);

    connect(item, SIGNAL(toggled(bool)), this, SLOT(updateCitationDisplay(bool)));
    _checkboxes.addButton(item);

    vlayout->addWidget(item);

  	++citeIt;
  }

  hlayout->addLayout(vlayout);
  vlayout = new QVBoxLayout;

  QGroupBox *groupBox = new QGroupBox("Citation format");
  int buttonCount = 0;

  QRadioButton *formatButton = new QRadioButton("APA", this);
  formatButton->setChecked(true);
  _radioButtons.addButton(formatButton, buttonCount++);
  connect(formatButton, SIGNAL(toggled(bool)), this, SLOT(updateCurrentFormat(bool)));
  vlayout->addWidget(formatButton);

  formatButton = new QRadioButton("BibTeX", this);
  _radioButtons.addButton(formatButton, buttonCount++);
  connect(formatButton, SIGNAL(toggled(bool)), this, SLOT(updateCurrentFormat(bool)));
  vlayout->addWidget(formatButton);

  formatButton = new QRadioButton("EndNote", this);
  _radioButtons.addButton(formatButton, buttonCount++);
  connect(formatButton, SIGNAL(toggled(bool)), this, SLOT(updateCurrentFormat(bool)));
  vlayout->addWidget(formatButton);

  formatButton = new QRadioButton("RIS (RefMan)", this);
  _radioButtons.addButton(formatButton, buttonCount++);
  connect(formatButton, SIGNAL(toggled(bool)), this, SLOT(updateCurrentFormat(bool)));
  vlayout->addWidget(formatButton);

  groupBox->setLayout(vlayout);
  hlayout->addWidget(groupBox);
  mainlayout->addLayout(hlayout);

  mainlayout->addWidget(_citationDisplay);
  updateCitationDisplay(true);

  //connect(&_checkboxes, SIGNAL(buttonReleased(QAbstractButton *)), this, SLOT(updateCitationDisplay(QAbstractButton *)));
  
  // checkboxes to select methods that need to be cited
  // show/hide selectable (not editable) text area
  // select all, copy
  // button to copy all (selected) citations
  // allow output of different formats, similar to Google Scholar

  qDebug() << "unicode version:" << QChar().unicodeVersion();
  qDebug() << "endash character:" << QChar(0x2013);
}

void CitationDialog::setupCitations()
{
  CitationRecord mjcr("Bandelt, Hans-Jurgen", "Median-joining networks for inferring intraspecific phylogenies", "Molecular Biology and Evolution", 1999, "Mol. Biol. Evol.", 16, 1, 37, 48);

  mjcr.addAuthor("Forster, Peter");
  mjcr.addAuthor(QString("R%1hl, Arne").arg(QChar(0x00f6)));

  _citations.insert("Median-Joining and Minimum spanning networks", mjcr);
	

  CitationRecord tcscr("Clement, Mark", "TCS: Estimating gene genealogies", "Parallel and Distributed Processing Symposium, International Proceedings", 2002, "I. P. D. P. S.", 2, -1, 184, 184);

  tcscr.addAuthor("Snell, Quinn");
  tcscr.addAuthor("Walker, Peter");
  tcscr.addAuthor("Posada, David");
  tcscr.addAuthor("Crandall, Keith");

  _citations.insert("TCS networks", tcscr);

  CitationRecord injcr("French, Nigel", "Evolution of Campylobacter species in New Zealand", "Campylobacter Ecology and Evolution", 2014, "", -1, 17, 221, 240, CitationRecord::Book, "Caister Academic Press", "Norfolk, England", "Sheppard, Samuel K.");

  injcr.addAuthor("Yu, Shoukai");
  injcr.addAuthor("Biggs, Patrick");
  injcr.addAuthor("Holland, Barbara");
  injcr.addAuthor("Fearnhead, Paul");
  injcr.addAuthor("Binney, Barbara");
  injcr.addAuthor("Fox, Andrew");
  injcr.addAuthor("Grove-White, Dai");
  injcr.addAuthor("Leigh, Jessica W.");
  injcr.addAuthor("Miller, William");
  injcr.addAuthor("Muellner, Petra");
  injcr.addAuthor("Carter, Philip");

  injcr.addEditor(QString("M%1ric, Guillaume").arg(QChar(0x00e9)));

  _citations.insert("Integer NJ networks", injcr);

}

void CitationDialog::updateCitationDisplay(bool checked)//QAbstractButton *button)
{
	//Q_UNUSED(button);
	//Q_UNUSED(checked);
  if (! checked)  return;
  
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
  	     displayText << cr.formatCitation(_currentFormat);
  	   }
  	 }

	}
  
  // TODO figure out whether unicode (used for MJN reference) or html works for QPlainTextEdit
  _citationDisplay->setPlainText(displayText.join("\n"));

}

void CitationDialog::updateCurrentFormat(bool checked)
{

	if (! checked)  return;
	qDebug() << "button ID:" << _radioButtons.checkedId();
  _currentFormat = static_cast<CitationRecord::CitationFormat>(_radioButtons.checkedId());

	updateCitationDisplay(true);
}


CitationDialog::CitationRecord::CitationRecord(const QString & author, const QString & title, const QString & journal, unsigned year, const QString & abbrevJ, int vol, int issue, int startP, int endP, CitationDialog::CitationRecord::CitationType type, const QString & publisher, const QString & location, const QString & editor)
 : _leadAuthor(author), _title(title), _journal(journal), _year(year), _abbrevJ(abbrevJ), _vol(vol), _issue(issue), _startP(startP), _endP(endP), _type(type), _publisher(publisher), _location(location)
{
	if (! editor.isEmpty())
		_editors.push_back(editor);


}

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

void CitationDialog::CitationRecord::setChapter(unsigned chapter)
{
	setIssue(chapter);
}

void CitationDialog::CitationRecord::setPages(unsigned start, int end)
{
  _startP = start;
  _endP = end;
}

void CitationDialog::CitationRecord::setType(CitationDialog::CitationRecord::CitationType type)
{
	_type = type;
}

void CitationDialog::CitationRecord::setPublisher(const QString & publisher)
{
	_publisher = publisher;
}

void CitationDialog::CitationRecord::addEditor(const QString & editor)
{
	_editors.push_back(editor);
}

QString CitationDialog::CitationRecord::formatAuthor(const QString & author, CitationDialog::CitationRecord::CitationFormat format)
{
	int idx = author.indexOf(',');
	if (idx < 0)
	{
		qDebug() << "comma not found"; 
		return author;
	}

	QString last = author.left(idx);

	int remainder = author.length() - idx; // just use section, since we can't use refs anyway.

	QStringList otherNames = author.right(remainder - 1).split(' ', QString::SkipEmptyParts);

  switch (format)
  {
  	case Apa:
  	{
      QString initials;

      foreach (QString name, otherNames)
      {
      	if (initials.isEmpty())
      		initials += QString("%1.").arg(name.at(0));
      	else
      		initials += QString(" %1.").arg(name.at(0));
      }

      return QString("%1, %2").arg(last).arg(initials);
      break;
    }

    case BibTeX:
    {

    	return QString("%1, %2").arg(last).arg(otherNames.join(QChar(' ')));
    	break;
    }

    case EndNote:
    case RIS:
    {
    	QStringList name;
    	name << last << ",";

      QString initials;

      foreach (QString part, otherNames)
      {
      	if (part.at(part.size() - 1) == '.') // This is an initial, or a Jr.
      	{
      		if (part.size() == 2)
      			initials += part.at(0);
      		else // a Jr.
      		{
      			if (! initials.isEmpty())
      			{
      				if (name.isEmpty())
        				name << initials;
        		  else
        		  	name << " " << initials;
      				initials = QString();
      			}
    				name << QString(", %1").arg(part.left(part.size() - 1));
      		}
      	}

      	else
      	{
      		if (part.size() > 1)
      		{
      			if (! initials.isEmpty())
      			{
      				if (name.isEmpty())
        				name << initials;
        			else
        				name << " " << initials;
      				initials = QString();
      			}

      			name << QString(" %1").arg(part);
      		}

      		else
      			initials += part;
      	}

      	if (! initials.isEmpty())
      	{
      		if (name.isEmpty())
        		name << initials;
        	else
        		name << " " << initials;
      	}
      }

    	return name.join("");

    	break;
    }

    case LastName:
    {
    	return last;
    	break;
    }

  	default:
  	{
  	 return author;
  	  break;
  	}
  }	

  return author;
}

QString CitationDialog::CitationRecord::formatEditor(const QString & editor, CitationDialog::CitationRecord::CitationFormat format)
{
	return formatAuthor(editor, format);
}

QString CitationDialog::CitationRecord::formatCitation(CitationDialog::CitationRecord::CitationFormat format) const
{

	if (_type == CitationRecord::Book)
		return formatBookCitation(format);

	QStringList citation;

  switch (format)
  {
  	case Apa:
  	{
			for (int i = 0; i < _authors.size(); i++)
			{
				const QString & author = _authors.at(i);

		    if (i == 0)
		    	citation << formatAuthor(author, format);
		    else if (i < _authors.size() - 1)
		    	citation << ", " << formatAuthor(author, format);
		    else
		    	citation << ", & " << formatAuthor(author, format);
			}

			citation << QString(" (%1). %2.").arg(_year).arg(_title);

		  citation << QString(" %1,").arg(_journal);

		  if (_vol >= 0)
		  {
		  	citation << QString(_vol);

		  	if (_issue >= 0)
		  		citation << QString(" %1(%2), ").arg(_vol).arg(_issue);

		  	else
		  		citation << QString(" %1, ").arg(_vol);
		  }

		  if (_startP >= 0)
		  {
		  	if (_endP > _startP)
		  		citation << QString("%1%2%3.").arg(_startP).arg(QChar(0x2013)).arg(_endP);

		  	else
		  		citation << QString("%1.").arg(_startP);
		  }

		  break;
		}

		case BibTeX:
		{
			citation << QString("@article{%1%2,\n").arg(formatAuthor(_leadAuthor, CitationRecord::LastName)).arg(_year);

			citation << QString("  title={%1},\n").arg(_title);
			citation << QString("  author={");

			for (int i = 0; i < _authors.size(); i++)
			{
				const QString & author = _authors.at(i);

		    if (i == 0)
		    	citation << formatAuthor(author, format);
		    else
		    	citation << " and " << formatAuthor(author, format);
			}
      
			citation << QString("},\n");

			citation << QString("  journal={%1},\n").arg(_journal);

			if (_vol >= 0)
				citation << QString("  volume={%1},\n").arg(_vol);

			if (_issue >= 0)
				citation << QString("  number={%1},\n").arg(_issue);

			if (_startP >= 0)
			{
				if (_endP > _startP)
					citation << QString("  pages={%1--%2},\n").arg(_startP).arg(_endP);
				else
					citation << QString("  pages={%1},\n").arg(_startP);
			}

			citation << QString("  year={%1}\n}\n").arg(_year);

			break;
		}

		case EndNote:
		{
			citation << "%0 Journal Article\n";
			citation << QString("%T %1\n").arg(_title);

			foreach (QString author, _authors)
			  citation << QString("%A %1\n").arg(formatAuthor(author, format));

		  citation << QString("%J %1\n").arg(_journal);

		  if (_vol >= 0)
  		  citation << QString("%V %1\n").arg(_vol);

  		if (_issue >= 0)
  			citation << QString("%N %1\n").arg(_issue);

			if (_startP >= 0)
			{
				if (_endP > _startP)
					citation << QString("%P %1-%2\n").arg(_startP).arg(_endP);
				else
					citation << QString("%P %1\n").arg(_startP);
			}

			citation << QString("%D %1\n\n").arg(_year);  		

			break;
		}

		case RIS:
		{
			citation << "TY  - JOUR\n";
			citation << QString("TI  - %1\n").arg(_title); // T1

			foreach (QString author, _authors)
			  citation << QString("AU  - %1\n").arg(formatAuthor(author, format)); // A1

		  citation << QString("JF  - %1\n").arg(_journal); // JO

		  if (! _abbrevJ.isEmpty())
		  	citation << QString("JA  - %1\n").arg(_abbrevJ);

		  if (_vol >= 0)
  		  citation << QString("VL  - %1\n").arg(_vol);

  		if (_issue >= 0)
  			citation << QString("IS  - %1\n").arg(_issue);

			if (_startP >= 0)
			{
				citation << QString("SP  - %1\n").arg(_startP);
				if (_endP > _startP)
					citation << QString("EP  - %1\n").arg(_endP);
			}

			citation << QString("PY  - %1\n").arg(_year); // Y1
			citation << "ER  - \n";

			break;
		}

		default:
		{
    	citation << "Unknown article citation format.";
	    break;
	  }
  }

	//citation << QString(" (%1). %2. %3, %4(%5), %6%7%8.").arg(_year).arg(_title).arg(_abbrevJ).arg(_vol).arg(_issue).arg(_startP).arg(QChar(0x2013)).arg(_endP);


	return citation.join(QChar());
}

QString CitationDialog::CitationRecord::formatBookCitation(CitationDialog::CitationRecord::CitationFormat format) const
{
	QStringList citation;

	switch (format)
	{
    case Apa:
    {
			for (int i = 0; i < _authors.size(); i++)
			{
				const QString & author = _authors.at(i);

		    if (i == 0)
		    	citation << formatAuthor(author, format);
		    else if (i < _authors.size() - 1)
		    	citation << ", " << formatAuthor(author, format);
		    else
		    	citation << ", & " << formatAuthor(author, format);
			}

			citation << QString(" (%1). %2. In ").arg(_year).arg(_title);


			for (int i = 0; i < _editors.size(); i++)
			{
				const QString & ed = _editors.at(i);

		    if (i == 0)
		    	citation << formatAuthor(ed, format);
		    else if (i < _editors.size() - 1)
		    	citation << ", " << formatEditor(ed, format);
		    else if (_editors.size() > 2)
		    	citation << ", & " << formatEditor(ed, format);
		    else
		    	citation << " & " << formatEditor(ed, format);
			}
      

      if (_editors.size() > 1)
      	citation << "(Ed.), ";
      else
      	citation << "(Eds.), ";

      citation << _journal; // actually a book title

      if (_startP >= 0)
      {
      	if (_endP > _startP)
      		citation << QString(" (pp. %1%2%3). ").arg(_startP).arg(QChar(0x2013)).arg(_endP);

      	else
      		citation << QString(" (p. %1). ").arg(_startP);

      }

      if (! _location.isEmpty())
      	citation << QString("%1: ").arg(_location);

      citation << QString("%1.").arg(_publisher);


      break;
    }
		case BibTeX:
		{
			citation << QString("@incollection{%1%2,\n").arg(formatAuthor(_leadAuthor, CitationRecord::LastName)).arg(_year);

			citation << QString("  title={%1},\n").arg(_title);
			citation << QString("  author={");

			for (int i = 0; i < _authors.size(); i++)
			{
				const QString & author = _authors.at(i);

		    if (i == 0)
		    	citation << formatAuthor(author, format);
		    else
		    	citation << " and " << formatAuthor(author, format);
			}
      
			citation << QString("},\n");

			citation << QString("  booktitle={%1},\n").arg(_journal);

			if (_vol >= 0)
				citation << QString("  volume={%1},\n").arg(_vol);

			if (_issue >= 0)
				citation << QString("  chapter={%1},\n").arg(_issue);

		  citation << QString("  editor={");

			for (int i = 0; i < _editors.size(); i++)
			{
				const QString & ed = _editors.at(i);

		    if (i == 0)
		    	citation << formatEditor(ed, format);
		    else
		    	citation << " and " << formatAuthor(ed, format);
			}
      
			citation << QString("},\n");


			if (_startP >= 0)
			{
				if (_endP > _startP)
					citation << QString("  pages={%1--%2},\n").arg(_startP).arg(_endP);
				else
					citation << QString("  pages={%1},\n").arg(_startP);
			}

			if (! _location.isEmpty())
				citation << QString("  address={%1},\n").arg(_location);

			citation << QString("  publisher={%1},\n").arg(_publisher);
			citation << QString("  year={%1}\n}\n").arg(_year);

			break;
		}

		case EndNote:
		{
			citation << "%0 Book Section\n";
			citation << QString("%T %1\n").arg(_title);

			foreach (QString author, _authors)
			  citation << QString("%A %1\n").arg(formatAuthor(author, format));

		  citation << QString("%B %1\n").arg(_journal); // book title

		  if (_vol >= 0)
  		  citation << QString("%V %1\n").arg(_vol);

		  if (_issue >= 0)
  		  citation << QString("%& %1\n").arg(_issue);

			foreach (QString ed, _editors)
			  citation << QString("%E %1\n").arg(formatEditor(ed, format));

			if (_startP >= 0)
			{
				if (_endP > _startP)
					citation << QString("%P %1-%2\n").arg(_startP).arg(_endP);
				else
					citation << QString("%P %1\n").arg(_startP);
			}

			if (! _location.isEmpty())
  			citation << QString("%C %1\n").arg(_location);
			citation << QString("%I %1\n").arg(_publisher);

			citation << QString("%D %1\n").arg(_year);  		

			break;
		}

		case RIS:
		{
			citation << "TY  - CHAP\n";
			citation << QString("TI  - %1\n").arg(_title); // T1

			foreach (QString author, _authors)
			  citation << QString("AU  - %1\n").arg(formatAuthor(author, format)); // A1

		  citation << QString("BT  - %1\n").arg(_journal); // T2

		  if (_vol >= 0)
  		  citation << QString("VL  - %1\n").arg(_vol);

      // Not sure about this one
  		//if (_issue >= 0)
  		//	citation << QString("CP  - %1\n").arg(_issue);

			foreach (QString ed, _editors)
			  citation << QString("ED  - %1\n").arg(formatEditor(ed, format)); // A2

			if (_startP >= 0)
			{
				citation << QString("SP  - %1\n").arg(_startP);
				if (_endP > _startP)
					citation << QString("EP  - %1\n").arg(_endP);
			}

			if (! _location.isEmpty())
  			citation << QString("CY  - %1\n").arg(_location);
			citation << QString("PB  - %1\n").arg(_publisher);

			citation << QString("PY  - %1\n").arg(_year);
			citation << "ER  - \n";

			break;
		}

    default:
    {
    	citation << "Unknown book citation format.";
      break;
    }
	}

	return citation.join(QChar());
}




