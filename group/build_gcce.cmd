@echo off
setlocal enabledelayedexpansion
title PvZ N95 - abld + GCCE (gcce urel)  [mirrors working Whisk3D toolchain]
cd /d "%~dp0"
set "ROOT=%~dp0.."
echo ===================================================
echo   PvZ N95  -  abld + GCCE (gcce urel) build
echo   (same compiler family as the working Whisk3D reference)
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
if not defined SDK ( echo [ERROR] S60 3rd FP1 SDK not found. Set EPOCROOT and retry. ^& pause ^& exit /b 1 )
echo [OK] SDK       : %SDK%

:: ============ 2. EPOCROOT must be drive-relative, trailing slash ============
set "SDKDRIVE=%SDK:~0,2%"
set "EPOCROOT=%SDK:~2%\"
%SDKDRIVE%
echo [OK] EPOCROOT  : %EPOCROOT%  (drive %SDKDRIVE%)
cd /d "%~dp0"

:: ============ 3. GCCE compiler must be on PATH ============
where arm-none-symbianelf-g++ >nul 2>nul
if errorlevel 1 (
  echo [X] GCCE compiler ^(arm-none-symbianelf-g++^) not found on PATH.
  echo     Install the CSL Arm Toolchain ^(GCCE^) and add its \bin to PATH.
  pause ^& exit /b 1
)
for /f "delims=" %%v in ('arm-none-symbianelf-g++ -dumpversion 2^>nul') do set GCCEVER=%%v
echo [OK] GCCE found ^(version %GCCEVER%^)

:: ============ 4. Build ============
:: Delete any STALE exe first so a failed link can't silently package an old binary.
set "EXE=%EPOCROOT%epoc32\release\gcce\urel\PvZ_N95.exe"
if exist "%EXE%" del /q "%EXE%"
echo [i] bldmake bldfiles ...
call bldmake bldfiles || (echo [X] bldmake failed ^& pause ^& exit /b 1)
echo [i] abld build gcce urel ...
call abld build gcce urel || (echo [X] abld build failed ^& pause ^& exit /b 1)

set "EXE=%EPOCROOT%epoc32\release\gcce\urel\PvZ_N95.exe"
if not exist "%EXE%" ( echo [X] Built exe not found: %EXE% ^& pause ^& exit /b 1 )
echo [OK] Built %EXE%

:: ============ 5. Package SIS ============
set "OUT=%~dp0..\sis"
cd /d "%OUT%"
echo [i] makesis PvZ_N95.pkg ...
call makesis PvZ_N95.pkg PvZ_N95.sis || (echo [X] makesis failed ^& pause ^& exit /b 1)
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
echo    C:\boot.log   ^(or  \boot.log^)
echo  With GCCE the EH self-test should now log:
echo    "EHTEST: SURVIVED, caught err=-1"   (proves Leave/TRAP works)
echo ===================================================
pause