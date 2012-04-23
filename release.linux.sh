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


#######################################################################
#                     functions                                       #
#######################################################################
notifyError(){
    notify-send -t 0 "Error occoured" "An error occours during the sankore build:\n\t$1" -i /usr/share/icons/oxygen/64x64/status/dialog-error.png
    exit 1
}

checkExecutable(){
    if [ ! -x $1 ]; then
        notify-send "$1 executable not found"
    fi
}

checkDirectory(){
    if [ ! -d $1 ]; then
        notify-send "$1 directory not found"
    fi
}

#######################################################################
#                     path definition                                 #
#######################################################################
QT_PATH="/usr/local/Trolltech/Qt-4.7.3"
PLUGINS_PATH="$QT_PATH/plugins"
RELEASE_DIR=build/linux/release
BUILD_DIR=$RELEASE_DIR/product
GUI_TRANSLATIONS_DIRECTORY_PATH="../Qt-sankore3.1/translations"
QT_LIBRARY_SOURCE_PATH="$QT_PATH/lib"

QMAKE_PATH="$QT_PATH/bin/qmake"
LRELEASE="/usr/local/Trolltech/Qt-4.7.3/bin/lrelease"

ARCHITECTURE=`uname -m`

#######################################################################
#                     initials checks                                 #
#######################################################################
checkExecutable $QMAKE_PATH
checkExecutable $LRELEASE

checkDirectory $GUI_TRANSLATIONS_DIRECTORY_PATH
checkDirectory $PLUGINS_PATH
checkDirectory $QT_LIBRARY_SOURCE_PATH

#######################################################################
#                            cleaning                                 #
#######################################################################
rm -rf $RELEASE_DIR


#######################################################################
#                       Internalization                               #
#######################################################################
notify-send "QT" "Internalization ..."

cd $GUI_TRANSLATIONS_DIRECTORY_PATH
$LRELEASE translations.pro
cd -
if [ ! -e $BUILD_DIR/i18n ]; then
    mkdir -p $BUILD_DIR/i18n
fi
#copying qt gui translation    
cp $GUI_TRANSLATIONS_DIRECTORY_PATH/qt_??.qm $BUILD_DIR/i18n/


$LRELEASE Sankore_3.1.pro


#######################################################################
#                            building                                 #
#######################################################################
notify-send "Open-Sankore" "Building Open-Sankore ..."

if [ "$ARCHITECTURE" == "x86_64" ]; then
    $QMAKE_PATH -spec linux-g++-64
else
    $QMAKE_PATH -spec linux-g++
fi

checkDirectory $BUILD_DIR

make -j 4 release-install


#######################################################################
#                            github tag                               #
#######################################################################
notify-send "Git Hub" "Make a tag of the delivered version"
VERSION=`cat $RELEASE_DIR/version`
if [ ! -f $RELEASE_DIR/version ]; then
    notifyError "version not found"
else
    LAST_COMMITED_VERSION="`git describe $(git rev-list --tags --max-count=1)`"
    if [ "v$VERSION" != "$LAST_COMMITED_VERSION" ]; then
        echo creating a tag with the version $VERSION
#        git tag -a "v$VERSION" -m "Generating setup for v$VERSION"
#        git push origin --tags 
    fi
fi

#######################################################################
#                       coping resources                              #
#######################################################################
cp resources/linux/run.sh $BUILD_DIR
chmod +x $BUILD_DIR/run.sh

