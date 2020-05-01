
In order to compile a version of openboard which runs in it's own
window instead of fullscreen, you should set the environment
variable OB_INAWINDOW to yes before running qmake.

For instance:
env OB_INAWINDOW=yes qmake-qt5 OpenBoard.pro -spec linux-g++-64

spd@daphne.cps.unizar.es
https://webdiis.unizar.es/~spd/openboard


