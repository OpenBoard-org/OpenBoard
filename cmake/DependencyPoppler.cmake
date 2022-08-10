# Find poppler

find_package(Poppler QUIET COMPONENTS Core Cpp)

if (Poppler_FOUND)
    target_link_libraries(${PROJECT_NAME}
        Poppler::Core
        Poppler::Cpp
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Poppler REQUIRED IMPORTED_TARGET poppler poppler-cpp)

    if (Poppler_FOUND)
        target_link_libraries(${PROJECT_NAME} 
            PkgConfig::Poppler
        )
    endif()

endif()
