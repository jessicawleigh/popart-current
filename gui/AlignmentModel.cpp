#include <QString>
#include <QTableWidgetItem>
#include <iostream>

#include "AlignmentModel.h"
#include "../seqio/GeneticCode.h"
#include "../seqio/SequenceError.h"


AlignmentModel::AlignmentModel(vector<Sequence *> & alignment, QObject *parent)
 : QAbstractTableModel(parent), _alignment(alignment)
{
  
  _nseq = _alignment.size();
  _nchar = 0;
  _inTranslation = false;
  _readingFrame = 1;
  
  _goodSeqs.resize(alignment.size(), true);
  
  for (unsigned i = 0; i < _nseq; i++)
  {
    if  (_alignment[i]->length() > _nchar)
      _nchar = _alignment[i]->length();
      
  }
  
  _nchar++;
  
  for (int i = 0; i < _nchar; i++)
    _mask.push_back(true);

}

int AlignmentModel::rowCount(const QModelIndex &parent) const
{
  return _nseq;
}

int AlignmentModel::columnCount(const QModelIndex &parent) const
{
  return _nchar;
}

QVariant AlignmentModel::data(const QModelIndex &index, int role) const
{
  if (index.row() >= _nseq)  return QVariant();
  
  if (index.column() >= _nchar)  return QVariant();
  
  char seqchar[2];
  seqchar[1] = '\0';  

  if (role == Qt::DisplayRole)
  { 
    if (index.column() >= _alignment.at(index.row())->length())
    {
      seqchar[0] = '~';
      return QString(seqchar);
    }
    
    else 
    {
      seqchar[0] = (*(_alignment.at(index.row())))[index.column()];
      return QString(seqchar);
    }
  }
  
  else if (role == Qt::UserRole)
  {
    if (index.column() >= _mask.size())
    {
      cerr << "Oops, accessing beyond the end of the mask, size=" << _mask.size() << endl;
      return QVariant(true);
    }
    
    if (index.row() >= _goodSeqs.size())
    {
      cerr << "Oops, accessing beyond the end of goodSeqs, size=" << _goodSeqs.size() << endl;
      return QVariant(true);
    }

    
    else
      return QVariant(_mask.at(index.column()) && _goodSeqs.at(index.row()));
  }
  
  else  return QVariant();
  
}

QVariant AlignmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
  {
    if  (section > _nchar)  return QVariant();
    else return QString("");
  }
  
  else  
  {
    if (section > _nseq)  return QVariant();
    else  return QString::fromStdString(_alignment.at(section)->name());
  }
}

bool AlignmentModel::removeRows(int row, int count, const QModelIndex & parent)
{
  int first = row;
  int last = row + count - 1;
  
  if (first < 0 || last > rowCount())  return false;
  
  if (last < first)  return false;
  
  beginRemoveRows(parent, first, last);
  

  vector<Sequence *>::iterator seqiter = _alignment.begin();
  seqiter += first;

  for (int i = first; i < last; i++)
  {
    delete (*seqiter);
    ++seqiter;
  }

  if (! _dnaAlignment.empty())
  {
    seqiter = _dnaAlignment.begin();

    seqiter += first;

    for (int i = first; i < last; i++)
    {
      delete (*seqiter);
      ++seqiter;
    }
  }

  _alignment.erase(_alignment.begin() + first, _alignment.begin() + last + 1);


  if (_inTranslation)
    _dnaAlignment.erase(_dnaAlignment.begin() + first, _dnaAlignment.begin() + last + 1);
  
  _nseq -= count;
  
  
  
  if (_nseq != _alignment.size())
    throw SequenceError("Sequence vector is the wrong size!");
    //cerr << "Strange: After row deletion, _seqvect apparently didn't adjust its size properly. New size: " << _alignment.size() << " Should be: " << _nseq <<  " and count: " << count <<  endl;

  endRemoveRows();
  
  int newnchar = 0;
  
  for (int i = 0; i < _nseq; i++)
    if (_alignment[i]->length() > newnchar)
      newnchar = _alignment[i]->length();
  
  newnchar++;
  
  if (newnchar < _nchar)
  {
    beginRemoveColumns(parent, newnchar, _nchar - 1);
    _nchar = newnchar;
    vector<bool>::iterator iter = _mask.begin();
    iter += newnchar;
    _mask.erase(iter);
    endRemoveColumns();
  }
  
  return true;
}

int AlignmentModel::locateEndRev(int aastart, int readingFrame)
{
  if (!_inTranslation)  return -1;
  
  int dnaseqlen = 0;
  
  vector<Sequence *>::const_iterator dnaiter = _dnaAlignment.begin();
  while (dnaiter != _dnaAlignment.end())
  {
    if ((*dnaiter)->length() > dnaseqlen)  dnaseqlen = (*dnaiter)->length();
    dnaiter++;
  }
  
  dnaseqlen--;

  return dnaseqlen - (aastart * 3) + readingFrame + 1;
  
}

