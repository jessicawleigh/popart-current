#ifndef CITATIONDIALOG_H
#define CITATIONDIALOG_H

#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
#include <QMap>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>

class CitationDialog : public QDialog
{
  Q_OBJECT
public:
  CitationDialog(QWidget * = 0, Qt::WindowFlags = 0);
  
private:
  class CitationRecord
  {
  public:
    enum CitationFormat {Mla, Apa, Chicago, BibTeX, EndNote, RefMan, RefWorks};

    CitationRecord(const QString &, const QString &, const QString &, unsigned, const QString & = 0, int = -1, int = -1, int = -1, int = -1);
    
    void addAuthor(const QString &);
    void setAbbrevJ(const QString &);
    void setVol(unsigned);
    void setIssue(unsigned);
    void setPages(unsigned, int);
    
    const QString & leadAuthor() const { return _leadAuthor; };
    const QStringList & authors() const { return _authors; };
    const QString & title() const { return _title; };
    const QString & journal(bool abbrev = false) const { if (abbrev)  return _abbrevJ; else  return _journal; };
      //abbrev ? return _abbrevJ : return _journal; };
    unsigned year() const { return _year; };
    int vol() const { return _vol; };
    int issue() const { return _issue; };
    int startPage() const { return _startP; };
    int endPage() const { return _endP; };

    QString formatCitation(CitationFormat) const;

  private:
    static QString formatAuthor(const QString &, CitationFormat);

    QString _leadAuthor;
    QStringList _authors;
    QString _title;
    QString _journal;
    unsigned _year;
    QString _abbrevJ;
    int _vol;
    int _issue;
    int _startP;
    int _endP;
  };
  
  void setupCitations();

  QMap<QString,CitationRecord> _citations;
  QButtonGroup _checkboxes;
  QPlainTextEdit *_citationDisplay;

private slots:
  void updateCitationDisplay(bool);

};

#endif

