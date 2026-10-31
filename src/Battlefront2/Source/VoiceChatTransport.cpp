#include <string.h>

#include "VoiceChatTransport.h"
#include "GameVoiceChat.h"

#include "VoiceChatUtil.h"



namespace VoiceChat
{
void* Alloc(unsigned int uiSize);
}

namespace VoiceChat
{
void Free(void* pBlock);
}


// FUNCTION: SWBF2 0x006745c0
VoiceChat::Transport::Method::Method(Type type, unsigned char numRelayNodes)
{
    SetType(type);
    SetNumRelayNodes(numRelayNodes);
}

// FUNCTION: SWBF2 0x00674620
unsigned char VoiceChat::Transport::Method::CalculateOptimalRelayNodes(unsigned char iNumNodes) const
{
    const unsigned char ucNodes = (unsigned char)iNumNodes;
    const unsigned char ucSelf = mNumRelayNodes;

    unsigned char ucRelay = (ucSelf > ucNodes) ? ucNodes : ucSelf;

    while (ucRelay > (unsigned char)((int)ucNodes / (int)(ucRelay + 1)))
    {
        --ucRelay;
    }

    return ucRelay ? ucRelay : 1;
}

// FUNCTION: SWBF2 0x006749b0
unsigned char VoiceChat::Transport::GetNumNodes() const
{
    return mNumNodes;
}

// FUNCTION: SWBF2 0x006749c0
unsigned char VoiceChat::Transport::GetNumNodesLog2() const
{
    return mNumNodesLog2;
}

// FUNCTION: SWBF2 0x00674f10
void VoiceChat::Transport::SetMethod(const Method& method, bool bBoth)
{
    mMethodOutgoing = method;

    if (bBoth)
    {
        mMethodIncoming = method;
    }
}

// FUNCTION: SWBF2 0x00674700
bool VoiceChat::Transport::Open(unsigned char numNodes, unsigned char numLocalNodes)
{
    const unsigned char roundedNodes = numNodes + 7;
    const unsigned char widthHeight = roundedNodes & ~7;
    const unsigned int widthBytes = roundedNodes >> 3;
    const unsigned int paddedWidth = (widthBytes + 3) & ~3;
    const unsigned int paddedLocalNodes = (numLocalNodes + 3) & ~3;
    const unsigned int bitmapSize = (widthHeight * widthHeight) >> 3;
    unsigned char* p = (unsigned char*)VoiceChat::Alloc(
        bitmapSize * 5 + paddedWidth * 9 + numNodes * 4 + paddedLocalNodes);
    mLocal.mSend = p;
    if (!p)
    {
        return false;
    }

    p += bitmapSize;
    mLocal.mReceive = p;
    p += bitmapSize;
    mLocal.mConnected = p;
    p += paddedWidth * 2;
    mLocal.mSending = p;
    p += paddedWidth;
    mIncoming.mSend = p;
    p += bitmapSize;
    mIncoming.mReceive = p;
    p += bitmapSize;
    mIncoming.mConnected = p;
    p += paddedWidth * 2;
    mIncoming.mSending = p;
    p += paddedWidth;
    mClientChangedNodes = p;
    p += paddedWidth;
    mHostChangedNodes = p;
    p += bitmapSize;
    mHostChangedPckID = (unsigned int*)p;
    p += numNodes * 4;
    mLocalNodes = p;
    p += paddedLocalNodes;
    mHostUpdated = p;

    mClientChangedPckID = 0;
    mSize = bitmapSize;
    mWidthHeight = widthHeight;
    mWidthBytes = (unsigned char)widthBytes;
    mNumNodes = numNodes;
    mNumNodesLog2 = (unsigned char)(VoiceChat::GetMSBIndex(numNodes) + 1);
    mClientUpdated = false;
    mNumLocalNodes = 0;
    mMaxLocalNodes = numLocalNodes;
    mMethodOutgoing.SetType(Method::type_none);
    mMethodOutgoing.SetNumRelayNodes(0);
    mMethodIncoming = mMethodOutgoing;

    memset(mLocal.mSend, 0, mSize);
    memset(mLocal.mReceive, 0, mSize);
    memset(mLocal.mConnected, 0, widthBytes * 2);
    memset(mLocal.mSending, 0, widthBytes);
    memset(mIncoming.mSend, 0, mSize);
    memset(mIncoming.mReceive, 0, mSize);
    memset(mIncoming.mConnected, 0, widthBytes * 2);
    memset(mIncoming.mSending, 0, widthBytes);
    memset(mClientChangedNodes, 0, widthBytes);
    memset(mHostChangedNodes, 0, mSize);
    memset(mHostChangedPckID, 0, mNumNodes * sizeof(unsigned int));
    memset(mLocalNodes, 0, numLocalNodes);
    memset(mHostUpdated, 0, widthBytes);
    return true;
}

