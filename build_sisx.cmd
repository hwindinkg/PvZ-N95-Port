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
  "D:\Symbian\9.2\S60_3rd_FP1"
) do (
  if not defined SDK if exist "%%~R\Epoc32\release\armv5\LIB\euser.dso" set "SDK=%%~R"
)
if not defined SDK (
  echo Scanning drives for the SDK, please wait...
  for %%D in (C D E F G) do (
    if not defined SDK if exist "%%D:\Symbian" (
      for /f "delims=" %%P in ('dir /b /s "%%D:\Symbian\euser.dso" 2^>nul ^| findstr /i "armv5\\LIB\\euser.dso"') do (
        if not defined SDK (
          set "_p=%%P"
          set "_p=!_p:\Epoc32\release\armv5\LIB\euser.dso=!"
          set "SDK=!_p!"
        )
      )
    )
  )
)
if not defined SDK (
  echo [ERROR] S60 3rd Edition FP1 SDK not found.
  echo         Install it, or set EPOCROOT to its folder, then re-run.
  pause & exit /b 1
)
echo [OK] SDK          : %SDK%
set "EPOCROOT=%SDK%"
set "L=%SDK%\Epoc32\release\armv5\LIB"
set "EEXE=%SDK%\Epoc32\release\armv5\UREL\EEXE.LIB"
set "SDKTOOLS=%SDK%\Epoc32\tools"

:: ============ 2. Detect ARM (GCCE) toolchain ============
set "GPP=" & set "TCBIN="
where arm-none-symbianelf-g++.exe >nul 2>&1 && (
  for /f "delims=" %%G in ('where arm-none-symbianelf-g++.exe') do if not defined GPP set "GPP=%%G"
)
if not defined GPP for %%T in (
  "%SDK%\Epoc32\gcc\bin"
  "C:\Program Files (x86)\CSL Arm Toolchain\bin"
  "C:\Program Files\CSL Arm Toolchain\bin"
  "C:\CSL Arm Toolchain\bin"
  "%SDK%\Epoc32\tools"
) do (
  if not defined GPP if exist "%%~T\arm-none-symbianelf-g++.exe" ( set "TCBIN=%%~T" & set "GPP=%%~T\arm-none-symbianelf-g++.exe" )
)
if not defined GPP (
  echo [ERROR] arm-none-symbianelf-g++ not found.
  echo         Install the CSL ARM toolchain ^(GCCE^) and re-run.
  pause & exit /b 1
)
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

:: ============ 4. Detect signing keys ============
set "KCER=" & set "KKEY=" & set "KPASS="
for %%K in ("%ROOT%" "%ROOT%\keys" "%ROOT%\sis" "%ROOT%\..") do (
  if not defined KCER if exist "%%~K\key.cer" if exist "%%~K\key.key" ( set "KCER=%%~K\key.cer" & set "KKEY=%%~K\key.key" )
)
if not defined KCER (
  echo [..] key.cer/key.key not found - generating a self-signed cert...
  if not exist "%BUILD%" mkdir "%BUILD%" 2>nul
  "%SDKTOOLS%\makekeys.exe" -cert -password pvz -len 2048 -dname "CN=PvZ N95;OU=Port;OR=Community;CO=US;EM=dev@example.com" "%BUILD%\key.key" "%BUILD%\key.cer" >nul 2>&1
  if exist "%BUILD%\key.cer" ( set "KCER=%BUILD%\key.cer" & set "KKEY=%BUILD%\key.key" & set "KPASS=pvz" & echo [OK] self-signed cert created ) else ( echo [WARN] could not auto-generate cert; place key.cer/key.key next to this script )
)
if defined KCER echo [OK] Cert         : %KCER%
echo.

:: ============ 5. Clean / prepare ============
if exist "%BUILD%\obj" rd /s /q "%BUILD%\obj" 2>nul
if exist "%BUILD%\out" rd /s /q "%BUILD%\out" 2>nul
mkdir "%OBJ%" 2>nul
mkdir "%OUT%" 2>nul

