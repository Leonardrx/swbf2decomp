#include <d3dx9math.h>
#include <windows.h>
#include <new>
#include <string.h>

#include "PathManager.h"

#include "AILowLevel.h"
#include "EventInfo.h"
#include "PathFinder.h"
#include "UnitController.h"


// FUNCTION: SWBF2 0x0050b400
void PblRef::Kill()
{
    mAliveFlag = 0;
}

// SYNTHETIC: SWBF2 0x00405e30
// `dynamic initializer for 'AIPathNode::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692d0
// `dynamic atexit destructor for 'AIPathNode::sMemoryPool''

// GLOBAL: SWBF2 0x01ef7df8
// AIPathNode::sMemoryPool
MemoryPool AIPathNode::sMemoryPool("AIPathNode", sizeof(AIPathNode));
// SYNTHETIC: SWBF2 0x00405e50
// `dynamic initializer for 'AIPath::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692e0
// `dynamic atexit destructor for 'AIPath::sMemoryPool''

// GLOBAL: SWBF2 0x01ef7e98
// AIPath::sMemoryPool
MemoryPool AIPath::sMemoryPool("AIPath", sizeof(AIPath));

namespace PathManager
{

// SYNTHETIC: SWBF2 0x00405e70
// `dynamic initializer for 'PathManager::PathRequest::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692f0
// `dynamic atexit destructor for 'PathManager::PathRequest::sMemoryPool''

// GLOBAL: SWBF2 0x01ef7e48
// PathRequest::sMemoryPool
MemoryPool PathRequest::sMemoryPool("PathRequest", sizeof(PathRequest));

// GLOBAL: SWBF2 0x01eaedc8
PblList<PathRequest>* sRequests;

// GLOBAL: SWBF2 0x01eaedcc
PathRequest* sCurRequest;

// GLOBAL: SWBF2 0x01eaedc4
int mCostConstraint;

// GLOBAL: SWBF2 0x01eaedc0
int sPathManagerUnknown0;
// GLOBAL: SWBF2 0x01eaedd0
int sPathManagerUnknown1;

// SYNTHETIC: SWBF2 0x00405e90
// PathManager::`dynamic initializer for 'mTimer''
// SYNTHETIC: SWBF2 0x00769300
// PathManager::`dynamic atexit destructor for 'mTimer''

// GLOBAL: SWBF2 0x007ebc38
// mTimer
PblPerformanceTimer mTimer;

}

// FUNCTION: SWBF2 0x00613fb0
AIPath::AIPath() : mNodes()
{
}

// FUNCTION: SWBF2 0x00614010
AIPath::~AIPath()
{
    PblList<AIPathNode>::Itor it(mNodes);
    while (it)
    {
        AIPathNode* pNode = it;
        ++it;
        mNodes.Remove(pNode->GetListNode());
        delete pNode;
    }
}

// FUNCTION: SWBF2 0x006140b0
void AIPath::Prepend(const PblVector3& pos, unsigned short uFlags)
{
    if (!AIPathNode::sMemoryPool.IsFull())
    {
        AIPathNode* pNode = new AIPathNode(pos, uFlags);
        mNodes.AddFirst(pNode->GetListNode(), pNode);
    }
}

// FUNCTION: SWBF2 0x00614120
void AIPath::Append(const PblVector3& pos, unsigned short uFlags)
{
    if (!AIPathNode::sMemoryPool.IsFull())
    {
        AIPathNode* pNode = new AIPathNode(pos, uFlags);
        mNodes.AddLast(pNode->GetListNode(), pNode);
    }
}

// FUNCTION: SWBF2 0x00614190
void AIPath::AppendPath(const AIPath* pOther)
{
    PblList<AIPathNode>::Itor it(pOther->GetNodeList());
    if (GetNodeList().GetLast()->_pObject->mPos == it->mPos)
    {
        ++it;
    }
    for (; it; ++it)
    {
        AIPathNode* pNode = it;
        Append(pNode->mPos, pNode->mFlags);
    }
}

