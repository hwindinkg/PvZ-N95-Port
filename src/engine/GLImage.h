#ifndef GL_IMAGE_H
#define GL_IMAGE_H

#include <e32base.h>
#include <GLES/gl.h>
#include "MemoryImage.h"

namespace Sexy {

class GLImage : public MemoryImage
{
public:
    GLImage();
    ~GLImage();

    int mPixelFormat;   // GL_RGBA, etc.
    int mTextureBits;   // bit depth

    // Hardware texture IDs per piece
    struct TexturePiece
    {
        GLuint mTexID;
        int mWidth, mHeight;
        int mX, mY; // position in source image
    };

    TexturePiece* mTexturePieces;
    int           mNumPieces;

    void CreateTexture();
    void DeleteTexture();
    void SetPixelFormat(int fmt);

    bool IsBltModulated() const { return mBltModulated; }
    void SetBltModulated(bool mod) { mBltModulated = mod; }

private:
    bool mBltModulated;
};

} // namespace Sexy

#endif // GL_IMAGE_H