bool AlignmentModel::removeColumns(int column, int count, const QModelIndex & parent)
{
  int first = column;
  int last = column + count - 1;
  int dnacolumn;
  int dnacount = count * 3;
  int thisStart, thisEnd, thisCount;
  // for reverse, seqlen should be seq.length() - 1
    
  if (_inTranslation)
  {
    if (_readingFrame > 0)
      dnacolumn = column * 3 + _readingFrame - 1;
    
    else
      dnacolumn = locateEndRev(column, _readingFrame);
  }
  
  if (last < first)  return false;
  if (first < 0 || last > columnCount())  
    return false;
  
  beginRemoveColumns(parent, first, last);
    
  vector<bool>::iterator maskiter = _mask.begin();
  maskiter += column;
  _mask.erase(maskiter, (maskiter + count));
  
  if (_inTranslation)
  {
    if (_readingFrame > 0)
    {
      vector<bool>::iterator dnamaskiter = _dnaMask.begin();
      dnamaskiter += dnacolumn;
      _dnaMask.erase(dnamaskiter, (dnamaskiter + dnacount));
    }

    else
    {
      vector<bool>::iterator dnamaskiter = _dnaMask.begin();
      if ((dnacolumn - dnacount) > 0)
        dnamaskiter += (dnacolumn - dnacount);
      _dnaMask.erase(dnamaskiter, (dnamaskiter + dnacount));
    }
  }
  
  for (unsigned i = 0; i < rowCount(); i++)
  {
    if (last >= _alignment[i]->length())
      _alignment[i]->delCharRange(column);
    else
      _alignment[i]->delCharRange(column, count);
    
    if (_inTranslation)
    {
      if (_readingFrame > 0)
      {
        if ((dnacolumn + dnacount) > _dnaAlignment[i]->length())
          _dnaAlignment[i]->delCharRange(dnacolumn);
        else
          _dnaAlignment[i]->delCharRange(dnacolumn, dnacount);
      }
      
      else
      {
        thisStart = dnacolumn - dnacount;
        thisEnd = dnacolumn;

        if (thisStart < 0)
          thisStart = 0;
        
        if (thisEnd > _dnaAlignment[i]->length())
          thisEnd = _dnaAlignment[i]->length();

        if (!(thisStart >= _dnaAlignment[i]->length()))
          _dnaAlignment[i]->delCharRange(thisStart, thisEnd - thisStart);
      }
    }
  }
  
  _nchar -= count;
  
  endRemoveColumns();
  return true;
}



bool AlignmentModel::addSequence(int row, QString & name)
{
  if (row > _nseq || row < 0)  return false;
  
  Sequence *s = new Sequence(name.toStdString(), string(_nchar, '-'));
  
  addSequence(row, s);
  return true;
}

bool AlignmentModel::addSequence(int row, Sequence * sequence)
{
  if (row > _nseq || row < 0)  return false;
  
  /*if (sequence.length() < _nchar)  
    sequence.pad(_nchar);*/
    
  /*else  if (sequence[sequence.length() - 1] != '-')
    sequence.pad(sequence.length() + 1);*/
  
  vector<Sequence *>::iterator seqiter = _alignment.begin();
  int idxcount = 0;
  bool inserted = false;
  
  beginInsertRows(QModelIndex(), row, row);
  
  if (row == _nseq)
  {
    if (sequence->length() > _nchar)
      insertColumns(_nchar, sequence->length() - _nchar);
      
    if (_nseq >= _alignment.size())  _alignment.push_back(sequence);
    else  _alignment.at(_nseq) = sequence;
    inserted = true;
  }
  
  else
  {
    while (seqiter != _alignment.end())
    {
      if (idxcount == row)
      {
        if (sequence->length() > _nchar)
          insertColumns(_nchar, sequence->length() - _nchar);
        
        _alignment.insert(seqiter, sequence);
        inserted = true;
        break;
      }
      idxcount ++;
      seqiter++;
    }
  }
  
  if (inserted)
  {
    _nseq++;
    _alignment.resize(_nseq);
    endInsertRows();
        
    return true;
  }
  
  else
  {
    endInsertRows();
    return false;
  }
  
  return false;
}

bool AlignmentModel::insertColumns(int column, int count, const QModelIndex & parent)
{
  int first = column;
  int last = column + count - 1;
  
  
  if (last < first)  return false;
  if (first < 0 || first > columnCount()) return false; //|| last > columnCount())  return false;
    
  int dnacolumn;
  int dnacount = count * 3;
  
  if (_inTranslation)
  {
    if (_readingFrame > 0)
      dnacolumn = column * 3 + _readingFrame - 1;
    
    else
      dnacolumn = locateEndRev(column, _readingFrame);
  }
  
  
  beginInsertColumns(parent, first, last);
  
  vector<bool>::iterator maskiter = _mask.begin();
  maskiter += column;
  bool val = true;
  _mask.insert(maskiter, count, val);
  
  if (_inTranslation)
  {
    maskiter = _dnaMask.begin();
    maskiter += dnacolumn;
    _dnaMask.insert(maskiter, dnacount, val);
  }
  
  for (int i = 0; i < _nseq; i++)  
  {
    _alignment[i]->insertGaps(column, count);
    
    if (_inTranslation)
    {
      _dnaAlignment[i]->insertGaps(dnacolumn, dnacount);
    }
  }
  
  _nchar += count;
  
  endInsertColumns();
  return true;
}

