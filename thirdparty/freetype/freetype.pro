TEMPLATE  = lib
CONFIG   += staticlib release

FREETYPE_DIR_WITH_VERSION = freetype-2.4.4

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
    DEFINES += "FT_MACINTOSH"

    CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = "10.4"
}

INCLUDEPATH += "$$FREETYPE_DIR_WITH_VERSION/include"

DEFINES = "FT2_BUILD_LIBRARY"

target.path    = "lib"
ft2build.path  = "$$FREETYPE_DIR_WITH_VERSION/include"
ft2build.files = "$$FREETYPE_DIR_WITH_VERSION/include/ft2build.h"
headers.path   = "$$FREETYPE_DIR_WITH_VERSION/include/freetype"
headers.files  = "$$FREETYPE_DIR_WITH_VERSION/include/freetype/config" "$$FREETYPE_DIR_WITH_VERSION/include/freetype/*.h"

INSTALLS += "target" "ft2build" "headers"

SOURCES = $$FREETYPE_DIR_WITH_VERSION/src/autofit/autofit.c \
          $$FREETYPE_DIR_WITH_VERSION/src/base/ftbase.c \
          $$FREETYPE_DIR_WITH_VERSION/src/base/ftbitmap.c \
          $$FREETYPE_DIR_WITH_VERSION/src/base/ftglyph.c \
          $$FREETYPE_DIR_WITH_VERSION/src/base/ftinit.c \
          $$FREETYPE_DIR_WITH_VERSION/src/base/ftsystem.c \
          $$FREETYPE_DIR_WITH_VERSION/src/base/fttype1.c \
          $$FREETYPE_DIR_WITH_VERSION/src/bdf/bdf.c \
          $$FREETYPE_DIR_WITH_VERSION/src/cache/ftcache.c \
          $$FREETYPE_DIR_WITH_VERSION/src/cff/cff.c \
          $$FREETYPE_DIR_WITH_VERSION/src/cid/type1cid.c \
          $$FREETYPE_DIR_WITH_VERSION/src/gxvalid/gxvalid.c \
          $$FREETYPE_DIR_WITH_VERSION/src/gzip/ftgzip.c \
          $$FREETYPE_DIR_WITH_VERSION/src/lzw/ftlzw.c \
          $$FREETYPE_DIR_WITH_VERSION/src/otvalid/otvalid.c \
          $$FREETYPE_DIR_WITH_VERSION/src/pcf/pcf.c \
          $$FREETYPE_DIR_WITH_VERSION/src/pfr/pfr.c \
          $$FREETYPE_DIR_WITH_VERSION/src/psaux/psaux.c \
          $$FREETYPE_DIR_WITH_VERSION/src/pshinter/pshinter.c \
          $$FREETYPE_DIR_WITH_VERSION/src/psnames/psnames.c \
          $$FREETYPE_DIR_WITH_VERSION/src/raster/raster.c \
          $$FREETYPE_DIR_WITH_VERSION/src/sfnt/sfnt.c \
          $$FREETYPE_DIR_WITH_VERSION/src/smooth/smooth.c \
          $$FREETYPE_DIR_WITH_VERSION/src/truetype/truetype.c \
          $$FREETYPE_DIR_WITH_VERSION/src/type1/type1.c \
          $$FREETYPE_DIR_WITH_VERSION/src/type42/type42.c \
          $$FREETYPE_DIR_WITH_VERSION/src/winfonts/winfnt.c
