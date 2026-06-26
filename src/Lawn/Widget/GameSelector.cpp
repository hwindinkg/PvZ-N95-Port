/*
 * GameSelector.cpp -- [M3] First REAL game frame.
 *
 * KEY FIX (2026-06-26): the previous build called
 *   mResourceManager->LoadImageByResName("IMAGE_BACKGROUND1")
 * inside Draw. That function does NOT cache -- it re-reads the PAK and
 * re-DECODES the image every call. The 1400x600 lawn JPEG was already decoded
 * once during the bulk resource load; decoding a SECOND copy here fails on the
 * N95's tight heap -> returned NULL -> magenta fallback (exactly what gs_log.txt
 * showed: "bg=NULL (LoadImageByResName failed)").
 *
 * The engine already keeps the loaded lawn in the global Sexy::IMAGE_BACKGROUND1
 * (set via GetImageThrow during ExtractDelayLoad_Background1Resources) -- this is
 * the SAME pointer Board::DrawBackdrop uses. So we just draw that. No reload, no
 * second decode. Fallback to ResourceManager::GetImage (cache lookup, still no
 * reload) only if the global is somehow NULL.
 *
 * Drawn with a PLAIN DrawImage(bg, 0, 0) (no transform): the canvas is 400x300,
 * so we see the top-left 400x300 of the 1400x600 lawn = house + start of rows =
 * an unmistakable real frame. gs_log.txt records the source + bits presence.
 */
#include "GameSelector.h"

#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../engine/ResourceManager.h"
#include "../../engine/Graphics.h"
#include "../../engine/Image.h"

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

    // Use the ALREADY-LOADED lawn -- never reload/redecode here.
    static Image* sBg    = NULL;
    static bool   sTried = false;
    static int    sSrc   = 0;   // 0=none 1=global 2=cache
    if (!sTried)
    {
        sTried = true;
        if (IMAGE_BACKGROUND1 != NULL)             // engine global (canonical)
        {
            sBg = IMAGE_BACKGROUND1;
            sSrc = 1;
        }
        else if (mApp != NULL && mApp->mResourceManager != NULL)
        {
            sBg = mApp->mResourceManager->GetImage("IMAGE_BACKGROUND1"); // cache, no reload
            sSrc = sBg ? 2 : 0;
        }
    }

    // One-time diagnostic dump.
    static bool sLogged = false;
    if (!sLogged)
    {
        sLogged = true;
        TBuf8<256> b;
        if (sBg != NULL)
        {
            b.Format(_L8("GS:Draw RAN src=%d bg=%08x w=%d h=%d\n"),
                     sSrc, (TUint)sBg, sBg->GetWidth(), sBg->GetHeight());
        }
        else
        {
            b.Format(_L8("GS:Draw RAN bg=NULL src=%d (global+cache both empty)\n"), sSrc);
        }
        GSLog(b);
    }

    if (sBg != NULL && sBg->GetWidth() > 0 && sBg->GetHeight() > 0)
    {
        // PLAIN draw, no transform -- top-left 400x300 of the 1400x600 lawn.
        g->DrawImage(sBg, 0, 0);
    }
    else
    {
        // Background unavailable -> obvious magenta so we can tell this branch ran.
        g->SetColor(Color(255, 0, 255, 255));
        g->FillRect(0, 0, 400, 300);
    }
}

} // namespace Sexy