bool AlignmentModel::insertColumns(int column, QString &substr, const QModelIndex &parent)
{
  
  bool allgaps = true;
  for (int i = 0; i < substr.length(); i++)
    if (substr.at(i) != '-')  allgaps = false;
    
  if (allgaps)
    return insertColumns(column, substr.length(), parent);
  
  int first = column;
  int last = column + substr.length() - 1;
  
  if (last < first)  return false;
  if (first < 0 || last > columnCount())  return false;
  
  
  if (_inTranslation)
  {
    cerr << "Only gaps can be inserted in translation mode." << endl;
    return false;
  }

  beginInsertColumns(parent, first, last);
  
  for (int i = 0; i < _nseq; i++)  
    _alignment.at(i)->insertChars(column, substr.toStdString());
  
  vector<bool>::iterator maskiter = _mask.begin();
  maskiter += column;
  _mask.insert(maskiter, substr.length(), true);
  _nchar += substr.length();
  
  endInsertColumns();
  return true;
  
}

bool AlignmentModel::regionContainsNonGaps(int top, int left, int bottom, int right)
{
  
  // this is legal, but no nongaps selected
  if (top < 0 || left < 0)  return false; 
  
  if (bottom > _nseq || right >= _nchar)  
    throw SequenceError ("Attempt to access beyond the end of Alignment.");

  int seqRight;
  
  for (int i = top; i <= bottom; i++)
  {
    if (_alignment.at(i)->length() <= right)
      seqRight = _alignment.at(i)->length() - 1;
    else
      seqRight = right;
    
    for (int j = left; j <= seqRight; j++)
      if ((*(_alignment.at(i)))[j] != '-')  return true;
  }
  
  return false;
}

bool AlignmentModel::deleteRange(int top, int left, int bottom, int right, const QModelIndex &parent)
{
 
  if (top < 0 || left < 0)  return false;
  if (bottom >= _nseq || right >= _nchar)  return false;
        
  int dnaleft, dnaright;
  int dnacount = 3 * (right - left + 1);
  
  if (_inTranslation)
  {
    if (_readingFrame > 0)
      dnaleft = left * 3 + _readingFrame - 1;
      
    else
      dnaright = locateEndRev(left, _readingFrame);
  }
  
  if (left == 0 && right == (_nchar - 1))  
    removeRows(top, bottom - top + 1, parent);
    
  else if (top == 0 && bottom == (_nseq - 1))  
    removeColumns(left, right - left + 1, parent);
  
  else
  {
    for (int i = top; i <= bottom; i++)
    {
      if (_alignment.at(i)->length() <= right)
        _alignment.at(i)->delCharRange(left);
      else
        _alignment.at(i)->delCharRange(left, right - left + 1);
     // _alignment[i].pad(_nchar);
        
      if (_inTranslation)
      {
        if (_readingFrame > 0)
        {
          if (dnaleft + dnacount > _dnaAlignment.at(i)->length())
            _dnaAlignment.at(i)->delCharRange(dnaleft);
          else
            _dnaAlignment.at(i)->delCharRange(dnaleft, dnacount);
        }
        
        else
        {
          dnaleft = dnaright - dnacount;
          
          if (dnaleft < 0)  
            dnaleft = 0;
            
          if (dnaright > _dnaAlignment.at(i)->length())
            dnaright = _dnaAlignment.at(i)->length();
          
          if (!(dnaleft >= _dnaAlignment.at(i)->length()))
            _dnaAlignment.at(i)->delCharRange(dnaleft, dnaright - dnaleft);
        }
      }
    }
  }

  emit dataChanged(index(top, left, parent), index(bottom, _nchar - 1, parent));
  
  if (top == bottom && left == right)
    emit characterDeleted(index(top, left, parent));
  
  int newnchar = 0;
  for (int i = 0; i < _nseq; i++)
    if (_alignment.at(i)->length() > newnchar)
      newnchar = _alignment.at(i)->length();
  
  newnchar++;
  if (newnchar < _nchar)
  {
    beginRemoveColumns(parent, newnchar, _nchar - 1);
    _nchar = newnchar;
    
    vector<bool>::iterator iter = _mask.begin();
    iter += newnchar;
    _mask.erase(iter);
    
    endRemoveColumns();
  }
  
  return true;
}

bool AlignmentModel::deleteCharacter(int row, int col, const QModelIndex & parent)
{
  if (col < 0 || col > columnCount())  return false;
  if (row < 0 || row > rowCount())  return false;

  if (col < _alignment.at(row)->length())
  {
    _alignment.at(row)->delCharRange(col, 1);
    //_alignment[row].pad(_nchar);
  }
  
  if (_inTranslation)
  {
    int dnacolumn;
    int dnacount = 3;

    if (_readingFrame > 0)
    {
      dnacolumn = col * 3 + _readingFrame - 1;
      if (dnacolumn < _dnaAlignment.at(row)->length())
        _dnaAlignment.at(row)->delCharRange(dnacolumn, 3);
    }
    
    else
    {
      dnacolumn = locateEndRev(col, _readingFrame);
      if (dnacolumn <= _dnaAlignment.at(row)->length())
        _dnaAlignment.at(row)->delCharRange(dnacolumn - 3, 3);
      else if ((dnacolumn - 3) < _dnaAlignment.at(row)->length())
        _dnaAlignment.at(row)->delCharRange(dnacolumn - 3);
    }  
  }
   
  emit dataChanged(index(row, col, parent), index(row, _nchar - 1, parent));
  emit characterDeleted(index(row, col, parent)); 
  
  int newnchar = 0;
  for (int i = 0; i < _nseq; i++)
    if (_alignment.at(i)->length() > newnchar)
      newnchar = _alignment.at(i)->length();
  
  newnchar++;
  if (newnchar < _nchar)
  {
    beginRemoveColumns(parent, newnchar, _nchar - 1);
    _nchar = newnchar;
    
    vector<bool>::iterator iter = _mask.begin();
    iter += newnchar;
    _mask.erase(iter);
    
    endRemoveColumns();
  }

  return true;
}

