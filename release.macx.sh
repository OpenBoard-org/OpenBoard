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


APPLICATION_NAME="OpenBoard"
BASE_TROLLTECH_DIRECTORY=/usr/local/Trolltech/Qt-4.8.0
# Executables
QMAKE=$BASE_TROLLTECH_DIRECTORY/bin/qmake
MACDEPLOYQT=$BASE_TROLLTECH_DIRECTORY/bin/macdeployqt
DMGUTIL="`pwd`/../Sankore-ThirdParty/refnum/dmgutil/dmgutil.pl"
DSYMUTIL=/usr/bin/dsymutil
STRIP=/usr/bin/strip
PLISTBUDDY=/usr/libexec/PlistBuddy
ICEBERG=/usr/local/bin/freeze
LRELEASE=$BASE_TROLLTECH_DIRECTORY/bin/lrelease

# Directories
BUILD_DIR="build/macx/release"
PRODUCT_DIR="$BUILD_DIR/product"
BASE_QT_TRANSLATIONS_DIRECTORY=../Qt4.8/translations

function notify {
    GROWLNOTIFY=`which growlnotify`
    if [ -x "$GROWLNOTIFY" ]; then
        $GROWLNOTIFY --name OpenBoard-build --iconpath /Developer/Applications/Xcode.app --message "$1" "OpenBoard"
    fi
    printf "\033[32m--->\033[0m $1\n"
}

function abort {
    printf "\033[31merror:\033[0m $1\n"
    exit 1
}

function warn {
    abort "$1"
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
    git reset --hard
    git pull
    rm -rf ${importerName}.app
    rm MakeFile*
    rm -rf release
    rm -rf debug
    $QMAKE ${importerName}.pro
    make -j4 release
    $MACDEPLOYQT ${importerName}.app 
    cd -
}

trap "defaults write org.oe-f.OpenBoard.release Running -bool NO" EXIT

notify "Running OpenBoard release script (`date`)"

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

QMAKE_CMD="$QMAKE $APPLICATION_NAME.pro -spec macx-g++"

$QMAKE_CMD

# build
notify "Compiling ..."
make -j4 release

notify "Qt Translations ..."
$LRELEASE $BASE_QT_TRANSLATIONS_DIRECTORY/translations.pro 
addQtTranslations

cp -R resources/customizations $PRODUCT_DIR/$APPLICATION_NAME.app/Contents/Resources
cp -R $importerDir/$importerName.app $PRODUCT_DIR/$APPLICATION_NAME.app/Contents/Resources

VERSION=`cat "$BUILD_DIR/version"`
if [ ! -f "$BUILD_DIR/version" ]; then
    echo "version not found"
    exit 1
else
    notify "Tagging ..."
    LAST_COMMITED_VERSION="`git describe $(git rev-list --tags --max-count=1)`"
    if [ "v$VERSION" != "$LAST_COMMITED_VERSION" ]; then
	echo creating a tag with the version $VERSION
#	git tag -a "v$VERSION" -m "Generated setup for v$VERSION"
#	git push origin --tags
    fi
fi
  
if [ $? != 0 ]; then
    abort "compilation failed"
fi


DMG="$APPLICATION_NAME.dmg"
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
$PLISTBUDDY -c "Set :CFBundleGetInfoString $APPLICATION_NAME" "$INFO_PLIST"

# bundle Qt Frameworks into the app bundle
notify "Bulding frameworks ..."
cd "`pwd`/build/macx/release/product/"
$MACDEPLOYQT "`pwd`/$APPLICATION_NAME.app"
cd -

notify "Extracting debug information ..."
$DSYMUTIL "$APP/Contents/MacOS/$APPLICATION_NAME" -o "$DSYM"
$STRIP -S "$APP/Contents/MacOS/$APPLICATION_NAME"

if [ "$1" == "pkg" ]; then
    BASE_ICEBERG_CONFIG_FILE="$APPLICATION_NAME.packproj"
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

notify "Creating dmg ..."
umount "$VOLUME" 2> /dev/null
$DMGUTIL --open --volume="$APPLICATION_NAME" "$DMG"

#cp *.pdf "$VOLUME"
cp -R "$APP" "$VOLUME"
ln -s /Applications "$VOLUME"

$DMGUTIL --set --iconsize=96 --toolbar=false --icon=resources/macx/OpenBoard.icns "$VOLUME"
$DMGUTIL --set --x=20 --y=60 --width=580 --height=440 "$VOLUME"
$DMGUTIL --set --x=180 --y=120 "$VOLUME/`basename \"$APP\"`"
$DMGUTIL --set --x=400 --y=120 "$VOLUME/Applications"
#$DMGUTIL --set --x=180 --y=280 "$VOLUME/ReleaseNotes.pdf"

$DMGUTIL --close --volume="$APPLICATION_NAME" "$DMG"

notify "$APPLICATION_NAME is built"

PRODUCT_DIR="install/mac/"

if [ ! -d "${PRODUCT_DIR}" ]; then
    mkdir -p "${PRODUCT_DIR}"
fi

mv "$DMG" "${PRODUCT_DIR}"

exit 0

