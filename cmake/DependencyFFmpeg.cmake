# Find FFmpeg
#
# FFmpeg only supports PkgConfig

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswresample libswscale)

if (FFmpeg_FOUND)
    target_link_libraries(${PROJECT_NAME} 
        PkgConfig::FFmpeg
    )
endif()