// FUNCTION: SWBF2 0x00614210
void AIPath::PrependPath(const AIPath* pOther)
{
    for (PblList<AIPathNode>::Itor it(pOther->GetNodeList()); it; ++it)
    {
        AIPathNode* pNode = it;
        Prepend(pNode->mPos, pNode->mFlags);
    }
}

// FUNCTION: SWBF2 0x00614250
void AIPath::PopHead()
{
    AIPathNode* pNode = mNodes.GetFirst()->_pObject;
    mNodes.Remove(pNode->GetListNode());
    delete pNode;
}

// FUNCTION: SWBF2 0x006142a0
void AIPath::TrimBefore(AIPathNode* pStop)
{
    PblList<AIPathNode>::Itor it(mNodes);
    while (it && (AIPathNode*)it != pStop)
    {
        AIPathNode* pNode = it;
        ++it;
        mNodes.Remove(pNode->GetListNode());
        delete pNode;
    }
}

// FUNCTION: SWBF2 0x00614320
bool AIPath::SmoothCorners(float fCornerDist, float fRadius, int eProhibit)
{
    if (mNodes.GetCount() < 3)
    {
        return false;
    }

    bool bSmoothed = false;
    PblList<AIPathNode>::Itor it(mNodes);
    AIPathNode* pCur = it;
    ++it;
    AIPathNode* pNext = it;
    for (++it; it; ++it)
    {
        AIPathNode* pPrev = pCur;
        pCur = pNext;
        pNext = it;

        if ((int)(AIPathNode::sMemoryPool.mCount - AIPathNode::sMemoryPool.mUsed) < 64 || mNodes.GetCount() >= 30)
        {
            break;
        }

        PblVector3& vCur = pCur->mPos;
        const PblVector3& vPrev = pPrev->mPos;
        const PblVector3& vNext = pNext->mPos;
        float fMinDistSq = fCornerDist * fCornerDist * 4.0f;

        if (fMinDistSq > SquaredLength(vPrev - vCur))
        {
            continue;
        }

        if (fMinDistSq > SquaredLength(vCur - vNext))
        {

            ++it;
            if (!it)
            {
                break;
            }
            continue;
        }

        PblVector3 vIn = vCur - vPrev;
        PblVector3 vDirIn;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDirIn), reinterpret_cast<const D3DXVECTOR3*>(&vIn));
        PblVector3 vOut = vNext - vCur;
        PblVector3 vDirOut;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDirOut), reinterpret_cast<const D3DXVECTOR3*>(&vOut));
        if (DotProduct(vDirIn, vDirOut) > 0.984f)
        {
            continue;
        }

        PblVector3 vStart = vCur - vDirIn * fCornerDist;
        PblVector3 vEnd = vCur + vDirOut * fCornerDist;
        PblVector3 vMid = (vCur + vStart + vEnd) / 3.0f;

        if (PathFinder::TestTrivialStraightLineHit(vStart, vMid, fRadius, eProhibit, 0, 0, false))
        {
            continue;
        }
        if (PathFinder::TestTrivialStraightLineHit(vMid, vEnd, fRadius, eProhibit, 0, 0, false))
        {
            continue;
        }

        AIPathNode* pStart = new AIPathNode(vStart, 0);
        mNodes.AddBefore(pCur->GetListNode(), pStart->GetListNode(), pStart);

        AIPathNode* pEnd = new AIPathNode(vEnd, 0);
        mNodes.AddAfter(pCur->GetListNode(), pEnd->GetListNode(), pEnd);

        bSmoothed = true;
        vCur = vMid;

        pCur = pEnd;
    }
    return bSmoothed;
}

// SYNTHETIC: SWBF2 0x0050b420
// PblRef::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006148b0
unsigned char PathManager::GetPathPriority(const PblVector3& aSrc, const PblVector3& aDest)
{
    float lengthSqXZ = SquaredLengthXZ(aSrc - aDest);
    return !(lengthSqXZ < 2500.0f);
}

