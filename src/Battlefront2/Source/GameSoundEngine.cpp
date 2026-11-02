#include <new>
#include <stdlib.h>

#include <windows.h>

#include "GameSoundEngine.h"
#include "GameMemory.h"
#include "GameState.h"
#include "GameVoiceChat.h"
#include "LoadUtil.h"
#include "Music.h"
#include "NetSetup.h"
#include "PlayerProfile.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/sndfreelist.h"
#include "SoundFL/source/sndi3dl2reverb.h"
#include "SoundFL/source/sndsoundlayered.h"
#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndsoundstream.h"
#include "SoundFL/source/win/sndengine.h"
#include "SoundFL/source/win/sndstream.h"
#include "SoundFL/source/sndsoundparameterized.h"
#include "EntitySound.h"
#include "FoleyFX.h"
#include "LuaCallbacks_Sound.h"
#include "VehicleEngine.h"
#include "TriggerSoundRegion.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatDecoder.h"
#include "GameVoiceChatEncoder.h"
#include "VoiceChatTransport.h"
#include "GameVoiceChatTransport.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblTime.h"
#include "GameLoop.h"
#include "GameMusic.h"

// GLOBAL: SWBF2 0x01faa688

static unsigned __int64 sLastSoundTick;
// GLOBAL: SWBF2 0x01faa684

static float sAccumulatedSoundTime;

namespace FrameUpdate
{
extern float fOneOverTicksPerSecond;
}
extern int g_InGameMoviePlayerState;
namespace Lua_Callbacks
{
extern bool sGameMusicRetrigger;
}
namespace Lua_Callbacks
{
extern unsigned int sGameMusicCurDelayTime;
}
namespace Lua_Callbacks
{
extern void* sGameMusic;
}
namespace Lua_Callbacks
{
extern unsigned int sGameMusicDelayTime;
}

extern HWND hGlobalWnd;
namespace GameSoundEngine
{

// GLOBAL: SWBF2 0x007e68e4
int gSamplerate = 48000;
}
namespace GameSoundEngine
{
// GLOBAL: SWBF2 0x007e68e8
int gMaxVoices = 32;
}
namespace GameSoundEngine
{
// GLOBAL: SWBF2 0x007e68ec
float gSoftMixerBufferSizeMs = 100.0f;
}

// GLOBAL: SWBF2 0x007e68f0
bool gCentreSpeakerEnhancement = true;

namespace GameSoundEngine
{
extern bool sHwMixingSupported;
}
namespace GameSoundEngine
{
extern bool sHwEffectsSupported;
}

namespace GameVoiceChat
{
bool Open(float fSeconds, bool bSoundWorking);
}

// GLOBAL: SWBF2 0x01e56282
bool gSoundEnabled;

namespace GameSoundEngine
{

// SYNTHETIC: SWBF2 0x00403df0
// GameSoundEngine::`dynamic initializer for 'gSoundHeap''

// GLOBAL: SWBF2 0x01ec3238
// gSoundHeap
int gSoundHeap = GameMemory::RunTimeHeap;

// GLOBAL: SWBF2 0x01e56283
bool gSoundIsOpen;
// GLOBAL: SWBF2 0x01e56398
unsigned long gWindowsSpeakerConfig;

// GLOBAL: SWBF2 0x01e56394
Snd::EngineBase::SpeakerConfig gDefaultSpeakerConfig;

// FUNCTION: SWBF2 0x005390e0
void SetHeap(int iHeap)
{
    gSoundHeap = iHeap;
}

}

// FUNCTION: SWBF2 0x005387a0
void* __cdecl SndAlloc(unsigned int uiSize)
{
    return RedAllocFromHeap(GameSoundEngine::gSoundHeap, uiSize, 0);
}

// FUNCTION: SWBF2 0x005387c0
void __cdecl SndFree(void* pBlock)
{
    RedFreeToHeap(GameSoundEngine::gSoundHeap, pBlock, 0);
}

// FUNCTION: SWBF2 0x005387e0
void* __cdecl SndTempAlloc(unsigned int uiSize)
{
    return RedAllocFromHeap(GameMemory::RunTimeHeap, uiSize, 0);
}

// FUNCTION: SWBF2 0x00538800
void __cdecl SndTempFree(void* pBlock)
{
    RedFreeToHeap(GameMemory::RunTimeHeap, pBlock, 0);
}

