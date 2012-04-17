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


make clean
rm -rf build/linux/release/
rm -rf install

QT_PATH="/usr/local/Trolltech/Qt-4.7.3"
PLUGINS_PATH="$QT_PATH/plugins"
QMAKE_PATH="$QT_PATH/bin/qmake"
LRELEASES="/usr/local/Trolltech/Qt-4.7.3/bin/lrelease"
QT_GUI_TRANSLATIONS_DIRECTORY="../Qt-sankore3.1/translations"
QT_GUI_TRANSLATIONS_PRO_FILE="$QT_GUI_TRANSLATIONS_DIRECTORY/translations.pro"

if [ ! -e "$QMAKE_PATH" ]; then
    echo "qmake command not found at $QMAKE_PATH"
    exit 1
fi


if [ ! -e "$PLUGINS_PATH" ]; then
    echo "plugins path not found at $PLUGINS_PATH"
    exit 1
fi

ARCHITECTURE=`uname -m`
if [ "$ARCHITECTURE" == "x86_64" ]; then
    $QMAKE_PATH -spec linux-g++-64
else
    $QMAKE_PATH -spec linux-g++
fi

make -j 4 release-install


if [ ! -f $QT_GUI_TRANSLATIONS ]; then 
    echo "impossible to create and integrate the qt gui translation"
    exit 1
else
    $LRELEASES $QT_GUI_TRANSLATIONS_PRO_FILE
elif

VERSION=`cat build/linux/release/version`
if [ ! -f build/linux/release/version ]; then
    echo "version not found"
    exit 1
else
    LAST_COMMITED_VERSION="`git describe $(git rev-list --tags --max-count=1)`"
    if [ "v$VERSION" != "$LAST_COMMITED_VERSION" ]; then
        echo creating a tag with the version $VERSION
        git tag -a "v$VERSION" -m "Generating setup for v$VERSION"
        git push origin --tags 
    fi
fi

cp resources/linux/run.sh build/linux/release/product
chmod +x build/linux/release/product/run.sh

cp -R resources/linux/qtlinux/* build/linux/release/product/

#copying plugins
cp -R $PLUGINS_PATH build/linux/release/product/
#removing debug version
find build/linux/release/product/ -name *.debug -exec rm {} \;

#copying custom qt library
QT_LIBRARY_DEST_PATH="build/linux/release/product/qtlib"
mkdir $QT_LIBRARY_DEST_PATH
QT_LIBRARY_SOURCE_PATH="$QT_PATH/lib"

copyQtLibrary(){
    if [ ! -e "$QT_LIBRARY_SOURCE_PATH/$1.so.4.7.3" ]; then
        echo "library not found: $QT_LIBRARY_SOURCE_PATH"
        exit 1
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

cp "$QT_LIBRARY_SOURCE_PATH/libphonon.so.4" "$QT_LIBRARY_DEST_PATH/"
cp "$QT_LIBRARY_SOURCE_PATH/libphonon.so.4.4.0" "$QT_LIBRARY_DEST_PATH/"

if [ ! -e $GUI_TRANSLATIONS_DIRECTORY_PATH ]; then
    notifyError "gui translations pro file not found at: $GUI_TRANSLATIONS_DIRECTORY_PATH"
else
    cd $GUI_TRANSLATIONS_DIRECTORY_PATH
    $LRELEASES translations.pro
    cd -
    if [ ! -e build/linux/release/product/i18n ]; then
        mkdir build/linux/release/product/i18n
    fi
    #copying qt gui translation    
    cp $GUI_TRANSLATIONS_DIRECTORY_PATH/qt_??.qm build/linux/release/product/i18n/
fi

$LRELEASES Sankore_3.1.pro
cp resources/i18n/*.qm build/linux/release/product/i18n/


rm -rf install/linux
mkdir -p install/linux

mv build/linux/release/product build/linux/release/Open-Sankore.$VERSION
cd build/linux/release

# "Removing .svn directories ..."
find . -name .svn -exec rm -rf {} \; 2> /dev/null
tar cvzf ../../../install/linux/Open-Sankore.tar.gz Open-Sankore.$VERSION -C . 
notify-send "Open-Sankore"  "Build Finished"
