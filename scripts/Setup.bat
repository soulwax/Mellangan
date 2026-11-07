@echo off
setlocal
pushd "%~dp0.."

set SCONS_VERSION=4.11.1
set VENV=vendor\scons
set SDL_SRC=extern\Vulkan2D\extern\SDL

if not defined VULKAN_SDK (
    for /d %%d in ("C:\VulkanSDK\*") do set "VULKAN_SDK=%%d"
)
if not defined VULKAN_SDK (
    echo Vulkan SDK not found. Install it from https://vulkan.lunarg.com and run this again.
    goto :fail
)
echo Using Vulkan SDK %VULKAN_SDK%

where python >nul 2>nul || (echo Python 3 is needed for SCons. & goto :fail)
where cmake >nul 2>nul || (echo CMake is needed to build SDL3. & goto :fail)

if not exist "%SDL_SRC%\CMakeLists.txt" (
    echo Fetching submodules...
    git submodule update --init --recursive || goto :fail
)

if not exist "%VENV%\Scripts\scons.exe" (
    echo Installing SCons %SCONS_VERSION%...
    python -m venv "%VENV%" || goto :fail
    "%VENV%\Scripts\python.exe" -m pip install --quiet --disable-pip-version-check scons==%SCONS_VERSION% || goto :fail
)

call scripts\Win-GenProjects.bat nopause || goto :fail

echo.
echo Done. Open Mellangan.sln in Visual Studio 2026.
popd
if /i not "%~1"=="nopause" pause
exit /b 0

:fail
echo.
echo Setup failed.
popd
if /i not "%~1"=="nopause" pause
exit /b 1
