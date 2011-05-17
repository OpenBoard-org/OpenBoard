TEMPLATE  = lib
CONFIG   += staticlib release 

XPDF_DIR = "xpdf-3.02"

linux-g++ {
     SUB_LIB = "linux"
}

macx {
    SUB_LIB = "macx"
}

win32 {
    SUB_LIB = "win32"
}

DESTDIR = "lib/$$SUB_LIB"

OBJECTS_DIR  = "objects"

macx {
    CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = "10.5"
}

# Free type includes and lib
FREETYPE_DIR = "../freetype/freetype-2.4.4"
INCLUDEPATH += "$$FREETYPE_DIR/include" "$$FREETYPE_DIR/include/freetype2"
LIBS        += "-L../freetype/lib/$$SUB_LIB" "-lfreetype"

INCLUDEPATH += $$XPDF_DIR "$$XPDF_DIR/fofi" "$$XPDF_DIR/goo" "$$XPDF_DIR/splash" "$$XPDF_DIR/include" "xpdf-3.02Extra"

aconf.path    = $$XPDF_DIR
aconf.files = $$XPDF_DIR/aconf.h
headers.path  = "$$XPDF_DIR/xpdf"
headers.files = $$XPDF_DIR/goo/gmem.h \
                $$XPDF_DIR/goo/gtypes.h \
                $$XPDF_DIR/goo/GMutex.h \
                $$XPDF_DIR/goo/GString.h \
                $$XPDF_DIR/splash/SplashTypes.h \
                $$XPDF_DIR/splash/SplashBitmap.h \
                $$XPDF_DIR/xpdf/config.h \
                $$XPDF_DIR/xpdf/Array.h \
                $$XPDF_DIR/xpdf/CharTypes.h \
                $$XPDF_DIR/xpdf/Dict.h \
                $$XPDF_DIR/xpdf/Function.h \
                $$XPDF_DIR/xpdf/GfxState.h \
                $$XPDF_DIR/xpdf/GlobalParams.h \
                $$XPDF_DIR/xpdf/Object.h \
                $$XPDF_DIR/xpdf/OutputDev.h \
                $$XPDF_DIR/xpdf/PDFDoc.h \
                $$XPDF_DIR/xpdf/SplashOutputDev.h \
                $$XPDF_DIR/xpdf/Catalog.h \
                $$XPDF_DIR/xpdf/Page.h \
                $$XPDF_DIR/xpdf/Stream.h \
                $$XPDF_DIR/xpdf/XRef.h

#INSTALLS += "aconf" "headers"
INSTALLS += "headers"

SOURCES += $$XPDF_DIR/fofi/FoFiBase.cc \
           $$XPDF_DIR/fofi/FoFiEncodings.cc \
           $$XPDF_DIR/fofi/FoFiTrueType.cc \
           $$XPDF_DIR/fofi/FoFiType1.cc \
           $$XPDF_DIR/fofi/FoFiType1C.cc \
           $$XPDF_DIR/goo/gfile.cc \
           $$XPDF_DIR/goo/GHash.cc \
           $$XPDF_DIR/goo/GList.cc \
           $$XPDF_DIR/goo/gmem.cc \
           $$XPDF_DIR/goo/GString.cc \
           $$XPDF_DIR/splash/Splash.cc \
           $$XPDF_DIR/splash/SplashBitmap.cc \
           $$XPDF_DIR/splash/SplashClip.cc \
           $$XPDF_DIR/splash/SplashFTFont.cc \
           $$XPDF_DIR/splash/SplashFTFontEngine.cc \
           $$XPDF_DIR/splash/SplashFTFontFile.cc \
           $$XPDF_DIR/splash/SplashFont.cc \
           $$XPDF_DIR/splash/SplashFontEngine.cc \
           $$XPDF_DIR/splash/SplashFontFile.cc \
           $$XPDF_DIR/splash/SplashFontFileID.cc \
           $$XPDF_DIR/splash/SplashPath.cc \
           $$XPDF_DIR/splash/SplashPattern.cc \
           $$XPDF_DIR/splash/SplashScreen.cc \
           $$XPDF_DIR/splash/SplashState.cc \
           $$XPDF_DIR/splash/SplashXPath.cc \
           $$XPDF_DIR/splash/SplashXPathScanner.cc \
           $$XPDF_DIR/xpdf/Annot.cc \
           $$XPDF_DIR/xpdf/Array.cc \
           $$XPDF_DIR/xpdf/BuiltinFont.cc \
           $$XPDF_DIR/xpdf/BuiltinFontTables.cc \
           $$XPDF_DIR/xpdf/Catalog.cc \
           $$XPDF_DIR/xpdf/CharCodeToUnicode.cc \
           $$XPDF_DIR/xpdf/CMap.cc \
           $$XPDF_DIR/xpdf/CoreOutputDev.cc \
           $$XPDF_DIR/xpdf/Decrypt.cc \
           $$XPDF_DIR/xpdf/Dict.cc \
           $$XPDF_DIR/xpdf/Error.cc \
           $$XPDF_DIR/xpdf/FontEncodingTables.cc \
           $$XPDF_DIR/xpdf/Function.cc \
           $$XPDF_DIR/xpdf/Gfx.cc \
           $$XPDF_DIR/xpdf/GfxFont.cc \
           $$XPDF_DIR/xpdf/GfxState.cc \
           $$XPDF_DIR/xpdf/GlobalParams.cc \
           $$XPDF_DIR/xpdf/ImageOutputDev.cc \
           $$XPDF_DIR/xpdf/JArithmeticDecoder.cc \
           $$XPDF_DIR/xpdf/JBIG2Stream.cc \
           $$XPDF_DIR/xpdf/JPXStream.cc \
           $$XPDF_DIR/xpdf/Lexer.cc \
           $$XPDF_DIR/xpdf/Link.cc \
           $$XPDF_DIR/xpdf/NameToCharCode.cc \
           $$XPDF_DIR/xpdf/Object.cc \
           $$XPDF_DIR/xpdf/Outline.cc \
           $$XPDF_DIR/xpdf/OutputDev.cc \
           $$XPDF_DIR/xpdf/Page.cc \
           $$XPDF_DIR/xpdf/Parser.cc \
           $$XPDF_DIR/xpdf/PDFCore.cc \
           $$XPDF_DIR/xpdf/PDFDoc.cc \
           $$XPDF_DIR/xpdf/PDFDocEncoding.cc \
           $$XPDF_DIR/xpdf/PreScanOutputDev.cc \
           $$XPDF_DIR/xpdf/PSOutputDev.cc \
           $$XPDF_DIR/xpdf/PSTokenizer.cc \
           $$XPDF_DIR/xpdf/SecurityHandler.cc \
           $$XPDF_DIR/xpdf/SplashOutputDev.cc \
           $$XPDF_DIR/xpdf/Stream.cc \
           $$XPDF_DIR/xpdf/TextOutputDev.cc \
           $$XPDF_DIR/xpdf/UnicodeMap.cc \
           $$XPDF_DIR/xpdf/UnicodeTypeTable.cc \
           $$XPDF_DIR/xpdf/XRef.cc