// FUNCTION: SWBF2 0x00538820
void __cdecl PrintDebugString(char const*)
{
}

namespace GameSoundEngine
{

// FUNCTION: SWBF2 0x00538830
bool __cdecl UseKmixer()
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        switch (pProfile->mSoundOptions.mPlaybackMode)
        {
        case 1:
        case 2:
        case 3:
        case 4:
            return false;
        default:
            return true;
        }
    }
    return false;
}
}

namespace GameSoundEngine
{

// FUNCTION: SWBF2 0x00538850
bool Init(unsigned int uiFlags, bool bEnable)
{
    if (gSoundIsOpen)
    {
        return true;
    }

    Snd::Globals::smAlloc = SndAlloc;
    Snd::Globals::smFree = SndFree;
    Snd::Globals::smAllocPerm = malloc;
    Snd::Globals::smFreePerm = free;
    Snd::Globals::smTempAlloc = malloc;
    Snd::Globals::smTempFree = free;
    Snd::Globals::smWarningFunction = PrintDebugString;

    if (!g_bNoSound)
    {
        bool bSoftwareMixing = !GameSoundEngine::UseKmixer();

        unsigned long* pulWindowsSpeakerConfig =
            (gWindowsSpeakerConfig != 0) ? 0 : &gWindowsSpeakerConfig;

        Snd::Engine::MixConfig eMixConfig =
            (uiFlags != 0) ? (Snd::Engine::MixConfig)uiFlags : Snd::Engine::mixConfig_Software;

        gSoundIsOpen = Snd::Engine::Open(hGlobalWnd, eMixConfig, bEnable, GameSoundEngine::gSoftMixerBufferSizeMs,
                                         bSoftwareMixing, (unsigned int)GameSoundEngine::gSamplerate,
                                         (unsigned int)GameSoundEngine::gMaxVoices,
                                         pulWindowsSpeakerConfig, 0);

        if (uiFlags == Snd::Engine::mixConfig_DirectSoundHardware &&
            Snd::Engine::GetMixConfig() != Snd::Engine::mixConfig_DirectSoundHardware)
        {
            GameSoundEngine::sHwMixingSupported = false;
            if (bEnable)
            {
                GameSoundEngine::sHwEffectsSupported = false;
            }
        }

        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();

        if (gDefaultSpeakerConfig == Snd::EngineBase::speakerConfig_Invalid)
        {

            gDefaultSpeakerConfig = Snd::EngineBase::smSpeakerConfig;
            if (pProfile)
            {
                pProfile->mSoundOptions.mPlaybackMode =
                    (unsigned char)Snd::EngineBase::smSpeakerConfig;
            }
        }

        if (pProfile)
        {
            Snd::Engine::SetSpeakerConfig(
                (Snd::EngineBase::SpeakerConfig)pProfile->mSoundOptions.mPlaybackMode);

            pProfile->mSoundOptions.mPlaybackMode =
                (unsigned char)Snd::EngineBase::smSpeakerConfig;
        }
    }
    else
    {
        gSoundIsOpen = Snd::Engine::Open(0, Snd::Engine::mixConfig_Disabled, true, 100.0f,
                                         true, 48000, 32, 0, 0);
    }

    bool bSoundWorking = (Snd::Engine::GetMixConfig() != Snd::Engine::mixConfig_Disabled);

    if (gSoundIsOpen)
    {
        GameVoiceChat::Open(20.0f, bSoundWorking);
    }

    gSoundHeap = GameMemory::RunTimeHeap;
    return gSoundIsOpen;
}

}


namespace GameSoundEngine
{

// FUNCTION: SWBF2 0x00538a20
void Create()
{

    gSoundHeap = GameMemory::RunTimeHeap;

    for (unsigned int i = 0; i < 4; ++i)
    {
        Snd::EngineBase::smListeners[i].mDopplerFactor = 1.0f;
    }

    Snd::SoundStream::Init();

    Snd::SoundParameterized::Alloc();
}

}

extern int sVoiceChatHeap;
namespace GameVoiceChat
{
extern bool gPlayerChange;
}
extern void* sVoiceChatEncoders[1];
namespace GameVoiceChat
{
extern void* gNodes[1];
}
extern void* sVoiceChatDecoders[1];
extern void* sEncoderStorage;
extern void* sDecoderStorage;
extern void* sNodeStorage;

