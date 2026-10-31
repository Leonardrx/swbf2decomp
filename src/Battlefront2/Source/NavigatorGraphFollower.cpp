#include <d3dx9math.h>
#include <string.h>

#include "NavigatorGraphFollower.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "ConnectivityGraph.h"
#include "ConnectivityGraphFollower.h"
#include "Controllable.h"
#include "EventInfo.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "PathManager.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"


namespace
{

}

// SYNTHETIC: SWBF2 0x004043d0
// `dynamic initializer for 'NavigatorGraphFollower::rttiHashNavigatorGraphFollower''
// NavigatorGraphFollower::rttiHashNavigatorGraphFollower

// GLOBAL: SWBF2 0x01ecb250
// NavigatorGraphFollower::rttiHashNavigatorGraphFollower
PblHash NavigatorGraphFollower::rttiHashNavigatorGraphFollower("NavigatorGraphFollower");
// GLOBAL: SWBF2 0x0078a620
// NavigatorGraphFollower::rttiNameNavigatorGraphFollower
const char NavigatorGraphFollower::rttiNameNavigatorGraphFollower[] = "NavigatorGraphFollower";







// SYNTHETIC: SWBF2 0x005ad250
// NavigatorGraphFollower::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ad310
NavigatorGraphFollower::NavigatorGraphFollower(AILowLevel* pOwner, const PblVector3& globalDest,
                                               const Obstacle** ppIgnore, int iIgnoreCnt,
                                               bool bRollIntoDest)
    : Navigator(pOwner)
{
    mGlobalDest = globalDest;
    mRollIntoDest = bRollIntoDest;
    mGraphFollower = 0;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    mIgnoreCnt = iIgnoreCnt;
    mCurState = STATE_START;
    mNextState = STATE_PATHING;
}

// FUNCTION: SWBF2 0x005ad370
NavigatorGraphFollower::~NavigatorGraphFollower()
{
    if (mGraphFollower)
    {
        delete mGraphFollower;
        mGraphFollower = 0;
    }
}

// FUNCTION: SWBF2 0x005ad730
void NavigatorGraphFollower::ExitState()
{
}

