
INCLUDEPATH  += "$$PWD/../zlib/1.2.3/include"

HEADERS += crypt.h \
           ioapi.h \
           quazip.h \
           quazipfile.h \
           quazipfileinfo.h \
           quazipnewinfo.h \
           unzip.h \
           zip.h

SOURCES += ioapi.c \
           quazip.cpp \
           quazipfile.cpp \
           quazipnewinfo.cpp \
           unzip.c \
           zip.c


macx {

   LIBS += -lz
}

win32 {
    
    LIBS     += "-L$$PWD/../zlib/1.2.3/lib" "-lzlib"
   
}

linux-g++ {

   LIBS += -lz

}