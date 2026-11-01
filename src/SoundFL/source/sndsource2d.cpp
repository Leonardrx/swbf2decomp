#include "sndsource2d.h"
#include "sndglobals.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblChunk.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074bd30
void Source2D::CalculateGain(float fGain, float fReverbGain,
                             GainMatrix& rMatrix) const
{
    const float fPan = mPan;
    if (fPan > 0.0f)
    {
        rMatrix.frontLeft = (1.0f - fPan) * fGain;
        rMatrix.frontRight = fGain;
    }
    else
    {
        rMatrix.frontLeft = fGain;
        rMatrix.frontRight = (mPan + 1.0f) * fGain;
    }

    rMatrix.rearLeft = rMatrix.frontLeft;
    rMatrix.rearRight = rMatrix.frontRight;
    rMatrix.reverb = fReverbGain;
}

// FUNCTION: SWBF2 0x0074bda0
bool Source2D::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    if (uiPropertyID != 0x4F55EEECu)
    {
        return false;
    }
    chunk._pFile->Read8();
    SetPan(Clamp(chunk._pFile->ReadFloat(), -1.0f, 1.0f));
    return true;
}

}
