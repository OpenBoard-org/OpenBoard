TEMPLATE  = lib
CONFIG   += staticlib release warn_off

DESTDIR = "lib"

win32 {

    CONFIG += qaxcontainer
    
    # The folowing COM wrappers are generated from dumcpp
    #
    # need to register first the reply ocx regsvr32 ocx\Reply2005.ocx
    #
    
    REG = $$system(regsvr32 ocx\Reply2005.ocx)
    TYPELIBS = $$system(dumpcpp ocx\Reply2005.ocx)
   
    SOURCES  += replyxcontrol1.cpp
        
    HEADERS  += replyxcontrol1.h
                    
}


