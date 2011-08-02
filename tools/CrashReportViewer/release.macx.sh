# --------------------------------------------------------------------
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ---------------------------------------------------------------------

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

