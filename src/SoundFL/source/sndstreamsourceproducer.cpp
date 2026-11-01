
#include "sndstreamsourceproducer.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073c740
StreamSourceNProducer::StreamSourceNProducer()
    : mProducers(0), mNumProducers(0)
{
}

// FUNCTION: SWBF2 0x0073c760
StreamSourceProducer::StreamSourceProducer()
{
    mPackets = 0;
    mNumPackets = 0;
    mPacketSize = 0;
    mReadPacketIndex = 0;
    mLockPacketIndex = 0;
}



// FUNCTION: SWBF2 0x0073c7a0
StreamSourceNProducer::~StreamSourceNProducer()
{
}

// FUNCTION: SWBF2 0x0073c7b0
StreamSourceProducer::~StreamSourceProducer()
{

    Flush();
}

// FUNCTION: SWBF2 0x0073cb10
void Snd::StreamSourceProducer::SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets, unsigned int uiNumPackets, const bool bCheckPacketBuffers)
{
    (void)bCheckPacketBuffers;

    if (uiPacketSize == 0 || uiNumPackets == 0 || pPackets == 0)
    {
        mPackets = 0;
        mNumPackets = 0;
        mPacketSize = 0;
        return;
    }

    mPackets = pPackets;
    mNumPackets = uiNumPackets;
    mPacketSize = uiPacketSize;

    for (unsigned int i = 0; i < mNumPackets; ++i)
    {
        mPackets[i].SetUsed(0);
    }
}

// FUNCTION: SWBF2 0x0073cb80
void StreamSourceProducer::SetPackets(unsigned int uiPacketSize,
                                      StreamPacket* pPackets,
                                      unsigned int uiNumPackets)
{
    SetPackets(uiPacketSize, pPackets, uiNumPackets, true);
}

// FUNCTION: SWBF2 0x0073cba0
void StreamSourceNProducer::SetProducers(StreamSourceProducer* pProducers,
                                         unsigned int uiNumProducers)
{
    if (pProducers == 0 || uiNumProducers == 0)
    {
        mProducers = 0;
        mNumProducers = 0;
        return;
    }

    mProducers = pProducers;
    mNumProducers = uiNumProducers;
}

// FUNCTION: SWBF2 0x0073c8e0
void StreamSourceProducer::Flush()
{
    StreamPacket* pPacket;
    do
    {
        pPacket = GetPacket(0);
        if (pPacket != 0)
        {
            pPacket->Unlock();
        }
    } while (pPacket != 0);
}

// FUNCTION: SWBF2 0x0073c9a0
StreamPacket* StreamSourceProducer::GetPacket(unsigned int uiOutput)
{
    (void)uiOutput;
    if (mPackets)
    {
        StreamPacket* const pPacket = &mPackets[mLockPacketIndex];
        if (!pPacket->IsLocked() && pPacket->GetUsed())
        {
            pPacket->Lock();
            if (++mLockPacketIndex == mNumPackets)
            {
                mLockPacketIndex = 0;
            }
            return pPacket;
        }
    }
    return 0;
}




// FUNCTION: SWBF2 0x0073c930
unsigned int StreamSourceProducer::GetNumQueuedPackets() const
{
    unsigned int uiIndex = mReadPacketIndex;
    unsigned int uiQueued = 0;
    const unsigned int uiNumPackets = mNumPackets;

    if (uiNumPackets != 0)
    {
        const StreamPacket* const pPackets = mPackets;
        for (unsigned int i = uiNumPackets; i != 0; --i)
        {
            const StreamPacket& packet = pPackets[uiIndex++];
            if (packet.mRefCount > 0 || packet.mBufferUsed != 0)
            {
                ++uiQueued;
            }
            if (uiIndex == uiNumPackets)
            {
                uiIndex = 0;
            }
        }
    }
    return uiQueued;
}

// FUNCTION: SWBF2 0x0073c970
StreamPacket* StreamNodeSource::GetPacket(unsigned int uiOutput)
{
    (void)uiOutput;
    return 0;
}

// FUNCTION: SWBF2 0x0073c980
StreamPacket* StreamSourceNProducer::GetPacket(unsigned int uiOutput)
{
    StreamSourceProducer* const pOut = mProducers + uiOutput;
    return pOut->GetPacket(0);
}

// FUNCTION: SWBF2 0x0073ca10
StreamPacket* StreamSourceProducer::GetReadPacket()
{
    StreamPacket* const pPacket = &mPackets[mReadPacketIndex];
    if (pPacket->GetUsed() || pPacket->IsLocked())
    {
        return 0;
    }
    return pPacket;
}

// FUNCTION: SWBF2 0x0073ca90
void StreamSourceProducer::PacketComplete(StreamPacket* const pPacket,
                                          unsigned int uiBufferUsed,
                                          unsigned int uiUnused,
                                          bool bFlagBit0, bool bFlagBit1,
                                          unsigned int uiPos)
{
    (void)uiUnused;

    pPacket->SetUsed(uiBufferUsed);
    pPacket->SetPosition(uiPos);

    if (bFlagBit0)
    {
        pPacket->mFlags |= 1;
    }
    else
    {
        pPacket->mFlags &= 0xFFFE;
    }

    if (bFlagBit1)
    {
        pPacket->mFlags |= 2;
    }
    else
    {
        pPacket->mFlags &= 0xFFFD;
    }

    ++mReadPacketIndex;
    if (mReadPacketIndex == mNumPackets)
    {
        mReadPacketIndex = 0;
    }
}

// FUNCTION: SWBF2 0x0073caf0
void StreamPacket::SetDiscontinuity(bool bDiscontinuity)
{
    if (bDiscontinuity)
    {
        mFlags |= 2;
    }
    else
    {
        mFlags &= 0xFFFD;
    }
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x0073c9e0
StreamPacket* StreamSourceNProducer::GetReadPacket(const unsigned int uiOutput) const
{
    return GetProducer(uiOutput)->GetReadPacket();
}
}

namespace Snd
{

// FUNCTION: SWBF2 0x0073ca30
void StreamSourceNProducer::PacketComplete(StreamPacket* const pPacket,
                                   unsigned int uiBufferUsed, unsigned int uiUnused,
                                   bool bFlagBit0, bool bFlagBit1, unsigned int uiPos,
                                   unsigned int uiOutput)
{
    GetProducer(uiOutput)->PacketComplete(pPacket, uiBufferUsed, uiUnused,
                                         bFlagBit0, bFlagBit1, uiPos);
}
}
