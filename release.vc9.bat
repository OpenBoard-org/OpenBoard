set QT_DIR=C:\Qt\4.7.1
set QT_BIN=%QT_DIR%\bin

set VS_BIN="C:\Program Files\Microsoft Visual Studio 9.0\VC\bin"
set WIN_SDK_BIN="C:\Program Files\Microsoft SDKs\Windows\v6.1\Bin"
set INNO_EXE="C:\Program Files\Inno Setup 5\iscc.exe "
set BUILD_DIR=build\win32\release
set SVN_BIN="C:\Program Files\Subversion\bin"
set UB_DATA_DIR="D:\"

set PATH=%QT_BIN%;%PATH%;%WIN_SDK_BIN%;%SVN_BIN%

call %VS_BIN%\vcvars32.bat

GOTO END_COMMENT1
rmdir /S /Q %BUILD_DIR%
:END_COMMENT1

set EDITION=MNEMIS_EDITION
set EDITION_LABEL=

if %2.==Wacom. set EDITION=WACOM_EDITION 
if %2.==Wacom. set EDITION_LABEL=Wacom Edition 

qmake "DEFINES+=%EDITION%"

set VERSION=1
set LONG_VERSION=1.0.a.0
set SVN_VERSION=0

nmake release-install

.\thirdparty\google-breakpad\r318\bin\win32\dump_syms.exe .\build\win32\release\product\Sankore 3.1.pdb > "%UB_DATA_DIR%\releases\uniboard\sym\win32\Sankore 3.1.exe\%LONG_VERSION%%EDITION%.sym"

del .\build\win32\release\product\Sankore 3.1.pdb

set INSTALLER_NAME=Sankore 3.1 %VERSION%setup

set INSTALLER_PATH=.\install\win32\%INSTALLER_NAME%.exe

call %INNO_EXE% "Sankore 3.1.iss" /F"%INSTALLER_NAME%"
