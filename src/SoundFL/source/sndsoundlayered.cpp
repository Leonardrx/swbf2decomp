#include "sndsoundlayered.h"

#include <float.h>

#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

template <>
void Allocator<SoundLayered::FireForgetLayer>::operator delete(void* p, unsigned int uiSize)
{
    (void)uiSize;
    switch (((SoundLayered::FireForgetLayer*)p)->GetAllocType())
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
}

namespace Snd
{

// GLOBAL: SWBF2 0x01e29db0
SoundLayered::FireForgetLayer* SoundLayered::smFireForgetLayers;
// GLOBAL: SWBF2 0x01e29db4
bool SoundLayered::smIsOpen;
// GLOBAL: SWBF2 0x01e29db8
unsigned char SoundLayered::smFireForgetLayerStorage[5888];
// SYNTHETIC: SWBF2 0x0040bbb0
// `dynamic initializer for 'Snd::SoundLayered::FireForgetLayer::smInactiveList''
// SYNTHETIC: SWBF2 0x0076a3c0
// `dynamic atexit destructor for 'Snd::SoundLayered::FireForgetLayer::smInactiveList''

// GLOBAL: SWBF2 0x007e36bc
// SoundLayered::FireForgetLayer::smInactiveList
PblListDouble SoundLayered::FireForgetLayer::smInactiveList;

// SYNTHETIC: SWBF2 0x0040bb40
// `dynamic initializer for 'Snd::SoundLayered::FireForgetLayer::smActiveList''
// SYNTHETIC: SWBF2 0x0076a370
// `dynamic atexit destructor for 'Snd::SoundLayered::FireForgetLayer::smActiveList''

// GLOBAL: SWBF2 0x007e36b4
// SoundLayered::FireForgetLayer::smActiveList
PblListDouble SoundLayered::FireForgetLayer::smActiveList;

// SYNTHETIC: SWBF2 0x0040bb50
// `dynamic initializer for 'Snd::SoundLayered::Properties::Layer::smFreeList''
// SYNTHETIC: SWBF2 0x0076a390
// `dynamic atexit destructor for 'Snd::SoundLayered::Properties::Layer::smFreeList''

// GLOBAL: SWBF2 0x007e3678
// SoundLayered::Properties::Layer::smFreeList
FreeList SoundLayered::Properties::Layer::smFreeList(sizeof(SoundLayered::Properties::Layer), 1, true, 8);

// SYNTHETIC: SWBF2 0x0040bb70
// `dynamic initializer for 'Snd::SoundLayered::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a3a0
// `dynamic atexit destructor for 'Snd::SoundLayered::Properties::smFreeList''

// GLOBAL: SWBF2 0x007e3694
// SoundLayered::Properties::smFreeList
FreeList SoundLayered::Properties::smFreeList(0x0c, 1, true, 8);

// SYNTHETIC: SWBF2 0x0040bb90
// `dynamic initializer for 'Snd::SoundLayered::SoundLayer::smFreeList''
// SYNTHETIC: SWBF2 0x0076a3b0
// `dynamic atexit destructor for 'Snd::SoundLayered::SoundLayer::smFreeList''

// GLOBAL: SWBF2 0x007e365c
// SoundLayered::SoundLayer::smFreeList
FreeList SoundLayered::SoundLayer::smFreeList(sizeof(SoundLayered::SoundLayer), 1, false, 0x40);

// SYNTHETIC: SWBF2 0x0040bbc0
// `dynamic initializer for 'Snd::SoundLayered::Properties::smPropertyList''
// SYNTHETIC: SWBF2 0x0076a3e0
// `dynamic atexit destructor for 'Snd::SoundLayered::Properties::smPropertyList''

// GLOBAL: SWBF2 0x007e36b0
// SoundLayered::Properties::smPropertyList
PblListSingle SoundLayered::Properties::smPropertyList;

// SYNTHETIC: SWBF2 0x0040bbd0
// `dynamic initializer for 'Snd::SoundLayered::smSoundLayeredList''
// SYNTHETIC: SWBF2 0x0076a3f0
// `dynamic atexit destructor for 'Snd::SoundLayered::smSoundLayeredList''

// GLOBAL: SWBF2 0x007e36c4
// SoundLayered::smSoundLayeredList
PblListDouble SoundLayered::smSoundLayeredList;

// FUNCTION: SWBF2 0x00738cf0
template <>
void Allocator<SoundLayered::FireForgetLayer>::operator delete(void* p,
                                                               void* pMemory,
                                                               bool bTemp)
{
    (void)pMemory;
    (void)bTemp;

    if (!p)
    {
        return;
    }

    switch (((SoundLayered::FireForgetLayer*)p)->GetAllocType())
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

// FUNCTION: SWBF2 0x00738f40
void SoundLayered::Open()
{
    for (unsigned int i = 0; i < 64; ++i)
    {
        new (&((FireForgetLayer*)smFireForgetLayerStorage)[i], false) FireForgetLayer();
    }

    smFireForgetLayers = (FireForgetLayer*)smFireForgetLayerStorage;
    smIsOpen = true;
}

void SoundLayered::FireForgetLayer::Update(float fDeltaTime)
{
    if (mTimeElapsed >= mTriggerTime)
    {
        const Sound::Properties* pProps = mLayer->mSoundProps;
        Sound::Play(pProps->Get3DMode() ? &mSource3D : 0, pProps, 0, 0, true);
        mNode.Remove();
        smInactiveList.AddLast(&mNode);
    }
    mTimeElapsed += fDeltaTime;
}

// FUNCTION: SWBF2 0x00739a70
void SoundLayered::FireForgetLayer::UpdateAll(float fDeltaTime)
{
    for (PblListDouble::Itor itor(smActiveList); itor.GetNode() != smActiveList.GetTerminator();)
    {
        FireForgetLayer* const pLayer = (FireForgetLayer*)itor.GetNode()->GetObjectA(&smActiveList, offsetof(FireForgetLayer, mNode));
        ++itor;
        pLayer->Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x00739b20
void SoundLayered::UpdateAll(float fDeltaTime)
{
    PblListDouble::Node* pLink = smSoundLayeredList.mTerminator.mNext;
    while (pLink != &smSoundLayeredList.mTerminator)
    {
        ((SoundLayered*)pLink->GetObjectA(&smSoundLayeredList, offsetof(SoundLayered, mNode)))->Update(fDeltaTime);
        pLink = pLink->mNext;
    }
    FireForgetLayer::UpdateAll(fDeltaTime);
}

void SoundLayered::SoundLayerDestroy(SoundLayer* pLayer)
{
    delete pLayer;
}

__forceinline void SoundLayered::FreeLayers(void (__cdecl* pLayerDestroy)(SoundLayer*))
{
    if (mProperties)
    {
        mProperties = 0;
    }
    while (!mSoundList.IsEmpty())
    {
        pLayerDestroy((SoundLayer*)mSoundList.GetFirst()->GetObjectA(
            &mSoundList, offsetof(SoundLayer, mLayerNode)));
    }
}

// FUNCTION: SWBF2 0x007394b0
void SoundLayered::RemoveAll()
{
    for (PblListDouble::Itor itor(smSoundLayeredList); itor.GetNode() != smSoundLayeredList.GetTerminator();)
    {
        SoundLayered* const pSound = (SoundLayered*)itor.GetNode()->GetObjectA(&smSoundLayeredList, offsetof(SoundLayered, mNode));
        ++itor;
        pSound->Stop();
        pSound->mNode.Remove();
        pSound->FreeLayers(SoundLayerDestroy);
    }
}

// FUNCTION: SWBF2 0x00739780
void SoundLayered::FireForgetLayer::StopAll()
{
    for (PblListDouble::Itor itor(smActiveList); itor.GetNode() != smActiveList.GetTerminator();)
    {
        FireForgetLayer* const pLayer = (FireForgetLayer*)itor.GetNode()->GetObjectA(&smActiveList, offsetof(FireForgetLayer, mNode));
        ++itor;
        pLayer->Stop();
    }
}

// FUNCTION: SWBF2 0x00738da0
SoundLayered::Properties* __cdecl SoundLayered::Properties::FindByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Node* pNode = smPropertyList.mHead.mNext; pNode != &smPropertyList.mHead;
         pNode = pNode->mNext)
    {
        Properties* pProps = (Properties*)pNode->GetObjectA(&smPropertyList, offsetof(Properties, mNode));
        if (pProps->mHashID == uiHashID)
        {
            return pProps;
        }
    }
    return 0;
}

extern PblRandom sndRandom;

// FUNCTION: SWBF2 0x00738b10
SoundLayered::SoundLayered()
    : mTimeElapsed(0.0f),
      mStopTime(FLT_MAX),
      mProperties(0)
{
    smSoundLayeredList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00738c20
SoundLayered::~SoundLayered()
{
    FreeLayers(SoundLayerDestroy);
    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00738e00
Sound* SoundLayered::FindLayerByHashID(unsigned int uiHashID)
{
    PblListSingle::Node* pSoundNode = mSoundList.mHead.mNext;
    for (const PblListSingle::Node* pNode = mProperties->mLayerList.mHead.mNext;
         pNode != &mProperties->mLayerList.mHead;
         pNode = pNode->mNext, pSoundNode = pSoundNode->mNext)
    {
        const Properties::Layer* pLayer =
            (const Properties::Layer*)pNode->GetObjectA(&mProperties->mLayerList, offsetof(Properties::Layer, mNode));
        if (pLayer->mHashID == uiHashID)
        {            return (SoundLayer*)pSoundNode->GetObjectA(
                &mSoundList, offsetof(SoundLayer, mLayerNode));
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x007397e0
void SoundLayered::Update(float fDeltaTime)
{
    PlayState ePlayState = GetCurrentPlayState();
    const PlayState eDesired = QueryDesiredPlayState();
    SourceTransport::mFlags = (unsigned short)
        (SourceTransport::mFlags & ~(unsigned int)flags_updateTransport);
    const Properties* const pProps = mProperties;

    if (pProps && eDesired != playState_invalid && ePlayState != eDesired)
    {
        PblListSingle::Itor soundItor(mSoundList);
        switch (eDesired)
        {
        case playState_playing:
        {
            PblListSingle::Itor itor(pProps->mLayerList);
            mTimeElapsed = 0.0f;
            mStopTime = 0.0f;
            for (; itor.GetNode() != pProps->mLayerList.GetTerminator(); ++itor, ++soundItor)
            {
                const Properties::Layer* const pLayer = (const Properties::Layer*)itor.GetNode()->GetObjectA(
                    &pProps->mLayerList, offsetof(Properties::Layer, mNode));
                SoundLayer* const pSound = (SoundLayer*)soundItor.GetNode()->GetObjectA(
                    &mSoundList, offsetof(SoundLayer, mLayerNode));
                pSound->mPlayTime =
                    (sndRandom.GetFloat() - 0.5f) * pLayer->mDelayVar * 2.0f + pLayer->mDelay;
                pSound->mStopTime =
                    (sndRandom.GetFloat() - 0.5f) * pLayer->mPlayTimeVar * 2.0f
                    + pLayer->mPlayTime + pSound->mPlayTime;
                if (pSound->mStopTime > mStopTime)
                {
                    mStopTime = pSound->mStopTime;
                }
            }
            break;
        }

        case playState_paused:
            for (; soundItor.GetNode() != mSoundList.GetTerminator(); ++soundItor)
            {
                ((SoundLayer*)soundItor.GetNode()->GetObjectA(&mSoundList, offsetof(SoundLayer, mLayerNode)))->Pause();
            }
            break;

        case playState_stopped:
            for (; soundItor.GetNode() != mSoundList.GetTerminator(); ++soundItor)
            {
                SoundLayer* const pSound = (SoundLayer*)soundItor.GetNode()->GetObjectA(
                    &mSoundList, offsetof(SoundLayer, mLayerNode));
                pSound->SourceBus::Stop(pSound);
                pSound->VoiceVirtual::Stop();
            }
            break;
        }
        SetPlayState(eDesired);
        ePlayState = eDesired;
    }

    if (ePlayState == playState_playing && mProperties)
    {
        for (PblListSingle::Itor soundItor(mSoundList);
             soundItor.GetNode() != mSoundList.GetTerminator(); ++soundItor)
        {
            SoundLayer* const pSound = (SoundLayer*)soundItor.GetNode()->GetObjectA(
                &mSoundList, offsetof(SoundLayer, mLayerNode));
            if (pSound->GetPlayState() == playState_playing)
            {
                if (mTimeElapsed >= pSound->mStopTime)
                {
                    pSound->SourceBus::Stop(pSound);
                    pSound->VoiceVirtual::Stop();
                }
            }
            else if (mTimeElapsed >= pSound->mPlayTime)
            {
                pSound->Play(true);
            }
        }
        if (mTimeElapsed >= mStopTime)
        {
            Stop();
        }
        mTimeElapsed += fDeltaTime;
    }
}

inline float __cdecl SoundLayered::FireForgetLayer::CalculatePriority(Source3D* pSource3D,
                                                                      const Sound::Properties* pProps)
{
    SourceTransport transport;
    transport.mPlaybackPosition = 0;
    transport.mFlags = SourceTransport::flags_stopped;
    transport.Play(true);
    const Source2D3D* pSource2D3D = pProps;
    const Sample* pSample = (const Sample*)pProps->mSelObjectWeightList->GetObjectA();
    return pProps->SourceVirtual::CalculatePriority(&transport, pSource2D3D, pSource3D, 0,
                                                    pSample, 1.0f, false);
}

__forceinline SoundLayered::FireForgetLayer* __cdecl SoundLayered::FireForgetLayer::StealLayer(float fPriority)
{
    FireForgetLayer* pSteal = 0;
    float fLowest = FLT_MAX;
    for (PblListDouble::Node* pLink = smActiveList.mTerminator.mNext; pLink != &smActiveList.mTerminator;
         pLink = pLink->mNext)
    {
        FireForgetLayer* pLayer = pLink == &smInactiveList.mTerminator
            ? 0
            : (FireForgetLayer*)((char*)pLink - offsetof(FireForgetLayer, mNode));
        if (fPriority > pLayer->mPriority && fLowest > pLayer->mPriority)
        {
            fLowest = pLayer->mPriority;
            pSteal = pLayer;
        }
    }
    return pSteal;
}

// FUNCTION: SWBF2 0x00738e50
SoundLayered::FireForgetLayer* __cdecl SoundLayered::FireForgetLayer::GetFreeLayer()
{
    PblListDouble::Node* pFirst = smInactiveList.mTerminator.mNext;
    if (pFirst == &smInactiveList.mTerminator)
    {
        return 0;
    }
    return (FireForgetLayer*)((char*)pFirst - offsetof(FireForgetLayer, mNode));
}

// FUNCTION: SWBF2 0x00739030
void SoundLayered::FireForgetLayer::Play(Source3D* pSource3D, const Properties::Layer* pLayer, float fPriority)
{
    if (pSource3D)
    {
        mSource3D = *pSource3D;
        mPos = *pSource3D->mPos3D;
        mVel = *pSource3D->mVel3D;
        mSource3D.SetPosition(&mPos);
        mSource3D.SetVelocity(&mVel);
    }
    mLayer = pLayer;
    mTimeElapsed = 0.0f;
    mTriggerTime = (sndRandom.GetFloat() - 0.5f) * pLayer->mDelayVar * 2.0f + pLayer->mDelay;
    mNode.Remove();
    smActiveList.AddLast(&mNode);
    if (0.0f == fPriority)
    {
        mPriority = CalculatePriority(pSource3D, pLayer->mSoundProps);
    }
    else
    {
        mPriority = fPriority;
    }
}

// FUNCTION: SWBF2 0x007391d0
void __cdecl SoundLayered::Play(Source3D* pSource3D, const Properties* pProps)
{
    const PblListSingle* const pList = &pProps->mLayerList;
    for (PblListSingle::Itor itor(pList); itor.GetNode() != pList->GetTerminator(); ++itor)
    {
        Properties::Layer* const pLayer = (Properties::Layer*)itor.GetNode()->GetObjectA(
            pList, offsetof(Properties::Layer, mNode));
        if (0.0f == pLayer->mDelay)
        {
            Sound::Play(pLayer->mSoundProps->Get3DMode() ? pSource3D : 0,
                        pLayer->mSoundProps, 0, 0, true);
        }
        else
        {
            float fPriority = 0.0f;
            FireForgetLayer* pFireForget = FireForgetLayer::GetFreeLayer();
            pSource3D->SetProperties(pLayer->mSoundProps);
            pSource3D->mFlags |= Source3D::flags_updateListener;
            pSource3D->DetermineClosestListener();
            if (!pFireForget)
            {
                fPriority = FireForgetLayer::CalculatePriority(pSource3D,
                                                             pLayer->mSoundProps);
                pFireForget = FireForgetLayer::StealLayer(fPriority);
            }
            if (pFireForget)
            {
                pFireForget->Play(pSource3D, pLayer, fPriority);
            }
            pSource3D->SetProperties(0);
        }
    }
}

// FUNCTION: SWBF2 0x00739620
void SoundLayered::SetPosition(const PblVector3* pPos)
{
    for (PblListSingle::Itor it(mSoundList); it.GetNode() != mSoundList.GetTerminator(); ++it)
    {
        PblListSingle::Node* pNode = it.GetNode();
        SoundLayer* pSound = pNode == &mSoundList.mHead
            ? 0
            : (SoundLayer*)((char*)pNode - offsetof(SoundLayer, mLayerNode));
        pSound->Source3D::SetPosition(pPos);
    }
}

// FUNCTION: SWBF2 0x00739660
void SoundLayered::SetProperties(const Properties* pProps, SoundLayer* (__cdecl* pLayerCreate)(void),
                                 void (__cdecl* pLayerDestroy)(SoundLayer*))
{
    FreeLayers(pLayerDestroy);
    if (!pProps)
    {
        return;
    }
    for (PblListSingle::Itor it(pProps->mLayerList); it.GetNode() != pProps->mLayerList.GetTerminator(); ++it)
    {
        const Properties::Layer* pLayer =
            (const Properties::Layer*)it.GetNode()->GetObjectA(&pProps->mLayerList, offsetof(Properties::Layer, mNode));
        SoundLayer* pSound = pLayerCreate();
        if (!pSound)
        {
            while (!mSoundList.IsEmpty())
            {
                pLayerDestroy((SoundLayer*)mSoundList.GetFirst()->GetObjectA(
                    &mSoundList, offsetof(SoundLayer, mLayerNode)));
            }
            return;
        }
        mSoundList.AddLast(&pSound->mLayerNode);
        pSound->Sound::SetProperties(pLayer->mSoundProps);
    }
    mProperties = pProps;
}

// FUNCTION: SWBF2 0x00739740
void SoundLayered::SetVelocity(const PblVector3* pVel)
{
    for (PblListSingle::Itor it(mSoundList); it.GetNode() != mSoundList.GetTerminator(); ++it)
    {
        PblListSingle::Node* pNode = it.GetNode();
        SoundLayer* pSound = pNode == &mSoundList.mHead
            ? 0
            : (SoundLayer*)((char*)pNode - offsetof(SoundLayer, mLayerNode));
        pSound->Source3D::SetVelocity(pVel);
    }
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x00738b70
SoundLayered::Properties::~Properties()
{
    while (!mLayerList.IsEmpty())
    {
        delete (Layer*)((char*)mLayerList.mHead.mNext - offsetof(Layer, mNode));
    }
}
}

namespace Snd
{

// FUNCTION: SWBF2 0x00738d20
void SoundLayered::Close()
{
    for (unsigned int i = 0; i < 64; ++i)
    {
        delete (FireForgetLayer*)(smFireForgetLayerStorage + i * sizeof(FireForgetLayer));
    }
    smFireForgetLayers = 0;
    smIsOpen = false;
}
}

namespace Snd
{

// FUNCTION: SWBF2 0x00738dd0
SoundLayered::Properties::Layer* SoundLayered::Properties::FindLayerByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Node* pNode = mLayerList.mHead.mNext; pNode != &mLayerList.mHead;
         pNode = pNode->mNext)
    {
        Layer* const pLayer = (Layer*)pNode->GetObjectA(&mLayerList, offsetof(Layer, mNode));
        if (pLayer->mHashID == uiHashID)
        {
            return pLayer;
        }
    }
    return 0;
}
}

namespace Snd
{

// FUNCTION: SWBF2 0x00738e70
void __cdecl SoundLayered::Properties::Load(PblFileChunk& chunk, bool bOnce)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144u)
        {
            continue;
        }

        child._pFile->Read32();
        child = chunk.ReadNextChild();

        Properties* const pProperties = new Properties;
        pProperties->Read(child);
        if (bOnce)
        {
            break;
        }
    }
}
}

namespace Snd
{

// FUNCTION: SWBF2 0x00739330
void SoundLayered::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        switch (uiID)
        {
        case 0x07A640F6u:
        {
            const char cVersion = child._pFile->Read8();
            const unsigned int uiHashID = child._pFile->Read32();
            Sound::Properties* const pSoundProps =
                (Sound::Properties*)Sound::Properties::FindByHashID(child._pFile->Read32());
            if (pSoundProps)
            {
                const float fDelay = child._pFile->ReadFloat();
                const float fDelayVar = cVersion >= 4 ? child._pFile->ReadFloat() : 0.0f;
                const float fPlayTime = cVersion >= 5 ? child._pFile->ReadFloat() : FLT_MAX;
                const float fPlayTimeVar = cVersion >= 6 ? child._pFile->ReadFloat() : 0.0f;
                new Layer(pSoundProps, fDelay, fDelayVar, fPlayTime, fPlayTimeVar, uiHashID,
                          mLayerList);
            }
            break;
        }
        case 0x8D39BDE6u:
        {
            child._pFile->Read8();
            mHashID = child._pFile->Read32();
            break;
        }
        }
    }
}
}
