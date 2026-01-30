# Find Pipewire
#
# Pipewire only supports PkgConfig

find_package(PkgConfig REQUIRED)
pkg_check_modules(Pipewire REQUIRED IMPORTED_TARGET libpipewire-0.3)

if (Pipewire_FOUND)
    target_link_libraries(${PROJECT_NAME} 
        PkgConfig::Pipewire
    )
endif()
