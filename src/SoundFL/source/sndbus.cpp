#include "sndbus.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndglobals.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00730de0
void Fader::Fade(float fLength, float fEndLevel, float fStartLevel)
{
    mTimeElasped = 0.0f;

    mStartLevel = (fStartLevel == -1.0f) ? mGain : fStartLevel;

    mEndLevel = fEndLevel;
    mLength = fLength;
}

static inline Bus* BusFromNode(PblListSingle::Node* pNode)
{
    return reinterpret_cast<Bus*>(reinterpret_cast<char*>(pNode) - offsetof(Bus, mNode));
}

// SYNTHETIC: SWBF2 0x0040b450
// `dynamic initializer for 'Snd::Bus::smFreeList''
// SYNTHETIC: SWBF2 0x0076a140
// `dynamic atexit destructor for 'Snd::Bus::smFreeList''

// GLOBAL: SWBF2 0x007e325c
// Bus::smFreeList
FreeList Bus::smFreeList(sizeof(Bus), 1, true, 8);

// SYNTHETIC: SWBF2 0x0040b470
// `dynamic initializer for 'Snd::Bus::smRootBus''
// SYNTHETIC: SWBF2 0x0076a150
// `dynamic atexit destructor for 'Snd::Bus::smRootBus''

// GLOBAL: SWBF2 0x007e3278
// Bus::smRootBus
Bus Bus::smRootBus(0, 0x20FD0E45u);

// GLOBAL: SWBF2 0x009cf494
bool Bus::smBusSolo;

// FUNCTION: SWBF2 0x00731360
__declspec(noinline) void Bus::Update(float fDeltaTime, bool bForce)
{
    const float fOldGain = Fader::mGain;
    Fader::Update(fDeltaTime);
    if (fOldGain != Fader::mGain || bForce)
    {
        UpdateFinalGain();
        bForce = true;
    }

    for (PblListSingle::Node* pNode = mSubBusList.mHead.mNext;
         pNode != &mSubBusList.mHead; pNode = pNode->mNext)
    {
        Bus* const pSub = (Bus*)pNode->GetObjectA(&mSubBusList, offsetof(Bus, mNode));
        pSub->Update(fDeltaTime, bForce);
    }
}

// FUNCTION: SWBF2 0x00731280
void Bus::SetGain(float fGain, bool bStopFade)
{
    if (mFlags & flags_Mute)
    {
        return;
    }

    mGain = fGain;
    if (bStopFade)
    {
        Stop();
    }
    Fader::Update(0.0f);
    UpdateFinalGain();

    for (PblListSingle::Node* pNode = mSubBusList.mHead.mNext;
         pNode != &mSubBusList.mHead; pNode = pNode->mNext)
    {
        Bus* const pSub = (Bus*)pNode->GetObjectA(&mSubBusList, offsetof(Bus, mNode));
        pSub->Update(0.0f, true);
    }
}

// FUNCTION: SWBF2 0x00730f40
void __cdecl Bus::MuteAll(Bus* pBus)
{
    if (!pBus)
    {
        pBus = &smRootBus;
    }

    for (PblListSingle::Itor itor(&pBus->mSubBusList);
         itor.GetNode() != pBus->mSubBusList.GetTerminator(); ++itor)
    {
        Bus* const pSub = BusFromNode(itor.GetNode());
        if (!(pSub->mFlags & flags_Solo))
        {
            pSub->SetGain(0.0f, true);
            pSub->mFlags |= flags_Mute;
        }
        MuteAll(pSub);
    }
}

// FUNCTION: SWBF2 0x00730db0
void Bus::DestroySubBuses()
{
    for (;;)
    {
        PblListSingle::Node* const pNode = mSubBusList.mHead.mNext;
        Bus* const pBus = pNode == &mSubBusList.mHead ? 0 : BusFromNode(pNode);
        if (!pBus)
        {
            break;
        }
        delete pBus;
    }
}



// FUNCTION: SWBF2 0x00730cd0
Bus* Bus::AddSubBus(unsigned int uiHashID)
{
    return new Bus(this, uiHashID);
}

// FUNCTION: SWBF2 0x00730e70
Bus* Bus::FindSubBusByHashID(unsigned int uiHashID)
{
    if (mHashID == uiHashID)
    {
        return this;
    }

    for (PblListSingle::Node* pNode = mSubBusList.mHead.mNext;
         pNode != &mSubBusList.mHead; pNode = pNode->mNext)
    {
        Bus* const pSub = (Bus*)pNode->GetObjectA(&mSubBusList, offsetof(Bus, mNode));
        Bus* const pFound = pSub->FindSubBusByHashID(uiHashID);
        if (pFound)
        {
            return pFound;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00730e20
__declspec(noinline) Bus* Bus::FindByHashID(unsigned int uiHashID)
{
    return smRootBus.FindSubBusByHashID(uiHashID);
}

// FUNCTION: SWBF2 0x00731000
void Bus::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch ((unsigned int)child._pFile->Read32())
        {
        case 0x247DB064u:
            child._pFile->Read8();
            mAttenuateBus = FindByHashID(child._pFile->Read32());
            break;
        case 0x1B5426FEu:
            child._pFile->Read8();
            SetGain(Clamp(child._pFile->ReadFloat(), 0.0f, 1.0f), true);
            break;
        case 0x5BA00EF7u:
            child._pFile->Read8();
            mAttenuation = Clamp(child._pFile->ReadFloat(), 0.0f, 1.0f);
            break;
        case 0x8D39BDE6u:
            child._pFile->Read8();
            mHashID = child._pFile->Read32();
            break;
        case 0xD16089AAu:
            child._pFile->Read8();
            mAttenuationLeadInTime = child._pFile->ReadFloat();
            break;
        case 0xE9634B4Au:
            child._pFile->Read8();
            mFlags = (unsigned char)((mFlags & ~flags_Mute) | flags_Solo);
            smBusSolo = true;
            break;
        case 0xD6069F54u:
            child._pFile->Read8();
            SetGain(0.0f, true);
            mFlags |= flags_Mute;
            break;
        case 0xEACDFCFDu:
        {
            child._pFile->Read8();
            Bus* const pParent = FindByHashID(child._pFile->Read32());
            mNode.Remove();
            pParent->mSubBusList.AddLast(&mNode);
            mParentBus = pParent;
            UpdateFinalGain();
            break;
        }
        case 0xF247D163u:
            child._pFile->Read8();
            mAttenuationLeadOutTime = child._pFile->ReadFloat();
            break;
        }
    }

    if (smBusSolo)
    {
        for (Bus* pBus = mParentBus; pBus && pBus != &smRootBus; pBus = pBus->mParentBus)
        {
            pBus->mFlags |= flags_Solo;
        }
        MuteAll(0);
    }
}

// FUNCTION: SWBF2 0x00730eb0
void __cdecl Bus::Load(PblFileChunk& chunk, bool bOnce)
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
        Bus* const pBus = smRootBus.AddSubBus(0);
        pBus->Read(child);
        if (bOnce)
        {
            break;
        }
    }
}

}
