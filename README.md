# PopART: Population Analysis with Reticulate Trees

General purpose population genetics software developed thanks to funding from the Allan Wilson Centre Imaging Evolution Initiative.

Some documentation is available from http://popart.otago.ac.nz

Comments and questions should be directed to Jessica Leigh: jessica.w.leigh@gmail.com

PopART is provided with no promise of support. It was developed while I was a postdoc, and I currently have very little time for maintenance, let alone support. Pull requests are welcome, though!


## Compilation

PopART can be built on most (if not all) up-to-date Linux systems. Requirements:

1. Qt5: https://www.qt.io/
2. Marble: https://marble.kde.org/
3. lp_solve: http://lpsolve.sourceforge.net/5.5/index.htm

You'll need development headers for all three. On Ubuntu (tested on 18.04) this means running:
apt-get install libmarble-dev libqt5-dev liblpsolve55-dev

Then you should be able to run:
qmake && make

On distributions other than Ubuntu, you might need to edit popart.pro (the qmake configuration file).

## Prebuilt binaries

Outdated binaries are available from http://popart.otago.ac.nz.
