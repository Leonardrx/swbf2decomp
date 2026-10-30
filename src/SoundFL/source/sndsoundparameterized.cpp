#include "sndsoundparameterized.h"

#include "sndglobals.h"
#include "sndparametergraph.h"

#include <float.h>
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// SYNTHETIC: SWBF2 0x0040b7f0
// `dynamic initializer for 'Snd::SoundParameterized::Layer::smFreeList''
// SYNTHETIC: SWBF2 0x0076a250
// `dynamic atexit destructor for 'Snd::SoundParameterized::Layer::smFreeList''

// GLOBAL: SWBF2 0x007e3504
// SoundParameterized::Layer::smFreeList
FreeList SoundParameterized::Layer::smFreeList(0x100, 1, true, 0xa0);


// SYNTHETIC: SWBF2 0x0040b810
// `dynamic initializer for 'Snd::SoundParameterized::Controller::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a260
// `dynamic atexit destructor for 'Snd::SoundParameterized::Controller::Properties::smFreeList''

// GLOBAL: SWBF2 0x007e34cc
// SoundParameterized::Controller::Properties::smFreeList
FreeList SoundParameterized::Controller::Properties::smFreeList(0x10, 1, true, 0x10);

// SYNTHETIC: SWBF2 0x0040b830
// `dynamic initializer for 'Snd::SoundParameterized::Layer::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a270
// `dynamic atexit destructor for 'Snd::SoundParameterized::Layer::Properties::smFreeList''

// GLOBAL: SWBF2 0x007e34e8
// SoundParameterized::Layer::Properties::smFreeList
FreeList SoundParameterized::Layer::Properties::smFreeList(0x0c, 1, true, 0x10);

// SYNTHETIC: SWBF2 0x0040b850
// `dynamic initializer for 'Snd::SoundParameterized::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a280
// `dynamic atexit destructor for 'Snd::SoundParameterized::Properties::smFreeList''

// GLOBAL: SWBF2 0x007e3520
// SoundParameterized::Properties::smFreeList
FreeList SoundParameterized::Properties::smFreeList(0x10, 1, true, 0x10);

// SYNTHETIC: SWBF2 0x0040b870
// `dynamic initializer for 'Snd::SoundParameterized::Properties::smPropertyList''
// SYNTHETIC: SWBF2 0x0076a290
// `dynamic atexit destructor for 'Snd::SoundParameterized::Properties::smPropertyList''

// GLOBAL: SWBF2 0x007e353c
// SoundParameterized::Properties::smPropertyList
PblListSingle SoundParameterized::Properties::smPropertyList;

// SYNTHETIC: SWBF2 0x0040b880
// `dynamic initializer for 'Snd::SoundParameterized::smSoundParameterizedList''
// SYNTHETIC: SWBF2 0x0076a2a0
// `dynamic atexit destructor for 'Snd::SoundParameterized::smSoundParameterizedList''

// GLOBAL: SWBF2 0x007e3540
// SoundParameterized::smSoundParameterizedList
PblListDouble SoundParameterized::smSoundParameterizedList;

