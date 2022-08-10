target_sources(openboard PRIVATE
    UBBase32.cpp
    UBBase32.h
    UBCoreGraphicsScene.cpp
    UBCoreGraphicsScene.h
    UBCryptoUtils.cpp
    UBCryptoUtils.h
    UBFileSystemUtils.cpp
    UBFileSystemUtils.h
    UBGeometryUtils.cpp
    UBGeometryUtils.h
    UBPlatformUtils.cpp
    UBPlatformUtils.h
    UBStringUtils.cpp
    UBStringUtils.h
    UBVersion.cpp
    UBVersion.h
)

if(LINUX)
    target_sources(openboard PRIVATE
        UBPlatformUtils_linux.cpp
    )
elseif(WIN32)
    target_sources(openboard PRIVATE
        UBPlatformUtils_win.cpp
    )
elseif(MACOS)
    target_sources(openboard PRIVATE
        UBPlatformUtils_mac.mm
    )
endif()
