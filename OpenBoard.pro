TARGET = "OpenBoard"
TEMPLATE = app

CONFIG += c++17
CONFIG -= flat
CONFIG += lrelease
CONFIG += debug_and_release \
          no_include_pwd

# Don't use deprecated APIs
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050900
DEFINES += QT_DEPRECATED_WARNINGS

VERSION_MAJ = 1
VERSION_MIN = 7
VERSION_PATCH = 0
VERSION_TYPE = rc # a = alpha, b = beta, rc = release candidate, r = release, other => error
VERSION_BUILD = 231102

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

QT += svg
greaterThan(QT_MAJOR_VERSION, 5):QT += svgwidgets
QT += network
QT += xml
QT += uitools
QT += multimedia
QT += multimediawidgets
QT += webenginewidgets
QT += printsupport
QT += core
QT += concurrent
greaterThan(QT_MAJOR_VERSION, 5): win32: QT += core5compat
greaterThan(QT_MAJOR_VERSION, 5): macx: QT += core5compat
linux: QT += dbus

INCLUDEPATH += src

include(src/adaptors/adaptors.pri)
include(src/api/api.pri)
include(src/board/board.pri)
include(src/core/core.pri)
include(src/document/document.pri)
include(src/domain/domain.pri)
include(src/frameworks/frameworks.pri)
include(src/gui/gui.pri)
include(src/network/network.pri)
include(src/pdf/pdf.pri)
include(src/podcast/podcast.pri)
include(src/tools/tools.pri)
include(src/desktop/desktop.pri)
include(src/web/web.pri)
include(src/singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

DEPENDPATH += src/pdf-merger
INCLUDEPATH += src/pdf-merger
include(src/pdf-merger/pdfMerger.pri)

#plugins
include(plugins/plugins.pri)
INCLUDEPATH += plugins/cffadaptor/src

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

win32 {


   LIBS += -lUser32
   LIBS += -lGdi32
   LIBS += -lAdvApi32
   LIBS += -lOle32

   THIRD_PARTY_PATH=../OpenBoard-ThirdParty
   include($$THIRD_PARTY_PATH/libs.pri)

   DEPENDPATH += $$THIRD_PARTY_PATH/quazip/
   INCLUDEPATH += $$THIRD_PARTY_PATH/quazip/
   include($$THIRD_PARTY_PATH/quazip/quazip.pri)

   RC_FILE = resources/win/OpenBoard.rc
   CONFIG += axcontainer
   exists(console):CONFIG += console
   QMAKE_CXXFLAGS += /MP
   CONFIG( debug, debug|release ) {
      QMAKE_CXXFLAGS += /MDd
   } else {
      QMAKE_CXXFLAGS += /MD
   }
   QMAKE_CXXFLAGS_RELEASE += /Od /Zi
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


   LIBS += -L/usr/local/lib -lquazip1-qt6.1.4
   INCLUDEPATH += /opt/local/include/QuaZip-Qt6-1.4/quazip
   
   LIBS += -L/opt/local/lib
   INCLUDEPATH += /usr/local/opt/openssl/include
   INCLUDEPATH += /opt/local/include

   LIBS        += -L/opt/local/lib -lpoppler
   INCLUDEPATH += /opt/local/include/poppler

   QMAKE_MAC_SDK = macosx
   QMAKE_MACOSX_DEPLOYMENT_TARGET = 12

   # For universal builds
   QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64
   #QMAKE_APPLE_DEVICE_ARCHS = arm64

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

    exists(resources/i18n/OpenBoard_ar.qm) {
        TRANSLATION_ar.files = resources/i18n/OpenBoard_ar.qm \
            resources/i18n/Localizable.strings
        TRANSLATION_ar.path = "$$RESOURCES_DIR/ar.lproj"
        QMAKE_BUNDLE_DATA += TRANSLATION_ar
    }
    exists(resources/i18n/OpenBoard_bg.qm) {
        TRANSLATION_bg.files = resources/i18n/OpenBoard_bg.qm \
            resources/i18n/Localizable.strings
        TRANSLATION_bg.path = "$$RESOURCES_DIR/bg.lproj"
        QMAKE_BUNDLE_DATA += TRANSLATION_bg
    }
    exists(resources/i18n/OpenBoard_ca.qm) {
        TRANSLATION_ca.files = resources/i18n/OpenBoard_ca.qm \
            resources/i18n/Localizable.strings
        TRANSLATION_ca.path = "$$RESOURCES_DIR/ca.lproj"
        QMAKE_BUNDLE_DATA += TRANSLATION_ca
    }
    exists(resources/i18n/OpenBoard_cs.qm) {
        TRANSLATION_cs.files = resources/i18n/OpenBoard_cs.qm \
            resources/i18n/localizable.strings
        TRANSLATION_cs.path = "$$RESOURCES_DIR/cs.lproj"
        QMAKE_BUNDLE_DATA += TRANSLATION_cs
    }
    exists(resources/i18n/OpenBoard_da.qm) {
        TRANSLATION_da.files = resources/i18n/OpenBoard_da.qm \
            resources/i18n/Localizable.strings
        TRANSLATION_da.path = "$$RESOURCES_DIR/da.lproj"
        QMAKE_BUNDLE_DATA += TRANSLATION_da
    }
    exists(resources/i18n/OpenBoard_de.qm) {
        TRANSLATION_de.files = resources/i18n/OpenBoard_de.qm \
            resources/i18n/Localizable.strings
        TRANSLATION_de.path = "$$RESOURCES_DIR/de.lproj"
        QMAKE_BUNDLE_DATA += TRANSLATION_de
    }
    exists(resources/i18n/OpenBoard_el.qm) {
        TRANSLATION_el.files = resources/i18n/OpenBoard_el.qm \
            resources/i18n/Localizable.strings
        TRANSLATION_el.path = "$$RESOURCES_DIR/el.lproj"
        QMAKE_BUNDLE_DATA += TRANSLATION_el
    }
   exists(resources/i18n/OpenBoard_en.qm) {
       TRANSLATION_en.files = resources/i18n/OpenBoard_en.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_en.path = "$$RESOURCES_DIR/en.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_en
   }
   exists(resources/i18n/OpenBoard_en_UK.qm) {
       TRANSLATION_en_UK.files = resources/i18n/OpenBoard_en_UK.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_en_UK.path = "$$RESOURCES_DIR/en_UK.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_en_UK
   }
   exists(resources/i18n/OpenBoard_es.qm) {
       TRANSLATION_es.files = resources/i18n/OpenBoard_es.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_es.path = "$$RESOURCES_DIR/es.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_es
   }
   exists(resources/i18n/OpenBoard_eu.qm) {
       TRANSLATION_eu.files = resources/i18n/OpenBoard_eu.qm \
           resources/i18n/localizable.strings
       TRANSLATION_eu.path = "$$RESOURCES_DIR/eu.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_eu
   }
   exists(resources/i18n/OpenBoard_fr.qm) {
       TRANSLATION_fr.files = resources/i18n/OpenBoard_fr.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_fr.path = "$$RESOURCES_DIR/fr.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_fr
   }
   exists(resources/i18n/OpenBoard_fr_CH.qm) {
       TRANSLATION_fr_CH.files = resources/i18n/OpenBoard_fr_CH.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_fr_CH.path = "$$RESOURCES_DIR/fr_CH.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_fr_CH
   }
   exists(resources/i18n/OpenBoard_gl.qm) {
       TRANSLATION_gl.files = resources/i18n/OpenBoard_gl.qm \
           resources/i18n/localizable.strings
       TRANSLATION_gl.path = "$$RESOURCES_DIR/gl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_gl
   }
   exists(resources/i18n/OpenBoard_hr.qm) {
       TRANSLATION_hr.files = resources/i18n/OpenBoard_hr.qm \
           resources/i18n/localizable.strings
       TRANSLATION_hr.path = "$$RESOURCES_DIR/hr.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_hr
   }
   exists(resources/i18n/OpenBoard_hu.qm) {
       TRANSLATION_hu.files = resources/i18n/OpenBoard_hu.qm \
           resources/i18n/localizable.strings
       TRANSLATION_hu.path = "$$RESOURCES_DIR/hu.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_hu
   }
   exists(resources/i18n/OpenBoard_it.qm) {
       TRANSLATION_it.files = resources/i18n/OpenBoard_it.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_it.path = "$$RESOURCES_DIR/it.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_it
   }
   exists(resources/i18n/OpenBoard_iw.qm) {
       TRANSLATION_iw.files = resources/i18n/OpenBoard_iw.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_iw.path = "$$RESOURCES_DIR/iw.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_iw
   }
   exists(resources/i18n/OpenBoard_ja.qm) {
       TRANSLATION_ja.files = resources/i18n/OpenBoard_ja.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ja.path = "$$RESOURCES_DIR/ja.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ja
   }
   exists(resources/i18n/OpenBoard_ko.qm) {
       TRANSLATION_ko.files = resources/i18n/OpenBoard_ko.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ko.path = "$$RESOURCES_DIR/ko.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ko
   }
   exists(resources/i18n/OpenBoard_mg.qm) {
       TRANSLATION_mg.files = resources/i18n/OpenBoard_mg.qm \
           resources/i18n/localizable.strings
       TRANSLATION_mg.path = "$$RESOURCES_DIR/mg.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_mg
   }
   exists(resources/i18n/OpenBoard_nb.qm) {
       TRANSLATION_nb.files = resources/i18n/OpenBoard_nb.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_nb.path = "$$RESOURCES_DIR/nb.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_nb
   }
   exists(resources/i18n/OpenBoard_nl.qm) {
       TRANSLATION_nl.files = resources/i18n/OpenBoard_nl.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_nl.path = "$$RESOURCES_DIR/nl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_nl
   }
   exists(resources/i18n/OpenBoard_pl.qm) {
       TRANSLATION_pl.files = resources/i18n/OpenBoard_pl.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pl.path = "$$RESOURCES_DIR/pl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pl
   }
   exists(resources/i18n/OpenBoard_pt.qm) {
       TRANSLATION_pt.files = resources/i18n/OpenBoard_pt.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pt.path = "$$RESOURCES_DIR/pt.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pt
   }
   exists(resources/i18n/OpenBoard_pt_BR.qm) {
       TRANSLATION_pt_BR.files = resources/i18n/OpenBoard_pt_BR.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pt_BR.path = "$$RESOURCES_DIR/pt_BR.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pt_BR
   }
   exists(resources/i18n/OpenBoard_ro.qm) {
       TRANSLATION_ro.files = resources/i18n/OpenBoard_ro.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ro.path = "$$RESOURCES_DIR/ro.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ro
   }
   exists(resources/i18n/OpenBoard_ru.qm) {
       TRANSLATION_ru.files = resources/i18n/OpenBoard_ru.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ru.path = "$$RESOURCES_DIR/ru.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ru
   }
   exists(resources/i18n/OpenBoard_sk.qm) {
       TRANSLATION_sk.files = resources/i18n/OpenBoard_sk.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_sk.path = "$$RESOURCES_DIR/sk.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_sk
   }
   exists(resources/i18n/OpenBoard_sv.qm) {
       TRANSLATION_sv.files = resources/i18n/OpenBoard_sv.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_sv.path = "$$RESOURCES_DIR/sv.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_sv
   }
   exists(resources/i18n/OpenBoard_tr.qm) {
       TRANSLATION_tr.files = resources/i18n/OpenBoard_tr.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_tr.path = "$$RESOURCES_DIR/tr.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_tr
   }
   exists(resources/i18n/OpenBoard_uk.qm) {
       TRANSLATION_uk.files = resources/i18n/OpenBoard_uk.qm \
           resources/i18n/localizable.strings
       TRANSLATION_uk.path = "$$RESOURCES_DIR/uk.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_uk
   }
   exists(resources/i18n/OpenBoard_zh.qm) {
       TRANSLATION_zh.files = resources/i18n/OpenBoard_zh.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_zh.path = "$$RESOURCES_DIR/zh.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_zh
   }
   exists(resources/i18n/OpenBoard_zh_CN.qm) {
       TRANSLATION_zh_CN.files = resources/i18n/OpenBoard_zh_CN.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_zh_CN.path = "$$RESOURCES_DIR/zh_CN.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_zh_CN
   }
   exists(resources/i18n/OpenBoard_zh_TW.qm) {
       TRANSLATION_zh_TW.files = resources/i18n/OpenBoard_zh_TW.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_zh_TW.path = "$$RESOURCES_DIR/zh_TW.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_zh_TW
   }

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

    greaterThan(QT_MAJOR_VERSION, 5) {
        LIBS += -lquazip6
        INCLUDEPATH += "/usr/include/quazip6"
    } else {
        LIBS += -lquazip5
        INCLUDEPATH += "/usr/include/quazip5"
    }

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

# When adding a translation here, also add it in the macx part
TRANSLATIONS = resources/i18n/OpenBoard_ar.ts \
    resources/i18n/OpenBoard_bg.ts \
    resources/i18n/OpenBoard_ca.ts \
    resources/i18n/OpenBoard_cs.ts \
    resources/i18n/OpenBoard_da.ts \
    resources/i18n/OpenBoard_de.ts \
    resources/i18n/OpenBoard_el.ts \
    resources/i18n/OpenBoard_en.ts \
    resources/i18n/OpenBoard_en_UK.ts \
    resources/i18n/OpenBoard_es.ts \
    resources/i18n/OpenBoard_eu.ts \
    resources/i18n/OpenBoard_fr.ts \
    resources/i18n/OpenBoard_fr_CH.ts \
    resources/i18n/OpenBoard_gl.ts \
    resources/i18n/OpenBoard_hr.ts \
    resources/i18n/OpenBoard_hu.ts \
    resources/i18n/OpenBoard_it.ts \
    resources/i18n/OpenBoard_iw.ts \
    resources/i18n/OpenBoard_ja.ts \
    resources/i18n/OpenBoard_ko.ts \
    resources/i18n/OpenBoard_mg.ts \
    resources/i18n/OpenBoard_nb.ts \
    resources/i18n/OpenBoard_nl.ts \
    resources/i18n/OpenBoard_pl.ts \
    resources/i18n/OpenBoard_pt.ts \
    resources/i18n/OpenBoard_pt_BR.ts \
    resources/i18n/OpenBoard_ro.ts \
    resources/i18n/OpenBoard_ru.ts \
    resources/i18n/OpenBoard_sk.ts \
    resources/i18n/OpenBoard_sv.ts \
    resources/i18n/OpenBoard_tr.ts \
    resources/i18n/OpenBoard_uk.ts \
    resources/i18n/OpenBoard_zh.ts \
    resources/i18n/OpenBoard_zh_CN.ts \
    resources/i18n/OpenBoard_zh_TW.ts

INSTALLS = UB_ETC \
   UB_I18N \
   UB_LIBRARY \
   UB_THIRDPARTY_INTERACTIVE

DISTFILES += \
    resources/images/moveDown.svg \
    resources/images/moveDownDisabled.svg