// FUNCTION: SWBF2 0x007357f0
Sound::Sound()
    : VoiceVirtualBus(0, 0, true),
      mProperties(0)
{
    smSoundList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00735880
SoundParameterized::SoundParameterized()
{
    mUpdateTime = false;
    mProperties = 0;
    mNode.Remove();
    smSoundParameterizedList.AddLast(&mNode);
}

SoundParameterized::Layer::Properties::Properties() : mHashID(0)
{
}

SoundParameterized::Properties::Properties() : mHashID(0), mSoundLayeredProps(0)
{
    smPropertyList.AddLast(&mNode);
}

SoundParameterized::Properties::~Properties()
{
    while (mLayerList.GetFirst() != mLayerList.GetTerminator())
    {
        delete (Layer::Properties*)mLayerList.GetFirst()->GetObjectA(&mLayerList, offsetof(Layer::Properties, mNode));
    }
}

// FUNCTION: SWBF2 0x007358e0
SoundParameterized::Layer::Properties::~Properties()
{
    while (mControllerPropertyList.GetFirst() != mControllerPropertyList.GetTerminator())
    {
        delete (Controller::Properties*)mControllerPropertyList.GetFirst()->GetObjectA(&mControllerPropertyList, offsetof(Controller::Properties, mNode));
    }
}

// FUNCTION: SWBF2 0x00735a40
SoundParameterized::~SoundParameterized()
{
    Stop();
    SoundLayered::Update(0.0f);
    SetProperties(0);
}

// FUNCTION: SWBF2 0x00735b40
void __cdecl SoundParameterized::Properties::DestroyAll()
{
    while (smPropertyList.GetFirst() != smPropertyList.GetTerminator())
    {
        delete (Properties*)smPropertyList.GetFirst()->GetObjectA(&smPropertyList, offsetof(Properties, mNode));
    }
}

// FUNCTION: SWBF2 0x00735c00
SoundParameterized::Properties* __cdecl SoundParameterized::Properties::FindByHashID(unsigned int uiHashID)
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

// FUNCTION: SWBF2 0x00735c30
void __cdecl SoundParameterized::Free()
{
    unsigned int uiNumBlocks = 0;
    for (const PblListSingle::Node* pNode = Snd::SoundParameterized::Layer::smFreeList.mBlockList.mHead.mNext;
         pNode != &Snd::SoundParameterized::Layer::smFreeList.mBlockList.mHead; pNode = pNode->mNext)
    {
        ++uiNumBlocks;
    }
    if (uiNumBlocks)
    {
        Snd::SoundParameterized::Layer::smFreeList.mFlags &= ~FreeList::flags_locked;
        Snd::SoundParameterized::Layer::smFreeList.PurgeBlocks();
    }
}

// FUNCTION: SWBF2 0x00735d20
void __cdecl SoundParameterized::Properties::Load(PblFileChunk& chunk, bool bOnce)
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

// FUNCTION: SWBF2 0x00735df0
bool SoundParameterized::Properties::QueryParameter(unsigned int uiParameterID) const
{
    for (PblListSingle::Itor layerItor(mLayerList);
         layerItor.GetNode() != mLayerList.GetTerminator(); ++layerItor)
    {
        const Layer::Properties* pLayer =
            (const Layer::Properties*)layerItor.GetNode()->GetObjectA(&mLayerList, offsetof(Layer::Properties, mNode));
        for (PblListSingle::Itor controllerItor(pLayer->mControllerPropertyList);
             controllerItor.GetNode() != pLayer->mControllerPropertyList.GetTerminator(); ++controllerItor)
        {
            const Controller::Properties* pController =
                (const Controller::Properties*)controllerItor.GetNode()->GetObjectA(
                    &pLayer->mControllerPropertyList, offsetof(Controller::Properties, mNode));
            if (pController->mInputParameterID == uiParameterID)
            {
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00735e40
void SoundParameterized::Layer::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        child._pFile->Read8();
        switch (uiID)
        {
        case 0x8D39BDE6u:
            mHashID = child._pFile->Read32();
            break;
        case 0xE657A914u:
        {
            Controller::Properties* const pController = new Controller::Properties;
            pController->mInputParameterID = child._pFile->Read32();
            pController->mGraph = ParameterGraph::FindByHashID(child._pFile->Read32());
            pController->mSoundParameterID = child._pFile->Read32();
            mControllerPropertyList.AddLast(&pController->mNode);
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x00735f20
void SoundParameterized::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        child._pFile->Read8();
        switch (uiID)
        {
        case 0x07A640F6u:
        {
            Layer::Properties* const pLayer = new Layer::Properties;
            mLayerList.AddLast(&pLayer->mNode);
            child = chunk.ReadNextChild();
            pLayer->Read(child);
            break;
        }
        case 0x8D39BDE6u:
            mHashID = child._pFile->Read32();
            break;
        case 0xA54A2138u:
            mSoundLayeredProps = SoundLayered::Properties::FindByHashID(child._pFile->Read32());
            break;
        }
    }

    if (mSoundLayeredProps)
    {
        for (PblListSingle::Itor itor(mLayerList);
             itor.GetNode() != mLayerList.GetTerminator(); ++itor)
        {
            const Layer::Properties* const pLayer =
                (const Layer::Properties*)itor.GetNode()->GetObjectA(&mLayerList, offsetof(Layer::Properties, mNode));
            const_cast<SoundLayered::Properties*>(mSoundLayeredProps)->FindLayerByHashID(pLayer->mHashID);
        }
    }
}

// FUNCTION: SWBF2 0x00736040
void __cdecl SoundParameterized::RemoveAll()
{
    PblListDouble::Node* pNode = smSoundParameterizedList.mTerminator.mNext;
    while (pNode != &smSoundParameterizedList.mTerminator)
    {
        SoundParameterized* pSound =
            (SoundParameterized*)((char*)pNode - offsetof(SoundParameterized, mNode));
        pNode = pNode->mNext;
        pSound->Stop();
        pSound->mNode.Remove();
        pSound->SetProperties(0);
    }
}

// FUNCTION: SWBF2 0x007360c0
void SoundParameterized::ResetParameters()
{
    const PblListSingle& layerList = mProperties->mLayerList;
    for (PblListSingle::Itor itor(layerList);
         itor.GetNode() != layerList.GetTerminator(); ++itor)
    {
        const Layer::Properties* pLayerProps =
            (const Layer::Properties*)itor.GetNode()->GetObjectA(&layerList, offsetof(Layer::Properties, mNode));
        Layer* pLayer = static_cast<Layer*>(FindLayerByHashID(pLayerProps->mHashID));
        if (pLayer)
        {
            pLayer->Reset();
        }
    }
}

// FUNCTION: SWBF2 0x00736120
void SoundParameterized::SetParameter(unsigned int uiParameterID, float fValue, bool bScale)
{
    const PblListSingle& layerList = mProperties->mLayerList;
    for (PblListSingle::Itor layerItor(layerList);
         layerItor.GetNode() != layerList.GetTerminator(); ++layerItor)
    {
        const Layer::Properties* pLayerProps =
            (const Layer::Properties*)layerItor.GetNode()->GetObjectA(&layerList, offsetof(Layer::Properties, mNode));
        const PblListSingle& controlList = pLayerProps->mControllerPropertyList;
        for (PblListSingle::Itor controlItor(controlList);
             controlItor.GetNode() != controlList.GetTerminator(); ++controlItor)
        {
            const Controller::Properties* pController =
                (const Controller::Properties*)controlItor.GetNode()->GetObjectA(
                    &controlList, offsetof(Controller::Properties, mNode));
            if (pController->mInputParameterID != uiParameterID)
            {
                continue;
            }

            Layer* pLayer = static_cast<Layer*>(FindLayerByHashID(pLayerProps->mHashID));
            if (!pLayer)
            {
                continue;
            }

            const unsigned int uIndex =
                Sound::FindParameterIndexByHashID(pController->mSoundParameterID);
            if (bScale)
            {
                pLayer->mParameterValues[uIndex] *= fValue;
            }
            else if (pController->mGraph)
            {
                pLayer->mParameterValues[uIndex] *= pController->mGraph->CalculateOutputValue(fValue);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00736200
void SoundParameterized::SetProperties(const Properties* pProps)
{
    if (mProperties)
    {
        mUpdateTime = false;
        mProperties = 0;
        SoundLayered::SetProperties(0, &LayerCreate, &LayerDestroy);
    }
    if (pProps)
    {
        SoundLayered::SetProperties(pProps->mSoundLayeredProps, &LayerCreate, &LayerDestroy);
        if (SoundLayered::mProperties)
        {
            mProperties = pProps;
            mUpdateTime = pProps->QueryParameter(0x5D3C9BE4u);
        }
    }
}

// FUNCTION: SWBF2 0x007364f0
void __cdecl SoundParameterized::UpdateAll(float fDeltaTime)
{
    for (PblListDouble::Node* pNode = smSoundParameterizedList.mTerminator.mNext;
         pNode != &smSoundParameterizedList.mTerminator; pNode = pNode->mNext)
    {
        ((SoundParameterized*)pNode->GetObjectA(&smSoundParameterizedList, offsetof(SoundParameterized, mNode)))
            ->Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x00736270
void SoundParameterized::Update(float fDeltaTime)
{
    SoundLayered::Update(fDeltaTime);
    if (!mProperties || GetPlayState() != playState_playing)
    {
        return;
    }

    if (mUpdateTime)
    {
        SetParameter(0x5D3C9BE4u, mTimeElapsed, false);
    }

    const PblListSingle& layerList = mProperties->mLayerList;
    for (PblListSingle::Itor itor(layerList);
         itor.GetNode() != layerList.GetTerminator(); ++itor)
    {
        const Layer::Properties* pLayerProps =
            (const Layer::Properties*)itor.GetNode()->GetObjectA(&layerList, offsetof(Layer::Properties, mNode));
        Layer* pLayer = static_cast<Layer*>(FindLayerByHashID(pLayerProps->mHashID));
        if (!pLayer)
        {
            continue;
        }

        pLayer->SetGain(Clamp(pLayer->mProperties->mGain * pLayer->mParameterValues[0], 0.0f, 1.0f));
        pLayer->SetPitch(Clamp(pLayer->mProperties->mPitch * pLayer->mParameterValues[1], 0.1f, FLT_MAX));
        pLayer->SetReverbGain(Clamp(pLayer->mProperties->mReverbGain * pLayer->mParameterValues[2], 0.0f, FLT_MAX));
        pLayer->SetPan(Clamp(pLayer->mProperties->mPan * pLayer->mParameterValues[3], -1.0f, 1.0f));
        const float fMinDistance = Clamp(pLayer->mProperties->mMinDistance * pLayer->mParameterValues[4], 0.0f, FLT_MAX);
        pLayer->SetMinDistance(fMinDistance);
        const float fMuteDistance = Clamp(pLayer->mProperties->mMuteDistance * pLayer->mParameterValues[5], fMinDistance, FLT_MAX);
        pLayer->SetMuteDistance(fMuteDistance);
        pLayer->SetMaxDistance(Clamp(pLayer->mProperties->mMaxDistance * pLayer->mParameterValues[6], fMuteDistance, FLT_MAX));
        pLayer->SetRollOff(Clamp(pLayer->mProperties->mRollOffFactor * pLayer->mParameterValues[7], 0.0f, FLT_MAX));
        pLayer->SetRollIn(Clamp(pLayer->mProperties->mRollInFactor * pLayer->mParameterValues[9], 0.0f, FLT_MAX));
        pLayer->SetRollInDistance(Clamp(pLayer->mProperties->mRollInDistance * pLayer->mParameterValues[10], 0.0f, FLT_MAX));
    }
}

}

namespace Snd
{
// FUNCTION: SWBF2 0x00735b00
void SoundParameterized::Alloc()
{
    unsigned int uiBlocks = 0;
    for (const PblListSingle::Node* pNode = Layer::smFreeList.mBlockList.mHead.mNext;
         pNode != &Layer::smFreeList.mBlockList.mHead;
         pNode = pNode->mNext)
    {
        ++uiBlocks;
    }

    if (uiBlocks == 0)
    {

        Layer::smFreeList.AllocBlocks(1, 0);
        Layer::smFreeList.mFlags |= FreeList::flags_locked;
    }
}
}
