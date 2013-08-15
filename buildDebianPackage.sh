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


#**********************
#     functions
#**********************

checkUser()
{
  if [ `id -u` -ne 0 ]; then
    echo "Please run the script as root, may be using fakeroot command as follow"
    echo "fakeroot ./buildDebianPackage.sh [options]"
    exit 1
  fi
}

initializeVariables()
{
  APPLICATION_NAME="OpenBoard"
  MAKE_TAG=true
  CREATE_DIENA_DISTRIBUTION_ZIP=false
  STANDARD_QT_USED=false

  PRODUCT_PATH="build/linux/release/product"
  QT_PATH="/usr/local/Trolltech/Qt-4.8.0"
  PLUGINS_PATH="$QT_PATH/plugins"
  GUI_TRANSLATIONS_DIRECTORY_PATH="../Qt-4.8/translations"
  QT_LIBRARY_DEST_PATH="$PRODUCT_PATH/qtlib"
  QT_LIBRARY_SOURCE_PATH="$QT_PATH/lib"
  ARCHITECTURE=`uname -m`
  if [ "$ARCHITECTURE" == "x86_64" ]; then
      ARCHITECTURE="amd64"
  fi
  if [ "$ARCHITECTURE" == "i686" ]; then
      ARCHITECTURE="i386"
  fi

  NOTIFY_CMD=`which notify-send`
  QMAKE_PATH="$QT_PATH/bin/qmake"
  LRELEASES="$QT_PATH/bin/lrelease"
  ZIP_PATH=`which zip`

}


notifyError(){
    if [ -e "$NOTIFY_CMD" ]; then
        $NOTIFY_CMD -t 0 -i "/usr/share/icons/oxygen/64x64/status/dialog-error.png" "$1"
    else
        printf "\033[31merror:\033[0m $1\n"
    fi
    exit 1
}

notifyProgress(){
    if [ -e "$NOTIFY_CMD" ]; then
        $NOTIFY_CMD "$1" "$2"
    else
        printf "\033[32m--> Achieved task:\033[0m $1:\n\t$2\n"
    fi
}

alertIfPreviousVersionInstalled(){
    APT_CACHE=`which apt-cache`
    if [ ! -e "$APT_CACHE" ]; then
        notifyError "apt-cache command not found"
    else
        SEARCH_RESULT=`$APT_CACHE search ${APPLICATION_NAME}`
        if [ `echo $SEARCH_RESULT | grep -c ${APPLICATION_NAME}` -ge 1 ]; then
            notifyError "Found a previous version of ${APPLICATION_NAME}. Remove it to avoid to put it as dependency"
        fi
    fi
}

checkDir(){
    if [ ! -d "$1" ]; then
        notifyError "Directory not found : $1"
    fi
}

checkExecutable(){
    if [ ! -e "$1" ]; then
        notifyError "$1 command not found"
    fi
}

copyQtLibrary(){
    if ls "$QT_LIBRARY_SOURCE_PATH/$1.so" &> /dev/null; then
        cp $QT_LIBRARY_SOURCE_PATH/$1.so.? "$QT_LIBRARY_DEST_PATH/"
        cp $QT_LIBRARY_SOURCE_PATH/$1.so.?.?.? "$QT_LIBRARY_DEST_PATH/"
    else
        notifyError "$1 library not found in path: $QT_LIBRARY_SOURCE_PATH"
    fi
}


buildWithStandardQt(){
  STANDARD_QT=`which qmake-qt4`
  if [ $? == "0" ]; then
    QT_VERSION=`$STANDARD_QT --version | grep -i "Using Qt version" | sed -e "s/Using Qt version \(.*\) in.*/\1/"`
    if [ `echo $QT_VERSION | sed -e "s/\.//g"` -gt 480 ]; then
        notifyProgress "Standard QT" "A recent enough qmake has been found. Using this one instead of custom one"
        STANDARD_QT_USED=true
        QMAKE_PATH=$STANDARD_QT
        LRELEASES=`which lrelease`
        if [ "`arch`" == "i686" ]; then
            QT_PATH="/usr/lib/i386-linux-gnu"
        else
            QT_PATH="/usr/lib/`arch`-linux-gnu"
        fi
        PLUGINS_PATH="$QT_PATH/qt4/plugins"
    fi
  fi
}

buildImporter(){
    IMPORTER_DIR="../OpenSankoreToOpenBoard"
    IMPORTER_NAME="OpenBoardImporter"
    checkDir $IMPORTER_DIR
    cd ${IMPORTER_DIR}
    
    rm moc_*
    rm -rf debug release
    rm *.o

    git reset --hard
    git pull

    $QMAKE_PATH ${IMPORTER_NAME}.pro
    make clean
    make -j4
    checkExecutable $IMPORTER_NAME
    cd -
}

#**********************
#     script
#**********************
checkUser
initializeVariables
buildWithStandardQt

for var in "$@"
do
   if [ $var == "notag" ]; then
      MAKE_TAG=false;
   fi
   if [ $var == "diena" ]; then
       CREATE_DIENA_DISTRIBUTION_ZIP=true;
   fi
