#include "GameSound.h"

#include <string.h>

#include "GameLoop.h"
#include "GameMemory.h"
#include "GameState.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "SoundFL/source/sndsound.h"
#include "SoundFL/source/sndsoundlayered.h"
#include "SoundFL/source/sndsoundstream.h"
#include "SoundFL/source/sndvoicevirtual.h"

#include "NetGame.h"
#include "SkyDome.h"
#include "SkyStars.h"
#include "WeatherEffect.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/PC/pcRedIndexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Interface/RedBorderElement.h"
#include "RedEngineFL/Interface/RedHotSpot.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "LuaCallbacks_PC.h"
#include "LuaHelper.h"
#include "LuaCallbacks_Net.h"
#include "FLInputManager.h"
#include "PlayerProfile.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Interface/Red3DModelAnimator.h"
#include "RedEngineFL/Interface/Red3DModelElement.h"
#include "RedEngineFL/movie/RedFileAsync.h"
#include "SoundFL/source/sndstreamimaadpcmdecoder.h"
#include "BoundaryManager.h"
#include "CommandPost.h"
#include "EndDisplay.h"
#include "EventManager_CommandPost.h"
#include "EventManager_Object.h"
#include "EventManager_Team.h"
#include "FLEffect.h"
#include "GameObject.h"
#include "WeaponTowCable.h"
#include "HUDElementBarBitmap.h"
#include "HUDElementMap.h"
#include "HUDElementText.h"
#include "HUDEvent.h"
#include "HUDProceduralBarBitmap.h"
#include "LeafEffect.h"
#include "LoadUtil.h"
#include "LuaCallbacks_Sound.h"
#include "OrdnanceBeam.h"
#include "StatusDisplay.h"
#include "TargetManager.h"
#include "Team.h"
#include "VOHelper.h"
#include "VoiceOverManager.h"
#include "LuaFL/include/lauxlib.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndstreambase.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "SoundFL/source/win/sndengine.h"
#include "SoundFL/source/win/sndstream.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "FLJoystick.h"

extern bool gSoundEnabled;


// GLOBAL: SWBF2 0x01e56390
GameSound** GameSound::sDeferredSounds;
// GLOBAL: SWBF2 0x01e5638c
unsigned int GameSound::sNumDeferredSounds;

// FUNCTION: SWBF2 0x00537f00
void GameSound::InitDeferred()
{
    sDeferredSounds = (GameSound**)RedAllocFromHeap(GameMemory::TempLoadHeap, 2000, 0);
    sNumDeferredSounds = 0;
}

// FUNCTION: SWBF2 0x00537f30
void GameSound::SetupDeferred()
{
    if (!sDeferredSounds)
    {
        return;
    }

    for (unsigned int i = 0; i < sNumDeferredSounds; ++i)
    {
        sDeferredSounds[i]->SetID(sDeferredSounds[i]->GetPropertiesHashID(), false, false);
    }
    RedFreeToHeap(GameMemory::TempLoadHeap, sDeferredSounds, 0);
    sDeferredSounds = 0;
    sNumDeferredSounds = 0;
}

// FUNCTION: SWBF2 0x00537f90
void GameSound::AddDeferred(GameSound* const pSound)
{
    if (sDeferredSounds && sNumDeferredSounds < 500)
    {
        sDeferredSounds[sNumDeferredSounds++] = pSound;
    }
}

// FUNCTION: SWBF2 0x00537fb0
GameSound::GameSound(bool bDeferred)
{
    (void)bDeferred;
    mProps = 0;
    mType = 0;
}

// FUNCTION: SWBF2 0x00537fc0
GameSound::~GameSound()
{
}

