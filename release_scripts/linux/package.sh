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
#    | prerm
#    | postinst
#    usr/
#    | bin/
#    | | openboard <-- actually a symlink to run.sh
#    | share/
#    | | applications/
#    | | | OpenBoard.desktop
#    opt/
#    | openboard/
#    | | importer/
#    | | library/
#    | | etc/
#    | | qtlib/
#    | | plugins/
#    | | OpenBoard
#    | | OpenBoard.png
#    | | qt.conf
#    | | run.sh
#
# ----------------------------------------------------------------------------

checkUser()
{
  if [ `id -u` -ne 0 ]; then
    echo "Please run the script as root" 
    exit 1
  fi
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
        cp -P $QT_LIBRARY_SOURCE_PATH/$1.so.? "$QT_LIBRARY_DEST_PATH/"
        cp -P $QT_LIBRARY_SOURCE_PATH/$1.so.?.? "$QT_LIBRARY_DEST_PATH/"
        cp -P $QT_LIBRARY_SOURCE_PATH/$1.so.?.?.? "$QT_LIBRARY_DEST_PATH/"

        strip $QT_LIBRARY_DEST_PATH/$1.so.?.?.?
        chmod 644 $QT_LIBRARY_DEST_PATH/$1.so.?.?.? # 644 = rw-r-r
    else
        notifyError "$1 library not found in path: $QT_LIBRARY_SOURCE_PATH"
    fi
}

