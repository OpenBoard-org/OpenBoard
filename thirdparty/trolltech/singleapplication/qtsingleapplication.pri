INCLUDEPATH	+= $$PWD
DEPENDPATH      += $$PWD
HEADERS		+= $$PWD/qtsingleapplication.h
SOURCES		+= $$PWD/qtsingleapplication.cpp
win32:SOURCES	+= $$PWD/qtsingleapplication_win.cpp
mac:SOURCES     += $$PWD/qtsingleapplication_mac.cpp
mac:LIBS += -framework CoreFoundation
win32:LIBS += user32.lib
unix:!mac {
	SOURCES	+= $$PWD/qtsingleapplication_x11.cpp
	CONFIG	+= x11
}

win32:contains(TEMPLATE, lib):contains(CONFIG, shared) {
    DEFINES += QT_QTSINGLEAPPLICATION_EXPORT=__declspec(dllexport)
}