bool AlignmentModel::insertCharacter(char c, int row, int col, const QModelIndex & parent)
{
  if (_inTranslation && c != '-')
  {
    cerr << "Only gaps can be inserted in translation mode." << endl;
    return false;
  }

  if (col < 0 || col > columnCount())  return false;
  if (row < 0 || row > rowCount())  return false;
  
  int dnacolumn;
  int dnacount = 3;
  
  if (_inTranslation)
  {
    if (_readingFrame > 0)
      dnacolumn = col * 3 + _readingFrame - 1;
    
    else
      dnacolumn = locateEndRev(col, _readingFrame);
  }
  
  char cstr[2] = {c, '\0'};
  
  bool colinserted = false;
  if (_alignment.at(row)->length() >= (_nchar - 1))
  {
    colinserted = true;
    beginInsertColumns(parent, _nchar, _nchar);
    _alignment.at(row)->insertChars(col, cstr);
    _nchar++;
    _mask.push_back(true);
    endInsertColumns();
  
  }
  
  else
    _alignment.at(row)->insertChars(col, cstr);
  

  /*for (int i = 0; i < rowCount(); i++)  
  {
    if (i == row)
    {
      _alignment[i].insertChars(col, cstr); 
      if (_inTranslation && dnacolumn < _dnaAlignment[i].length())
        _dnaAlignment[i].insertGaps(dnacolumn, dnacount);
    }
    
    else
      _alignment[i].pad(_nchar);
  } */ 
   
  QModelIndex currentIdx = index(row, col, parent);
  
  emit dataChanged(currentIdx, index(row, _nchar - 1, parent));
  emit characterInserted(currentIdx);
  
  return true;
  
}

bool AlignmentModel::insertCharacters(int column, int topRow, int bottomRow, int count, const QModelIndex &parent)
{
  
  if (column < 0 || column > columnCount())  return false;
  if (bottomRow < topRow || topRow < 0 || bottomRow > rowCount())  return false;

  if (topRow == 0 && bottomRow == (_nseq - 1))
    insertColumns(column, count, parent);
  else
  {
    
    int dnacolumn;
    int dnacount = 3 * count;

    if (_inTranslation)
    {
      if (_readingFrame > 0)
        dnacolumn = column * 3 + _readingFrame - 1;

      else
        dnacolumn = locateEndRev(column, _readingFrame);
    }
    
    int maxSeqLen = 0;
    for (int i = topRow; i <= bottomRow; i++)
      
      if (_alignment.at(i)->length() > maxSeqLen)
        maxSeqLen = _alignment.at(i)->length();
        
    int newcolcount = count - (_nchar - maxSeqLen) + 1;

    if (newcolcount > 0)
      beginInsertColumns(parent, _nchar, _nchar + newcolcount - 1);//count - 1);
    
    for (int i = topRow; i <= bottomRow; i++)
    {
      _alignment.at(i)->insertGaps(column, count);
      if (_inTranslation && dnacolumn < _dnaAlignment.at(i)->length())
        _dnaAlignment.at(i)->insertGaps(dnacolumn, dnacount);
    }
    /*for (int i = 0; i < _nseq; i++)
    {
      if (i >= topRow && i <= bottomRow)
      {
        _alignment[i].insertGaps(column, count);
        if (_inTranslation && dnacolumn < _dnaAlignment[i].length()) 
          _dnaAlignment[i].insertGaps(dnacolumn, dnacount);
      }
      
      else
        _alignment[i].pad(_nchar + count);
    } */
    
    if (newcolcount > 0)
    {
      _nchar += newcolcount;//count;
      int j;
      for (int i = 0; i < newcolcount; i++) //count; i++)
      {
        _mask.push_back(true);
        for (j = 0; j < 3; j++)
          _dnaMask.push_back(true);
      }
      endInsertColumns();
    }
    emit dataChanged(index(topRow, column, parent), index(bottomRow, _nchar -1, parent));
    
  }
  
  return true;
  
}

