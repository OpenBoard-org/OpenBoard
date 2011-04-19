TEMPLATE  = lib
CONFIG   += staticlib release

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
OBJECTS_DIR  = "objs"

macx {
    DEFINES += "FT_MACINTOSH"

    CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = "10.4"
}

INCLUDEPATH += "include"

DEFINES = "FT2_BUILD_LIBRARY"

FREETYPE_DIR   = "$$[UNIBOARD_DIR]/thirdparty/freetype/2.3.8"
target.path    = "$$FREETYPE_DIR/lib"
ft2build.path  = "$$FREETYPE_DIR/include"
ft2build.files = "include/ft2build.h"
headers.path   = "$$FREETYPE_DIR/include/freetype"
headers.files  = "include/freetype/config" "include/freetype/*.h"

INSTALLS += "target" "ft2build" "headers"

SOURCES = src/autofit/autofit.c \
          src/base/ftbase.c \
          src/base/ftbitmap.c \
          src/base/ftglyph.c \
          src/base/ftinit.c \
          src/base/ftsystem.c \
          src/base/fttype1.c \
          src/bdf/bdf.c \
          src/cache/ftcache.c \
          src/cff/cff.c \
          src/cid/type1cid.c \
          src/gxvalid/gxvalid.c \
          src/gzip/ftgzip.c \
          src/lzw/ftlzw.c \
          src/otvalid/otvalid.c \
          src/pcf/pcf.c \
          src/pfr/pfr.c \
          src/psaux/psaux.c \
          src/pshinter/pshinter.c \
          src/psnames/psnames.c \
          src/raster/raster.c \
          src/sfnt/sfnt.c \
          src/smooth/smooth.c \
          src/truetype/truetype.c \
          src/type1/type1.c \
          src/type42/type42.c \
          src/winfonts/winfnt.c
