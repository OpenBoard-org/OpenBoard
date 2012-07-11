
HEADERS      += src/frameworks/UBGeometryUtils.h \
                src/frameworks/UBPlatformUtils.h \
                src/frameworks/UBFileSystemUtils.h \
                src/frameworks/UBStringUtils.h \
                src/frameworks/UBVersion.h \
                src/frameworks/UBCoreGraphicsScene.h \
                src/frameworks/UBCryptoUtils.h \
                src/frameworks/UBBase32.h

SOURCES      += src/frameworks/UBGeometryUtils.cpp \
                src/frameworks/UBPlatformUtils.cpp \
                src/frameworks/UBFileSystemUtils.cpp \
                src/frameworks/UBStringUtils.cpp \
                src/frameworks/UBVersion.cpp \
                src/frameworks/UBCoreGraphicsScene.cpp \
                src/frameworks/UBCryptoUtils.cpp \
                src/frameworks/UBBase32.cpp


win32 {

    SOURCES  += src/frameworks/UBPlatformUtils_win.cpp
}            


macx {                

    OBJECTIVE_SOURCES  += src/frameworks/UBPlatformUtils_mac.mm
                
}       


linux-g++ {

    SOURCES  += src/frameworks/UBPlatformUtils_linux.cpp
}         
linux-g++-32 {

    SOURCES  += src/frameworks/UBPlatformUtils_linux.cpp
}
linux-g++-64 {

    SOURCES  += src/frameworks/UBPlatformUtils_linux.cpp
}
