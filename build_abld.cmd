@echo off
setlocal enabledelayedexpansion
title PvZ N95 - abld (standard SDK) Builder
cd /d "%~dp0"
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
set "GROUP=%ROOT%\group"
set "BUILD=%ROOT%\build"
set "OUT=%BUILD%\out"
if not exist "%OUT%" mkdir "%OUT%"

echo ===================================================
echo   PvZ N95  -  Standard abld + GCCE build
echo   (matches the working Whisk3D reference toolchain)
echo ===================================================
echo Repo root: %ROOT%
echo.

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
if not defined SDK ( echo [ERROR] S60 3rd FP1 SDK not found. Set EPOCROOT and retry. & pause & exit /b 1 )
echo [OK] SDK       : %SDK%
set "SDKTOOLS=%SDK%\Epoc32\tools"

:: ============ 2. EPOCROOT must be drive-relative, trailing slash ============
:: strip drive letter from SDK -> EPOCROOT, switch to that drive
set "SDKDRIVE=%SDK:~0,2%"
set "EPOCROOT=%SDK:~2%\"
%SDKDRIVE%
echo [OK] EPOCROOT  : %EPOCROOT%  (drive %SDKDRIVE%)

:: ============ 3. Put SDK + GCCE + Perl on PATH ============
set "PATH=%SDKTOOLS%;%SDK%\Epoc32\gcc\bin;%PATH%"

:: GCCE (CSL arm-none-symbianelf) toolchain
where arm-none-symbianelf-g++ >nul 2>&1
if ERRORLEVEL 1 (
  for %%G in (
    "C:\Program Files (x86)\CSL Arm Toolchain\bin"
    "C:\Program Files\CSL Arm Toolchain\bin"
    "C:\CSL Arm Toolchain\bin"
    "C:\Program Files (x86)\GNU Tools ARM\bin"
  ) do ( if exist "%%~G\arm-none-symbianelf-g++.exe" set "PATH=%%~G;!PATH!" )
)
where arm-none-symbianelf-g++ >nul 2>&1
if ERRORLEVEL 1 ( echo [ERROR] GCCE toolchain ^(arm-none-symbianelf-g++^) not on PATH. & pause & exit /b 1 )
for /f "delims=" %%g in ('where arm-none-symbianelf-g++ 2^>nul') do ( echo [OK] GCCE      : %%g & goto :gcceok )
:gcceok

:: Perl (abld/bldmake are Perl scripts)
where perl >nul 2>&1
if ERRORLEVEL 1 (
  for %%P in (
    "%SDK%\Epoc32\tools\perl\bin"
    "C:\Perl\bin"
    "C:\Perl64\bin"
    "C:\Strawberry\perl\bin"
  ) do ( if exist "%%~P\perl.exe" set "PATH=%%~P;!PATH!" )
)
where perl >nul 2>&1
if ERRORLEVEL 1 ( echo [ERROR] Perl not found - required by abld/bldmake. & pause & exit /b 1 )

:: ============ 4. bldmake + abld build gcce urel ============
pushd "%GROUP%"
echo.
echo === bldmake bldfiles ===
call bldmake bldfiles
if ERRORLEVEL 1 ( echo [ERROR] bldmake failed & popd & pause & exit /b 1 )

echo.
echo === abld build gcce urel ===
call abld build gcce urel
if ERRORLEVEL 1 ( echo [ERROR] abld build failed - see messages above & popd & pause & exit /b 1 )
popd

:: ============ 5. Verify the EXE was produced ============
set "EXE=%SDK%\Epoc32\release\gcce\urel\PvZ_N95.exe"
if not exist "%EXE%" ( echo [ERROR] abld did not produce %EXE% & pause & exit /b 1 )
echo [OK] Built EXE : %EXE%

:: ============ 6. Signing cert (reuse or self-generate) ============
set "KCER=" & set "KKEY=" & set "KPASS="
for %%K in ("%ROOT%" "%BUILD%" "%ROOT%\sis") do (
  if not defined KCER if exist "%%~K\key.cer" if exist "%%~K\key.key" ( set "KCER=%%~K\key.cer" & set "KKEY=%%~K\key.key" )
)
if not defined KCER (
  echo [..] key.cer/key.key not found - generating self-signed cert...
  "%SDKTOOLS%\makekeys.exe" -cert -password pvz -len 2048 -dname "CN=PvZ N95;OU=Port;OR=Community;CO=US;EM=dev@example.com" "%BUILD%\key.key" "%BUILD%\key.cer" >nul 2>&1
  if exist "%BUILD%\key.cer" ( set "KCER=%BUILD%\key.cer" & set "KKEY=%BUILD%\key.key" & set "KPASS=pvz" ) else ( echo [WARN] could not auto-generate cert )
)

:: ============ 7. makesis + signsis ============
set "PKG=%ROOT%\sis\PvZ_N95.pkg"
if not exist "%PKG%" set "PKG=%BUILD%\package.pkg"
echo.
echo === makesis %PKG% ===
"%SDKTOOLS%\makesis.exe" "%PKG%" "%OUT%\PvZ_N95.sis"
if ERRORLEVEL 1 ( echo [ERROR] makesis failed & pause & exit /b 1 )
echo === signsis ===
if defined KCER (
  if defined KPASS ( "%SDKTOOLS%\signsis.exe" "%OUT%\PvZ_N95.sis" "%OUT%\PvZ_N95.sisx" "%KCER%" "%KKEY%" %KPASS% ) else ( "%SDKTOOLS%\signsis.exe" "%OUT%\PvZ_N95.sis" "%OUT%\PvZ_N95.sisx" "%KCER%" "%KKEY%" )
  if ERRORLEVEL 1 ( echo [ERROR] signsis failed & pause & exit /b 1 )
)

echo.
echo ===================================================
echo   DONE.
if exist "%OUT%\PvZ_N95.sisx" ( echo   SISX : %OUT%\PvZ_N95.sisx ) else ( echo   SIS  : %OUT%\PvZ_N95.sis  unsigned )
echo   Copy it to your Nokia N95 and install.
echo ===================================================
pause
