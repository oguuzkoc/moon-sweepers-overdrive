@echo off
rem Installs the mod into the Moon Sweepers folder.
rem Usage: install.bat "C:\Program Files (x86)\Steam\steamapps\common\Moon Sweepers"
rem Run it from the folder that contains version.dll (release zip) or after build.bat (build\version.dll).
setlocal
set "GAME=%~1"
if "%GAME%"=="" set "GAME=C:\Program Files (x86)\Steam\steamapps\common\Moon Sweepers"
if not exist "%GAME%\MoonSweepers.exe" (
    echo Oyun klasoru bulunamadi: "%GAME%"
    echo Kullanim: install.bat "oyun klasoru"
    exit /b 1
)
set "DLL=%~dp0version.dll"
if not exist "%DLL%" set "DLL=%~dp0build\version.dll"
if not exist "%DLL%" (
    echo version.dll bulunamadi. Once build.bat calistirin ya da release zip'ini kullanin.
    exit /b 1
)
if exist "%GAME%\version.dll" (
    fc /b "%GAME%\version.dll" "%DLL%" >nul && (echo Mod zaten kurulu. & exit /b 0)
    echo "%GAME%\version.dll" zaten var ^(baska bir mod yukleyici olabilir^). Once onu kaldirin.
    exit /b 1
)
rem The mod forwards the real version.dll exports to version_sys.dll: a copy of the system DLL.
copy /y "%SystemRoot%\System32\version.dll" "%GAME%\version_sys.dll" >nul || exit /b 1
copy /y "%DLL%" "%GAME%\version.dll" >nul || exit /b 1
echo Kuruldu. Oyunu baslatin; ayarlar ilk acilista "%GAME%\MoonSweepers_ConveyorMod.ini" olarak olusur.
