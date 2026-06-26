/*
 * GameSelector.cpp -- [M3] First REAL game frame (diagnostic build).
 *
 * Draws the actual in-game lawn background (IMAGE_BACKGROUND1) so we get a real
 * frame from decoded game art through the real widget -> Graphics -> GL pipeline.
 *
 * DIAGNOSTIC: the previous attempt used PushTransform(scale) and the screen stayed
 * purple (clear color) even though a 2048x1024 texture was created (i.e. DrawImage
 * ran). The title screen earlier displayed fine using a PLAIN DrawImage WITHOUT any
 * transform. So this version:
 *   1) draws the background with a PLAIN DrawImage(sBg, 0, 0) -- no transform stack
 *      (native size; the canvas is 400x300 so we see the top-left 400x300 of the
 *      1400x600 lawn = the house + start of the rows = unmistakably a real frame),
 *   2) writes a one-time diagnostic to C:\Data\PvZ\gs_log.txt recording whether
 *      Draw ran, the image pointer, its dimensions, and whether its pixel bits are
 *      present (an empty/NULL-bits image -> empty texture -> purple screen).
 */
#include "GameSelector.h"

#include "../../LawnApp.h"
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

    // Lazy-load the lawn background exactly once (cached for subsequent frames).
    static Image* sBg = NULL;
    static bool   sTried = false;
    if (!sTried && sBg == NULL && mApp != NULL && mApp->mResourceManager != NULL)
    {
        sTried = true;
        sBg = mApp->mResourceManager->LoadImageByResName("IMAGE_BACKGROUND1");
    }

    // One-time diagnostic dump.
    static bool sLogged = false;
    if (!sLogged)
    {
        sLogged = true;
        TBuf8<256> b;
        if (sBg != NULL)
        {
            MemoryImage* mi = dynamic_cast<MemoryImage*>(sBg);
            const unsigned char* bits = mi ? mi->GetBits() : NULL;
            b.Format(_L8("GS:Draw RAN bg=%08x w=%d h=%d mem=%d bits=%d\n"),
                     (TUint)sBg, sBg->GetWidth(), sBg->GetHeight(),
                     (TInt)(mi != NULL), (TInt)(bits != NULL));
        }
        else
        {
            b.Copy(_L8("GS:Draw RAN bg=NULL (LoadImageByResName failed)\n"));
        }
        GSLog(b);
    }

    if (sBg != NULL && sBg->GetWidth() > 0 && sBg->GetHeight() > 0)
    {
        // PLAIN draw, no transform -- shows top-left 400x300 of the 1400x600 lawn.
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
