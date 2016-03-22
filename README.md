# OpenBoard
OpenBoard is an open source cross-platform interactive white board application designed for use in schools. It is a fork of Open-Sankoré, which was itself based on Uniboard.

Supported platforms are Windows (7+), OS X (10.9+) and Linux (tested on Ubuntu 14.04, but should work with other distributions too).

# Dependencies

The latest version (1.3) requires Qt 5.5. See below for Linux-specific instructions.

OpenBoard makes use of several third-party libraries, which are available in the OpenBoard-ThirdParty repository. These should be built first; instructions are provided for each library.

## Linux

### Qt
Due to a shared library conflict within Qt5 on Linux (the Qt Multimedia and Qt Webkit modules are built against different versions of gstreamer by default), a specific installation of Qt5.5 is needed for all of OpenBoard's features to work correctly.

It can either be built from source, with the configure flag `-gstreamer 1.0` (see [here](http://doc.qt.io/qt-5/linux-building.html)), or installed from Stephan Binner's PPAs on Ubuntu.
In the latter case, simply add the repositories and install Qt 5.5.1 like so:

    sudo add-apt-repository ppa:beineri/opt-qt551-trusty
    sudo apt-get update
    sudo apt-get install qt-latest

### Onboard
OpenBoard has a built-in virtual keyboard, but also allows the use of an external OSK (which is now the default, as the built-in keyboard will likely be removed in a future release). 
On Linux, Onboard is currently used as the external OSK. In case you can't or won't install it, you can simply use the built-in OSK instead. 

# Installation & Deployment

Deployment scripts are provided for all three platforms. These take care of compiling OpenBoard, including the translations (for OpenBoard and for Qt), stripping the debug symbols, creating the installers etc.
Minor modification to those scripts may be necessary depending on your configuration, to set the correct Qt path for example.


