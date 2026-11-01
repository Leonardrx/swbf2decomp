#include <assert.h>
#include <string.h>

#include "sndstreamgsmdecoder.h"
#include "sndstreamsourceproducer.h"


namespace Snd
{

// FUNCTION: SWBF2 0x0073d040
StreamGSMDecoder::StreamGSMDecoder()
    : mInputPacket(0),
      mInputPacketOffset(0),
      mInputPacketSize(0),
      mOutputPacketOffset(0),
      mFrameOffset(160)
{
    mDecimate = false;
    mState.Init();
}




// FUNCTION: SWBF2 0x0073d170
void StreamGSMDecoder::Flush()
{
    if (mInputPacket != 0)
    {
        mInputPacket->Unlock();
        mInputPacket = 0;
        mInputPacketOffset = 0;
        mInputPacketSize = 0;
    }

    mOutputPacketOffset = 0;
    mFrameOffset = 160;
    mState.Init();
    StreamSourceProducer::Flush();
}

// FUNCTION: SWBF2 0x0073d300
void StreamGSMDecoder::Update()
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
            mInputPacketSize = mInputPacket->mBufferUsed;
            mInputPacketOffset = 0;
        }

        do
        {
            if (pOutput == 0)
            {
                break;
            }

            bool bEnd = false;
            short* const pSamples = (short*)pOutput->mBuffer;
            unsigned char* const pInput = (unsigned char*)mInputPacket->mBuffer;
            bool bDiscontinuity = false;
            bool bLast = false;

            if (mDecimate)
            {
                if (mFrameOffset == 160)
                {
                    mState.Decode((unsigned char (*)[33])(pInput + mInputPacketOffset),
                                  mFrame);
                    mInputPacketOffset += 33;
                    mFrameOffset = 0;
                }

                unsigned int i;
                for (i = 0; i < 79; ++i)
                {
                    short* const pOut = pSamples + (mOutputPacketOffset + i * 2);
                    const unsigned int uiFrame = mFrameOffset + i;
                    pOut[0] = (*mFrame)[uiFrame];
                    pOut[1] = (short)(((*mFrame)[uiFrame] + (*mFrame)[uiFrame + 1]) >> 1);
                }

                short* const pOut = pSamples + (mOutputPacketOffset + i * 2);
                const unsigned int uiFrame = mFrameOffset + i;
                pOut[0] = (*mFrame)[uiFrame];
                pOut[1] = (*mFrame)[uiFrame];

                mOutputPacketOffset += 160;
                mFrameOffset += 80;
            }
            else
            {
                mState.Decode((unsigned char (*)[33])(pInput + mInputPacketOffset),
                              (short (*)[160])(pSamples + mOutputPacketOffset));
                mInputPacketOffset += 33;
                mOutputPacketOffset += 160;
            }

            if (mInputPacketOffset == mInputPacket->mBufferUsed &&
                (!mDecimate || mFrameOffset == 160))
            {
                bLast = mInputPacket->GetLast();
                bDiscontinuity = mInputPacket->GetDiscontinuity();
                bEnd = bLast | bDiscontinuity;
                if (bEnd)
                {
                    mState.Init();
                }

                mInputPacket->Unlock();
                mInputPacket = 0;
                mInputPacketOffset = 0;
                mInputPacketSize = 0;
            }

            const unsigned int uiSamples = mOutputPacketOffset;
            if (uiSamples == GetPacketSize() >> 1 || bEnd)
            {
                const unsigned int uiPos = mPlaybackPosition;
                PacketComplete(pOutput, uiSamples * 2, uiSamples, bLast,
                               bDiscontinuity, uiPos);
                mOutputPacketOffset = 0;
                pOutput = 0;
                mPlaybackPosition = uiPos + uiSamples;
            }
        } while (mInputPacket != 0);

        pOutput = GetReadPacket();
    }
}

// FUNCTION: SWBF2 0x0073d2f0
void Snd::StreamGSMDecoder::SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets, unsigned int uiNumPackets)
{
    void* pDecoder = (void*)this;
    ((StreamSourceProducer*)pDecoder)->SetPackets(uiPacketSize, pPackets,
                                                  uiNumPackets);
}

}