// FUNCTION: SWBF2 0x00537fd0
bool GameSound::CanPlay() const
{
    if (!gSoundEnabled)
    {
        return false;
    }
    if (GameState::GetState() != 0x11E1FC01)
    {
        if (GameLoop::IsGameOver() && mType == type_soundStream)
        {
            return false;
        }
        if (!NetGame::IsOnceClientTurn())
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00538010
void GameSound::Play(const PblVector3* pPos, const PblVector3* pVel, bool bArg3,
                     bool bArg4) const
{
    (void)bArg3;
    while (CanPlay() && mProps)
    {
        switch (mType)
        {
        case type_sound:
        {
            const Snd::Sound::Properties* pProps =
                (const Snd::Sound::Properties*)mProps;
            if (pProps->Get3DMode())
            {
                Snd::Source3D source3D;
                source3D.SetPosition(pPos);
                source3D.SetVelocity(pVel);
                Snd::Sound::Play(&source3D, pProps, 0, 0, true);
            }
            else
            {
                Snd::Sound::Play(0, pProps, 0, 0, true);
            }
            return;
        }
        case type_soundLayered:
        {
            const Snd::SoundLayered::Properties* pProps =
                (const Snd::SoundLayered::Properties*)mProps;
            Snd::Source3D source3D;
            source3D.SetPosition(pPos);
            source3D.SetVelocity(pVel);
            Snd::SoundLayered::Play(&source3D, pProps);
            return;
        }
        case type_soundStream:
        {
            const Snd::SoundStream::Properties* pProps =
                (const Snd::SoundStream::Properties*)mProps;
            if (pProps->Get3DMode())
            {
                Snd::Source3D source3D;
                source3D.SetPosition(pPos);
                source3D.SetVelocity(pVel);
                Snd::SoundStream::Play(pProps, &source3D, bArg4);
            }
            else
            {
                Snd::SoundStream::Play(pProps, bArg4);
            }
            return;
        }
        case type_ID:
            const_cast<GameSound*>(this)->SetID((unsigned int)mProps, false, false);
            if (mType == type_ID)
            {
                return;
            }
            bArg4 = true;
            break;
        default:
            return;
        }
    }
}

// FUNCTION: SWBF2 0x005381c0
void GameSound::SetID(const char* pName, bool, bool bDeferred)
{
    char buffer[512];
    char* pModifier = buffer;
    mProps = 0;
    mType = type_invalid;
    if (!pName || !strlen(pName))
    {
        return;
    }

    strcpy(buffer, pName);
    while (*pModifier != ' ' && *pModifier)
    {
        ++pModifier;
    }
    if (*pModifier == ' ')
    {
        *pModifier++ = 0;
    }

    unsigned int uID = PblHash(buffer);
    if ((strlen(pModifier) && PblHash(pModifier) == 0x1A01FB3B) || bDeferred)
    {
        mType = type_ID;
        mProps = (const void*)uID;
        AddDeferred(this);
    }
    else
    {
        SetID(uID, false, false);
    }
}

// FUNCTION: SWBF2 0x005382a0
void GameSound::SetID(unsigned int uID, bool, bool)
{
    if (!uID)
    {
        mProps = 0;
        return;
    }
    mProps = Snd::Sound::Properties::FindByHashID(uID);
    mType = mProps ? type_sound : type_invalid;
    if (mType == type_invalid)
    {
        mProps = Snd::SoundLayered::Properties::FindByHashID(uID);
        mType = mProps ? type_soundLayered : type_invalid;
    }
    if (mType == type_invalid)
    {
        mProps = Snd::SoundStream::Properties::FindByHashID(uID);
        mType = mProps ? type_soundStream : type_invalid;
    }
}

// FUNCTION: SWBF2 0x00538310
unsigned int GameSound::GetPropertiesHashID() const
{
    switch (mType)
    {
    case type_sound:
        return ((const Snd::Sound::Properties*)GetProperties())->mHashID;
    case type_soundLayered:
        return ((const Snd::SoundLayered::Properties*)GetProperties())->GetHashID();
    case type_soundStream:
        return ((const Snd::SoundStream::Properties*)GetProperties())->mHashID;
    case type_ID:
        return (unsigned int)GetProperties();
    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x00538350
float GameSound::GetMuteDistance() const
{
    if (mProps)
    {
        switch (mType)
        {
        case type_sound:
            return ((const Snd::Sound::Properties*)mProps)->mMuteDistance;
        case type_soundLayered:
        {
            const Snd::SoundLayered::Properties* pLayered = (const Snd::SoundLayered::Properties*)mProps;
            const PblListSingle::Node* pFirst = pLayered->mLayerList.GetFirst();
            pFirst = (pFirst == pLayered->mLayerList.GetTerminator()) ? 0 : pFirst;
            Snd::SoundLayered::Properties::Layer* pLayer = static_cast<Snd::SoundLayered::Properties::Layer*>(
                pFirst->GetObjectA(&pLayered->mLayerList, offsetof(Snd::SoundLayered::Properties::Layer, mNode)));
            return pLayer->mSoundProps->mMuteDistance;
        }
        case type_soundStream:
            return ((const Snd::SoundStream::Properties*)mProps)->mMuteDistance;
        }
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x005383a0
const void* GameSound::GetProperties() const
{
    return mProps;
}

// FUNCTION: SWBF2 0x005383b0
GameSound::Type GameSound::GetPropertiesType() const
{
    return (Type)mType;
}

// FUNCTION: SWBF2 0x005383c0
GameSound& GameSound::operator=(const GameSound& other)
{
    if (other.GetPropertiesType() == type_ID)
    {
        const_cast<GameSound&>(other).SetID((unsigned int)other.GetProperties(), false, false);
    }
    mProps = other.mProps;
    mType = other.mType;
    return *this;
}

// FUNCTION: SWBF2 0x00538400
float GameSound::GetGain() const
{
    switch (mType)
    {
    case type_sound:
        return ((const Snd::Sound::Properties*)mProps)->mGain;
    case type_soundLayered:
    {
        const Snd::SoundLayered::Properties* pLayered = (const Snd::SoundLayered::Properties*)mProps;
        const PblListSingle::Node* pFirst = pLayered->mLayerList.GetFirst();
        pFirst = (pFirst == pLayered->mLayerList.GetTerminator()) ? 0 : pFirst;
        Snd::SoundLayered::Properties::Layer* pLayer = static_cast<Snd::SoundLayered::Properties::Layer*>(
            pFirst->GetObjectA(&pLayered->mLayerList, offsetof(Snd::SoundLayered::Properties::Layer, mNode)));
        return pLayer->mSoundProps->mGain;
    }
    case type_soundStream:
        return ((const Snd::SoundStream::Properties*)mProps)->mGain;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x00538450
float GameSound::GetPitch() const
{
    switch (mType)
    {
    case type_sound:
        return ((const Snd::Sound::Properties*)mProps)->mPitch;
    case type_soundLayered:
    {
        const Snd::SoundLayered::Properties* pLayered = (const Snd::SoundLayered::Properties*)mProps;
        const PblListSingle::Node* pFirst = pLayered->mLayerList.GetFirst();
        pFirst = (pFirst == pLayered->mLayerList.GetTerminator()) ? 0 : pFirst;
        Snd::SoundLayered::Properties::Layer* pLayer = static_cast<Snd::SoundLayered::Properties::Layer*>(
            pFirst->GetObjectA(&pLayered->mLayerList, offsetof(Snd::SoundLayered::Properties::Layer, mNode)));
        return pLayer->mSoundProps->mPitch;
    }
    case type_soundStream:
        return ((const Snd::SoundStream::Properties*)mProps)->mPitch;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x005384a0
GameSoundControllable::GameSoundControllable()
{
    mVoiceVirtualHandle = 0;
    mFlags = 0;
}

// FUNCTION: SWBF2 0x005384b0
GameSoundControllable::~GameSoundControllable()
{
    if (mVoiceVirtualHandle)
    {
        ReleaseVoice();
    }
}

// FUNCTION: SWBF2 0x005384c0
void GameSoundControllable::Play(const GameSound* pSound, const PblVector3* pPos,
                                 const PblVector3* pVel, bool bArg)
{
    if (!pSound->CanPlay())
    {
        return;
    }
    const void* pProps = pSound->GetProperties();
    if (!pProps)
    {
        return;
    }

    Snd::Source3D source3D;
    source3D.SetPosition(pPos);
    source3D.SetVelocity(pVel);

    switch (pSound->GetPropertiesType())
    {
    case GameSound::type_sound:
    {
        const Snd::Sound::Properties* pSoundProps = (const Snd::Sound::Properties*)pProps;
        if (mVoiceVirtualHandle)
        {
            ReleaseVoice();
        }
        Snd::VoiceVirtual* pVoice =
            Snd::Sound::Play((pPos || pSoundProps->Get3DMode()) ? &source3D : 0,
                             pSoundProps, StolenCallback, this, true);
        mFlags = (unsigned char)(flags_playing
                                 | (pSoundProps->GetLooping() ? flags_looping : 0));
        mVoiceVirtualHandle =
            pVoice ? Snd::Sound::VoiceVirtualToVoiceVirtualHandle(pVoice) : 0;
        break;
    }
    case GameSound::type_soundLayered:
        Snd::SoundLayered::Play(pPos ? &source3D : 0,
                                (const Snd::SoundLayered::Properties*)pProps);
        mFlags = flags_playing;
        break;
    case GameSound::type_soundStream:
        Snd::SoundStream::Play((const Snd::SoundStream::Properties*)pProps, &source3D, bArg);
        mFlags = flags_playing;
        break;
    case GameSound::type_ID:
        const_cast<GameSound*>(pSound)->SetID((unsigned int)pProps, false, false);
        if (pSound->GetPropertiesType() != GameSound::type_ID)
        {
            Play(pSound, pPos, pVel, true);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00538630
Snd::VoiceVirtual* GameSoundControllable::ReleaseVoice()
{
    Snd::VoiceVirtual* pVoice = Snd::Sound::VoiceVirtualHandleToVoiceVirtual(mVoiceVirtualHandle);
    Snd::Sound::VoiceVirtualRelease(pVoice);
    mVoiceVirtualHandle = 0;
    return pVoice;
}

// FUNCTION: SWBF2 0x00538660
void GameSoundControllable::Stop(bool bArg)
{
    if (mVoiceVirtualHandle)
    {
        Snd::VoiceVirtual* pVoice = ReleaseVoice();
        if (bArg)
        {
            pVoice->Stop();
        }
    }
    mFlags = 0;
}

// FUNCTION: SWBF2 0x005386b0
void GameSoundControllable::Update(const GameSound* pSound, const PblVector3* pPos, const PblVector3* pVel)
{
    if (!pSound->CanPlay() || !GetPlaying())
    {
        return;
    }

    if (!mVoiceVirtualHandle)
    {
        if ((mFlags & flags_looping) && pSound)
        {
            Play(pSound, pPos, pVel, true);
        }
        else
        {
            Stop(true);
        }
    }

    if (mVoiceVirtualHandle)
    {
        Snd::VoiceVirtual* pVoice = Snd::Sound::VoiceVirtualHandleToVoiceVirtual(mVoiceVirtualHandle);
        if (pPos)
        {
            pVoice->SetPosition(pPos);
        }
        if (pVel)
        {
            pVoice->SetVelocity(pVel);
        }
    }
}

// FUNCTION: SWBF2 0x00538690
Snd::VoiceVirtual* GameSoundControllable::GetVoiceVirtual() const
{
    if (mVoiceVirtualHandle)
    {
        return Snd::Sound::VoiceVirtualHandleToVoiceVirtual(mVoiceVirtualHandle);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00538730
void GameSoundControllable::StolenCallback(Snd::VoiceVirtual* pVoice, void* pData)
{
    GameSoundControllable* pThis = (GameSoundControllable*)pData;
    if (pVoice->GetPlayState() == Snd::SourceTransport::playState_stopped)
    {
        pThis->Stop(true);
    }
    else
    {
        pThis->ReleaseVoice();
    }
}

// SYNTHETIC: SWBF2 0x0047e660
// GameSound::`default constructor closure'
