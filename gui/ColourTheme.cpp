#include "ColourTheme.h"

#include <vector>
using namespace std;

QVector<QColor> initTheme(QColor *array, int ncols)
{
  vector<QColor> tmp;
  tmp.assign(array, array + ncols);

  QVector<QColor> vect = QVector<QColor>::fromStdVector(tmp);

  return vect;
}

#define NCOLS 10

QColor greyarray[NCOLS] = {Qt::white, QColor(204, 204, 204), Qt::black, QColor(249, 249, 249), QColor(26, 26, 26),
                              QColor(242, 242, 242), QColor(179, 179, 179),QColor(51, 51, 51), QColor(230, 230, 230),
                              QColor(102, 102, 102)};
const QVector<QColor> ColourTheme::_greyscale = initTheme(greyarray, NCOLS);
    /*initTheme((QColor[NCOLS]){Qt::white, QColor(204, 204, 204), Qt::black, QColor(249, 249, 249), QColor(26, 26, 26),
                              QColor(242, 242, 242), QColor(179, 179, 179),QColor(51, 51, 51), QColor(230, 230, 230),
                              QColor(102, 102, 102)}, NCOLS);*/

QColor camoarray[NCOLS] = {QColor(80, 45, 22), QColor(51, 128, 0), QColor(77, 77, 77), QColor(188, 211, 95),
                              QColor(22, 80, 22), QColor(200, 190, 183), QColor(31, 36, 28), QColor(136, 170, 0),
                              QColor(172, 147, 147), QColor(83, 108, 83)};
const QVector<QColor> ColourTheme::_camo = initTheme(camoarray, NCOLS);
    /*initTheme((QColor[NCOLS]){QColor(80, 45, 22), QColor(51, 128, 0), QColor(77, 77, 77), QColor(188, 211, 95),
                              QColor(22, 80, 22), QColor(200, 190, 183), QColor(31, 36, 28), QColor(136, 170, 0),
                              QColor(172, 147, 147), QColor(83, 108, 83)}, NCOLS);*/

QColor pastellearray[NCOLS] = {QColor(175, 198, 233), QColor(170, 255, 170), QColor(229, 128, 255), QColor(255, 238, 170),
                              QColor(255, 128, 229), QColor(255, 170, 170), QColor(198, 175, 233), QColor(233, 175, 198),
                              QColor(230, 255, 180), QColor(100, 255, 255)};
const QVector<QColor> ColourTheme::_pastelle = initTheme(pastellearray, NCOLS);
    /*initTheme((QColor[NCOLS]){QColor(175, 198, 233), QColor(170, 255, 170), QColor(229, 128, 255), QColor(255, 238, 170),
                              QColor(255, 128, 229), QColor(255, 170, 170), QColor(198, 175, 233), QColor(233, 175, 198),
                              QColor(230, 255, 180), QColor(100, 255, 255)}, NCOLS);*/

QColor vibrantarray[NCOLS] = {QColor(255, 0, 0), QColor(55, 200, 55), QColor(102, 0, 128), QColor(255, 204, 0),
                              QColor(255, 0, 204), QColor(170, 0, 0), QColor(85, 0, 212), QColor(255, 0, 102),
                              QColor(215, 255, 42), QColor(0, 204, 255)};
const QVector<QColor> ColourTheme::_vibrant = initTheme(vibrantarray, NCOLS);
    /*initTheme((QColor[NCOLS]){QColor(255, 0, 0), QColor(55, 200, 55), QColor(102, 0, 128), QColor(255, 204, 0),
                              QColor(255, 0, 204), QColor(170, 0, 0), QColor(85, 0, 212), QColor(255, 0, 102),
                              QColor(215, 255, 42), QColor(0, 204, 255)}, NCOLS);*/

QColor springarray[NCOLS] =  {QColor(255, 85, 85), QColor(221, 255, 85), QColor(229, 128, 255), QColor(128, 179, 255),
                              QColor(255, 0, 204), QColor(0, 255, 102), QColor(198, 175, 233), QColor(255, 85, 153),
                              QColor(55, 200, 171), QColor(102, 255, 255)};
const QVector<QColor> ColourTheme::_spring = initTheme(springarray, NCOLS);
    /*initTheme((QColor[NCOLS]){QColor(255, 85, 85), QColor(221, 255, 85), QColor(229, 128, 255), QColor(128, 179, 255),
                              QColor(255, 0, 204), QColor(0, 255, 102), QColor(198, 175, 233), QColor(255, 85, 153),
                              QColor(55, 200, 171), QColor(102, 255, 255)}, NCOLS); */

QColor summerarray[NCOLS] = {QColor(0, 212, 0), QColor(255, 42, 127), QColor(0, 128, 102), QColor(255, 102, 0),
                              QColor(0, 102, 255), QColor(212, 0, 170), QColor(255, 85, 85), QColor(255, 204, 0),
                              QColor(171, 55, 200), QColor(192, 0, 0)};
const QVector<QColor> ColourTheme::_summer = initTheme(summerarray, NCOLS);
    /*initTheme((QColor[NCOLS]){QColor(0, 212, 0), QColor(255, 42, 127), QColor(0, 128, 102), QColor(255, 102, 0),
                              QColor(0, 102, 255), QColor(212, 0, 170), QColor(255, 85, 85), QColor(255, 204, 0),
                              QColor(171, 55, 200), QColor(192, 0, 0)}, NCOLS);*/

QColor autumnarray[NCOLS] = {QColor(51, 51, 51), QColor(128, 0, 0), QColor(255, 104, 0), Qt::red, QColor(255, 102, 0),
                              QColor(160, 190, 44), QColor(80, 45, 22), QColor(22, 80, 22), QColor(200, 55, 55),
                              QColor(200, 171, 55)};
const QVector<QColor> ColourTheme::_autumn = initTheme(autumnarray, NCOLS);
    /*initTheme((QColor[NCOLS]){QColor(51, 51, 51), QColor(128, 0, 0), QColor(255, 104, 0), Qt::red, QColor(255, 102, 0),
                              QColor(160, 190, 44), QColor(80, 45, 22), QColor(22, 80, 22), QColor(200, 55, 55),
                              QColor(200, 171, 55)}, NCOLS);*/

QColor winterarray[NCOLS] = {QColor(128, 0, 0), QColor(0, 85, 68), QColor(51, 51, 51), QColor(198, 175, 233),
                              QColor(22, 80, 22), Qt::black, QColor(102, 0, 128), Qt::white, QColor(51, 0, 128),
                              QColor(100, 255, 255)};
const QVector<QColor> ColourTheme::_winter = initTheme(winterarray, NCOLS);
    /*initTheme((QColor[NCOLS]){QColor(128, 0, 0), QColor(0, 85, 68), QColor(51, 51, 51), QColor(198, 175, 233),
                              QColor(22, 80, 22), Qt::black, QColor(102, 0, 128), Qt::white, QColor(51, 0, 128),
                              QColor(100, 255, 255)}, NCOLS);*/

