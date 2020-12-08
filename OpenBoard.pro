TARGET = "OpenBoard"
TEMPLATE = app

CONFIG += c++14
CONFIG -= flat
CONFIG += debug_and_release \
          no_include_pwd


VERSION_MAJ = 1
VERSION_MIN = 6
VERSION_PATCH = 1
VERSION_TYPE = a # a = alpha, b = beta, rc = release candidate, r = release, other => error
VERSION_BUILD = 1116

VERSION = "$${VERSION_MAJ}.$${VERSION_MIN}.$${VERSION_PATCH}-$${VERSION_TYPE}.$${VERSION_BUILD}"

equals(VERSION_TYPE, r) {
    VERSION = "$${VERSION_MAJ}.$${VERSION_MIN}.$${VERSION_PATCH}"
}


LONG_VERSION = "$${VERSION}.$${SVN_VERSION}"
macx:OSX_VERSION = "$${VERSION} (r$${SVN_VERSION})"

VERSION_RC = $$VERSION_MAJ,$$VERSION_MIN,$$VERSION_PATCH,$$VERSION_TYPE,$$VERSION_BUILD
VERSION_RC = $$replace(VERSION_RC, "a", "160") # 0xA0
VERSION_RC = $$replace(VERSION_RC, "b", "176") # 0xB0
VERSION_RC = $$replace(VERSION_RC, "rc", "192" ) # 0xC0
VERSION_RC = $$replace(VERSION_RC, "r", "240") # 0xF0

QT += core \
      widgets \
      svg \
      multimediawidgets \
      xml \
      script

INCLUDEPATH += src

include(src/desktop/desktop.pri)
include(src/core/core.pri)
include(src/gui/gui.pri)
include(src/board/board.pri)
include(src/domain/domain.pri)
include(src/frameworks/frameworks.pri)
include(src/tools/tools.pri)
include(src/document/document.pri)

FORMS += resources/forms/mainWindow.ui \
   resources/forms/preferences.ui \
   resources/forms/brushProperties.ui \
   resources/forms/documents.ui \
   resources/forms/blackoutWidget.ui \
   resources/forms/trapFlash.ui \
   resources/forms/youTubePublishingDialog.ui \
   resources/forms/capturePublishing.ui \
   resources/forms/intranetPodcastPublishingDialog.ui

