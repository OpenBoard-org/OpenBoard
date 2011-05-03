#!/bin/sh

make clean
rm -rf build/linux/release/

/usr/bin/qmake-qt4

make -j 4 release-install

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

cp -R /usr/lib/qt4/plugins build/linux/release/product/

rm -rf install/linux
mkdir -p install/linux

mv build/linux/release/product build/linux/release/Sankore_3.1.$VERSION
cd build/linux/release

# "Removing .svn directories ..."
find . -name .svn -exec rm -rf {} \; 2> /dev/null
tar cvzf ../../../install/linux/Sankore_3.1.$VERSION.tar.gz Sankore_3.1.$VERSION -C .
