#include "SelectionRange.h"

SelectionRange::SelectionRange()
{
  resetAll();
}

SelectionRange::SelectionRange(int top, int left, int bottom, int right)
{
  _top = top;
  _left = left;
  _bottom = bottom;
  _right = right;
}

int SelectionRange::top() const
{
  return _top;
}

int SelectionRange::left() const
{
  return _left;
}

int SelectionRange::bottom() const
{
  return _bottom;
}

int SelectionRange::right() const
{
  return _right;
}

int & SelectionRange::top()
{
  return _top;
}

int & SelectionRange::left()
{
  return _left;
}

int & SelectionRange::bottom()
{
  return _bottom;
}

int & SelectionRange::right()
{
  return _right;
}

void SelectionRange::resetAll()
{
    _top = _left = _bottom = _right = -1;
}

