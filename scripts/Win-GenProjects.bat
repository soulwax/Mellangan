@echo off
pushd "%~dp0.."
if not exist vendor\scons\Scripts\scons.exe (
    echo SCons is missing, run scripts\Setup.bat first.
    popd
    if /i not "%~1"=="nopause" pause
    exit /b 1
)
vendor\scons\Scripts\scons.exe -Q msvs
set RESULT=%ERRORLEVEL%
popd
if /i not "%~1"=="nopause" pause
exit /b %RESULT%
