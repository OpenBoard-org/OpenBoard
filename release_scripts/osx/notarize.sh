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
read -s -p "Password for $USER is required: " PASSWORD
printf "\n"

cd $INSTALL_DIR;

notify "================================================"
notify "Submitting $APPLICATION_NAME for notarization..."
notify "================================================"

NOTARIZE_APP_OUTPUT=$(2>&1 xcrun altool --notarize-app -f OpenBoard.dmg --primary-bundle-id ch.openboard.id -u "$USER" -p "$PASSWORD")
SUBMISSION_ID=$(echo "$NOTARIZE_APP_OUTPUT" | grep "RequestUUID" | sed -Ee "s|.*= (.*)$|\1|")


if [[ "$SUBMISSION_ID" == "" ]]; then
    NOTARIZE_APP_ERROR_LOG_NAME="notarization-submission-error.log"
    NOTARIZE_APP_ERROR_LOG_PATH="$SCRIPT_PATH/$NOTARIZE_APP_ERROR_LOG_NAME"

    echo "$NOTARIZE_APP_OUTPUT" > "$NOTARIZE_APP_ERROR_LOG_PATH"

    warn "================================================"
    warn "Submission of $APPLICATION_NAME failed !"
    warn "See $NOTARIZE_APP_ERROR_LOG_NAME for details."
    warn "================================================"

    abort "$APPLICATION_NAME notarization failed"
else
    NOTARIZE_APP_SUCCESS_LOG_NAME="notarization-submission-success.log"
    NOTARIZE_APP_SUCCESS_LOG_PATH="$SCRIPT_PATH/$NOTARIZE_APP_SUCCESS_LOG_NAME"

    echo "$OUTPUT" > "$NOTARIZE_APP_SUCCESS_LOG_PATH"

    notify "================================================"
    notify "Submission of  $APPLICATION_NAME succeed."
    notify "See $NOTARIZE_APP_SUCCESS_LOG_NAME for details."
    notify "================================================"

    notify "================================================"
    notify "Checking status of notarization (RequestUUID = $SUBMISSION_ID)"
    notify "================================================"

    while true; do
        NOTARIZATION_INFO_OUTPUT=$(2>&1 xcrun altool --notarization-info "$SUBMISSION_ID" -u "$USER" -p "$PASSWORD")
        STATUS=$(echo "$NOTARIZATION_INFO_OUTPUT" | grep "Status:" | sed -Ee "s|.*: (.*)$|\1|" )
        notify "notarization status: $STATUS"
        if [[ "$STATUS" != "in progress" ]]; then
            break
        fi
        sleep 30
    done

    if [[ $STATUS == "success" ]]; then
        NOTARIZATION_SUCCESS_LOG_NAME="notarization-success.log"
        NOTARIZATION_SUCCESS_LOG="$SCRIPT_PATH/$NOTARIZATION_SUCCESS_LOG_NAME"
        echo "$NOTARIZATION_INFO_OUTPUT" > "$NOTARIZATION_SUCCESS_LOG"

        notify "================================================"
        notify "$APPLICATION_NAME was notarized sucessfully. You can now distribute it."
        notify "See $NOTARIZATION_SUCCESS_LOG_NAME for details."
        notify "================================================"
    else
        NOTARIZATION_ERROR_LOG_NAME="notarization-error.log"
        NOTARIZATION_ERROR_LOG="$SCRIPT_PATH/$NOTARIZATION_ERROR_LOG_NAME"
        echo "$NOTARIZATION_INFO_OUTPUT" > "$NOTARIZATION_ERROR_LOG"

        warn "================================================"
        warn "$APPLICATION_NAME could not be notarized."
        warn "See $NOTARIZATION_ERROR_LOG_NAME for details."
        warn "================================================"

        abort "$APPLICATION_NAME notarization failed"
    fi
fi
exit 0

