@echo off

if defined InstallDir goto :jump
echo.
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (set InstallDir=%%i)

if exist "%InstallDir%\Common7\Tools\VsDevCmd.bat" (call "%InstallDir%\Common7\Tools\VsDevCmd.bat" -arch=x86)
echo.
:jump
