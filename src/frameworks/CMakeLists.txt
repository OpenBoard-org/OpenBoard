target_sources(openboard PRIVATE
    UBBackgroundLoader.cpp
    UBBackgroundLoader.h
    UBBase32.cpp
    UBBase32.h
    UBBlockingBuffer.cpp
    UBBlockingBuffer.h
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

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    target_sources(openboard PRIVATE
        linux/UBScreenCastDesktopPortalWrapper.cpp
        linux/UBScreenCastDesktopPortalWrapper.h
        linux/UBScreenshotDesktopPortalWrapper.cpp
        linux/UBScreenshotDesktopPortalWrapper.h
        linux/UBDesktopPortalTokenGenerator.cpp
        linux/UBDesktopPortalTokenGenerator.h
        linux/UBPipewireSink.cpp
        linux/UBPipewireSink.h
        UBPlatformUtils_linux.cpp
    )
endif()
