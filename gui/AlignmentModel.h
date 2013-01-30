#ifndef ALIGNMENTMODEL_H
#define ALIGNMENTMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QModelIndex>
#include <QVariant>
#include <vector>
using namespace std;

#include "../seqio/Sequence.h"

class AlignmentModel : public QAbstractTableModel
{ 
  Q_OBJECT  
public:
  typedef enum DIRECTION
  {
    Right,
    Left
  } Direction;
  
  AlignmentModel(vector<Sequence*> &, QObject * = 0);
  virtual ~AlignmentModel() {};
  virtual int rowCount(const QModelIndex & = QModelIndex() ) const; 
  virtual int columnCount(const QModelIndex & = QModelIndex() ) const; 
  virtual QVariant data(const QModelIndex &, int = Qt::DisplayRole) const;
  virtual QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;
  virtual bool removeColumns(int, int, const QModelIndex & = QModelIndex());
  virtual bool removeRows(int, int, const QModelIndex & = QModelIndex());
  virtual bool insertColumns(int, int, const QModelIndex & = QModelIndex());
  virtual bool insertColumns(int, QString&, const QModelIndex & = QModelIndex());
  
  void maskColumns(int, int);
  void maskRows(int, int);
  
  bool addSequence(int, QString &);
  bool addSequence(int, Sequence *);
  
  bool regionContainsNonGaps(int, int, int, int);
  bool deleteRange(int, int, int, int, const QModelIndex & = QModelIndex());
  bool deleteCharacter(int, int, const QModelIndex & = QModelIndex());
  bool insertCharacter(char, int, int, const QModelIndex & = QModelIndex());
  bool insertCharacters(int, int, int, int, const QModelIndex & = QModelIndex());
  bool insertCharacters(int, int, int, QString &, const QModelIndex & = QModelIndex());
  
  bool fetchCharacters(int, int, int, Direction);
  bool moveCharacters(int, int, int, int, int);
  bool pushCharacters(int, int, int, int, Direction);
  
  void toggleTranslation(int, int, bool);
  void translateAgain(int, int);
  int locateEndRev(int aastart, int readingFrame);
  const vector<Sequence *> & alignment() const;
  void setAlignment(vector<Sequence *> &);
  
  void setCharType(Sequence::CharType);
  Sequence::CharType charType() const;
  
  vector<bool> mask() const;
  void setMask(vector<bool> &);
  
private:
  int translationHelper(vector<Sequence *>*, vector<Sequence *>*, int, int);
  void maskTranslate(vector<bool>*,  vector<bool>*, int);
  
  vector<Sequence *> &_alignment;
  vector<Sequence *> _dnaAlignment;
  vector<bool> _goodSeqs;
  vector<bool> _mask;
  vector<bool> _dnaMask;
  bool _inTranslation;
  size_t _nchar;
  size_t _nseq;
  int _readingFrame;
  Sequence::CharType _chartype;
  
/*private slots:
  void moveSequence(int, int, int);*/
  
signals:
  void characterInserted(const QModelIndex &);
  void characterDeleted(const QModelIndex &);
  void fetchedTo(const QModelIndex &, const QModelIndex &);
  void pushedTo(const QModelIndex &, const QModelIndex &);
  void charTypeChanged(Sequence::CharType);
  void maskChanged(int, int);
  void rowMaskChanged(int, int);
  
};

#endif
