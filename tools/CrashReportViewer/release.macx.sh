#!/bin/bash

# Qt base directory
QTDIR=/usr/local/Trolltech/Qt-4.6.1

# Executables
MACDEPLOYQT="$QTDIR/bin/macdeployqt"
QMAKE="$QTDIR/bin/qmake"

# Directories
BUILD_DIR="build/macx"

function abort {
    printf "\033[31merror:\033[0m $1\n"
    exit 1
}

function warn {
    if [ $testMode == 1 ]; then
        printf "\033[33mwarning:\033[0m $1\n"
    else
        abort "$1"
    fi
}

function checkExecutable {
    if [ ! -x "$1" ]; then
        abort "$1 not found"
    fi
}

# Check for executables
checkExecutable "$MACDEPLOYQT"
checkExecutable "$QMAKE"


# delete the build directory
rm -rf "$BUILD_DIR"

# generate Makefile (makefile.macx)
$QMAKE -spec macx-g++ -config release

# build
make

if [ $? != 0 ]; then
    abort "compilation failed"
fi

APP="$BUILD_DIR/Crash Report Viewer.app"

# bundle Qt Frameorks (/Library/Framworks) into the app bundle
$MACDEPLOYQT "$APP" 

