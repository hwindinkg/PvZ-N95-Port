#include "Image.h"

namespace Sexy {

Image::Image()
    : mVerticalMode(false)
    , mWidth(0)
    , mHeight(0)
    , mNumRows(0)
    , mNumCols(0)
    , mRows(NULL)
    , mCols(NULL)
    , mS(0)
    , mT(0)
{
}

Image::Image(int w, int h)
    : mVerticalMode(false)
    , mWidth(w)
    , mHeight(h)
    , mNumRows(0)
    , mNumCols(0)
    , mRows(NULL)
    , mCols(NULL)
    , mS(0)
    , mT(0)
{
}

Image::~Image()
{
    delete[] mRows;
    mRows = NULL;
    delete[] mCols;
    mCols = NULL;
}

void Image::SetSize(int w, int h)
{
    mWidth = w;
    mHeight = h;
}

} // namespace Sexy
