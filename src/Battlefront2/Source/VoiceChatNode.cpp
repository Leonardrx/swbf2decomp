#include <string.h>

#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "VoiceChatUtil.h"
#include "VOHelper.h"
#include "NetPacket.h"
#include "PebbleFL/Common/PblValues.h"
#include "GameVoiceChatUtil.h"
#include "GameVoiceChatDecoder.h"
#include <float.h>

#include "GameVoiceChat.h"

#include "GameVoiceChatNode.h"

// GLOBAL: SWBF2 0x01eafd3c
VoiceChat::Transport* VoiceChat::Node::sTransport;
// GLOBAL: SWBF2 0x01eafd40
unsigned short VoiceChat::Node::sSeqNumVariance;
// GLOBAL: SWBF2 0x01eafd42
unsigned char  VoiceChat::Node::sSkipDropped;
// GLOBAL: SWBF2 0x01eafd44
float          VoiceChat::Node::sBufferSizeMs;
// GLOBAL: SWBF2 0x01eafd48
unsigned short VoiceChat::Node::sRepeatMax;
// GLOBAL: SWBF2 0x01eafd4c
float          VoiceChat::Node::sReceiveBufferSize;
// GLOBAL: SWBF2 0x01eafd50
VoiceChat::Node::SendMetrics* VoiceChat::Node::sSendMetrics;
// GLOBAL: SWBF2 0x01eafd54
VoiceChat::Node::ReceiveMetrics* VoiceChat::Node::sReceiveMetrics;
// GLOBAL: SWBF2 0x01eafd58
float          VoiceChat::Node::sTime;

// FUNCTION: SWBF2 0x00672770
VoiceChat::Node::Header::Header()
{
    mBitmap = 0;
    mSize = 0;
    mBits = 0;
}

// SYNTHETIC: SWBF2 0x00406f80
// `dynamic initializer for 'VoiceChat::Node::sReadHeader''

// GLOBAL: SWBF2 0x01f9a26c
// VoiceChat::Node::sReadHeader
VoiceChat::Node::Header VoiceChat::Node::sReadHeader;

// SYNTHETIC: SWBF2 0x00406f90
// `dynamic initializer for 'VoiceChat::Node::sWriteHeader''

// GLOBAL: SWBF2 0x01f9a730
// VoiceChat::Node::sWriteHeader
VoiceChat::Node::Header VoiceChat::Node::sWriteHeader;

namespace VoiceChat
{
    void* Alloc(unsigned int uiSize);
}

namespace VoiceChat
{
    void Free(void* pBlock);
}

// GLOBAL: SWBF2 0x007ebee8
void* VoiceChat::Node::sList[4] = { VoiceChat::Node::sList, VoiceChat::Node::sList, VoiceChat::Node::sList, 0 };
// GLOBAL: SWBF2 0x007ebef8

int sRegisteredNodeCount;

// FUNCTION: SWBF2 0x00674580
void VoiceChat::Node::SetSkipOutOfSequence(bool skip, float fRate)
{
    const float fPeriod = VoiceChat::Node::sBufferSizeMs / 1000.0f;

    VoiceChat::Node::sSkipDropped = skip;

    const int iCount = (int)(fRate / fPeriod);
    VoiceChat::Node::sRepeatMax = (iCount > 0xFFFF) ? (unsigned short)0xFFFF
                                     : (unsigned short)iCount;
}

// FUNCTION: SWBF2 0x00673d40
bool VoiceChat::Node::OpenStatic(Transport* transport, float fSeconds, float fA, float fB)
{

    VoiceChat::Node::sTransport = transport;

    const unsigned char uiNodes = transport->GetNumNodes();
    const unsigned int uiNodesLog2 = transport->GetNumNodesLog2();

    const unsigned int uiBitmapBytes = ((uiNodesLog2 + 8 + uiNodes) & ~7u) >> 3;
    VoiceChat::Node::sReadHeader.mSize = uiBitmapBytes;
    VoiceChat::Node::sWriteHeader.mSize = uiBitmapBytes;

    const unsigned int uiSize = uiNodes * 68 + ((uiBitmapBytes + 3) >> 2) * 8;

    char* p = (char*)VoiceChat::Alloc(uiSize);
    VoiceChat::Node::sReadHeader.mBitmap = (unsigned char*)p;
    VoiceChat::Node::sReadHeader.mBits = 0;
    VoiceChat::Node::sWriteHeader.mBits = 0;

    if (!p)
    {
        VoiceChat::Node::sTransport = 0;
        VoiceChat::Node::sReadHeader.mSize = 0;
        return false;
    }

    char* pSecond = p + ((VoiceChat::Node::sReadHeader.mSize + 3) >> 2) * 4;
    VoiceChat::Node::sWriteHeader.mBitmap = (unsigned char*)pSecond;

    char* pPerNode = pSecond + ((VoiceChat::Node::sWriteHeader.mSize + 3) >> 2) * 4;
    VoiceChat::Node::sReceiveMetrics = (ReceiveMetrics*)pPerNode;
    VoiceChat::Node::sSendMetrics = (SendMetrics*)(pPerNode + uiNodes * sizeof(ReceiveMetrics));

    memset(VoiceChat::Node::sReadHeader.mBitmap, 0, VoiceChat::Node::sWriteHeader.mSize + uiNodes * 68 + VoiceChat::Node::sReadHeader.mSize);

    VoiceChat::Node::sTime = 0;
    VoiceChat::Node::sBufferSizeMs = (1.0f / fSeconds) * 2.0f * 1000.0f;
    VoiceChat::Node::sReceiveBufferSize = fA / 1000.0f;

    for (unsigned char i = 0; i < uiNodes; ++i)
    {
        ((ReceiveMetrics*)pPerNode)[i].receiveLatency = fB / 1000.0f;
    }

    VoiceChat::Node::sSeqNumVariance = (unsigned short)(int)fSeconds;
    if (VoiceChat::Node::sSeqNumVariance > 0x3FF)
    {
        VoiceChat::Node::sSeqNumVariance = 0x3FF;
    }

    VoiceChat::Node::SetSkipOutOfSequence(false, 0.2f);
    return true;
}

