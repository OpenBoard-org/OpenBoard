#
# Version
#

file(READ "version.txt" version)

if(version MATCHES "VERSION_MAJ *= *([0-9]+)")
    set(VERSION_MAJ ${CMAKE_MATCH_1})
else()
    set(VERSION_ERROR "VERSION_MAJ")
endif()

if(version MATCHES "VERSION_MIN *= *([0-9]+)")
    set(VERSION_MIN ${CMAKE_MATCH_1})
else()
    set(VERSION_ERROR "VERSION_MIN")
endif()

if(version MATCHES "VERSION_PATCH *= *([0-9]+)")
    set(VERSION_PATCH ${CMAKE_MATCH_1})
else()
    set(VERSION_ERROR "VERSION_PATCH")
endif()

if(version MATCHES "VERSION_TYPE *= *(a|b|rc|r)")
    set(VERSION_TYPE ${CMAKE_MATCH_1})
else()
    set(VERSION_ERROR "VERSION_TYPE")
endif()

if(version MATCHES "VERSION_BUILD *= *([0-9]+)")
    set(VERSION_BUILD ${CMAKE_MATCH_1})
elseif(NOT VERSION_TYPE STREQUAL "r")
    set(VERSION_ERROR "VERSION_BUILD")
endif()

if (VERSION_ERROR)
    message(FATAL_ERROR "Error parsing version at " ${VERSION_ERROR})
endif()

set(VERSION_NUMBER ${VERSION_MAJ}.${VERSION_MIN}.${VERSION_PATCH})

if(VERSION_TYPE STREQUAL "r")
    set(VERSION "${VERSION_NUMBER}")
else()
    set(VERSION "${VERSION_NUMBER}-${VERSION_TYPE}.${VERSION_BUILD}")
endif()

message(STATUS "Version " ${VERSION})
