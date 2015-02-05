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
    enum CitationFormat {Apa, BibTeX, EndNote, RIS, LastName};
    enum CitationType {Article, Book};

    CitationRecord(const QString &, const QString &, const QString &, unsigned, const QString & = 0, int = -1, int = -1, int = -1, int = -1, CitationType = Article, const QString & = "", const QString & = "", const QString & = "");
    
    void addAuthor(const QString &);
    void setAbbrevJ(const QString &);
    void setVol(unsigned);
    void setIssue(unsigned);
    void setChapter(unsigned);
    void setPages(unsigned, int);
    void setType(CitationType);
    void setPublisher(const QString &);
    void setLocation(const QString &);
    void addEditor(const QString &);
    
    const QString & leadAuthor() const { return _leadAuthor; };
    const QStringList & authors() const { return _authors; };
    const QString & title() const { return _title; };
    const QString & journal(bool abbrev = false) const { if (abbrev)  return _abbrevJ; else  return _journal; };
      //abbrev ? return _abbrevJ : return _journal; };
    unsigned year() const { return _year; };
    int vol() const { return _vol; };
    int issue() const { return _issue; };
    int chapter() const { return _issue; };
    int startPage() const { return _startP; };
    int endPage() const { return _endP; };
    CitationType type() const { return _type; };
    const QString & publisher() const { return _publisher; };
    const QString & location() const { return _location; };
    const QStringList & editors() const { return _editors; };

    QString formatCitation(CitationFormat) const;
    QString formatBookCitation(CitationFormat) const;

  private:
    static QString formatAuthor(const QString &, CitationFormat);
    static QString formatEditor(const QString &, CitationFormat);

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
    CitationType _type;
    QString _publisher;
    QString _location;
    QStringList _editors;
  };
  
  void setupCitations();

  QMap<QString,CitationRecord> _citations;
  QButtonGroup _checkboxes;
  QButtonGroup _radioButtons;
  QPlainTextEdit *_citationDisplay;
  CitationRecord::CitationFormat _currentFormat;

private slots:
  void updateCitationDisplay(bool);
  void updateCurrentFormat(bool);

};

#endif

