#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#include "SoundDebugDisplay.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatDecoder.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatTransport.h"
#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "SoundFL/source/sndsourcetransport.h"
#include "SoundFL/source/sndstreamnode.h"
#include "SoundFL/source/sndstreammixer.h"
#include "SoundFL/source/sndstreamsourceproducer.h"
#include "SoundFL/source/sndstreamgsmdecoder.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "DeathCamera.h"
#include "EntityWalker.h"
#include "EntityDroideka.h"
#include "EntityHover.h"
#include "EntityCarrier.h"
#include "Trackable.h"
#include "Music.h"
#include <math.h>
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndspace.h"
#include "SoundFL/source/sndi3dl2reverbpreset.h"
#include "SoundFL/source/sndvoicevirtual.h"
#include "SoundFL/source/sndstreambase.h"
#include "SoundFL/source/win/sndsamplebank.h"

 

// GLOBAL: SWBF2 0x01eafb28
SoundDebugDisplay* SoundDebugDisplay::sInstance;

// FUNCTION: SWBF2 0x0064b430
void SoundDebugDisplay::DisplayVoiceChatInfo()
{
    GameVoiceChat::Transport* pTransport = GameVoiceChat::GetTransport();
    const unsigned char ucNodes = pTransport->GetNumNodes();
    const unsigned int uiEnabledNodes = GameVoiceChat::GetNumEnabledNodes();
    for (unsigned char uiEnabled = 0; uiEnabled < uiEnabledNodes; ++uiEnabled)
    {
        GameVoiceChat::Node* pNode = GameVoiceChat::GetEnabledNode(uiEnabled);
        const unsigned char ucNodeID = pNode->GetID();
        char* const pEncoder = (char*)&pNode->GetEncoder();
        VoiceChat::Decoder* const pBaseDecoder = &pNode->GetDecoder();
        GameVoiceChat::Decoder* const pDecoder = pBaseDecoder ?
            static_cast<GameVoiceChat::Decoder*>(pBaseDecoder) : 0;
        Printf("Headset %d (player id=%d)\n", uiEnabled, ucNodeID);

        unsigned char ucConnections = 0;
        float fTotalOut = 0.0f;
        for (unsigned char ucOther = 0; ucOther < ucNodes; ++ucOther)
        {
            if (!pTransport->IsConnectedNet(ucOther))
            {
                continue;
            }
            const VoiceChat::Node::SendMetrics& metrics = VoiceChat::Node::GetSendMetrics(ucOther);
            Printf("out %2d, dsc %04d, dst %04.02f, tsc %04d, tst %04.02f, drp %3d, kps %02.02f : %02.02f\n",
                   (int)ucOther, metrics.sendCountDirect, metrics.lastSendTimeDirect,
                   metrics.sendCount, metrics.lastSendTime, metrics.droppedCount,
                   metrics.bandwidth, metrics.bandwidthAverage);
            fTotalOut += metrics.bandwidthAverage;
            ++ucConnections;
        }
        Printf("total out %2.02f\n", fTotalOut);

        for (unsigned char ucOther = 0; ucOther < ucNodes; ++ucOther)
        {
            if (!pTransport->IsConnectedNet(ucOther))
            {
                continue;
            }
            const VoiceChat::Node::ReceiveMetrics& metrics = VoiceChat::Node::GetReceiveMetrics(ucOther);
            const VoiceChat::Node::ReceiveStream& stream = pNode->GetReceiveStream(ucOther);
            const char* pcSend = pTransport->GetSendPossible(ucNodeID, ucOther) ? "sp"
                               : pTransport->GetSendEnable(ucNodeID, ucOther) ? "se" : "sd";
            const char* pcReceive = pTransport->GetReceivePossible(ucNodeID, ucOther) ? "rp"
                                  : pTransport->GetReceiveEnable(ucNodeID, ucOther) ? "re" : "rd";
            Snd::StreamSourceProducer* pSource = &pDecoder->mDecoders[ucOther];
            const unsigned int uiDecoded = pSource->GetNumQueuedPackets();
            Printf("in  %2d, bfd %2d:%2d, %s, %s, ltc %04.02f, sfq %04.02f, sct %2d, seq %2d %s %s, dec %2d\n",
                   (int)ucOther, pNode->CalculateNumPackets(stream),
                   stream.numPackets, stream.activated ? "act" : "dis",
                   stream.starving ? "stv" : "ok ", metrics.receiveLatency,
                   metrics.starvedFrequency, metrics.starvedCount, metrics.outOfOrderCount,
                   pcReceive, pcSend, uiDecoded);
        }
        if (!ucConnections)
        {
            Printf("No other players connected\n");
        }

        Printf("capture: src %s, agc %s, vad %s, enc %s",
               GetPlayStateStringShort(((Snd::SourceTransport*)(pEncoder + 0x08))->GetPlayState()),
               GetPlayStateStringShort(((Snd::SourceTransport*)(pEncoder + 0x4b8))->GetPlayState()),
               GetPlayStateStringShort(((Snd::SourceTransport*)(pEncoder + 0x478))->GetPlayState()),
               GetPlayStateStringShort(((Snd::SourceTransport*)(pEncoder + 0x78))->GetPlayState()));

        Snd::StreamNode& rRenderer = pDecoder->mAuxRenderer;
        Printf("render : mix %s, aux %s %s %2d:%2d\n",
               pDecoder->mMixer.IsActive() ? "ply" : "stp",
               GetPlayStateStringShort(rRenderer.GetPlayState()),
               rRenderer.IsStarving() ? "stv" : "ok ", 0, 0);
    }
}

