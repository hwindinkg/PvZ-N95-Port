@echo off
setlocal enabledelayedexpansion
title PvZ N95 - Auto SISX Builder
cd /d "%~dp0"
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
set "SRC=%ROOT%\src"
set "BUILD=%ROOT%\build"
set "OBJ=%BUILD%\obj"
set "OUT=%BUILD%\out"

echo ===================================================
echo   PvZ N95  -  Self-contained SISX Builder
echo   Auto-detects: SDK, ARM toolchain, Perl, keys
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
if not defined SDK (
  echo Scanning drives for the SDK, please wait...
  for %%D in (C D E F G) do (
    if not defined SDK if exist "%%D:\Symbian" (
      for /f "delims=" %%P in ('dir /b /s "%%D:\Symbian\euser.dso" 2^>nul ^| findstr /i "armv5\\LIB\\euser.dso"') do (
        if not defined SDK ( set "_p=%%P" & set "_p=!_p:\Epoc32\release\armv5\LIB\euser.dso=!" & set "SDK=!_p!" )
      )
    )
  )
)
if not defined SDK ( echo [ERROR] S60 3rd Edition FP1 SDK not found. Set EPOCROOT and retry. & pause & exit /b 1 )
echo [OK] SDK          : %SDK%
set "EPOCROOT=%SDK%"
set "L=%SDK%\Epoc32\release\armv5\LIB"
set "SDKTOOLS=%SDK%\Epoc32\tools"

:: locate eexe.lib + usrt2_2.lib (urel first, then LIB)
set "EEXE=" & set "USRT="
for %%P in ("%SDK%\Epoc32\release\armv5\urel" "%SDK%\Epoc32\release\armv5\UREL" "%SDK%\Epoc32\release\armv5\LIB") do (
  if not defined EEXE if exist "%%~P\eexe.lib" set "EEXE=%%~P\eexe.lib"
  if not defined USRT if exist "%%~P\usrt2_2.lib" set "USRT=%%~P\usrt2_2.lib"
)
if not defined EEXE ( echo [ERROR] eexe.lib not found in SDK & pause & exit /b 1 )
if not defined USRT ( echo [ERROR] usrt2_2.lib not found in SDK & pause & exit /b 1 )
echo [OK] eexe.lib     : %EEXE%
echo [OK] usrt2_2.lib  : %USRT%

:: ============ 2. Detect ARM (GCCE) toolchain ============
set "GPP=" & set "TCBIN="
where arm-none-symbianelf-g++.exe >nul 2>&1 && ( for /f "delims=" %%G in ('where arm-none-symbianelf-g++.exe') do if not defined GPP set "GPP=%%G" )
if not defined GPP for %%T in (
  "%SDK%\Epoc32\gcc\bin"
  "C:\Program Files (x86)\CSL Arm Toolchain\bin"
  "C:\Program Files\CSL Arm Toolchain\bin"
  "C:\CSL Arm Toolchain\bin"
) do ( if not defined GPP if exist "%%~T\arm-none-symbianelf-g++.exe" ( set "TCBIN=%%~T" & set "GPP=%%~T\arm-none-symbianelf-g++.exe" ) )
if not defined GPP ( echo [ERROR] arm-none-symbianelf-g++ not found. Install CSL ARM toolchain. & pause & exit /b 1 )
if defined TCBIN set "PATH=%TCBIN%;%PATH%"
set "PATH=%SDKTOOLS%;%PATH%"
echo [OK] GCCE         : %GPP%

:: ============ 3. Detect Perl (optional) ============
set "PERLOK="
where perl.exe >nul 2>&1 && set "PERLOK=1"
if not defined PERLOK for %%T in ("C:\Perl\bin" "D:\Perl\bin" "C:\Perl64\bin" "C:\Strawberry\perl\bin") do (
  if not defined PERLOK if exist "%%~T\perl.exe" ( set "PATH=%%~T;%PATH%" & set "PERLOK=1" )
)
if defined PERLOK ( echo [OK] Perl         : found ) else ( echo [..] Perl         : not found ^(usually fine^) )

:: ============ 4. Runtime libs (libsupc++ / libgcc) ============
set "LSUPC=%ROOT%\build\lib\libsupc++.a"
set "LGCC=%ROOT%\build\lib\libgcc.a"
if not exist "%LSUPC%" ( echo [ERROR] build\lib\libsupc++.a missing & pause & exit /b 1 )
if not exist "%LGCC%"  ( echo [ERROR] build\lib\libgcc.a missing & pause & exit /b 1 )
echo [OK] runtime libs : build\lib\libsupc++.a, libgcc.a

