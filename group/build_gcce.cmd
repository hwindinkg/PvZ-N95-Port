@echo off
setlocal enabledelayedexpansion
title PvZ N95 - abld + GCCE (gcce urel)  [mirrors working Whisk3D toolchain]

:: ================================================================
::  Plants vs Zombies -> Nokia N95 (S60 3rd FP1)  GCCE build
::  GCCE is the SAME toolchain the working Whisk3D reference uses.
::  Its default link provides a WORKING C++ EH unwinder (libgcc/
::  libsupc++), so User::Leave / TRAP actually work -> no KERN-EXEC 3.
:: ================================================================

cd /d "%~dp0"

:: ---- 1. EPOCROOT ----
if not defined EPOCROOT set EPOCROOT=\
echo [i] EPOCROOT=%EPOCROOT%

:: ---- 2. GCCE compiler must be on PATH ----
where arm-none-symbianelf-g++ >nul 2>nul
if errorlevel 1 (
  echo [X] GCCE compiler ^(arm-none-symbianelf-g++^) not found on PATH.
  echo     Install the CSL Arm Toolchain ^(GCCE^) and add its \bin to PATH.
  echo     This is the SAME compiler the working Whisk3D build uses.
  pause & exit /b 1
)
for /f "delims=" %%v in ('arm-none-symbianelf-g++ -dumpversion 2^>nul') do set GCCEVER=%%v
echo [OK] GCCE found ^(version %GCCEVER%^)

:: ---- 3. Build ----
echo [i] bldmake bldfiles ...
call bldmake bldfiles || (echo [X] bldmake failed & pause & exit /b 1)
echo [i] abld build gcce urel ...
call abld build gcce urel || (echo [X] abld build failed & pause & exit /b 1)

set EXE=%EPOCROOT%epoc32\release\gcce\urel\PvZ_N95.exe
if not exist "%EXE%" ( echo [X] Built exe not found: %EXE% & pause & exit /b 1 )
echo [OK] Built %EXE%

:: ---- 4. Package SIS ----
set OUT=%~dp0..\sis
cd /d "%OUT%"
echo [i] makesis PvZ_N95.pkg ...
call makesis PvZ_N95.pkg PvZ_N95.sis || (echo [X] makesis failed & pause & exit /b 1)
echo [i] signsis ^(self-signed^) ...
call signsis PvZ_N95.sis PvZ_N95.sisx rd.cer rd.key 1>nul 2>nul
if exist "%OUT%\PvZ_N95.sisx" (
  echo   SISX : %OUT%\PvZ_N95.sisx  ^(signed^)
) else (
  echo   SIS  : %OUT%\PvZ_N95.sis   ^(unsigned - install on a cert-patched phone^)
)
echo ===================================================
echo  DONE. Copy the .sisx/.sis to the N95 and install.
echo  After it runs/crashes, pull the boot log from the phone:
echo    C:\boot.log   ^(or  \boot.log  depending on caps^)
echo  and paste it back. With GCCE the EH self-test should now log:
echo    "EHTEST: SURVIVED, caught err=-1"   (proves Leave/TRAP works)
echo ===================================================
pause