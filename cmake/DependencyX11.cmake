# Find FFmpeg

find_package(X11 QUIET)

if (X11_FOUND)
    target_link_libraries(${PROJECT_NAME}
        X11
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED x11)

    if (X11_FOUND)
        target_link_libraries(${PROJECT_NAME} 
            PkgConfig::X11
        )
    endif()
endif()