bool AlignmentModel::insertCharacters(int column, int topRow, int bottomRow, QString &substr, const QModelIndex &parent)
{

  if (column < 0 || column > columnCount())  return false;
  if (bottomRow < topRow || topRow < 0 || bottomRow > rowCount())  return false;
    
  if (_inTranslation)
  {
    bool allgaps = true;
    
    for (int i = 0; i < substr.length(); i++)
      if (substr.at(i) != '-')  allgaps = false;
      
    if (!allgaps)
    {
      cerr << "Only gaps can be inserted in translation mode." << endl;
      return false;
    }
    
    else
      return insertCharacters(column, topRow, bottomRow, substr.length(), parent);
    
  }

  if (topRow == 0 && bottomRow == (_nseq - 1))
    insertColumns(column, substr, parent);
  else
  {
    int ncols = substr.length();
    int maxSeqLen = 0;
    for (int i = topRow; i <= bottomRow; i++)
    {
      if (_alignment.at(i)->length() > maxSeqLen)
        maxSeqLen = _alignment.at(i)->length();
    }
    
    int newcolcount = ncols - (_nchar - maxSeqLen) + 1;
    
    if (newcolcount > 0)
      beginInsertColumns(parent, _nchar, _nchar + newcolcount - 1);
    
    for (int i = topRow; i <= bottomRow; i++)
      _alignment.at(i)->insertChars(column, substr.toStdString());

    if (newcolcount > 0)
    {
      _nchar += newcolcount; //ncols;
      for (int i = 0; i < newcolcount; i++)//ncols; i++)
        _mask.push_back(true);
        
      endInsertColumns();
    }
    emit dataChanged(index(topRow, column, parent), index(bottomRow, _nchar - 1, parent));
  }
  
  return true;
}

bool AlignmentModel::fetchCharacters(int column, int top, int bottom, AlignmentModel::Direction direction)
{
  int currentcol = column;
  bool colsremain = true;
  int i;
  int swapcol = -1;
  
  if (direction == Left)
  {
    currentcol--;
    colsremain = (currentcol > 0);
  }
  
  if (direction == Right)
  {
    currentcol++;
    colsremain = (currentcol < (_nchar - 2));
  }
  
  int startcol = currentcol;
  
  while (colsremain)
  {
    for (i = top; i <= bottom; i++)
    {
      if ((currentcol >= _alignment.at(i)->length()) ||
          (currentcol < 0))  
        colsremain = false;
      
      else if ((*(_alignment.at(i)))[currentcol] != '-')
      {   
         /* region not adjacent to a gap */
        if (currentcol == startcol)  return false; 
        else
        {
          swapcol = currentcol;
          colsremain = false;
        }
      }
    } // end for
    if (direction == Right)  currentcol++;
    else  currentcol--;
  } // end while
  
  if (swapcol >= 0)
  {
    for (i = top; i <= bottom; i++)
    {
      (*(_alignment.at(i)))[startcol] =  (*(_alignment.at(i)))[swapcol];
      (*(_alignment.at(i)))[swapcol] = '-';
    }
    
    if (direction == Right)
    {
      emit dataChanged(index(top, startcol), index(bottom, swapcol));
      emit fetchedTo(index(top, startcol), index(bottom, startcol));
      return true;
    }
    
    else
    {
      emit dataChanged(index(top, swapcol), index(bottom, startcol));
      emit fetchedTo(index(top, startcol), index(bottom, startcol));
      return true;
    }  
  }
  
  return false; // if never find a non-gap
}

bool AlignmentModel::moveCharacters(int top, int left, int bottom, int right, int distance)
{ 
  if (distance < 0 && (left + distance) < 0)  
    return false;
  
  if (distance > 0 && (right + distance) >= columnCount())
    return false;
  
  string leftSubseq, rightSubseq;
  int j;
  int lenSubseq;
  
  for (int i = top; i <= bottom; i++)
  {
    if (distance < 0)
    {
      if (right >= _alignment.at(i)->length())
      {
        rightSubseq = _alignment.at(i)->subseq(left);
        while (rightSubseq.length() < (right - left + 1)) 
          rightSubseq.push_back('-');
      }
      
      else
        rightSubseq = _alignment.at(i)->subseq(left, right - left + 1);
      
      leftSubseq = _alignment.at(i)->subseq(left + distance, -distance);
      
      lenSubseq = leftSubseq.length() + rightSubseq.length();
      
      if ((lenSubseq + left) > _alignment.at(i)->length())
        lenSubseq = _alignment.at(i)->length();
      
      _alignment.at(i)->replace(left + distance, lenSubseq, rightSubseq + leftSubseq);
      
    }
    
    else // distance is positive
    {
      if (right + distance >= _alignment.at(i)->length())
      {
        rightSubseq = _alignment.at(i)->subseq(right + 1);
        while (rightSubseq.length() < distance)
          rightSubseq.push_back('-');
      }
      
      else
        rightSubseq = _alignment.at(i)->subseq(right + 1, distance);
      
      leftSubseq = _alignment.at(i)->subseq(left, right - left + 1);
      
      lenSubseq = leftSubseq.length() + rightSubseq.length();
      
      if ((lenSubseq + left) > _alignment.at(i)->length())
        lenSubseq = _alignment.at(i)->length();
      
      _alignment.at(i)->replace(left, lenSubseq, rightSubseq + leftSubseq);
      
    }
  }
  
  if (distance < 0)
    emit dataChanged(index(top, left + distance), index(bottom, right));
  
  else
    emit dataChanged(index(top, left), index(bottom, right + distance));

  emit pushedTo(index(top, left + distance), index(bottom, right + distance));  
  
  return true;
}