:: ============ 5. Signing keys ============
set "KCER=" & set "KKEY=" & set "KPASS="
for %%K in ("%ROOT%" "%ROOT%\keys" "%ROOT%\sis" "%ROOT%\..") do (
  if not defined KCER if exist "%%~K\key.cer" if exist "%%~K\key.key" ( set "KCER=%%~K\key.cer" & set "KKEY=%%~K\key.key" )
)
if not defined KCER (
  echo [..] key.cer/key.key not found - generating self-signed cert...
  if not exist "%BUILD%" mkdir "%BUILD%" 2>nul
  "%SDKTOOLS%\makekeys.exe" -cert -password pvz -len 2048 -dname "CN=PvZ N95;OU=Port;OR=Community;CO=US;EM=dev@example.com" "%BUILD%\key.key" "%BUILD%\key.cer" >nul 2>&1
  if exist "%BUILD%\key.cer" ( set "KCER=%BUILD%\key.cer" & set "KKEY=%BUILD%\key.key" & set "KPASS=pvz" ) else ( echo [WARN] could not auto-generate cert )
)
if defined KCER echo [OK] Cert         : %KCER%
echo.

:: ============ 6. Clean ============
if exist "%OBJ%" rd /s /q "%OBJ%" 2>nul
if exist "%OUT%" rd /s /q "%OUT%" 2>nul
mkdir "%OBJ%" 2>nul
mkdir "%OUT%" 2>nul

:: ============ 7. Compile flags ============
set "PRODINC=%SDK%/Epoc32/include/variant/Symbian_OS_v9.2.hrh"
set "D=-DNDEBUG -D_UNICODE -D__GCCE__ -D__SYMBIAN32__ -D__SERIES60_31__ -D__SERIES60_3X__ -D__EPOC32__ -D__MARM__ -D__EABI__ -D__MARM_ARMV5__ -D__EXE__ -D__PRODUCT_INCLUDE__=%PRODINC%"
set "F=-O2 -fno-unit-at-a-time -Wall -Wno-ctor-dtor-privacy -Wno-unknown-pragmas -fexceptions -march=armv5t -mapcs -pipe -nostdinc -c -msoft-float"
set "I=-I %SRC%/platform/symbian -I %SRC% -I %SRC%/engine -I %SRC%/Sexy.TodLib -I %SRC%/Lawn -I %SRC%/stl_stubs -I %SDK%/Epoc32/include -I %SDK%/Epoc32/include/variant -I %SDK%/Epoc32/include/stdapis"
set "G=%SDK%/Epoc32/include/GCCE/GCCE.h"
set "CC=arm-none-symbianelf-g++"

set "SD_SRC=%SRC%"
set "SD_ENG=%SRC%\engine"
set "SD_PLA=%SRC%\platform\symbian"
set "SD_TOD=%SRC%\Sexy.TodLib"
set "SD_LWN=%SRC%\Lawn"

:: ============ 8. Compile all sources ============
echo === Compiling source files ===
set "NFAIL=0"
call :cc SD_PLA main_symbian
call :cc SD_PLA PvZApplication
call :cc SD_PLA PvZDocument
call :cc SD_PLA PvZAppUi
call :cc SD_PLA PvZGameView
call :cc SD_ENG SymbianFixes
call :cc SD_ENG PvZVfs
call :cc SD_ENG PakInterface
call :cc SD_ENG GLInterface
call :cc SD_ENG Image
call :cc SD_ENG NativeDisplay
call :cc SD_ENG MemoryImage
call :cc SD_ENG GLImage
call :cc SD_ENG Color
call :cc SD_ENG SexyMatrix
call :cc SD_ENG Buffer
call :cc SD_ENG Graphics
call :cc SD_ENG TodStringFile
call :cc SD_ENG SexyAppBase
call :cc SD_ENG Widget
call :cc SD_ENG WidgetContainer
call :cc SD_ENG WidgetManager
call :cc SD_ENG Font
call :cc SD_ENG Stubs
call :cc SD_TOD TodCommon
call :cc SD_TOD TodDebug
call :cc SD_SRC Resources_stub
call :cc SD_LWN GameObject
call :cc SD_LWN Board
call :cc SD_LWN Plant
call :cc SD_LWN Zombie
call :cc SD_LWN Projectile
call :cc SD_LWN Coin
call :cc SD_LWN LawnMower
call :cc SD_LWN GridItem
call :cc SD_LWN Challenge
call :cc SD_SRC LawnApp
call :cc SD_ENG ResourceManager
call :cc SD_ENG stb_image
if not "%NFAIL%"=="0" ( echo [ERROR] %NFAIL% file^(s^) failed to compile. & pause & exit /b 1 )

:: assemble newop.s (provides operator new/delete)
if exist "%SRC%\engine\newop.s" (
  echo   [AS] newop.s
  arm-none-symbianelf-as -o "%OBJ%\newop.o" "%SRC%\engine\newop.s"
  if ERRORLEVEL 1 ( echo [ERROR] assembler failed & pause & exit /b 1 )
)
echo [OK] All sources compiled.
echo.

