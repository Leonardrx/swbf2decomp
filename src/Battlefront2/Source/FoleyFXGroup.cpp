#include "FoleyFX.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// SYNTHETIC: SWBF2 0x00403aa0
// `dynamic initializer for 'FoleyFXGroup::smList''
// SYNTHETIC: SWBF2 0x007687d0
// `dynamic atexit destructor for 'FoleyFXGroup::smList''

// GLOBAL: SWBF2 0x007eb8cc
// FoleyFXGroup::smList
PblListSingle FoleyFXGroup::smList;

// GLOBAL: SWBF2 0x01e55f74
FoleyFXGroup* FoleyFXGroup::smTerrain;

// GLOBAL: SWBF2 0x01e55f78
FoleyFXGroup* FoleyFXGroup::smWater;

// FUNCTION: SWBF2 0x0052c780
FoleyFXGroup::FoleyFXGroup()
    : mID(0)
{
    smList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0052c7c0
FoleyFXGroup::~FoleyFXGroup()
{
}

// FUNCTION: SWBF2 0x0052c800
void FoleyFXGroup::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._pFile->Read32())
        {
        case 0x8d39bde6:
            child._pFile->Read8();
            mID = child._pFile->Read32();
            switch (mID)
            {
            case 0x26c04c0e:
                smTerrain = this;
                break;
            case 0x425aa124:
                smWater = this;
                break;
            }
            break;
        case 0xe64c1c32:
        {
            child._pFile->Read8();
            FoleyFX* pFX = FoleyFX::FindByID(child._pFile->Read32());
            if (pFX)
            {
                mList.Add(pFX);
            }
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x0052c7f0
unsigned int FoleyFXGroup::GetID() const
{
    return mID;
}

// FUNCTION: SWBF2 0x0052c940
FoleyFXGroup* FoleyFXGroup::FindByID(unsigned int uID)
{
    for (PblListSingle::Node* pNode = smList.mHead.mNext; pNode != &smList.mHead;
         pNode = pNode->mNext)
    {
        FoleyFXGroup* pGroup = (FoleyFXGroup*)((char*)pNode - offsetof(FoleyFXGroup, mNode));
        if (pGroup->GetID() == uID)
        {
            return pGroup;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0052c8f0
FoleyFX* FoleyFXGroup::FindFoleyFX(unsigned int uClassID) const
{
    FoleyFX* pDefault = 0;
    for (const PblListSingle::Node* pNode = mList.mHead.mNext; pNode != &mList.mHead;
         pNode = pNode->mNext)
    {
        FoleyFX* pFX = (FoleyFX*)((const Snd::ObjectList::Node*)pNode)->mObject;
        if (uClassID == pFX->GetClassID())
        {
            return pFX;
        }
        if (!pDefault && !pFX->GetClassID())
        {
            pDefault = pFX;
        }
    }
    return pDefault;
}

// FUNCTION: SWBF2 0x0052c970
void FoleyFXGroup::DestroyAll()
{
    while (smList.mHead.mNext != &smList.mHead)
    {
        FoleyFXGroup* pGroup =
            (FoleyFXGroup*)((char*)smList.mHead.mNext - offsetof(FoleyFXGroup, mNode));
        delete pGroup;
    }
    smWater = 0;
    smTerrain = 0;
}

// FUNCTION: SWBF2 0x0052c9c0
FoleyFXGroup* FoleyFXGroup::GetWaterFX()
{
    return smWater;
}

// FUNCTION: SWBF2 0x0052c9d0
FoleyFXGroup* FoleyFXGroup::GetTerrainFX()
{
    return smTerrain;
}
