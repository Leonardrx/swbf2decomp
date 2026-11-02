#include <stdio.h>
#include <string.h>

#include "EntitySound.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "SoundFL/source/sndspace.h"

class SoundSpaceRegion : public Snd::Space::Region
{
public:

    SoundSpaceRegion(const RedRegion& rRegion) : Region(rRegion) {}

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }

    ~SoundSpaceRegion() {}

    // SYNTHETIC: SWBF2 0x004fad10
    // SoundSpaceRegion::operator delete
    static void operator delete(void* p) { sMemoryPool.Free(p); }

    static void DestroyAll();

    static MemoryPool sMemoryPool;
};
 

// SYNTHETIC: SWBF2 0x00402dd0
// `dynamic initializer for 'EntitySound::rttiHashEntitySound''
// EntitySound::rttiHashEntitySound

// GLOBAL: SWBF2 0x01ebc1a0
// EntitySound::rttiHashEntitySound
PblHash EntitySound::rttiHashEntitySound("EntitySound");
// GLOBAL: SWBF2 0x007895e4
// EntitySound::rttiNameEntitySound
const char EntitySound::rttiNameEntitySound[] = "EntitySound";

// SYNTHETIC: SWBF2 0x00402df0
// `dynamic initializer for 'EntitySoundClass::rttiHashEntitySoundClass''
// EntitySoundClass::rttiHashEntitySoundClass

// GLOBAL: SWBF2 0x01ebc0fc
// EntitySoundClass::rttiHashEntitySoundClass
PblHash EntitySoundClass::rttiHashEntitySoundClass("EntitySoundClass");
// GLOBAL: SWBF2 0x007895f0
// EntitySoundClass::rttiNameEntitySoundClass
const char EntitySoundClass::rttiNameEntitySoundClass[] = "EntitySoundClass";

// SYNTHETIC: SWBF2 0x00402e00
// `dynamic initializer for 'SoundSpaceRegion::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766fe0
// `dynamic atexit destructor for 'SoundSpaceRegion::sMemoryPool''

// GLOBAL: SWBF2 0x01ebc150
// SoundSpaceRegion::sMemoryPool
MemoryPool SoundSpaceRegion::sMemoryPool("SoundSpaceRegion", sizeof(SoundSpaceRegion));

// SYNTHETIC: SWBF2 0x00402de0
// `dynamic initializer for 'EntitySound::sEntitySoundList''
// SYNTHETIC: SWBF2 0x00766fd0
// `dynamic atexit destructor for 'EntitySound::sEntitySoundList''

// GLOBAL: SWBF2 0x007eb884
// EntitySound::sEntitySoundList
PblListSingle EntitySound::sEntitySoundList;













