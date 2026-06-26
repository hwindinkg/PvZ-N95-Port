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
 *  - IMAGE_BACKGROUND1 is 1400x600 (left ~800x600 = the lawn playfield), and lives
 *    in the "DelayLoad_Background1" group
 *    which is NOT loaded at menu time, so we lazy-load just that one image on first
 *    Draw via ResourceManager::LoadImageByResName (the same per-image loader the
 *    resource log shows working 264x). If it isn't in the PAK we simply skip.
 *  - Graphics::DrawImage/DrawImageF draw at the image's NATIVE size and IGNORE
 *    Graphics' mScaleX/Y, so we push a UNIFORM 0.5 scale via PushTransform()/
 *    PopTransform() -> canvas shows the left 800x600 playfield at correct aspect.
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
        // background1.jpg is 1400x600: the LEFT ~800x600 region is the actual
        // lawn playfield (house + 5 rows + the near grass), the rest is off-screen
        // scroll area. The logical canvas is 400x300, so a UNIFORM 0.5 scale maps
        // that left 800x600 onto the full 400x300 screen with the CORRECT aspect
        // ratio (no horizontal squish). DrawImage draws at native size from (0,0),
        // and DrawImageF/DrawImage IGNORE Graphics' mScale, so we apply the scale
        // via the transform stack (which ApplyTransform feeds to GL each Flush).
        const float kScale = 300.0f / static_cast<float>(sBg->GetHeight()); // 0.5
        SexyTransform2D t(true);   // identity
        t.Scale(kScale, kScale);
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