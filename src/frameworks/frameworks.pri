
HEADERS      += src/frameworks/UBGeometryUtils.h \
                src/frameworks/UBPlatformUtils.h \
                src/frameworks/UBFileSystemUtils.h \
                src/frameworks/UBStringUtils.h \
                src/frameworks/UBVersion.h \
                src/frameworks/UBCoreGraphicsScene.h \
                src/frameworks/UBCryptoUtils.h \
                src/frameworks/UBBackgroundLoader.h \
                src/frameworks/UBBlockingBuffer.h \
                src/frameworks/UBBase32.h

SOURCES      += src/frameworks/UBGeometryUtils.cpp \
                src/frameworks/UBPlatformUtils.cpp \
                src/frameworks/UBFileSystemUtils.cpp \
                src/frameworks/UBStringUtils.cpp \
                src/frameworks/UBVersion.cpp \
                src/frameworks/UBCoreGraphicsScene.cpp \
                src/frameworks/UBCryptoUtils.cpp \
                src/frameworks/UBBackgroundLoader.cpp \
                src/frameworks/UBBlockingBuffer.cpp \
                src/frameworks/UBBase32.cpp


win32 {

    SOURCES  += src/frameworks/UBPlatformUtils_win.cpp
}


macx {

    OBJECTIVE_SOURCES  += src/frameworks/UBPlatformUtils_mac.mm

}


linux-g++* {
    HEADERS  += src/frameworks/linux/UBScreenCastDesktopPortalWrapper.h \
                src/frameworks/linux/UBScreenshotDesktopPortalWrapper.h \
                src/frameworks/linux/UBPipewireSink.h
    SOURCES  += src/frameworks/UBPlatformUtils_linux.cpp \
                src/frameworks/linux/UBScreenCastDesktopPortalWrapper.cpp \
                src/frameworks/linux/UBScreenshotDesktopPortalWrapper.cpp \
                src/frameworks/linux/UBPipewireSink.cpp

    CONFIG += link_pkgconfig
    PKGCONFIG += libpipewire-0.3
}
