@echo off
rem compile.cmd -- Build LIB-PM-ComboBoxSupport with OpenWatcom
rem Run from the project root (where makefile.wat lives).
rem Requires OpenWatcom to be installed and WATCOM env var to be set.

if "%WATCOM%" == "" (
    echo ERROR: WATCOM environment variable is not set.
    echo        Source the OpenWatcom environment setup before running.
    goto :EOF
)

echo.
echo === LIB-PM-ComboBoxSupport -- OpenWatcom 32-bit OS/2 build ===
echo.

wmake -f makefile.wat clean
wmake -f makefile.wat %1 %2 %3 > compile.log 2>&1
set BLDRC=%ERRORLEVEL%

type compile.log

if %BLDRC% NEQ 0 (
    echo.
    echo *** Build FAILED  -- see compile.log ***
) else (
    echo.
    echo === Build complete ===
    echo   combosup.dll      -- PM combo box DLL
    echo   lib\combosup.lib  -- Import library
    echo   combosmp.exe      -- Sample PM application
)