// FUNCTION: SWBF2 0x00674920
void VoiceChat::Transport::Close()
{

    VoiceChat::Free(mLocal.mSend);

    mClientUpdated = false;

    mLocal.mSend = 0;
    mLocal.mReceive = 0;
    mLocal.mConnected = 0;
    mLocal.mSending = 0;
    mIncoming.mSend = 0;
    mIncoming.mReceive = 0;
    mIncoming.mConnected = 0;
    mIncoming.mSending = 0;
    mClientChangedNodes = 0;
    mHostChangedNodes = 0;
    mHostChangedPckID = 0;
    mLocalNodes = 0;

    mHostUpdated = 0;
    mClientChangedPckID = 0;
    mSize = 0;

    mWidthHeight = 0;
    mWidthBytes = 0;
    mNumNodes = 0;
    mNumNodesLog2 = 0;

    mNumLocalNodes = 0;
    mMaxLocalNodes = 0;

    mMethodOutgoing.SetType((VoiceChat::Transport::Method::Type)0);
    mMethodOutgoing.SetNumRelayNodes(0);

    mMethodIncoming = mMethodOutgoing;
}

// FUNCTION: SWBF2 0x00674670
VoiceChat::Transport::Transport()
    : mClientChangedNodes(0), mHostChangedNodes(0), mHostChangedPckID(0), mLocalNodes(0),
      mHostUpdated(0), mClientChangedPckID(0), mSize(0), mWidthHeight(0), mWidthBytes(0),
      mNumNodes(0), mNumNodesLog2(0), mMethodOutgoing(Method::type_invalid, 0),
      mMethodIncoming(Method::type_invalid, 0)
{
    mClientUpdated = false;
}

VoiceChat::Transport::~Transport()
{
}

// FUNCTION: SWBF2 0x00674f50
unsigned char* VoiceChat::Transport::GetBitmapElementAndMask(unsigned char* const bitmap,
                                                             unsigned char x, unsigned char y,
                                                             unsigned char& mask)
{
    return VoiceChat::GetBitmapElementAndMask(bitmap, y * mWidthHeight + x, mask);
}

// FUNCTION: SWBF2 0x00674f80
bool VoiceChat::Transport::SetBit(unsigned char* const bitmap, unsigned char x, unsigned char y,
                                  bool value, unsigned char* const changed, bool changedByX)
{
    unsigned char mask;
    unsigned char* element = GetBitmapElementAndMask(bitmap, x, y, mask);
    const unsigned char old = *element;
    if (value)
    {
        *element = old | mask;
    }
    else
    {
        *element = ~mask & old;
    }

    const bool bChanged = *element != old;
    if (bChanged && changed)
    {
        VoiceChat::SetBit(changed, changedByX ? x : y, true);
    }
    return bChanged;
}

// FUNCTION: SWBF2 0x00674ff0
bool VoiceChat::Transport::GetBit(const unsigned char* const bitmap, unsigned char x,
                                  unsigned char y) const
{
    unsigned char mask;
    const unsigned char* element = const_cast<Transport*>(this)->GetBitmapElementAndMask(
        const_cast<unsigned char*>(bitmap), x, y, mask);
    return (*element & mask) != 0;
}