bool AlignmentModel::pushCharacters(int top, int left, int bottom, int right, AlignmentModel::Direction direction)
{
  int startcol, endcol;
  
  if (direction == Right)
  {
    startcol = right + 1;
    endcol = startcol;
  }
  
  else
  {
    startcol = left - 1;
    endcol = startcol;
  }  
  
  bool endgapfound = false;
  int i;
  
  while (!endgapfound)
  {

    if (direction == Right)
      endcol++;
    else
      endcol--;
      
    for (i = top; i <= bottom; i++)
    {
      if (endcol >= (_alignment.at(i)->length() - 1))
        endgapfound = true;
      else
        if ((*(_alignment.at(i)))[endcol] != '-')
        {
          /* not adjacent to a gap */
          if ((endcol == (startcol - 1)) || (endcol == (startcol + 1)))
            return false;
          else
            endgapfound = true;
        }
    }
    if (endcol <= 0 || endcol >= (_nchar - 1))
      endgapfound = true;
    
  }
  
  if (direction == Right)
  {
    int ngaps = endcol - startcol;
    for (i = top; i <= bottom; i++)
    {
      _alignment.at(i)->delCharRange(startcol, ngaps);
      _alignment.at(i)->insertGaps(left, ngaps);
    }
    emit dataChanged(index(top, left), index(bottom, endcol));
    emit pushedTo(index(top, left + ngaps), index(bottom, right + ngaps));
    return true;
  }

  else
  {
    int ngaps = startcol - endcol;
    for (i = top; i <= bottom; i++)
    {
      _alignment.at(i)->insertGaps(right + 1, ngaps);
      _alignment.at(i)->delCharRange(endcol + 1, ngaps);
    }
    emit dataChanged(index(top, endcol), index(bottom, right));
    emit pushedTo(index(top, left - ngaps), index(bottom, right - ngaps));
    return true;
  }
  // Should never be reached
  return false;
}

void AlignmentModel::maskRows(int top, int bottom)
{
  if (top < 0 || bottom >= _nseq || top > bottom)
  {
    cerr << "Problem masking rows, ignoring." << endl;
    return;
  }
  
  for (int i = top; i <= bottom; i++)
    _goodSeqs.at(i) = false;
  
  emit rowMaskChanged(top, bottom);
  
}

void AlignmentModel::maskColumns(int left, int right)
{

  vector<bool>::reverse_iterator rdnaiter;
  vector<bool>::iterator dnaiter;
 
  if (_inTranslation)
  {    
    if (_readingFrame > 0)  
    {    
      dnaiter = _dnaMask.begin();
      dnaiter += (_readingFrame - 1) + (left * 3);
    }
    
    else
    {    
      rdnaiter = _dnaMask.rbegin();
      rdnaiter += (-_readingFrame - 1) + (left * 3);  
    }
  }

  int j;
  for (int i = left; i <= right; i++)
  {
    _mask.at(i) = !_mask.at(i);
    

    
    if (_inTranslation)
    {
      if (_readingFrame > 0)
      {
        for (j = 0; j < 3; j++)
        {
          if (dnaiter != _dnaMask.end())  
          {          
            (*dnaiter) = _mask.at(i);
            dnaiter++;
          }            
          else 
            _dnaMask.push_back(_mask.at(i));
        }
        
      }
      else // if readingFrame < 0
      {
        for (j = 0; j < 3; j++)
        {
          if (rdnaiter != _dnaMask.rend())
          {
            (*rdnaiter) = _mask.at(i);
            rdnaiter++;
          }
          
          else
          {
            dnaiter = _dnaMask.begin();
            _dnaMask.insert(dnaiter, _mask.at(i));
          }
            
        }
      }
    }
  }
 
  emit maskChanged(left, right);
}
  
void AlignmentModel::translateAgain(int readingFrame, int codeID)
{
  if (!_inTranslation)
  {    
    cerr << "Unexpected. Not in translation yet, but getting a signal to re-translate. Just returning." << endl;
    return;
  }

  vector<Sequence *> newAlignment(_nseq);
  int newnchar = translationHelper(&_dnaAlignment, &newAlignment, readingFrame, codeID);
  
  maskTranslate(&_dnaMask, &_mask, readingFrame);

  vector<Sequence *>::iterator seqiter = _alignment.begin();
  while (seqiter != _alignment.end())
  {
    delete (*seqiter);
    ++seqiter;
  }
    
  if (newnchar < _nchar)
  {
    beginRemoveColumns(QModelIndex(), newnchar, _nchar - 1);
    _nchar = newnchar;
    _alignment = newAlignment;
    endRemoveColumns();
  }
  
  else if (newnchar > _nchar)
  {
    beginInsertColumns(QModelIndex(), _nchar, newnchar - 1);
    _nchar = newnchar;
    _alignment = newAlignment;
    endInsertColumns();
  }
  
  else  
    _alignment = newAlignment;
  
  
  _readingFrame = readingFrame;
  emit dataChanged(index(0, 0, QModelIndex()), index(_nseq - 1, _nchar - 1, QModelIndex())); 
}

