/* engine/ImageLib.cpp - Stub for Symbian port
 * Image loading will use Symbian ICL (Image Conversion Library) later.
 * Based on PvZ-Portable ImageLib.cpp framework. */

#include "Common.h"
#include "ImageLib.h"
#include "Image.h"
#include "PakInterface.h"

namespace Sexy {

int ImageLib::gAlphaComposeColor = 0xFFFFFF;
bool ImageLib::gAutoLoadAlpha = true;
bool ImageLib::gIgnoreJPEG2000Alpha = true;

Image* GetPNGImage(const std::string& theFileName)
{
    (void)theFileName;
    return NULL;
}

Image* GetTGAImage(const std::string& theFileName)
{
    (void)theFileName;
    return NULL;
}

Image* GetGIFImage(const std::string& theFileName)
{
    (void)theFileName;
    return NULL;
}

Image* GetJPEGImage(const std::string& theFileName)
{
    (void)theFileName;
    return NULL;
}

bool ImageLib::WriteJPEGImage(const std::string& theFileName, Image* theImage)
{
    (void)theFileName;
    (void)theImage;
    return false;
}

bool ImageLib::WritePNGImage(const std::string& theFileName, Image* theImage)
{
    (void)theFileName;
    (void)theImage;
    return false;
}

bool ImageLib::WriteTGAImage(const std::string& theFileName, Image* theImage)
{
    (void)theFileName;
    (void)theImage;
    return false;
}

} // namespace Sexy