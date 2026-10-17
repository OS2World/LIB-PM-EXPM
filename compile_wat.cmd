@echo off
rem compile_wat.cmd -- Build EXPM with OpenWatcom.
rem Run from the project root (where Makefile.wat lives).
rem Generates compile.log in the project root.

rem Configure the OpenWatcom environment if not already set up.
if "%WATCOM%" == "" set WATCOM=C:\WATCOM
set PATH=%WATCOM%\binp;%WATCOM%\bin;%WATCOM%\bat;%PATH%
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os2;%INCLUDE%
set LIB=%WATCOM%\lib386;%WATCOM%\lib386\os2;%LIB%

echo.
echo === EXPM -- OpenWatcom 32-bit OS/2 build ===
echo.

wmake -f Makefile.wat clean < NUL
wmake -f Makefile.wat %1 %2 %3 < NUL > compile.log 2>&1

type compile.log

if errorlevel 1 goto buildfail

echo.
echo === Build complete ===
echo   dll\expm.dll                 -- EXPM 32-bit PM library DLL
echo   lib\expm.lib                 -- EXPM import library
echo   sample\combo\combosmp.exe    -- combo box sample application
echo   sample\cell\celmp.exe        -- split views + toolbar sample
echo   sample\ctlplus\ctlpsmp.exe   -- Control Plus controls sample
echo   sample\colorwheel\cwsmp.exe  -- ColorWheel dialog sample
echo   sample\anibtn\anismp.exe     -- AniButtn animated buttons sample
echo   sample\shadebtn\shbsmp.exe   -- Shaded Buttons sample
echo   sample\arrowhead\arsmp.exe  -- ArrowHead drawing sample
echo   sample\owndraw\ownsmpl.exe  -- Owner-Draw Listbox sample
goto buildend

:buildfail
echo.
echo *** Build FAILED -- see compile.log ***

:buildend
