# Building PvZ_N95.sisx

This repo includes an **automated builder** that finds everything it needs
(SDK, ARM toolchain, Perl, signing keys) and produces a signed `.sisx`.

## Quick start (Windows)

1. **Clone** this repository.
2. Make sure you have installed:
   - **S60 3rd Edition FP1 SDK** (provides `epoc32`, `makesis`, `signsis`, `elf2e32`)
   - **CSL ARM toolchain (GCCE)** — `arm-none-symbianelf-g++`
3. (Optional) Put your **`key.cer`** and **`key.key`** next to `build_sisx.cmd`.
   - If absent, the builder auto-generates a self-signed certificate.
4. Double-click **`build_sisx.cmd`** (or run it in a terminal).

The script:
- auto-detects the SDK (checks `EPOCROOT`, common paths, then scans drives),
- auto-detects the GCCE toolchain and Perl,
- compiles all 39 source files,
- links + converts to an E32 image (`elf2e32`),
- packages the 3 precompiled resources (`data/*.rsc`),
- runs `makesis` + `signsis`.

**Output:** `build/out/PvZ_N95.sisx` — copy it to your Nokia N95 and install.

## Notes
- The ICL PNG decoder requires `imageconversion.dso` — already wired into the
  link + `elf2e32` steps by the builder.
- If your SDK is in a non-standard location, set `EPOCROOT` before running:
  `set EPOCROOT=D:\Symbian\9.2\S60_3rd_FP1_2` then run `build_sisx.cmd`.
- Resource sources (`data/*.rss`) are included for reference; the builder uses
  the committed precompiled `.rsc` files for reliability.