// FUNCTION: SWBF2 0x00672b00
VoiceChat::Node::Node(Encoder* const encoder, Decoder* const decoder)
    : mEncoder(encoder),
      mDecoder(decoder),
      mLastCaptureTime(0.0f),
      mLastSendTime(0.0f),
      mLastReceiveTime(0.0f),
      mBufferUpdateTime(0.0f),
      mReceiveStreams(0),
      mAllocatedInputs(0),
      mMaxReceiveSize(0)
{
    mMaxReceivePackets = 0;
    mSendSeq = 0;
    mIsOpen = false;
    mNodeID = 0xFF;
    mNumDecoderInputs = 0;
}

// FUNCTION: SWBF2 0x00673080
unsigned int VoiceChat::Node::UpdateReceiveStreamLatency(unsigned char ucIndex)
{
    const unsigned int uiIndex = ucIndex;

    ReceiveStream& stream = mReceiveStreams[uiIndex];
    const ReceiveMetrics& slot = ((const ReceiveMetrics*)sReceiveMetrics)[uiIndex];

    int iDenom = 1;
    if (slot.lastPacketSize)
    {
        iDenom = slot.lastPacketSize;
    }

    const unsigned int uiBytes = mEncoder->GetMaxOutputPacketSizeBytes();

    const float fShare = (float)(int)uiBytes / (float)iDenom;
    const float fRate = slot.receiveLatency / mEncoderPacketSizeSec;

    unsigned int uiCount = (unsigned int)(int)(fShare * fRate);
    if (uiCount == 0)
    {
        uiCount = 1;
    }
    else
    {
        const unsigned int uiPackets = mMaxReceivePackets;
        if (uiCount > uiPackets)
        {
            uiCount = uiPackets;
        }
    }

    stream.numPackets = (unsigned short)uiCount;

    return (unsigned short)uiCount;
}

// FUNCTION: SWBF2 0x00672b70
bool VoiceChat::Node::Open(unsigned int captureDeviceIndex, unsigned int renderDeviceIndex, unsigned char numDecoderInputs)
{
    if (mEncoder->Open(captureDeviceIndex, sBufferSizeMs, 2))
    {
        if (mDecoder->Open(renderDeviceIndex, mEncoder->GetMaxOutputPacketSizeMs(), 2, numDecoderInputs))
        {
            const float fSeconds = mEncoder->GetMaxOutputPacketSizeMs() / 1000.0f;
            const unsigned short uiPackets =
                (unsigned short)(int)(VoiceChat::Node::sReceiveBufferSize / fSeconds);

            const unsigned int uiBytes = mEncoder->GetMaxOutputPacketSizeBytes();
            const unsigned char uiNodes = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();
            const unsigned int uiAlignedBytes = (uiBytes + 3u) & ~3u;

            char* const pBlock = (char*)VoiceChat::Alloc(
                ((uiAlignedBytes + 0x10u) * uiPackets + 0x1Cu) * uiNodes + 0x0Cu);
            mReceiveStreams = (ReceiveStream*)pBlock;

            if (pBlock)
            {
                mIsOpen = true;
                mEncoderPacketSizeSec = fSeconds;
                mMaxReceiveSize = uiBytes;
                mMaxReceivePackets = uiPackets;
                mLastCaptureTime = 0;
                mLastSendTime = 0;
                mLastReceiveTime = 0;
                mBufferUpdateTime = 0;
                mSendSeq = 0;
                mNumDecoderInputs = numDecoderInputs;

                char* pEntryBuf = pBlock + uiNodes * 0x18;
                char* pData = (char*)((((unsigned long)(pEntryBuf) +
                                        uiPackets * uiNodes * 0x10u) + 3u) & ~3u);

                for (unsigned char i = 0; i < uiNodes; ++i)
                {
                    ReceiveStream& entry = mReceiveStreams[i];
                    unsigned char* const pEntry = (unsigned char*)&entry;

                    pEntry[0x17] &= 0xFC;
                    *(unsigned int*)(pEntry + 0x12) = 0;
                    entry.packet = (AudioPacket*)pEntryBuf;
                    *(unsigned int*)(pEntry + 0x04) = 0;
                    *(unsigned int*)(pEntry + 0x08) = 0;
                    *(unsigned int*)(pEntry + 0x0C) = 0;
                    pEntry[0x16] = (char)0xFF;

                    UpdateReceiveStreamLatency(i);

                    for (unsigned int p = uiPackets; p != 0; --p)
                    {
                        pEntryBuf[0x0C] &= 0xFE;
                        ((AudioPacket*)pEntryBuf)->mData = pData;
                        pData += uiAlignedBytes;
                        *(unsigned int*)(pEntryBuf + 0x04) = 0;
                        *(unsigned int*)(pEntryBuf + 0x08) = 0;
                        pEntryBuf += 0x10;
                    }
                }

                mAllocatedInputs = (unsigned char*)pData;
                for (unsigned char b = 0; b < mNumDecoderInputs; ++b)
                {
                    mAllocatedInputs[b] = (unsigned char)0xFF;
                }

                return true;
            }
            mDecoder->Close();
        }
        mEncoder->Close();
    }
    return false;
}

// FUNCTION: SWBF2 0x00672d50
bool VoiceChat::Node::IsOpen() const
{
    return mIsOpen;
}

// FUNCTION: SWBF2 0x00672d60
void VoiceChat::Node::Close()
{
    Close(false, false);
}

