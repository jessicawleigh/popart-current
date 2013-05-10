#ifndef COLOURTHEME_H_
#define COLOURTHEME_H_


#include <QColor>
#include <QVector>

class ColourTheme
{
public:
  typedef enum {Greyscale, Camo, Pastelle, Vibrant, Spring, Summer, Autumn, Winter}  Theme;

  static const QVector<QColor> & greyscale() { return _greyscale; };
  static const QVector<QColor> & camo() { return _camo; };
  static const QVector<QColor> & pastelle() { return _pastelle; };
  static const QVector<QColor> & vibrant() { return _vibrant; };
  static const QVector<QColor> & spring() { return _spring; };
  static const QVector<QColor> & summer() { return _summer; };
  static const QVector<QColor> & autumn() { return _autumn; };
  static const QVector<QColor> & winter() { return _winter; };

private:
  const static QVector<QColor> _greyscale;
  const static QVector<QColor> _camo;
  const static QVector<QColor> _pastelle;
  const static QVector<QColor> _vibrant;
  const static QVector<QColor> _spring;
  const static QVector<QColor> _summer;
  const static QVector<QColor> _autumn;
  const static QVector<QColor> _winter;

};


#endif
