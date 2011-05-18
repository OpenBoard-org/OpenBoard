How to build freetype
=====================

instructions 

- Download freetype (http://www.freetype.org/download.html#stable)
- Decompress freetype.XXX.tar.gz

WIN
  - qmake
  - nmake

MAC
  - qmake freetype.pro -spec macx-g++
  - make

LINUX:
  - qmake freetype.pro -spec linux-g++
  - make 

