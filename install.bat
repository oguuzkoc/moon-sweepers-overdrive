@echo off
rem Installs the mod into the Moon Sweepers folder.
rem Usage: install.bat "C:\Program Files (x86)\Steam\steamapps\common\Moon Sweepers"
rem Run it from the folder that contains version.dll (release zip) or after build.bat (build\version.dll).
setlocal
set "GAME=%~1"
if "%GAME%"=="" set "GAME=C:\Program Files (x86)\Steam\steamapps\common\Moon Sweepers"
if not exist "%GAME%\MoonSweepers.exe" (
    echo Game folder not found: "%GAME%"
    echo Usage: install.bat "path\to\Moon Sweepers"
    exit /b 1
)
set "DLL=%~dp0version.dll"
if not exist "%DLL%" set "DLL=%~dp0build\version.dll"
if not exist "%DLL%" (
    echo version.dll not found. Run build.bat first or use the release zip.
    exit /b 1
)
if exist "%GAME%\version.dll" (
    fc /b "%GAME%\version.dll" "%DLL%" >nul && (echo The mod is already installed. & exit /b 0)
    if not exist "%GAME%\version_sys.dll" (
        echo "%GAME%\version.dll" already exists ^(probably another mod loader^). Remove it first.
        exit /b 1
    )
    echo Updating the installed mod...
)
rem The mod forwards the real version.dll exports to version_sys.dll: a copy of the system DLL.
copy /y "%SystemRoot%\System32\version.dll" "%GAME%\version_sys.dll" >nul || exit /b 1
copy /y "%DLL%" "%GAME%\version.dll" >nul || (echo Could not copy version.dll. Close the game and try again. & exit /b 1)
echo Installed. Start the game; the settings file is created on first launch: "%GAME%\MoonSweepers_ConveyorMod.ini"
