REM --------------------------------------------------------------------
REM This program is free software: you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation, either version 3 of the License, or
REM (at your option) any later version.
REM 
REM This program is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM 
REM You should have received a copy of the GNU General Public License
REM along with this program.  If not, see <http://www.gnu.org/licenses/>.
REM ---------------------------------------------------------------------

set QT_DIR=..\Qt-sankore3.1
set QT_BIN=%QT_DIR%\bin

set PROGRAM_FILES_PATH=C:\Program Files (x86)

set GIT_BIN=%PROGRAM_FILES_PATH%\Git\bin
set VS_BIN=%PROGRAM_FILES_PATH%\Microsoft Visual Studio 9.0\VC\bin
set WIN_SDK_BIN=%PROGRAM_FILES_PATH%\Microsoft SDKs\Windows\v6.0A\Bin
set INNO_EXE=%PROGRAM_FILES_PATH%\Inno Setup 5\iscc.exe 

set BUILD_DIR=build\win32\release

set PATH=%QT_BIN%;%PATH%;%WIN_SDK_BIN%;%GIT_BIN%

call "%VS_BIN%\vcvars32.bat"

echo %PATH%

REM this checks if the custom qt directory path
REM is correct. This is important because installer
REM pick up dll from this directory
IF NOT EXIST "..\Qt-sankore3.1\lib\QtCore4.dll" GOTO EXIT_WITH_ERROR

rmdir /S /Q %BUILD_DIR%

set EDITION=MNEMIS_EDITION

"%QT_BIN%\qmake.exe" "DEFINES+=%EDITION%"

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

del ".\build\win32\release\product\Sankore.pdb"

set INSTALLER_NAME=Sankore

set INSTALLER_PATH=.\install\win32\%INSTALLER_NAME%.exe

call "%INNO_EXE%" "Sankore.iss" /F"%INSTALLER_NAME%"

:EXIT_WITH_ERROR
	echo ERROR
