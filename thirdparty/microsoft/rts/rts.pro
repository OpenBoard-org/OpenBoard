TEMPLATE  = lib
CONFIG   += static release warn_off

DESTDIR = "lib"

win32 {
  
    SOURCES  += src/RTSCOM_i.c
        
    HEADERS  += include/RTSCOM.h
}


