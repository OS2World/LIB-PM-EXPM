@echo off
rem compile.cmd -- Build ShadedButtons_1-15 with OpenWatcom (32-bit OS/2)
rem Run from the project root (where makefile.wat lives).
rem Requires OpenWatcom to be installed and WATCOM env var to be set.

if "%WATCOM%" == "" (
    echo ERROR: WATCOM environment variable is not set.
    echo        Source the OpenWatcom environment setup before running.
    goto :EOF
)

echo.
echo === ShadedButtons_1-15 -- OpenWatcom 32-bit OS/2 build ===
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
    echo   sample\Test1\Test1.exe  -- Shaded button demo (icon buttons)
    echo   sample\Test2\Test2.exe  -- Shaded button demo (resize)
    echo   sample\Test3\Test3.exe  -- Shaded button demo (C)
    echo.
    echo   Place dll\ShadeBtn.dll on the LIBPATH before running.
    echo.
    echo   NOTE: dll\ShadeBtn.dll and dll\ShBtn.dll are pre-built.
    echo   To rebuild the DLLs, see the blocked targets in makefile.wat.
)
