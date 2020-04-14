THIRD_PARTY_PATH="../OpenBoard-ThirdParty"

linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += poppler
    PKGCONFIG += poppler-splash
    PKGCONFIG += freetype2

    # Find different versions of quazip
    packagesExist(quazip) {
        PKGCONFIG += quazip
        message("using quazip >= 0.7.4 with pkgconfig support")
    } else {

        # Debian stretch
        exists(/usr/include/quazip/quazip.h) {
            INCLUDEPATH += "/usr/include/quazip"
            LIBS        += "-lquazip5"
            message("using quazip =< 0.7.4 without pkgconfig support, headers in /usr/include/quazip")
        }

        # Debian buster and beyond
        exists(/usr/include/quazip5/quazip.h) {
            INCLUDEPATH += "/usr/include/quazip5"
            LIBS        += "-lquazip5"
            message("using quazip =< 0.7.4 without pkgconfig support, headers in /usr/include/quazip5")
        }
    }
}

!linux {
    DEFINES += USE_XPDF
    include($$THIRD_PARTY_PATH/libs.pri)
    #ThirdParty
    DEPENDPATH += $$THIRD_PARTY_PATH/quazip/
    INCLUDEPATH += $$THIRD_PARTY_PATH/quazip/
    include($$THIRD_PARTY_PATH/quazip/quazip.pri)
}
