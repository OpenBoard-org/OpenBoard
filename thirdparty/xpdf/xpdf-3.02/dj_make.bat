set CC=gcc
set CFLAGS=-g -O2 -I.. -I..\fofi -I..\goo
set CXX=gpp
set CXXFLAGS=%CFLAGS%
set LIBPROG=ar

copy aconf-dj.h aconf.h

cd goo
%CXX% %CXXFLAGS% -c GHash.cc
%CXX% %CXXFLAGS% -c GList.cc
%CXX% %CXXFLAGS% -c GString.cc
%CXX% %CXXFLAGS% -c gmem.cc
%CXX% %CXXFLAGS% -c gmempp.cc
%CXX% %CXXFLAGS% -c gfile.cc
%CC% %CFLAGS% -c parseargs.c
del libGoo.a
%LIBPROG% -rc libGoo.a GHash.o GList.o GString.o gmempp.o gfile.o gmem.o parseargs.o

cd ..\fofi
%CXX% %CXXFLAGS% -c FoFiBase.cc
%CXX% %CXXFLAGS% -c FoFiEncodings.cc
%CXX% %CXXFLAGS% -c FoFiTrueType.cc
%CXX% %CXXFLAGS% -c FoFiType1.cc
%CXX% %CXXFLAGS% -c FoFiType1C.cc
%LIBPROG% -rc libfofi.a FoFiBase.o FoFiEncodings.o FoFiTrueType.o FoFiType1.o FoFiType1C.o

cd ..\xpdf
del *.o
%CXX% %CXXFLAGS% -c Annot.cc
%CXX% %CXXFLAGS% -c Array.cc
%CXX% %CXXFLAGS% -c BuiltinFont.cc
%CXX% %CXXFLAGS% -c BuiltinFontTables.cc
%CXX% %CXXFLAGS% -c CMap.cc
%CXX% %CXXFLAGS% -c Catalog.cc
%CXX% %CXXFLAGS% -c CharCodeToUnicode.cc
%CXX% %CXXFLAGS% -c Decrypt.cc
%CXX% %CXXFLAGS% -c Dict.cc
%CXX% %CXXFLAGS% -c Error.cc
%CXX% %CXXFLAGS% -c FontEncodingTables.cc
%CXX% %CXXFLAGS% -c Function.cc
%CXX% %CXXFLAGS% -c Gfx.cc
%CXX% %CXXFLAGS% -c GfxFont.cc
%CXX% %CXXFLAGS% -c GfxState.cc
%CXX% %CXXFLAGS% -c GlobalParams.cc
%CXX% %CXXFLAGS% -c ImageOutputDev.cc
%CXX% %CXXFLAGS% -c JArithmeticDecoder.cc
%CXX% %CXXFLAGS% -c JBIG2Stream.cc
%CXX% %CXXFLAGS% -c JPXStream.cc
%CXX% %CXXFLAGS% -c Lexer.cc
%CXX% %CXXFLAGS% -c Link.cc
%CXX% %CXXFLAGS% -c NameToCharCode.cc
%CXX% %CXXFLAGS% -c Object.cc
%CXX% %CXXFLAGS% -c Outline.cc
%CXX% %CXXFLAGS% -c OutputDev.cc
%CXX% %CXXFLAGS% -c PDFDoc.cc
%CXX% %CXXFLAGS% -c PDFDocEncoding.cc
%CXX% %CXXFLAGS% -c PSOutputDev.cc
%CXX% %CXXFLAGS% -c PSTokenizer.cc
%CXX% %CXXFLAGS% -c Page.cc
%CXX% %CXXFLAGS% -c Parser.cc
%CXX% %CXXFLAGS% -c SecurityHandler.cc
%CXX% %CXXFLAGS% -c Stream.cc
%CXX% %CXXFLAGS% -c TextOutputDev.cc
%CXX% %CXXFLAGS% -c UnicodeMap.cc
%CXX% %CXXFLAGS% -c UnicodeTypeTable.cc
%CXX% %CXXFLAGS% -c XRef.cc
del libxpdf.a
%LIBPROG% -rc libxpdf.a *.o

%CXX% %CXXFLAGS% -o pdftops.exe pdftops.cc libxpdf.a ..\fofi\libfofi.a ..\goo\libGoo.a

%CXX% %CXXFLAGS% -o pdftotext.exe pdftotext.cc libxpdf.a ..\fofi\libfofi.a ..\goo\libGoo.a

%CXX% %CXXFLAGS% -o pdfinfo.exe pdfinfo.cc libxpdf.a ..\fofi\libfofi.a ..\goo\libGoo.a

%CXX% %CXXFLAGS% -o pdffonts.exe pdffonts.cc libxpdf.a ..\fofi\libfofi.a ..\goo\libGoo.a

%CXX% %CXXFLAGS% -o pdfimages.exe pdfimages.cc libxpdf.a ..\fofi\libfofi.a ..\goo\libGoo.a

cd ..