namespace GameVoiceChat
{
unsigned int GetNumNodes();
}

extern int netVoiceMode;

namespace GameVoiceChat
{

// FUNCTION: SWBF2 0x0053c040
bool Open(float fSeconds, bool bSoundWorking)
{
    sVoiceChatHeap = GameMemory::ApplicationHeap;
    int iOldHeap = RedSetCurrentHeap(GameMemory::ApplicationHeap);

    if (GameVoiceChat::gTransport.Open(0x32, 1))
    {
        if (VoiceChat::Node::OpenStatic(&GameVoiceChat::gTransport, fSeconds, 1000.0f, 1000.0f))
        {
            unsigned int uiNodes = (unsigned int)GameVoiceChat::GetNumNodes();
            bool bAllOpen = true;

            for (unsigned int i = 0; i < uiNodes; ++i)
            {
                sVoiceChatEncoders[i] = new ((GameVoiceChat::Encoder*)sEncoderStorage + i) GameVoiceChat::Encoder;
                sVoiceChatDecoders[i] = new ((GameVoiceChat::Decoder*)sDecoderStorage + i) GameVoiceChat::Decoder;
                GameVoiceChat::gNodes[i] = new ((GameVoiceChat::Node*)sNodeStorage + i)
                    GameVoiceChat::Node((GameVoiceChat::Encoder*)sVoiceChatEncoders[i],
                                        (GameVoiceChat::Decoder*)sVoiceChatDecoders[i]);
            }

            GameVoiceChat::gPlayerChange = true;

            VoiceChat::Transport::Method cMethod(
                (VoiceChat::Transport::Method::Type)(netVoiceMode ? netVoiceMode : 2), 0x32);
            cMethod.SetNumRelayNodes(cMethod.CalculateOptimalRelayNodes(0x32));
            GameVoiceChat::gTransport.SetMethod(cMethod, true);

            if (!bSoundWorking)
            {
                RedSetCurrentHeap(iOldHeap);
                return true;
            }

            unsigned int i;

            for (i = 0; i < uiNodes; ++i)
            {
                if (!bAllOpen)
                {
                    break;
                }
                bAllOpen &= ((VoiceChat::Node*)GameVoiceChat::gNodes[i])->Open(0xFF, 0xFF, 0x0C);
            }

            if (bAllOpen)
            {
                for (i = 0; i < uiNodes; ++i)
                {
                    ((GameVoiceChat::Node*)GameVoiceChat::gNodes[i])->SetSendMask(GameVoiceChat::Transport::mask_TeamFriend, true, false);
                    ((GameVoiceChat::Node*)GameVoiceChat::gNodes[i])->SetReceiveMask(GameVoiceChat::Transport::mask_Everyone, true, false);
                }
                RedSetCurrentHeap(iOldHeap);
                return true;
            }

            for (i = 0; i < uiNodes; ++i)
            {
                if (((VoiceChat::Node*)GameVoiceChat::gNodes[i])->IsOpen())
                {
                    ((VoiceChat::Node*)GameVoiceChat::gNodes[i])->Close();
                }
            }

            for (i = 0; i < uiNodes; ++i)
            {
                ((GameVoiceChat::Encoder*)sVoiceChatEncoders[i])->~Encoder();

                ((GameVoiceChat::Decoder*)sVoiceChatDecoders[i])->~Decoder();

                sVoiceChatEncoders[i] = 0;
                sVoiceChatDecoders[i] = 0;
                GameVoiceChat::gNodes[i] = 0;
            }

            VoiceChat::Node::CloseStatic();
        }
        GameVoiceChat::gTransport.Close();
    }
    RedSetCurrentHeap(iOldHeap);
    return false;
}
}

class SoundSpaceRegion
{
public:
    static void DestroyAll();
};

