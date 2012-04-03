#-------------------------------------------------
#
# Project created by QtCreator 2012-02-14T13:30:14
#
#-------------------------------------------------

win32: SUB_DIR = win32
macx: SUB_DIR = macx
linux-g++: SUB_DIR = linux
linux-g++-32: SUB_DIR = linux
linux-g++-64: SUB_DIR = linux

QUAZIP_DIR   = "$$PWD/../../../Sankore-ThirdParty/quazip"
ZLIB_DIR     = "$$PWD/../../../Sankore-ThirdParty/zlib"

INCLUDEPATH += src \
            "$$QUAZIP_DIR/quazip-0.3" \
            "$$ZLIB_DIR/1.2.3/include"

LIBS        += "-L$$QUAZIP_DIR/lib/$$SUB_DIR" "-lquazip"

QT       += xml xmlpatterns core
QT       += gui
QT       += svg 

TARGET = CFF_Adaptor
TEMPLATE = lib
win32{
    CONFIG += dll
}

DEFINES += UBCFFADAPTOR_LIBRARY
DEFINES += NO_THIRD_PARTY_WARNINGS

SOURCES += \
    src/UBCFFAdaptor.cpp

HEADERS +=\
    $$PWD/../../src/globals/UBGlobals.h \
    src/UBCFFAdaptor.h \
    src/UBCFFAdaptor_global.h \
    src/UBCFFConstants.h

OBJECTS_DIR = $$PWD/objects
MOC_DIR = $$PWD/moc
DESTDIR = $$PWD/lib/$$SUB_DIR

RESOURCES += \
    resources/resources.qrc
