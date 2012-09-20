HEADERS += src/desktop/UBDesktopPalette.h \
           src/desktop/UBDesktopAnnotationController.h \
           src/desktop/UBCustomCaptureWindow.h \
           src/desktop/UBWindowCapture.h \
		   src/desktop/UBDesktopPropertyPalette.h

SOURCES += src/desktop/UBDesktopPalette.cpp \
           src/desktop/UBDesktopAnnotationController.cpp \
           src/desktop/UBCustomCaptureWindow.cpp \
    	   src/desktop/UBDesktopPropertyPalette.cpp

win32 { 
    HEADERS += src/desktop/UBWindowCaptureDelegate_win.h
    SOURCES += src/desktop/UBWindowCapture_win.cpp \
               src/desktop/UBWindowCaptureDelegate_win.cpp
}

macx:OBJECTIVE_SOURCES += src/desktop/UBWindowCapture_mac.mm

linux-g++:SOURCES += src/desktop/UBWindowCapture_linux.cpp
linux-g++-32:SOURCES += src/desktop/UBWindowCapture_linux.cpp
linux-g++-64:SOURCES += src/desktop/UBWindowCapture_linux.cpp
