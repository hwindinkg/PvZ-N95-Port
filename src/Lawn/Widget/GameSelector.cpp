/*
 * GameSelector.cpp -- [M3] First REAL game frame.
 *
 * The original GameSelector is the main-menu widget. Its full content (Adventure/
 * Survival buttons, zen garden, etc.) is still being ported, so for now this draws
 * the actual in-game lawn background (IMAGE_BACKGROUND1, the day level) full-screen.
 * This is the first frame produced from REAL decoded game art going through the REAL
 * widget -> Graphics -> GL pipeline (not a stub test pattern).
 *
 * Key facts that shape this code:
 *  - The logical canvas is 400x300 (GL ortho, see GLInterface::SetProjection).
 *  - IMAGE_BACKGROUND1 is 800x600, and lives in the "DelayLoad_Background1" group
 *    which is NOT loaded at menu time, so we lazy-load just that one image on first
 *    Draw via ResourceManager::LoadImageByResName (the same per-image loader the
 *    resource log shows working 264x). If it isn't in the PAK we simply skip.
 *  - Graphics::DrawImage/DrawImageF draw at the image's NATIVE size and IGNORE
 *    Graphics' mScaleX/Y, so to fit 800x600 into the 400x300 canvas we must push a
 *    scale transform (0.5, 0.5) via PushTransform()/PopTransform().
 */
#include "GameSelector.h"

#include "../../LawnApp.h"
#include "../../engine/ResourceManager.h"
#include "../../engine/Graphics.h"
#include "../../engine/Image.h"
#include "../../engine/SexyMatrix.h"

namespace Sexy {

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

    if (sBg != NULL && sBg->GetWidth() > 0 && sBg->GetHeight() > 0)
    {
        const float kCanvasW = 400.0f;   // GLInterface::SetProjection(0,0,400,300)
        const float kCanvasH = 300.0f;
        const float sx = kCanvasW / static_cast<float>(sBg->GetWidth());
        const float sy = kCanvasH / static_cast<float>(sBg->GetHeight());

        SexyTransform2D t(true);   // identity
        t.Scale(sx, sy);
        g->PushTransform(t);
        g->DrawImageF(sBg, 0.0f, 0.0f);
        g->PopTransform();
    }
    else
    {
        // Background not available yet -> clear to the lawn-sky purple so the frame
        // is obviously "ours" (not a stub test pattern) while we debug PAK contents.
        g->SetColor(Color(38, 13, 51));
        g->FillRect(0, 0, 400, 300);
    }
}

} // namespace Sexy
