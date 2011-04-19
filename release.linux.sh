#!/bin/sh

SVN_REVISION=`svnversion`
# only accept up to date, non modified, non switched svn revisions
if echo $SVN_REVISION | grep -q [:MS]
then
    echo "bad subversion revision ($SVN_REVISION)"
#    exit 1
fi

make clean
rm -rf build/linux/release/

#/usr/local/Trolltech/Qt-4.6.1/bin/qmake
/usr/bin/qmake-qt4

VERSION=`cat build/linux/release/version`
if [ "$VERSION" = "" ]; then
    echo "version not found"
    exit 1
fi

make -j 4 release-install

cp resources/linux/run.sh build/linux/release/product
chmod +x build/linux/release/product/run.sh

cp -R resources/linux/qtlinux/* build/linux/release/product/

rm -rf install/linux
mkdir -p install/linux

mv build/linux/release/product build/linux/release/Sankore_3.1.$VERSION.$SVN_REVISION
cd build/linux/release

# "Removing .svn directories ..."
find . -name .svn -exec rm -rf {} \; 2> /dev/null
tar cvzf ../../../install/linux/Sankore_3.1.$VERSION.$SVN_REVISION.tar.gz Sankore_3.1.$VERSION.$SVN_REVISION -C .
