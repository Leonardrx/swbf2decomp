#include "FoleyFXSoldier.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// SYNTHETIC: SWBF2 0x00403b10
// `dynamic initializer for 'FoleyFXSoldier::rttiHashFoleyFXSoldier''
// FoleyFXSoldier::rttiHashFoleyFXSoldier

// GLOBAL: SWBF2 0x01ebd13c
// FoleyFXSoldier::rttiHashFoleyFXSoldier
PblHash FoleyFXSoldier::rttiHashFoleyFXSoldier("FoleyFXSoldier");
// GLOBAL: SWBF2 0x007898d4
// FoleyFXSoldier::rttiNameFoleyFXSoldier
const char FoleyFXSoldier::rttiNameFoleyFXSoldier[] = "FoleyFXSoldier";

// SYNTHETIC: SWBF2 0x00403af0
// `dynamic initializer for 'FoleyFXSoldier::smFreeList''
// SYNTHETIC: SWBF2 0x00768800
// `dynamic atexit destructor for 'FoleyFXSoldier::smFreeList''

// GLOBAL: SWBF2 0x007ec1a4
// FoleyFXSoldier::smFreeList
Snd::FreeList FoleyFXSoldier::smFreeList(sizeof(FoleyFXSoldier), 4, true, 8);







// FUNCTION: SWBF2 0x0052cc90
FoleyFXSoldier::FoleyFXSoldier()
{
}

// SYNTHETIC: SWBF2 0x0052cd20
// FoleyFXSoldier::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052cd50
FoleyFXSoldier::~FoleyFXSoldier()
{
}

// FUNCTION: SWBF2 0x0052cd60
bool FoleyFXSoldier::Read(PblFileChunk& chunk, unsigned int uChunkID)
{
    switch (uChunkID)
    {
    case 0x2a9f6673:
        chunk._pFile->Read8();
        mFootstepLeftWalk.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0xe79eb7c2:
        chunk._pFile->Read8();
        mFootstepRightWalk.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x640f6efb:
        chunk._pFile->Read8();
        mFootstepLeftRun.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0xa4df1b00:
        chunk._pFile->Read8();
        mFootstepRightRun.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0xb59a8702:
        chunk._pFile->Read8();
        mFootstepLeftStop.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x54e8af1f:
        chunk._pFile->Read8();
        mFootstepRightStop.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0xa73f5c0d:
        chunk._pFile->Read8();
        mJump.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x6dea369c:
        chunk._pFile->Read8();
        mLand.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x16ff3d1e:
        chunk._pFile->Read8();
        mRoll.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x6dd99a25:
        chunk._pFile->Read8();
        mProne.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x65bfa4e1:
        chunk._pFile->Read8();
        mSquat.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x75493323:
        chunk._pFile->Read8();
        mStand.SetID(chunk._pFile->Read32(), false, false);
        return true;
    case 0x70dc49d0:
        chunk._pFile->Read8();
        mBodyFall.SetID(chunk._pFile->Read32(), false, false);
        return true;
    }
    return FoleyFXImpact::Read(chunk, uChunkID);
}
