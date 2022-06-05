HEADERS += src/desktop/UBDesktopPalette.h \
           src/desktop/UBDesktopAnnotationController.h \
           src/desktop/UBCustomCaptureWindow.h \
		   src/desktop/UBDesktopPropertyPalette.h

SOURCES += src/desktop/UBDesktopPalette.cpp \
           src/desktop/UBDesktopAnnotationController.cpp \
           src/desktop/UBCustomCaptureWindow.cpp \
    	   src/desktop/UBDesktopPropertyPalette.cpp

win32 { 
    HEADERS +=
    SOURCES +=
}

macx:OBJECTIVE_SOURCES += src/desktop/UBWindowCapture_mac.mm

linux-g++:SOURCES +=
linux-g++-32:SOURCES +=
linux-g++-64:SOURCES +=
