/*
 * GameSelector.cpp -- [M3] First REAL game frame.
 *
 * WHY NOT THE LAWN (IMAGE_BACKGROUND1)? Two hard N95 limits, both proven by logs:
 *   1) Decoding the 1400x600 lawn to 32-bit fails on the device heap -- gs_log
 *      previously read "bg=NULL (LoadImageByResName failed)" and rmgr_log cut off
 *      mid-convert with no "decoded OK".
 *   2) Even decoded, its POT texture is 2048x1024, but PowerVR MBX Lite (N95) has
 *      GL_MAX_TEXTURE_SIZE = 1024 -> glTexImage2D rejects it -> it renders as
 *      nothing (you see only the purple clear colour).
 *
 * So for the FIRST real frame we draw IMAGE_TITLESCREEN: an 800x600 JPEG that
 * decodes robustly (it's the very first asset the loader ever touched) and whose
 * POT texture is 1024x1024 -- exactly at the MBX limit, so it uploads fine.
 *
 * It was DeleteImage'd after the loading screen, so GetImage re-decodes it on
 * demand (and caches it). We draw it at native size at (0,0): the screen's
 * logical canvas is ~400x300, so the top-left 400x300 of the 800x600 title art is
 * visible -- unmistakable real pixels. No transform, no scale (keeps the GL path
 * dead simple for this proof). Scaling to fit comes next once a frame is visible.
 *
 * Drawn via static_cast<MemoryImage*> + the direct DrawImage(MemoryImage*)
 * overload, bypassing the dynamic_cast in DrawImageF (RTTI is unreliable under
 * GCCE 3.4.3; the engine copy of that cast was also switched to static_cast).
 */
#include "GameSelector.h"

#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../engine/ResourceManager.h"
#include "../../engine/Graphics.h"
#include "../../engine/Image.h"
#include "../../engine/MemoryImage.h"

#include <e32std.h>
#include <f32file.h>

namespace Sexy {

static void GSLog(const TDesC8& aMsg)
{
    RFs fs; RFile f;
    if (fs.Connect() == KErrNone)
    {
        fs.MkDirAll(_L("C:\\Data\\PvZ"));
        if (f.Replace(fs, _L("C:\\Data\\PvZ\\gs_log.txt"), EFileWrite) == KErrNone)
        {
            f.Write(aMsg);
            f.Close();
        }
        fs.Close();
    }
}

void GameSelector::Draw(Graphics* g)
{
    if (g == NULL)
        return;

    // Retry each frame until the image is available (GetImage caches on success,
    // so this is cheap once loaded -- and we never permanently cache a NULL).
    static Image* sImg = NULL;
    if (sImg == NULL && mApp != NULL && mApp->mResourceManager != NULL)
        sImg = mApp->mResourceManager->GetImage("IMAGE_TITLESCREEN");

    // One-time diagnostic dump.
    static bool sLogged = false;
    if (!sLogged && sImg != NULL)
    {
        sLogged = true;
        TBuf8<160> b;
        b.Format(_L8("GS:Draw drawing IMAGE_TITLESCREEN %dx%d ptr=%08x\n"),
                 sImg->GetWidth(), sImg->GetHeight(), (TUint)sImg);
        GSLog(b);
    }
    else if (!sLogged && mApp != NULL && mApp->mResourceManager != NULL)
    {
        // image still not ready this frame -- record the miss once-ish
        static bool sMissLogged = false;
        if (!sMissLogged)
        {
            sMissLogged = true;
            GSLog(_L8("GS:Draw IMAGE_TITLESCREEN not ready yet (GetImage NULL)\n"));
        }
    }

    if (sImg != NULL && sImg->GetWidth() > 0 && sImg->GetHeight() > 0)
    {
        // Native draw at top-left. static_cast is safe: ResourceManager only ever
        // produces MemoryImage-backed images; the direct overload skips the
        // RTTI-dependent dynamic_cast inside DrawImageF.
        MemoryImage* mem = static_cast<MemoryImage*>(sImg);
        g->DrawImage(mem, 0, 0);
    }
    else
    {
        // Nothing to draw yet -> obvious magenta so this branch is visible.
        g->SetColor(Color(255, 0, 255, 255));
        g->FillRect(0, 0, 400, 300);
    }
}

} // namespace Sexy
