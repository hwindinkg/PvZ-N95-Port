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
:: Prefer the GCCE toolchain's OWN runtime libs. Their __gnu_Unwind_Find_exidx is
:: Symbian-aware: it locates .ARM.exidx via the E32 image exception descriptor.
:: Generic/bundled libgcc uses ELF program headers (PT_ARM_EXIDX), which do NOT
:: survive elf2e32 -> User::Leave/TRAP crash (KERN-EXEC 3) on the first throw.
set "LGCC=" & set "LSUPC="
for /f "delims=" %%g in ('arm-none-symbianelf-g++ -print-libgcc-file-name 2^>nul') do set "LGCC=%%g"
for /f "delims=" %%g in ('arm-none-symbianelf-g++ -print-file-name=libsupc++.a 2^>nul') do set "LSUPC=%%g"
if not exist "%LGCC%"  set "LGCC=%ROOT%\build\lib\libgcc.a"
if not exist "%LSUPC%" set "LSUPC=%ROOT%\build\lib\libsupc++.a"
if not exist "%LSUPC%" ( echo [ERROR] no libsupc++.a found & pause & exit /b 1 )
if not exist "%LGCC%"  ( echo [ERROR] no libgcc.a found & pause & exit /b 1 )
echo [OK] libsupc++    : %LSUPC%
echo [OK] libgcc       : %LGCC%

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
set "CC=%GPP%"
for %%A in ("%GPP%") do set "ASBIN=%%~dpA"
set "AS=%ASBIN%arm-none-symbianelf-as.exe"
if not exist "%AS%" set "AS=arm-none-symbianelf-as"

set "SD_SRC=%SRC%"
set "SD_ENG=%SRC%\engine"
set "SD_PLA=%SRC%\platform\symbian"
set "SD_TOD=%SRC%\Sexy.TodLib"
set "SD_LWN=%SRC%\Lawn"

