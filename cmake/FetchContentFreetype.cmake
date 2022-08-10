# Fetch Freetype

include(FetchContent)

FetchContent_Declare(
    Freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG        VER-2-12-1
)

FetchContent_MakeAvailable(
    Freetype
)
