TARGET   = "pdf2image.bin"
TEMPLATE  = app
CONFIG   += console warn_off

UNIBOARD_SRC = ../../src
DESTDIR     = "build/Product"
OBJECTS_DIR = "build/objects"
MOC_DIR     = "build/moc"
RCC_DIR     = "build/rcc"
UI_DIR      = "build/ui"

PDF_FONTS.path = $$DESTDIR/resources
PDF_FONTS.files = ../../resources/fonts

SCRIPT.path = $$DESTDIR
SCRIPT.files = resources/**

QT_CORE.path = $$DESTDIR
QT_CORE.files = /home/mnemis/qtsdk-2009.02/qt/lib/libQtCore.so.4.5.1

QT_GUI.path = $$DESTDIR/
QT_GUI.files = /home/mnemis/qtsdk-2009.02/qt/lib/libQtGui.so.4.5.1

SOURCES = pdf2image.cpp \
          core/UBPlatformUtils.cpp \
          $$UNIBOARD_SRC/pdf/PDFRenderer.cpp \
          $$UNIBOARD_SRC/pdf/XPDFRenderer.cpp

HEADERS = $$UNIBOARD_SRC/pdf/PDFRenderer.h \
          $$UNIBOARD_SRC/pdf/XPDFRenderer.h

include(../../thirdparty/libs.pri)

INCLUDEPATH += $$UNIBOARD_SRC/pdf

macx {
    CONFIG -= app_bundle
}

INSTALLS = PDF_FONTS QT_CORE QT_GUI SCRIPT