// FUNCTION: SWBF2 0x0064a6e0
SoundDebugDisplay::SoundDebugDisplay()
{
    sInstance = this;
    mMode = mode_Inactive;
    mGameObjectPrev = 0;
    mSpeedPrev = 0.0f;
    mStreamID = 0;
    mSampleID = 0;

    mGroupElement.Enable(false);
    mGroupElement.SetPosition(0.0f, 0.0f);

    mBitmapElement.Init();
    mBitmapElement.SetRect(0.0f, 0.0f, (float)RedInterfaceScreen::GetScreenWidth(), 0.0f);
    const RedColor backColor = { 0x00, 0x44, 0x00, 0x40 };
    mBitmapElement.SetColor(backColor);
    mGroupElement.AddElement(&mBitmapElement);

    const RedColor textColor = { 0xff, 0xff, 0xff, 0xff };
    float fY = 0.0f;
    for (int i = 0; i < NUM_TEXT_ELEMENTS; ++i)
    {
        RedTextElement& text = mTextElement[i];
        text.SetIntercharacterSpacing(0);
        text.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_NORMAL);
        text.SetColor(textColor);
        text.SetFont("systemfont");
        text.SetTextScale(0.8f, 0.8f);
        text.Enable(false);
        text.SetPosition(0.0f, fY);
        mGroupElement.AddElement(&text);
        fY += 15.0f;
    }

    FLRenderer::AddInterfaceElement(&mGroupElement, RedInterfaceScreen::LEFT, RedInterfaceScreen::TOP, true);
    Clear();
}

// FUNCTION: SWBF2 0x0064a930
SoundDebugDisplay::~SoundDebugDisplay()
{
    sInstance = 0;
}

// FUNCTION: SWBF2 0x0064a9c0
void SoundDebugDisplay::Init()
{
    for (int i = 0; i < NUM_TEXT_ELEMENTS; ++i)
    {
        mTextElement[i].SetFont("systemfont");
        mTextElement[i].SetTextScale(0.8f, 0.8f);
    }
}