:: ============ 6. Compile flags (from working build) ============
set "PRODINC=%SDK%/Epoc32/include/variant/Symbian_OS_v9.2.hrh"
set "D=-DNDEBUG -D_UNICODE -D__GCCE__ -D__SYMBIAN32__ -D__SERIES60_31__ -D__SERIES60_3X__ -D__EPOC32__ -D__MARM__ -D__EABI__ -D__MARM_ARMV5__ -D__EXE__ -D__PRODUCT_INCLUDE__=%PRODINC%"
set "F=-O2 -fno-unit-at-a-time -Wall -Wno-ctor-dtor-privacy -Wno-unknown-pragmas -fexceptions -march=armv5t -mapcs -pipe -nostdinc -c -msoft-float"
set "I=-I %SRC%/platform/symbian -I %SRC% -I %SRC%/engine -I %SRC%/Sexy.TodLib -I %SRC%/Lawn -I %SRC%/stl_stubs -I %SDK%/Epoc32/include -I %SDK%/Epoc32/include/variant -I %SDK%/Epoc32/include/stdapis"
set "G=%SDK%/Epoc32/include/GCCE/GCCE.h"

set "SD_SRC=%SRC%"
set "SD_ENG=%SRC%\engine"
set "SD_PLA=%SRC%\platform\symbian"
set "SD_TOD=%SRC%\Sexy.TodLib"
set "SD_LWN=%SRC%\Lawn"

set FILES=^
SD_PLA,main_symbian ^
SD_PLA,PvZApplication ^
SD_PLA,PvZDocument ^
SD_PLA,PvZAppUi ^
SD_PLA,PvZGameView ^
SD_ENG,SymbianFixes ^
SD_ENG,PvZVfs ^
SD_ENG,PakInterface ^
SD_ENG,GLInterface ^
SD_ENG,Image ^
SD_ENG,NativeDisplay ^
SD_ENG,MemoryImage ^
SD_ENG,GLImage ^
SD_ENG,Color ^
SD_ENG,SexyMatrix ^
SD_ENG,Buffer ^
SD_ENG,Graphics ^
SD_ENG,TodStringFile ^
SD_ENG,SexyAppBase ^
SD_ENG,Widget ^
SD_ENG,WidgetContainer ^
SD_ENG,WidgetManager ^
SD_ENG,Font ^
SD_ENG,Stubs ^
SD_TOD,TodCommon ^
SD_TOD,TodDebug ^
SD_SRC,Resources_stub ^
SD_LWN,GameObject ^
SD_LWN,Board ^
SD_LWN,Plant ^
SD_LWN,Zombie ^
SD_LWN,Projectile ^
SD_LWN,Coin ^
SD_LWN,LawnMower ^
SD_LWN,GridItem ^
SD_LWN,Challenge ^
SD_SRC,LawnApp ^
SD_ENG,ResourceManager ^
SD_ENG,stb_image

:: ============ 7. Compile ============
echo === Compiling source files ===
set "CC=arm-none-symbianelf-g++"
set "NFAIL=0"
for %%E in (%FILES%) do (
  for /f "tokens=1,2 delims=," %%a in ("%%E") do (
    set "DIR=!%%a!"
    if not exist "!DIR!\%%b.cpp" ( echo   [SKIP] %%b.cpp ^(missing^) ) else (
      echo   [CC] %%b.cpp
      "%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\%%b.o" "!DIR!\%%b.cpp"
      if ERRORLEVEL 1 ( echo   [ERROR] compile failed: %%b.cpp & set /a NFAIL+=1 )
    )
  )
)
if not "%NFAIL%"=="0" ( echo [ERROR] %NFAIL% file^(s^) failed to compile. & pause & exit /b 1 )
echo [OK] All sources compiled.
echo.

