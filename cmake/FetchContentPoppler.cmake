# Fetch libpoppler

include(FetchContent)

FetchContent_Declare(
    Poppler
    GIT_REPOSITORY https://gitlab.freedesktop.org/poppler/poppler.git
    GIT_TAG        poppler-22.08.0
)

FetchContent_MakeAvailable(
    Poppler
)

target_link_libraries(${PROJECT_NAME}
    Poppler::Core
    Poppler::Cpp
)
