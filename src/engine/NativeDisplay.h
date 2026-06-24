#ifndef NATIVE_DISPLAY_H
#define NATIVE_DISPLAY_H

#include <e32base.h>
#include <GLES/gl.h>

class NativeDisplay
{
public:
    NativeDisplay();

    static int mRedBits, mGreenBits, mBlueBits, mAlphaBits;
    static int mRedShift, mGreenShift, mBlueShift, mAlphaShift;

    static void Init();
};

#endif // NATIVE_DISPLAY_H