namespace GameSoundEngine
{

// GLOBAL: SWBF2 0x007ec1dc
BusGains gBusGainsDefault;
// GLOBAL: SWBF2 0x007ec1f8
BusGains gBusGainsUser;

// GLOBAL: SWBF2 0x01e5639c
bool gMixConfigChanged;

// GLOBAL: SWBF2 0x01e5639d
bool gDisableStartupMusic;

// FUNCTION: SWBF2 0x005390f0
int GetHeap()
{
    return gSoundHeap;
}

// FUNCTION: SWBF2 0x00539100
void DisableStartupMusic()
{
    gDisableStartupMusic = true;
}

// FUNCTION: SWBF2 0x00538ba0
bool MixConfigChanged()
{
    bool bChanged = gMixConfigChanged;
    gMixConfigChanged = false;
    return bChanged;
}

// FUNCTION: SWBF2 0x005389e0
void Term(bool bFlag)
{
    if (gSoundIsOpen)
    {
        if (GameVoiceChat::IsOpen())
        {
            GameVoiceChat::Close();
        }
        Destroy();
        Snd::Engine::Close(gWindowsSpeakerConfig, bFlag);
        gSoundIsOpen = false;
        gMixConfigChanged = false;
    }
}

// FUNCTION: SWBF2 0x00538a60
void Destroy()
{
    VehicleEngineClass::Boost::DestroyAll();
    EntitySound::DestroyAll();
    Music::DestroyAll();
    VehicleEngine::DeactivateAll();
    StopAll();
    FoleyFXGroup::DestroyAll();
    FoleyFX::DestroyAll();

    for (unsigned int i = 0; i < 6; i++)
    {
        Snd::Stream* pStream = &Snd::EngineBase::smStreams[i];
        pStream->SetBus(0);
        if (!pStream->mInfoList.IsEmpty())
        {
            pStream->Close();
        }
        if (pStream->GetLocked())
        {
            pStream->Lock(false);
        }
    }

    Snd::Sound::RemoveAll();
    Snd::SoundLayered::RemoveAll();
    Snd::SoundParameterized::RemoveAll();
    Snd::SoundParameterized::Free();
    SoundSpaceRegion::DestroyAll();
    TriggerSoundRegion::DestroyAll();

    for (unsigned int i = 0; i < 4; i++)
    {
        Snd::EngineBase::smListeners[i].SetSpace(0);
    }
    Snd::Engine::Update(0.1f);
    for (unsigned int i = 0; i < 4; i++)
    {
        Snd::EngineBase::smListeners[i].Enable(false);
    }

    Snd::EngineBase::DestroyAll();
    Lua_Callbacks::InitDelayedSounds();
    Snd::Stream::EnableStreaming(true);
}

// FUNCTION: SWBF2 0x00538bb0
bool ChangeMixConfig(unsigned int uiFlags, bool bEnable)
{
    if (GameState::GetState() == 0x11E1FC01)
    {
        if (uiFlags != 2 ||
            ((GameSoundEngine::sHwMixingSupported || GameSoundEngine::UseKmixer()) &&
             (!bEnable || GameSoundEngine::sHwEffectsSupported)))
        {
            Term(false);
            if (Init(uiFlags, bEnable))
            {
                unsigned int auiChunks[2] = { 0x5E262256, 0x5F646E73 };
                Snd::EngineBase::smListeners[0].Enable(true);
                LoadUtil::ReadDataFileOnHeap("core.lvl", -1, auiChunks, 2, 0, 0, 0);
                gMixConfigChanged = true;
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00538c60
void StopAll()
{
    for (unsigned int i = 0; i < 6; i++)
    {
        Snd::Stream* pStream = Snd::EngineBase::GetStream(i);
        if (pStream->GetPlayState() == Snd::SourceTransport::playState_playing)
        {
            pStream->SourceBus::Stop(pStream);
            pStream->SourceTransport::Stop();
        }
    }

    Snd::Sound::StopAll();
    Snd::SoundLayered::FireForgetLayer::StopAll();
    Music::StopAll();
    Snd::Engine::Update(0.00001f);

    Snd::I3DL2Reverb reverb;
    reverb.mFlags |= Snd::I3DL2Reverb::flags_roomGainChanged;
    reverb.mRoomGain = 0.0f;
    Snd::Engine::SetGlobalReverb(&reverb);
}

// FUNCTION: SWBF2 0x00538e50
void UpdateBusGains(float fSFX, float fMusic, float fVoiceOver, float fBattleVO, float fMaster)
{
    gBusGainsUser.mSFX = fSFX;
    gBusGainsUser.mMusic = fMusic;
    gBusGainsUser.mVoiceOver = fVoiceOver;
    gBusGainsUser.mBattleVO = fBattleVO;
    gBusGainsUser.mMaster = fMaster;

    Snd::Bus* pBus;
    if ((pBus = Snd::Bus::FindByHashID(0x3b13aa4b)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mVoiceOver * gBusGainsUser.mVoiceOver, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0x74d771fc)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mBattleVO * gBusGainsUser.mBattleVO, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0xeceb308a)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mSFX * gBusGainsUser.mSFX, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0x9f9c4fd4)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mMusic * gBusGainsUser.mMusic, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0xaebbad4d)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mAmbience * gBusGainsUser.mSFX, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0x82c7f467)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mShellFX * gBusGainsUser.mSFX, false);
    }
    Snd::Bus::smRootBus.SetGain(gBusGainsUser.mMaster, false);
}

