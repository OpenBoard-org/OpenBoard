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
#    | | openboard <-- actually a symlink to run.sh or OpenBoard
#    | share/
#    | | applications/
#    | | | openboard.desktop
#    opt/
#    | openboard/
#    | | importer/
#    | | library/
#    | | etc/
#    | | qtlib/ (*)
#    | | plugins/ (*)
#    | | OpenBoard
#    | | OpenBoard.png
#    | | run.sh (*)
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


  # Include Qt libraries and plugins in the package, or not
  # (this is necessary if the target system doesn't provide Qt 5.5.1)
  BUNDLE_QT=true

  # Qt installation path. This may vary across machines
  QT_PATH="/home/dev/Qt/5.15.2/gcc_64"
  QT_PLUGINS_SOURCE_PATH="$QT_PATH/plugins"
  GUI_TRANSLATIONS_DIRECTORY_PATH="/usr/share/qt5/translations"
  QT_LIBRARY_SOURCE_PATH="$QT_PATH/lib"

  NOTIFY_CMD=`which notify-send`
  ZIP_PATH=`which zip`
}

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
        cp -P $QT_LIBRARY_SOURCE_PATH/$1.so "$QT_LIBRARY_DEST_PATH/"
        cp -P $QT_LIBRARY_SOURCE_PATH/$1.so.* "$QT_LIBRARY_DEST_PATH/"

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
checkUser

cd $PROJECT_ROOT

rm -rf $PACKAGE_DIRECTORY
mkdir -p $PACKAGE_DIRECTORY

rm -rf $PACKAGE_BUILD_DIR


notifyProgress "Copying product directory and resources"
cp -R $PRODUCT_PATH/* $PACKAGE_DIRECTORY
chown -R root:root $PACKAGE_DIRECTORY

cp -R resources/customizations $PACKAGE_DIRECTORY/
cp resources/linux/openboard-ubz.xml $PACKAGE_DIRECTORY/etc/

if $BUNDLE_QT; then
    cp -R resources/linux/run.sh $PACKAGE_DIRECTORY/
    chmod a+x $PACKAGE_DIRECTORY/run.sh
fi

notifyProgress "Copying importer"
mkdir -p $PACKAGE_DIRECTORY/importer
cp -R "$IMPORTER_DIR/$IMPORTER_NAME" "$PACKAGE_DIRECTORY/importer"

notifyProgress "Stripping importer and main executable"
strip $PACKAGE_DIRECTORY/$APPLICATION_NAME
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
    #copyQtPlugin qtwebengine
    copyQtPlugin sceneparsers
    copyQtPlugin xcbglintegrations

    notifyProgress "Copying and stripping Qt libraries"
    mkdir -p $QT_LIBRARY_DEST_PATH
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
    copyQtLibrary libQt5WebKit
    copyQtLibrary libQt5WebKitWidgets
    copyQtLibrary libQt5WebSockets
    copyQtLibrary libQt5Widgets
    copyQtLibrary libQt5XcbQpa
    copyQtLibrary libQt5Xml
    copyQtLibrary libQt5XmlPatterns
    copyQtLibrary libicuuc
    copyQtLibrary libicui18n
    copyQtLibrary libicudata
fi

notifyProgress "Copying Qt translations"
mkdir -p $PACKAGE_DIRECTORY/i18n
cp $GUI_TRANSLATIONS_DIRECTORY_PATH/qt_??.qm $PACKAGE_DIRECTORY/i18n/


# ----------------------------------------------------------------------------
# DEBIAN directory of package (control, md5sums, postinst etc)
# ----------------------------------------------------------------------------
notifyProgress "Generating control files for package"

mkdir -p "$BASE_WORKING_DIR/DEBIAN"

# Copy prerm script
cp -r "$SCRIPT_PATH/debian_package_files/prerm" "$BASE_WORKING_DIR/DEBIAN/"
chmod 755 "$BASE_WORKING_DIR/DEBIAN/prerm"

# Generate postinst script (can't copy it like prerm because some paths vary depending on
# the values of the variables in this script)

SYMLINK_TARGET="/$APPLICATION_PATH/$APPLICATION_CODE/$APPLICATION_NAME"
if $BUNDLE_QT ; then
    SYMLINK_TARGET="/$APPLICATION_PATH/$APPLICATION_CODE/run.sh"
fi

cat > "$BASE_WORKING_DIR/DEBIAN/postinst" << EOF
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

xdg-desktop-menu install --novendor /usr/share/applications/${APPLICATION_CODE}.desktop
xdg-mime install --mode system /$APPLICATION_PATH/$APPLICATION_CODE/etc/openboard-ubz.xml
xdg-mime default /usr/share/applications/${APPLICATION_CODE}.desktop application/ubz

ln -s $SYMLINK_TARGET /usr/bin/$APPLICATION_CODE

exit 0
EOF

chmod 755 "$BASE_WORKING_DIR/DEBIAN/postinst"


# Generate md5 sums of everything in the application path (e.g /opt) and the desktop entry
cd $BASE_WORKING_DIR
find $APPLICATION_PATH/ -exec md5sum {} > DEBIAN/md5sums 2>/dev/null \;
find $DESKTOP_FILE_PATH/ -exec md5sum {} >> DEBIAN/md5sums 2>/dev/null \;
cd $PROJECT_ROOT

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

unset tab
declare -a tab
let count=0

if $BUNDLE_QT; then
    for l in `objdump -p $PACKAGE_DIRECTORY/${APPLICATION_NAME} | grep NEEDED | awk '{ print $2 }'`; do
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
    for l in `objdump -p $PACKAGE_DIRECTORY/${APPLICATION_NAME} | grep NEEDED | awk '{ print $2 }'`; do
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
# .desktop file
# ----------------------------------------------------------------------------
mkdir -p $DESKTOP_FILE_PATH
echo "[Desktop Entry]" > $APPLICATION_SHORTCUT
echo "Version=$VERSION" >> $APPLICATION_SHORTCUT
echo "Encoding=UTF-8" >> $APPLICATION_SHORTCUT
echo "Name=${APPLICATION_NAME}" >> $APPLICATION_SHORTCUT
echo "Comment=$DESCRIPTION" >> $APPLICATION_SHORTCUT
echo "Exec=$APPLICATION_CODE %f" >> $APPLICATION_SHORTCUT
echo "Icon=/$APPLICATION_PATH/$APPLICATION_CODE/${APPLICATION_NAME}.png" >> $APPLICATION_SHORTCUT
echo "StartupNotify=true" >> $APPLICATION_SHORTCUT
echo "Terminal=false" >> $APPLICATION_SHORTCUT
echo "Type=Application" >> $APPLICATION_SHORTCUT
echo "MimeType=application/ubz" >> $APPLICATION_SHORTCUT
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
