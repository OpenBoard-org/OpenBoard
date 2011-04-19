TEMPLATE  = lib
CONFIG   += staticlib release warn_off

DESTDIR = "lib"

win32 {

    CONFIG += qaxcontainer
    
    # The folowing COM wrappers are generated from dumcpp
    #
    TYPELIBS = $$system(dumpcpp -getfile {91493440-5A91-11CF-8700-00AA0060263B})
    TYPELIBS += $$system(dumpcpp -getfile {2DF8D04C-5BFA-101B-BDE5-00AA0044DE52})
   
    SOURCES  += msppt.cpp \
                mso.cpp
        
    HEADERS  += msppt.h \
                mso.h
                
}


