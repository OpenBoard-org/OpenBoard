TEMPLATE  = lib
CONFIG   += staticlib release warn_off

DESTDIR = "lib"

win32 {

    CONFIG += qaxcontainer
    
    # The folowing COM wrappers are generated from dumcpp
    #
    # need to register first the reply ocx regsvr32 ocx\WRS970.ocx
    #
    
    REG = $$system(regsvr32 ocx\WRS970.ocx)
    TYPELIBS = $$system(dumpcpp ocx\WRS970.ocx)
   
    SOURCES  += wrs970.cpp
        
    HEADERS  += wrs970.h
                    
}


