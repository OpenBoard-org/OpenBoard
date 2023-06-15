# Build OpenBoard on Windows 10 (1.7-dev)

## ⚠️Disclaimer⚠️

These instructions only work for the upcoming version : OpenBoard 1.7. At the time you are reading, you need to place yourself on the ```dev``` branch (explained below). If you want to build OpenBoard 1.6.4, you'll need to install QtWebKit instead of QtWebEngine, but we strongly recommend you to use the QtWebEngine library and the ``dev`` branch. If you still want to build OpenBoard 1.6.4, please refer to the instructions written for OpenBoard 1.6.4



## Prepare your environment

### Install git bash
 - [https://gitforwindows.org/](https://gitforwindows.org)
### Install Qt
go to Qt downloads page and follow the instructions to download the opensource version of Qt

 - [https://www.qt.io/download-open-source](https://www.qt.io/download-open-source)
 - at the bottom of the page, click on the "Download the Qt Online Installer" button
 - on the next page, Qt chose automatically the version corresponding to your current OS, so be sure it is the version you want to download (for this example, "Qt Online Installer for Windows (64-bit)")
 - click on the "Download" button
Follow the instructions (for this example, Qt is installed in C:\ and pick the <Qt_version> you want (for example 5.15.2).

**Important Note** : The Qt WebEngine binary provided by Qt does not come with proprietary codecs enabled, which means that you won't be able to play videos using a proprietary codec for sound or video. Some major video platforms like Vimeo and PeerTube won't work. If you need proprietary-codecs to be enabled, please follow [these instructions](https://github.com/OpenBoard-org/OpenBoard/wiki/Build-Qt-WebEngine-on-Windows-10), instead of picking Qt Webengine in the following lines.

In the Qt version you'll select, you'll need to pick :

- Desktop msvc2019
- Qt WebEngine (if not building Qt WebEngine with proprietary codecs)
- Qt Debug Information Files (recommended, but not mandatory)

For the rest of this tutorial, we supposed Qt is installed at "C:\Qt"

### Add qmake to environment variables
 
- Add qmake folder to environment variables :
  - search for "Environment Variables" on Windows searchbar, click on it.
  - click on the last button at the bottom of the page, called "environment variables"
  - on the "system variables" window, add the Qt bin folder - where qmake.exe, using msvc 2019, is located - to the ``Path`` variable (for example "C:\Qt\5.14.2\msvc2019_64\bin")

  
You can verify that you correctly configured qmake and nmake by respectively taping qmake -v and nmake/? these commands should show something like following :


```C:\Program Files (x86)\Microsoft Visual Studio\2019\Community>qmake -v
QMake version x.y
Using Qt version <Qt_version> in <path_to_your_Qt_folder>/<Qt_version>/msvc2019_64/lib

C:\Program Files (x86)\Microsoft Visual Studio\2019\Community>nmake /?
Microsoft (R) Program Maintenance Utility Version 14.26.27034.0
Copyright (C) Microsoft Corporation. Tous droits réservés.


````
### Download Qt .dll libraries
Navigate to the folder where Qt is located (for example "C:\Qt\<Qt_version>\msvc2019_64\bin")

Check for the existence of the following libraries within the folder:

- [x] icuuc65.dll
- [x] icuin65.dll
- [x] icudt65.dll
- [x] libxslt.dll
- [x] libxml2.dll

If they don't exist, download them:

- [icuuc65.dll](https://www.dll-files.com/search/?q=icuuc65.dll)
- [icuin65.dll](https://www.dll-files.com/search/?q=icuin65)
- [icudt65.dll](https://www.dllme.com/dll/files/icudt65/4a9d4fbcd00508cd3de6f252310dcd68)
- [libxslt.dll](https://www.dllme.com/dll/files/libxslt)
- [libxml2.dll](https://www.dllme.com/dll/files/libxml2)

copy their .dll files into the installed Qt directory ("C:\Qt\<Qt_version>\msvc2019_64\bin")





## Download OpenBoard sources
- Open git bash

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
git clone https://github.com/OpenBoard-org/OpenBoard-Importer.git
git clone https://github.com/OpenBoard-org/OpenBoard-ThirdParty.git

# Place yourself on the dev branch
cd OpenBoard
git checkout dev

````

## Compile OpenBoard

First, you need to compile the third-party libraries that OpenBoard is going to need.

Still in this Command Prompt, go to Openboard-ThirdParty's directory

```
cd <path_to_openboard>
cd Openboard-ThirdParty
````
### Freetype

```
cd <path_to_openboard>
cd Openboard-ThirdParty
````

### quazip

```
cd quazip
qmake quazip.pro
nmake
````

### xpdf

```
cd xpdf
qmake xpdf.pro
nmake
````

### OpenBoard
Now, you can go back to OpenBard's directory

```
cd <path_to_openboard>
cd Openboard
````
You can now compile OpenBoard

```
qmake OpenBoard.pro
nmake
```
## Package OpenBoard

### Install Inno Setup

- Download the installer of Inno Setup 6 => [https://jrsoftware.org/download.php/is.exe](https://jrsoftware.org/download.php/is.exe)
- Launch it and follow the instructions

### Configure the packaging script
Go to :
```
cd release_scripts\windows
```
Open release.win7.vc9.bat in your favourite text editor, and modify these lines to point to the correct locations :
```
#In our example
set QT_DIR=C:\Qt\5.15.2\msvc2019_64
 
#...
set INNO_EXE=%PROGRAMS_FILE_PATH%\Inno Setup 6\iscc.exe
```

### Launch the packaging

```
release.win7.vc9.bat
```
Now, you should see your OpenBoard installer in `<path_to_openboard>/Openboard/install/windows>`
