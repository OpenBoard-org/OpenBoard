﻿@echo off
@echo off
REM --------------------------------------------------------------------
REM This program is free software: you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation, either version 2 of the License, or
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

set SCRIPT_PATH=%~dp0
set PROJECT_ROOT=%SCRIPT_PATH%\..\..

set APPLICATION_NAME=OpenBoard
set QT_DIR=D:\qt\x64-windows
set QT_BIN=%QT_DIR%\tools\Qt6\bin

set PROGRAMS_FILE_PATH=C:\Program Files (x86)

set GIT_BIN=C:\Program Files\Git\bin
set VS_BIN=%PROGRAMS_FILE_PATH%\Microsoft Visual Studio 14.0\VC\bin
set WIN_SDK_BIN=%PROGRAMS_FILE_PATH%\Microsoft SDKs\Windows\v6.0A\Bin
set INNO_EXE=%PROGRAMS_FILE_PATH%\Inno Setup 6\iscc.exe
set BUILD_DIR=%PROJECT_ROOT%\build\win32\release
set LRELEASE=C:\Qt\6.7.2\msvc2019_64\bin\lrelease.exe
set BASE_QT_TRANSLATIONS_DIRECTORY=%QT_DIR%\translations\Qt6

set PATH=%QT_BIN%;%PATH%;%WIN_SDK_BIN%;%GIT_BIN%

REM call "%VS_BIN%\vcvars32.bat"

echo %PATH%

cd %PROJECT_ROOT%

rmdir /S /Q %BUILD_DIR%
rmdir /S /Q install

"%QT_BIN%\qmake.exe" %APPLICATION_NAME%.pro

call "%LRELEASE%" "%APPLICATION_NAME%.pro"

set /p VERSION= < build\win32\release\version
REM remove the last character that is a space
set VERSION=%VERSION: =%


nmake release-install
IF NOT EXIST build\win32\release\product\%APPLICATION_NAME%.exe GOTO EXIT_WITH_ERROR

xcopy C:\%APPLICATION_NAME%\bin\*.dll build\win32\release\product\
xcopy "%QT_DIR%\bin\Qt6OpenGL.dll" build\win32\release\product\

set CUSTOMIZATIONS=build\win32\release\product\customizations
mkdir %CUSTOMIZATIONS%
xcopy /s resources\customizations %CUSTOMIZATIONS%

set STARTUP_HINTS=build\win32\release\product\startupHints
mkdir %STARTUP_HINTS%
xcopy /s resources\startupHints %STARTUP_HINTS%

set I18n=build\win32\release\product\i18n
xcopy /s "%BASE_QT_TRANSLATIONS_DIRECTORY%\qt_*.qm" %I18n%\

call "%INNO_EXE%" "%SCRIPT_PATH%\%APPLICATION_NAME%.iss" /F"%APPLICATION_NAME%_Installer_%VERSION%"

GOTO END

:EXIT_WITH_ERROR
echo "Error found"
GOTO EOF

:END
echo "%APPLICATION_NAME% build finished"

:EOF