// FUNCTION: SWBF2 0x00672d70
void VoiceChat::Node::Close(bool, bool)
{
    mEncoder->Stop();
    mEncoder->Close();
    mDecoder->Stop();
    mDecoder->Close();

    SetID(0xFF);
    VoiceChat::Free(mReceiveStreams);

    mIsOpen = false;

    mEncoderPacketSizeSec = 0;
    mReceiveStreams = 0;
    mMaxReceiveSize = 0;
    mMaxReceivePackets = 0;
    mSendSeq = 0;
    mNumDecoderInputs = 0;
    mAllocatedInputs = 0;
    mLastCaptureTime = 0;
    mLastSendTime = 0;
    mLastReceiveTime = 0;
    mBufferUpdateTime = 0;
}

// FUNCTION: SWBF2 0x00672de0
void VoiceChat::Node::SetID(unsigned char uChannel)
{
    if (mNodeID == uChannel)
    {
        return;
    }

    if (uChannel == 0xFF || mNodeID != 0xFF)
    {
        ((VoiceChat::Transport*)VoiceChat::Node::sTransport)->SetLocalNode(mNodeID, false, false, false);
        --sRegisteredNodeCount;

        _pList = 0;
        _pObject = 0;
        _pNext->_pPrev = _pPrev;
        _pPrev->_pNext = _pNext;

        const unsigned int uNodes = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();

        VoiceChat::AudioPacket packet;
        packet.mDiscontinuity = true;
        packet.mData = 0;
        packet.mSize = 0;
        packet.mAppData = 0;

        for (unsigned int i = 0; i < uNodes; ++i)
        {
            Flush(mReceiveStreams[i], packet);
        }

        mEncoder->Stop();
        mDecoder->Stop();
    }

    if (uChannel != 0xFF)
    {
        mEncoder->Play();
        mDecoder->Play();

        ((VoiceChat::Transport*)VoiceChat::Node::sTransport)->SetLocalNode(uChannel, true, mDecoder->IsEnabled(), mEncoder->IsEnabled() != 0);
        ++sRegisteredNodeCount;

        _pObject = this;
        _pList = (PblList<Node>*)VoiceChat::Node::sList;
        _pNext = (PblList<Node>::Node*)VoiceChat::Node::sList;
        _pPrev = (PblList<Node>::Node*)VoiceChat::Node::sList[2];
        VoiceChat::Node::sList[2] = this;
        _pPrev->_pNext = this;
    }

    mNodeID = uChannel;
}

// FUNCTION: SWBF2 0x00673ee0
void VoiceChat::Node::CloseStatic()
{

    VoiceChat::Free(VoiceChat::Node::sReadHeader.mBitmap);

    VoiceChat::Node::sTransport = 0;
    VoiceChat::Node::sReadHeader.mBitmap = 0;
    VoiceChat::Node::sReadHeader.mSize = 0;
    VoiceChat::Node::sReadHeader.mBits = 0;
    VoiceChat::Node::sWriteHeader.mBitmap = 0;
    VoiceChat::Node::sWriteHeader.mSize = 0;
    VoiceChat::Node::sWriteHeader.mBits = 0;
    VoiceChat::Node::sReceiveMetrics = 0;
    VoiceChat::Node::sBufferSizeMs = 0.0f;
    VoiceChat::Node::sTime = 0;

}


// FUNCTION: SWBF2 0x00672f40
float VoiceChat::Node::GetLastSendTime() const
{
    return mLastSendTime;
}

// FUNCTION: SWBF2 0x00672f50
bool VoiceChat::Node::IsReceiving() const
{
    return (GetTime() - mLastReceiveTime) * 1000.0f < GetBufferSizeMs() * 2.0f;
}

// FUNCTION: SWBF2 0x00672f80
bool VoiceChat::Node::IsCapturing() const
{
    return (GetTime() - mLastCaptureTime) * 1000.0f < GetBufferSizeMs() * 2.0f;
}

// FUNCTION: SWBF2 0x00674340
float VoiceChat::Node::GetTime()
{
    return VoiceChat::Node::sTime;
}

// FUNCTION: SWBF2 0x00674350
float VoiceChat::Node::GetBufferSizeMs()
{
    return VoiceChat::Node::sBufferSizeMs;
}

