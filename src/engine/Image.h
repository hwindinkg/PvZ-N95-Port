#ifndef IMAGE_H
#define IMAGE_H

#include <e32base.h>

namespace Sexy {

class Image
{
public:
    Image();
    Image(int w, int h);
    virtual ~Image();

    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    int GetAbsWidth() const { return mWidth; }
    int GetAbsHeight() const { return mHeight; }
    int GetNumRows() const { return mNumRows; }
    int GetNumCols() const { return mNumCols; }
    int GetCelWidth() const { return mNumCols > 0 ? mWidth / mNumCols : mWidth; }
    int GetCelHeight() const { return mNumRows > 0 ? mHeight / mNumRows : mHeight; }
    int GetRow(int col) const { return mRows ? mRows[col] : 0; }
    int GetCol(int row) const { return mCols ? mCols[row] : 0; }

    bool mVerticalMode;
    int mWidth, mHeight;
    int mNumRows, mNumCols;
    int* mRows;
    int* mCols;
    int mS, mT; // for texture coordinates

    virtual void SetSize(int w, int h);
};

} // namespace Sexy

#endif // IMAGE_H
