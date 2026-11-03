@echo off
rem Removes the mod from the Moon Sweepers folder (saves are not touched).
setlocal
set "GAME=%~1"
if "%GAME%"=="" set "GAME=C:\Program Files (x86)\Steam\steamapps\common\Moon Sweepers"
if not exist "%GAME%\MoonSweepers.exe" (
    echo Oyun klasoru bulunamadi: "%GAME%"
    exit /b 1
)
del /q "%GAME%\version.dll" "%GAME%\version_sys.dll" "%GAME%\ConveyorMod.log" 2>nul
echo Mod kaldirildi. Ayar dosyasi MoonSweepers_ConveyorMod.ini istenirse elle silinebilir.