// FUNCTION: SWBF2 0x00538fc0
void InitializeBusGains()
{
    Snd::Bus* pBus;
    if ((pBus = Snd::Bus::FindByHashID(0x3b13aa4b)) != 0)
    {
        gBusGainsDefault.mVoiceOver = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0x74d771fc)) != 0)
    {
        gBusGainsDefault.mBattleVO = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0xeceb308a)) != 0)
    {
        gBusGainsDefault.mSFX = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0x9f9c4fd4)) != 0)
    {
        gBusGainsDefault.mMusic = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0xaebbad4d)) != 0)
    {
        gBusGainsDefault.mAmbience = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0x82c7f467)) != 0)
    {
        gBusGainsDefault.mShellFX = pBus->mGain;
    }
    gBusGainsDefault.mMaster = Snd::Bus::smRootBus.mGain;

    if (gDisableStartupMusic)
    {
        gBusGainsDefault.mMusic = 0.0f;
        gBusGainsUser.mMusic = 0.0f;
        gDisableStartupMusic = false;
    }

    UpdateBusGains(gBusGainsUser.mSFX, gBusGainsUser.mMusic, gBusGainsUser.mVoiceOver,
                   gBusGainsUser.mBattleVO, gBusGainsUser.mMaster);
}

}

// FUNCTION: SWBF2 0x00538d00
void GameSoundEngine::Update(float fDeltaTime)
{
    if (NetGame::IsEnabled() && NetGame::IsDedicated())
    {
        return;
    }
    if (!Snd::EngineBase::smOpened)
    {
        return;
    }

    // The sound engine runs on its own clock: the frame's delta time is replaced by the time since the last sound update.
    const unsigned __int64 uiNow = PblTime::GetTickCount();
    const int iTicks = (int)((unsigned int)(uiNow - sLastSoundTick) & 0x7fffffff);
    fDeltaTime = (float)iTicks * FrameUpdate::fOneOverTicksPerSecond;
    sLastSoundTick = uiNow;

    sAccumulatedSoundTime += fDeltaTime;
    if (!(sAccumulatedSoundTime > 0.02f))
    {
        return;
    }
    sAccumulatedSoundTime = 0.0f;

    Snd::Engine::Update(fDeltaTime);

    if (g_InGameMoviePlayerState)
    {
        return;
    }
    if (GameState::GetState() == 0x8ff60339 && GameLoop::IsGameOver())
    {
        return;
    }

    Music::UpdateAll(fDeltaTime);
    Lua_Callbacks::UpdateDelayedSounds(fDeltaTime);

    if (Lua_Callbacks::sGameMusicRetrigger)
    {
        float& fCountdown = *(float*)&Lua_Callbacks::sGameMusicCurDelayTime;
        if (fCountdown > 0.0f)
        {
            fCountdown -= fDeltaTime;
        }
        else if (Lua_Callbacks::sGameMusic)
        {
            GameMusic* pMusic = (GameMusic*)Lua_Callbacks::sGameMusic;
            if (pMusic->GetCurrentPlayState() == GameMusic::playState_stopped)
            {
                fCountdown = *(float*)&Lua_Callbacks::sGameMusicDelayTime;
                pMusic->Play();
            }
        }
    }
}

// GLOBAL: SWBF2 0x007e2e20

float sDefaultSoundDamping = 0.95f;


// SYNTHETIC: SWBF2 0x00403e00
// `dynamic initializer for 'sSoundDamping''

// GLOBAL: SWBF2 0x01ec323c
// sSoundDamping
float sSoundDamping = sDefaultSoundDamping;
namespace GameSoundEngine
{

// GLOBAL: SWBF2 0x007e666e
bool sHwMixingSupported = true;
}
namespace GameSoundEngine
{

// GLOBAL: SWBF2 0x007e666f
bool sHwEffectsSupported = true;
}
