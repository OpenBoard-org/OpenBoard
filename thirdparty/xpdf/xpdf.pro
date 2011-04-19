TEMPLATE  = lib
CONFIG   += staticlib release warn_off

XPDF_DIR = "3.02"

linux-g++ {
     SUB_LIB = "linux"
}

macx {   
    SUB_LIB = "macx"
}

win32 {
    SUB_LIB = "win32"
}

DESTDIR = "$$XPDF_DIR/lib/$$SUB_LIB"

OBJECTS_DIR  = "objects"

macx {
    CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = "10.4"
}

# Free type includes and lib
FREETYPE_DIR = "../freetype/2.3.8"
INCLUDEPATH += "$$FREETYPE_DIR/include" "$$FREETYPE_DIR/include/freetype2"
LIBS        += "-L$$FREETYPE_DIR/lib" "-lfreetype"

INCLUDEPATH += $$XPDF_DIR $$XPDF_DIR/src "$$XPDF_DIR/src/fofi" "$$XPDF_DIR/src/goo" "$$XPDF_DIR/src/splash" "$$XPDF_DIR/include"

aconf.path    = "$$XPDF_DIR/include"
aconf.files   = aconf2.h
headers.path  = "$$XPDF_DIR/include/xpdf"
headers.files = $$XPDF_DIR/src/goo/gmem.h \
                $$XPDF_DIR/src/goo/gtypes.h \
                $$XPDF_DIR/src/goo/GMutex.h \
                $$XPDF_DIR/src/goo/GString.h \
                $$XPDF_DIR/src/splash/SplashTypes.h \
                $$XPDF_DIR/src/splash/SplashBitmap.h \
                $$XPDF_DIR/src/xpdf/config.h \
                $$XPDF_DIR/src/xpdf/Array.h \
                $$XPDF_DIR/src/xpdf/CharTypes.h \
                $$XPDF_DIR/src/xpdf/Dict.h \
                $$XPDF_DIR/src/xpdf/Function.h \
                $$XPDF_DIR/src/xpdf/GfxState.h \
                $$XPDF_DIR/src/xpdf/GlobalParams.h \
                $$XPDF_DIR/src/xpdf/Object.h \
                $$XPDF_DIR/src/xpdf/OutputDev.h \
                $$XPDF_DIR/src/xpdf/PDFDoc.h \
                $$XPDF_DIR/src/xpdf/SplashOutputDev.h \
                $$XPDF_DIR/src/xpdf/Catalog.h \
                $$XPDF_DIR/src/xpdf/Page.h \
                $$XPDF_DIR/src/xpdf/Stream.h \
                $$XPDF_DIR/src/xpdf/XRef.h

INSTALLS += "aconf" "headers"

