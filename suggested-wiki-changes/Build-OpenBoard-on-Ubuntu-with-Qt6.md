These instructions are based on trial-and-error builds on Ubuntu 23.10 and pre-release nighly builds of Ubuntu 24.04.

[OpenBoard's default instructions](Build-OpenBoard-on-Ubuntu.md) 
for building their software from source are based on 
Qt 5.15.X, which was the default Qt for Ubuntu 2022.4 LTS.  

With the upcoming release of Ubuntu 2024.4, the 
[Qt Group end of life policy](https://www.qt.io/blog/qt-5.15-extended-support-for-subscription-license-holders) 
makes this Qt version less than ideal (final patch release for extended 
support subscribers will go out in May 2025, users who don't have an 
extended support agreement are already out of support as at March 2024).  

OpenBoard's primary README.md reports that the software can be built against Qt6, this 
page describes one possible process for doing this on recent and future Ubuntu 
(i.e. the non-LTS 2023.10, the pre-release nightly builds of candidates to become 
2024.04 LTS).  The process will the hopefully be applicable to 2024.04 LTS releases when 
they become available during April 2024.

Openboard's instructions for building from Qt 5.15.3 include the following note:

> **Important note** : The QtWebEngine binary provided by Qt does not come with proprietary codecs enabled, which means that you won't be able to play videos using a > proprietary codec for sound or video. Some major video platforms like Vimeo and PeerTube won't work. If you want them to work, you'll have to build QtWebEngine from > source. To do so, follow [this link](https://github.com/OpenBoard-org/OpenBoard/wiki/Build-Qt-WebEngine-on-Ubuntu-20.04)

Presumably a new version of the QtWebEngine build document will be required for users
who require a build under with proprietary codec support under Qt6, this new document 
version does not yet exist. 

## Prepare your environment

### Install Qt6, qmake6 and OpenBoard dependencies

OpenBoard's instructions for building against Qt5.15.X provide separate sections for
installation of Qt, qmake and the dependencies of the OpenBoard software itself.

For simplicity, all installations are bundled in a single command here:

```
sudo apt install build-essential libgl1-mesa-dev libssl-dev libpoppler-dev libpoppler-cpp-dev libpoppler-private-dev libavcodec-dev libavformat-dev libswscale-dev libpaper-dev libva-dev libxcb-shm0-dev libasound2-dev libx264-dev libvpx-dev libvorbis-dev  libtheora-dev libmp3lame-dev libsdl1.2-dev libopus-dev  libfdk-aac-dev libass-dev liblzma-dev libbz2-dev libquazip5-dev libxcb-shape0-dev libxcb-xfixes0-dev qt6-tools-dev qt6-svg-dev qt6-multimedia-dev qt6-webengine-dev
```

After this installation has completed, run the following command to verify the version number and home directory for the Ubuntu Qt build:

```
qmake6 -v
```

on the system(s) where this process has been tested to date this command reports something like:

```
QMake version 3.1
Using Qt version 6.4.2 in /usr/lib/x86_64-linux-gnu
```

## Download OpenBoard sources
```
# place yourself in the directory you want 
# (in this example at user's home directory)
cd

# create the folder
mkdir openboard

# enter in it
cd openboard

# download openboard sources
https://github.com/tim-littlefair/OpenBoard.git
```

**Important note** : A pull request will be submitted to the upstream OpenBoard project
containing this document and the minor changes to OpenBoard.pro required to get the build 
working - if/when the upstream maintainers accept and merge this PR the sources should
be downloaded using 
```
git clone https://github.com/OpenBoard-org/OpenBoard.git
```
instead

## Compile OpenBoard



Once the installations and git clone operation are complete, you should be
able to continue by running the following commands:
```
qmake6 OpenBoard.pro
make
``` 

and then continuing with
[the compile instructions in OpenBoard's original document](Build-OpenBoard-on-Ubuntu.md#compile-openboard).

As of the current version the following error messages are returned from the 'qmake6' command:
> Info: creating stash file /home/tim/github/OpenBoard/.qmake.stash
> Cannot read /home/tim/github/OpenBoard/version.txt: No such file or directory
> /bin/sh: 1: Syntax error: "&" unexpected
> /bin/sh: 1: Syntax error: "&" unexpected
> /bin/sh: 1: Syntax error: "&" unexpected
These do not appear to reflect a fatal error - qmake6 generates a 
Makefile and the 'make' command succeeds.


