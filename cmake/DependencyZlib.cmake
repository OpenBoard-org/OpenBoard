# Find zlib

find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME}
    z
)
