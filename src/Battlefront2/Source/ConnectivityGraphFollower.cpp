#include "ConnectivityGraphFollower.h"

#include "AIUtil.h"
#include "Controllable.h"
#include "PathFinder.h"
#include "UnitController.h"

// SYNTHETIC: SWBF2 0x00402740
// `dynamic initializer for 'ConnectivityGraphFollower::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766eb0
// `dynamic atexit destructor for 'ConnectivityGraphFollower::sMemoryPool''

// GLOBAL: SWBF2 0x01eb9750
// ConnectivityGraphFollower::sMemoryPool
MemoryPool ConnectivityGraphFollower::sMemoryPool("ConnectivityGraphFollower", sizeof(ConnectivityGraphFollower));

static inline float DistSq(const PblVector3& a, const PblVector3& b)
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z);
}

// FUNCTION: SWBF2 0x00482020
ConnectivityGraphFollower::ConnectivityGraphFollower(UnitController* pCtrl)
{
    mCtrl = pCtrl;
    mDestPos = PblVector3(0.0f, 0.0f, 0.0f);
    mUseDest2ForWeights = false;
    mGraph = 0;
    mDestNode = 0;
    mType = 0;
}

// FUNCTION: SWBF2 0x00482070
bool ConnectivityGraphFollower::SetDestination(const ConnectivityGraph* pGraph,
                                               const PblVector3& vFrom, const PblVector3& vTo,
                                               ConnectivityGraph::ProhibitFlag eType)
{
    mGraph = const_cast<ConnectivityGraph*>(pGraph);
    mDestPos = vTo;
    mType = eType;
    mCurArc = 0;
    mDestNode2 = 0;

    const ConnectivityNode* pFromNode = 0;
    const ConnectivityArc* pFromArc = 0;
    const int iFromClass = pGraph->ClassifyPoint(vFrom, &pFromNode, &pFromArc, eType);
    const ConnectivityNode* pToNode = 0;
    const ConnectivityArc* pToArc = 0;
    mDestClassification = pGraph->ClassifyPoint(mDestPos, &pToNode, &pToArc, eType);
    if (iFromClass == 0)
    {
        pFromArc = pGraph->FindClosestArc(vFrom, eType);
    }
    if (mDestClassification == 0)
    {
        pToArc = pGraph->FindClosestArc(mDestPos, eType);
    }

    if (iFromClass == 1)
    {
        if (mDestClassification == 1)
        {
            mDestNode = const_cast<ConnectivityNode*>(pToNode);
            if (pFromNode == pToNode)
            {
                mCurDestNode = const_cast<ConnectivityNode*>(pToNode);
                mCurDestPos = mDestPos;
                return true;
            }
            mCurDestNode = const_cast<ConnectivityNode*>(pFromNode);
            mCurDestPos = vFrom;
            return GotoNextLeg();
        }

        if (pFromNode == pToArc->n1 || pFromNode == pToArc->n2)
        {
            mDestNode = const_cast<ConnectivityNode*>(pFromNode);
            mCurDestNode = const_cast<ConnectivityNode*>(pFromNode);
            mCurDestPos = mDestPos;
            return true;
        }
        mCurDestNode = const_cast<ConnectivityNode*>(pFromNode);
        mCurDestPos = vFrom;
        const ConnectivityArc* pArc1 = pGraph->GetNextArc(0, *pToArc->n1, *pFromNode, eType);
        const ConnectivityArc* pArc2 = mGraph->GetNextArc(0, *pToArc->n2, *pFromNode, mType);
        if (!pArc1 || pArc1 == pToArc)
        {
            mDestNode = pToArc->n2;
        }
        else if (!pArc2 || pArc2 == pToArc)
        {
            mDestNode = pToArc->n1;
        }
        else
        {
            const PblVector3 vDiffA = pToArc->n1->GetCenter() - vTo;
            const PblVector3 vDiffB = pToArc->n2->GetCenter() - vTo;
            if (SquaredLength(vDiffA) < SquaredLength(vDiffB))
            {
                mDestNode = pToArc->n1;
            }
            else
            {
                mDestNode = pToArc->n2;
            }
        }
        mDestNode2 = mDestNode == pToArc->n1 ? pToArc->n2 : pToArc->n1;
        mUseDest2ForWeights = pArc1 && pArc2
                              && SquaredLength(mDestNode2->GetCenter() - mDestPos)
                                     < SquaredLength(mDestNode->GetCenter() - mDestPos);
        return GotoNextLeg();
    }

    if (mDestClassification == 1)
    {
        if (pFromArc->n1 == pToNode || pFromArc->n2 == pToNode)
        {
            mDestNode = const_cast<ConnectivityNode*>(pToNode);
            mCurDestNode = const_cast<ConnectivityNode*>(pToNode);
            mCurDestPos = mDestPos;
            return true;
        }
        mDestNode = const_cast<ConnectivityNode*>(pToNode);
        mCurDestPos = vFrom;
        const ConnectivityArc* pArc1 = pGraph->GetNextArc(0, *pFromArc->n1, *pToNode, eType);
        const ConnectivityArc* pArc2 = mGraph->GetNextArc(0, *pFromArc->n2, *pToNode, mType);
        if (!pArc2 || pArc1 == pFromArc)
        {
            mCurDestNode = pFromArc->n1;
        }
        else if (!pArc1 || pArc2 == pFromArc)
        {
            mCurDestNode = pFromArc->n2;
        }
        else
        {
            const PblVector3 vDiffA = pFromArc->n1->GetCenter() - vFrom;
            const PblVector3 vDiffB = pFromArc->n2->GetCenter() - vFrom;
            if (SquaredLength(vDiffA) < SquaredLength(vDiffB))
            {
                mCurDestNode = pFromArc->n2;
            }
            else
            {
                mCurDestNode = pFromArc->n1;
            }
            mCurArc = const_cast<ConnectivityArc*>(
                mCurDestNode->GetRandomArc(pFromArc, *pGraph, eType));
        }
        return GotoNextLeg();
    }

    if (pFromArc == pToArc)
    {
        mDestNode = 0;
        mCurDestNode = 0;
        mCurDestPos = mDestPos;
        return true;
    }
    if (pFromArc->n1 == pToArc->n1 || pFromArc->n1 == pToArc->n2)
    {
        mDestNode = pFromArc->n1;
        mCurDestNode = pFromArc->n2;
        mCurDestPos = vFrom;
        return GotoNextLeg();
    }
    if (pFromArc->n2 == pToArc->n1 || pFromArc->n2 == pToArc->n2)
    {
        mDestNode = pFromArc->n2;
        mCurDestNode = pFromArc->n1;
        mCurDestPos = vFrom;
        return GotoNextLeg();
    }

    mCurDestPos = vFrom;
    const ConnectivityArc* pArc11 = pGraph->GetNextArc(0, *pFromArc->n1, *pToArc->n1, eType);
    const ConnectivityArc* pArc12 = mGraph->GetNextArc(0, *pFromArc->n1, *pToArc->n2, mType);
    const ConnectivityArc* pArc21 = mGraph->GetNextArc(0, *pFromArc->n2, *pToArc->n1, mType);
    const ConnectivityArc* pArc22 = mGraph->GetNextArc(0, *pFromArc->n2, *pToArc->n2, mType);
    if (pArc11 == pFromArc && pArc12 == pFromArc)
    {
        mCurDestNode = pFromArc->n1;
    }
    else if (pArc21 == pFromArc && pArc22 == pFromArc)
    {
        mCurDestNode = pFromArc->n2;
    }
    else
    {
        if (pArc11 == pArc12)
        {
            mCurDestNode = pFromArc->n2;
        }
        else if (pArc21 == pArc22)
        {
            mCurDestNode = pFromArc->n1;
        }
        else
        {
            const PblVector3 vDiffA = pFromArc->n1->GetCenter() - vFrom;
            const PblVector3 vDiffB = pFromArc->n2->GetCenter() - vFrom;
            if (SquaredLength(vDiffA) < SquaredLength(vDiffB))
            {
                mCurDestNode = pFromArc->n2;
            }
            else
            {
                mCurDestNode = pFromArc->n1;
            }
        }
        mCurArc = const_cast<ConnectivityArc*>(
            mCurDestNode->GetRandomArc(pFromArc, *mGraph, eType));
    }

    const ConnectivityNode* pOther = mCurDestNode == pFromArc->n1 ? pFromArc->n2 : pFromArc->n1;
    const ConnectivityArc* pArc1 = mGraph->GetNextArc(0, *pToArc->n1, *pOther, mType);
    const ConnectivityArc* pArc2 = mGraph->GetNextArc(0, *pToArc->n2, *pOther, mType);
    if (!pArc1 || pArc1 == pToArc)
    {
        mDestNode = pToArc->n2;
    }
    else if (!pArc2 || pArc2 == pToArc)
    {
        mDestNode = pToArc->n1;
    }
    else
    {
        const PblVector3 vDiffA = pToArc->n1->GetCenter() - vTo;
        const PblVector3 vDiffB = pToArc->n2->GetCenter() - vTo;
        if (SquaredLength(vDiffA) < SquaredLength(vDiffB))
        {
            mDestNode = pToArc->n1;
        }
        else
        {
            mDestNode = pToArc->n2;
        }
    }
    mDestNode2 = mDestNode == pToArc->n1 ? pToArc->n2 : pToArc->n1;
    mUseDest2ForWeights =
        pArc1 && pArc2
        && SquaredLength(mDestNode2->GetCenter() - mDestPos) < SquaredLength(mDestNode->GetCenter() - mDestPos);
    mCurArc = const_cast<ConnectivityArc*>(pFromArc);
    mCurDestNode = mCurDestNode == pFromArc->n1 ? pFromArc->n2 : pFromArc->n1;
    mCurDestPos = mCurDestNode->GetRandomPointInside(ConnectivityGraphFollower::GetNodeBufferRadius(mCtrl));
    return true;
}

