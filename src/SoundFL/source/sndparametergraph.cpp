#include "sndparametergraph.h"

#include <stddef.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// SYNTHETIC: SWBF2 0x0040c3c0
// `dynamic initializer for 'Snd::ParameterGraph::ControlPoint::smFreeList''
// SYNTHETIC: SWBF2 0x0076a4d0
// `dynamic atexit destructor for 'Snd::ParameterGraph::ControlPoint::smFreeList''

// GLOBAL: SWBF2 0x007e39b8
// ParameterGraph::ControlPoint::smFreeList
FreeList ParameterGraph::ControlPoint::smFreeList(12, 1, true, 16);

// SYNTHETIC: SWBF2 0x0040c3e0
// `dynamic initializer for 'Snd::ParameterGraph::smFreeList''
// SYNTHETIC: SWBF2 0x0076a4e0
// `dynamic atexit destructor for 'Snd::ParameterGraph::smFreeList''

// GLOBAL: SWBF2 0x007e39d4
// ParameterGraph::smFreeList
FreeList ParameterGraph::smFreeList(16, 1, true, 8);

// SYNTHETIC: SWBF2 0x0040c400
// `dynamic initializer for 'Snd::ParameterGraph::smParameterGraphs''
// SYNTHETIC: SWBF2 0x0076a4f0
// `dynamic atexit destructor for 'Snd::ParameterGraph::smParameterGraphs''

// GLOBAL: SWBF2 0x007e39f0
// ParameterGraph::smParameterGraphs
PblListSingle ParameterGraph::smParameterGraphs;

// FUNCTION: SWBF2 0x00748b30
ParameterGraph::~ParameterGraph()
{
    while (mControlPoints.GetFirst() != mControlPoints.GetTerminator())
    {
        delete (ControlPoint*)mControlPoints.GetFirst()->GetObjectA(&mControlPoints, offsetof(ControlPoint, mNode));
    }
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x00748d40
const ParameterGraph* __cdecl ParameterGraph::FindByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Node* pLink = smParameterGraphs.mHead.mNext; pLink != &smParameterGraphs.mHead;
         pLink = pLink->mNext)
    {
        const ParameterGraph* const pGraph = (const ParameterGraph*)pLink->GetObjectA(&smParameterGraphs, offsetof(ParameterGraph, mNode));
        if (pGraph->mHashID == uiHashID)
        {
            return pGraph;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00748d70
void __cdecl ParameterGraph::Load(PblFileChunk& chunk, bool bOnce)
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

        ParameterGraph* const pGraph = new ParameterGraph;
        pGraph->Read(child);
        if (bOnce)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00748e40
void ParameterGraph::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        child._pFile->Read8();
        switch (uiID)
        {
        case 0x415B9110u:
        {
            const float fInput = child._pFile->ReadFloat();
            const float fOutput = child._pFile->ReadFloat();
            new ControlPoint(fInput, fOutput, mControlPoints);
            break;
        }
        case 0x8D39BDE6u:
            mHashID = child._pFile->Read32();
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00748c70
float ParameterGraph::CalculateOutputValue(float fInput) const
{
    const ControlPoint* pPrev = 0;
    for (PblListSingleSorted::Itor itor(mControlPoints);
         itor.GetNode(&mControlPoints) != mControlPoints.GetTerminator(); ++itor)
    {
        PblListSingleSorted::Node* const pNode = itor.GetNode(&mControlPoints);
        const ControlPoint* const pPoint = (const ControlPoint*)pNode->GetObjectA(
            &mControlPoints, offsetof(ControlPoint, mNode));
        if (pPoint->mNode.mSortValue > fInput)
        {
            if (pPrev)
            {
                const float fBlend = (pPoint->mNode.mSortValue - fInput)
                                   / (pPoint->mNode.mSortValue - pPrev->mNode.mSortValue);
                return (1.0f - fBlend) * pPoint->mOutput + pPrev->mOutput * fBlend;
            }
            return pPoint->mOutput;
        }
        pPrev = pPoint;
    }
    return pPrev->mOutput;
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x00748d00
void __cdecl ParameterGraph::DestroyAll()
{
    while (!smParameterGraphs.IsEmpty())
    {
        delete (ParameterGraph*)smParameterGraphs.GetFirst()->GetObjectA(
            &smParameterGraphs, offsetof(ParameterGraph, mNode));
    }
}
}
