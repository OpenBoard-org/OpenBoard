set QT_BIN=C:\Qt\4.5.0\bin

set VS_BIN="C:\Program Files\Microsoft Visual Studio 9.0\VC\bin"
set INNO_EXE="C:\Program Files\Inno Setup 5\iscc.exe "
set BUILD_DIR=".\build\win32\release"

set PATH=%QT_BIN%;%PATH%;

call %VS_BIN%\vcvars32.bat

nmake clean

rmdir /S /Q %BUILD_DIR%

qmake

nmake all