// FUNCTION: SWBF2 0x0064aa10
bool SoundDebugDisplay::Update(float fDeltaTime)
{
    Clear();
    if (0.0f == fDeltaTime)
    {
        return true;
    }

    switch (mMode)
    {
    case mode_Inactive:
        Hide();
        break;
    case mode_VirtualVoices:
    case mode_Sounds:
        DisplayVirtualVoices(0);
        break;
    case mode_ChaseObject:
        DisplayChaseInfo(fDeltaTime);
        break;
    case mode_Music:
        DisplayMusicInfo();
        break;
    case mode_Stream:
        DisplayStreamInfo();
        break;
    case mode_VoiceChat:
        DisplayVoiceChatInfo();
        break;
    case mode_Space:
        DisplaySpaceInfo();
        break;
    }
    return true;
}

// FUNCTION: SWBF2 0x0064aae0
void SoundDebugDisplay::SetMode(int)
{
    mMode = mode_Inactive;
}

// FUNCTION: SWBF2 0x0064aaf0
void SoundDebugDisplay::Hide()
{
    if (IsThreadActive())
    {
        DeactivateThread();
        Clear();
        RedInterfaceElement* pGroup = &mGroupElement;
        pGroup->Enable(false);
    }
}

// FUNCTION: SWBF2 0x0064ab20
void SoundDebugDisplay::Clear()
{
    mCursorPosY = 0;
    RedTextElement* pText = mTextElement;
    for (int i = 0; i < NUM_TEXT_ELEMENTS; ++i)
    {
        pText[i].Enable(false);
    }
}

// FUNCTION: SWBF2 0x0064ab50
void SoundDebugDisplay::Printf(const char* pcFormat, ...)
{
    if (mCursorPosY < NUM_TEXT_ELEMENTS)
    {
        char acBuffer[256];
        va_list args;
        va_start(args, pcFormat);
        _vsnprintf(acBuffer, sizeof(acBuffer), pcFormat, args);
        va_end(args);

        for (char* p = acBuffer; *p; ++p)
        {
            if (*p == '-')
            {
                *p = 'N';
            }
        }

        mTextElement[mCursorPosY].SetText(acBuffer, true);
        mTextElement[mCursorPosY].Enable(true);
        ++mCursorPosY;
    }

    RedBitmapElement* pBitmap = &mBitmapElement;
    pBitmap->SetRect(0.0f, 0.0f, (float)RedInterfaceScreen::GetScreenWidth(),
                           (float)mCursorPosY * 15.0f);
}

// FUNCTION: SWBF2 0x0064ac50
void SoundDebugDisplay::DisplaySpaceInfo()
{
    const Snd::Listener& listener = Snd::EngineBase::smListeners[0];
    if (!listener.Enabled())
    {
        return;
    }

    const Snd::Space* pSpace = listener.mSpace;
    if (!pSpace)
    {
        return;
    }

    Printf("Space\n");
    Printf("Name : 0x%08x %s\n", pSpace->mHashID,
           pSpace == &Snd::Space::smDefaultSpace ? "Default" : "");
    Printf("DirectGain : %f\n", pSpace->mDirectGain);
    Printf("DirectHFGain : %f\n", pSpace->mDirectHFGain);
    Printf("ObstructionHFGain : %f\n", pSpace->mObstructionHFGain);
    Printf("ObstructionLFRatio : %f\n", pSpace->mObstructionLFRatio);
    Printf("OcclusionHFGain : %f\n", pSpace->mOcclusionHFGain);
    Printf("OcclusionLFRatio : %f\n", pSpace->mOcclusionLFRatio);
    Printf("Priority : %f\n", pSpace->mPriority);
    Printf("RoomGain : %f\n", pSpace->mRoomGain);
    Printf("RoomHFGain : %f\n", pSpace->mRoomHFGain);
    Printf("RoomRollOff : %f\n", pSpace->mRoomRollOffFactor);

    const Snd::I3DL2ReverbPreset* pPreset = (const Snd::I3DL2ReverbPreset*)pSpace->mReverbPreset;
    Printf("ReverbPreset : 0x%08x\n", pPreset ? pPreset->mHashID : 0);
}

