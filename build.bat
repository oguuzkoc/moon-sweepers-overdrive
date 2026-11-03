@echo off
rem Builds version.dll from src\moonsweepers_mod.cpp with the MSVC x64 toolchain.
rem Requires Visual Studio 2022 (or Build Tools) with the "Desktop development with C++" workload.
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo vswhere.exe bulunamadi. Visual Studio 2022 / Build Tools kurulu olmali.
    exit /b 1
)
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS=%%i"
if not defined VS (
    echo C++ derleyicisi bulunamadi. "Desktop development with C++" bilesenini kurun.
    exit /b 1
)
call "%VS%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
if not exist build mkdir build
cl /nologo /O2 /EHsc /MT /std:c++17 /utf-8 /LD src\moonsweepers_mod.cpp user32.lib /Fobuild\ /Fdbuild\ /link /OUT:build\version.dll /IMPLIB:build\version.lib || exit /b 1
echo.
echo Derlendi: build\version.dll
