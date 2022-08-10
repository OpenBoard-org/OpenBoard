# Find supported Qt version

set(QT_COMPONENTS
    Concurrent
    DBus
    Multimedia
    MultimediaWidgets
    Network
    PrintSupport
    Svg
    UiTools
    WebEngineWidgets
    Xml
)

if(QT_VERSION STREQUAL "5")
    find_package(Qt5 5.12 REQUIRED COMPONENTS
        ${QT_COMPONENTS}
        LinguistTools
    )
elseif(QT_VERSION STREQUAL "6")
    find_package(Qt6 6.2 REQUIRED COMPONENTS
        ${QT_COMPONENTS}
        SvgWidgets
        LinguistTools
    )

    target_link_libraries(${PROJECT_NAME}
        Qt6::SvgWidgets
    )
else()
    message(FATAL_ERROR "Qt Version ${QT_VERSION} not supported")
endif()

list(TRANSFORM QT_COMPONENTS PREPEND Qt::)

target_link_libraries(${PROJECT_NAME}
    ${QT_COMPONENTS}
)
