# Fetch QuaZip

include(FetchContent)

if(QT_VERSION STREQUAL "5")
    set(QUAZIP_QT_MAJOR_VERSION 5 CACHE STRING "QuaZip Qt version")
elseif(QT_VERSION STREQUAL "6")
    set(QUAZIP_QT_MAJOR_VERSION 6)
endif()

FetchContent_Declare(
    QuaZip
    GIT_REPOSITORY https://github.com/stachenov/quazip.git
    GIT_TAG        v1.3
)

FetchContent_MakeAvailable(
    QuaZip
)

target_link_libraries(${PROJECT_NAME}
    QuaZip::QuaZip
)