// FUNCTION: SWBF2 0x00674e00
bool VoiceChat::Transport::GetSendEnable(unsigned char localNode, unsigned char node) const
{
    return GetBit(mLocal.mSend, node, localNode);
}

// FUNCTION: SWBF2 0x00674e50
bool VoiceChat::Transport::GetReceiveEnable(unsigned char localNode, unsigned char node) const
{
    return GetBit(mLocal.mReceive, node, localNode);
}

// FUNCTION: SWBF2 0x00675020
void VoiceChat::Transport::SetColumnBits(unsigned char* const bitmap, unsigned char x, bool value,
                                         unsigned char except)
{
    for (unsigned char i = 0; i < mNumNodes; ++i)
    {
        if (except != i)
        {
            SetBit(bitmap, x, i, value, 0, true);
        }
    }
}

// FUNCTION: SWBF2 0x00675070
void VoiceChat::Transport::SetRowBits(unsigned char* const bitmap, unsigned char y, bool value,
                                      unsigned char except)
{
    for (unsigned char i = 0; i < mNumNodes; ++i)
    {
        if (except != i)
        {
            SetBit(bitmap, i, y, value, 0, true);
        }
    }
}

// FUNCTION: SWBF2 0x00674c80
void VoiceChat::Transport::UpdateLocalState(unsigned char node, bool)
{
    if (!IsLocal(node))
    {
        unsigned char mask;
        memcpy(GetBitmapElementAndMask(mLocal.mSend, 0, node, mask),
               GetBitmapElementAndMask(mIncoming.mSend, 0, node, mask), mWidthBytes);
        memcpy(GetBitmapElementAndMask(mLocal.mReceive, 0, node, mask),
               GetBitmapElementAndMask(mIncoming.mReceive, 0, node, mask), mWidthBytes);

        const bool bReceive = GetBit(mIncoming.mConnected, node, 0);
        const bool bSend = GetBit(mIncoming.mConnected, node, 1);
        const bool bChanged = SetBit(mLocal.mConnected, node, 1, bSend, 0, true) |
                              SetBit(mLocal.mConnected, node, 0, bReceive, 0, true);
        const bool bConnected = bChanged && (bReceive || bSend);

        VoiceChat::SetBit(mLocal.mSending, node, VoiceChat::GetBit(mIncoming.mSending, node));
        if (bConnected)
        {
            SetRowBits(mHostChangedNodes, node, true, node);
            VoiceChat::SetBit(mHostUpdated, node, true);
        }
    }
    SetColumnBits(mHostChangedNodes, node, true, node);
    SetRowBits(mHostUpdated, 0, true, node);
}

// FUNCTION: SWBF2 0x00674b00
void VoiceChat::Transport::DisconnectNode(unsigned char node)
{
    if (SetBit(mIncoming.mConnected, node, 1, false, 0, true) | SetBit(mIncoming.mConnected, node, 0, false, 0, true))
    {
        unsigned char mask;
        memset(GetBitmapElementAndMask(mIncoming.mSend, 0, node, mask), 0, mWidthBytes);
        memset(GetBitmapElementAndMask(mIncoming.mReceive, 0, node, mask), 0, mWidthBytes);
        SetBit(mIncoming.mSending, node, 0, false, 0, true);
        UpdateLocalState(node, false);
    }
}

