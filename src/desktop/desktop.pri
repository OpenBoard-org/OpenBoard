HEADERS += src/desktop/UBDesktopPalette.h \
           src/desktop/UBDesktopAnnotationController.h \
           src/desktop/UBCustomCaptureWindow.h \
           src/desktop/UBWindowCapture.h \
           src/desktop/UBDesktopToolsPalette.h \
    src/desktop/UBDesktopPenPalette.h \
    src/desktop/UBDesktopMarkerPalette.h \
    src/desktop/UBDesktopEraserPalette.h

SOURCES += src/desktop/UBDesktopPalette.cpp \
           src/desktop/UBDesktopAnnotationController.cpp \
           src/desktop/UBCustomCaptureWindow.cpp \
           src/desktop/UBDesktopToolsPalette.cpp \
    src/desktop/UBDesktopPenPalette.cpp \
    src/desktop/UBDesktopMarkerPalette.cpp \
    src/desktop/UBDesktopEraserPalette.cpp

win32 { 
    HEADERS += src/desktop/UBWindowCaptureDelegate_win.h
    SOURCES += src/desktop/UBWindowCapture_win.cpp \
               src/desktop/UBWindowCaptureDelegate_win.cpp
}

macx:SOURCES += src/desktop/UBWindowCapture_mac.mm

linux-g++:SOURCES += src/desktop/UBWindowCapture_linux.cpp
