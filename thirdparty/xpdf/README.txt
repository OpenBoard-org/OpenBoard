How to build xpdf
=================

instructions for version 3.02

- Make sure freetype2 is installed in Uniboard/thirdparty/freetype
- Decompress xpdf-3.02.xx.tar.gz

Windows:
    - Copy aconf-win32.h to aconf.h
    - undefine unicode API support by adding '#undef UNICODE' aconf.h

OSX:
    - run ./configure (may need extrea options ?)

Linux:
    - run ./configure --with-freetype2-library=/usr/lib --with-freetype2-includes=/usr/include/freetype2

- Run qmake (-spec macx-g++ on OSX)
- Run make (nmake on windows)
