#include <string.h>

#include "NavigatorPathFollower.h"
#include "AILowLevel.h"
#include "EventInfo.h"
#include "PathFollower.h"
#include "UnitController.h"

// SYNTHETIC: SWBF2 0x004043e0
// `dynamic initializer for 'NavigatorPathFollower::rttiHashNavigatorPathFollower''
// NavigatorPathFollower::rttiHashNavigatorPathFollower

// GLOBAL: SWBF2 0x01ecb254
// NavigatorPathFollower::rttiHashNavigatorPathFollower
PblHash NavigatorPathFollower::rttiHashNavigatorPathFollower("NavigatorPathFollower");
// GLOBAL: SWBF2 0x0078a650
// NavigatorPathFollower::rttiNameNavigatorPathFollower
const char NavigatorPathFollower::rttiNameNavigatorPathFollower[] = "NavigatorPathFollower";







// SYNTHETIC: SWBF2 0x005adb20
// NavigatorPathFollower::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005adb50
NavigatorPathFollower::NavigatorPathFollower(AILowLevel* pOwner, bool bDoCallbacksOnPts,
                                             const Obstacle** ppIgnore, int iIgnoreCnt,
                                             bool bRollIntoDest)
    : Navigator(pOwner)
{
    mRollIntoDest = bRollIntoDest;
    mDoCallbacksOnPts = bDoCallbacksOnPts;
    mCurState = STATE_WAITFORPATH;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
}

// FUNCTION: SWBF2 0x005adb90
NavigatorPathFollower::NavigatorPathFollower(AILowLevel* pOwner, bool bDoCallbacksOnPts,
                                             const Obstacle** ppIgnore, int iIgnoreCnt,
                                             bool bRollIntoDest, AIPath* pPath)
    : Navigator(pOwner)
{
    mRollIntoDest = bRollIntoDest;
    mDoCallbacksOnPts = bDoCallbacksOnPts;
    mCurState = STATE_START;
    mNextState = STATE_FOLLOWPATH;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    mPathFollower = new PathFollower(mOwner->mOwner, pPath, (const Obstacle**)mIgnore,
                                     mIgnoreCnt, bDoCallbacksOnPts);
}

// FUNCTION: SWBF2 0x005adc50
NavigatorPathFollower::~NavigatorPathFollower()
{
}

// FUNCTION: SWBF2 0x005adc60
void NavigatorPathFollower::EnterState()
{
}

// FUNCTION: SWBF2 0x005adc70
void NavigatorPathFollower::ExitState()
{
}

// FUNCTION: SWBF2 0x005adc80
void NavigatorPathFollower::UpdateState(float dt)
{
    if (mCurState == STATE_FOLLOWPATH)
    {
        mPathFollower->Update(dt);
        if (mPathFollower->mDone)
        {
            delete mPathFollower;
            mPathFollower = 0;
            SetNextState(100);

            AI::EventInfo info(AI::EVT_Nav_NavigatorFinished);
            mOwner->mOwner->SendEvent(&info);
        }
    }
}

// FUNCTION: SWBF2 0x005add00
void NavigatorPathFollower::ReceivePathResult(AIPath* pPath)
{
    if (!pPath)
    {
        mFailedPathSearch = true;
        SetNextState(100);
        return;
    }

    if (mRollIntoDest)
    {
        pPath->GetNodeList().GetLast()->_pObject->mFlags |= AIPathNode::AIPN_ROLLIN;
    }

    if (mCurState == STATE_WAITFORPATH)
    {
        UnitController* pController = mOwner->mOwner;
        mPathFollower = new PathFollower(pController, pPath, (const Obstacle**)mIgnore,
                                         mIgnoreCnt, mDoCallbacksOnPts);
        SetNextState(STATE_FOLLOWPATH);
    }
    else if (mPathFollower)
    {
        mPathFollower->AppendPath(pPath, false);
    }
}
