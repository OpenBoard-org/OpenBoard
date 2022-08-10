#
# Define LINUX and MACOS platform specifiers
# Set platform specific variables
#

if(UNIX)
    if(APPLE)
        set(MACOS 1)
    else()
        set(LINUX 1)
    endif()
endif()

if(MACOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
endif()
