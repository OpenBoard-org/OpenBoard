# Fetch xpdf

include(FetchContent)

if(QT_VERSION STREQUAL "5")
    set(CMAKE_DISABLE_FIND_PACKAGE_Qt6Widgets TRUE)
    set(CMAKE_DISABLE_FIND_PACKAGE_Qt4Widgets TRUE)
elseif(QT_VERSION STREQUAL "6")
    set(CMAKE_DISABLE_FIND_PACKAGE_Qt5Widgets TRUE)
    set(CMAKE_DISABLE_FIND_PACKAGE_Qt4Widgets TRUE)
endif()

set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

FetchContent_Declare(
    Xpdf
    URL https://dl.xpdfreader.com/xpdf-4.04.tar.gz
)

FetchContent_MakeAvailable(
    Xpdf
)
# this does not work. Xpdf does not build a library for reuse, it only builds executable tools
target_link_libraries(${PROJECT_NAME}
    goo
    fofi
    splash
)
