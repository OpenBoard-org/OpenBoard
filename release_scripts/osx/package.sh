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
set -o pipefail


SCRIPT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_PATH/../.."


APPLICATION_NAME="OpenBoard"
BASE_QT_DIR=/Users/dev/Qt/6.9.3/macos
# Executables
QMAKE=$BASE_QT_DIR/bin/qmake
MACDEPLOYQT=$BASE_QT_DIR/bin/macdeployqt
DMGUTIL="$PROJECT_ROOT/release_scripts/osx/refnum/dmgutil/dmgutil.pl"
DSYMUTIL=/usr/bin/dsymutil
STRIP=/usr/bin/strip
PLISTBUDDY=/usr/libexec/PlistBuddy
ICEBERG=/usr/local/bin/freeze
LRELEASE=$BASE_QT_DIR/bin/lrelease

# Directories
BUILD_DIR="$PROJECT_ROOT/build/macx/release"
PRODUCT_DIR="$BUILD_DIR/product"
BASE_QT_TRANSLATIONS_DIRECTORY=$BASE_QT_DIR/translations


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

trap "defaults write org.oe-f.OpenBoard.release Running -bool NO" EXIT

notify "Running OpenBoard release script (`date`)"

cd $PROJECT_ROOT

script_is_running=`defaults read org.oe-f.OpenBoard.release Running 2>/dev/null`
if [[ $? -eq 0 ]] && [[ "$script_is_running" = "1" ]]; then
    trap EXIT
    abort "another release script already running"
fi
defaults write org.oe-f.OpenBoard.release Running -bool YES

# Check for executables
checkExecutable "$QMAKE"
checkExecutable "$MACDEPLOYQT"
checkExecutable "$DMGUTIL"
checkExecutable "$DSYMUTIL"
checkExecutable "$STRIP"
checkExecutable "$PLISTBUDDY"
checkExecutable "$ICEBERG"
checkExecutable "$LRELEASE"

DMG="$APPLICATION_NAME.dmg"

VOLUME="/Volumes/$APPLICATION_NAME"
APP="$PRODUCT_DIR/$APPLICATION_NAME.app"

VERSION_FILE="$PROJECT_ROOT/version.txt"

if [ ! -f "$VERSION_FILE" ]; then
    abort "version.txt not found"
fi

VERSION_MAJ=$(awk -F= '/VERSION_MAJ/   {gsub(/ /,"",$2); sub(/#.*/,"",$2); print $2}' "$VERSION_FILE")
VERSION_MIN=$(awk -F= '/VERSION_MIN/   {gsub(/ /,"",$2); sub(/#.*/,"",$2); print $2}' "$VERSION_FILE")
VERSION_PATCH=$(awk -F= '/VERSION_PATCH/ {gsub(/ /,"",$2); sub(/#.*/,"",$2); print $2}' "$VERSION_FILE")
VERSION_TYPE=$(awk -F= '/VERSION_TYPE/  {gsub(/ /,"",$2); sub(/#.*/,"",$2); print $2}' "$VERSION_FILE")
VERSION_BUILD=$(awk -F= '/VERSION_BUILD/ {gsub(/ /,"",$2); sub(/#.*/,"",$2); print $2}' "$VERSION_FILE")

FULL_VERSION="$VERSION_MAJ.$VERSION_MIN.$VERSION_PATCH${VERSION_TYPE}-${VERSION_BUILD}"

DSYM_NAME="$APPLICATION_NAME-$FULL_VERSION.dSYM"
DSYM="$PRODUCT_DIR/$DSYM_NAME"
GSYM_i386="$PRODUCT_DIR/$APPLICATION_NAME i386.sym"
INFO_PLIST="$APP/Contents/Info.plist"

if [ "$1" == "pkg" ]; then
    BASE_ICEBERG_CONFIG_FILE="$SCRIPT_PATH/$APPLICATION_NAME.packproj"
    #copy the standard file for working with
    ICEBERG_CONFIG_FILE="$APPLICATION_NAME-working.packproj"
    cp -r $BASE_ICEBERG_CONFIG_FILE $ICEBERG_CONFIG_FILE
    # set version information
    $PLISTBUDDY -c "Set :Hierarchy:Attributes:Settings:Description:International:IFPkgDescriptionVersion $VERSION" "$ICEBERG_CONFIG_FILE"
    $PLISTBUDDY -c "Set :Hierarchy:Attributes:Settings:Display\ Information:CFBundleShortVersionString $VERSION" "$ICEBERG_CONFIG_FILE"
    $PLISTBUDDY -c "Set :Hierarchy:Attributes:Settings:Version:IFMajorVersion `echo $VERSION | awk 'BEGIN { FS = "." }; { print $1 }'`" "$ICEBERG_CONFIG_FILE"
    $PLISTBUDDY -c "Set :Hierarchy:Attributes:Settings:Version:IFMinorVersion `echo $VERSION | awk 'BEGIN { FS = "." }; { print $2 }'`" "$ICEBERG_CONFIG_FILE"


    PRODUCT_DIR="install/mac/"

    if [ ! -d "${PRODUCT_DIR}" ]; then
	mkdir -p "${PRODUCT_DIR}"
    fi
    $ICEBERG $ICEBERG_CONFIG_FILE 

    #clean up mess
    rm -rf $ICEBERG_CONFIG_FILE

    exit 0
fi

notify "Creating a resizable DMG (2 GB) volume"
# Unmount if already there
umount "$VOLUME" 2> /dev/null

# Create a 2Go sparse image
SPARSE_IMAGE="${DMG%.dmg}.sparseimage"
hdiutil create -size 2g -type SPARSE -fs HFS+ -volname "$APPLICATION_NAME" "$SPARSE_IMAGE"

# attach sparse image to volume
hdiutil attach "$SPARSE_IMAGE" -mountpoint "$VOLUME"

notify "Copying app to volume"
rsync -a "$APP" "$VOLUME"/
RSYNC_STATUS=$?

if [ $RSYNC_STATUS -ne 0 ]; then
    error "Copy failed (likely out of space). DMG will remain mounted for inspection."
    error "Volume contents:"
    ls -lh "$VOLUME"
    error "Free space:"
    df -h "$VOLUME"
    exit 1
fi
ln -s /Applications "$VOLUME"

$DMGUTIL --set --iconsize=96 --toolbar=false --icon=resources/macx/OpenBoard.icns "$VOLUME"
$DMGUTIL --set --x=20 --y=60 --width=580 --height=440 "$VOLUME"
$DMGUTIL --set --x=180 --y=120 "$VOLUME/`basename \"$APP\"`"
$DMGUTIL --set --x=400 --y=120 "$VOLUME/Applications"

if mount | grep -q "$VOLUME"; then
    notify "Closing DMG..."
    $DMGUTIL --close --volume="$APPLICATION_NAME" "$SPARSE_IMAGE"
else
    warn "DMG volume already unmounted or copy failed — skipping close"
fi

# Convert sparseImage to compressed DMG
DMG="$PRODUCT_DIR/$APPLICATION_NAME-$FULL_VERSION.dmg"
notify "Converting sparseimage to compressed DMG..."
hdiutil convert "$SPARSE_IMAGE" -format UDZO -o "$DMG"

# Delete sparseImage
rm "$SPARSE_IMAGE"

PRODUCT_DIR="install/mac/"

if [ ! -d "${PRODUCT_DIR}" ]; then
    mkdir -p "${PRODUCT_DIR}"
fi

mv "$DMG" "${PRODUCT_DIR}"

notify "$APPLICATION_NAME is now packaged. You can submit this dmg file to notarization using notarize.sh"

exit 0

