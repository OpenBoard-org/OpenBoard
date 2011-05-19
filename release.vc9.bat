set QT_DIR=C:\Qt\4.7.1
set QT_BIN=%QT_DIR%\bin

set VS_BIN="C:\Program Files\Microsoft Visual Studio 9.0\VC\bin"
set WIN_SDK_BIN="C:\Program Files\Microsoft SDKs\Windows\v6.1\Bin"
set INNO_EXE="C:\Program Files\Inno Setup 5\iscc.exe "
set BUILD_DIR=build\win32\release
set UB_DATA_DIR="D:\"

set PATH=%QT_BIN%;%PATH%;%WIN_SDK_BIN%

call %VS_BIN%\vcvars32.bat

GOTO END_COMMENT1
rmdir /S /Q %BUILD_DIR%
:END_COMMENT1

set EDITION=MNEMIS_EDITION

qmake "DEFINES+=%EDITION%"

set /p VERSION= < build\win32\release\version
git rev-list --tags --max-count=1 > tmp
set /p LAST_TAG= < tmp
erase tmp
git describe %LAST_TAG% > tmp
set /p LAST_TAG_VERSION=< tmp
erase tmp

echo %VERSION%
echo %LAST_TAG_VERSION%

if not v%VERSION%==%LAST_TAG_VERSION% GOTO EXIT_WITH_ERROR


nmake release-install

.\thirdparty\google-breakpad\r318\bin\win32\dump_syms.exe .\build\win32\release\product\Sankore 3.1.pdb > "%UB_DATA_DIR%\releases\uniboard\sym\win32\Sankore 3.1.exe\%LONG_VERSION%%EDITION%.sym"

del .\build\win32\release\product\Sankore 3.1.pdb

set INSTALLER_NAME=Sankore 3.1

set INSTALLER_PATH=.\install\win32\%INSTALLER_NAME%.exe

call %INNO_EXE% "Sankore 3.1.iss" /F"%INSTALLER_NAME%"

:EXIT_WITH_ERROR
	echo version %VERSION%
	echo last tag version %LAST_TAG_VERSION%
