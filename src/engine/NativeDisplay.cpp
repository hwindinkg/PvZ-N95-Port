#include "NativeDisplay.h"

int NativeDisplay::mRedBits   = 0;
int NativeDisplay::mGreenBits = 0;
int NativeDisplay::mBlueBits  = 0;
int NativeDisplay::mAlphaBits = 0;
int NativeDisplay::mRedShift   = 0;
int NativeDisplay::mGreenShift = 0;
int NativeDisplay::mBlueShift  = 0;
int NativeDisplay::mAlphaShift = 0;

NativeDisplay::NativeDisplay()
{
}

void NativeDisplay::Init()
{
    // Hardcoded RGBA 8-8-8-8 pixel format for GLES
    mRedBits   = 8;
    mGreenBits = 8;
    mBlueBits  = 8;
    mAlphaBits = 8;

    // Byte order for GL_RGBA on little-endian:
    // byte 0 (LSB) = R, byte 1 = G, byte 2 = B, byte 3 (MSB) = A
    mRedShift   = 0;
    mGreenShift = 8;
    mBlueShift  = 16;
    mAlphaShift = 24;
}
