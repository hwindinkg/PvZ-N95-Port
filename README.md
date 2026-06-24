# PvZ-N95-Port

A native **Symbian S60 3rd Edition (FP1)** port of
[PvZ-Portable](https://github.com/wszqkzqk/PvZ-Portable) — *Plants vs. Zombies*
running on the **Nokia N95** (Symbian OS 9.2, ARMv5, GCCE, OpenGL ES 1.1).

> ⚠️ **Work in progress.** Repo has the C++ source, build files
> (`.mmp`/`bld.inf`) and resource scripts. Game assets are **not** shipped —
> provide `main.pak` on the device (see *Assets*).

---

## Status (current)

The app now **boots through the whole Symbian startup**: `E32Main` →
`CPvZApplication::NewApplication` → `CreateDocumentL` → `CPvZAppUi::ConstructL`.
All the early-boot structural crashes (purple flashing screen, un-closable
window, double `E32Main`) are fixed.

**Active blocker:** a `KERN-EXEC 3` on the **very first C++ `throw`** during the
EH self-test in `ConstructL` (`boot.log` stops right after
`EH T1: raw C++ throw/catch`, before `EH T1 caught=123`). This means C++
exception support is not fully wired in the executable yet — see *Exception
handling* below for the in-progress fix.

`boot.log` / `rmgr_log.txt` / `wgt_log.txt` are written to `C:\Data\PvZ\`.

---

## Build (standard `abld` toolchain)

The working N95 GL reference
([Whisk3D](https://github.com/Dante-Leoncini/Whisk3D/tree/symbian)) builds with
the **standard Symbian `abld` system**, which emits the C++ exception-unwinding
tables (`.ARM.exidx`) and the E32 exception descriptor. We build the same way —
the old hand-rolled GCCE link did not, which is why exceptions/`User::Leave`
crashed.

Requires the **S60 3rd Edition FP1 SDK (Symbian OS 9.2)**, the **GCCE**
(`arm-none-symbianelf`) toolchain, Perl, and the **Open C / PIPS** plug-in
(provides `libc.lib`/`estlib.lib`, needed by the C++ exception allocator and the
engine's C runtime calls).

```bat
git pull
build_abld.cmd        :: detects SDK+toolchain, runs bldmake + abld build gcce urel,
                      :: then makesis + signsis -> build\out\PvZ_N95.sisx
```

Manual equivalent:

```bat
cd group
bldmake bldfiles
abld build gcce urel
makesis ..\sis\PvZ_N95.pkg
signsis PvZ_N95.sis PvZ_N95.sisx <cert>.cer <key>.key
```

> **Signing keys are intentionally NOT in this repo.** Generate your own
> self-signed cert (`makekeys`); never commit `.key`/`.cer`.

---

## Fix history

### Early-boot / structural (DONE)
| Symptom | Root cause | Fix |
|---|---|---|
| Un-closable window, **purple flashing**, system dialogs overdrawn, double `E32Main` | Whole game loop ran **inside `ConstructL`**, starving the window server | Loop moved to a **`CPeriodic` heartbeat** active object; `ConstructL` returns normally |
| Commands/exit ignored | `CAknAppUi::BaseConstructL()` never called | `BaseConstructL()` added; `SetMopParent`+`AddToStackL` for the GL control |
| `DrawAll: mGL NULL` | `Sexy::Graphics` GL pointer never set | `mGraphics->SetGLInterface(GetGL())` after `InitGLES()` |
| green/purple checkerboard art | PNG decode was a magenta placeholder | Real decode via **ICL** (`CImageDecoder`→`CFbsBitmap EColor16MA`→ARGB); added `imageconversion.lib` |
| case-insensitive PAK lookup no-op | typo `c += ('a'-'a')` | `c += ('a'-'A')` |
| Loader death before `E32Main` (no `boot.log`) | empty `__cpp_initialize__aeabi_` suppressed **all** C++ static init under `abld` | removed the empty override + dead `CallThrdProcEntry`; `MkDirAll` added to early loggers |
| `abld` link: `undefined reference to E32Main()` | `E32Main` was `extern "C"`; SDK startup (`callfirstprocessfn`) wants C++ linkage | `GLDEF_C TInt E32Main()` |
| missing `scppnwdl.dll` import (not on N95) | default `operator new/delete` | `src/newdel_compat.cpp`: new/delete → `User::Alloc/Free` |
| `.mmp` | wrong UID2 / tiny heap | UID2 `0x100039CE`, `EPOCHEAPSIZE 0x20000 0x4000000` (64 MB), `EPOCSTACKSIZE`, `CAPABILITY` |

### Exception handling (IN PROGRESS — current blocker)
Symptom: `KERN-EXEC 3` on the **first `throw`/`User::Leave`**.

Investigated and changed so far:
1. **Removed fake EH typeinfo** `_ZTI15XLeaveException = 0` (Stubs.cpp) — it was a
   4-byte-zero object the runtime dereferenced as a `type_info*` → NULL vtable call.
2. **Removed fake RTTI base vtables** (`__cxxabiv1::__class_type_info` etc. with a
   single dummy virtual) — wrong vtable layout; the real ones come from
   **`drtaeabi.dll`** (already imported). Also removed the broken NULL-returning
   `__dynamic_cast`.
3. **`OPTION GCCE -fexceptions -frtti`** in the `.mmp` so OUR frames get
   `.ARM.exidx` unwind tables.
4. **Added `LIBRARY estlib.lib` + `LIBRARY libc.lib`** — GCCE's `throw` calls
   `__cxa_allocate_exception`, which needs a `malloc`/`free` heap. Without libc the
   first throw dies in the allocator (matches the reference, which links both).

A **layered EH self-test** lives at the top of `CPvZAppUi::ConstructL`
(`src/platform/symbian/PvZAppUi.cpp`):
- `EH T1` = raw `try{throw 123;}catch` → isolates pure C++ EH / `.ARM.exidx`.
- `EH T2` = `TRAPD(.., User::Leave(-42))` → isolates the Symbian leave mechanism.

**Reading the result in `boot.log`:**
- `EH T1 caught=123` **and** `EH T2 caught err=-42` → EH works; remove the test and proceed to game init.
- `EH T1` logs but no `caught` → pure C++ EH still broken (exidx/exception-descriptor/allocator).
- `EH T1 caught=123` but `EH T2` dies → Symbian `TTrap`/leave path (see `SymbianFixes.cpp` no-op `TTrap::Trap`).

> If the device now fails to **load** (no `boot.log` at all), `libc.dll` may be
> absent on the N95 firmware — install the **Open C / PIPS** runtime, or revisit
> whether the exception allocator can be backed by `User::Alloc` instead.

---

## Notes for the next session
- Build = `build_abld.cmd` (standard `abld`), NOT the old custom GCCE link script.
- `src/engine/Stubs.cpp` + `src/engine/SymbianFixes.cpp` hold the remaining
  runtime stubs (soft-float `__aeabi_*`, `__cxa_*`, no-op `TTrap::Trap`). Several
  were bandaids for the old link; prefer letting `euser`/`drtaeabi`/`libc`/
  `libsupc++` provide the real ones (the linker uses a local weak/strong def in
  preference to a `.dso` import, so a local stub silently shadows the real symbol).
- Reference to diff against: **Whisk3D** (working N95 GL app; links `libc.lib`/
  `estlib.lib`, no custom EH stubs).

---

## Assets
Place a converted PvZ-Portable `main.pak` at `C:\Data\PvZ\main.pak` before launching.

## Project layout
```
group/   bld.inf, PvZ_N95.mmp        (build config)
data/    *.rss                        (Avkon resources + registration)
sis/     PvZ_N95.pkg                  (installer descriptor)
src/
  main_symbian.cpp                    (E32Main)
  platform/symbian/                   Application, Document, AppUi, GameView (EGL/GLES, heartbeat)
  engine/                             GLInterface, Graphics, ResourceManager (ICL), PAK/VFS, Stubs, SymbianFixes
  Lawn/ Sexy.TodLib/ ...              (ported game/engine code)
```

## Credits & License
- Original: PopCap *Plants vs. Zombies*.
- PvZ-Portable by **Zhou Qiankang (wszqkzqk)** — https://github.com/wszqkzqk/PvZ-Portable (LGPL-3.0-or-later AND LicenseRef-PopCap).
- Symbian/N95 port: this repository. Engine/port source **LGPL-3.0-or-later**; PopCap-derived content under its original license.
