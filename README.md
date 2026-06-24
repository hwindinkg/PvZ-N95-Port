# Plants vs. Zombies — Nokia N95 Port (Symbian S60 3rd Edition FP1)

Port of the SexyAppFramework-based PvZ to the Nokia N95 (Symbian OS 9.2, S60 3rd FP1),
OpenGL ES 1.1 renderer.

> **READ THIS FIRST if you are an AI/dev continuing the work.** This file is the
> single source of truth for the build journey, dead-ends already ruled out, and the
> current plan. Do **not** repeat the mistakes listed under "Dead ends".

---

## Current state (2026-06-24)

- App **builds, signs, installs** and **launches** on the device.
- It reaches `AppUi::ConstructL` and then **crashes (KERN-EXEC 3) on the very first raw
  C++ `throw/catch`** (the `EH T1` self-test in boot.log).
- **Root cause = broken C++ exception handling under GCCE 3.4.3** on S60 3rd FP1.
  GCCE's stack unwinder cannot find `.ARM.exidx` in the E32 image, so the first
  `throw` (and therefore the first `User::Leave` on EKA2, and Avkon's
  `BaseConstructL`) dies.

## THE PLAN: switch toolchain to RVCT (like the reference project)

The working reference — **Whisk3D** (OpenGL ES app for N95):
https://github.com/Dante-Leoncini/Whisk3D/tree/symbian — does **not** use GCCE.
It builds with **ARM RealView (RVCT/ARMCC)** via standard `abld`. RVCT generates
correct exception/unwind data and matches the RVCT-built Symbian runtime, so
`throw`/`Leave`/`TRAP` all work. **We are migrating to RVCT.**

RVCT 2.2 (build 349) is installed at:
`C:\Symbian\ARM\RVCT\Programs\2.2\349\win_32-pentium` (armcc.exe etc.)

> Symbian 9.2 officially expects RVCT 2.2 **build 435+**. Build **349** is older and
> may be rejected by the SDK's version gate (`\epoc32\tools\compilation_config\`).
> If `abld build armv5` complains about the RVCT build number, that gate must be
> relaxed/patched (or install build 435). This is the FIRST thing to check if the
> RVCT build refuses to start.

### How to build (RVCT path — preferred)
```
git pull
build_rvct.cmd        :: auto-detects SDK + RVCT, runs: abld build armv5 urel -> makesis -> signsis
```
Output: `build\out\PvZ_N95.sisx`. Install, run, send **boot.log** (C:\Data\PvZ\boot.log
or wherever MkDirAll points). Success marker now: `EH T1 caught=123` then
`EH T2 caught err=-42` -> exceptions + Leave both work -> proceed to game init.

### How to build (GCCE path — legacy/diagnostic only, EH is BROKEN)
```
build_abld.cmd        :: abld build gcce urel; only useful for the .ARM.exidx diagnostic
```

---

## What changed for the RVCT migration (commit on 2026-06-24)

| File | Change |
|---|---|
| `build_rvct.cmd` | **NEW.** Detects SDK + RVCT 2.2 (any build under `...\Programs\2.2\*`), sets `RVCT22BIN/INC/LIB` + PATH, runs `bldmake` + `abld build armv5 urel`, then makesis/signsis. EXE path = `epoc32\release\armv5\urel\PvZ_N95.exe`. |
| `group\bld.inf` | `PRJ_PLATFORMS` now `ARMV5 GCCE WINSCW` (added ARMV5 for RVCT). |
| `group\PvZ_N95.mmp` | Added `OPTION ARMCC --exceptions --rtti` and `ALWAYS_BUILD_AS_ARM` (kept GCCE option for fallback). |
| `src\engine\Stubs.cpp` | **Guarded** the hand-rolled soft-float (`__aeabi_*`), `abort`, and broken C-lib stubs (`fmod`->0, `rand`->0, `abs`, `strncmp`) under `#if defined(__GCCE__)`. RVCT links the **real** runtime for all of these — the GCCE shims must NOT compile under RVCT (they would conflict with / wrongly override the correct routines). Game stubs (`gSexyAppBase`, `DrawSeedPacket`, `__cxa_pure_virtual`, `TCppRTExceptionsGlobals`) remain unconditional. |
| `src\engine\SymbianFixes.cpp` | **Entire body guarded** under `#if defined(__GCCE__)`. It only contains GCCE link-gap shims (weak `__attribute__`, GCCE-mangled euser symbols like `_ZN5TTrap4TrapERi`, `__cxa_call_terminate`). RVCT provides the real runtime; the GCCE-only syntax wouldn't even compile under ARMCC. |

