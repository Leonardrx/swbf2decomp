#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "snddsp.h"
#include "sndstreampacket.h"
#include "sndstreamgsmencoder.h"
#include "sndstreamsourceproducer.h"
#include "sndsourcetransport.h"

extern "C" void __cdecl gsm_encode(char* pGsm, short* pFrame, unsigned char* pOut);
extern "C" int __cdecl gsm_option(void* pGsm, int iOption, int* pValue);

namespace Snd
{

// FUNCTION: SWBF2 0x007416c0
StreamGSMEncoder::StreamGSMEncoder()
    : mInputPacket(0),
      mInputPacketOffset(0),
      mInputPacketSize(0),
      mInputFrameOffset(0),
      mOutputPacketOffset(0),
      mFilter(0)
{
    mDecimate = false;
    mState.Init();
}

// FUNCTION: SWBF2 0x007419b0
void Snd::StreamGSMEncoder::SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets, unsigned int uiNumPackets)
{
    void* pGSMEncoder = (void*)this;

    ((StreamSourceProducer*)pGSMEncoder)->SetPackets(uiPacketSize, pPackets,
                                                     uiNumPackets);
}

// FUNCTION: SWBF2 0x00741780
StreamGSMEncoder::~StreamGSMEncoder()
{
    if (GetPacketSize() != 0)
    {
        Flush();
    }
}

// FUNCTION: SWBF2 0x00741870
void Snd::StreamGSMEncoder::Flush()
{

    StreamPacket* const pPacket = mInputPacket;
    if (pPacket != 0)
    {

        pPacket->Unlock();

        mInputPacket = 0;
        mInputPacketOffset = 0;
        mInputPacketSize = 0;
    }

    mInputFrameOffset = 0;
    mOutputPacketOffset = 0;

    DSP::HighLowPassFilter* const pFilter = mFilter;
    if (pFilter != 0)
    {
        pFilter->ResetState();
    }

    mState.Init();

    StreamSourceProducer::Flush();
}




// FUNCTION: SWBF2 0x007419c0
void StreamGSMEncoder::Update()
{
    if (StreamCodec::Update())
    {
        Flush();
    }

    if (!BufferPackets())
    {
        return;
    }

    StreamPacket* pOutput = GetReadPacket();
    while (pOutput != 0)
    {
        if (mInputPacket == 0)
        {
            mInputPacket = ConnectionGetPacket();
            if (mInputPacket == 0)
            {
                return;
            }
            mInputPacketSize = mInputPacket->mBufferUsed / 2;
            mInputPacketOffset = 0;
        }

        do
        {
            if (pOutput == 0)
            {
                break;
            }

            bool bFlag1 = false;
            bool bFlag0 = false;
            bool bLast = false;
            short* const pInput = (short*)mInputPacket->mBuffer;
            unsigned char* const pOut = (unsigned char*)pOutput->mBuffer;
            unsigned int uiSamples = mInputPacketSize - mInputPacketOffset;

            if (mDecimate)
            {
                uiSamples /= 2;
                if (uiSamples != 0)
                {
                    unsigned int uiCount = uiSamples;
                    if (160 - mInputFrameOffset < uiCount)
                    {
                        uiCount = 160 - mInputFrameOffset;
                    }
                    uiCount *= 2;

                    short filtered[320];
                    mFilter->Process16(pInput + mInputPacketOffset, uiCount, filtered);
                    unsigned int i;
                    for (i = 0; i < uiCount; i += 2)
                    {
                        mInputFrame[mInputFrameOffset++] = filtered[i];
                    }
                    mInputPacketOffset += i;
                    uiSamples = 0;
                }
                else
                {
                    mInputPacketOffset = mInputPacketSize;
                }
            }

            if (mInputFrameOffset != 0)
            {
                unsigned int uiCount = 160 - mInputFrameOffset;
                if (uiSamples < uiCount)
                {
                    uiCount = uiSamples;
                }
                if (uiCount != 0)
                {
                    memcpy(&mInputFrame[mInputFrameOffset],
                           pInput + mInputPacketOffset, uiCount * 2);
                    mInputPacketOffset += uiCount;
                    mInputFrameOffset += uiCount;
                }
                if (mInputFrameOffset == 160)
                {
                    mState.Encode(&mInputFrame, (unsigned char (*)[33])(pOut + mOutputPacketOffset));
                    mOutputPacketOffset += 33;
                    mInputFrameOffset = 0;
                }
            }
            else if (uiSamples >= 160)
            {
                mState.Encode((short (*)[160])(pInput + mInputPacketOffset),
                              (unsigned char (*)[33])(pOut + mOutputPacketOffset));
                mInputPacketOffset += 160;
                mOutputPacketOffset += 33;
            }
            else
            {
                memcpy(mInputFrame, pInput + mInputPacketOffset, uiSamples * 2);
                mInputFrameOffset += uiSamples;
                mInputPacketOffset += uiSamples;
            }

            if (mInputPacketOffset == mInputPacketSize)
            {
                bFlag0 = mInputPacket->GetLast();
                bFlag1 = mInputPacket->GetDiscontinuity();
                bLast = bFlag0 | bFlag1;

                if (bLast && mInputFrameOffset != 0 && mOutputPacketOffset < GetPacketSize())
                {
                    memset(&mInputFrame[mInputFrameOffset], 0,
                           (160 - mInputFrameOffset) * 2);
                    mState.Encode(&mInputFrame, (unsigned char (*)[33])(pOut + mOutputPacketOffset));
                    mOutputPacketOffset += 33;
                    mInputFrameOffset = 0;
                    mState.Init();
                    if (mFilter != 0)
                    {
                        mFilter->ResetState();
                    }
                }

                mInputPacket->Unlock();
                mInputPacket = 0;
                mInputPacketOffset = 0;
                mInputPacketSize = 0;
            }

            if (mOutputPacketOffset == GetPacketSize() || bLast)
            {
                const unsigned int uiPos = mPlaybackPosition;
                const unsigned int uiDecoded = (mOutputPacketOffset / 33) * 160;
                PacketComplete(pOutput, mOutputPacketOffset, uiDecoded,
                               bFlag0, bFlag1, uiPos);
                mOutputPacketOffset = 0;
                mPlaybackPosition = uiPos + uiDecoded;
                pOutput = 0;
            }
        } while (mInputPacket != 0);

        pOutput = GetReadPacket();
    }
}

}

