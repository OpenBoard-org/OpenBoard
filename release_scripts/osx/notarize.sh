#!/bin/bash
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

SCRIPT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_PATH/../.."


APPLICATION_NAME="OpenBoard"
BASE_QT_DIR=~/Qt/5.15.2/clang_64
# Executables
QMAKE=$BASE_QT_DIR/bin/qmake
MACDEPLOYQT=$BASE_QT_DIR/bin/macdeployqt
CODESIGN=/usr/bin/codesign
DMGUTIL="$PROJECT_ROOT/release_scripts/osx/refnum/dmgutil/dmgutil.pl"
DSYMUTIL=/usr/bin/dsymutil
STRIP=/usr/bin/strip
PLISTBUDDY=/usr/libexec/PlistBuddy
ICEBERG=/usr/local/bin/freeze
LRELEASE=$BASE_QT_DIR/bin/lrelease
USER=$1
TEAMID=$2

# Directories
BUILD_DIR="$PROJECT_ROOT/build/macx/release"
PRODUCT_DIR="$BUILD_DIR/product"
RESOURCES_DIR="$PROJECT_ROOT/resources"
MACX_RESOURCES_DIR="$RESOURCES_DIR/macx"
BASE_QT_TRANSLATIONS_DIRECTORY=$BASE_QT_DIR/translations
INSTALL_DIR="$PROJECT_ROOT/install/mac"

function notify {
    GROWLNOTIFY=`which growlnotify`
    if [ -x "$GROWLNOTIFY" ]; then
        $GROWLNOTIFY --name OpenBoard-build --iconpath /Developer/Applications/Xcode.app --message "$1" "OpenBoard"
    fi
    printf "\033[48;5;120m--->\033[0m $1\n"
}

function warn {
    printf "\033[48;5;178m--->\033[0m $1\n"
}

function error
{
    printf "\033[48;5;160;38;5;15m--->\033[0m $1\n"
}

function abort {
    error "$1"
    exit 1
}

function checkExecutable {
    if [ ! -x "$1" ]; then
        abort "$1 not found"
    fi
}

notify "================================================"
notify "=============== NOTARIZATION ==================="
notify "================================================"
printf "\n"

cd $INSTALL_DIR;

notify "================================================"
notify "Submitting $APPLICATION_NAME for notarization..."
notify "================================================"

echo $(2>&1 xcrun notarytool submit --wait --apple-id "$USER" --team-id "$TEAMID" OpenBoard.dmg)

exit 0