done


alertIfPreviousVersionInstalled

#check of directories and executables
checkDir $QT_PATH
checkDir $PLUGINS_PATH
checkDir $GUI_TRANSLATIONS_DIRECTORY_PATH

checkExecutable $QMAKE_PATH
checkExecutable $LRELEASES
checkExecutable $ZIP_PATH

#build third party application
buildImporter 
notifyProgress "OpenBoardImporter" "Built Importer"

# cleaning the build directory
rm -rf "build/linux/release"
rm -rf install

notifyProgress "QT" "Internalization"
$LRELEASES ${APPLICATION_NAME}.pro
cd $GUI_TRANSLATIONS_DIRECTORY_PATH
$LRELEASES translations.pro
cd -

notifyProgress "${APPLICATION_NAME}" "Building ${APPLICATION_NAME}"

if [ "$ARCHITECTURE" == "amd64" ]; then
    $QMAKE_PATH ${APPLICATION_NAME}.pro -spec linux-g++-64
else
    $QMAKE_PATH ${APPLICATION_NAME}.pro -spec linux-g++
fi

make -j 4 release-install

if [ ! -e "$PRODUCT_PATH/${APPLICATION_NAME}" ]; then
    notifyError "${APPLICATION_NAME} build failed"
fi

notifyProgress "Git Hub" "Make a tag of the delivered version"

VERSION=`cat build/linux/release/version`

if [ ! -f build/linux/release/version ]; then
    notifyError "version not found"
else
    LAST_COMMITED_VERSION="`git describe $(git rev-list --tags --max-count=1)`"
    if [ "v$VERSION" != "$LAST_COMMITED_VERSION" ]; then
        if [ $MAKE_TAG == true ]; then
            git tag -a "OBv$VERSION" -m "OpenBoard setup for v$VERSION"
            git push origin --tags
        fi 
    fi
fi

cp resources/linux/run.sh $PRODUCT_PATH
chmod a+x $PRODUCT_PATH/run.sh

