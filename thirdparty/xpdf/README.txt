How to build xpdf
=================

instructions for version 3.02

- Make sure freetype2 is installed in Uniboard/thirdparty/freetype
- Decompress xpdf-3.02.xx.tar.gz

Windows:
    - Copy aconf-win32.h (from 3.02/src) to 3.02/aconf.h
    - undefine unicode API support by adding '#undef UNICODE' to 3.02/aconf.h

OSX:
    - run ./configure (may need extrea options ?)
    - Copy resulting aconf.h (from 3.02/src) to 3.02/aconf.h

Linux (Ubuntu 9.10):
    - run ./configure --with-freetype2-library=/usr/lib --with-freetype2-includes=/usr/include/freetype2
    - Copy resulting aconf.h (from 3.02/src) to 3.02/aconf.h

- Run qmake (-spec macx-g++ on OSX)
- Run make (nmake on windows)
- Run make (nmake on windows) install
