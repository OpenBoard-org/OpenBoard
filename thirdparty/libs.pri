
linux-g++ {
    SUB_LIB = "linux"
}

macx {
    SUB_LIB = "macx"
}

win32 {
    SUB_LIB = "win32"
}

FREETYPE_DIR = "$$PWD/freetype/freetype-2.4.4"
LIBS        += "-L$$PWD/freetype/lib/$$SUB_LIB" "-lfreetype"
# no INCLUDEPATH, freetype is not used directly

XPDF_DIR     = "$$PWD/xpdf/xpdf-3.02"
LIBS        += "-L$$PWD/xpdf/lib/$$SUB_LIB" "-lxpdf"
INCLUDEPATH += "$$XPDF_DIR"
INCLUDEPATH += "$$XPDF_DIR/goo"
INCLUDEPATH += "$$XPDF_DIR/splash"

BREAKPAD_DIR = "$$PWD/google-breakpad/r318"
INCLUDEPATH += "$$BREAKPAD_DIR/include" "$$BREAKPAD_DIR/include/google_breakpad"

win32 {
CONFIG(debug, debug|release) {
        LIBS        += "-L$$BREAKPAD_DIR/lib/$$SUB_LIB" "-ldbreakpad"
}
CONFIG(release, debug|release) {
        LIBS        += "-L$$BREAKPAD_DIR/lib/$$SUB_LIB" "-lbreakpad"
}
    LIBS        += "-L$$PWD/openssl/0.9.8i/lib/VC/static" "-llibeay32MD"
    INCLUDEPATH += "$$PWD/openssl/0.9.8i/include"

    LIBS        += "-lWmvcore"
    LIBS        += "-lWinmm"

    LIBS        += "-L$$PWD/fleetwood/reply2005/lib" "-lreply2005"
    INCLUDEPATH += "$$PWD/fleetwood/reply2005/include"

    LIBS        += "-L$$PWD/fleetwood/replyWRS970/lib" "-lreplyWRS970"
    INCLUDEPATH += "$$PWD/fleetwood/replyWRS970/include"

}

macx {
    QMAKE_CFLAGS += "-F$$PWD/Frameworks"
    LIBS         += "-F$$PWD/Frameworks"
    
    LIBS         += "-framework QuickTime"
    LIBS         += "-framework QuartzCore"
    LIBS         += "-framework AudioToolbox"
    LIBS         += "-framework CoreAudio"
    LIBS         += "-framework ApplicationServices"
    LIBS         += "-framework Carbon"
            
    LIBS         += "-lcrypto"
    LIBS         += "-L$$BREAKPAD_DIR/lib/$$SUB_LIB" "-lbreakpad"
    LIBS         += "$$PWD/unsanity/ape/APELite.o"
    INCLUDEPATH  += "$$PWD/unsanity/ape"
}