cp -R resources/linux/qtlinux/* $PRODUCT_PATH/

notifyProgress "QT" "Copying plugins and library ..."
cp -R $PLUGINS_PATH $PRODUCT_PATH/

# copying customization
cp -R resources/customizations $PRODUCT_PATH/

# copying importer
mkdir -p $PRODUCT_PATH/Importer
cp -R ${IMPORTER_DIR}/${IMPORTER_NAME} $PRODUCT_PATH/Importer

if [ $STANDARD_QT_USED == false ]; then 
#copying custom qt library
  mkdir -p $QT_LIBRARY_DEST_PATH
  copyQtLibrary libQtDBus
  copyQtLibrary libQtScript
  copyQtLibrary libQtSvg
  copyQtLibrary libQtXmlPatterns
  copyQtLibrary libQtNetwork
  copyQtLibrary libQtXml
  copyQtLibrary libQtGui
  copyQtLibrary libQtCore
  copyQtLibrary libphonon
  copyQtLibrary libQtWebKit
fi

notifyProgress "QT" "Internalization"
if [ ! -e $PRODUCT_PATH/i18n ]; then
    mkdir $PRODUCT_PATH/i18n
fi
#copying qt gui translation    
cp $GUI_TRANSLATIONS_DIRECTORY_PATH/qt_??.qm $PRODUCT_PATH/i18n/

rm -rf install/linux
mkdir -p install/linux

#Removing .svn directories ...
cd $PRODUCT_PATH
find . -name .svn -exec rm -rf {} \; 2> /dev/null
cd -

notifyProgress "Building ${APPLICATION_NAME}" "Finished to build ${APPLICATION_NAME} building the package"

BASE_WORKING_DIR="packageBuildDir"

#creating package directory
mkdir $BASE_WORKING_DIR
mkdir "$BASE_WORKING_DIR/DEBIAN"
mkdir -p "$BASE_WORKING_DIR/usr/share/applications"
mkdir -p "$BASE_WORKING_DIR/usr/local"


cat > "$BASE_WORKING_DIR/DEBIAN/prerm" << EOF
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

xdg-desktop-menu uninstall /usr/share/applications/${APPLICATION_NAME}.desktop
exit 0
#DEBHELPER#
EOF

cat > "$BASE_WORKING_DIR/DEBIAN/postint" << EOF
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

xdg-desktop-menu install --novendor /usr/share/applications/${APPLICATION_NAME}.desktop
exit 0
#DEBHELPER#
EOF


APPLICATION_DIRECTORY_NAME="${APPLICATION_NAME}-$VERSION"
PACKAGE_DIRECTORY="$BASE_WORKING_DIR/usr/local/$APPLICATION_DIRECTORY_NAME"
#move build directory to packages directory
cp -R $PRODUCT_PATH $PACKAGE_DIRECTORY 


cat > $BASE_WORKING_DIR/usr/local/$APPLICATION_DIRECTORY_NAME/run.sh << EOF
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

env LD_LIBRARY_PATH=/usr/local/$APPLICATION_DIRECTORY_NAME/qtlib:$LD_LIBRARY_PATH /usr/local/$APPLICATION_DIRECTORY_NAME/${APPLICATION_NAME}
EOF


CHANGE_LOG_FILE="$BASE_WORKING_DIR/DEBIAN/changelog-${APPLICATION_NAME}-$VERSION.txt"
CONTROL_FILE="$BASE_WORKING_DIR/DEBIAN/control"
CHANGE_LOG_TEXT="changelog.txt"

echo "${APPLICATION_NAME} ($VERSION) $ARCHITECTURE; urgency=low" > "$CHANGE_LOG_FILE"
echo >> "$CHANGE_LOG_FILE"
cat $CHANGE_LOG_TEXT >> "$CHANGE_LOG_FILE"
echo >> "$CHANGE_LOG_FILE"
echo "-- Claudio Valerio <claudio.valerio@oe-f.org>  `date`" >> "$CHANGE_LOG_FILE"

echo "Package: ${APPLICATION_NAME}" > "$CONTROL_FILE"
echo "Version: $VERSION" >> "$CONTROL_FILE"
echo "Section: education" >> "$CONTROL_FILE"
echo "Priority: optional" >> "$CONTROL_FILE"
echo "Architecture: $ARCHITECTURE" >> "$CONTROL_FILE"
echo "Essential: no" >> "$CONTROL_FILE"
echo "Installed-Size: `du -s $PACKAGE_DIRECTORY | awk '{ print $1 }'`" >> "$CONTROL_FILE"
echo "Maintainer: ${APPLICATION_NAME} Developers team <dev@oe-f.org>" >> "$CONTROL_FILE"
echo "Homepage: http://www.openboard.org" >> "$CONTROL_FILE"
echo -n "Depends: " >> "$CONTROL_FILE"
unset tab
declare -a tab
let count=0
for l in `objdump -p $PACKAGE_DIRECTORY/${APPLICATION_NAME} | grep NEEDED | awk '{ print $2 }'`; do 
    for lib in `dpkg -S  $l | awk -F":" '{ print $1 }'`; do
        #echo $lib
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
echo "" >> "$CONTROL_FILE"
echo "Description: This a interactive white board that uses a free standard format." >> "$CONTROL_FILE"

find $BASE_WORKING_DIR/usr/ -exec md5sum {} > $BASE_WORKING_DIR/DEBIAN/md5sums 2>/dev/null \; 
APPLICATION_SHORTCUT="$BASE_WORKING_DIR/usr/share/applications/${APPLICATION_NAME}.desktop"
echo "[Desktop Entry]" > $APPLICATION_SHORTCUT
echo "Version=$VERSION" >> $APPLICATION_SHORTCUT
echo "Encoding=UTF-8" >> $APPLICATION_SHORTCUT
echo "Name=${APPLICATION_NAME} ($VERSION)" >> $APPLICATION_SHORTCUT
echo "GenericName=${APPLICATION_NAME}" >> $APPLICATION_SHORTCUT
echo "Comment=Logiciel de création de présentations pour tableau numérique interactif (TNI)" >> $APPLICATION_SHORTCUT 
echo "Exec=/usr/local/$APPLICATION_DIRECTORY_NAME/run.sh" >> $APPLICATION_SHORTCUT
echo "Icon=/usr/local/$APPLICATION_DIRECTORY_NAME/${APPLICATION_NAME}.png" >> $APPLICATION_SHORTCUT
echo "StartupNotify=true" >> $APPLICATION_SHORTCUT
echo "Terminal=false" >> $APPLICATION_SHORTCUT
echo "Type=Application" >> $APPLICATION_SHORTCUT
echo "Categories=Education" >> $APPLICATION_SHORTCUT
cp "resources/images/${APPLICATION_NAME}.png" "$PACKAGE_DIRECTORY/${APPLICATION_NAME}.png"
chmod 755 "$BASE_WORKING_DIR/DEBIAN"
chmod 755 "$BASE_WORKING_DIR/DEBIAN/prerm"
chmod 755 "$BASE_WORKING_DIR/DEBIAN/postint"

mkdir -p "install/linux"
DEBIAN_PACKAGE_NAME="${APPLICATION_NAME}_${VERSION}_$ARCHITECTURE.deb"

chown -R root:root $BASE_WORKING_DIR 
dpkg -b "$BASE_WORKING_DIR" "install/linux/$DEBIAN_PACKAGE_NAME"

#clean up mess
rm -rf $BASE_WORKING_DIR

notifyProgress "${APPLICATION_NAME}" "Package built"


if [ $CREATE_DIENA_DISTRIBUTION_ZIP == true ]; then

    ZIP_NAME="${APPLICATION_NAME}_`lsb_release -is`_`lsb_release -rs`_${VERSION}_${ARCHITECTURE}.zip"
    cd install/linux
    $ZIP_PATH -1 --junk-paths ${ZIP_NAME} ${DEBIAN_PACKAGE_NAME} ../../ReleaseNotes.pdf ../../JournalDesModifications.pdf
    cd -
    notifyProgress "${APPLICATION_NAME}" "Build Diena zip file for distribution"
fi

exit 0