// FUNCTION: SWBF2 0x0064ade0
void SoundDebugDisplay::DisplayVirtualVoices(Snd::VoiceVirtualManager* pManager)
{
    if (!pManager)
    {
        pManager = Snd::EngineBase::GetVoiceVirtualManager(true);
    }

    const PblListDoubleSorted& list = pManager->mActiveList;
    PblListDoubleSorted::Itor itor(list);
    Printf("voice, pri., dist., 3d gain, play pos., min, max, roll");

    PblListDoubleSorted::Node* pNode;
    for (; (pNode = itor.GetNode(&list)) != list.GetTerminator(); ++itor)
    {
        Snd::VoiceVirtual* pVoice =
            (Snd::VoiceVirtual*)((char*)pNode - offsetof(Snd::VoiceVirtual, mNode));

        float fGain;
        pVoice->Snd::Source3D::CalculateGain(fGain);

        if ((mMode == mode_Sounds && pVoice->mCustomFlags == 0) ||
            (mMode != mode_Sounds && mSampleID != 0 &&
             (pVoice->mSample == 0 || pVoice->mSample->mHashID != mSampleID)))
        {
            continue;
        }

        Printf("0x%08x, %.02f, %.02f, %.02f, 0x%08x, %.02f, %.02f, %.02f",
               pVoice->mVoice, pVoice->mNode.mSortValue, pVoice->mDistanceLength, fGain,
               pVoice->mPlaybackPosition, pVoice->mMinDistance, pVoice->mMaxDistance,
               pVoice->mRollOffFactor);
    }
}

// FUNCTION: SWBF2 0x0064aef0
void SoundDebugDisplay::DisplayChaseInfo(float fDeltaTime)
{
    CameraManager* pManager = CameraManager::sInstance;
    if (!pManager || !pManager->IsChaseMode(0))
    {
        return;
    }

    Trackable* pOwner = pManager->mCurGameCamera[0] == pManager->mDeathCamera[0]
                      ? pManager->mDeathCamera[0]->mOwner
                      : pManager->mChaseCamera[0]->mOwner;
    if (!pOwner)
    {
        return;
    }
    GameObject* pObject = pOwner->GetGameObject();
    if (pObject)
    {
        const PblVector3& vVelocity = pObject->GetVelocity();
        const float fSpeed = Length(vVelocity);
        const float fAccel = (fSpeed - mSpeedPrev) / fDeltaTime;
        Printf("Speed %03.03f", fSpeed);
        if (fDeltaTime > 0.0f)
        {
            if (mGameObjectPrev == pObject)
            {
                Printf("Acceleration %03.03f", fAccel);
            }
            else
            {
                mSpeedPrev = 0.0f;
            }
        }
        mSpeedPrev = fSpeed;

        if (pObject->IsRtti(0x8b3e2650 /* "EntityWalker" */))
        {
            EntityWalker* pWalker = (EntityWalker*)pObject;
            const unsigned int uiFeet = 2 * pWalker->mLegPairCount;
            for (unsigned int i = 0; i < uiFeet; ++i)
            {
                Printf("FH %d %d %d %03.03f", (pWalker->mFootState >> i) & 1,
                       (pWalker->mFootState >> (i + 8)) & 1,
                       (pWalker->mFootState >> (i + 16)) & 1,
                       pWalker->mLastFootHeight[i]);
            }
        }
        else if (pObject->IsRtti(0x3639fed7 /* "EntityDroideka" */))
        {
            EntityDroideka* pDroideka = (EntityDroideka*)pObject;
            const unsigned int uiFeet = pDroideka->mClass->mNumFeet;
            for (unsigned int i = 0; i < uiFeet; ++i)
            {
                Printf("FH %d %d %d %03.03f", (pDroideka->mFootState >> i) & 1,
                       (pDroideka->mFootState >> (i + 8)) & 1,
                       (pDroideka->mFootState >> (i + 16)) & 1,
                       pDroideka->mLastFootHeight[i]);
            }
        }
        else if (pObject->IsRtti(0x26395ec0 /* "EntityFlyer" */))
        {
            EntityFlyer* pFlyer = (EntityFlyer*)pObject;
            Printf("Speed %03.03f", pFlyer->mSpeedPrev);
            float fClassAccel = pFlyer->mClass->mAcceleration;
            if (fClassAccel == 0.0f)
            {
                fClassAccel = 1.0f;
            }
            Printf("Acceleration %03.03f", fAccel / fClassAccel);
        }
        else if (pObject->IsRtti(0xc6cd61cc /* "EntityHover" */))
        {
            EntityHover* pHover = (EntityHover*)pObject;
            Printf("Speed %03.03f", pHover->mSpeedPrev);
            float fClassAccel = pHover->mClass->mAcceleration;
            if (fClassAccel == 0.0f)
            {
                fClassAccel = 1.0f;
            }
            Printf("Acceleration %03.03f", fAccel / fClassAccel);
            Printf("Height %03.03f", 1.0f - pHover->mGroundRatio);
        }
    }
    mGameObjectPrev = pObject;
}

