// src/engine/Stubs.cpp -- port-specific symbols ONLY.
//
// GCCE build (mirrors the working Whisk3D reference). The C runtime
// (str*/fmod/rand), soft-float EABI helpers, operator new/delete and the
// C++ exception + RTTI runtime are ALL provided by the standard GCCE link
// (libgcc / libsupc++) plus estlib.lib + libc.lib. The hand-rolled versions
// that used to live here were built on the false premise that "no C++ runtime
// is needed". Under GCCE they multiply-define against the real runtime and the
// fake EH/RTTI stubs broke real TRAP/Leave unwinding -> KERN-EXEC 3 at the first
// User::Leave inside BaseConstructL. This file now holds nothing but genuinely
// port-specific symbols.
#include "SexyAppBase.h"
#include "Image.h"
#include "../Lawn/LawnCommon.h"

namespace Sexy { SexyAppBase* gSexyAppBase = NULL; }

void DrawSeedPacket(Sexy::Graphics* g, float x, float y, int st, int it, float al, int gr, bool sel, bool im) {
    (void)g; (void)x; (void)y; (void)st; (void)it; (void)al; (void)gr; (void)sel; (void)im;
}
