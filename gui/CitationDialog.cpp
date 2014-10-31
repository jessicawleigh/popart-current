#include "CitationDialog.h"

CitationDialog::CitationDialog(QWidget * parent, Qt::WindowFlags flags)
 : QDialog(parent, flags)
{
  setupCitations();
}

void CitationDialog::setupCitations()
{}


CitationDialog::CitationRecord::CitationRecord(const QString & author, const QString & title, const QString & journal, unsigned year, const QString & abbrevJ, int vol, int issue, int startP, int endP)
{
  
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

void CitationDialog::CitationRecord::setPages(unsigned start, int end)
{
  _startP = start;
  _endP = end;
}
