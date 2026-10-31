#include "FoleyFX.h"
#include "FoleyFXOrdnance.h"
#include "FoleyFXSoldier.h"
#include "FoleyFXWalker.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// SYNTHETIC: SWBF2 0x00403a90
// `dynamic initializer for 'FoleyFX::rttiHashFoleyFX''
// FoleyFX::rttiHashFoleyFX

// GLOBAL: SWBF2 0x01ebc2bc
// FoleyFX::rttiHashFoleyFX
PblHash FoleyFX::rttiHashFoleyFX("FoleyFX");
// GLOBAL: SWBF2 0x007898a4
// FoleyFX::rttiNameFoleyFX
const char FoleyFX::rttiNameFoleyFX[] = "FoleyFX";

// SYNTHETIC: SWBF2 0x00403a80
// `dynamic initializer for 'FoleyFX::smList''
// SYNTHETIC: SWBF2 0x007687c0
// `dynamic atexit destructor for 'FoleyFX::smList''

// GLOBAL: SWBF2 0x007eb8c8
// FoleyFX::smList
PblListSingle FoleyFX::smList;







// FUNCTION: SWBF2 0x0052c090
FoleyFX::FoleyFX()
    : mID(0), mClass(0)
{
    smList.AddFirst(&mNode);
}

// FUNCTION: SWBF2 0x0052c0f0
FoleyFX::~FoleyFX()
{
}

// SYNTHETIC: SWBF2 0x0052c0c0
// FoleyFX::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052c120
unsigned int FoleyFX::GetID() const
{
    return mID;
}

// FUNCTION: SWBF2 0x0052c130
unsigned int FoleyFX::GetClassID() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x0052c140
void FoleyFX::ReadChunk(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uChunkID = child._pFile->Read32();
        if (!Read(child, uChunkID))
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0052c1a0
bool FoleyFX::Read(PblFileChunk& chunk, unsigned int uChunkID)
{
    switch (uChunkID)
    {
    case 0x8d39bde6: /* "name" */
        chunk._pFile->Read8();
        mID = chunk._pFile->Read32();
        return true;
    case 0xab3e0bff: /* "Class" */
        chunk._pFile->Read8();
        mClass = chunk._pFile->Read32();
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0052c200
void FoleyFX::Load(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }

        switch ((unsigned int)child._pFile->Read32())
        {
        case 0xa1c9cbc5:
        {
            FoleyFXGroup* pGroup = new FoleyFXGroup;
            child = chunk.ReadNextChild();
            pGroup->Read(child);
            continue;
        }
        case 0xd098618c:
        {
            FoleyFX* pFX = new FoleyFXSoldier;
            child = chunk.ReadNextChild();
            pFX->ReadChunk(child);
            continue;
        }
        case 0x5e9b9948:
        {
            FoleyFX* pFX = new FoleyFXWalker;
            child = chunk.ReadNextChild();
            pFX->ReadChunk(child);
            continue;
        }
        case 0xfd545f86:
        {
            FoleyFX* pFX = new FoleyFXImpact;
            child = chunk.ReadNextChild();
            pFX->ReadChunk(child);
            continue;
        }
        default:
            continue;
        }
    }
}

// FUNCTION: SWBF2 0x0052c390
FoleyFX* FoleyFX::FindByID(unsigned int uID)
{
    for (PblListSingle::Node* pNode = smList.mHead.mNext; pNode != &smList.mHead;
         pNode = pNode->mNext)
    {
        FoleyFX* pFX = (FoleyFX*)((char*)pNode - offsetof(FoleyFX, mNode));
        if (pFX->GetID() == uID)
        {
            return pFX;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0052c3c0
void FoleyFX::DestroyAll()
{
    while (smList.mHead.mNext != &smList.mHead)
    {
        FoleyFX* pFX = (FoleyFX*)((char*)smList.mHead.mNext - offsetof(FoleyFX, mNode));
        delete pFX;
    }
    smList.mHead.mNext = &smList.mHead;
}
