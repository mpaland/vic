@echo off
rem ---------------------------------------------------------------------------
rem Build and run the vic unit tests with MSVC (Ninja generator)
rem Usage: build.cmd [Debug|Release] [extra cmake args...]
rem ---------------------------------------------------------------------------
setlocal enabledelayedexpansion

rem remember the script folder before any shift (shift also moves %0)
set "ROOT=%~dp0"

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug
shift /1

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
  echo vswhere.exe not found, is Visual Studio installed?
  exit /b 1
)

set VSDIR=
for /f "usebackq delims=" %%i in (`call "!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if "!VSDIR!"=="" (
  echo No Visual Studio installation with C++ tools found
  exit /b 1
)

call "!VSDIR!\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1

set EXTRA=
:args
if "%~1"=="" goto run
set EXTRA=!EXTRA! %1
shift /1
goto args

:run
cmake -S "!ROOT!." -B "!ROOT!build" -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG% !EXTRA! || exit /b 1
cmake --build "!ROOT!build" || exit /b 1
ctest --test-dir "!ROOT!build" --output-on-failure
