# Run This Local Windows Build

This file documents the local Windows setup used to build and run this modified OpenBoard checkout.

## Quick Run

The app has already been built and deployed here:

```powershell
D:\Ai\product\own\OpenBoard\OpenBoard\build\win32\release\product\OpenBoard.exe
```

Double-click `OpenBoard.exe`, or run:

```powershell
cd D:\Ai\product\own\OpenBoard\OpenBoard\build\win32\release\product
.\OpenBoard.exe
```

## What Was Installed

This local build uses:

```text
Qt 6.8.3 MSVC 2022 x64: D:\Ai\Qt\6.8.3\msvc2022_64
OpenBoard source:        D:\Ai\product\own\OpenBoard\OpenBoard
Third-party libraries:   D:\Ai\product\own\OpenBoard\OpenBoard-ThirdParty
Windows SDK:             10.0.18362
Visual Studio Build Tools
```

## Rebuild After Code Changes

Open a normal PowerShell terminal and run:

```powershell
cd D:\Ai\product\own\OpenBoard\OpenBoard

cmd.exe /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64 && "D:\Ai\Qt\6.8.3\msvc2022_64\bin\qmake.exe" OpenBoard.pro && nmake'
```

If `quazip.lib` is missing, rebuild QuaZip first:

```powershell
cmd.exe /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64 && cd /d "D:\Ai\product\own\OpenBoard\OpenBoard-ThirdParty\quazip" && "D:\Ai\Qt\6.8.3\msvc2022_64\bin\qmake.exe" quazip.pro && nmake release'
```

Then rebuild OpenBoard again.

## Deploy Runtime Files

After a successful build, copy OpenBoard resources and deploy Qt dependencies:

```powershell
cd D:\Ai\product\own\OpenBoard\OpenBoard

cmd.exe /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64 && cd /d "D:\Ai\product\own\OpenBoard\OpenBoard" && nmake install'

& D:\Ai\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe --release --compiler-runtime --qmldir src build\win32\release\product\OpenBoard.exe

$product = Resolve-Path build\win32\release\product
Copy-Item ..\OpenBoard-ThirdParty\poppler\bin\*.dll -Destination $product -Force
Copy-Item ..\OpenBoard-ThirdParty\openssl\openssl-3.0.15-win64\bin\*.dll -Destination $product -Force
Copy-Item ..\OpenBoard-ThirdParty\zlib\1.2.11\bin\*.dll -Destination $product -Force
```

## Test The New Changes

After launching OpenBoard:

- Confirm the toolbar shows the new `Minimize` button.
- Click `Minimize` and confirm OpenBoard goes to the Windows taskbar.
- Restore OpenBoard from the taskbar.
- Check the system tray icon menu for `Show OpenBoard`, `Minimize`, and `Exit`.
- Open `Preferences -> Display`.
- Enable `Run in windowed mode (shows minimize/maximize/close buttons)`.
- Restart OpenBoard and confirm the native Windows title bar appears.

Fullscreen mode remains the default when the windowed mode preference is disabled.

## Notes

- This checkout is nested one level below `D:\Ai\product\own\OpenBoard`.
- `OpenBoard-ThirdParty` must sit next to the `OpenBoard` source folder.
- If `rc.exe` is missing, install the Windows SDK.
- If `qmake` resolves to Anaconda Qt 5, use the full Qt 6 qmake path shown above.