// FUNCTION: SWBF2 0x004827e0
const PblVector3& ConnectivityGraphFollower::GetCurrentDestination() const
{
    return mCurDestPos;
}

// FUNCTION: SWBF2 0x004827f0
bool ConnectivityGraphFollower::GotoNextLeg()
{
    if (mCurDestNode == mDestNode || mCurDestNode == mDestNode2)
    {
        mDestNode = 0;
        mDestNode2 = 0;
        mCurArc = 0;
        mCurDestNode = 0;
    }
    else
    {
        const ConnectivityArc* pArc = mGraph->GetNextArc(
            mCurArc, *mCurDestNode, mUseDest2ForWeights ? *mDestNode2 : *mDestNode, mType);
        if (!pArc)
        {
            return false;
        }
        mCurArc = const_cast<ConnectivityArc*>(pArc);
        mCurDestNode = const_cast<ConnectivityNode*>(pArc->GetOtherNode(mCurDestNode));
        if (mCurDestNode != mDestNode || mDestClassification != 1)
        {
            mCurDestPos = mCurDestNode->GetRandomPointInside(ConnectivityGraphFollower::GetNodeBufferRadius(mCtrl));
            return true;
        }
    }
    mCurDestPos = mDestPos;
    return true;
}

// FUNCTION: SWBF2 0x004828b0
bool ConnectivityGraphFollower::CanGetNewRandomPoint() const
{
    if (mCurDestNode == mDestNode && mDestClassification == 1)
    {
        return false;
    }
    return mCurDestNode ? true : false;
}

// FUNCTION: SWBF2 0x004828d0
bool ConnectivityGraphFollower::GetNewValidRandomPoint(float fRadius)
{
    if (!CanGetNewRandomPoint())
    {
        return false;
    }
    bool bChanged = false;
    int iTries = 0;
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(mCtrl->mOwner->GetGameObject());
    const PblVector3& vDest = GetCurrentDestination();
    while (!PathFinder::IsPointValid(vDest, fRadius, mType, 0, &pIgnore, 1) && iTries++ < 100)
    {
        mCurDestPos = mCurDestNode->GetRandomPointInside(ConnectivityGraphFollower::GetNodeBufferRadius(mCtrl));
        bChanged = true;
    }
    return bChanged;
}

// FUNCTION: SWBF2 0x00482990
float ConnectivityGraphFollower::GetNodeBufferRadius(const UnitController* pCtrl)
{
    if (pCtrl && AIUtil::GetEntityType(pCtrl->mOwner->GetEntity()) == AIUtil::ENTITY_DROIDEKA)
    {
        return 4.0f;
    }
    return 0.0f;
}
