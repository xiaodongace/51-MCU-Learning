@echo off
setlocal

rem Windows/Parallels guest build entry point.
rem The project root is calculated from this script location, so the project
rem folder can be copied and renamed without changing this file.

set "PROJECT_ROOT=%~dp0.."
set "KEIL_ROOT=C:\keil"
set "UV4=%KEIL_ROOT%\UV4\UV4.exe"
set "LOG=%PROJECT_ROOT%\build\keil-build.log"

if not exist "%PROJECT_ROOT%\build\output" mkdir "%PROJECT_ROOT%\build\output"

echo ==== 51_Template Keil build probe ==== > "%LOG%"
echo Project root: %PROJECT_ROOT% >> "%LOG%"
echo Keil executable: %UV4% >> "%LOG%"

if not exist "%UV4%" (
    echo ERROR: Keil UV4.exe was not found under %KEIL_ROOT%.
    echo ERROR: Keil UV4.exe was not found under %KEIL_ROOT%. >> "%LOG%"
    pause
    exit /b 2
)

rem Find the Keil project automatically. This allows a copied project to keep
rem its original .uvproj filename or use a renamed one.
set "PROJECT_FILE="
for %%F in ("%PROJECT_ROOT%\keil\*.uvproj") do (
    if not defined PROJECT_FILE set "PROJECT_FILE=%%~fF"
)

echo Keil executable: %UV4%
echo Project root: %PROJECT_ROOT%
echo Project file: %PROJECT_FILE%
echo Keil executable found. >> "%LOG%"

if not exist "%PROJECT_FILE%" (
    echo ERROR: Keil project file was not found.
    echo ERROR: Keil project file was not found. >> "%LOG%"
    echo Log written to: %LOG%
    pause
    exit /b 3
)

echo Building Keil project...
"%UV4%" -b "%PROJECT_FILE%" -j0 -z -o "%LOG%"
set "BUILD_RESULT=%ERRORLEVEL%"
echo UV4 exit code: %BUILD_RESULT% >> "%LOG%"

rem UV4 return codes: 0 = no errors, 1 = warnings only, 2+ = build error.
if %BUILD_RESULT% GEQ 2 (
    echo ERROR: Keil build failed. See %LOG%
    echo.
    type "%LOG%"
    pause
    exit /b %BUILD_RESULT%
)

if "%BUILD_RESULT%"=="1" echo Keil build completed with warnings.
if "%BUILD_RESULT%"=="0" echo Keil build completed without warnings.
echo HEX should be under: %PROJECT_ROOT%\build\output\
echo.
echo Log written to: %LOG%
pause
exit /b 0
