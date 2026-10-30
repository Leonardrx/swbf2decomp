#include "sndenginebase.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndbus.h"
#include "sndglobals.h"
#include "sndi3dl2reverbpreset.h"
#include "sndparametergraph.h"
#include "sndsamplebankbase.h"
#include "sndsoundparameterized.h"
#include "sndsound.h"
#include "sndsoundlayered.h"
#include "sndsoundstream.h"
#include "sndspace.h"
#include "sndvoicebase.h"
#include "win/sndstream.h"
#include "win/sndvoice.h"

namespace Snd
{

// GLOBAL: SWBF2 0x009d8410
// EngineBase::smStreams
Stream* EngineBase::smStreams;
// GLOBAL: SWBF2 0x009d8414
Voice* EngineBase::smVoices;
// GLOBAL: SWBF2 0x009d8418
bool EngineBase::smOpened;

// GLOBAL: SWBF2 0x009d8420
unsigned char EngineBase::smVoiceStorage[0xA800];

// GLOBAL: SWBF2 0x009e2c20
unsigned char EngineBase::smStreamStorage[0x1446A68];

// GLOBAL: SWBF2 0x01e29688
// EngineBase::smTimeElapsed
float EngineBase::smTimeElapsed;

// GLOBAL: SWBF2 0x007e3440
// EngineBase::smSpeakerConfig
EngineBase::SpeakerConfig EngineBase::smSpeakerConfig = speakerConfig_Stereo;

// SYNTHETIC: SWBF2 0x0040b760
// `dynamic initializer for 'Snd::EngineBase::smVoiceVirtualManager''
// SYNTHETIC: SWBF2 0x0076a210
// `dynamic atexit destructor for 'Snd::EngineBase::smVoiceVirtualManager''

// GLOBAL: SWBF2 0x007e3450
// EngineBase::smVoiceVirtualManager
VoiceVirtualManager EngineBase::smVoiceVirtualManager;

// SYNTHETIC: SWBF2 0x0040b750
// `dynamic initializer for 'Snd::EngineBase::smSoundRAMAllocator''

// GLOBAL: SWBF2 0x01e2968c
// EngineBase::smSoundRAMAllocator
MemoryBitmap EngineBase::smSoundRAMAllocator;



// SYNTHETIC: SWBF2 0x0040b720
// `dynamic initializer for 'Snd::EngineBase::smListeners''
// SYNTHETIC: SWBF2 0x0076a1f0
// `dynamic atexit destructor for 'Snd::EngineBase::smListeners''

// GLOBAL: SWBF2 0x01e296a8
// EngineBase::smListeners
Listener EngineBase::smListeners[4];

// FUNCTION: SWBF2 0x00733ea0
template <>
void Allocator<VoiceBase>::operator delete(void* p, void* pMemory, bool bTemp)
{
    (void)pMemory;
    (void)bTemp;

    if (!p)
    {
        return;
    }

    switch (((VoiceBase*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:

        break;
    }
}

template <>
void Allocator<VoiceBase>::operator delete(void* p, unsigned int uiSize)
{
    (void)uiSize;
    switch (((VoiceBase*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00734050
Listener* EngineBase::GetFreeListener()
{
    for (unsigned int i = 0; i < 4; i++)
    {
        if (!smListeners[i].Enabled())
        {
            return &smListeners[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x007340b0
Stream* EngineBase::GetOpenStream(unsigned int uiHashID)
{
    Stream* pResult = 0;
    for (unsigned int i = 0; i < sizeof(smStreamStorage) / sizeof(Stream); ++i)
    {
        Stream* const pStream = &smStreams[i];
        if (pStream->mInfoList.IsEmpty())
        {
            continue;
        }
        for (StreamBase::InfoItor it = pStream->GetInfoItor(); const StreamBase::Info* pInfo = it.GetInfo(); ++it)
        {
            if (pInfo->mHashID == uiHashID)
            {
                pResult = &smStreams[i];
                if (pResult->GetPlayState() == SourceTransport::playState_stopped)
                {
                    return pResult;
                }
            }
        }
    }
    return pResult;
}

// FUNCTION: SWBF2 0x00734160
VoiceVirtualManager* EngineBase::GetVoiceVirtualManager(bool)
{
    return &smVoiceVirtualManager;
}

// FUNCTION: SWBF2 0x00734560
void EngineBase::SetSpeakerConfig(SpeakerConfig eConfig)
{
    if (eConfig > speakerConfig_AnalogSurround)
    {
        smSpeakerConfig = speakerConfig_AnalogSurround;
        return;
    }

    smSpeakerConfig = eConfig < speakerConfig_Mono ? speakerConfig_Mono : eConfig;
}

// FUNCTION: SWBF2 0x00734400
bool EngineBase::Open()
{

    smOpened = true;
    smVoices = (Voice*)smVoiceStorage;

    for (unsigned int uiOffset = 0; uiOffset < 0xA800; uiOffset += sizeof(Voice))
    {
        void* const pMemory = (unsigned char*)smVoices + uiOffset;
        Voice* const pVoice = new (pMemory, false) Voice;

        smVoiceVirtualManager.mVoiceList.AddLast(&pVoice->mNode);
        pVoice->mManager = &smVoiceVirtualManager;
        ++smVoiceVirtualManager.mNumManagedVoices;
    }

    smStreams = (Stream*)smStreamStorage;

    for (unsigned int uiOffset = 0; uiOffset < 0x1446A68; uiOffset += sizeof(Stream))
    {
        void* const pMemory = (unsigned char*)smStreams + uiOffset;
        new (pMemory, false) Stream;
    }

    Sound::Open();
    SoundStream::Init();
    Space::Open();
    SoundLayered::Open();

    smTimeElapsed = 0.0f;

    return true;
}

// FUNCTION: SWBF2 0x00734590
void EngineBase::Update(float fDeltaTime, bool b)
{
    if (fDeltaTime > 0.0f)
    {
        smTimeElapsed += fDeltaTime;
        if (b)
        {
            for (unsigned int i = 0; i < 4; i++)
            {
                if (smListeners[i].Enabled())
                {
                    smListeners[i].Update();
                }
            }

            for (unsigned int i = 0; i < sizeof(smVoiceStorage) / sizeof(Voice); ++i)
            {
                smVoices[i].Update();
            }

            for (unsigned int i = 0; i < sizeof(smStreamStorage) / sizeof(Stream); ++i)
            {
                smStreams[i].Update(fDeltaTime);
            }

            StreamBase::Info::UpdateAll(fDeltaTime);
        }
        Bus::UpdateAll(fDeltaTime);
        SoundParameterized::UpdateAll(fDeltaTime);
        SoundLayered::UpdateAll(fDeltaTime);
        Sound::UpdateAll(fDeltaTime);
        SoundStream::UpdateAll();
        smVoiceVirtualManager.Update(fDeltaTime);
    }
}

}

namespace Snd
{



}

void __cdecl PblTraceP(const char* Format, ...);

namespace Snd
{

// FUNCTION: SWBF2 0x00734170
bool EngineBase::LoadConfiguration(PblFileChunk& chunk, char const* pcName, bool bLoadBanks,
                                   CallbackOverride const* pOverrides,
                                   unsigned int uiNumOverrides)
{
    PblFileChunk parent = chunk;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();

        CallbackOverride const* pOverride = 0;
        if (pOverrides)
        {
            for (unsigned int i = 0; i < uiNumOverrides; ++i)
            {
                if (pOverrides[i].chunkID == child._iId)
                {
                    pOverride = &pOverrides[i];
                    break;
                }
            }
        }
        if (pOverride)
        {
            pOverride->callback(child, pcName, pOverride->callbackData);
            continue;
        }

        switch (child._iId)
        {
        case 0x5F646E73u:
            LoadConfiguration(child, 0, true, 0, 0);
            break;

        case 0x5E262256u:
        case 0x9A644D15u:
            if (bLoadBanks)
            {
                SampleBankBase::LoadBank(child, pcName, true, 0);
            }
            break;

        case 0x454D414Eu:
        case 0x5BB97F21u:
            break;

        case 0x41544144u:
        {
            const unsigned int uiType = child._pFile->Read32();

            pOverride = 0;
            if (pOverrides)
            {
                for (unsigned int i = 0; i < uiNumOverrides; ++i)
                {
                    if (pOverrides[i].chunkID == uiType)
                    {
                        pOverride = &pOverrides[i];
                        break;
                    }
                }
            }
            if (pOverride)
            {
                pOverride->callback(parent, pcName, pOverride->callbackData);
                chunk = parent;
                continue;
            }

            switch (uiType)
            {
            case 0x5FD56DF9u:
                Bus::Load(parent, true);
                break;
            case 0x3553E285u:
                Space::Load(parent, true);
                break;
            case 0x2E93EF4Cu:
            case 0x4CA38B31u:
                SoundStream::Properties::Load(parent, true);
                break;
            case 0x0E0D9594u:
            case 0xEE65C519u:
                Sound::Properties::Load(parent, true);
                break;
            case 0x12EBD555u:
                SoundParameterized::Properties::Load(parent, true);
                break;
            case 0xA54A2138u:
                SoundLayered::Properties::Load(parent, true);
                break;
            case 0xE657A914u:
                ParameterGraph::Load(parent, true);
                break;
            case 0xABD36816u:
                I3DL2ReverbPreset::Load(parent, true);
                break;
            default:
                return false;
            }
            chunk = parent;
            break;
        }

        default:
            PblTraceP("Snd::EngineBase::LoadConfiguration : Skipping chunk %c%c%c%c (0x%08x)\n",
                      child._iId & 0xFF, (child._iId >> 8) & 0xFF, (child._iId >> 16) & 0xFF,
                      child._iId >> 24, child._iId);
            break;
        }
    }

    return true;
}

// FUNCTION: SWBF2 0x00733fa0
void EngineBase::DestroyAll()
{
    SoundParameterized::Properties::DestroyAll();
    ParameterGraph::DestroyAll();
    SoundLayered::Properties::DestroyAll();
    Sound::Properties::DestroyAll();
    SoundStream::Properties::DestroyAll();
    Space::DestroyAll();
    I3DL2ReverbPreset::DestroyAll();
    SampleBankBase::DestroyAllBanks();

    for (unsigned int i = 0; i < sizeof(smStreamStorage) / sizeof(Stream); ++i)
    {
        smStreams[i].SourceBus::SetBus(&Bus::smRootBus);
    }

    Bus::smRootBus.DestroySubBuses();
}

// FUNCTION: SWBF2 0x00733ed0
void EngineBase::Close()
{
    DestroyAll();
    SoundLayered::Close();
    Space::Close();
    SoundStream::Init();
    Sound::Close();

    for (unsigned int i = 0; i < sizeof(smVoiceStorage) / sizeof(Voice); ++i)
    {
        if (smVoices[i].mNode.mNext)
        {
            smVoices[i].RemoveFromManager();
        }
        delete &smVoices[i];
    }
    smVoices = 0;

    for (unsigned int i = 0; i < sizeof(smStreamStorage) / sizeof(Stream); ++i)
    {
        delete &smStreams[i];
    }
    smStreams = 0;
    smOpened = false;
}

// FUNCTION: SWBF2 0x00734010
void __cdecl SoundLayered::Properties::DestroyAll()
{
    while (!smPropertyList.IsEmpty())
    {
        delete (Properties*)((char*)smPropertyList.mHead.mNext - offsetof(Properties, mNode));
    }
}

// FUNCTION: SWBF2 0x00734130
Stream* EngineBase::GetStream(unsigned int uiIndex)
{
    return &smStreams[uiIndex];
}

// FUNCTION: SWBF2 0x00734830
Stream::~Stream()
{
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x00734080
Stream* EngineBase::GetFreeStream()
{
    for (unsigned int i = 0; i < 6; ++i)
    {
        if (!smStreams[i].IsOpen())
        {
            return &smStreams[i];
        }
    }
    return 0;
}
}

namespace Snd
{

// FUNCTION: SWBF2 0x00734150
float EngineBase::GetTimeElapsed()
{
    return smTimeElapsed;
}
}
