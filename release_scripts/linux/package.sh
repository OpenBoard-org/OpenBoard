#!/bin/bash
# --------------------------------------------------------------------
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
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

# ----------------------------------------------------------------------------
# Packaging script for OpenBoard, for Debian-compatible distributions. 
#
# This should be run after `build.sh`.
#
# The generated package structure is as follows : 
#
#    DEBIAN/
#    | control
#    | md5sums
#    usr/
#    | bin/
#    | | openboard
#    | share/
#    | | applications/
#    | | | ch.openboard.OpenBoard.desktop
#    | | icons/
#    | | | hicolor/
#    | | | | scalable/
#    | | | | | apps/
#    | | | | | | ch.openboard.OpenBoard.svg
#    | | | | | mimetypes/
#    | | | | | | ch.openboard.application-ubz.svg
#    | | mime/
#    | | | packages/
#    | | | | ch.openboard.openboard-ubz.xml
#    opt/
#    | openboard/
#    | | customizations/
#    | | | fonts/
#    | | etc/
#    | | fonts/
#    | | i18n/
#    | | importer/
#    | | library/
#
# (*) Only included if Qt libs and plugins are bundled. It is necessary to
# bundle these if the target system doesn't provide Qt 5.5.1, for example.
# ----------------------------------------------------------------------------

initializeVariables()
{
  # This script's path
  SCRIPT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

  PROJECT_ROOT="$SCRIPT_PATH/../.."

  # Where the application was built (see build.sh)
  BUILD_DIR="$PROJECT_ROOT/build/linux/release"
  PRODUCT_PATH="$BUILD_DIR/product"
  IMPORTER_DIR="$PROJECT_ROOT/../OpenBoard-Importer/"
  IMPORTER_NAME="OpenBoardImporter"

  # Where the package is built to
  PACKAGE_BUILD_DIR="$PROJECT_ROOT/install"

  # Temporary folder, where we put all the files that will be built into the
  # package
  BASE_WORKING_DIR="debianPackage"

  APPLICATION_NAME="openboard"
  APPLICATION_CODE="openboard"
  APPLICATION_PATH="opt"
  SYSTEM_PATH="usr"

  PACKAGE_DIRECTORY=$BASE_WORKING_DIR/$APPLICATION_PATH/$APPLICATION_CODE
  BINARY_DIRECTORY=$BASE_WORKING_DIR/usr/bin
  PACKAGE_QT_DIRECTORY="$PACKAGE_DIRECTORY/qt"
  QT_LIBRARY_DEST_PATH="$PACKAGE_QT_DIRECTORY/lib"
  QT_LIBRARY_EXECUTABLES_DEST_PATH="$PACKAGE_QT_DIRECTORY/libexec"
  QT_PLUGINS_DEST_PATH="$PACKAGE_QT_DIRECTORY/plugins"
  QT_RESOURCES_DEST_PATH="$PACKAGE_QT_DIRECTORY/resources"
  QT_TRANSLATIONS_DEST_PATH="$PACKAGE_QT_DIRECTORY/translations"

  DESKTOP_FILE_PATH="$BASE_WORKING_DIR/usr/share/applications"
  APPLICATION_SHORTCUT="$DESKTOP_FILE_PATH/${APPLICATION_CODE}.desktop"

  DESCRIPTION="OpenBoard, an interactive white board application"
  VERSION=`cat $BUILD_DIR/version`
  ARCHITECTURE=`cat buildContext`


  # Include Qt libraries and plugins in the package, or not
  # (this is necessary if the target system doesn't provide Qt 5.5.1)
  BUNDLE_QT=false

  # Qt installation path. This may vary across machines
  QT_PATH="/usr/lib/x86_64-linux-gnu/qt5"
  GUI_TRANSLATIONS_DIRECTORY_PATH="/usr/share/qt5/translations"
  QT_LIBRARY_SOURCE_PATH="$QT_PATH/lib"
  QT_LIBRARY_EXECUTABLES_SOURCE_PATH="$QT_PATH/libexec"
  QT_PLUGINS_SOURCE_PATH="$QT_PATH/plugins"
  QT_RESOURCES_SOURCE_PATH="$QT_PATH/resources"
  QT_TRANSLATIONS_SOURCE_PATH="$QT_PATH/translations"

  NOTIFY_CMD=`which notify-send`
  ZIP_PATH=`which zip`
}