:: ============ 9. Link ELF (g++ driver) ============
echo === Linking ELF executable ===
set "OBJS="
for %%f in ("%OBJ%\*.o") do set "OBJS=!OBJS! "%OBJ%\%%~nxf""
"%CC%" -nostdlib -shared -Wl,--target1-abs,--allow-multiple-definition,--soname,"PvZ_N95{000a0000}[e1234567].exe",--entry,_E32Startup,-u,_E32Startup %OBJS% "%EEXE%" "%USRT%" "%LSUPC%" "%LGCC%" "%L%\euser.dso" "%L%\cone.dso" "%L%\eikcore.dso" "%L%\avkon.dso" "%L%\apparc.dso" "%L%\ws32.dso" "%L%\gdi.dso" "%L%\bitgdi.dso" "%L%\fbscli.dso" "%L%\efsrv.dso" "%L%\bafl.dso" "%L%\mediaclientaudio.dso" "%L%\libgles_cm.dso" "%L%\imageconversion.dso" -o "%OUT%\PvZ_N95_elf.exe" -Wl,-Map,"%OUT%\PvZ_N95_elf.map"
if ERRORLEVEL 1 ( echo [ERROR] link failed & pause & exit /b 1 )
echo [OK] Linked.
echo.

:: ============ 10. Convert to E32 image ============
echo === Converting to E32 image (elf2e32) ===
"%SDKTOOLS%\elf2e32.exe" --targettype=EXE --uid1=0x1000007a --uid2=0x100039ce --uid3=0xE1234567 --sid=0xE1234567 --capability=NONE --fpu=softvfp --elfinput="%OUT%\PvZ_N95_elf.exe" --output="%OUT%\PvZ_N95.exe" --linkas="PvZ_N95{000a0000}[e1234567].exe" --libpath="%L%" --dso "%L%\euser.dso" --dso "%L%\cone.dso" --dso "%L%\eikcore.dso" --dso "%L%\avkon.dso" --dso "%L%\apparc.dso" --dso "%L%\ws32.dso" --dso "%L%\gdi.dso" --dso "%L%\bitgdi.dso" --dso "%L%\fbscli.dso" --dso "%L%\efsrv.dso" --dso "%L%\bafl.dso" --dso "%L%\mediaclientaudio.dso" --dso "%L%\libgles_cm.dso" --dso "%L%\imageconversion.dso"
if ERRORLEVEL 1 ( echo [ERROR] elf2e32 failed & pause & exit /b 1 )
echo [OK] E32 image created.
echo.

:: ============ 11. Package resources ============
echo === Assembling package ===
set "PKGDIR=%OUT%\pkg"
if exist "%PKGDIR%" rd /s /q "%PKGDIR%" 2>nul
mkdir "%PKGDIR%" 2>nul
copy /Y "%OUT%\PvZ_N95.exe" "%PKGDIR%\PvZ_N95.exe" >nul
for %%R in (PvZ_N95.rsc PvZ_N95_loc.rsc PvZ_N95_reg.rsc) do (
  if exist "%ROOT%\data\%%R" ( copy /Y "%ROOT%\data\%%R" "%PKGDIR%\%%R" >nul ) else ( echo [WARN] resource missing: %%R )
)
copy /Y "%ROOT%\build\package.pkg" "%PKGDIR%\PvZ_N95.pkg" >nul

:: ============ 12. makesis + signsis ============
pushd "%PKGDIR%"
echo === Creating SIS ===
"%SDKTOOLS%\makesis.exe" PvZ_N95.pkg "%OUT%\PvZ_N95.sis"
if ERRORLEVEL 1 ( echo [ERROR] makesis failed & popd & pause & exit /b 1 )
echo === Signing SISX ===
if defined KCER (
  if defined KPASS ( "%SDKTOOLS%\signsis.exe" "%OUT%\PvZ_N95.sis" "%OUT%\PvZ_N95.sisx" "%KCER%" "%KKEY%" %KPASS% ) else ( "%SDKTOOLS%\signsis.exe" "%OUT%\PvZ_N95.sis" "%OUT%\PvZ_N95.sisx" "%KCER%" "%KKEY%" )
  if ERRORLEVEL 1 ( echo [ERROR] signsis failed & popd & pause & exit /b 1 )
) else ( echo [WARN] No keys - leaving unsigned SIS only. )
popd

echo.
echo ===================================================
echo   BUILD COMPLETE
if exist "%OUT%\PvZ_N95.sisx" ( echo   SISX : %OUT%\PvZ_N95.sisx ) else ( echo   SIS  : %OUT%\PvZ_N95.sis ^(unsigned^) )
echo   Copy it to your Nokia N95 and install.
echo ===================================================
pause
goto :eof

:: ---------- compile one file: %1=dir-var %2=basename ----------
:cc
set "DIR=!%~1!"
if not exist "!DIR!\%~2.cpp" ( echo   [SKIP] %~2.cpp ^(missing^) & goto :eof )
echo   [CC] %~2.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\%~2.o" "!DIR!\%~2.cpp"
if ERRORLEVEL 1 ( echo   [ERROR] compile failed: %~2.cpp & set /a NFAIL+=1 )
goto :eof
