These instructions were written for and tested on Ubuntu 22.04 but should at least guide you on any other Debian-like distribution. 

## Prepare your environment
### Install Qt
We'll use Ubuntu's default version of Qt (Qt 5.15.3).

If you want or need to install another version of Qt, please refer to [this section](#install-another-version-of-qt)
### Install qmake

```
sudo apt install qt5-qmake
```
If you type `qmake -v` on a terminal, it should returns something like this :
```
QMake version x.y
Using Qt version <Qt_version> in <path_to_your_Qt_lib_folder>/lib
```

## Install OpenBoard dependencies
```
# what is going to be installed :
- build-essential # Informational list of build-essential packages (needed to build debian (.deb) packages)
- libgl1-mesa-dev # free implementation of the OpenGL API -- GLX development files
- libssl-dev # Secure Sockets Layer toolkit - development files
- libpoppler-dev # PDF rendering library -- development files
- libpoppler-cpp-dev # PDF rendering library -- development files (CPP interface)
- libpoppler-private-dev # PDF rendering library -- private development files
- libavcodec-dev libavformat-dev libswscale-dev libpaper-dev libva-dev libxcb-shm0-dev
  libasound2-dev libx264-dev libvpx-dev libvorbis-dev  libtheora-dev libmp3lame-dev 
  libsdl1.2-dev libopus-dev  libfdk-aac-dev libass-dev liblzma-dev libbz2-dev # ffmpeg libraries and associated dependencies
- libquazip5-dev # C++ wrapper for ZIP/UNZIP (development files)
- libxcb-shape0-dev # X C Binding, shape extension, development files
- libxcb-xfixes0-dev # X C Binding, xfixes extension, development files
- libqt5svg5-dev qttools5-dev qtmultimedia5-dev qtwebengine5-dev # Additional Qt libs needed for OpenBoard

# execute the following command line to install all of the above

sudo apt install build-essential libgl1-mesa-dev libssl-dev libpoppler-dev libpoppler-cpp-dev libpoppler-private-dev libavcodec-dev libavformat-dev libswscale-dev libpaper-dev libva-dev libxcb-shm0-dev libasound2-dev libx264-dev libvpx-dev libvorbis-dev  libtheora-dev libmp3lame-dev libsdl1.2-dev libopus-dev  libfdk-aac-dev libass-dev liblzma-dev libbz2-dev libquazip5-dev libxcb-shape0-dev libxcb-xfixes0-dev libqt5svg5-dev qttools5-dev qtmultimedia5-dev qtwebengine5-dev

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
git clone https://github.com/OpenBoard-org/OpenBoard.git
```

## Compile OpenBoard
``` 
cd <path_to_openboard>
cd Openboard
```

Then, you can compile OpenBoard
```
qmake OpenBoard.pro
make
``` 

If you want to populate OpenBoard's library with its default applications, sounds, and so on, you'll have to copy them from the `resources` directory to the location of the executable : 

```
cp -r resources/library build/linux/release/product/
```

If you want to use different translations : 
```
cp -r resources/i18n build/linux/release/product/
```

You will find the executable in the following directory : 
```
cd build/linux/release/product/
```

and can execute the version of OpenBoard you compiled yourself like following : 
```
# supposing you placed yourself in build/linux/release/product/

./OpenBoard

# or in another language
./OpenBoard -lang ja
```

## Package OpenBoard
```
cd release_scripts
cd linux
```

### If you installed another version of Qt
First, open build.sh and package.sh in your favorite text editor, and modify this line to point to the correct Qt path :
```
 QT_PATH="<path_to_your_Qt_folder>/<Qt_version>/gcc_64"
```

### Execute the scripts

Launch the build
```
sudo ./build.sh
```

Finally, launch the packaging
```
sudo ./package.sh
```

Now, you should see your OpenBoard package in `<path_to_openboard>/Openboard/install/linux>`

That's all ! Well done !


--------------------------------
## Install another version of Qt
#### Install Qt
If you want to install another version of Qt, go to Qt downloads page and follow the instructions to download the opensource version of Qt

- https://www.qt.io/download-open-source
- at the bottom of the page, click on the "Download the Qt Online Installer" button
- on the next page, Qt chose automatically the version corresponding to your current OS, so be sure it is the version you want to download (for this example, "Qt Online Installer for Linux (64-bit)")
- click on the "Download" button

You need to add the right to your user to execute this file : 
```
#for this example, I had to execute the followings commands
cd
cd Downloads
chmod u+x qt-unified-linux-x64-3.1.1-online.run
# launch the installer (and follow the instructions) by executing this command :
./qt-unified-linux-x64-3.1.1-online.run
```

Follow the instructions and pick the <Qt_version> you want (for example 6.3.2). You'll need : 
* Desktop gcc 64-bit
* Qt Multimedia
* Qt WebEngine
* Qt Positioning
* Qt WebChannel
* Qt 5 Compatibility Module (for Qt 6 only)
* Qt Debug Information Files (recommended but not mandatory)


**Important note** : The QtWebEngine binary provided by Qt does not come with proprietary codecs enabled, which means that you won't be able to play videos using a proprietary codec for sound or video. Some major video platforms like Vimeo and PeerTube won't work. If you want them to work, you'll have to build QtWebEngine from source. To do so, follow [this link](https://github.com/OpenBoard-org/OpenBoard/wiki/Build-Qt-WebEngine-on-Ubuntu-20.04)

#### Install qtchooser

```
sudo apt install qtchooser
```

```
#replace the <path_to_your_Qt_folder> by the correct value (for example, "/home/dev/Qt")
#replace the <Qt_version> by the correct value (for example, "5.15.2")
echo "<path_to_your_Qt_folder>/<Qt_version>/gcc_64/bin/" >> default.conf
echo "<path_to_your_Qt_folder>/<Qt_version>/gcc_64/lib/" >> default.conf
```

Finally, move it to the qtchooser directory : 
```
sudo mv default.conf /usr/lib/x86_64-linux-gnu/qtchooser/
```

If you type `qmake -v` on a terminal and it should returns something like this :
```
QMake version x.y
Using Qt version <Qt_version> in <path_to_your_Qt_lib_folder>/lib
```

When you successfully installed Qt, go back to [this section](#install-openboard-dependencies)

