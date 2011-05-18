How to build xpdf
=================

instructions for version 3.02

- Make sure freetype2 is installed in Uniboard/thirdparty/freetype
- Decompress xpdf-3.02.xx.tar.gz

Windows:
	- using cywin run ./configure
	- run qmake xpdf.pro -r -spec win32-g++
	- run make

OSX:
    - cd xpdf-3.02
    - ./configure
    - cd ..
    - qmake xpdf.pro -spec macx-g++
    - make

Linux:
    - cd xpdf-3.02
    - ./configure --with-freetype2-library="../../freetype/lib/linux" --with-freetype2-includes="../../freetype/freetype-2.4.4/include"
    - cd ..
    - qmake xpdf.pdf -spec linux-g++
    - make:

- Run qmake (-spec macx-g++ on OSX)
- Run make (nmake on windows)
