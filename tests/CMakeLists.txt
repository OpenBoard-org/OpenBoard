find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Gui Test)

# Unit tests only depend on the tested sources, not on the whole application.
# usage: openboard_add_test(<name> <sources>...)
function(openboard_add_test name)
    add_executable(${name} ${name}.cpp ${ARGN})

    target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/src)

    target_link_libraries(${name} PRIVATE
        Qt${QT_VERSION}::Core
        Qt${QT_VERSION}::Gui
        Qt${QT_VERSION}::Test
    )

    add_test(NAME ${name} COMMAND ${name})
endfunction()

openboard_add_test(tst_UBShapeRecognizer
    ${PROJECT_SOURCE_DIR}/src/frameworks/UBShapeRecognizer.cpp
)

openboard_add_test(tst_UBStrokeHoldDetector
    ${PROJECT_SOURCE_DIR}/src/frameworks/UBStrokeHoldDetector.cpp
    ${PROJECT_SOURCE_DIR}/src/frameworks/UBStrokeHoldDetector.h
)