void AlignmentModel::maskTranslate(vector<bool> *dnaMask,  vector<bool> *proteinMask, int readingFrame)
{
  //vector<bool>::const_iterator codonstart = (*dnaMask.begin());
  bool unmasked;
  int protMaskSize = 0;
  int insertAt;
  int i;
 
  
  if (readingFrame > 0)
  {
    int newnchar = (dnaMask->size() - readingFrame  + 2) / 3 + 1;
    
    vector<bool>::iterator codonstart = dnaMask->begin();
    codonstart += (readingFrame - 1);

    while (codonstart != dnaMask->end())
    {
      unmasked = false;
      for (i = 0; i < 3; i++)
      {
        if (codonstart != dnaMask->end())
        {
          unmasked = (unmasked || *codonstart);
          codonstart++;          
        }
        else
          unmasked = true;
      }
      if (protMaskSize < proteinMask->size())
        proteinMask->at(protMaskSize) = unmasked;
      else
        proteinMask->push_back(unmasked);
      protMaskSize++;
    }    
  }
  
  else
  {
    vector<bool>::reverse_iterator codonstart = dnaMask->rbegin();
    codonstart += (-readingFrame - 1);
    
    while (codonstart != dnaMask->rend())
    {
      unmasked = false;
      for (i = 0; i < 3; i++)
      {
        if (codonstart != dnaMask->rend())
        {
          unmasked = (unmasked || *codonstart);
          codonstart++;
        }
        else
          unmasked = true;
      }
      if (protMaskSize < proteinMask->size())
        proteinMask->at(protMaskSize) = unmasked;
      else
        proteinMask->push_back(unmasked);
      protMaskSize++;
    } 
  }
  
  proteinMask->resize(protMaskSize);  
}

int AlignmentModel::translationHelper(vector<Sequence *> *dnaAlignment, vector<Sequence*>* proteinAlignment, int readingFrame, int codeID)
{
  
  int oldnchar = 0;
  
  for (int i = 0; i < _nseq; i++)
    if (oldnchar < dnaAlignment->at(i)->length())
      oldnchar = dnaAlignment->at(i)->length();
  
  int newnchar;
  if (readingFrame > 0)
    newnchar = (oldnchar - readingFrame  + 3) / 3 + 1;
  
  else
    newnchar = (oldnchar + readingFrame + 3) / 3 + 1;

  GeneticCode gc = GeneticCode::AlternateCode(codeID);
  
  for (int i = 0; i < _nseq; i++)
  {
    ///* remove extra gap char inserted */
    //(*dnaAlignment)[i].delChar(oldnchar - 1);
    if (readingFrame < 0)
      dnaAlignment->at(i)->pad(oldnchar);
    proteinAlignment->at(i) = new Sequence(*(dnaAlignment->at(i)), readingFrame, gc);
    
    /* add an extra gap again, to both. */
    //(*proteinAlignment)[i].pad(newnchar);
    //(*dnaAlignment)[i].pad(oldnchar);
  }
  
  return newnchar;
  
}

// TODO disable this function for binary sequence alignment
void AlignmentModel::toggleTranslation(int readingFrame, int codeID, bool toAA)
{
      
  if (_inTranslation == toAA) 
    cerr << "Unexpected. toAA and _inTranslation have the same value" << endl;

  _inTranslation = toAA;
  int newnchar;
  
  if (toAA)
  {
    if (_chartype == Sequence::AAType)
      cerr << "Unexpected. Toggling to translated (protein) mode, already in protein mode" << endl;
          
    vector<Sequence *> newAlignment(_nseq);
    
    newnchar = translationHelper(&_alignment, &newAlignment, readingFrame, codeID);
    
    
    vector<bool> newmask(newnchar);
    
    beginRemoveColumns(QModelIndex(), newnchar, _nchar - 1);

    maskTranslate(&_mask, &newmask, readingFrame);
    
    _dnaMask = _mask;
    _mask = newmask;

    _nchar = newnchar;
    _dnaAlignment = _alignment;
    _alignment = newAlignment;
    _readingFrame = readingFrame;
    
    _chartype = Sequence::AAType;
    
    
    if (_mask.size() > _nchar)
    {
      cerr << "mask is bigger than it should be: " << _mask.size() << " vs. " << _nchar << endl;
      _mask.resize(_nchar);
    }
    
    else 
    {
      while (_mask.size() < _nchar)
      {
        cerr << "mask too small. Appending unmasked values." << endl;
        _mask.push_back(true);
      }
    }
    /*vector<bool>::iterator iter = _mask.begin();
    iter += _nchar + 1;
    _mask.erase(iter);*/
    
    endRemoveColumns();
    
  }
  
  else
  {
    if (_chartype == Sequence::DNAType)
      cerr << "Unexpected. Toggling out of translated (protein) mode, still in DNA mode" << endl;
      
    int newnchar = 0;
    for (int i = 0; i < _nseq; i++)
    
      if (_dnaAlignment.at(i)->length() > newnchar)
        newnchar = _dnaAlignment.at(i)->length();
    
    // new code:
    newnchar++;
       
    beginInsertColumns(QModelIndex(), _nchar, newnchar - 1);
    
    vector<Sequence*>::iterator seqiter = _alignment.begin();
    while (seqiter != _alignment.end())
    {
      delete (*seqiter);
      ++seqiter;
    }

    _mask = _dnaMask;
    _alignment = _dnaAlignment;
    _nchar = newnchar;
    _chartype = Sequence::DNAType;
    
    endInsertColumns();
  }
  
  emit dataChanged(index(0, 0, QModelIndex()), index(_nseq - 1, _nchar - 1));
  emit charTypeChanged(_chartype);
  emit maskChanged(0, _nchar - 1);
}