:: ============ 8. Link (ELF) ============
echo === Linking ELF executable ===
set "LD=arm-none-symbianelf-ld"
set "OBJS="
for %%f in ("%OBJ%\*.o") do set "OBJS=!OBJS! "%OBJ%\%%~nxf""
"%LD%" --target1-abs --no-undefined -nostdlib -shared -Ttext 0x8000 -Tdata 0x400000 --default-symver -soname "PvZ_N95{000a0000}[e1234567].exe" --entry _E32Startup -u _E32Startup "%EEXE%" -o "%OUT%\PvZ_N95_elf.exe" -Map "%OUT%\PvZ_N95.exe.map" %OBJS% -( "%L%\usrt2_2.lib" -) "%L%\euser.dso" "%L%\cone.dso" "%L%\eikcore.dso" "%L%\avkon.dso" "%L%\apparc.dso" "%L%\ws32.dso" "%L%\gdi.dso" "%L%\bitgdi.dso" "%L%\fbscli.dso" "%L%\efsrv.dso" "%L%\bafl.dso" "%L%\mediaclientaudio.dso" "%L%\libgles_cm.dso" "%L%\imageconversion.dso" "%L%\libc.dso" "%L%\libm.dso"
if ERRORLEVEL 1 ( echo [ERROR] link failed & pause & exit /b 1 )
echo [OK] Linked.
echo.

:: ============ 9. Convert to E32 image ============
echo === Converting to E32 image (elf2e32) ===
"%SDKTOOLS%\elf2e32.exe" --targettype=EXE --uid1=0x1000007a --uid2=0x100039ce --uid3=0xE1234567 --sid=0xE1234567 --capability=NONE --fpu=softvfp --elfinput="%OUT%\PvZ_N95_elf.exe" --output="%OUT%\PvZ_N95.exe" --linkas="PvZ_N95{000a0000}[e1234567].exe" --libpath="%L%" --dso "%L%\euser.dso" --dso "%L%\cone.dso" --dso "%L%\eikcore.dso" --dso "%L%\avkon.dso" --dso "%L%\apparc.dso" --dso "%L%\ws32.dso" --dso "%L%\gdi.dso" --dso "%L%\bitgdi.dso" --dso "%L%\fbscli.dso" --dso "%L%\efsrv.dso" --dso "%L%\bafl.dso" --dso "%L%\mediaclientaudio.dso" --dso "%L%\libgles_cm.dso" --dso "%L%\imageconversion.dso" --dso "%L%\libc.dso" --dso "%L%\libm.dso"
if ERRORLEVEL 1 ( echo [ERROR] elf2e32 failed & pause & exit /b 1 )
echo [OK] E32 image created.
echo.

:: ============ 10. Package resources ============
echo === Assembling package ===
set "PKGDIR=%OUT%\pkg"
if exist "%PKGDIR%" rd /s /q "%PKGDIR%" 2>nul
mkdir "%PKGDIR%" 2>nul
copy /Y "%OUT%\PvZ_N95.exe" "%PKGDIR%\PvZ_N95.exe" >nul
for %%R in (PvZ_N95.rsc PvZ_N95_loc.rsc PvZ_N95_reg.rsc) do (
  if exist "%ROOT%\data\%%R" ( copy /Y "%ROOT%\data\%%R" "%PKGDIR%\%%R" >nul ) else ( echo [WARN] resource missing: %%R )
)
copy /Y "%ROOT%\build\package.pkg" "%PKGDIR%\PvZ_N95.pkg" >nul

:: ============ 11. makesis + signsis ============
pushd "%PKGDIR%"
echo === Creating SIS ===
"%SDKTOOLS%\makesis.exe" PvZ_N95.pkg "%OUT%\PvZ_N95.sis"
if ERRORLEVEL 1 ( echo [ERROR] makesis failed & popd & pause & exit /b 1 )
echo === Signing SISX ===
if defined KCER (
  if defined KPASS (
    "%SDKTOOLS%\signsis.exe" "%OUT%\PvZ_N95.sis" "%OUT%\PvZ_N95.sisx" "%KCER%" "%KKEY%" %KPASS%
  ) else (
    "%SDKTOOLS%\signsis.exe" "%OUT%\PvZ_N95.sis" "%OUT%\PvZ_N95.sisx" "%KCER%" "%KKEY%"
  )
  if ERRORLEVEL 1 ( echo [ERROR] signsis failed & popd & pause & exit /b 1 )
) else (
  echo [WARN] No keys - leaving unsigned SIS only.
)
popd

echo.
echo ===================================================
echo   BUILD COMPLETE
if exist "%OUT%\PvZ_N95.sisx" ( echo   SISX : %OUT%\PvZ_N95.sisx ) else ( echo   SIS  : %OUT%\PvZ_N95.sis ^(unsigned^) )
echo   Copy it to your Nokia N95 and install.
echo ===================================================
pause
