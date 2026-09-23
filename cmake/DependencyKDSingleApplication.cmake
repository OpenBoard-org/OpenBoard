# Find kdsingleapplication

if(${QT_VERSION} STREQUAL "6")
    find_package(KDSingleApplication-qt6 CONFIG REQUIRED)

    if(KDSingleApplication-qt6_FOUND)
        target_link_libraries(${PROJECT_NAME}
            KDAB::kdsingleapplication
        )
    endif()
else()
    message(FATAL_ERROR "KDSingleApplication not supported for Qt Version ${QT_VERSION}")
endif()