cp -R resources/linux/qtlinux/* $BUILD_DIR

cp -R resources/customizations $BUILD_DIR

notify-send "QT" "Coping plugins and library ..."
cp -R $PLUGINS_PATH $BUILD_DIR

#copying custom qt library
QT_LIBRARY_DEST_PATH="$BUILD_DIR/qtlib"
mkdir $QT_LIBRARY_DEST_PATH

copyQtLibrary(){
    if [ ! -e "$QT_LIBRARY_SOURCE_PATH/$1.so.4.7.3" ]; then
        notifyError "$1 library not found in path: $QT_LIBRARY_SOURCE_PATH"
    fi
    cp "$QT_LIBRARY_SOURCE_PATH/$1.so.4" "$QT_LIBRARY_DEST_PATH/"
    cp "$QT_LIBRARY_SOURCE_PATH/$1.so.4.7.3" "$QT_LIBRARY_DEST_PATH/"
}

copyQtLibrary libQtWebKit
copyQtLibrary libQtDBus
copyQtLibrary libQtScript
copyQtLibrary libQtSvg
copyQtLibrary libQtXmlPatterns
copyQtLibrary libQtNetwork
copyQtLibrary libQtXml
copyQtLibrary libQtGui
copyQtLibrary libQtCore

if [ ! -e "$QT_LIBRARY_SOURCE_PATH/libphonon.so.4.4.0" ]; then
    notifyError "phonon library not found in path: $QT_LIBRARY_SOURCE_PATH"
else
    cp "$QT_LIBRARY_SOURCE_PATH/libphonon.so.4" "$QT_LIBRARY_DEST_PATH/"
    cp "$QT_LIBRARY_SOURCE_PATH/libphonon.so.4.4.0" "$QT_LIBRARY_DEST_PATH/"
fi


#######################################################################
#                  Removing unwanted files                            #
#######################################################################
cd $BUILD_DIR

#Removing .svn directories ...
find . -name .svn -exec rm -rf {} \; 2> /dev/null

cd -
notify-send "Building Sankore" "Finished to build Sankore building the package"

#######################################################################
#                          build debian                               #
#######################################################################
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

xdg-desktop-menu uninstall /usr/share/applications/Open-Sankore.desktop
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

xdg-desktop-menu install --novendor /usr/share/applications/Open-Sankore.desktop
exit 0
#DEBHELPER#
EOF


SANKORE_DIRECTORY_NAME="Open-Sankore-$VERSION"
SANKORE_PACKAGE_DIRECTORY="$BASE_WORKING_DIR/usr/local/$SANKORE_DIRECTORY_NAME"
#move sankore build directory to packages directory
cp -R $BUILD_DIR $SANKORE_PACKAGE_DIRECTORY 


cat > $BASE_WORKING_DIR/usr/local/$SANKORE_DIRECTORY_NAME/run.sh << EOF
!/bin/bash
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

env LD_LIBRARY_PATH=/usr/local/$SANKORE_DIRECTORY_NAME/qtlib:$LD_LIBRARY_PATH /usr/local/$SANKORE_DIRECTORY_NAME/Open-Sankore
EOF


CHANGE_LOG_FILE="$BASE_WORKING_DIR/DEBIAN/changelog-sankore-$VERSION.txt"
CONTROL_FILE="$BASE_WORKING_DIR/DEBIAN/control"
CHANGE_LOG_TEXT="changelog.txt"

if [ "$ARCHITECTURE" == "x86_64" ]; then
    ARCHITECTURE="amd64"
fi

if [ "$ARCHITECTURE" == "i686" ]; then
    ARCHITECTURE="i386"
fi

echo "Open-Sankore ($VERSION) $ARCHITECTURE; urgency=low" > "$CHANGE_LOG_FILE"
echo >> "$CHANGE_LOG_FILE"
cat $CHANGE_LOG_TEXT >> "$CHANGE_LOG_FILE"
echo >> "$CHANGE_LOG_FILE"
echo "-- Claudio Valerio <claudio@open-sankore.org>  `date`" >> "$CHANGE_LOG_FILE"

echo "Package: open-sankore" > "$CONTROL_FILE"
echo "Version: $VERSION" >> "$CONTROL_FILE"
echo "Section: education" >> "$CONTROL_FILE"
echo "Priority: optional" >> "$CONTROL_FILE"
echo "Architecture: $ARCHITECTURE" >> "$CONTROL_FILE"
echo "Essential: no" >> "$CONTROL_FILE"
echo "Installed-Size: `du -s $SANKORE_PACKAGE_DIRECTORY | awk '{ print $1 }'`" >> "$CONTROL_FILE"
echo "Maintainer: Open-Sankoré Developers team <dev@open-sankore.org>" >> "$CONTROL_FILE"
echo "Homepage: http://dev.open-sankore.org" >> "$CONTROL_FILE"
echo -n "Depends: " >> "$CONTROL_FILE"
unset tab
declare -a tab
let count=0
for l in `objdump -p $SANKORE_PACKAGE_DIRECTORY/Open-Sankore | grep NEEDED | awk '{ print $2 }'`; do 
    for lib in `dpkg -S  $l | awk -F":" '{ print $1 }'`; do
        #echo $lib
        presence=`echo ${tab[*]} | grep -c "$lib"`; 
        if [ "$presence" == "0" ]; then   
            tab[$count]=$lib;
            ((count++));
        fi; 
    done; 
done; 

#additional dependencies
tab[$count]="gtk2-engines-pixbuf"
((count++))
tab[$count]="ttf-mscorefonts-installer"
((count++))

for ((i=0;i<${#tab[@]};i++)); do
    if [ $i -ne "0" ]; then
        echo -n ",    " >> "$CONTROL_FILE"
    fi
    echo -n "${tab[$i]} (>= "`dpkg -p ${tab[$i]} | grep "Version: " | awk '{      print $2 }'`") " >> "$CONTROL_FILE"
done
echo "" >> "$CONTROL_FILE"
echo "Description: This a interactive white board that uses a free standard format." >> "$CONTROL_FILE"

find $BASE_WORKING_DIR/usr/ -exec md5sum {} > $BASE_WORKING_DIR/DEBIAN/md5sums 2>/dev/null \; 
SANKORE_SHORTCUT="$BASE_WORKING_DIR/usr/share/applications/Open-Sankore.desktop"
echo "[Desktop Entry]" > $SANKORE_SHORTCUT
echo "Version=$VERSION" >> $SANKORE_SHORTCUT
echo "Encoding=UTF-8" >> $SANKORE_SHORTCUT
echo "Name=Open-Sankore ($VERSION)" >> $SANKORE_SHORTCUT
echo "GenericName=Open-Sankore" >> $SANKORE_SHORTCUT
echo "Comment=Logiciel de création de présentations pour tableau numérique interactif (TNI)" >> $SANKORE_SHORTCUT 
echo "Exec=/usr/local/$SANKORE_DIRECTORY_NAME/run.sh" >> $SANKORE_SHORTCUT
echo "Icon=/usr/local/$SANKORE_DIRECTORY_NAME/sankore.png" >> $SANKORE_SHORTCUT
echo "StartupNotify=true" >> $SANKORE_SHORTCUT
echo "Terminal=false" >> $SANKORE_SHORTCUT
echo "Type=Application" >> $SANKORE_SHORTCUT
echo "Categories=Education" >> $SANKORE_SHORTCUT
echo "Name[fr_FR]=Open-Sankore ($VERSION)" >> $SANKORE_SHORTCUT
cp "resources/images/uniboard.png" "$SANKORE_PACKAGE_DIRECTORY/sankore.png"
chmod 755 "$BASE_WORKING_DIR/DEBIAN"
chmod 755 "$BASE_WORKING_DIR/DEBIAN/prerm"
chmod 755 "$BASE_WORKING_DIR/DEBIAN/postint"

mkdir -p "install/linux"

rm install/linux/Open-Sankore_*.deb

fakeroot  chown -R root:root $BASE_WORKING_DIR 
dpkg -b "$BASE_WORKING_DIR" install/linux/Open-Sankore_${VERSION}_$ARCHITECTURE.deb
notify-send "Open-Sankore" "Package built"

#clean up mess
fakeroot rm -rf $BASE_WORKING_DIR


#######################################################################
#                             tar.gz                                  #
#######################################################################
echo `pwd`
cp -R $RELEASE_DIR/product $RELEASE_DIR/Open-Sankore.$VERSION
cd $RELEASE_DIR

rm ../../../install/linux/Open-Sankore.tar.gz

tar cvzf ../../../install/linux/Open-Sankore.tar.gz Open-Sankore.$VERSION -C . 
notify-send "Open-Sankore"  "tar.gz Build done"