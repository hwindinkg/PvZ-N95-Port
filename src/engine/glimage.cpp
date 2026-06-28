#include "GLImage.h"
#include "SymbianFixes.h"

namespace Sexy {

GLImage::GLImage()
    : MemoryImage()
    , mPixelFormat(GL_RGBA)
    , mTextureBits(32)
    , mTexturePieces(NULL)
    , mNumPieces(0)
    , mBltModulated(false)
{
}

GLImage::~GLImage()
{
    DeleteTexture();
}

void GLImage::SetPixelFormat(int fmt)
{
    mPixelFormat = fmt;
}

void GLImage::CreateTexture()
{
    DeleteTexture();

    if (!mBits || mWidth <= 0 || mHeight <= 0)
    {
        return;
    }

    // Determine piece count based on row/col subdivision
    TInt pieceCount = 1;
    if (mNumRows > 0 && mNumCols > 0)
    {
        pieceCount = mNumRows * mNumCols;
    }
    else if (mNumRows > 0)
    {
        pieceCount = mNumRows;
    }
    else if (mNumCols > 0)
    {
        pieceCount = mNumCols;
    }

    mNumPieces = pieceCount;
    mTexturePieces = new TexturePiece[pieceCount];

    if (!mTexturePieces)
    {
        mNumPieces = 0;
        return;
    }

    TInt i;
    for (i = 0; i < pieceCount; i++)
    {
        mTexturePieces[i].mTexID   = 0;
        mTexturePieces[i].mWidth   = 0;
        mTexturePieces[i].mHeight  = 0;
        mTexturePieces[i].mX       = 0;
        mTexturePieces[i].mY       = 0;
    }

    if (pieceCount == 1)
    {
        // Single texture for the entire image
        glGenTextures(1, &mTexturePieces[0].mTexID);
        glBindTexture(GL_TEXTURE_2D, mTexturePieces[0].mTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            mPixelFormat,
            mWidth,
            mHeight,
            0,
            mPixelFormat,
            GL_UNSIGNED_BYTE,
            mBits);

        mTexturePieces[0].mWidth  = mWidth;
        mTexturePieces[0].mHeight = mHeight;
        mTexturePieces[0].mX      = 0;
        mTexturePieces[0].mY      = 0;
    }
    else
    {
        // Multi-piece: create one texture per cell defined by mRows/mCols
        TInt idx = 0;
        TInt r, c;

        for (r = 0; r < mNumRows; r++)
        {
            TInt y0   = mRows ? mRows[r] : 0;
            TInt y1   = (r + 1 < mNumRows && mRows) ? mRows[r + 1] : mHeight;
            TInt ch   = y1 - y0;
            if (ch <= 0) ch = 1;

            for (c = 0; c < mNumCols; c++)
            {
                TInt x0 = mCols ? mCols[c] : 0;
                TInt x1 = (c + 1 < mNumCols && mCols) ? mCols[c + 1] : mWidth;
                TInt cw = x1 - x0;
                if (cw <= 0) cw = 1;

                if (idx < mNumPieces)
                {
                    glGenTextures(1, &mTexturePieces[idx].mTexID);
                    glBindTexture(GL_TEXTURE_2D, mTexturePieces[idx].mTexID);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                    // Upload the sub-rect. In a full implementation we would
                    // extract the sub-rect pixels from mBits into a temporary
                    // buffer. For now, upload the full image as a placeholder.
                    // TODO: extract sub-rect for proper sprite-sheet support
                    glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        mPixelFormat,
                        mWidth,
                        mHeight,
                        0,
                        mPixelFormat,
                        GL_UNSIGNED_BYTE,
                        mBits);

                    mTexturePieces[idx].mWidth  = cw;
                    mTexturePieces[idx].mHeight = ch;
                    mTexturePieces[idx].mX      = x0;
                    mTexturePieces[idx].mY      = y0;

                    idx++;
                }
            }
        }
    }
}

void GLImage::DeleteTexture()
{
    if (mTexturePieces)
    {
        // NOTE: glDeleteTextures is not yet declared in the stub layer.
        // Texture resource cleanup is handled when the EGL context is destroyed.
        // A future implementation should call glDeleteTextures for each piece.
        //
        // TInt i;
        // GLuint* ids = new GLuint[mNumPieces];
        // for (i = 0; i < mNumPieces; i++) { ids[i] = mTexturePieces[i].mTexID; }
        // glDeleteTextures(mNumPieces, ids);
        // delete[] ids;

        delete[] mTexturePieces;
        mTexturePieces = NULL;
    }
    mNumPieces = 0;
}

} // namespace Sexy
