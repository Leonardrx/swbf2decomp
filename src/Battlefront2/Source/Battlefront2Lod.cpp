#include "Battlefront2Lod.h"
#include "RedEngineFL/Graphics/RedModel.h"

// GLOBAL: SWBF2 0x007b2640
// __xmm@00000000000000010000000100000001

// FUNCTION: SWBF2 0x00448830
OrdnanceLodData::OrdnanceLodData()
{
    mMaxDist = 1000.0f;
    _LodClass = -1;
    _LodBitmask = 4;
    _LodCost[0] = -1;
    _LodCost[1] = -1;
    _LodCost[2] = 10;
    _LodCost[3] = 0;
    mLowRezFlags = 1;
}

// FUNCTION: SWBF2 0x00448870
NoLodData::NoLodData()
{
    _LodClass = -1;
    _LodBitmask = 0;
    _LodCost[0] = -1;
    _LodCost[1] = -1;
    _LodCost[2] = -1;
    _LodCost[3] = -1;
    mLowRezFlags = 0;
}

// FUNCTION: SWBF2 0x004486f0
SoldierLodData::SoldierLodData(const RedModel* pModel, const RedModel* pLowResModel)
{
    _LodClass = 2;
    _LodCost[0] = 2000;
    _LodCost[1] = -1;
    if (pLowResModel)
    {
        _LodCost[2] = 2000;
        _LodCost[3] = 2000;
        mLowRezFlags = 1;
        _LodBitmask = 5;
    }
    else
    {
        _LodCost[2] = 1;
        _LodCost[3] = 0;
        mLowRezFlags = 0;
        _LodBitmask = 1;
    }
}

// SYNTHETIC: SWBF2 0x00402310
// `dynamic initializer for 'ordnanceloddata''

// GLOBAL: SWBF2 0x01eb0570
// ordnanceloddata
OrdnanceLodData ordnanceloddata;

// SYNTHETIC: SWBF2 0x00402320
// `dynamic initializer for 'noloddata''

// GLOBAL: SWBF2 0x01eb0550
// noloddata
NoLodData noloddata;

// FUNCTION: SWBF2 0x00448760
ModelLodData::ModelLodData(int, const RedModel* pLod0, const RedModel* pLod1, const RedModel* pLod2,
                           const RedModel* pLod3)
{
    _LodClass = 1;
    _LodBitmask = 0;
    _LodCost[0] = 0;
    _LodCost[1] = 0;
    _LodCost[2] = 0;
    _LodCost[3] = 0;
    mLowRezFlags = 0;
    if (pLod0)
    {
        _LodCost[0] = pLod0->_uiNumStripVtx;
        _LodBitmask |= 1;
        if (pLod0->HasTransparentSegment())
        {
            mLowRezFlags |= 4;
        }
    }
    if (pLod1)
    {
        _LodCost[1] = pLod1->_uiNumStripVtx;
        _LodBitmask |= 2;
        if (pLod1->HasTransparentSegment())
        {
            mLowRezFlags |= 4;
        }
    }
    if (pLod2)
    {
        _LodCost[2] = pLod2->_uiNumStripVtx;
        _LodBitmask |= 4;
        if (pLod2->HasTransparentSegment())
        {
            mLowRezFlags |= 4;
        }
    }
    if (pLod3)
    {
        _LodCost[3] = pLod3->_uiNumStripVtx;
        _LodBitmask |= 8;
        mLowRezFlags |= 3;
        if (pLod3->HasTransparentSegment())
        {
            mLowRezFlags |= 4;
        }
    }
}

// FUNCTION: SWBF2 0x00448630
SmallEffectLodData::SmallEffectLodData()
{
    _LodClass = 3;
    _LodBitmask = 1;
    _LodCost[0] = 1;
    _LodCost[1] = 1;
    _LodCost[2] = 1;
    _LodCost[3] = 0;
    mLowRezFlags = 0;
    mMaxDist = 50.0f;
}

// FUNCTION: SWBF2 0x00448670
MediumEffectLodData::MediumEffectLodData()
{
    _LodClass = 3;
    _LodBitmask = 1;
    _LodCost[0] = 1;
    _LodCost[1] = 1;
    _LodCost[2] = 1;
    _LodCost[3] = 0;
    mLowRezFlags = 0;
    mMaxDist = 150.0f;
}

// FUNCTION: SWBF2 0x004486b0
LargeEffectLodData::LargeEffectLodData()
{
    _LodClass = 3;
    _LodBitmask = 1;
    _LodCost[0] = 1;
    _LodCost[1] = 1;
    _LodCost[2] = 1;
    _LodCost[3] = 0;
    mLowRezFlags = 1;
    mMaxDist = 500.0f;
}

// SYNTHETIC: SWBF2 0x004022e0
// `dynamic initializer for 'smallEffectLodData''

// GLOBAL: SWBF2 0x01eb0590
// smallEffectLodData
SmallEffectLodData smallEffectLodData;

// SYNTHETIC: SWBF2 0x004022f0
// `dynamic initializer for 'mediumEffectLodData''

// GLOBAL: SWBF2 0x01eb05d0
// mediumEffectLodData
MediumEffectLodData mediumEffectLodData;

// SYNTHETIC: SWBF2 0x00402300
// `dynamic initializer for 'largeEffectLodData''

// GLOBAL: SWBF2 0x01eb05b0
// largeEffectLodData
LargeEffectLodData largeEffectLodData;