namespace PathManager
{

// FUNCTION: SWBF2 0x00614730
PathRequest::PathRequest(UnitController* pUnit, const PblVector3& vStart, const PblVector3& vGoal,
                         float fRadius, int eProhibit, unsigned char uPriority,
                         const ConnectivityArc*  ,
                         const Obstacle** ppIgnore, int iIgnoreCount)
{
    mUnit = pUnit;
    mStart = vStart;
    mGoal = vGoal;
    mRadius = fRadius;
    mProhibit = eProhibit;
    mPriority = uPriority;
    mObstacleCount = iIgnoreCount;
    memcpy(mObstacles, ppIgnore, sizeof(const Obstacle*) * iIgnoreCount);
}

// FUNCTION: SWBF2 0x006147d0
void Initialize()
{
    PathRequest::sMemoryPool.Create(0x2ee, PathRequest::sMemoryPool.GetSize(),
                                    PathRequest::sMemoryPool.GetGrow());
    AIPath::sMemoryPool.Create(0x4b0, AIPath::sMemoryPool.GetSize(), AIPath::sMemoryPool.GetGrow());
    AIPathNode::sMemoryPool.Create(0x2580, AIPathNode::sMemoryPool.GetSize(),
                                   AIPathNode::sMemoryPool.GetGrow());
    sRequests = new PblList<PathRequest>();
    sCurRequest = 0;
    sPathManagerUnknown0 = 0;
    sPathManagerUnknown1 = 0;
}

// FUNCTION: SWBF2 0x00614870
void Reset()
{
    sRequests = 0;
    sCurRequest = 0;
    PathRequest::sMemoryPool.Reset();
    AIPath::sMemoryPool.Reset();
    AIPathNode::sMemoryPool.Reset();
}

// FUNCTION: SWBF2 0x006148e0
void RemovePathRequest(UnitController* pCtrl)
{
    if (sCurRequest && pCtrl == sCurRequest->mUnit)
    {
        PathFinder::CancelPathFind();
        PathRequest::sMemoryPool.Free(sCurRequest);
        sCurRequest = 0;
    }

    PblList<PathRequest>& requests = *sRequests;
    for (PblList<PathRequest>::Itor it(requests); it; ++it)
    {
        PathRequest* pReq = it;
        if (pCtrl == pReq->mUnit)
        {
            requests.Remove(&pReq->mNode);
            PathRequest::sMemoryPool.Free(pReq);
            return;
        }
    }
}

// FUNCTION: SWBF2 0x006149b0
void LowerRequestPriority(UnitController* pCtrl)
{
    if (sCurRequest && sCurRequest->mUnit == pCtrl)
    {
        return;
    }

    for (PblList<PathRequest>::Itor it(*sRequests); it; ++it)
    {
        PathRequest* pReq = it;
        if (pReq->mUnit == pCtrl)
        {
            if (pReq->mPriority)
            {
                pReq->mPriority = pReq->mUnit->mLowLevel.GetPathQueueDepth();
            }
            return;
        }
    }
}

// FUNCTION: SWBF2 0x00614a60
bool RequestPath(UnitController* pUnit, const PblVector3& vStart, const PblVector3& vGoal,
                 float fRadius, int eProhibit, unsigned char uPriority,
                 const ConnectivityArc* pArc, const Obstacle** ppIgnore, int iIgnoreCount)
{
    AILowLevel* pLow = &pUnit->mLowLevel;
    uPriority += pLow->GetPathQueueDepth();
    RemovePathRequest(pUnit);

    if (pLow->GetPathQueueDepth() <= 4
        && !PathFinder::TestTrivialStraightLineHit(vStart, vGoal, fRadius, eProhibit, ppIgnore,
                                                    iIgnoreCount, false))
    {
        AIPath* pPath = (AIPath*)AIPath::sMemoryPool.Allocate(sizeof(AIPath));
        if (pPath)
        {
            pPath = ::new (pPath) AIPath();
        }
        if (pPath)
        {
            pPath->Prepend(vGoal, 0);
            pPath->Prepend(vStart, 0);
            AI::EventInfo_PathResult event(pPath);
            pUnit->SendEvent(&event);
            pPath->Release();
            return true;
        }
        return false;
    }

    if ((unsigned int)PathRequest::sMemoryPool.mUsed < PathRequest::sMemoryPool.mCount)
    {
        void* pMem = PathRequest::sMemoryPool.Allocate(sizeof(PathRequest));
        PathRequest* pReq = new (pMem) PathRequest(pUnit, vStart, vGoal, fRadius, eProhibit,
                                                   uPriority, pArc, ppIgnore, iIgnoreCount);
        sRequests->AddLast(&pReq->mNode, pReq);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00614bb0
PathRequest* GetNextPathRequest()
{
    PblList<PathRequest>::Itor it(*sRequests);
    for (unsigned char uPriority = 0; uPriority < 5; ++uPriority)
    {
        while (PathRequest* pReq = it)
        {
            ++it;
            if (pReq->mPriority == uPriority)
            {
                sRequests->Remove(&pReq->mNode);
                if (pReq->mUnit)
                {
                    return pReq;
                }
                PathRequest::sMemoryPool.Free(pReq);
            }
        }
        it = PblList<PathRequest>::Itor(*sRequests);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00614c40
bool IsDone()
{
    return PathFinder::IsCompleted() && sRequests->GetCount() == 0;
}

// FUNCTION: SWBF2 0x00614c60
void Update(float fDeltaTime)
{
    mCostConstraint = (int)(fDeltaTime * 10.0f * 5.0f / 5.0f);
    PathFinder::ResetCost();

    mTimer.Reset();
    mTimer.Start();

    if (IsWithinTimeConstraint())
    {
        for (;;)
        {
            if (PathFinder::IsCompleted())
            {
                sCurRequest = GetNextPathRequest();
                if (!sCurRequest)
                {
                    break;
                }
                PathFinder::NewPathFind(sCurRequest->mStart, sCurRequest->mGoal, sCurRequest->mRadius,
                                        sCurRequest->mProhibit, sCurRequest->mObstacles,
                                        sCurRequest->mObstacleCount);
            }
            if (!PathFinder::IsCompleted())
            {
                PathFinder::UpdatePathFind();
                if (PathFinder::GetPausedAtFinish())
                {
                    break;
                }
            }
            if (PathFinder::IsCompleted())
            {
                AIPath* pPath = PathFinder::GetPathResult();
                UnitController* pUnit = sCurRequest->mUnit;
                if (pUnit)
                {
                    AI::EventInfo_PathResult event(pPath);
                    pUnit->SendEvent(&event);
                }
                if (pPath)
                {
                    pPath->Release();
                }
                if (sCurRequest)
                {
                    PathRequest::sMemoryPool.Free(sCurRequest);
                }
                sCurRequest = 0;
            }
            if (!IsWithinTimeConstraint())
            {
                break;
            }
        }
    }

    mTimer.Stop();
}

// FUNCTION: SWBF2 0x00614e00
bool IsWithinTimeConstraint()
{
    return PathFinder::GetCost() < mCostConstraint;
}

}

// FUNCTION: SWBF2 0x00613f60
AIPathNode::AIPathNode(const PblVector3& pos, unsigned short flags) : mPos(pos), mFlags(flags) {}

// FUNCTION: SWBF2 0x00613fa0
PblList<AIPathNode>::Node* AIPathNode::GetListNode() { return &mNode; }

// FUNCTION: SWBF2 0x00614310
const PblList<AIPathNode>& AIPath::GetNodeList() const { return mNodes; }
