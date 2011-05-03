#!/bin/sh

make clean
rm -rf build/linux/release/

/usr/bin/qmake-qt4

VERSION=`cat build/linux/release/version`
if [ "$VERSION" = "" ]; then
    echo "version not found"
    exit 1
else
    LAST_COMMITED_VERSION="`git describe $(git rev-list --tags --max-count=1)`"
    if [ "v$VERSION" != "$LAST_COMMITED_VERSION" ]; then
	echo creating a tag with the version $VERSION
        git tag -a "v$VERSION"
	git push origin --tags 
    else
	if [ "$1" != "escape" ] ; then
	    echo "if you have already compiled a release (e.g. on a different os) please use the fallowing command line"
	    echo sh release.linux.sh escape
	    exit 2
	fi
    fi
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