// FUNCTION: SWBF2 0x00674360
VoiceChat::Node* VoiceChat::Node::FindByID(unsigned char uNodeID)
{
    PblList<VoiceChat::Node>::Node* pNode =
        (PblList<VoiceChat::Node>::Node*)VoiceChat::Node::sList[1];
    while (pNode->_pObject)
    {
        if (pNode->_pObject->GetID() == uNodeID)
        {
            return pNode->_pObject;
        }
        pNode = pNode->_pNext;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00672fd0
void VoiceChat::Node::Send()
{
    if (mNodeID == 0xFF)
    {
        return;
    }

    while (AudioPacket* packet = mEncoder->GetData())
    {
        if (packet->mSize || packet->mDiscontinuity)
        {
            PblValues::Record(packet->mSize, PVT_FULL);
            mLastCaptureTime = GetTime();
            SendAudioPacket(packet);
        }
        mEncoder->FreeAudioPacket(packet);
    }

    (VoiceChat::Node::sTransport)->SetSending(
        GetID(), (GetTime() - GetLastSendTime()) * 1000.0f < GetBufferSizeMs() * 2.0f);
}

// FUNCTION: SWBF2 0x00673a50
void VoiceChat::Node::Flush(ReceiveStream& stream, AudioPacket& packet)
{
    while (stream.packet[stream.readIndex].mAppData)
    {
        AudioPacket& queued = stream.packet[stream.readIndex++];
        queued.mAppData = 0;
        queued.mSize = 0;
        if (stream.readIndex == mMaxReceivePackets)
        {
            stream.readIndex = 0;
        }
    }
    DecoderReceiveAudioPacket((unsigned char)(((unsigned int)&stream - (unsigned int)mReceiveStreams) / sizeof(ReceiveStream)), &packet);
}

// FUNCTION: SWBF2 0x00673c10
bool VoiceChat::Node::DecoderReceiveAudioPacket(unsigned char stream,
                                                const AudioPacket* const packet)
{
    bool bReceived = false;
    ReceiveStream& s = mReceiveStreams[stream];

    if (packet->mSize)
    {
        if (s.decoderInput == 0xFF)
        {
            DecoderInputAlloc(stream);
        }
        if (s.decoderInput != 0xFF)
        {
            bReceived = mDecoder->ReceivePacket(s.decoderInput, packet);
        }
    }

    if (packet->mDiscontinuity && s.decoderInput != 0xFF)
    {
        if (!bReceived && !packet->mSize)
        {
            bReceived = mDecoder->ReceivePacket(s.decoderInput, packet);
        }
        if (bReceived)
        {
            DecoderInputFree(s.decoderInput, false);
            return true;
        }
    }
    return bReceived;
}

// FUNCTION: SWBF2 0x006731a0
void VoiceChat::Node::Decode(float fDeltaTime)
{
    const unsigned char numStreams = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();
    unsigned char i;
    bool bStarving = false;

    for (i = 0; i < numStreams; ++i)
    {
        ReceiveStream& stream = mReceiveStreams[i];
        if (stream.activated)
        {
            DecodeQueue(i, false);
            if (stream.bufferedData == 0.0f)
            {
                bStarving |= mReceiveStreams[i].decoderInput != 0xFF && VoiceChat::Node::sTransport->GetReceivePossible(mNodeID, i) &&
                             CalculateNumPackets(stream) == 0;
            }
        }
        if (stream.bufferedData != 0.0f)
        {
            stream.bufferedData -= fDeltaTime;
            if (stream.bufferedData < 0.0f)
            {
                stream.bufferedData = 0.0f;
            }
        }
    }

    const bool bWasReceiving = VoiceChat::Node::sTransport->IsConnectedReceive(mNodeID);
    const bool bDecoderEnabled = mDecoder->IsEnabled();
    VoiceChat::Node::sTransport->SetLocalNode(mNodeID, true, bDecoderEnabled, ((VoiceChat::Encoder*)mEncoder)->IsEnabled() != 0);
    if (bDecoderEnabled != bWasReceiving)
    {
        AudioPacket packet;
        packet.mDiscontinuity = true;
        packet.mData = 0;
        packet.mSize = 0;
        packet.mAppData = 0;
        for (unsigned char i = 0; i < numStreams; ++i)
        {
            Flush(mReceiveStreams[i], packet);
        }
    }

    if (bStarving || mDecoder->Update())
    {
        AudioPacket packet;
        packet.mDiscontinuity = true;
        packet.mData = 0;
        packet.mSize = 0;
        packet.mAppData = 0;
        for (i = 0; i < numStreams; ++i)
        {
            ReceiveStream& stream = mReceiveStreams[i];
            if (DecoderIsStarving(i, bStarving) && stream.activated)
            {
                ReceiveMetrics& metrics = VoiceChat::Node::sReceiveMetrics[i];
                const float fTime = VoiceChat::Node::sTime;
                const float fSinceStarve = fTime - metrics.lastStarveTime;
                ++metrics.starvedCount;
                metrics.lastStarveTime = fTime;
                metrics.starvedFrequency = (fSinceStarve + metrics.starvedFrequency) * 0.5f;
                metrics.receiveLatency = VoiceChat::Node::sReceiveBufferSize;
                UpdateReceiveStreamLatency(i);
                if (!stream.activated && CalculateNumPackets(stream) >= stream.numPackets)
                {
                    stream.activated = true;
                    stream.activationTime = GetTime();
                }
                mBufferUpdateTime = 1.0f;
                DecoderReceiveAudioPacket(i, &packet);
            }
        }
        if (bStarving)
        {
            mDecoder->Update();
        }
    }

    if (mBufferUpdateTime <= 0.0f)
    {
        for (unsigned char i = 0; i < numStreams; ++i)
        {
            if (VoiceChat::Node::sReceiveMetrics[i].activated)
            {
                UpdateReceiveStreamLatency(i);
            }
        }
        mBufferUpdateTime = 1.0f;
    }
    mBufferUpdateTime -= fDeltaTime;
}

// FUNCTION: SWBF2 0x00672fb0
const VoiceChat::Node::ReceiveStream& VoiceChat::Node::GetReceiveStream(unsigned char stream) const
{
    return mReceiveStreams[stream];
}

// FUNCTION: SWBF2 0x00673cf0
unsigned int VoiceChat::Node::CalculateNumPackets(const ReceiveStream& stream) const
{
    if (stream.writeIndex == stream.readIndex)
    {
        return stream.packet[stream.writeIndex].mAppData ? mMaxReceivePackets : 0;
    }
    if (stream.writeIndex > stream.readIndex)
    {
        return stream.writeIndex - stream.readIndex;
    }
    return stream.writeIndex + mMaxReceivePackets - stream.readIndex;
}

// FUNCTION: SWBF2 0x00674390
const VoiceChat::Node::SendMetrics& VoiceChat::Node::GetSendMetrics(unsigned char node)
{
    return VoiceChat::Node::sSendMetrics[node];
}

// FUNCTION: SWBF2 0x006743a0
const VoiceChat::Node::ReceiveMetrics& VoiceChat::Node::GetReceiveMetrics(unsigned char node)
{
    return VoiceChat::Node::sReceiveMetrics[node];
}

// FUNCTION: SWBF2 0x00674320
float VoiceChat::Node::GetLastReceiveTime(unsigned char ucNode)
{
    return VoiceChat::Node::sReceiveMetrics[ucNode].lastReceiveTime;
}

namespace VoiceChat
{

// FUNCTION: SWBF2 0x00672790
void Node::Header::SendSetup(const Transport& transport, unsigned char node, unsigned char receiver,
                             bool send, unsigned short sequence)
{
    Reset();
    WriteBits(receiver == 0xff, 1);
    WriteBits(send ? 1 : 0, 1);
    WriteBits(sequence, 10);
    WriteBits(node, transport.GetNumNodesLog2());
    if (receiver == 0xff)
    {
        WriteBits(0, transport.GetNumNodes());
    }
    else
    {
        WriteBits(receiver, transport.GetNumNodesLog2());
    }
}

// FUNCTION: SWBF2 0x00672830
void Node::Header::SetReceiveNode(const Transport& transport, unsigned char node, bool receive)
{
    const unsigned int bit = mBits + node - transport.GetNumNodes();
    VoiceChat::SetBit(mBitmap, bit, receive);
}

// FUNCTION: SWBF2 0x00672870
bool Node::Header::GetReceiveNode(const Transport& transport, unsigned char node)
{
    switch (GetFormat())
    {
    case format_direct:
        return node == ReadBits(12 + transport.GetNumNodesLog2(), transport.GetNumNodesLog2());
    case format_relay:
        return VoiceChat::GetBit(mBitmap, 12 + transport.GetNumNodesLog2() + node);
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x006728f0
unsigned int Node::Header::Write(NetPacketAudio* const pPacket) const
{
    const unsigned int nBytes = (mBits + 7) >> 3;
    pPacket->Write(mBitmap, nBytes);
    return nBytes;
}

// FUNCTION: SWBF2 0x00672920
bool Node::Header::Read(const Transport& transport, NetPacketAudio* const pPacket)
{
    Reset();
    if (pPacket->Read(mBitmap, 1) != 1)
    {
        return false;
    }
    const Format format = GetFormat();
    const unsigned int nBits = CalculateSize(transport, format);
    const unsigned int nBytes = ((nBits + 7) >> 3) - 1;
    if (nBytes != pPacket->Read((char*)mBitmap + 1, nBytes))
    {
        return false;
    }
    mBits = nBits;
    return true;
}

// FUNCTION: SWBF2 0x006729a0
unsigned char Node::Header::GetSendNodeID(const Transport& transport) const
{
    return (unsigned char)ReadBits(12, transport.GetNumNodesLog2());
}

// FUNCTION: SWBF2 0x006729c0
unsigned char Node::Header::GetNumReceiveNodes(const Transport& transport) const
{
    switch (GetFormat())
    {
    case format_direct:
        return 1;
    case format_relay:
    {
        unsigned char numNodes = 0;
        const unsigned int headerOffset = transport.GetNumNodesLog2() + 1;
        for (unsigned char i = 0; i < transport.GetNumNodes(); ++i)
        {
            if (ReadBits(headerOffset + i, 1))
            {
                ++numNodes;
            }
        }
        return numNodes;
    }
    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x00672a20
bool Node::Header::GetDiscontinuity() const
{
    return ReadBits(1, 1) != 0;
}

// FUNCTION: SWBF2 0x00672a30
unsigned short Node::Header::GetSeqNum() const
{
    return (unsigned short)ReadBits(2, 10);
}

// FUNCTION: SWBF2 0x00672a40
void Node::Header::Reset()
{
    memset(mBitmap, 0, mSize);
    mBits = 0;
}

// FUNCTION: SWBF2 0x00672a60
void Node::Header::WriteBits(unsigned int value, unsigned char count)
{
    VoiceChat::SetBits(mBitmap, mBits, value, count);
    mBits += count;
}

// FUNCTION: SWBF2 0x00672a90
unsigned int Node::Header::ReadBits(unsigned int bit, unsigned char count) const
{
    return VoiceChat::GetBits(mBitmap, bit, count);
}

// FUNCTION: SWBF2 0x00672ab0
Node::Header::Format Node::Header::GetFormat() const
{
    return (Format)(ReadBits(0, 1) + 1);
}

// FUNCTION: SWBF2 0x00672ac0
unsigned int Node::Header::CalculateSize(const Transport& transport, Format format) const
{
    const unsigned int fixedSize = 12 + transport.GetNumNodesLog2();
    switch (format)
    {
    case format_direct:
        return fixedSize + transport.GetNumNodesLog2();
    case format_relay:
        return fixedSize + transport.GetNumNodes();
    default:
        return 0;
    }
}

}

// FUNCTION: SWBF2 0x00673480
void VoiceChat::Node::SendAudioPacket(const AudioPacket* const packet)
{
    unsigned char node;
    Transport* transport = (Transport*)VoiceChat::Node::sTransport;
    const Transport::Method& method = transport->GetMethod();
    const unsigned char numNodes = transport->GetNumNodes();
    switch (method.GetType())
    {
    case Transport::Method::type_direct:
        for (node = 0; node < numNodes; ++node)
        {
            if (VoiceChat::Node::sTransport->GetSendPossible(mNodeID, node))
            {
                VoiceChat::Node::sWriteHeader.SendSetup((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), mNodeID, node, packet->mDiscontinuity, mSendSeq);
                SendAudioPacket(packet, &sWriteHeader, node, &mLastSendTime);
            }
        }
        ++mSendSeq;
        break;
    case Transport::Method::type_relay:
    {
        unsigned char receivers = 0;
        for (node = 0; node < numNodes; ++node)
        {
            if (VoiceChat::Node::sTransport->GetSendPossible(mNodeID, node))
            {
                ++receivers;
            }
        }
        const unsigned short sequence = mSendSeq++;
        SendAudioPacketPeerRelay(packet, mNodeID, numNodes, method.CalculateOptimalRelayNodes(receivers),
                   sequence, false, &mLastSendTime);
        break;
    }
    case Transport::Method::type_serverRelay:
    {
        const unsigned short sequence = mSendSeq++;
        VoiceChat::Node::sWriteHeader.SendSetup((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), mNodeID, 0xff, packet->mDiscontinuity, sequence);

        const float sendTime = GetTime();
        for (node = 0; node < numNodes; ++node)
        {
            const bool receive = VoiceChat::Node::sTransport->GetSendPossible(mNodeID, node);
            sWriteHeader.SetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), node, receive);
            SendMetrics& metrics = VoiceChat::Node::sSendMetrics[node];
            ++metrics.sendCount;
            metrics.lastSendTime = sendTime;
        }
        SendAudioPacket(packet, &sWriteHeader, 0xff, &mLastSendTime);
        break;
    }
    default:
        break;
    }
    mSendSeq &= 0x3ff;
}

// FUNCTION: SWBF2 0x00673670
void VoiceChat::Node::SendAudioPacketPeerRelay(const AudioPacket* const packet, unsigned char first,
                           unsigned char numNodes, unsigned char maxRelayNodes,
                           unsigned short sequence, bool incoming, float* sendTime)
{
    unsigned char node = first;
    unsigned char selected = 0xff;
    unsigned char grouped = 0;
    for (unsigned int index = 0; index < numNodes; ++index)
    {
        const bool last = index == numNodes - 1u;
        const bool receive = incoming ? VoiceChat::Node::sReadHeader.GetReceiveNode((*VoiceChat::Node::sTransport), node)
                                      : VoiceChat::Node::sTransport->GetSendPossible(first, node);
        bool handle = last;
        if (receive)
        {
            if (selected == 0xff)
            {
                VoiceChat::Node::sWriteHeader.SendSetup((*VoiceChat::Node::sTransport), first, 0xff, packet->mDiscontinuity, sequence);
                for (unsigned char i = 0; i < numNodes; ++i)
                {
                    VoiceChat::Node::sWriteHeader.SetReceiveNode((*VoiceChat::Node::sTransport), i, i == node);
                }
                selected = node;
                grouped = 0;
            }
            else
            {
                handle = true;
            }
        }
        if (handle && selected != 0xff)
        {
            VoiceChat::Node::sWriteHeader.SetReceiveNode((*VoiceChat::Node::sTransport), node, receive);
            if (++grouped == maxRelayNodes || last)
            {
                SendAudioPacket(packet, &VoiceChat::Node::sWriteHeader, selected, sendTime);
                selected = 0xff;
            }
        }
        if (++node == numNodes)
        {
            node = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006743b0
bool VoiceChat::Node::SendAudioPacket(const AudioPacket* const audio, const Header* const  ,
                                       unsigned char destination, float* const sendTime)
{
    VoiceChat::Node::SendMetrics* metrics = destination == 0xff ? 0
        : &((VoiceChat::Node::SendMetrics*)VoiceChat::Node::sSendMetrics)[destination];
    NetPacket* packet = (NetPacket*)VoiceChat::NetPacketAudio::Alloc();
    if (packet)
    {
        VoiceChat::NetPacketAudio wrapper(packet);
        const unsigned int headerBytes = VoiceChat::Node::sWriteHeader.Write(&wrapper);
        const unsigned int bytes = audio->mSize ? wrapper.Write(
            audio->mData, audio->mSize) : 0;
        VoiceChat::NetPacketAudio::Send(&wrapper, destination, headerBytes);
        if (sendTime)
        {
            *sendTime = VoiceChat::Node::GetTime();
        }
        if (metrics)
        {
            if (sendTime)
            {
                const float previous = metrics->lastSendTimeDirect;
                if (*sendTime > previous)
                {
                    metrics->lastSendInterval = *sendTime - previous;
                    metrics->bandwidth = (float)bytes * 0.0078125f / metrics->lastSendInterval;
                }
                else
                {
                    metrics->bandwidth = (float)bytes * 0.0078125f / metrics->lastSendInterval + metrics->bandwidth;
                }
                metrics->lastSendTime = *sendTime;
                metrics->lastSendTimeDirect = *sendTime;
            }
            ++metrics->sendCountDirect;
            ++metrics->sendCount;
        }
        return true;
    }
    if (metrics)
    {
        ++metrics->droppedCount;
    }
    return false;
}

// FUNCTION: SWBF2 0x006744d0
void VoiceChat::Node::RelayAudioPacket(const AudioPacket* const packet, unsigned char node,
                                        unsigned short sequence)
{
    VoiceChat::Transport* transport = (VoiceChat::Transport*)VoiceChat::Node::sTransport;
    const VoiceChat::Transport::Method& method = transport->GetMethod();
    switch (method.GetType())
    {
    case VoiceChat::Transport::Method::type_relay:
        SendAudioPacketPeerRelay(packet, node, transport->GetNumNodes(), method.GetNumRelayNodes(),
                   sequence, true, 0);
        break;
    case VoiceChat::Transport::Method::type_serverRelay:
    {
        const unsigned char numNodes = transport->GetNumNodes();
        for (unsigned char i = 0; i < numNodes; ++i)
        {
            if (VoiceChat::Node::sReadHeader.GetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), i))
            {
                VoiceChat::Node::sWriteHeader.SendSetup((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), node, i, packet->mDiscontinuity, sequence);
                SendAudioPacket(packet, &VoiceChat::Node::sWriteHeader, i, 0);
            }
        }
        break;
    }
    }
}

namespace VoiceChat
{

// FUNCTION: SWBF2 0x00673110
void Node::RepeatLastPacket(unsigned char stream)
{
    ReceiveStream& receive = mReceiveStreams[stream];
    if (!receive.lastPacketSize)
    {
        return;
    }
    const int previous = receive.readIndex ? receive.readIndex - 1 : mMaxReceivePackets - 1;
    AudioPacket& packet = receive.packet[previous];
    if (packet.mAppData)
    {
        return;
    }
    const bool different = &receive.packet[receive.writeIndex] != &packet;
    packet.mSize = receive.lastPacketSize;
    ReceiveAudioPacket(stream, &packet);
    if (different)
    {
        packet.mSize = 0;
    }
    DecodeQueue(stream, true);
}

// FUNCTION: SWBF2 0x006737a0
void Node::DecodeQueue(unsigned char stream, bool force)
{
    ReceiveStream& receive = mReceiveStreams[stream];
    if (!receive.activated && CalculateNumPackets(receive) < receive.numPackets && !force)
    {
        return;
    }

    AudioPacket* packet = &receive.packet[receive.readIndex];
    const float timePerByte = mEncoderPacketSizeSec / (float)(int)mEncoder->GetMaxOutputPacketSizeBytes();
    const bool connected = ((Transport*)VoiceChat::Node::sTransport)->GetReceivePossible(mNodeID, stream);
    if (!connected && receive.activated)
    {
        AudioPacket discontinuity;
        discontinuity.mDiscontinuity = true;
        discontinuity.mData = 0;
        discontinuity.mSize = 0;
        discontinuity.mAppData = 0;
        DecoderReceiveAudioPacket(stream, &discontinuity);
    }
    while (packet->mAppData)
    {
        if (connected && !DecoderReceiveAudioPacket(stream, packet))
        {
            return;
        }
        if (force)
        {
            receive.activated = true;
            receive.starving = connected;
        }
        else if (connected)
        {
            receive.activated = !packet->mDiscontinuity;
            receive.starving = false;
            receive.lastPacketSize = packet->mSize;
            receive.bufferedData += (float)(int)packet->mSize * timePerByte;
        }
        packet->mAppData = 0;
        packet->mSize = 0;
        if (++receive.readIndex == mMaxReceivePackets)
        {
            receive.readIndex = 0;
        }
        packet = &receive.packet[receive.readIndex];
    }
}

// FUNCTION: SWBF2 0x00673900
bool Node::ReceiveAudioPacket(unsigned char stream, const AudioPacket* packet)
{
    unsigned int copied = 0;
    if (packet->mSize)
    {
        mLastReceiveTime = Node::GetTime();
    }
    bool more;
    do
    {
        Node::ReceiveStream& receive = mReceiveStreams[stream];
        AudioPacket& slot = receive.packet[receive.writeIndex];
        DecodeQueue(stream, false);
        if (slot.mAppData)
        {
            AudioPacket discontinuity;
            discontinuity.mDiscontinuity = true;
            discontinuity.mData = 0;
            discontinuity.mSize = 0;
            discontinuity.mAppData = 0;
            Flush(receive, discontinuity);
            return false;
        }
        unsigned int size = packet->mSize - copied;
        const unsigned int remaining = mMaxReceiveSize - slot.mSize;
        if (size > remaining)
        {
            size = remaining;
        }
        if (size)
        {
            void* dst = (unsigned char*)slot.mData + slot.mSize;
            const void* src = (const unsigned char*)packet->mData + copied;
            if (src != dst)
            {
                memcpy(dst, src, size);
            }
        }
        slot.mSize += size;
        copied += size;
        more = copied < packet->mSize;
        slot.mDiscontinuity = !more && packet->mDiscontinuity;
        if (slot.mSize == mMaxReceiveSize || slot.mDiscontinuity)
        {
            slot.mAppData = this;
            if (++receive.writeIndex == mMaxReceivePackets)
            {
                receive.writeIndex = 0;
            }
            if (slot.mDiscontinuity)
            {
                receive.activated = true;
            }
            DecodeQueue(stream, false);
        }
    } while (more);
    return true;
}

// FUNCTION: SWBF2 0x00673ad0
unsigned char Node::DecoderInputAlloc(unsigned char stream)
{
    ReceiveStream* const streams = mReceiveStreams;
    ReceiveStream& receive = streams[stream];
    const unsigned char count = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();
    if (mNumDecoderInputs == count)
    {
        mAllocatedInputs[stream] = stream;
        receive.decoderInput = stream;
        return stream;
    }

    unsigned char oldest = 0xff;
    float oldestTime = FLT_MAX;
    for (unsigned char input = 0; input < mNumDecoderInputs; ++input)
    {
        const unsigned char owner = mAllocatedInputs[input];
        if (owner == 0xff)
        {
            mAllocatedInputs[input] = stream;
            receive.decoderInput = input;
            return input;
        }
        if (oldestTime > streams[owner].activationTime)
        {
            oldestTime = streams[owner].activationTime;
            oldest = input;
        }
    }
    DecoderInputFree(oldest, true);
    mAllocatedInputs[oldest] = stream;
    receive.decoderInput = oldest;
    return oldest;
}

// FUNCTION: SWBF2 0x00673ba0
void Node::DecoderInputFree(unsigned char input, bool flush)
{
    if (mNumDecoderInputs == ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes())
    {
        return;
    }
    unsigned char& owner = mAllocatedInputs[input];
    ReceiveStream& receive = mReceiveStreams[owner];
    if (flush)
    {
        AudioPacket packet;
        packet.mDiscontinuity = true;
        packet.mData = 0;
        packet.mSize = 0;
        packet.mAppData = 0;
        Flush(receive, packet);
    }
    else
    {
        receive.decoderInput = 0xff;
        owner = 0xff;
    }
}

// FUNCTION: SWBF2 0x00673cc0
bool Node::DecoderIsStarving(unsigned char stream, bool force)
{
    const unsigned char input = mReceiveStreams[stream].decoderInput;
    if (input == 0xff)
    {
        return false;
    }
    return mDecoder->IsStarving(input, force);
}
}

// FUNCTION: SWBF2 0x00672f10
unsigned char VoiceChat::Node::GetID() const
{
    return mNodeID;
}

// FUNCTION: SWBF2 0x00672f20
VoiceChat::Encoder& VoiceChat::Node::GetEncoder() const
{
    return *mEncoder;
}

// FUNCTION: SWBF2 0x00672f30
VoiceChat::Decoder& VoiceChat::Node::GetDecoder() const
{
    return *mDecoder;
}

// FUNCTION: SWBF2 0x00673ed0
bool VoiceChat::Node::IsOpenStatic()
{
    return VoiceChat::Node::sTransport != 0;
}

// FUNCTION: SWBF2 0x00674020
void VoiceChat::Node::Update(float fDeltaTime)
{
    void* pAppPacket;
    while ((pAppPacket = NetPacketAudio::Receive()) != 0)
    {
        NetPacketAudio kAudio(pAppPacket);
        if (VoiceChat::Node::sReadHeader.Read(*VoiceChat::Node::sTransport, &kAudio))
        {
            unsigned int uiSize;
            const unsigned short uSeq = VoiceChat::Node::sReadHeader.GetSeqNum();
            const unsigned char uID = VoiceChat::Node::sReadHeader.GetSendNodeID(*VoiceChat::Node::sTransport);
            ReceiveMetrics& rm = VoiceChat::Node::sReceiveMetrics[uID];
            const bool bDiscontinuity = VoiceChat::Node::sReadHeader.GetDiscontinuity();

            if (!(uSeq < rm.receiveSeq
                  && uSeq - rm.receiveSeq + 1023 >= (int)VoiceChat::Node::sSeqNumVariance
                  && rm.activated))
            {
                unsigned short uDropped = rm.activated
                    ? (unsigned short)(uSeq - rm.receiveSeq - 1) : 0;

                AudioPacket kPacket;
                uiSize = 0xFFFFFFFF;
                kPacket.mData = kAudio.Read(uiSize);
                kPacket.mSize = uiSize;
                kPacket.mAppData = 0;
                kPacket.mDiscontinuity = bDiscontinuity;

                if (uDropped)
                {
                    ++rm.outOfOrderCount;
                    if (uDropped > VoiceChat::Node::sRepeatMax)
                    {
                        uDropped = VoiceChat::Node::sRepeatMax;
                    }
                }

                if (ReceiveAudioPacketAllLocalNodes(uID, &kPacket, bDiscontinuity, uDropped))
                {
                    RelayAudioPacket(&kPacket, uID, uSeq);
                }

                if (rm.activated)
                {
                    const float fInterval = sTime - rm.lastReceiveTime;
                    if (fInterval != 0.0f)
                    {
                        if (fInterval > rm.receiveLatency)
                        {
                            rm.receiveLatency = rm.receiveLatency * 0.5f + fInterval * 0.5f;
                        }
                        else
                        {
                            rm.receiveLatency = rm.receiveLatency * 0.99f + fInterval * 0.01f;
                        }
                    }
                }

                rm.lastPacketSize = uiSize;
                rm.activated = !bDiscontinuity;
            }

            rm.receiveSeq = uSeq;
            rm.lastReceiveTime = sTime;
        }
        NetPacketAudio::Free(pAppPacket);
    }

    for (void** pLink = (void**)VoiceChat::Node::sList[1]; pLink[3]; pLink = (void**)pLink[1])
    {
        VoiceChat::Node::sSendMetrics[((Node*)pLink[3])->GetID()].bandwidth = 0.0f;
        if (((VoiceChat::Node*)pLink[3])->IsOpen())
        {
            ((Node*)pLink[3])->Send();
            ((Node*)pLink[3])->Decode(fDeltaTime);
        }
    }

    const unsigned char uNodes = VoiceChat::Node::sTransport->GetNumNodes();
    for (unsigned char i = 0; i < uNodes; ++i)
    {
        ReceiveMetrics& rm = VoiceChat::Node::sReceiveMetrics[i];
        if (rm.activated && sTime - rm.lastReceiveTime > VoiceChat::Node::sReceiveBufferSize)
        {
            rm.activated = false;

            AudioPacket kSilence;
            kSilence.mDiscontinuity = true;
            kSilence.mData = 0;
            kSilence.mSize = 0;
            kSilence.mAppData = 0;
            for (void** pLink = (void**)VoiceChat::Node::sList[1]; pLink[3]; pLink = (void**)pLink[1])
            {
                ((Node*)pLink[3])->ReceiveAudioPacket(i, &kSilence);
            }
        }

        SendMetrics& sm = VoiceChat::Node::sSendMetrics[i];
        sm.bandwidthAverage = sm.bandwidth * (1.0f - 0.9f) + sm.bandwidthAverage * 0.9f;
    }

    sTime += fDeltaTime;
}

// FUNCTION: SWBF2 0x00673f50
unsigned char __cdecl VoiceChat::Node::ReceiveAudioPacketAllLocalNodes(unsigned char uID,
                           const VoiceChat::AudioPacket* pPacket,
                           bool  , unsigned short uDropped)
{
    unsigned char uRemaining = VoiceChat::Node::sReadHeader.GetNumReceiveNodes((*VoiceChat::Node::sTransport));
    PblList<VoiceChat::Node>* pList = (PblList<VoiceChat::Node>*)VoiceChat::Node::sList;
    for (PblList<VoiceChat::Node>::Node* pLink = pList->GetFirst();
         pLink->_pObject; pLink = pLink->_pNext)
    {
        unsigned char uNodeID = pLink->_pObject->GetID();
        if (!VoiceChat::Node::sReadHeader.GetReceiveNode((*VoiceChat::Node::sTransport), uNodeID))
        {
            continue;
        }
        VoiceChat::Transport* pTransport = VoiceChat::Node::sTransport;
        if (pTransport->GetReceivePossible(uNodeID, uID))
        {
            if (!VoiceChat::Node::sSkipDropped)
            {
                for (unsigned short i = uDropped; i != 0; --i)
                {
                    pLink->_pObject->RepeatLastPacket(uID);
                }
            }
            pLink->_pObject->ReceiveAudioPacket(uID, pPacket);
        }
        if (VoiceChat::Node::sReadHeader.GetFormat() == VoiceChat::Node::Header::format_relay)
        {
            VoiceChat::Node::sReadHeader.SetReceiveNode((*VoiceChat::Node::sTransport), uNodeID, false);
        }
        --uRemaining;
    }
    return uRemaining;
}

// LIBRARY: SWBF2 0x0075490e
// DirectSoundCaptureCreate8