checkBuild() 
{
  if [ -z "$ARCHITECTURE" ]; then
    echo "Make sure you have built the software first using ./build.sh"
    exit 1
  fi
}

notifyError(){
    if [ -e "$NOTIFY_CMD" ]; then
        $NOTIFY_CMD -t 0 -i "/usr/share/icons/oxygen/64x64/status/dialog-error.png" "$1"
    fi
    printf "\033[31merror:\033[0m $1\n"
    exit 1
}

notifyProgress(){
    if [ -e "$NOTIFY_CMD" ]; then
        $NOTIFY_CMD "$1" "$2"
    fi
    printf "\033[32m--> \033[0m $1:\n\t$2\n"
}

copyQtLibrary(){
    echo -e "\t $1"
    if ls "$QT_LIBRARY_SOURCE_PATH/$1.so" &> /dev/null; then
        cp -P $QT_LIBRARY_SOURCE_PATH/$1.so "$QT_LIBRARY_DEST_PATH/"
        cp -P $QT_LIBRARY_SOURCE_PATH/$1.so.* "$QT_LIBRARY_DEST_PATH/"
        # do not package debug libraries
        rm $QT_LIBRARY_DEST_PATH/$1.so.*.debug

        strip $QT_LIBRARY_DEST_PATH/$1.so
        chmod 644 $QT_LIBRARY_DEST_PATH/$1.so.* # 644 = rw-r-r
    else
        notifyError "$1 library not found in path: $QT_LIBRARY_SOURCE_PATH"
    fi
}