SOURCES += $$XPDF_DIR/src/fofi/FoFiBase.cc \
           $$XPDF_DIR/src/fofi/FoFiEncodings.cc \
           $$XPDF_DIR/src/fofi/FoFiTrueType.cc \
           $$XPDF_DIR/src/fofi/FoFiType1.cc \
           $$XPDF_DIR/src/fofi/FoFiType1C.cc \
           $$XPDF_DIR/src/fofi/FoFiIdentifier.cc \
           $$XPDF_DIR/src/goo/gfile.cc \
           $$XPDF_DIR/src/goo/GHash.cc \
           $$XPDF_DIR/src/goo/GList.cc \
           $$XPDF_DIR/src/goo/gmem.cc \
           $$XPDF_DIR/src/goo/GString.cc \
           $$XPDF_DIR/src/splash/Splash.cc \
           $$XPDF_DIR/src/splash/SplashBitmap.cc \
           $$XPDF_DIR/src/splash/SplashClip.cc \
           $$XPDF_DIR/src/splash/SplashFTFont.cc \
           $$XPDF_DIR/src/splash/SplashFTFontEngine.cc \
           $$XPDF_DIR/src/splash/SplashFTFontFile.cc \
           $$XPDF_DIR/src/splash/SplashFont.cc \
           $$XPDF_DIR/src/splash/SplashFontEngine.cc \
           $$XPDF_DIR/src/splash/SplashFontFile.cc \
           $$XPDF_DIR/src/splash/SplashFontFileID.cc \
           $$XPDF_DIR/src/splash/SplashPath.cc \
           $$XPDF_DIR/src/splash/SplashPattern.cc \
           $$XPDF_DIR/src/splash/SplashScreen.cc \
           $$XPDF_DIR/src/splash/SplashState.cc \
           $$XPDF_DIR/src/splash/SplashXPath.cc \
           $$XPDF_DIR/src/splash/SplashXPathScanner.cc \
           $$XPDF_DIR/src/xpdf/Annot.cc \
           $$XPDF_DIR/src/xpdf/Array.cc \
           $$XPDF_DIR/src/xpdf/BuiltinFont.cc \
           $$XPDF_DIR/src/xpdf/BuiltinFontTables.cc \
           $$XPDF_DIR/src/xpdf/Catalog.cc \
           $$XPDF_DIR/src/xpdf/CharCodeToUnicode.cc \
           $$XPDF_DIR/src/xpdf/CMap.cc \
           $$XPDF_DIR/src/xpdf/CoreOutputDev.cc \
           $$XPDF_DIR/src/xpdf/Decrypt.cc \
           $$XPDF_DIR/src/xpdf/Dict.cc \
           $$XPDF_DIR/src/xpdf/Error.cc \
           $$XPDF_DIR/src/xpdf/FontEncodingTables.cc \
           $$XPDF_DIR/src/xpdf/Function.cc \
           $$XPDF_DIR/src/xpdf/Gfx.cc \
           $$XPDF_DIR/src/xpdf/GfxFont.cc \
           $$XPDF_DIR/src/xpdf/GfxState.cc \
           $$XPDF_DIR/src/xpdf/GlobalParams.cc \
           $$XPDF_DIR/src/xpdf/ImageOutputDev.cc \
           $$XPDF_DIR/src/xpdf/JArithmeticDecoder.cc \
           $$XPDF_DIR/src/xpdf/JBIG2Stream.cc \
           $$XPDF_DIR/src/xpdf/JPXStream.cc \
           $$XPDF_DIR/src/xpdf/Lexer.cc \
           $$XPDF_DIR/src/xpdf/Link.cc \
           $$XPDF_DIR/src/xpdf/NameToCharCode.cc \
           $$XPDF_DIR/src/xpdf/Object.cc \
           $$XPDF_DIR/src/xpdf/OptionalContent.cc \
           $$XPDF_DIR/src/xpdf/Outline.cc \
           $$XPDF_DIR/src/xpdf/OutputDev.cc \
           $$XPDF_DIR/src/xpdf/Page.cc \
           $$XPDF_DIR/src/xpdf/Parser.cc \
           $$XPDF_DIR/src/xpdf/PDFCore.cc \
           $$XPDF_DIR/src/xpdf/PDFDoc.cc \
           $$XPDF_DIR/src/xpdf/PDFDocEncoding.cc \
           $$XPDF_DIR/src/xpdf/PreScanOutputDev.cc \
           $$XPDF_DIR/src/xpdf/PSOutputDev.cc \
           $$XPDF_DIR/src/xpdf/PSTokenizer.cc \
           $$XPDF_DIR/src/xpdf/SecurityHandler.cc \
           $$XPDF_DIR/src/xpdf/SplashOutputDev.cc \
           $$XPDF_DIR/src/xpdf/Stream.cc \
           $$XPDF_DIR/src/xpdf/TextOutputDev.cc \
           $$XPDF_DIR/src/xpdf/UnicodeMap.cc \
           $$XPDF_DIR/src/xpdf/UnicodeTypeTable.cc \
           $$XPDF_DIR/src/xpdf/XRef.cc