void AlignmentModel::setCharType(Sequence::CharType chartype)
{
   _chartype = chartype;
  emit charTypeChanged(chartype);
}

Sequence::CharType AlignmentModel::charType() const
{
  return _chartype;
}


vector<bool> AlignmentModel::mask() const
{
  return _mask;
}

void AlignmentModel::setMask(vector<bool> & mask)
{
  _mask = mask;
  
  if (_mask.size() < _nchar)
    while (_mask.size() < _nchar)  _mask.push_back(true);
    
  else if (_mask.size() > _nchar)
    _mask.resize(_nchar);
  
  emit maskChanged(0, _nchar - 1);
}

const vector<Sequence *> & AlignmentModel::alignment() const
{
  return _alignment;  
}

void AlignmentModel::setAlignment(vector<Sequence *> & seqvect)
{  
  int newnchar = 0;
  int newnseq = 0;
  
  vector<Sequence *>::iterator seqiter = seqvect.begin();
  
  while (seqiter != seqvect.end())
  {
    if ((*seqiter)->length() > newnchar)  newnchar = (*seqiter)->length();
    seqiter++;
    newnseq++;
  }
  
  newnchar++;
  vector<bool>::iterator maskiter = _mask.begin();
  _mask.erase(maskiter);
  maskiter = _mask.begin();
  _mask.insert(maskiter, newnchar, true);
    
  seqiter = seqvect.begin();
  
  /*while (seqiter != seqvect.end())
  {
    (*seqiter).pad(newnchar);
    seqiter++;
  } */
  
  if (! _alignment.empty())
  {
    vector<Sequence*>::iterator seqiter = _alignment.begin();
    while (seqiter != _alignment.end())
    {
      delete (*seqiter);
      ++seqiter;
    }
  }



  if (newnchar > _nchar)
  {
    
    beginInsertColumns(QModelIndex(), _nchar, newnchar - 1);
    
    if (newnseq > _nseq)
    { 
      beginInsertRows(QModelIndex(), _nseq, newnseq - 1);
      _alignment = seqvect;
      _nseq = newnseq;
      endInsertRows();
    }

    else if (newnseq < _nseq)
    {    
      beginRemoveRows(QModelIndex(), newnseq, _nseq - 1);
      _alignment = seqvect;
      _nseq = newnseq;
      endRemoveRows();
    }
    
    else  _alignment = seqvect;
    
    _nchar = newnchar;
    endInsertColumns();
  }
  
  else if (newnchar < _nchar)
  {
    beginRemoveColumns(QModelIndex(), newnchar, _nchar - 1);

    if (newnseq > _nseq)
    {    
      beginInsertRows(QModelIndex(), _nseq, newnseq - 1);
      _alignment = seqvect;
      _nseq = newnseq;
      endInsertRows();
    }

    else if (newnseq < _nseq)
    {    
      beginRemoveRows(QModelIndex(), newnseq, _nseq - 1);
      _alignment = seqvect;      
      _nseq = newnseq;     
      endRemoveRows();
    }

    else  _alignment = seqvect;

    _nchar = newnchar;
    endRemoveColumns();
  }    

  else
  {
    if (newnseq > _nseq)
    {    
      beginInsertRows(QModelIndex(), _nseq, newnseq - 1);
      _alignment = seqvect;
      _nseq = newnseq;
      endInsertRows();
    }

    else if (newnseq < _nseq)
    {    
      beginRemoveRows(QModelIndex(), newnseq, _nseq - 1);
      _alignment = seqvect;
      _nseq = newnseq;
      endRemoveRows();
    }

    else  _alignment = seqvect;    
  }
    
  emit dataChanged(index(0, 0, QModelIndex()), index(_nseq - 1, _nchar - 1, QModelIndex()));

}

/*void AlignmentModel::moveSequence(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
  cout << "Sequence moved: " << logicalIndex << " " << oldVisualIndex << " " << newVisualIndex << endl;
  
  if (logicalIndex != oldVisualIndex) 
    cout << "logical: " << _alignment.at(logicalIndex).name() << " visual: " << _alignment.at(oldVisualIndex).name() << endl;
  
  cout << "size at start: " << _alignment.size() << endl;
 
  vector<Sequence>::iterator seqiter = _alignment.begin();
  seqiter += oldVisualIndex;
  Sequence seq = *seqiter;
  _alignment.erase(seqiter);
  
  cout << "size after removing:" << _alignment.size() << endl;
  cout << "name of moved seq:" << seq.name() << endl;
  
  //int newIndex = newVisualIndex;
  //if (newVisualIndex > oldVisualIndex)  newIndex--;
  
  seqiter = _alignment.begin();
  seqiter += newVisualIndex;
  
  _alignment.insert(seqiter, seq);
  cout << "size after re-inserting:" << _alignment.size() << endl;
  
  cout << "at insertion position: " << _alignment.at(newVisualIndex).name() << endl;
  
  
  seqiter = _alignment.begin();
  
  while (seqiter != _alignment.end())
  {
    cout << (*seqiter++).name() << endl;
  }
}*/

