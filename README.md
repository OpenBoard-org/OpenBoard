# OpenBoard
OpenBoard is an open source cross-platform interactive white board application designed primarily for use in schools. It was originally forked from Open-Sankoré, which was itself based on Uniboard.

Supported platforms are Windows (7+), OS X (10.9+) and Linux (tested on Ubuntu 16.04).


## Installing

Installers are available for Windows, OS X and Ubuntu on the [wiki](https://github.com/DIP-SEM/OpenBoard/wiki/Downloads). 

## Building from source
First, obtain the third party libraries from the OpenBoard-ThirdParty repository, and build them (instructions are provided for each library).

Then, you may use the build (and packaging) scripts which are provided for all three platforms. These take care of compiling OpenBoard, including the translations (for OpenBoard and for Qt), stripping the debug symbols, creating the installers etc.
Minor modification to those scripts may be necessary depending on your configuration, to set the correct Qt path for example.

Alternatively, you can easily build OpenBoard with qmake and make:

    qmake OpenBoard.pro -spec linux-g++-64      # replace linux-g++-64 by macx or win32 for other platforms
    make

Compilers used are gcc (Linux), clang (OS X) and MSVC 2010 (Windows). Make sure that your version of Qt matches this, as it is not possible e.g to build OpenBoard with clang if Qt was built with gcc.

## Dependencies
The latest version (1.5) requires Qt 5.5. (While it has been shown to mostly work with Qt 5.2, we cannot guarantee compatibility with Qt versions other than 5.5.)

### Qt 5.5 on Linux

Due to a shared library conflict within Qt 5 in some distributions / some Qt versions (the Multimedia and Webkit modules were built against different versions of gstreamer by default), a specific installation of Qt5.5 may be needed for all of OpenBoard's features to work correctly.

It can either be built from source, with the configure flag `-gstreamer 1.0` (see [here](http://doc.qt.io/qt-5/linux-building.html)), or installed from Stephan Binner's PPAs on Ubuntu.
In the latter case, simply add the repositories and install Qt 5.5.1 like so (example provided for Ubuntu 14.04, aka "Trusty"):

    sudo add-apt-repository ppa:beineri/opt-qt551-trusty
    sudo apt-get update
    sudo apt-get install qt-latest

Some distributions, such as Ubuntu 16.04, provide Qt 5.5.1 packages that work perfectly with OpenBoard, so you can simply install Qt from the official repository.