// FUNCTION: SWBF2 0x005ad740
void NavigatorGraphFollower::UpdateState(float dt)
{
    switch (mCurState)
    {
    case STATE_PATHING:
        mPathFollower->Update(dt);
        if (mPathFollower->mDone && mPathFollower->mDoneReason == PF_PATHBLOCKED)
        {
            SetNextState(STATE_PATHING);
        }
        else if (mPathFollower->mDone && !mGraphFollower)
        {
            delete mPathFollower;
            mPathFollower = 0;
            SetNextState(100);

            AI::EventInfo info(AI::EVT_Nav_NavigatorFinished);
            mOwner->mOwner->SendEvent(&info);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x005adac0
const PblVector3& NavigatorGraphFollower::GetGlobalDest() const
{
    return mGlobalDest;
}

// FUNCTION: SWBF2 0x005ad3e0
void NavigatorGraphFollower::EnterState()
{
    switch (mCurState)
    {
    case STATE_PATHING:
    {
        PblVector3 vFrom = mOwner->mOwner->mOwner->GetMatrix().GetTranslation();
        PblVector3 vTo = mGlobalDest;
        const GameObject* pObj = mOwner->mOwner->mOwner->GetGameObject();

        delete mPathFollower;
        mPathFollower = new PathFollower(mOwner->mOwner, vFrom, vFrom, (const Obstacle**)mIgnore,
                                         mIgnoreCnt, false);

        if (mGraphFollower)
        {
            delete mGraphFollower;
        }
        mGraphFollower = new ConnectivityGraphFollower(mOwner->mOwner);
        if (mGraphFollower)
        {
            mGraphFollower->SetDestination(
                ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1), vFrom,
                vTo, (ConnectivityGraph::ProhibitFlag)AIUtil::GetAIType(pObj));

            if (!PathFinder::IsPointValid(mGraphFollower->GetCurrentDestination(),
                                          AIUtil::GetCollisionRadius(pObj), AIUtil::GetAIType(pObj),
                                          0, (const Obstacle**)mIgnore, mIgnoreCnt))
            {
                mGraphFollower->GetNewValidRandomPoint(AIUtil::GetCollisionRadius(pObj));
            }

            const PblVector3& vDest = mGraphFollower->GetCurrentDestination();
            PathFinder::GetClosestValidPoint(vFrom, vFrom, AIUtil::GetCollisionRadius(pObj),
                                             AIUtil::GetAIType(pObj), (const Obstacle**)mIgnore,
                                             mIgnoreCnt);

            unsigned int uArcFlags = mGraphFollower->GetCurrentArcFlags();
            if (uArcFlags)
            {
                unsigned int uFlags = 0;
                if (uArcFlags & 2)
                {
                    uFlags = AIPathNode::AIPN_JUMP;
                }
                if (uArcFlags & 4)
                {
                    uFlags |= AIPathNode::AIPN_JETJUMP;
                }

                AIPath* pPath = new AIPath();
                if (pPath)
                {
                    pPath->Prepend(vDest, 0);
                    PblVector3 vDelta = vDest - vFrom;
                    PblVector3 vDir;
                    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir),
                                      reinterpret_cast<const D3DXVECTOR3*>(&vDelta));
                    PblVector3 vJump = vDir * 0.1f + vFrom;
                    pPath->Prepend(vJump, uFlags);
                    pPath->Prepend(vFrom, 0);

                    AI::EventInfo_PathResult info(pPath);
                    mOwner->mOwner->SendEvent(&info);
                    pPath->Release();
                }
            }
            else
            {
                UnitController* pController = mOwner->mOwner;
                PathManager::RequestPath(pController, vFrom, vDest, AIUtil::GetCollisionRadius(pObj),
                                         AIUtil::GetAIType(pObj), 1, 0, (const Obstacle**)mIgnore,
                                         mIgnoreCnt);
            }
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x005ad7e0
void NavigatorGraphFollower::ReceivePathResult(AIPath* pPath)
{
    if (pPath)
    {
        if (mRollIntoDest)
        {
            pPath->GetNodeList().GetLast()->_pObject->mFlags |= AIPathNode::AIPN_ROLLIN;
        }
        if (mPathFollower)
        {
            mPathFollower->AppendPath(pPath, false);
        }
    }
    else
    {
        mFailedPathSearch = true;
    }

    if (!mGraphFollower)
    {
        return;
    }

    if (!(mGraphFollower->GetCurrentDestination() != mGlobalDest))
    {
        delete mGraphFollower;
        mGraphFollower = 0;
        return;
    }

    const GameObject* pObj = mOwner->mOwner->mOwner->GetGameObject();
    PblVector3 vFrom = mGraphFollower->GetCurrentDestination();
    if (!mGraphFollower->GotoNextLeg())
    {
        return;
    }

    PblVector3 vTo = mGraphFollower->GetCurrentDestination();
    if (!PathFinder::IsPointValid(vTo, AIUtil::GetCollisionRadius(pObj), AIUtil::GetAIType(pObj), 0, 0, 0))
    {
        if (mGraphFollower->GetNewValidRandomPoint(AIUtil::GetCollisionRadius(pObj)) == true)
        {
            vTo = mGraphFollower->GetCurrentDestination();
        }
        else
        {
            delete mGraphFollower;
            mGraphFollower = 0;
            return;
        }
    }

    unsigned int uArcFlags = mGraphFollower->GetCurrentArcFlags();
    if (uArcFlags)
    {
        unsigned int uFlags = 0;
        if (uArcFlags & 2)
        {
            uFlags = AIPathNode::AIPN_JUMP;
        }
        if (uArcFlags & 4)
        {
            uFlags |= AIPathNode::AIPN_JETJUMP;
        }

        AIPath* pLegPath = new AIPath();
        if (pLegPath)
        {
            pLegPath->Prepend(vTo, 0);
            PblVector3 vDelta(vTo.x - vFrom.x, vTo.y - vFrom.y, vTo.z - vFrom.z);
            PblVector3 vDir;
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vDelta));
            PblVector3 vJump = vDir * 0.1f + vFrom;
            pLegPath->Prepend(vJump, uFlags);
            pLegPath->Prepend(vFrom, 0);

            AI::EventInfo_PathResult info(pLegPath);
            mOwner->mOwner->SendEvent(&info);
            pLegPath->Release();
        }
    }
    else
    {
        UnitController* pController = mOwner->mOwner;
        PathManager::RequestPath(pController, vFrom, vTo, AIUtil::GetCollisionRadius(pObj),
                                 AIUtil::GetAIType(pObj), 1, 0, (const Obstacle**)mIgnore,
                                 mIgnoreCnt);
    }
}
