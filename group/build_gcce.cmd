@echo off
setlocal enabledelayedexpansion
title PvZ N95 - abld + GCCE (gcce urel)  [mirrors working Whisk3D toolchain]

:: ================================================================
::  Plants vs Zombies -> Nokia N95 (S60 3rd FP1)  GCCE build
::  Outputs (exe / sis / sisx) are placed in  <repo>\build\out\
:: ================================================================

cd /d "%~dp0"
set "GROUP=%~dp0"
if "%GROUP:~-1%"=="\" set "GROUP=%GROUP:~0,-1%"
pushd "%GROUP%\.." & set "ROOT=%CD%" & popd
set "BUILD=%ROOT%\build"
set "OUT=%BUILD%\out"
if not exist "%OUT%" mkdir "%OUT%"
echo ===================================================
echo   PvZ N95  -  abld + GCCE (gcce urel) build
echo   Repo root : %ROOT%
echo   Output dir: %OUT%
echo ===================================================

:: ============ 1. Detect S60 3rd FP1 SDK ============
set "SDK="
if defined EPOCROOT (
  set "_e=%EPOCROOT%"
  if "!_e:~-1!"=="\" set "_e=!_e:~0,-1!"
  if exist "!_e!\Epoc32\release\armv5\LIB\euser.dso" set "SDK=!_e!"
)
for %%R in (
  "C:\Symbian\9.2\S60_3rd_FP1_2"
  "C:\Symbian\9.2\S60_3rd_FP1"
  "C:\S60\9.2\S60_3rd_FP1_2"
  "D:\Symbian\9.2\S60_3rd_FP1_2"
) do (
  if not defined SDK if exist "%%~R\Epoc32\release\armv5\LIB\euser.dso" set "SDK=%%~R"
)
if not defined SDK (
  echo [X] S60 3rd FP1 SDK not found. Set EPOCROOT and retry.
  goto :fail
)
echo [OK] SDK       : %SDK%

:: ============ 2. EPOCROOT (drive-relative, trailing slash) ============
set "SDKDRIVE=%SDK:~0,2%"
set "EPOCROOT=%SDK:~2%\"
%SDKDRIVE%
echo [OK] EPOCROOT  : %EPOCROOT%  (drive %SDKDRIVE%)

:: ============ 3. GCCE compiler must be on PATH ============
where arm-none-symbianelf-g++ >nul 2>nul
if errorlevel 1 (
  echo [X] GCCE compiler ^(arm-none-symbianelf-g++^) not found on PATH.
  goto :fail
)
for /f "delims=" %%v in ('arm-none-symbianelf-g++ -dumpversion 2^>nul') do set "GCCEVER=%%v"
echo [OK] GCCE       : version %GCCEVER%

:: ============ 3b. Make libgcc.a visible to the Symbian linker ============
:: GCCE links NO default libs -- libgcc supplies __aeabi_* (int divide,
:: soft-float) AND the C++ exception unwinder. Ask the compiler where its
:: libgcc.a is, then copy it where 'STATICLIBRARY libgcc.lib' is searched.
set "LIBGCC="
for /f "delims=" %%G in ('arm-none-symbianelf-g++ -print-libgcc-file-name 2^>nul') do set "LIBGCC=%%G"
if defined LIBGCC set "LIBGCC=%LIBGCC:/=\%"
if not defined LIBGCC goto :nolibgcc
if not exist "%LIBGCC%" goto :nolibgcc
echo [OK] libgcc.a  : %LIBGCC%
if not exist "%EPOCROOT%epoc32\release\armv5\urel" mkdir "%EPOCROOT%epoc32\release\armv5\urel" 2>nul
if not exist "%EPOCROOT%epoc32\release\gcce\urel"  mkdir "%EPOCROOT%epoc32\release\gcce\urel" 2>nul
copy /y "%LIBGCC%" "%EPOCROOT%epoc32\release\armv5\urel\libgcc.lib" >nul
copy /y "%LIBGCC%" "%EPOCROOT%epoc32\release\gcce\urel\libgcc.lib"  >nul 2>nul
echo [OK] libgcc.lib staged into SDK release dirs
goto :have_libgcc
:nolibgcc
echo [X] Could not locate libgcc.a via -print-libgcc-file-name.
echo     Link would fail with undefined __aeabi_* . Aborting.
goto :fail
:have_libgcc

:: ============ 4. Build (delete stale exe first!) ============
set "EXE=%EPOCROOT%epoc32\release\gcce\urel\PvZ_N95.exe"
if exist "%EXE%" del /q "%EXE%"
cd /d "%GROUP%"
echo [i] bldmake bldfiles ...
call bldmake bldfiles
if errorlevel 1 ( echo [X] bldmake failed & goto :fail )
echo [i] abld build gcce urel ...
call abld build gcce urel
:: abld.pl can return 0 even when the underlying make fails, so the ONLY
:: reliable success check is: did the exe actually get produced?
if not exist "%EXE%" (
  echo.
  echo [X] LINK FAILED -- no exe produced:
  echo     %EXE%
  echo     ^(No stale binary was packaged. Scroll up for the real ld error.^)
  goto :fail
)
echo [OK] Built %EXE%

:: ============ 5. Package into build\out ============
copy /y "%EXE%" "%OUT%\PvZ_N95.exe" >nul
cd /d "%ROOT%\sis"
echo [i] makesis -> %OUT%\PvZ_N95.sis ...
call makesis PvZ_N95.pkg "%OUT%\PvZ_N95.sis"
if not exist "%OUT%\PvZ_N95.sis" ( echo [X] makesis failed & goto :fail )
echo [i] signsis ^(self-signed^) ...
if exist rd.cer if exist rd.key call signsis "%OUT%\PvZ_N95.sis" "%OUT%\PvZ_N95.sisx" rd.cer rd.key >nul 2>nul

echo ===================================================
echo  DONE. Artifacts in: %OUT%
if exist "%OUT%\PvZ_N95.sisx" (
  echo    SISX : %OUT%\PvZ_N95.sisx   ^(signed^)
) else (
  echo    SIS  : %OUT%\PvZ_N95.sis    ^(unsigned - install on a cert-patched phone^)
)
echo    EXE  : %OUT%\PvZ_N95.exe
echo.
echo  Install on the N95, run it, then send back  C:\boot.log
echo  Look for:  "EHTEST: SURVIVED, caught err=-1"   (proves Leave/TRAP works)
echo ===================================================
popd 2>nul
pause
exit /b 0

:fail
echo.
echo *** BUILD ABORTED -- see the error above. Nothing was packaged. ***
popd 2>nul
pause
exit /b 1