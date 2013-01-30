#ifndef SELECTIONRANGE_H
#define SELECTIONRANGE_H

class SelectionRange
{
public:
  SelectionRange();
  SelectionRange(int, int, int, int);
  int top() const;
  int left() const;
  int bottom() const;
  int right() const;
  int & top();
  int & left();
  int & bottom();
  int & right();
  void resetAll();
private:
  int _top;
  int _left;
  int _bottom;
  int _right;
};


#endif