:: ============ 8. Compile all sources ============
echo === Compiling source files ===
echo   [CC] main_symbian.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\main_symbian.o" "%SD_SRC%\main_symbian.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: main_symbian.cpp & exit /b 1)
echo   [CC] PvZApplication.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\PvZApplication.o" "%SD_PLA%\PvZApplication.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: PvZApplication.cpp & exit /b 1)
echo   [CC] PvZDocument.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\PvZDocument.o" "%SD_PLA%\PvZDocument.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: PvZDocument.cpp & exit /b 1)
echo   [CC] PvZAppUi.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\PvZAppUi.o" "%SD_PLA%\PvZAppUi.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: PvZAppUi.cpp & exit /b 1)
echo   [CC] PvZGameView.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\PvZGameView.o" "%SD_PLA%\PvZGameView.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: PvZGameView.cpp & exit /b 1)
echo   [CC] SymbianFixes.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\SymbianFixes.o" "%SD_ENG%\SymbianFixes.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: SymbianFixes.cpp & exit /b 1)
echo   [CC] PvZVfs.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\PvZVfs.o" "%SD_ENG%\PvZVfs.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: PvZVfs.cpp & exit /b 1)
echo   [CC] PakInterface.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\PakInterface.o" "%SD_ENG%\PakInterface.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: PakInterface.cpp & exit /b 1)
echo   [CC] GLInterface.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\GLInterface.o" "%SD_ENG%\GLInterface.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: GLInterface.cpp & exit /b 1)
echo   [CC] Image.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Image.o" "%SD_ENG%\Image.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Image.cpp & exit /b 1)
echo   [CC] NativeDisplay.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\NativeDisplay.o" "%SD_ENG%\NativeDisplay.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: NativeDisplay.cpp & exit /b 1)
echo   [CC] MemoryImage.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\MemoryImage.o" "%SD_ENG%\MemoryImage.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: MemoryImage.cpp & exit /b 1)
echo   [CC] GLImage.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\GLImage.o" "%SD_ENG%\GLImage.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: GLImage.cpp & exit /b 1)
echo   [CC] Color.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Color.o" "%SD_ENG%\Color.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Color.cpp & exit /b 1)
echo   [CC] SexyMatrix.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\SexyMatrix.o" "%SD_ENG%\SexyMatrix.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: SexyMatrix.cpp & exit /b 1)
echo   [CC] Buffer.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Buffer.o" "%SD_ENG%\Buffer.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Buffer.cpp & exit /b 1)
echo   [CC] Graphics.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Graphics.o" "%SD_ENG%\Graphics.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Graphics.cpp & exit /b 1)
echo   [CC] TodStringFile.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\TodStringFile.o" "%SD_ENG%\TodStringFile.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: TodStringFile.cpp & exit /b 1)
echo   [CC] SexyAppBase.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\SexyAppBase.o" "%SD_ENG%\SexyAppBase.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: SexyAppBase.cpp & exit /b 1)
echo   [CC] Widget.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Widget.o" "%SD_ENG%\Widget.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Widget.cpp & exit /b 1)
echo   [CC] WidgetContainer.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\WidgetContainer.o" "%SD_ENG%\WidgetContainer.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: WidgetContainer.cpp & exit /b 1)
echo   [CC] WidgetManager.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\WidgetManager.o" "%SD_ENG%\WidgetManager.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: WidgetManager.cpp & exit /b 1)
echo   [CC] Font.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Font.o" "%SD_ENG%\Font.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Font.cpp & exit /b 1)
echo   [CC] Stubs.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Stubs.o" "%SD_ENG%\Stubs.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Stubs.cpp & exit /b 1)
echo   [CC] TodCommon.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\TodCommon.o" "%SD_TOD%\TodCommon.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: TodCommon.cpp & exit /b 1)
echo   [CC] TodDebug.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\TodDebug.o" "%SD_TOD%\TodDebug.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: TodDebug.cpp & exit /b 1)
echo   [CC] Resources.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Resources.o" "%SD_SRC%\Resources.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Resources.cpp & exit /b 1)
echo   [CC] GameObject.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\GameObject.o" "%SD_LWN%\GameObject.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: GameObject.cpp & exit /b 1)
echo   [CC] Board.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Board.o" "%SD_LWN%\Board.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Board.cpp & exit /b 1)
echo   [CC] Plant.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Plant.o" "%SD_LWN%\Plant.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Plant.cpp & exit /b 1)
echo   [CC] Zombie.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Zombie.o" "%SD_LWN%\Zombie.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Zombie.cpp & exit /b 1)
echo   [CC] Projectile.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Projectile.o" "%SD_LWN%\Projectile.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Projectile.cpp & exit /b 1)
echo   [CC] Coin.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Coin.o" "%SD_LWN%\Coin.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Coin.cpp & exit /b 1)
echo   [CC] LawnMower.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\LawnMower.o" "%SD_LWN%\LawnMower.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: LawnMower.cpp & exit /b 1)
echo   [CC] GridItem.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\GridItem.o" "%SD_LWN%\GridItem.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: GridItem.cpp & exit /b 1)
echo   [CC] Challenge.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\Challenge.o" "%SD_LWN%\Challenge.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: Challenge.cpp & exit /b 1)
echo   [CC] LawnApp.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\LawnApp.o" "%SD_SRC%\LawnApp.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: LawnApp.cpp & exit /b 1)
echo   [CC] ResourceManager.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\ResourceManager.o" "%SD_ENG%\ResourceManager.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: ResourceManager.cpp & exit /b 1)
echo   [CC] stb_image.cpp
"%CC%" %D% %F% -include "%G%" %I% -o "%OBJ%\stb_image.o" "%SD_ENG%\stb_image.cpp"
if errorlevel 1 (echo   [ERROR] compile FAILED: stb_image.cpp & exit /b 1)
echo   [AS] newop.s
"%AS%" -o "%OBJ%\newop.o" "%SRC%\engine\newop.s"
if errorlevel 1 (echo   [ERROR] compile FAILED: newop.cpp & exit /b 1)
echo [OK] All sources compiled.
echo.

echo === Linking ELF executable ===
set OBJS="%OBJ%\main_symbian.o" "%OBJ%\PvZApplication.o" "%OBJ%\PvZDocument.o" "%OBJ%\PvZAppUi.o" "%OBJ%\PvZGameView.o" "%OBJ%\SymbianFixes.o" "%OBJ%\PvZVfs.o" "%OBJ%\PakInterface.o" "%OBJ%\GLInterface.o" "%OBJ%\Image.o" "%OBJ%\NativeDisplay.o" "%OBJ%\MemoryImage.o" "%OBJ%\GLImage.o" "%OBJ%\Color.o" "%OBJ%\SexyMatrix.o" "%OBJ%\Buffer.o" "%OBJ%\Graphics.o" "%OBJ%\TodStringFile.o" "%OBJ%\SexyAppBase.o" "%OBJ%\Widget.o" "%OBJ%\WidgetContainer.o" "%OBJ%\WidgetManager.o" "%OBJ%\Font.o" "%OBJ%\Stubs.o" "%OBJ%\TodCommon.o" "%OBJ%\TodDebug.o" "%OBJ%\Resources.o" "%OBJ%\GameObject.o" "%OBJ%\Board.o" "%OBJ%\Plant.o" "%OBJ%\Zombie.o" "%OBJ%\Projectile.o" "%OBJ%\Coin.o" "%OBJ%\LawnMower.o" "%OBJ%\GridItem.o" "%OBJ%\Challenge.o" "%OBJ%\LawnApp.o" "%OBJ%\ResourceManager.o" "%OBJ%\stb_image.o" "%OBJ%\newop.o"
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