#include "GameVoiceChatUtil.h"

#include "NetGame.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatTransport.h"
#include "VoiceChatNode.h"

#include "NetPacket.h"
#include "SoundFL/source/sndqueue.h"


// FUNCTION: SWBF2 0x0053e300
void* VoiceChat::NetPacketAudio::Receive()
{
    NetPacket* pPacket = 0;
    for (unsigned int i = 0; i < 4; ++i)
    {
        if (!GameVoiceChat::gPacketQueue[i].IsEmpty())
        {
            GameVoiceChat::gPacketQueue[i].Get(pPacket);
            break;
        }
    }

    if (pPacket == 0)
    {
        pPacket = NetComm::Recv(PKT_VOICE, PKT_NONE);
        if (pPacket == 0)
        {
            return 0;
        }
    }

    pPacket->BeginRead();
    return pPacket;
}

// FUNCTION: SWBF2 0x0053e170
VoiceChat::NetPacketAudio::NetPacketAudio(void* packet) : mAppPacket(packet) {}

// FUNCTION: SWBF2 0x0053e180
unsigned int VoiceChat::NetPacketAudio::Write(const void* pData, unsigned int uiSize)
{
    return ((NetPacket*)mAppPacket)->Write((const char*)pData, (int)uiSize);
}

// FUNCTION: SWBF2 0x0053e190
unsigned int VoiceChat::NetPacketAudio::Read(void* pData, unsigned int uiSize)
{
    return ((NetPacket*)mAppPacket)->Read((char*)pData, (int)uiSize);
}

// FUNCTION: SWBF2 0x0053e1a0
void* VoiceChat::NetPacketAudio::Read(unsigned int& length)
{
    NetPacket* current = (NetPacket*)mAppPacket;
    unsigned int available = (unsigned int)((int)current->mMaxIndex - (int)current->mCurIndex);
    if (length > available)
    {
        length = available;
    }
    char* data = current->mData + current->mCurIndex;
    current->mCurIndex += (short)length;
    return data;
}

// FUNCTION: SWBF2 0x0053e1d0
void* VoiceChat::NetPacketAudio::Alloc()
{
    NetPacket* packet = NetPacket::Alloc(false);
    if (packet)
    {
        packet->BeginWrite();
        return packet;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0053e380
void VoiceChat::NetPacketAudio::Free(void* pPacket)
{
    ((NetPacket*)pPacket)->Free();
}

// TEMPLATE: SWBF2 0x0053e3e0
// Snd::Queue<NetPacket *,4>::~Queue<NetPacket *,4>
template Snd::Queue<NetPacket*, 4>::~Queue();

namespace GameVoiceChat
{

// FUNCTION: SWBF2 0x0053e1f0
bool __cdecl SendVoice(NetPacket* packet, unsigned char destination, unsigned int voiceDataOffset)
{
    unsigned int node = destination == 0xff ? 64 : destination;
    if (netInShell ? netEnabled : netEnabledNext)
    {
        if (NetComm::IsConnected(node))
        {
            NetComm::SendVoice(node, PKT_VOICE, packet, voiceDataOffset);
            return true;
        }
    }
    else if (destination != 0xff)
    {
        GameVoiceChat::Transport* transport = GameVoiceChat::GetTransport();
        if (transport)
        {
            if (transport->IsLocal((unsigned char)node))
            {
                unsigned int count = GameVoiceChat::GetNumNodes();
                unsigned int i = 0;
                if (count)
                {
                    VoiceChat::Node* voiceNode = GameVoiceChat::GetNode(i);
                    do
                    {
                        if (voiceNode && voiceNode->GetID() == node)
                        {
                            break;
                        }
                        ++i;
                    } while (i < count);
                }
                if (!GameVoiceChat::gPacketQueue[i].IsFull())
                {
                    GameVoiceChat::gPacketQueue[i].Add(packet);
                    return true;
                }
            }
        }
    }
    VoiceChat::NetPacketAudio::Free(packet);
    return false;
}

}

// FUNCTION: SWBF2 0x0053e2e0
void __cdecl VoiceChat::NetPacketAudio::Send(NetPacketAudio* packet, unsigned char destination,
                            unsigned int voiceDataOffset)
{
    NetPacket* appPacket = static_cast<NetPacket*>(packet->mAppPacket);
    appPacket->EndWrite();
    GameVoiceChat::SendVoice(appPacket, destination, voiceDataOffset);
}

// TEMPLATE: SWBF2 0x0053e3f0
// Snd::Queue<NetPacket *,4>::Add
template class Snd::Queue<NetPacket*, 4>;

// TEMPLATE: SWBF2 0x0053e3d0
// Snd::Queue<NetPacket *,4>::Queue<NetPacket *,4>

class NetPacket;

namespace GameVoiceChat
{

// SYNTHETIC: SWBF2 0x00403ef0
// GameVoiceChat::`dynamic initializer for 'gPacketQueue''
// SYNTHETIC: SWBF2 0x00768af0
// GameVoiceChat::`dynamic atexit destructor for 'gPacketQueue''

// GLOBAL: SWBF2 0x01ec6020
// gPacketQueue
Snd::Queue<NetPacket*, 4> gPacketQueue[4];
}
