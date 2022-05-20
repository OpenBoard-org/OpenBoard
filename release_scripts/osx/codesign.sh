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

IDENTITY=$1

CODESIGN=/usr/bin/codesign

BUILD_DIR="$PROJECT_ROOT/build/macx/release"
PRODUCT_DIR="$BUILD_DIR/product"
RESOURCES_DIR="$PROJECT_ROOT/resources"
MACX_RESOURCES_DIR="$RESOURCES_DIR/macx"
INSTALL_DIR="$PROJECT_ROOT/install/mac"

APPLICATION_NAME="OpenBoard"
APPLICATION_DOT_APP="$APPLICATION_NAME.app"
APPLICATION_DIR="$PRODUCT_DIR/$APPLICATION_DOT_APP"
APPLICATION_CONTENTS_DIR="$APPLICATION_DIR/Contents"
APPLICATION_RESOURCES_DIR="$APPLICATION_CONTENTS_DIR/Resources"

IMPORTER_NAME="OpenBoardImporter"
IMPORTER_DOT_APP="$IMPORTER_NAME.app"
IMPORTER_DIR="$APPLICATION_RESOURCES_DIR/$IMPORTER_DOT_APP"


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

checkExecutable $CODESIGN

function signImporter
{
    notify "signing $IMPORTER_NAME..."
    if [ ! -e ${IMPORTER_DIR} ]; then
        abort "${IMPORTER_DIR} not found"
    fi

    cd $APPLICATION_RESOURCES_DIR

    $CODESIGN --force --deep -o runtime --timestamp --verbose=4 -s "$IDENTITY" --digest-algorithm=sha1,sha256 "$IMPORTER_DOT_APP"
    cd -
}

function signOpenBoard
{
    notify "signing $APPLICATION_NAME..."
    if [ ! -e ${APPLICATION_DIR} ]; then
        abort "${APPLICATION_DIR} not found"
    fi

    cd $PRODUCT_DIR

    $CODESIGN --force --deep -o runtime --timestamp --entitlements "$MACX_RESOURCES_DIR/Entitlements.plist" --verbose=4 -s "$IDENTITY" --digest-algorithm=sha1,sha256 "$APPLICATION_DOT_APP"
    cd -
}

signImporter

signOpenBoard

notify "$APPLICATION_NAME is now signed. You can now package OpenBoard using the script 'package.sh'"

exit 0

