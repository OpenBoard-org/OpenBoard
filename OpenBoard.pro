TARGET = "OpenBoard"
TEMPLATE = app

THIRD_PARTY_PATH=../OpenBoard-ThirdParty

CONFIG -= flat
CONFIG += debug_and_release \
          no_include_pwd


VERSION_MAJ = 1
VERSION_MIN = 4
VERSION_PATCH = 2
VERSION_TYPE = b # a = alpha, b = beta, rc = release candidate, r = release, other => error
VERSION_BUILD = 0

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

QT += webkit
#QT += webengine
QT += svg
QT += network
QT += xml
QT += script
QT += xmlpatterns
QT += uitools
QT += multimedia
QT += webkitwidgets
#QT += webenginewidgets
QT += multimediawidgets
QT += printsupport
QT += core

INCLUDEPATH += src

include($$THIRD_PARTY_PATH/libs.pri)
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
include(src/canvas/canvas.pri)
include(src/customWidgets/customWidgets.pri)

DEPENDPATH += src/pdf-merger
INCLUDEPATH += src/pdf-merger
include(src/pdf-merger/pdfMerger.pri)

#ThirdParty
DEPENDPATH += $$THIRD_PARTY_PATH/quazip/
INCLUDEPATH += $$THIRD_PARTY_PATH/quazip/
include($$THIRD_PARTY_PATH/quazip/quazip.pri)
DEPENDPATH += $$THIRD_PARTY_PATH/qt/singleapplication
INCLUDEPATH += $$THIRD_PARTY_PATH/qt/singleapplication
include($$THIRD_PARTY_PATH/qt/singleapplication/qtsingleapplication.pri)
include($$THIRD_PARTY_PATH/qt/lockedfile/qtlockedfile.pri)

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
   LIBS += -llept
   LIBS += -ltesseract

   RC_FILE = resources/win/OpenBoard.rc
   CONFIG += axcontainer
   exists(console):CONFIG += console
   QMAKE_CXXFLAGS += /MP
   QMAKE_CXXFLAGS += /MD
   QMAKE_CXXFLAGS_RELEASE += /Od /Zi
   QMAKE_LFLAGS += /VERBOSE:LIB

    QMAKE_CFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS += -std=c++11

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
   LIBS += -framework Foundation
   LIBS += -framework Cocoa
   LIBS += -framework Carbon
   LIBS += -framework AVFoundation
   LIBS += -framework CoreMedia
   LIBS += -lcrypto
   LIBS += -llept
   LIBS += -ltesseract

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


    QMAKE_CFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS += -std=c++11
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
   exists(resources/i18n/OpenBoard_de.qm) {
       TRANSLATION_de.files = resources/i18n/OpenBoard_de.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_de.path = "$$RESOURCES_DIR/de.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_de
   }
   exists(resources/i18n/OpenBoard_nl.qm) {
       TRANSLATION_nl.files = resources/i18n/OpenBoard_nl.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_nl.path = "$$RESOURCES_DIR/nl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_nl
   }
   exists(resources/i18n/OpenBoard_es.qm) {
       TRANSLATION_es.files = resources/i18n/OpenBoard_es.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_es.path = "$$RESOURCES_DIR/es.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_es
   }
   exists(resources/i18n/OpenBoard_it.qm) {
       TRANSLATION_it.files = resources/i18n/OpenBoard_it.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_it.path = "$$RESOURCES_DIR/it.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_it
   }
   exists(resources/i18n/OpenBoard_pl.qm) {
       TRANSLATION_pl.files = resources/i18n/OpenBoard_pl.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pl.path = "$$RESOURCES_DIR/pl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pl
   }
   exists(resources/i18n/OpenBoard_ru.qm) {
       TRANSLATION_ru.files = resources/i18n/OpenBoard_ru.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ru.path = "$$RESOURCES_DIR/ru.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ru
   }
   exists(resources/i18n/OpenBoard_da.qm) {
       TRANSLATION_da.files = resources/i18n/OpenBoard_da.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_da.path = "$$RESOURCES_DIR/da.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_da
   }
   exists(resources/i18n/OpenBoard_nb.qm) {
       TRANSLATION_nb.files = resources/i18n/OpenBoard_nb.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_nb.path = "$$RESOURCES_DIR/nb.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_nb
   }
   exists(resources/i18n/OpenBoard_sv.qm) {
       TRANSLATION_sv.files = resources/i18n/OpenBoard_sv.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_sv.path = "$$RESOURCES_DIR/sv.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_sv
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
   exists(resources/i18n/OpenBoard_ro.qm) {
       TRANSLATION_ro.files = resources/i18n/OpenBoard_ro.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ro.path = "$$RESOURCES_DIR/ro.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ro
   }
   exists(resources/i18n/OpenBoard_ar.qm) {
       TRANSLATION_ar.files = resources/i18n/OpenBoard_ar.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ar.path = "$$RESOURCES_DIR/ar.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ar
   }
   exists(resources/i18n/OpenBoard_iw.qm) {
       TRANSLATION_iw.files = resources/i18n/OpenBoard_iw.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_iw.path = "$$RESOURCES_DIR/iw.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_iw
   }
   exists(resources/i18n/OpenBoard_pt.qm) {
       TRANSLATION_pt.files = resources/i18n/OpenBoard_pt.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pt.path = "$$RESOURCES_DIR/pt.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pt
   }
   exists(resources/i18n/OpenBoard_sk.qm) {
       TRANSLATION_sk.files = resources/i18n/OpenBoard_sk.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_sk.path = "$$RESOURCES_DIR/sk.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_sk
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
   exists(resources/i18n/OpenBoard_el.qm) {
       TRANSLATION_el.files = resources/i18n/OpenBoard_el.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_el.path = "$$RESOURCES_DIR/el.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_el
   }
   exists(resources/i18n/OpenBoard_tr.qm) {
       TRANSLATION_tr.files = resources/i18n/OpenBoard_tr.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_tr.path = "$$RESOURCES_DIR/tr.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_tr
   }
   exists(resources/i18n/OpenBoard_cs.qm) {
       TRANSLATION_cs.files = resources/i18n/OpenBoard_cs.qm \
           resources/i18n/localizable.strings
       TRANSLATION_cs.path = "$$RESOURCES_DIR/cs.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_cs
   }
   exists(resources/i18n/OpenBoard_mg.qm) {
       TRANSLATION_mg.files = resources/i18n/OpenBoard_mg.qm \
           resources/i18n/localizable.strings
       TRANSLATION_mg.path = "$$RESOURCES_DIR/mg.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_mg
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
    LIBS += -llept
    LIBS += -ltesseract
    #LIBS += -lprofiler
    LIBS += -lX11
    QMAKE_CFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp

    QMAKE_CFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS += -std=c++11

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
TRANSLATIONS = resources/i18n/OpenBoard_en.ts \
   resources/i18n/OpenBoard_en_UK.ts \
   resources/i18n/OpenBoard_fr.ts \
   resources/i18n/OpenBoard_fr_CH.ts \
   resources/i18n/OpenBoard_de.ts \
   resources/i18n/OpenBoard_nl.ts \
   resources/i18n/OpenBoard_es.ts \
   resources/i18n/OpenBoard_it.ts \
   resources/i18n/OpenBoard_pl.ts \
   resources/i18n/OpenBoard_ru.ts \
   resources/i18n/OpenBoard_da.ts \
   resources/i18n/OpenBoard_nb.ts \
   resources/i18n/OpenBoard_sv.ts \
   resources/i18n/OpenBoard_ja.ts \
   resources/i18n/OpenBoard_ko.ts \
   resources/i18n/OpenBoard_zh.ts \
   resources/i18n/OpenBoard_zh_CN.ts \
   resources/i18n/OpenBoard_zh_TW.ts \
   resources/i18n/OpenBoard_ro.ts \
   resources/i18n/OpenBoard_ar.ts \
   resources/i18n/OpenBoard_iw.ts \
   resources/i18n/OpenBoard_pt.ts \
   resources/i18n/OpenBoard_sk.ts \
   resources/i18n/OpenBoard_bg.ts \
   resources/i18n/OpenBoard_ca.ts \
   resources/i18n/OpenBoard_el.ts \
   resources/i18n/OpenBoard_tr.ts \
   resources/i18n/OpenBoard_cs.ts \
   resources/i18n/OpenBoard_mg.ts

INSTALLS = UB_ETC \
   UB_I18N \
   UB_LIBRARY \
   UB_THIRDPARTY_INTERACTIVE

DISTFILES += \
    resources/images/teacherGuide/document_large.gif \
    resources/images/teacherGuide/document_large_warning.gif \
    resources/images/cursors/drawCompass.png \
    resources/images/cursors/drawRulerLine.png \
    resources/images/cursors/eraser.png \
    resources/images/cursors/laser.png \
    resources/images/cursors/marker.png \
    resources/images/cursors/play.png \
    resources/images/cursors/resize.png \
    resources/images/cursors/rotate.png \
    resources/images/cursors/zoomIn.png \
    resources/images/cursors/zoomOut.png \
    resources/images/drawingPalette/align-bottom.png \
    resources/images/drawingPalette/align-center.png \
    resources/images/drawingPalette/align-left.png \
    resources/images/drawingPalette/align-middle.png \
    resources/images/drawingPalette/align-right.png \
    resources/images/drawingPalette/align-top.png \
    resources/images/drawingPalette/arrowsProperties.png \
    resources/images/drawingPalette/cercle.png \
    resources/images/drawingPalette/cercleON.png \
    resources/images/drawingPalette/ellipse.png \
    resources/images/drawingPalette/ellipseON.png \
    resources/images/drawingPalette/endArrowStyleArrow.png \
    resources/images/drawingPalette/endArrowStyleNone.png \
    resources/images/drawingPalette/endArrowStyleRound.png \
    resources/images/drawingPalette/fill-dotted.png \
    resources/images/drawingPalette/fill-gradient.png \
    resources/images/drawingPalette/fill-modes.png \
    resources/images/drawingPalette/fill-plain.png \
    resources/images/drawingPalette/fill-stripes.png \
    resources/images/drawingPalette/fill-transparent.png \
    resources/images/drawingPalette/free-shape.png \
    resources/images/drawingPalette/free-shapeOn.png \
    resources/images/drawingPalette/freeHand.png \
    resources/images/drawingPalette/freeHandOn.png \
    resources/images/drawingPalette/hepta.png \
    resources/images/drawingPalette/heptaON.png \
    resources/images/drawingPalette/hexa.png \
    resources/images/drawingPalette/hexaON.png \
    resources/images/drawingPalette/line-thickness.png \
    resources/images/drawingPalette/octo.png \
    resources/images/drawingPalette/octoON.png \
    resources/images/drawingPalette/paint-bucket.png \
    resources/images/drawingPalette/paint-bucketOn.png \
    resources/images/drawingPalette/paint-palette.png \
    resources/images/drawingPalette/paint-paletteOn.png \
    resources/images/drawingPalette/Penta.png \
    resources/images/drawingPalette/PentaON.png \
    resources/images/drawingPalette/rectangle.png \
    resources/images/drawingPalette/rectangleON.png \
    resources/images/drawingPalette/square.png \
    resources/images/drawingPalette/squareON.png \
    resources/images/drawingPalette/startArrowStyleArrow.png \
    resources/images/drawingPalette/startArrowStyleNone.png \
    resources/images/drawingPalette/startArrowStyleRound.png \
    resources/images/drawingPalette/strokeStyleCustomDash.png \
    resources/images/drawingPalette/strokeStyleDotLine .png \
    resources/images/drawingPalette/strokeStyleSolidLine.png \
    resources/images/drawingPalette/triangle.png \
    resources/images/drawingPalette/triangleON.png \
    resources/images/extraPalette/blackout.png \
    resources/images/extraPalette/blackoutOn.png \
    resources/images/extraPalette/compass.png \
    resources/images/extraPalette/compassOn.png \
    resources/images/extraPalette/keyboard.png \
    resources/images/extraPalette/keyboardOn.png \
    resources/images/extraPalette/mask.png \
    resources/images/extraPalette/maskOn.png \
    resources/images/extraPalette/oEmbed.png \
    resources/images/extraPalette/podcast.png \
    resources/images/extraPalette/podcastOn.png \
    resources/images/extraPalette/protractor.png \
    resources/images/extraPalette/protractorOn.png \
    resources/images/extraPalette/ruler.png \
    resources/images/extraPalette/rulerOn.png \
    resources/images/extraPalette/trapFlash.png \
    resources/images/extraPalette/webTrap.png \
    resources/images/extraPalette/webTrapEnabled.png \
    resources/images/flags/ar.png \
    resources/images/flags/bg.png \
    resources/images/flags/ca.png \
    resources/images/flags/cs.png \
    resources/images/flags/da.png \
    resources/images/flags/de.png \
    resources/images/flags/el.png \
    resources/images/flags/en.png \
    resources/images/flags/eo.png \
    resources/images/flags/es.png \
    resources/images/flags/et.png \
    resources/images/flags/eu.png \
    resources/images/flags/fa.png \
    resources/images/flags/fi.png \
    resources/images/flags/fo.png \
    resources/images/flags/fr.png \
    resources/images/flags/ga.png \
    resources/images/flags/gl.png \
    resources/images/flags/he.png \
    resources/images/flags/hr.png \
    resources/images/flags/hu.png \
    resources/images/flags/id.png \
    resources/images/flags/is.png \
    resources/images/flags/it.png \
    resources/images/flags/ja.png \
    resources/images/flags/km.png \
    resources/images/flags/lb.png \
    resources/images/flags/lt.png \
    resources/images/flags/lv.png \
    resources/images/flags/nb.png \
    resources/images/flags/nl.png \
    resources/images/flags/nn.png \
    resources/images/flags/pl.png \
    resources/images/flags/pt-br.png \
    resources/images/flags/pt-pt.png \
    resources/images/flags/ro.png \
    resources/images/flags/ru.png \
    resources/images/flags/sco.png \
    resources/images/flags/se.png \
    resources/images/flags/sk.png \
    resources/images/flags/sl.png \
    resources/images/flags/sq.png \
    resources/images/flags/sv.png \
    resources/images/flags/tg.png \
    resources/images/flags/th.png \
    resources/images/flags/tl.png \
    resources/images/flags/tr.png \
    resources/images/flags/uk.png \
    resources/images/flags/vi.png \
    resources/images/flags/zh-hans.png \
    resources/images/flags/zh-hant.png \
    resources/images/imageBackgroundPalette/adjustBackground.png \
    resources/images/imageBackgroundPalette/centerBackground.png \
    resources/images/imageBackgroundPalette/extendBackground.png \
    resources/images/imageBackgroundPalette/fillBackground.png \
    resources/images/imageBackgroundPalette/mosaicBackground.png \
    resources/images/libpalette/back.png \
    resources/images/libpalette/home.png \
    resources/images/libpalette/loading.png \
    resources/images/libpalette/miniFavorite.png \
    resources/images/libpalette/miniNewFolder.png \
    resources/images/libpalette/miniSearch.png \
    resources/images/libpalette/miniTrash.png \
    resources/images/libpalette/notFound.png \
    resources/images/libpalette/social.png \
    resources/images/licenses/ccby.png \
    resources/images/licenses/ccbync.png \
    resources/images/licenses/ccbyncnd.png \
    resources/images/licenses/ccbyncsa.png \
    resources/images/licenses/ccbynd.png \
    resources/images/licenses/ccbysa.png \
    resources/images/print/onepage.png \
    resources/images/print/thumbnails.png \
    resources/images/print/twopages.png \
    resources/images/stylusPalette/arrow.png \
    resources/images/stylusPalette/arrowOn.png \
    resources/images/stylusPalette/captureArea.png \
    resources/images/stylusPalette/captureAreaOn.png \
    resources/images/stylusPalette/drawing.png \
    resources/images/stylusPalette/eraser.png \
    resources/images/stylusPalette/eraserArrow.png \
    resources/images/stylusPalette/eraserOn.png \
    resources/images/stylusPalette/eraserOnArrow.png \
    resources/images/stylusPalette/hand.png \
    resources/images/stylusPalette/handOn.png \
    resources/images/stylusPalette/handPlay.png \
    resources/images/stylusPalette/handPlayOn.png \
    resources/images/stylusPalette/laser.png \
    resources/images/stylusPalette/laserOn.png \
    resources/images/stylusPalette/line.png \
    resources/images/stylusPalette/lineOn.png \
    resources/images/stylusPalette/marker.png \
    resources/images/stylusPalette/markerArrow.png \
    resources/images/stylusPalette/markerOn.png \
    resources/images/stylusPalette/markerOnArrow.png \
    resources/images/stylusPalette/pen.png \
    resources/images/stylusPalette/penArrow.png \
    resources/images/stylusPalette/penOn.png \
    resources/images/stylusPalette/penOnArrow.png \
    resources/images/stylusPalette/restoreZoom.png \
    resources/images/stylusPalette/richTextEditor.png \
    resources/images/stylusPalette/richTextEditorOn.png \
    resources/images/stylusPalette/text.png \
    resources/images/stylusPalette/textOn.png \
    resources/images/stylusPalette/zoomIn.png \
    resources/images/stylusPalette/zoomInOn.png \
    resources/images/stylusPalette/zoomOut.png \
    resources/images/stylusPalette/zoomOutOn.png \
    resources/images/textEditor/add-table.png \
    resources/images/textEditor/cell-properties.png \
    resources/images/textEditor/delete-column.png \
    resources/images/textEditor/delete-row.png \
    resources/images/textEditor/insert-column-left.png \
    resources/images/textEditor/insert-column-right.png \
    resources/images/textEditor/insert-row-bottom.png \
    resources/images/textEditor/insert-row-top.png \
    resources/images/toolbar/addBookmark.png \
    resources/images/toolbar/addInteractiveToLibrary.png \
    resources/images/toolbar/addToDocument.png \
    resources/images/toolbar/addToLibrary.png \
    resources/images/toolbar/addToolToLibrary.png \
    resources/images/toolbar/addToPage.png \
    resources/images/toolbar/addToWorkingDoc.png \
    resources/images/toolbar/background.png \
    resources/images/toolbar/board.png \
    resources/images/toolbar/bookmark.png \
    resources/images/toolbar/bookmarks.png \
    resources/images/toolbar/captureArea.png \
    resources/images/toolbar/captureScreen.png \
    resources/images/toolbar/captureWindow.png \
    resources/images/toolbar/clearPage.png \
    resources/images/toolbar/color.png \
    resources/images/toolbar/copy.png \
    resources/images/toolbar/cut.png \
    resources/images/toolbar/deleteDocument.png \
    resources/images/toolbar/deleteFolder.png \
    resources/images/toolbar/display.png \
    resources/images/toolbar/documents.png \
    resources/images/toolbar/duplicate.png \
    resources/images/toolbar/eraserTool.png \
    resources/images/toolbar/export.png \
    resources/images/toolbar/extraTool.png \
    resources/images/toolbar/eyeClosed.png \
    resources/images/toolbar/eyeOpened.png \
    resources/images/toolbar/group.png \
    resources/images/toolbar/hide.png \
    resources/images/toolbar/home.png \
    resources/images/toolbar/import.png \
    resources/images/toolbar/largeEraser.png \
    resources/images/toolbar/largePen.png \
    resources/images/toolbar/library.png \
    resources/images/toolbar/lineTool.png \
    resources/images/toolbar/mediumEraser.png \
    resources/images/toolbar/mediumPen.png \
    resources/images/toolbar/menu.png \
    resources/images/toolbar/mic.png \
    resources/images/toolbar/micOff.png \
    resources/images/toolbar/newDocument.png \
    resources/images/toolbar/newFolder.png \
    resources/images/toolbar/newPage.png \
    resources/images/toolbar/next.png \
    resources/images/toolbar/nextPage.png \
    resources/images/toolbar/nextPageOn.png \
    resources/images/toolbar/open.png \
    resources/images/toolbar/pageSize.png \
    resources/images/toolbar/paraschool.png \
    resources/images/toolbar/paste.png \
    resources/images/toolbar/pause.png \
    resources/images/toolbar/pdf.png \
    resources/images/toolbar/plusBlack.png \
    resources/images/toolbar/plusBlackOn.png \
    resources/images/toolbar/previous.png \
    resources/images/toolbar/previousPage.png \
    resources/images/toolbar/previousPageOn.png \
    resources/images/toolbar/print.png \
    resources/images/toolbar/profile.png \
    resources/images/toolbar/proto.png \
    resources/images/toolbar/quit.png \
    resources/images/toolbar/quitOn.png \
    resources/images/toolbar/record.png \
    resources/images/toolbar/redo.png \
    resources/images/toolbar/redoOn.png \
    resources/images/toolbar/reload.png \
    resources/images/toolbar/remove.png \
    resources/images/toolbar/removeBackground.png \
    resources/images/toolbar/rename.png \
    resources/images/toolbar/setBackground.png \
    resources/images/toolbar/settings.png \
    resources/images/toolbar/smallEraser.png \
    resources/images/toolbar/smallerText.png \
    resources/images/toolbar/smallPen.png \
    resources/images/toolbar/softwareUpdate.png \
    resources/images/toolbar/stop.png \
    resources/images/toolbar/stylus.png \
    resources/images/toolbar/stylusTab.png \
    resources/images/toolbar/tools.png \
    resources/images/toolbar/tutorial.png \
    resources/images/toolbar/undo.png \
    resources/images/toolbar/undoOn.png \
    resources/images/toolbar/ungroup.png \
    resources/images/toolbar/updates.png \
    resources/images/toolbar/web.png \
    resources/images/toolbar/webTrap.png \
    resources/images/toolbar/webTrapEnabled.png \
    resources/images/toolbar/webUpload.png \
    resources/images/toolbar/window.png \
    resources/images/toolPalette/aristoTool.png \
    resources/images/toolPalette/cacheTool.png \
    resources/images/toolPalette/compassTool.png \
    resources/images/toolPalette/magnifierTool.png \
    resources/images/toolPalette/maskTool.png \
    resources/images/toolPalette/protractorTool.png \
    resources/images/toolPalette/rulerTool.png \
    resources/images/toolPalette/triangleTool.png \
    resources/images/virtual.keyboard/29/backspace.png \
    resources/images/virtual.keyboard/29/capslock.png \
    resources/images/virtual.keyboard/29/centre-active.png \
    resources/images/virtual.keyboard/29/centre-passive.png \
    resources/images/virtual.keyboard/29/left-active.png \
    resources/images/virtual.keyboard/29/left-passive.png \
    resources/images/virtual.keyboard/29/right-active.png \
    resources/images/virtual.keyboard/29/right-passive.png \
    resources/images/virtual.keyboard/29/shift.png \
    resources/images/virtual.keyboard/29/tab.png \
    resources/images/virtual.keyboard/41/backspace.png \
    resources/images/virtual.keyboard/41/capslock.png \
    resources/images/virtual.keyboard/41/centre-active.png \
    resources/images/virtual.keyboard/41/centre-passive.png \
    resources/images/virtual.keyboard/41/left-active.png \
    resources/images/virtual.keyboard/41/left-passive.png \
    resources/images/virtual.keyboard/41/right-active.png \
    resources/images/virtual.keyboard/41/right-passive.png \
    resources/images/virtual.keyboard/41/shift.png \
    resources/images/virtual.keyboard/41/tab.png \
    resources/images/banner.png \
    resources/images/banner_linear.png \
    resources/images/bigOpenBoard.png \
    resources/images/bigUniboard.png \
    resources/images/cache_circle.png \
    resources/images/cache_close.png \
    resources/images/cache_open.png \
    resources/images/cache_square.png \
    resources/images/collapse-all.png \
    resources/images/currentDocument.png \
    resources/images/defaultWidgetIcon.png \
    resources/images/down_arrow.png \
    resources/images/download_close.png \
    resources/images/download_open.png \
    resources/images/duration1.png \
    resources/images/duration2.png \
    resources/images/duration3.png \
    resources/images/edit-mode.png \
    resources/images/expand-all.png \
    resources/images/favorites.png \
    resources/images/folder.png \
    resources/images/left_arrow.png \
    resources/images/library_close.png \
    resources/images/library_open.png \
    resources/images/myContent.png \
    resources/images/myFolder.png \
    resources/images/navig_arrow.png \
    resources/images/online.png \
    resources/images/OpenBoard.png \
    resources/images/pages_close.png \
    resources/images/pages_open.png \
    resources/images/paletteLibrary.png \
    resources/images/paletteNavigator.png \
    resources/images/right_arrow.png \
    resources/images/save.png \
    resources/images/shape.png \
    resources/images/tab_mask.png \
    resources/images/teacher_close.png \
    resources/images/teacher_open.png \
    resources/images/teacherGuideResources_close.png \
    resources/images/teacherGuideResources_open.png \
    resources/images/tools.png \
    resources/images/trash.png \
    resources/images/uniboard.png \
    resources/images/up_arrow.png \
    resources/images/backgroundPalette/background1.svg \
    resources/images/backgroundPalette/background1On.svg \
    resources/images/backgroundPalette/background2.svg \
    resources/images/backgroundPalette/background2On.svg \
    resources/images/backgroundPalette/background3.svg \
    resources/images/backgroundPalette/background3On.svg \
    resources/images/backgroundPalette/background4.svg \
    resources/images/backgroundPalette/background4On.svg \
    resources/images/createLinkPalette/LinkPage.svg \
    resources/images/createLinkPalette/LinkWeb.svg \
    resources/images/createLinkPalette/PlayAudio.svg \
    resources/images/erasePalette/eraseAll.svg \
    resources/images/erasePalette/eraseAnnotation.svg \
    resources/images/erasePalette/eraseBackground.svg \
    resources/images/erasePalette/eraseItem.svg \
    resources/images/libpalette/ApplicationsCategory.svg \
    resources/images/libpalette/AudiosCategory.svg \
    resources/images/libpalette/BookmarkCategory.svg \
    resources/images/libpalette/bookmarkIcon.svg \
    resources/images/libpalette/FavoritesCategory.svg \
    resources/images/libpalette/FlashCategory.svg \
    resources/images/libpalette/FlashIcon.svg \
    resources/images/libpalette/folder.svg \
    resources/images/libpalette/InteractivesCategory.svg \
    resources/images/libpalette/movieIcon.svg \
    resources/images/libpalette/MoviesCategory.svg \
    resources/images/libpalette/PicturesCategory.svg \
    resources/images/libpalette/ShapesCategory.svg \
    resources/images/libpalette/soundIcon.svg \
    resources/images/libpalette/trash_favorite.svg \
    resources/images/libpalette/TrashCategory.svg \
    resources/images/libpalette/WebSearchCategory.svg \
    resources/images/teacherGuide/audio_24x24.svg \
    resources/images/teacherGuide/flash_24x24.svg \
    resources/images/teacherGuide/image_24x24.svg \
    resources/images/teacherGuide/link_24x24.svg \
    resources/images/teacherGuide/movie_24x24.svg \
    resources/images/teacherGuide/pencil.svg \
    resources/images/teacherGuide/w3c_24x24.svg \
    resources/images/textEditor/align-center.svg \
    resources/images/textEditor/align-left.svg \
    resources/images/textEditor/align-right.svg \
    resources/images/textEditor/bold.svg \
    resources/images/textEditor/bucket.svg \
    resources/images/textEditor/code.svg \
    resources/images/textEditor/decrease-font-size.svg \
    resources/images/textEditor/font.svg \
    resources/images/textEditor/increase-font-size.svg \
    resources/images/textEditor/indent.svg \
    resources/images/textEditor/italic.svg \
    resources/images/textEditor/link.svg \
    resources/images/textEditor/ordered-list.svg \
    resources/images/textEditor/table.svg \
    resources/images/textEditor/underline.svg \
    resources/images/textEditor/unindent.svg \
    resources/images/textEditor/unordered-list.svg \
    resources/images/toolbar/bookmark.svg \
    resources/images/toolPalette/compassTool.svg \
    resources/images/toolPalette/podcast.svg \
    resources/images/toolPalette/podcastOn.svg \
    resources/images/trapFlashPalette/downloadToBoard.svg \
    resources/images/trapFlashPalette/downloadToLibrary.svg \
    resources/images/trapFlashPalette/linkToBoard.svg \
    resources/images/trapFlashPalette/linkToLibrary.svg \
    resources/images/trapFlashPalette/trappedFlashIcon.svg \
    resources/images/trapFlashPalette/trappedIcon.svg \
    resources/images/trapFlashPalette/trappedSoundIcon.svg \
    resources/images/trapFlashPalette/trappedVideoIcon.svg \
    resources/images/addItemToCurrentPage.svg \
    resources/images/addItemToLibrary.svg \
    resources/images/addItemToNewPage.svg \
    resources/images/angleMarker.svg \
    resources/images/banner.svg \
    resources/images/banner_linear.svg \
    resources/images/bold.svg \
    resources/images/centerAligned.svg \
    resources/images/circle.svg \
    resources/images/close.svg \
    resources/images/closeDisabled.svg \
    resources/images/closeTool.svg \
    resources/images/code.svg \
    resources/images/color.svg \
    resources/images/decrease.svg \
    resources/images/duplicate.svg \
    resources/images/duplicateDisabled.svg \
    resources/images/edit-mode.svg \
    resources/images/eyeClosed.svg \
    resources/images/eyeOpened.svg \
    resources/images/flash.svg \
    resources/images/font.svg \
    resources/images/frozen.svg \
    resources/images/groupItems.svg \
    resources/images/hflipTool.svg \
    resources/images/home.svg \
    resources/images/increase.svg \
    resources/images/italic.svg \
    resources/images/leftAligned.svg \
    resources/images/locked.svg \
    resources/images/menu.svg \
    resources/images/menuDisabled.svg \
    resources/images/minus.svg \
    resources/images/moveUp.svg \
    resources/images/moveUpDisabled.svg \
    resources/images/notAligned.svg \
    resources/images/pause.svg \
    resources/images/pin.svg \
    resources/images/play.svg \
    resources/images/plus.svg \
    resources/images/reload.svg \
    resources/images/resetAngle.svg \
    resources/images/resetTool.svg \
    resources/images/resize.svg \
    resources/images/resizeBottom.svg \
    resources/images/resizeButton.svg \
    resources/images/resizeCompass.svg \
    resources/images/resizeLeft.svg \
    resources/images/resizeRight.svg \
    resources/images/resizeRuler.svg \
    resources/images/resizeTool.svg \
    resources/images/resizeTop.svg \
    resources/images/rightAligned.svg \
    resources/images/rotate.svg \
    resources/images/rotateProtractor.svg \
    resources/images/rotateTool.svg \
    resources/images/roundeRrectangle.svg \
    resources/images/save.svg \
    resources/images/soundOff.svg \
    resources/images/soundOn.svg \
    resources/images/stop.svg \
    resources/images/toque.svg \
    resources/images/underline.svg \
    resources/images/unfrozen.svg \
    resources/images/ungroupItems.svg \
    resources/images/unlocked.svg \
    resources/images/unpin.svg \
    resources/images/vflipTool.svg \
    resources/images/z_layer_down.svg \
    resources/images/z_layer_up.svg \
    resources/images/toolbar/mediaFloppy.png \
    resources/images/toolbar/floppy.png