UB_ETC.files = resources/etc
UB_I18N.files = resources/i18n/*.qm
UB_LIBRARY.files = resources/library
UB_FONTS.files = resources/fonts
UB_THIRDPARTY_INTERACTIVE.files = thirdparty/interactive

DEFINES += NO_THIRD_PARTY_WARNINGS
DEFINES += UBVERSION=\"\\\"$${LONG_VERSION}\"\\\" \
   UBVERSION_RC=$$VERSION_RC
ALPHA_BETA_STR = $$find(VERSION, "[ab]")
count(ALPHA_BETA_STR, 1):DEFINES += PRE_RELEASE
BUILD_DIR = build

macx:BUILD_DIR = $$BUILD_DIR/macx
win32:BUILD_DIR = $$BUILD_DIR/win32
linux-g++*:BUILD_DIR = $$BUILD_DIR/linux

CONFIG(debug, debug|release):BUILD_DIR = $$BUILD_DIR/debug
CONFIG(release, debug|release) {
   BUILD_DIR = $$BUILD_DIR/release
   CONFIG += warn_off
}

DESTDIR = $$BUILD_DIR/product
OBJECTS_DIR = $$BUILD_DIR/objects
MOC_DIR = $$BUILD_DIR/moc
RCC_DIR = $$BUILD_DIR/rcc
UI_DIR = $$BUILD_DIR/ui

THIRD_PARTY_PATH=../OpenBoard-ThirdParty
win32 {
   LIBS += -lUser32
   LIBS += -lGdi32
   LIBS += -lAdvApi32
   LIBS += -lOle32

   RC_FILE = resources/win/OpenBoard.rc
   CONFIG += axcontainer
   exists(console):CONFIG += console
   QMAKE_CXXFLAGS += /MP
   CONFIG( debug, debug|release ) {
      QMAKE_CXXFLAGS += /MDd
   } else {
      QMAKE_CXXFLAGS += /MD
   }
   QMAKE_LFLAGS += /VERBOSE:LIB
   UB_LIBRARY.path = $$DESTDIR
   UB_I18N.path = $$DESTDIR/i18n
   UB_ETC.path = $$DESTDIR
   UB_THIRDPARTY_INTERACTIVE.path = $$DESTDIR/library
   system(md $$replace(BUILD_DIR, /, \\))
   system(echo "$$VERSION" > $$BUILD_DIR/version)
   system(echo "$$LONG_VERSION" > $$BUILD_DIR/longversion)
   system(echo "$$SVN_VERSION" > $$BUILD_DIR/svnversion)

   DEFINES += NOMINMAX # avoids compilation error in qdatetime.h


   # Windows doesn't support file versions with more than 4 fields, so
   # we omit the build number (which is only used for pre-release versions
   # anyway)

   VERSION_RC = $$VERSION_MAJ,$$VERSION_MIN,$$VERSION_PATCH,$$VERSION_TYPE
   VERSION_RC = $$replace(VERSION_RC, "a", "160") # 0xA0
   VERSION_RC = $$replace(VERSION_RC, "b", "176") # 0xB0
   VERSION_RC = $$replace(VERSION_RC, "rc", "192" ) # 0xC0
   VERSION_RC = $$replace(VERSION_RC, "r", "240") # 0xF0


   DEFINES += UBVERSION=\"\\\"$${VERSION}\"\\\" \
        UBVERSION_RC=$$VERSION_RC
}

macx {
   DEFINES += Q_WS_MACX

   LIBS += -framework Foundation
   LIBS += -framework Cocoa
   LIBS += -framework Carbon
   LIBS += -framework AVFoundation
   LIBS += -framework CoreMedia
   LIBS += -lcrypto

   LIBS += -L/usr/local/opt/openssl/lib

   LIBS += -L/usr/local/opt/ffmpeg/lib
   INCLUDEPATH += /usr/local/opt/openssl/include
   INCLUDEPATH += /usr/local/opt/ffmpeg/include

   LIBS        += -L/usr/local/opt/poppler/lib -lpoppler
   INCLUDEPATH += /usr/local/opt/poppler/include
   INCLUDEPATH += /usr/local/opt/poppler/include/poppler

   CONFIG(release, debug|release):CONFIG += x86_64
   CONFIG(debug, debug|release):CONFIG += x86_64

   QMAKE_MAC_SDK = macosx
   QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10

   QMAKE_CXXFLAGS += -Wno-overloaded-virtual
   #VERSION_RC_PATH = "$$BUILD_DIR/version_rc"

   # No references to breakpad in the code =>is this still used?
   # Embed version into executable for breakpad
   #QMAKE_LFLAGS += -sectcreate \
   #    __DATA \
   #    __version \
   #    $$VERSION_RC_PATH

   QMAKE_CXXFLAGS_RELEASE += -gdwarf-2 \
       -mdynamic-no-pic

#    QMAKE_CFLAGS += -fopenmp
 #   QMAKE_CXXFLAGS += -fopenmp
  #  QMAKE_LFLAGS += -fopenmp

   CONTENTS_DIR = "Contents"
   RESOURCES_DIR = "Contents/Resources"
   FRAMEWORKS_DIR = "Contents/Frameworks"

   UB_ETC.files = "resources/etc"
   UB_ETC.path = "$$RESOURCES_DIR"
   UB_LIBRARY.files = "resources/library"
   UB_LIBRARY.path = "$$RESOURCES_DIR"
   UB_FONTS.files = "resources/fonts"
   UB_FONTS.path = "$$RESOURCES_DIR"
   UB_THIRDPARTY_INTERACTIVE.files = $$files($$THIRD_PARTY_PATH/interactive/*)
   UB_THIRDPARTY_INTERACTIVE.path = "$$RESOURCES_DIR/library/interactive"
   UB_MACX_ICNS.files = $$files(resources/macx/*.icns)
   UB_MACX_ICNS.path = "$$RESOURCES_DIR"
   UB_MACX_EXTRAS.files = "resources/macx/Save PDF to OpenBoard.workflow"
   UB_MACX_EXTRAS.path = "$$RESOURCES_DIR"
   UB_I18N.path = $$DESTDIR/i18n # not used

   QMAKE_BUNDLE_DATA += UB_ETC \
       UB_LIBRARY \
       UB_FONTS \
       UB_THIRDPARTY_INTERACTIVE \
       UB_MACX_ICNS \
       UB_MACX_EXTRAS \
       SPARKLE_KEY \
       FRAMEWORKS

   QMAKE_PKGINFO_TYPEINFO = "OpenB"
   QMAKE_INFO_PLIST = "resources/macx/Info.plist"

   # For packger (release.macx.sh script) to know OpenBoard version
   system(mkdir -p $$BUILD_DIR)
   system(printf \""$$OSX_VERSION"\" > $$BUILD_DIR/osx_version)
   system(printf \""$$VERSION"\" > $$BUILD_DIR/version)
  # system(printf "%02x%02x%02x%02x" `printf $$VERSION_RC | cut -d ',' -f 1` `printf $$VERSION_RC | cut -d ',' -f 2` `printf $$VERSION_RC | cut -d ',' -f 3` `printf $$VERSION_RC | cut -d ',' -f 4` | xxd -r -p > "$$VERSION_RC_PATH")
}

linux-g++* {
    CONFIG += link_prl
    LIBS += -lcrypto
    #LIBS += -lprofiler
    LIBS += -lX11
    LIBS += -lquazip5
    INCLUDEPATH += "/usr/include/quazip"

    LIBS += -lpoppler
    INCLUDEPATH += "/usr/include/poppler"

    QMAKE_CFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
    UB_LIBRARY.path = $$DESTDIR
    UB_I18N.path = $$DESTDIR/i18n
    UB_ETC.path = $$DESTDIR
    UB_THIRDPARTY_INTERACTIVE.path = $$DESTDIR/library
    system(mkdir -p $$BUILD_DIR)
    system(echo "$$VERSION" > $$BUILD_DIR/version)
    system(echo "$$LONG_VERSION" > $$BUILD_DIR/longversion)
    system(echo "$$SVN_VERSION" > $$BUILD_DIR/svnversion)
}


RESOURCES += resources/OpenBoard.qrc