### IMPORTANT: GCCE vs RVCT macro
- GCCE defines `__GCCE__`. RVCT/ARMCC defines `__ARMCC_VERSION`.
- All GCCE-specific runtime shims are now behind `#if defined(__GCCE__)`.
- **When the RVCT build throws new errors, the fix pattern is usually:** another
  GCCE-only hack needs a `#if defined(__GCCE__)` guard, OR a missing library/symbol
  that RVCT expects. Check the build log and guard/add accordingly — do NOT re-add
  broken hand-rolled runtime.

---

## Expected first-RVCT-build issues (and how to react)

1. **RVCT build-number rejected by abld** -> relax the SDK version gate (see note above).
2. **Undefined refs to C functions** (`fmod`, `rand`, `strncmp`, `memcpy`...) -> RVCT may
   need a C runtime lib. Try adding `LIBRARY euser.lib` already present; if stdio/string
   are missing, the SDK's `estlib.lib` may help. **Do NOT** re-enable `libc.lib`/Open C —
   `libc.dso` is NOT in this SDK (confirmed missing).
3. **Duplicate symbol** for any `__aeabi_*`/`abort` -> means a GCCE shim leaked; ensure it's
   under `#if defined(__GCCE__)`.
4. **`--exceptions` unknown / EH still off** -> RVCT 2.2 supports `--exceptions --rtti`;
   verify the option reached armcc in the verbose log.

---

## Dead ends — DO NOT retry these

- ❌ **Fixing C++ EH under GCCE** (months of attempts): linking Symbian vs generic
  libgcc/libsupc++, custom `__cxa_allocate_exception`, weak typeinfo/RTTI stubs,
  `-fno-exceptions` experiments, elf2e32 exception-descriptor fiddling. GCCE 3.4.3
  EH on S60 3rd is fundamentally unreliable here. **Use RVCT instead.**
- ❌ **Custom `__cxa_allocate_exception` via User::Alloc** — was unsafe (didn't reserve
  the `__cxa_exception` header); rolled back. Never reintroduce.
- ❌ **Faking RTTI/typeinfo** (`_ZTI15XLeaveException = 0`, fake `__class_type_info`
  vtables) — caused KERN-EXEC 3. Removed. `drtaeabi.dll` provides the real ones.
- ❌ **Empty `__cpp_initialize__aeabi_`** — suppressed all static init -> crash. Removed.
- ❌ **`libc.lib` / `estlib.lib` blindly** — `libc.dso` absent in this SDK.
- ❌ **Whisk3D's exact ARMV6 + `--fpu vfpv2`** — that targets a newer SDK. Our 9.2 SDK
  uses **ARMV5** (runs fine on N95's ARM11). Keep ARMV5.

---

## Code fixes already in place (still valid, toolchain-independent)

These were real bugs fixed earlier and remain correct:
1. **Blocking UI loop removed** — game loop moved from `ConstructL` into a `CPeriodic`
   heartbeat (`RenderTick` ~16ms, `EPriorityIdle`). Was freezing the window server.
2. **`BaseConstructL()`** called for proper Avkon init.
3. **GL wired**: `SetGLInterface(...)`, plus `SetMopParent()` + `AddToStackL()` for the GL view.
4. **PNG/JPEG decoder**: real Symbian ICL (`CImageDecoder` -> `CFbsBitmap(EColor16MA)` ->
   ARGB `0xAARRGGBB` buffer) via `imageconversion.lib`. Replaced the purple/green checkerboard.
5. **UID2 fixed** to `0x100039CE` (GUI app); was wrongly `0x1000007A`.
6. **EPOCHEAPSIZE** up to 64MB, **EPOCSTACKSIZE** 80KB.
7. **`newdel_compat.cpp`**: `operator new/delete` -> `User::Alloc/Free` (no `scppnwdl.dll`
   on N95). Guarded `#if !defined(__WINS__)` (works for both GCCE and RVCT).
8. **`Resources_stub.cpp`** compiled instead of full `Resources.cpp`.
9. Case-insensitive PAK lookup typo fixed.

---

## Environment

- SDK: `C:\Symbian\9.2\S60_3rd_FP1_2` (S60 3rd FP1, Symbian 9.2)
- RVCT: `C:\Symbian\ARM\RVCT\Programs\2.2\349\win_32-pentium` (preferred)
- GCCE (legacy/fallback): CSL ARM Toolchain (arm-none-symbianelf-g++ 3.4.3)
- Build copy on dev machine: `C:\Symbian\PvZ-N95-Port-main` (git pull here, then run build_rvct.cmd)

## Security
- A GitHub PAT was exposed in chat earlier — **revoke it** (GitHub Settings ->
  Developer settings -> Personal access tokens) if not already done.