// FUNCTION: SWBF2 0x0064b260
void SoundDebugDisplay::DisplayMusicInfo()
{
    Printf("Active  , Cur. PS, Des. PS, Pos., Inact., Play, Pri.");

    const PblListDoubleSorted& list = Music::sMusicActive;
    PblListDoubleSorted::Itor itor(list);
    PblListDoubleSorted::Node* pNode;
    for (; (pNode = itor.GetNode(&list)) != list.GetTerminator(); ++itor)
    {
        Music* pMusic = (Music*)((char*)pNode - offsetof(Music, mNode));

        float fInactiveTimeLeft = pMusic->GetInactiveTimeLeft();
        if (!(fInactiveTimeLeft > 0.0f))
        {
            fInactiveTimeLeft = 0.0f;
        }
        float fPlaybackTimeLeft = pMusic->mMinPlaybackTime - pMusic->mPlaybackTime;
        if (!(fPlaybackTimeLeft > 0.0f))
        {
            fPlaybackTimeLeft = 0.0f;
        }

        Printf("0x%08x, %s, %s, 0x%08x, %02.02f, %02.02f, %02.02f", pMusic->mHashID,
               GetMusicPlayStateString(pMusic->mPlayStateCurrent),
               GetMusicPlayStateString(pMusic->mPlayStateDesired), pMusic->mPlaybackPosition,
               fInactiveTimeLeft, fPlaybackTimeLeft, pNode->mSortValue);
    }
}

// FUNCTION: SWBF2 0x0064b310
const char* SoundDebugDisplay::GetMusicPlayStateString(unsigned int uiPlayState)
{
    switch (uiPlayState)
    {
    case Music::playState_invalid:
    case Music::playState_paused:
        return "Paused ";
    case Music::playState_pausing:
        return "Pausing";
    case Music::playState_playing:
        return "Playing";
    case Music::playState_starting:
        return "Starting";
    case Music::playState_stopped:
        return "Stopped ";
    case Music::playState_stopping:
        return "Stopping";
    }
    return "";
}

// FUNCTION: SWBF2 0x0064b380
const char* SoundDebugDisplay::GetPlayStateStringShort(unsigned int uiPlayState)
{
    switch (uiPlayState)
    {
    case Snd::SourceTransport::playState_playing:
        return "ply";
    case Snd::SourceTransport::playState_paused:
        return "psd";
    case Snd::SourceTransport::playState_pausing:
        return "psg";
    case Snd::SourceTransport::playState_stopped:
        return "stp";
    }
    return "";
}

// FUNCTION: SWBF2 0x0064b3d0
void SoundDebugDisplay::DisplayStreamInfo()
{
    const unsigned int uiStreamID = mStreamID;
    Snd::StreamBase::Info* pInfo = Snd::StreamBase::Info::Find(uiStreamID);
    if (pInfo)
    {
        DisplayVirtualVoices(pInfo->GetStreamManager());
    }
    else
    {
        mStreamID = 0;
        SetMode(mode_Inactive);
    }
}

// SYNTHETIC: SWBF2 0x0064a900
// SoundDebugDisplay::`scalar deleting destructor'

 
 
 
 
 
 
