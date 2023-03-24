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
BASE_QT_DIR=/opt/local/libexec/qt5
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

function addQtTranslations {
for eachTranslation in `ls $BASE_QT_TRANSLATIONS_DIRECTORY/qt_??.qm`
do
    # looking fo the language code for each qt translation file
    languageCode=`echo $eachTranslation | sed 's/.*qt_\(.*\).qm/\1/'`
    basicDir=$PRODUCT_DIR/$APPLICATION_NAME.app/Contents/Resources/
    for eachDirectory in `ls $basicDir`
    do
        # looping through the OpenBoard availables languages
        directoryLanguageCode=`echo $eachDirectory | sed 's/\(.*\)\.lproj/\1/'`
        if [ ! -z $directoryLanguageCode ]; then
            if [[ $eachDirectory == *".lproj"* && $eachDirectory != "empty.lproj" && $directoryLanguageCode == *$languageCode* ]]; then
                # OpenBoard translation found for qt translation file
                cp $eachTranslation $basicDir/$eachDirectory
                if [ $directoryLanguageCode != $languageCode ]; then
                    # handling fr and fr_CH code.
                    mv $basicDir/$eachDirectory/qt_$languageCode.qm $basicDir/$eachDirectory/qt_$directoryLanguageCode.qm
                fi
            fi
        fi
    done
done

}


function addImporter {
    importerDir="`pwd`/../OpenBoard-Importer"
    importerName="OpenBoardImporter"

    if [ ! -e ${importerDir} ]; then
        abort "${importerDir} not found"
    fi

    cd ${importerDir}
#    git reset --hard
#    git pull
    rm -rf ${importerName}.app
    rm MakeFile*
    rm -rf release
    rm -rf debug
    $QMAKE ${importerName}.pro -spec macx-clang
    make -j4 release
    $MACDEPLOYQT ${importerName}.app 
    cd -
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

addImporter

# delete the build directory
notify "Cleaning ..."
rm -rf "$BUILD_DIR"

# application translations
notify "Generating applications translatons"
$LRELEASE "$APPLICATION_NAME.pro"

# generate Makefiles
notify "Generating Makefile ..."


if [ "$1" == "1010" ]; then
   QMAKE_CMD="$QMAKE \"DEFINES+=OS_NEWER_THAN_OR_EQUAL_TO_1010\" $APPLICATION_NAME.pro -spec macx-clang"
else
   QMAKE_CMD="$QMAKE $APPLICATION_NAME.pro -spec macx-clang"
fi
$QMAKE_CMD

# build
notify "Compiling ..."
make -j4 release

notify "Qt Translations ..."
#$LRELEASE $BASE_QT_TRANSLATIONS_DIRECTORY/translations.pro 
addQtTranslations

cp -R resources/customizations $PRODUCT_DIR/$APPLICATION_NAME.app/Contents/Resources
cp -R $importerDir/$importerName.app $PRODUCT_DIR/$APPLICATION_NAME.app/Contents/Resources

VERSION=`cat "$BUILD_DIR/version"`
if [ ! -f "$BUILD_DIR/version" ]; then
    echo "version not found"
    exit 1
fi

VOLUME="/Volumes/$APPLICATION_NAME"
APP="$PRODUCT_DIR/$APPLICATION_NAME.app"
DSYM_NAME="$APPLICATION_NAME (r$SVN_REVISION).dSYM"
DSYM="$PRODUCT_DIR/$DSYM_NAME"
GSYM_i386="$PRODUCT_DIR/$APPLICATION_NAME i386.sym"
INFO_PLIST="$APP/Contents/Info.plist"

rm -f "$APP/Contents/Resources/empty.lproj"

# set various version infomration in Info.plist
$PLISTBUDDY -c "Set :CFBundleVersion $VERSION" "$INFO_PLIST"
$PLISTBUDDY -c "Set :CFBundleShortVersionString $VERSION" "$INFO_PLIST"
$PLISTBUDDY -c "Set :CFBundleGetInfoString $VERSION" "$INFO_PLIST"

# bundle Qt Frameworks into the app bundle
notify "Bulding frameworks ..."
cd "`pwd`/build/macx/release/product/"
$MACDEPLOYQT "`pwd`/$APPLICATION_NAME.app"
cd -

notify "Extracting debug information ..."
$DSYMUTIL "$APP/Contents/MacOS/$APPLICATION_NAME" -o "$DSYM"
$STRIP -S "$APP/Contents/MacOS/$APPLICATION_NAME"

notify "$APPLICATION_NAME is now built. You can now sign the application using the script 'codesign.sh'."
notify "Usage : sudo ./codesign.sh \"Developer ID Application: ...\""
notify "You can also ignore signing step if you don't intend to distribute this version"
notify "If so, you can directly package OpenBoard using the script 'package.sh'"
notify "Usage : sudo ./package.sh"

exit 0

