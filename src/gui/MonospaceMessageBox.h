#ifndef MONOSPACEMESSAGEBOX_H_
#define MONOSPACEMESSAGEBOX_H_


#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QString>
#include <QPlainTextEdit>
#include <QPushButton>

class MonospaceMessageBox : public QDialog
{
  Q_OBJECT
public:
  MonospaceMessageBox(QWidget * = 0, Qt::WindowFlags = 0);
  //MonospaceMessageBox(Icon icon, const QString & title, const QString & text, StandardButtons buttons = NoButton, QWidget * parent = 0, Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
  
  void setText(const QString &);
  QString text() const { return _text; };// const;
  
  void setInformativeText(const QString &);
  QString informativeText() const { return _informativeText; };
  
  void setDetailedText(const QString &);
  QString detailedText() const { return _detailedText; };
  
private:
  QString _text;
  QLabel *_textLabel;
  QString _informativeText;
  QLabel *_infTextLabel;
  QPlainTextEdit *_detailedTextArea;
  QFrame *_line;
  QString _detailedText;
  QPushButton *_detailedTextButton;
  QHBoxLayout *_buttonLayout;
  QVBoxLayout *_mainLayout;
  QHBoxLayout *_topLayout;
  QVBoxLayout *_outerLayout;

  
private slots:
  void toggleShowDetailedText(bool);
  
};

#endif
