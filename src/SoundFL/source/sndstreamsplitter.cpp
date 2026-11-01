#include "sndstreamsplitter.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074d100
void StreamSplitter::Setup(StreamPacket** const ppQueue,
                           unsigned short* const puRefCounts,
                           unsigned short uNumRefCounts,
                           unsigned short* const puRemaining,
                           unsigned short uNumRemaining)
{
    if (uNumRefCounts == 0 || uNumRemaining == 0)
    {
        Flush();

        mPackets = 0;
        mRemaining = 0;
        mRefCount = 0;
        mNumOutputs = 0;
        mNumPackets = 0;
        mReadPos = 0;
        return;
    }

    mPackets = ppQueue;
    mRefCount = puRefCounts;
    mRemaining = puRemaining;
    mNumOutputs = uNumRemaining;
    mNumPackets = uNumRefCounts;

    for (unsigned int i = 0; i < mNumPackets; i++)
    {
        mPackets[i] = 0;
        mRefCount[i] = 0;
    }

    for (unsigned int i = 0; i < mNumOutputs; i++)
    {
        mRemaining[i] = 0;
    }
}

// FUNCTION: SWBF2 0x0074cf70
void StreamSplitter::Flush()
{
    mFlushing = 1;

    for (unsigned int uiOutput = 0; uiOutput < mNumOutputs; uiOutput++)
    {
        StreamPacket* pPacket;
        do
        {
            pPacket = GetPacket(uiOutput);
            if (pPacket)
            {
                pPacket->Unlock();
            }
        } while (pPacket);
    }

    mReadPos = 0;
    mFlushing = 0;
}

}

namespace Snd
{


// FUNCTION: SWBF2 0x0074cfe0
StreamPacket* StreamSplitter::GetPacket(unsigned int uiOutput)
{
    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamNodeSource* pThis,
                                                    unsigned int uiOutput);

    StreamPacket* pResult = 0;

    if (StreamNode::GetPlayState() != SourceTransport::playState_stopped ||
        mFlushing)
    {
        StreamPacket** const ppWrite = &mPackets[mReadPos];
        if (*ppWrite == 0 && !mFlushing)
        {
            StreamNodeSource* const pInput = StreamNodeSink::mConnection.mInputNode;
            *ppWrite = pInput
                ? pInput->GetPacket(StreamNodeSink::mConnection.mInputNodeOutputIndex)
                : 0;
            if (*ppWrite)
            {
                mRefCount[mReadPos] = 0;
                if (++mReadPos == mNumPackets)
                {
                    mReadPos = 0;
                }
                for (unsigned int i = 0; i < mNumOutputs; i++)
                {
                    ++mRemaining[i];
                }
            }
        }

        if (mRemaining[uiOutput] != 0)
        {
            const unsigned int uiIndex = mReadPos < mRemaining[uiOutput]
                ? mReadPos + (mNumPackets - mRemaining[uiOutput])
                : mReadPos - mRemaining[uiOutput];
            StreamPacket** const ppRead = &mPackets[uiIndex];
            StreamPacket* const pPacket = *ppRead;
            if (pPacket != 0)
            {
                unsigned short* const puRefCount = &mRefCount[uiIndex];
                --mRemaining[uiOutput];
                pPacket->Lock();
                if (++*puRefCount == mNumOutputs)
                {
                    pPacket->Unlock();
                    *ppRead = 0;
                    *puRefCount = 0;
                }
                pResult = pPacket;
            }
        }
    }
    return pResult;
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x0074cee0
StreamSplitter::StreamSplitter()
    : mPackets(0),
      mRemaining(0),
      mRefCount(0),
      mNumOutputs(0),
      mNumPackets(0),
      mReadPos(0),
      mFlushing(0)
{

}
}

namespace Snd
{

// FUNCTION: SWBF2 0x0074cf30
StreamSplitter::~StreamSplitter()
{

}
}