copyQtPlugin(){
    echo -e "\t $1"
    if ls "$QT_PLUGINS_SOURCE_PATH/$1" &> /dev/null; then
        cp -r $QT_PLUGINS_SOURCE_PATH/$1 $QT_PLUGINS_DEST_PATH/

        strip $QT_PLUGINS_DEST_PATH/$1/*
        chmod 644 $QT_PLUGINS_DEST_PATH/$1/* # 644 = rw-r-r

    else
        notifyError "$1 plugin not found in path: $QT_PLUGINS_SOURCE_PATH"
    fi
}

initializeVariables()
{
  # This script's path
  SCRIPT_PATH=`pwd` 

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

  APPLICATION_NAME="OpenBoard"
  APPLICATION_CODE="openboard"
  APPLICATION_PATH="opt"

  PACKAGE_DIRECTORY=$BASE_WORKING_DIR/$APPLICATION_PATH/$APPLICATION_CODE
  QT_PLUGINS_DEST_PATH="$PACKAGE_DIRECTORY/plugins"
  QT_LIBRARY_DEST_PATH="$PACKAGE_DIRECTORY/qtlib"

  DESKTOP_FILE_PATH="$BASE_WORKING_DIR/usr/share/applications"
  APPLICATION_SHORTCUT="$DESKTOP_FILE_PATH/${APPLICATION_CODE}.desktop"

  DESCRIPTION="OpenBoard, an interactive white board application"
  VERSION=`cat $BUILD_DIR/version`
  ARCHITECTURE=`cat buildContext`

  # Qt installation path. This may vary across machines
  QT_PATH="/opt/qt55"
  QT_PLUGINS_SOURCE_PATH="$QT_PATH/plugins"
  GUI_TRANSLATIONS_DIRECTORY_PATH="$QT_PATH/translations"
  QT_LIBRARY_SOURCE_PATH="/home/craig/openboard/qtlib"

  NOTIFY_CMD=`which notify-send`
  ZIP_PATH=`which zip`
}

# ----------------------------------------------------------------------------
# Copying the application, libs etc. to the temporary working directory
# ----------------------------------------------------------------------------

initializeVariables

checkBuild
checkUser

cd $PROJECT_ROOT

rm -rf $PACKAGE_DIRECTORY
mkdir -p $PACKAGE_DIRECTORY

rm -rf $PACKAGE_BUILD_DIR


notifyProgress "Copying product directory and resources"
cp -R $PRODUCT_PATH/* $PACKAGE_DIRECTORY
chown -R root:root $PACKAGE_DIRECTORY

cp resources/linux/run.sh $PACKAGE_DIRECTORY
chmod a+x $PACKAGE_DIRECTORY/run.sh

cp -R resources/customizations $PACKAGE_DIRECTORY/
cp -R resources/linux/qtlinux/* $PACKAGE_DIRECTORY/

notifyProgress "Copying importer"
mkdir -p $PACKAGE_DIRECTORY/importer
cp -R "$IMPORTER_DIR/$IMPORTER_NAME" "$PACKAGE_DIRECTORY/importer"

notifyProgress "Stripping importer and main executable"
strip $PACKAGE_DIRECTORY/$APPLICATION_NAME
strip $PACKAGE_DIRECTORY/importer/$IMPORTER_NAME

notifyProgress "Copying and stripping Qt plugins"
mkdir -p $QT_PLUGINS_DEST_PATH
copyQtPlugin audio
copyQtPlugin generic
copyQtPlugin iconengines
copyQtPlugin imageformats
copyQtPlugin mediaservice
copyQtPlugin platforminputcontexts
copyQtPlugin platforms
copyQtPlugin platformthemes
copyQtPlugin position
copyQtPlugin printsupport
copyQtPlugin qtwebengine
copyQtPlugin sceneparsers
copyQtPlugin xcbglintegrations

notifyProgress "Copying and stripping Qt libraries"
mkdir -p $QT_LIBRARY_DEST_PATH
copyQtLibrary libQt5Core
copyQtLibrary libQt5Gui
copyQtLibrary libQt5Multimedia
copyQtLibrary libQt5MultimediaWidgets
copyQtLibrary libQt5Network
copyQtLibrary libQt5OpenGL
copyQtLibrary libQt5PrintSupport
copyQtLibrary libQt5Script
copyQtLibrary libQt5Svg
copyQtLibrary libQt5WebChannel
copyQtLibrary libQt5WebKit
copyQtLibrary libQt5WebKitWidgets
copyQtLibrary libQt5Widgets
copyQtLibrary libQt5XcbQpa
copyQtLibrary libQt5Xml
copyQtLibrary libQt5XmlPatterns

notifyProgress "Copying Qt translations"
mkdir -p $PACKAGE_DIRECTORY/i18n
cp $GUI_TRANSLATIONS_DIRECTORY_PATH/qt_??.qm $PACKAGE_DIRECTORY/i18n/


# ----------------------------------------------------------------------------
# DEBIAN directory of package (control, md5sums, postinst etc)
# ----------------------------------------------------------------------------
notifyProgress "Generating control files for package"

mkdir -p "$BASE_WORKING_DIR/DEBIAN"

# Copy prerm, postinst scripts
cp -r "$SCRIPT_PATH/debian_package_files/prerm" "$BASE_WORKING_DIR/DEBIAN/"
cp -r "$SCRIPT_PATH/debian_package_files/postinst" "$BASE_WORKING_DIR/DEBIAN/"
chmod 755 "$BASE_WORKING_DIR/DEBIAN/prerm"
chmod 755 "$BASE_WORKING_DIR/DEBIAN/postinst"

# Generate md5 sums of everything in the application path (e.g /opt) and the desktop entry
find $BASE_WORKING_DIR/$APPLICATION_PATH/ -exec md5sum {} > $BASE_WORKING_DIR/DEBIAN/md5sums 2>/dev/null \;
find $DESKTOP_FILE_PATH/ -exec md5sum {} > $BASE_WORKING_DIR/DEBIAN/md5sums 2>/dev/null \;

# Generate control file
CONTROL_FILE="$BASE_WORKING_DIR/DEBIAN/control"

echo "Package: ${APPLICATION_CODE}" > "$CONTROL_FILE"
echo "Version: $VERSION" >> "$CONTROL_FILE"
echo "Section: education" >> "$CONTROL_FILE"
echo "Priority: optional" >> "$CONTROL_FILE"
echo "Architecture: $ARCHITECTURE" >> "$CONTROL_FILE"
echo "Essential: no" >> "$CONTROL_FILE"
echo "Installed-Size: `du -s $PACKAGE_DIRECTORY | awk '{ print $1 }'`" >> "$CONTROL_FILE"
echo "Maintainer: ${APPLICATION_NAME} Developers team <dev@openboard.ch>" >> "$CONTROL_FILE"
echo "Homepage: https://github.com/DIP-SEM/OpenBoard" >> "$CONTROL_FILE"

# Generate dependency list
echo -n "Depends: " >> "$CONTROL_FILE"
#echo -n "libpaper1, zlib1g (>= 1.2.8), libssl1.0.0 (>= 1.0.1), libx11-6, libgl1-mesa-glx, libc6 (>= 2.19), libstdc++6 (>= 4.8.4), libgomp1, onboard" >> "$CONTROL_FILE"

unset tab
declare -a tab
let count=0
for l in `objdump -p $PACKAGE_DIRECTORY/${APPLICATION_NAME} | grep NEEDED | awk '{ print $2 }'`; do
    for lib in `dpkg -S  $l | grep -v "libqt5" | grep -v "qt55" | awk -F":" '{ print $1 }'`; do
        presence=`echo ${tab[*]} | grep -c "$lib"`;
        if [ "$presence" == "0" ]; then
            tab[$count]=$lib;
            ((count++));
        fi;
    done;
done;

for ((i=0;i<${#tab[@]};i++)); do
    if [ $i -ne "0" ]; then
        echo -n ",    " >> "$CONTROL_FILE"
    fi
    echo -n "${tab[$i]} (>= "`dpkg -p ${tab[$i]} | grep "Version: " | awk '{      print $2 }' | sed -e 's/\([:. 0-9?]*\).*/\1/g' | sed -e 's/\.$//'`") " >> "$CONTROL_FILE"
done
echo -n ",  onboard" >> "$CONTROL_FILE"

echo "" >> "$CONTROL_FILE"
echo "Description: $DESCRIPTION" >> "$CONTROL_FILE"

# ----------------------------------------------------------------------------
# .desktop file
# ----------------------------------------------------------------------------
mkdir -p $DESKTOP_FILE_PATH
echo "[Desktop Entry]" > $APPLICATION_SHORTCUT
echo "Version=$VERSION" >> $APPLICATION_SHORTCUT
echo "Encoding=UTF-8" >> $APPLICATION_SHORTCUT
echo "Name=${APPLICATION_NAME}" >> $APPLICATION_SHORTCUT
echo "Comment=$DESCRIPTION" >> $APPLICATION_SHORTCUT
#echo "Exec=$APPLICATION_PATH/$APPLICATION_CODE/run.sh" >> $APPLICATION_SHORTCUT
echo "Exec=$APPLICATION_CODE %f" >> $APPLICATION_SHORTCUT
echo "Icon=/$APPLICATION_PATH/$APPLICATION_CODE/${APPLICATION_NAME}.png" >> $APPLICATION_SHORTCUT
echo "StartupNotify=true" >> $APPLICATION_SHORTCUT
echo "Terminal=false" >> $APPLICATION_SHORTCUT
echo "Type=Application" >> $APPLICATION_SHORTCUT
echo "Categories=Education;" >> $APPLICATION_SHORTCUT
cp "resources/images/${APPLICATION_NAME}.png" "$PACKAGE_DIRECTORY/${APPLICATION_NAME}.png"



# ----------------------------------------------------------------------------
# Building the package
# ----------------------------------------------------------------------------
notifyProgress "Building package"
mkdir -p "$PACKAGE_BUILD_DIR/linux"
PACKAGE_NAME="${APPLICATION_NAME}_`lsb_release -is`_`lsb_release -rs`_${VERSION}_$ARCHITECTURE.deb"
PACKAGE_NAME=`echo "$PACKAGE_NAME" | awk '{print tolower($0)}'`

dpkg -b "$BASE_WORKING_DIR" "$PACKAGE_BUILD_DIR/linux/$PACKAGE_NAME"

#clean up mess
rm -rf $BASE_WORKING_DIR

notifyProgress "${APPLICATION_NAME}" "Package built"


exit 0