copyQtPlugin(){
    echo -e "\t $1"
    if ls "$QT_PLUGINS_SOURCE_PATH/$1" &> /dev/null; then
        cp -r $QT_PLUGINS_SOURCE_PATH/$1 $QT_PLUGINS_DEST_PATH/
        # do not package debug libraries
        rm $QT_PLUGINS_DEST_PATH/$1/*.debug

        strip $QT_PLUGINS_DEST_PATH/$1/*
        chmod 644 $QT_PLUGINS_DEST_PATH/$1/* # 644 = rw-r-r
        chmod +rx $QT_PLUGINS_DEST_PATH/$1

    else
        notifyError "$1 plugin not found in path: $QT_PLUGINS_SOURCE_PATH"
    fi
}


# ----------------------------------------------------------------------------
# Copying the application, libs etc. to the temporary working directory
# ----------------------------------------------------------------------------


initializeVariables

checkBuild

cd $PROJECT_ROOT

rm -rf $PACKAGE_DIRECTORY
mkdir -p $PACKAGE_DIRECTORY

rm -rf $PACKAGE_BUILD_DIR


notifyProgress "Copying product directory and resources"
cp -R $PRODUCT_PATH/* $BASE_WORKING_DIR

if $BUNDLE_QT; then
    STARTER=$BASE_WORKING_DIR/usr/local/bin/openboard
    QT_INSTALL=/$APPLICATION_PATH/$APPLICATION_CODE/qt
    mkdir -p $(dirname $STARTER)
    echo "#!/bin/bash" > $STARTER
    echo "env LD_LIBRARY_PATH=$QT_INSTALL/lib:\$LD_LIBRARY_PATH QT_PLUGIN_PATH=$QT_INSTALL/plugins /usr/bin/openboard \"\$@\"" >> $STARTER
    chmod a+x $STARTER
fi

notifyProgress "Copying importer"
mkdir -p $PACKAGE_DIRECTORY/importer
cp -R "$IMPORTER_DIR/$IMPORTER_NAME" "$PACKAGE_DIRECTORY/importer"

notifyProgress "Stripping importer and main executable"
strip $BINARY_DIRECTORY/$APPLICATION_NAME
strip $PACKAGE_DIRECTORY/importer/$IMPORTER_NAME

if $BUNDLE_QT; then
    notifyProgress "Copying and stripping Qt plugins"
    mkdir -p $QT_PLUGINS_DEST_PATH
    copyQtPlugin audio
    copyQtPlugin bearer
    copyQtPlugin generic
    copyQtPlugin iconengines
    copyQtPlugin imageformats
    copyQtPlugin mediaservice
    copyQtPlugin platforminputcontexts
    copyQtPlugin platforms
    copyQtPlugin platformthemes
    copyQtPlugin position
    copyQtPlugin printsupport
    copyQtPlugin sceneparsers
    copyQtPlugin xcbglintegrations

    notifyProgress "Copying and stripping Qt libraries"
    mkdir -p $QT_LIBRARY_DEST_PATH
    copyQtLibrary libQt5Concurrent
    copyQtLibrary libQt5Core
    copyQtLibrary libQt5DBus
    copyQtLibrary libQt5Gui
    copyQtLibrary libQt5Multimedia
    copyQtLibrary libQt5MultimediaGstTools
    copyQtLibrary libQt5MultimediaWidgets
    copyQtLibrary libQt5Network
    copyQtLibrary libQt5OpenGL
    copyQtLibrary libQt5Positioning
    copyQtLibrary libQt5PrintSupport
    copyQtLibrary libQt5Qml
    copyQtLibrary libQt5QmlModels
    copyQtLibrary libQt5Quick
    copyQtLibrary libQt5Sensors
    copyQtLibrary libQt5Sql
    copyQtLibrary libQt5Svg
    copyQtLibrary libQt5WebChannel
    copyQtLibrary libQt5WebEngineCore
    copyQtLibrary libQt5WebEngineWidgets
    copyQtLibrary libQt5QuickWidgets
    copyQtLibrary libQt5WebSockets
    copyQtLibrary libQt5Widgets
    copyQtLibrary libQt5XcbQpa
    copyQtLibrary libQt5Xml
    copyQtLibrary libicuuc
    copyQtLibrary libicui18n
    copyQtLibrary libicudata

    notifyProgress "Copying Qt translations"
    mkdir -p $PACKAGE_DIRECTORY/i18n
    cp $GUI_TRANSLATIONS_DIRECTORY_PATH/qt_??.qm $PACKAGE_DIRECTORY/i18n/

    # ----------------------------------------------------------------------------
    # QT WebEngine
    # ----------------------------------------------------------------------------
    notifyProgress "Copying Qt WebEngine dependencies"
    mkdir -p "$QT_LIBRARY_EXECUTABLES_DEST_PATH"
    cp $QT_LIBRARY_EXECUTABLES_SOURCE_PATH/QtWebEngineProcess $QT_LIBRARY_EXECUTABLES_DEST_PATH

    mkdir -p "$QT_RESOURCES_DEST_PATH"
    cp $QT_RESOURCES_SOURCE_PATH/* $QT_RESOURCES_DEST_PATH

    mkdir -p "$QT_TRANSLATIONS_DEST_PATH/qtwebengine_locales"
    cp $QT_TRANSLATIONS_SOURCE_PATH/qtwebengine_locales/* $QT_TRANSLATIONS_DEST_PATH/qtwebengine_locales
fi

# ----------------------------------------------------------------------------
# DEBIAN directory of package (control, md5sums, postinst etc)
# ----------------------------------------------------------------------------
notifyProgress "Generating control files for package"

mkdir -p "$BASE_WORKING_DIR/DEBIAN"

# Generate md5 sums of everything in the application path (e.g /opt) and the desktop entry
cd $BASE_WORKING_DIR
find $APPLICATION_PATH/ -exec md5sum {} > DEBIAN/md5sums 2>/dev/null \;
find $SYSTEM_PATH/ -exec md5sum {} >> DEBIAN/md5sums 2>/dev/null \;
cd $PROJECT_ROOT

# Generate control file
CONTROL_FILE="$BASE_WORKING_DIR/DEBIAN/control"

echo "Package: ${APPLICATION_CODE}" > "$CONTROL_FILE"
echo "Version: $VERSION" >> "$CONTROL_FILE"
echo "Section: education" >> "$CONTROL_FILE"
echo "Priority: optional" >> "$CONTROL_FILE"
echo "Architecture: $ARCHITECTURE" >> "$CONTROL_FILE"
echo "Essential: no" >> "$CONTROL_FILE"
echo "Installed-Size: `du -s --exclude="*/DEBIAN/*" $BASE_WORKING_DIR | awk '{ print $1 }'`" >> "$CONTROL_FILE"
echo "Maintainer: ${APPLICATION_NAME} Developers team <dev@openboard.ch>" >> "$CONTROL_FILE"
echo "Homepage: https://github.com/Open-Board-org/OpenBoard" >> "$CONTROL_FILE"

# Generate dependency list
echo -n "Depends: " >> "$CONTROL_FILE"

unset tab
declare -a tab
let count=0

if $BUNDLE_QT; then
    for l in `objdump -p $BINARY_DIRECTORY/${APPLICATION_NAME} | grep NEEDED | awk '{ print $2 }'`; do
        for lib in `dpkg -S  $l | grep -v "libqt5" | grep -v "qt55" | awk -F":" '{ print $1 }'`; do
            presence=`echo ${tab[*]} | grep -c "$lib"`;
            if [ "$presence" == "0" ]; then
		if [ "$lib" != "openboard" ]; then
			 echo "adding $lib to needed dependencies"
        	        tab[$count]=$lib;
        	        ((count++));
		fi;
            fi;
        done;
    done;
else
    for l in `objdump -p $BINARY_DIRECTORY/${APPLICATION_NAME} | grep NEEDED | awk '{ print $2 }'`; do
        for lib in `dpkg -S  $l | awk -F":" '{ print $1 }'`; do
            presence=`echo ${tab[*]} | grep -c "$lib"`;
            if [ "$presence" == "0" ]; then
                tab[$count]=$lib;
                ((count++));
            fi;
        done;
    done;
fi


for ((i=0;i<${#tab[@]};i++)); do
    if [ $i -ne "0" ]; then
        echo -n ", " >> "$CONTROL_FILE"
    fi
    # conditional dependency when libavcodec is found
    depdVer=$(apt-cache show ${tab[$i]} | grep "Version: " | head -1 | awk '{ print $2 }' | sed -e 's/\([~:. 0-9?]*\).*/\1/g' | sed -e 's/\.$//')
    if [[ "${tab[$i]}" == *"libavcodec"* ]]; then
        depName="${tab[$i]::-2}"
        versionNumber="${tab[$i]: -2}"
        depdVer_part1=`echo ${depdVer} | awk -F'.' '{print $1}'`
        depdVer_part2=`echo ${depdVer} | awk -F'.' '{print $2}'`
        echo -n "${depName}${versionNumber} (>= ${depdVer_part1}.${depdVer_part2}) | ${depName}-extra${versionNumber} (>= ${depdVer_part1}.${depdVer_part2})" >> "$CONTROL_FILE"
    else
      echo -n "${tab[$i]} (>= ${depdVer})" >> "$CONTROL_FILE"
    fi
done
echo -n ",  onboard" >> "$CONTROL_FILE"

if $BUNDLE_QT; then
    # Listing some dependencies manually; ideally we should use dpkg -p recursively 
    # to get the dependencies of the bundled shared libs & plugins. Or use static libs.
    echo -n ",  libxcb1" >> "$CONTROL_FILE"
    echo -n ",  libxcb-icccm4" >> "$CONTROL_FILE"
    echo -n ",  libxcb-xkb1" >> "$CONTROL_FILE"
    echo -n ",  libxcb-image0" >> "$CONTROL_FILE"
    echo -n ",  libxcb-render-util0" >> "$CONTROL_FILE"
    echo -n ",  libxcb-xinerama0" >> "$CONTROL_FILE"
else
    echo -n ",  libqt5multimedia5-plugins" >> "$CONTROL_FILE"
fi

echo "" >> "$CONTROL_FILE"
echo "Description: $DESCRIPTION" >> "$CONTROL_FILE"

# ----------------------------------------------------------------------------
# Building the package
# ----------------------------------------------------------------------------
notifyProgress "Building package"
mkdir -p "$PACKAGE_BUILD_DIR/linux"
PACKAGE_NAME="${APPLICATION_NAME}_`lsb_release -is`_`lsb_release -rs`_${VERSION}_$ARCHITECTURE.deb"
PACKAGE_NAME=`echo "$PACKAGE_NAME" | awk '{print tolower($0)}'`

dpkg-deb --root-owner-group -b "$BASE_WORKING_DIR" "$PACKAGE_BUILD_DIR/linux/$PACKAGE_NAME"

#clean up mess
rm -rf $BASE_WORKING_DIR

notifyProgress "${APPLICATION_NAME}" "Package built"


exit 0
