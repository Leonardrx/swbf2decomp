#include "sndsourcetransportcontrol.h"

#include "sndglobals.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndenginebase.h"
#include "PebbleFL/Common/PblRandom.h"

namespace Snd
{
extern PblRandom sndRandom;
}

namespace Snd
{

// FUNCTION: SWBF2 0x0074b510
SourceTransportControl::SourceTransportControl()
    : mPlayProbability(1.0f),
      mPlayInterval(0.0f),
      mPlayIntervalDev(0.0f),
      mPlayIntervalNext(0.0f),
      mRandomPlaybackPosition(0)
{
}

// FUNCTION: SWBF2 0x0074b650
bool SourceTransportControl::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    switch (uiPropertyID)
    {
    case 0xA7930B6Eu:
        chunk._pFile->Read8();
        mPlayProbability = Clamp(chunk._pFile->ReadFloat(), 0.0f, 1.0f);
        return true;
    case 0x010B9ECAu:
        chunk._pFile->Read8();
        mPlayInterval = chunk._pFile->ReadFloat();
        return true;
    case 0x3AF79D76u:
        chunk._pFile->Read8();
        SetRandomPlayPosition(chunk._pFile->ReadFloat() == 1.0f);
        return true;
    case 0xEDCE1E71u:
        chunk._pFile->Read8();
        mPlayIntervalDev = chunk._pFile->ReadFloat();
        return true;
    }
    return false;
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x0074b540
bool SourceTransportControl::DeterminePlaySource(bool bArg) const
{
    const float fTime = EngineBase::GetTimeElapsed();
    if (fTime >= mPlayIntervalNext)
    {
        if (mPlayInterval != 0.0f || mPlayIntervalDev != 0.0f)
        {
            const_cast<SourceTransportControl*>(this)->mPlayIntervalNext =
                (sndRandom.GetFloat() - 0.5f) * mPlayIntervalDev * 2.0f + (fTime + mPlayInterval);
        }

        if (bArg || 1.0f == mPlayProbability)
        {
            return true;
        }
        if (mPlayProbability >= sndRandom.GetFloat())
        {
            return true;
        }
    }
    return false;
}
}