// SYNTHETIC: SWBF2 0x004fad20
// EntitySound::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fad50
EntitySound::EntitySound(EntitySoundClass* pClass, const EntityDesc& desc)
{
    mSoundId = pClass->mSoundId;
    if (pClass->mFlags & 1)
    {
        mMinDistance = pClass->mMinDistance;
        mFlags |= 1;
    }
    if (pClass->mFlags & 4)
    {
        mMuteDistance = pClass->mMuteDistance;
        mFlags |= 4;
    }
    if (pClass->mFlags & 2)
    {
        mMaxDistance = pClass->mMaxDistance;
        mFlags |= 2;
    }
    mPosition = desc.mMatrix.GetTranslation();
    sEntitySoundList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x004fadf0
EntitySound::~EntitySound()
{
}

// FUNCTION: SWBF2 0x004fae20
void EntitySound::SetProperty(unsigned int uProperty, const char* pcValue)
{
    float fValue;
    switch (uProperty)
    {
    case 0x0E0D9594:
    case 0x2E93EF4C:
        mSoundId = PblHash(pcValue);
        break;
    case 0xB835CF50:
        sscanf(pcValue, "%f", &fValue);
        SetMinDistance(fValue);
        break;
    case 0xE6915CF6:
        sscanf(pcValue, "%f", &fValue);
        SetMuteDistance(fValue);
        SetMaxDistance(fValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004faec0
void EntitySound::PlayAll()
{
    for (PblListSingle::Node* pNode = sEntitySoundList.mHead.mNext;
         pNode != &sEntitySoundList.mHead; pNode = pNode->mNext)
    {
        ((EntitySound*)((char*)pNode - offsetof(EntitySound, mNode)))->Play();
    }
}

// FUNCTION: SWBF2 0x004faef0
void EntitySound::DestroyAll()
{
    while (sEntitySoundList.mHead.mNext != &sEntitySoundList.mHead)
    {
        delete (EntitySound*)((char*)sEntitySoundList.mHead.mNext - offsetof(EntitySound, mNode));
    }
}

// FUNCTION: SWBF2 0x004faf20
EntitySoundClass::EntitySoundClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mSoundId = 0;
}

// SYNTHETIC: SWBF2 0x004faf60
// EntitySoundClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004faf90
EntitySoundClass::EntitySoundClass(EntitySoundClass* pBase, unsigned int uHash)
    : EntityClass(uHash)
{
    mSoundId = pBase->mSoundId;
}

// FUNCTION: SWBF2 0x004fafd0
EntitySoundClass::~EntitySoundClass()
{
}

// FUNCTION: SWBF2 0x004fafe0
void EntitySoundClass::SetProperty(unsigned int uProperty, const char* pcValue)
{
    float fValue;
    switch (uProperty)
    {
    case 0x0E0D9594:
    case 0x2E93EF4C:
        mSoundId = PblHash(pcValue);
        break;
    case 0xB835CF50:
        sscanf(pcValue, "%f", &fValue);
        SetMinDistance(fValue);
        break;
    case 0xE6915CF6:
        sscanf(pcValue, "%f", &fValue);
        SetMuteDistance(fValue);
        SetMaxDistance(fValue);
        break;
    default:
        EntityClass::SetProperty(uProperty, pcValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004fb090
EntitySound* EntitySoundClass::CreateSoundFromRegion(RedRegionDesc& desc, const char* pcName,
                                                     const char* pcPrefix, EntitySoundClass& soundClass)
{
    EntityDesc entityDesc;
    float fScale = 0.0f;
    char acSoundName[512];
    char acFormat[512];

    acSoundName[0] = 0;
    entityDesc.mMatrix = desc.mMatrix;
    entityDesc.mNameId = PblHash(pcName);
    entityDesc.mPlayerId = 0;

    strncpy(acFormat, pcPrefix, sizeof(acFormat) - 1);
    acFormat[sizeof(acFormat) - 1] = 0;
    strcat(acFormat, " %s %f");
    sscanf(pcName, acFormat, acSoundName, &fScale);

    EntitySound* pSound = (EntitySound*)soundClass.Build(entityDesc);
    if (pSound)
    {
        pSound->SetSoundId(PblHash(acSoundName));
        if (fScale > 0.0f)
        {
            pSound->SetMinDistance(desc.mSize.w / fScale);
        }
        pSound->mMuteDistance = desc.mSize.w;
        pSound->mFlags |= 4;
        pSound->mMaxDistance = desc.mSize.w;
        pSound->mFlags |= 2;
    }
    return pSound;
}

unsigned int Snd::Space::GetCount()
{
    unsigned int uCount = 0;
    for (PblListSingle::Itor it(smSpaceList); it.GetNode() != smSpaceList.GetTerminator(); ++it)
    {
        ++uCount;
    }
    return uCount;
}

// FUNCTION: SWBF2 0x004fac30
unsigned int Snd::Space::GetRegionCount() const
{
    unsigned int uCount = 0;
    for (PblListSingle::Itor it(mList); it.GetNode() != mList.GetTerminator(); ++it)
    {
        ++uCount;
    }
    return uCount;
}

// FUNCTION: SWBF2 0x004fac50
Snd::Space::Region* Snd::Space::FindRegionByIndex(unsigned int uIndex)
{
    PblListSingle::Node* pNode = mList.mHead.mNext;
    if (pNode == &mList.mHead)
    {
        return 0;
    }
    for (; uIndex; --uIndex)
    {
        pNode = pNode->mNext;
    }
    if (!pNode || pNode == &mList.mHead)
    {
        return 0;
    }
    return reinterpret_cast<Region*>(reinterpret_cast<char*>(pNode) - offsetof(Region, mSpaceNode));
}

Snd::Space* Snd::Space::FindByIndex(unsigned int uIndex)
{
    PblListSingle::Node* pNode = 0;
    if (!smSpaceList.IsEmpty())
    {
        pNode = smSpaceList.mHead.mNext;
        for (; uIndex; --uIndex)
        {
            pNode = pNode->mNext;
        }
    }
    if (pNode == &smSpaceList.mHead)
    {
        return 0;
    }
    return reinterpret_cast<Space*>(reinterpret_cast<char*>(pNode) - offsetof(Space, mNode));
}

// FUNCTION: SWBF2 0x004fb1e0
void SoundSpaceRegion::DestroyAll()
{
    unsigned int uCount = 0;
    for (PblListSingle::Itor it(Snd::Space::smSpaceList); it.GetNode() != Snd::Space::smSpaceList.GetTerminator(); ++it)
    {
        ++uCount;
    }
    for (unsigned int i = 0; i < uCount; ++i)
    {
        PblListSingle::Node* pNode = Snd::Space::smSpaceList.mHead.mNext;
        if (Snd::Space::smSpaceList.IsEmpty())
        {
            pNode = 0;
        }
        else
        {
            for (unsigned int uStep = 0; uStep < i; ++uStep)
            {
                pNode = pNode->mNext;
            }
        }
        Snd::Space* pSpace = pNode == &Snd::Space::smSpaceList.mHead
            ? 0
            : reinterpret_cast<Snd::Space*>(reinterpret_cast<char*>(pNode) - offsetof(Snd::Space, mNode));
        while (pSpace->GetRegionCount())
        {
            SoundSpaceRegion* pRegion =
                static_cast<SoundSpaceRegion*>(pSpace->FindRegionByIndex(0));
            delete pRegion;
        }
    }
}

// FUNCTION: SWBF2 0x004fb2c0
void* SoundSpaceRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    char acFormat[512];
    char acSpaceName[512];

    strncpy(acFormat, GetPrefix(), sizeof(acFormat) - 1);
    acFormat[sizeof(acFormat) - 1] = 0;
    strcat(acFormat, " %s");
    sscanf(pcName, acFormat, acSpaceName);

    Snd::Space* const pSpace = Snd::Space::Find(PblHash(acSpaceName));
    if (pSpace)
    {
        RedRegion region(desc);
        SoundSpaceRegion* const pRegion = new SoundSpaceRegion(region);
        if (pRegion)
        {
            pSpace->mList.AddLast(&pRegion->mSpaceNode);
            return pRegion;
        }
    }
    return 0;
}
