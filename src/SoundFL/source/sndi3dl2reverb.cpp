#include "sndi3dl2reverb.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndglobals.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073bb50
I3DL2Reverb::I3DL2Reverb()
{
    mRoomGain = 0.0f;
    mRoomHFGain = 0.0f;
    mRoomRollOff = 0.0f;
    mDecayTime = 1.0f;
    mDecayHFRatio = 0.5f;
    mERGain = 0.0f;
    mERDelay = 0.02f;
    mReverbGain = 0.0f;
    mReverbDelay = 0.04f;
    mDiffusion = 100.0f;
    mDensity = 100.0f;
    mHFReference = 5000.0f;
}

// FUNCTION: SWBF2 0x0073bbb0
bool I3DL2Reverb::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    bool bHandled = true;
    switch (uiPropertyID)
    {
    case 0x5676AB7Du:
        chunk._pFile->Read8();
        mRoomGain = Clamp(chunk._pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_roomGainChanged;
        break;
    case 0x89695A93u:
        chunk._pFile->Read8();
        mRoomHFGain = Clamp(chunk._pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_roomHFGainChanged;
        break;
    case 0xB3571EDAu:
        chunk._pFile->Read8();
        mRoomRollOff = Clamp(chunk._pFile->ReadFloat(), 0.0f, 10.0f);
        mFlags |= flags_roomRollOffChanged;
        break;
    case 0x1C098B3Cu:
        chunk._pFile->Read8();
        mDecayTime = Clamp(chunk._pFile->ReadFloat(), 0.01f, 20.0f);
        mFlags |= flags_decayTimeChanged;
        break;
    case 0x80D9B27Eu:
        chunk._pFile->Read8();
        mDecayHFRatio = Clamp(chunk._pFile->ReadFloat(), 0.01f, 2.0f);
        mFlags |= flags_decayHFRatioChanged;
        break;
    case 0xF2C65989u:
        chunk._pFile->Read8();
        mERGain = Clamp(chunk._pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_erGainChanged;
        break;
    case 0xBE463E19u:
        chunk._pFile->Read8();
        mERDelay = Clamp(chunk._pFile->ReadFloat(), 0.0f, 0.3f);
        mFlags |= flags_erDelayChanged;
        break;
    case 0x6C408CFEu:
        chunk._pFile->Read8();
        mReverbGain = Clamp(chunk._pFile->ReadFloat(), 0.0f, 10.0f);
        mFlags |= flags_reverbGainChanged;
        break;
    case 0xB2F683D8u:
        chunk._pFile->Read8();
        mReverbDelay = Clamp(chunk._pFile->ReadFloat(), 0.0f, 0.1f);
        mFlags |= flags_reverbDelayChanged;
        break;
    case 0xC8A3E804u:
        chunk._pFile->Read8();
        mDiffusion = Clamp(chunk._pFile->ReadFloat(), 0.0f, 100.0f);
        mFlags |= flags_diffusionChanged;
        break;
    case 0x72B9059Bu:
        chunk._pFile->Read8();
        mDensity = Clamp(chunk._pFile->ReadFloat(), 0.0f, 100.0f);
        mFlags |= flags_densityChanged;
        break;
    case 0xE1A099A8u:
        chunk._pFile->Read8();
        mHFReference = Clamp(chunk._pFile->ReadFloat(), 20.0f, 20000.0f);
        mFlags |= flags_hrReferenceChanged;
        break;
    default:
        bHandled = false;
        break;
    }
    return bHandled;
}

}
