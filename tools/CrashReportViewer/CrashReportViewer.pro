TEMPLATE = app
VERSION = 1.0
TARGET = "Crash Report Viewer"

DESTDIR     = "build/macx"
OBJECTS_DIR = "build/macx/objects"
MOC_DIR     = "build/macx/moc"
RCC_DIR     = "build/macx/rcc"
UI_DIR      = "build/macx/ui"

INCLUDEPATH += ../../thirdparty/google-breakpad/r318/include/google_breakpad
LIBS += "-L../../thirdparty/google-breakpad/r318/lib/macx" "-lbreakpad"

SOURCES      = main.cpp \
               CrashWindow.cpp \
               CrashReporterSymbolSupplier.cpp

HEADERS      += CrashWindow.h \
                CrashReporterSymbolSupplier.h

FORMS        += CrashReportViewer.ui

QMAKE_INFO_PLIST = Info.plist

ICON.files = "Bug.icns"
ICON.path  = "Contents/Resources"

QMAKE_BUNDLE_DATA = "ICON"
