@echo off
setlocal
set "ROOT=%~dp0"

rem CMake: portable copy in tools\ if present, otherwise the one on the PATH.
set "CMAKE=cmake"
if exist "%ROOT%tools\cmake-3.30.5-windows-x86_64\bin\cmake.exe" set "CMAKE=%ROOT%tools\cmake-3.30.5-windows-x86_64\bin\cmake.exe"

rem MSVC: the most recent Visual Studio / Build Tools with the C++ workload.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -all -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH (
    echo Visual Studio or Build Tools with the C++ workload is required.
    exit /b 1
)
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1

"%CMAKE%" -S "%ROOT%." -B "%ROOT%build" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release || exit /b 1
"%CMAKE%" --build "%ROOT%build" || exit /b 1

echo.
echo Standalone: build\TechMachine_artefacts\Release\Standalone\Tech Machine.exe
echo VST3:       build\TechMachine_artefacts\Release\VST3\Tech Machine.vst3
