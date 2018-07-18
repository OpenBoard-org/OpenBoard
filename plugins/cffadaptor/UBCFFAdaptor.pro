#-------------------------------------------------
#
# Project created by QtCreator 2012-02-14T13:30:14
#
#-------------------------------------------------

TARGET = CFF_Adaptor
TEMPLATE = lib

win32: SUB_DIR = win32
macx: SUB_DIR = macx
linux-g++: SUB_DIR = linux
linux-g++-32: SUB_DIR = linux
linux-g++-64: SUB_DIR = linux

THIRD_PARTY_PATH = ../../../Sankore-ThirdParty
QUAZIP_DIR   = "$$PWD/../../../Sankore-ThirdParty/quazip/quazip-0.3"

BUILD_DIR = build/$$SUB_DIR
CONFIG(debug, debug|release):BUILD_DIR = $$BUILD_DIR/debug
CONFIG(release, debug|release) {
   BUILD_DIR = $$BUILD_DIR/release
   CONFIG += warn_off
}

win32{
    CONFIG += dll
}

OBJECTS_DIR = $$BUILD_DIR/objects
MOC_DIR = $$BUILD_DIR/moc
DESTDIR = $$BUILD_DIR/lib
RCC_DIR = $$BUILD_DIR/rcc
SANKORE_DIR = ../..
win32:{
    QMAKE_POST_LINK += copy $$replace(DESTDIR,/,\\)\\CFF_Adaptor.dll" $$replace(SANKORE_DIR,/,\\)\\CFF_Adaptor.dll /y"
}
macx:{
    QMAKE_POST_LINK += bash postScript_mac.sh "$$DESTDIR" "$$SANKORE_DIR/$$BUILD_DIR/product/Open-Sankore.app/Contents/MacOS"
}

LIBS     += "-L$$THIRD_PARTY_PATH/quazip/lib/$$SUB_DIR" "-lquazip"

QT       += xml xmlpatterns core
QT       += gui
QT       += svg


DEFINES += UBCFFADAPTOR_LIBRARY
DEFINES += NO_THIRD_PARTY_WARNINGS

INCLUDEPATH += src

DEPENDPATH  += $$THIRD_PARTY_PATH/quazip/
INCLUDEPATH += $$THIRD_PARTY_PATH/quazip/
include($$THIRD_PARTY_PATH/quazip/quazip.pri)


SOURCES += \
    src/UBCFFAdaptor.cpp

HEADERS +=\
    $$PWD/../../src/globals/UBGlobals.h \
    src/UBCFFAdaptor.h \
    src/UBCFFAdaptor_global.h \
    src/UBCFFConstants.h

RESOURCES += \
    resources/resources.qrc