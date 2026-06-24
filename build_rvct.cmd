@echo off
setlocal enabledelayedexpansion
title PvZ N95 - abld + RVCT 2.2 (ARMV5) Builder
cd /d "%~dp0"
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
set "GROUP=%ROOT%\group"
set "BUILD=%ROOT%\build"
set "OUT=%BUILD%\out"
if not exist "%OUT%" mkdir "%OUT%"

echo ===================================================
echo   PvZ N95  -  abld + RVCT 2.2 (ARMV5) build
echo   (same RVCT compiler family as the Whisk3D reference)
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
set "SDKDRIVE=%SDK:~0,2%"
set "EPOCROOT=%SDK:~2%\"
%SDKDRIVE%
echo [OK] EPOCROOT  : %EPOCROOT%  (drive %SDKDRIVE%)

:: ============ 3. Detect RVCT 2.2 (ARM RealView) toolchain ============
:: Flat detection via subroutines (nested for/if blocks break cmd.exe parsing).
set "RVCT22BIN="
set "RVROOT="
set "RVBUILD="
:: Fast path: the known install location.
if exist "C:\Symbian\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe" (
  set "RVCT22BIN=C:\Symbian\ARM\RVCT\Programs\2.2\349\win_32-pentium"
  set "RVROOT=C:\Symbian\ARM\RVCT"
  set "RVBUILD=349"
)
if not defined RVCT22BIN call :findrvct "C:\Symbian\ARM\RVCT"
if not defined RVCT22BIN call :findrvct "C:\Program Files (x86)\ARM\RVCT"
if not defined RVCT22BIN call :findrvct "C:\Program Files\ARM\RVCT"
if not defined RVCT22BIN call :findrvct "C:\apps\ARM\RVCT"
if not defined RVCT22BIN call :findrvct "C:\ARM\RVCT"
if not defined RVCT22BIN goto :norvct

set "RVCT22INC="
set "RVCT22LIB="
if exist "%RVROOT%\Data\2.2\%RVBUILD%\include" set "RVCT22INC=%RVROOT%\Data\2.2\%RVBUILD%\include"
if exist "%RVROOT%\Data\2.2\%RVBUILD%\lib"     set "RVCT22LIB=%RVROOT%\Data\2.2\%RVBUILD%\lib"
echo [OK] RVCT22BIN : %RVCT22BIN%  (build %RVBUILD%)
if defined RVCT22INC ( echo [OK] RVCT22INC : %RVCT22INC% ) else ( echo [warn] RVCT22INC not found - armcc will use its built-in include path )
if defined RVCT22LIB ( echo [OK] RVCT22LIB : %RVCT22LIB% ) else ( echo [warn] RVCT22LIB not found - armcc will use its built-in lib path )
goto :rvdone

:findrvct
if not exist "%~1\Programs\2.2" exit /b
for /d %%B in ("%~1\Programs\2.2\*") do call :tryrvct "%~1" "%%~B"
exit /b

:tryrvct
if defined RVCT22BIN exit /b
if not exist "%~2\win_32-pentium\armcc.exe" exit /b
set "RVCT22BIN=%~2\win_32-pentium"
set "RVROOT=%~1"
for %%N in ("%~2") do set "RVBUILD=%%~nxN"
exit /b

:norvct
echo [ERROR] RVCT 2.2 (armcc.exe) not found.
echo         Expected: C:\Symbian\ARM\RVCT\Programs\2.2\^<build^>\win_32-pentium\armcc.exe
echo         Or set RVCT22BIN/RVCT22INC/RVCT22LIB manually and retry.
pause
exit /b 1

:rvdone

:: ============ 4. PATH: SDK tools + RVCT + Perl ============
set "PATH=%SDKTOOLS%;%RVCT22BIN%;%PATH%"
where armcc >nul 2>&1
if ERRORLEVEL 1 ( echo [ERROR] armcc not on PATH after setup. & pause & exit /b 1 )
echo --- armcc version ---
armcc --version_number
armcc 2>nul | findstr /I "RVCT ARM" 
echo ---------------------

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

:: ============ 5. bldmake + abld build armv5 urel ============
pushd "%GROUP%"
echo.
echo === bldmake bldfiles ===
call bldmake bldfiles
if ERRORLEVEL 1 ( echo [ERROR] bldmake failed & popd & pause & exit /b 1 )

echo.
set "EXE=%SDK%\Epoc32\release\armv5\urel\PvZ_N95.exe"
if exist "%EXE%" del /q "%EXE%"

echo === abld build armv5 urel ===
call abld build armv5 urel
if ERRORLEVEL 1 ( echo [ERROR] abld build failed - see messages above & popd & pause & exit /b 1 )
popd

:: ============ 5a. Verify the EXE was produced (fresh) ============
if not exist "%EXE%" ( echo [ERROR] abld build FAILED - PvZ_N95.exe was not produced. Scroll up for the compiler error. & pause & exit /b 1 )
echo [OK] Built EXE : %EXE%
echo.

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