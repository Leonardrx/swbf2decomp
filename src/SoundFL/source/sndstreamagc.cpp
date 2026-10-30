#include <stdlib.h>

#include "sndstreamagc.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00741da0
StreamAGC::StreamAGC()
    : mClipThreshold(0x7FFF),
      mGateThreshold(0),
      mGainDest(0x7FFF),
      mGain(0x7FFF),
      mGainMax(0x7FFF),
      mShiftRate(4)
{
    mShiftRateMask = 0xFFFFFFF8u;
}

// FUNCTION: SWBF2 0x00741e00
StreamAGC::~StreamAGC()
{
}


}

// FUNCTION: SWBF2 0x00741e40
void Snd::StreamAGC::Flush()
{
    void* pAGC = (void*)this;
    Snd::StreamAGC* const pThis = (Snd::StreamAGC*)pAGC;
    pThis->mGain = pThis->mGainDest;
}

namespace Snd
{

// FUNCTION: SWBF2 0x00741e50
StreamPacket* StreamAGC::GetPacket(unsigned int uiOutput)
{
    (void)uiOutput;

    if (GetCurrentPlayState() != playState_playing)
    {
        return 0;
    }

    StreamPacket* const pPacket = mConnection.mInputNode != 0
        ? mConnection.mInputNode->GetPacket(mConnection.mInputNodeOutputIndex)
        : 0;
    if (pPacket != 0)
    {
        short* const pSamples = (short*)pPacket->mBuffer;
        const unsigned int uiNumSamples = pPacket->mBufferUsed >> 1;
        mPlaybackPosition = pPacket->mPos;

        for (unsigned int i = 0; i < uiNumSamples; i++)
        {
            const unsigned short uLevel = (unsigned short)abs(pSamples[i]);
            unsigned __int64 uValue = mGain * uLevel;

            const unsigned int uiDesiredGain = uLevel > mGateThreshold
                ? (mClipThreshold * 0x7FFFu) / uLevel : 0;
            mGainDest = uiDesiredGain > mGainMax ? mGainMax : uiDesiredGain;

            uValue >>= 16;
            if (uValue > 0x7FFF)
            {
                uValue = 0x7FFF;
            }
            const int iValue = (int)uValue;
            pSamples[i] = (short)(pSamples[i] < 0 ? -iValue : iValue);

            if (mGainDest < mGain)
            {
                const unsigned int uDelta = mGain - mGainDest;
                if ((uDelta & mShiftRateMask) != 0)
                {
                    mGain = mGain - (uDelta >> mShiftRate) - 1;
                }
                else
                {
                    mGain = mGainDest;
                }
            }
            else if (mGainDest > mGain)
            {
                const unsigned int uDelta = mGainDest - mGain;
                if ((uDelta & mShiftRateMask) != 0)
                {
                    mGain = mGain + (uDelta >> mShiftRate) + 1;
                }
                else
                {
                    mGain = mGainDest;
                }
            }
        }

        if ((pPacket->mFlags & 1) != 0)
        {
            Stop();
        }

    }

    return pPacket;
}

// FUNCTION: SWBF2 0x00741fa0
void StreamAGC::Update()
{
    if (StreamNode::UpdateNode(false))
    {
        mGain = mGainDest;
    }
}

}