// FUNCTION: SWBF2 0x00674ac0
bool VoiceChat::Transport::IsLocal(unsigned char node) const
{
    for (unsigned char i = 0; i < mNumLocalNodes; ++i)
    {
        if (mLocalNodes[i] == node)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00674bd0
bool VoiceChat::Transport::IsConnectedSend(unsigned char node) const
{
    return GetBit(mLocal.mConnected, node, 1);
}

// FUNCTION: SWBF2 0x00674bf0
bool VoiceChat::Transport::IsConnectedReceive(unsigned char node) const
{
    return GetBit(mLocal.mConnected, node, 0);
}

// FUNCTION: SWBF2 0x00674c10
void VoiceChat::Transport::SetSending(unsigned char node, bool sending)
{
    mClientUpdated |= SetBit(mLocal.mSending, node, 0, sending, mClientChangedNodes, true);
}

// FUNCTION: SWBF2 0x00674dd0
void VoiceChat::Transport::SetSendEnable(unsigned char localNode, unsigned char node, bool enable,
                                         bool)
{
    mClientUpdated |= SetBit(mLocal.mSend, node, localNode, enable, mClientChangedNodes, false);
}

// FUNCTION: SWBF2 0x00674e20
void VoiceChat::Transport::SetReceiveEnable(unsigned char localNode, unsigned char node,
                                            bool enable, bool)
{
    mClientUpdated |= SetBit(mLocal.mReceive, node, localNode, enable, mClientChangedNodes, false);
}

// FUNCTION: SWBF2 0x006749d0
void VoiceChat::Transport::SetLocalNode(unsigned char node, bool local, bool receive, bool send)
{
    const unsigned char numNodes = mNumLocalNodes;
    if (local)
    {
        bool bFound = false;
        for (unsigned char i = 0; i < numNodes; ++i)
        {
            bFound = (mLocalNodes[i] == node);
            if (bFound)
            {
                break;
            }
        }
        if (!bFound)
        {
            mLocalNodes[numNodes] = node;
            ++mNumLocalNodes;
        }
    }
    else
    {
        for (unsigned char i = 0; i < numNodes; ++i)
        {
            if (mLocalNodes[i] == node)
            {
                for (; i < numNodes - 1; ++i)
                {
                    mLocalNodes[i] = mLocalNodes[i + 1];
                }
                --mNumLocalNodes;
                break;
            }
        }
    }

    const bool bReceive = local && receive;
    const bool bSend = local && send;
    const bool bReceiveChanged = SetBit(mLocal.mConnected, node, 0, bReceive, mClientChangedNodes, true);
    const bool bSendChanged = SetBit(mLocal.mConnected, node, 1, bSend, mClientChangedNodes, true);
    mClientUpdated |= bReceiveChanged | bSendChanged;
}

// FUNCTION: SWBF2 0x00674c40
bool VoiceChat::Transport::GetSending(unsigned char node) const
{
    return VoiceChat::GetBit(mLocal.mSending, node) && IsConnectedSend(node);
}

// FUNCTION: SWBF2 0x00674610
unsigned char VoiceChat::Transport::Method::GetNumRelayNodes() const
{
    return mNumRelayNodes;
}

// FUNCTION: SWBF2 0x00674ba0
bool VoiceChat::Transport::IsConnectedNet(unsigned char node) const
{
    return IsConnectedSend(node) || IsConnectedReceive(node);
}

// FUNCTION: SWBF2 0x00674e70
bool VoiceChat::Transport::GetSendPossible(unsigned char local, unsigned char node) const
{
    return GetSendEnable(local, node) && GetReceiveEnable(node, local) &&
           IsConnectedSend(local) && IsConnectedReceive(node);
}

// FUNCTION: SWBF2 0x00674ec0
bool VoiceChat::Transport::GetReceivePossible(unsigned char node, unsigned char stream) const
{
    return GetReceiveEnable(node, stream) && GetSendEnable(stream, node) &&
           IsConnectedReceive(node) && IsConnectedSend(stream);
}

// FUNCTION: SWBF2 0x006745e0
void VoiceChat::Transport::Method::SetType(Type type) { mType = type; }

// FUNCTION: SWBF2 0x00674600
void VoiceChat::Transport::Method::SetNumRelayNodes(unsigned char numRelayNodes) { mNumRelayNodes = numRelayNodes; }

// FUNCTION: SWBF2 0x006745f0
VoiceChat::Transport::Method::Type VoiceChat::Transport::Method::GetType() const { return mType; }

// FUNCTION: SWBF2 0x00674f40
const VoiceChat::Transport::Method& VoiceChat::Transport::GetMethod() const { return mMethodIncoming; }
