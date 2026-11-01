#include <d3dx9math.h>
#include <float.h>
#include <string.h>

#include "NavigatorSlide.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"

extern PblVector3 gVectorZero;


class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

class EntityDroideka
{
public:
    static PblHash rttiHashEntityDroideka;
};

class EntityDroid
{
public:
    static PblHash rttiHashEntityDroid;
};

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

namespace
{


    inline const PblMatrix& GameObjectGetMatrix(const GameObject* pObj)
    {
        return static_cast<const CollisionObject&>(*pObj).GetMatrix();
    }

    inline const PblVector3& GameObjectGetVelocity(const GameObject* pObj)
    {
        return pObj->GetVelocity();
    }

    float GameObjectGetMaxSpeed(const GameObject* pObj)
    {
        return pObj->GetMaxSpeed();
    }
}

// GLOBAL: SWBF2 0x007ec4c4

int gUnchangedVehicleSlideCount = 1;
// GLOBAL: SWBF2 0x007ec4c8

int gClearVehicleSlideCount = 1;

// SYNTHETIC: SWBF2 0x004043f0
// `dynamic initializer for 'NavigatorSlide::rttiHashNavigatorSlide''
// NavigatorSlide::rttiHashNavigatorSlide

// GLOBAL: SWBF2 0x01ecb258
// NavigatorSlide::rttiHashNavigatorSlide
PblHash NavigatorSlide::rttiHashNavigatorSlide("NavigatorSlide");
// GLOBAL: SWBF2 0x0078a678
// NavigatorSlide::rttiNameNavigatorSlide
const char NavigatorSlide::rttiNameNavigatorSlide[] = "NavigatorSlide";







// SYNTHETIC: SWBF2 0x005ade40
// NavigatorSlide::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ade70
NavigatorSlide* NavigatorSlidePtr(Navigator* pNavigator)
{
    if (pNavigator && pNavigator->IsRtti(NavigatorSlide::rttiHashNavigatorSlide))
    {
        return (NavigatorSlide*)pNavigator;
    }
    return 0;
}

// FUNCTION: SWBF2 0x005ade90
NavigatorSlide::NavigatorSlide(AILowLevel* pOwner, const PblVector3& globalDest,
                               const Obstacle** ppIgnore, int iIgnoreCnt)
    : Navigator(pOwner), mGlobalDest(globalDest)
{
    mUseAttackHelper = false;
    mOffset = 0.0f;
    mCurState = STATE_START;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));

    Entity* pEntity = pOwner->mOwner->mOwner->GetEntity();
    if (pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier) ||
        pEntity->IsRtti(EntityDroideka::rttiHashEntityDroideka) ||
        pEntity->IsRtti(EntityDroid::rttiHashEntityDroid))
    {
        mNextState = STATE_InfantryToPosition;
    }
    else
    {
        mNextState = pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer) ? 100 : STATE_VehicleToPosition;
    }
}

// FUNCTION: SWBF2 0x005adfa0
NavigatorSlide::NavigatorSlide(AILowLevel* pOwner, const GameObject* pSlideObj, float fOffset,
                               bool bUseAttackHelper, const Obstacle** ppIgnore, int iIgnoreCnt)
    : Navigator(pOwner), mGlobalDest(0.0f, 0.0f, 0.0f), mSlideObj(pSlideObj)
{
    mUseAttackHelper = bUseAttackHelper;
    mOffset = fOffset;
    mCurState = STATE_START;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    mNextState = STATE_InfantryToObject;
}

// FUNCTION: SWBF2 0x005ae010
NavigatorSlide::~NavigatorSlide()
{
}

// FUNCTION: SWBF2 0x005ae020
bool NavigatorSlide::IsEqual(AILowLevel* pOwner, const PblVector3& globalDest)
{
    return mOwner == pOwner && !(mGlobalDest != globalDest) && mCurState != 100;
}

// FUNCTION: SWBF2 0x005ae070
bool NavigatorSlide::IsEqual(AILowLevel* pOwner, const GameObject* pSlideObj, float fOffset,
                             bool bUseAttackHelper)
{
    return mOwner == pOwner && mSlideObj == pSlideObj && mOffset == fOffset &&
           mUseAttackHelper == bUseAttackHelper && mCurState != 100;
}

// FUNCTION: SWBF2 0x005ae0d0
void NavigatorSlide::EnterState()
{
    switch (mCurState)
    {
    case STATE_InfantryToPosition:
        mBounceCount = 0;
        mBounceAtLocalDest = false;
        NextInfantrySlidePos(mGlobalDest);
        break;

    case STATE_VehicleToPosition:
        mSlideDirection = SLIDE_NONE;
        mSlidePos = mGlobalDest;
        mSlidePathDest = gVectorZero;
        NextVehicleSlidePos();
        break;

    case STATE_InfantryToObject:
        mGlobalDest = PblVector3(-FLT_MAX, 0, -FLT_MAX);
        mPrevMeleeTime = -100.0f;
        UpdateInfantrySlideObject();
        break;

    case 100:
        if (mPathFollower)
        {
            delete mPathFollower;
            mPathFollower = 0;
        }
        {
            AI::EventInfo info(AI::EVT_Nav_NavigatorFinished);
            mOwner->mOwner->SendEvent(&info);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x005ae240
void NavigatorSlide::ExitState()
{
}

// FUNCTION: SWBF2 0x005ae250
void NavigatorSlide::UpdateState(float dt)
{
    if (mPathFollower)
    {
        mPathFollower->Update(dt);
    }

    switch (mCurState)
    {
    case STATE_InfantryToPosition:
        UpdateInfantrySlidePos();
        break;
    case STATE_VehicleToPosition:
        UpdateVehicleSlidePos();
        break;
    case STATE_InfantryToObject:
        UpdateInfantrySlideObject();
        break;
    }
}

// FUNCTION: SWBF2 0x005ae2a0
void NavigatorSlide::UpdateInfantrySlidePos()
{
    if (mPathFollower->mDone)
    {
        if (mPathFollower->mFinalPoint != mGlobalDest && ++mBounceCount < 6)
        {
            NextInfantrySlidePos(mBounceAtLocalDest ? GetBouncePosition() : mGlobalDest);
        }
        else
        {
            SetNextState(100);
        }
    }
}

// FUNCTION: SWBF2 0x005ae330
PblVector3 NavigatorSlide::GetBouncePosition() const
{
    float fDist = (float)AIUtil::GetRandomInt(10, 20);
    float fAngle = AIUtil::GetRandomFloat(PblMath::fPi / 4.0f, PblMath::fPi / 2.0f);
    PblVector3 dir;
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (AIUtil::GetRandomInt(0, 1) ? -1 : 1) * fAngle);
        dir = PblVector3(mBarrierNormal.x * fCos - mBarrierNormal.z * fSin,
                         mBarrierNormal.y,
                         mBarrierNormal.z * fCos + mBarrierNormal.x * fSin) * fDist;
    }

    return dir + GetMatrix().GetTranslation();
}

// FUNCTION: SWBF2 0x005ae430
void NavigatorSlide::NextInfantrySlidePos(const PblVector3& dest)
{
    PblVector3 pos = GetMatrix().GetTranslation();
    PblVector3 slideDest = dest;

    const GameObject* pObj = mOwner->mOwner->mOwner->GetGameObject();
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(pObj);
    PathFinder::GetClosestValidPoint(pos, pos, AIUtil::GetCollisionRadius(pObj),
                                     AIUtil::GetAIType(pObj), &pIgnore, 1);
    mBounceAtLocalDest = PathFinder::PushValidPoint(pos, slideDest, mBarrierNormal,
                                                    AIUtil::GetCollisionRadius(pObj),
                                                    AIUtil::GetAIType(pObj), &pIgnore, 1);
    if (mBounceAtLocalDest)
    {
        const float dx = slideDest.x - pos.x;
        const float dy = slideDest.y - pos.y;
        const float dz = slideDest.z - pos.z;
        if (dx * dx + dy * dy + dz * dz > 0.1f)
        {
            PblVector3 delta(dx, dy, dz);
            PblVector3 dir;
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&dir), reinterpret_cast<const D3DXVECTOR3*>(&delta));
            if (dx * dx + dy * dy + dz * dz > 1.0f)
            {
                slideDest = PblVector3(slideDest.x - dir.x, slideDest.y - dir.y, slideDest.z - dir.z);
            }
            else
            {
                slideDest = pos;
            }
        }
        else
        {
            slideDest = pos;
        }
    }

    delete mPathFollower;
    mPathFollower = new PathFollower(mOwner->mOwner, pos, slideDest, 0, 0, false);
}

// FUNCTION: SWBF2 0x005ae660
void NavigatorSlide::UpdateInfantrySlideObject()
{
    PblVector3 slidePos;
    if (!GetObjectSlideToPosition(&slidePos))
    {
        SetNextState(100);
        return;
    }

    const GameObject* pObj = AIUtil::GetAlivePtr(mSlideObj, true);
    if (mOwner->mOwner->mOwner->GetCurrentCollision() == pObj)
    {
        mOwner->SetDestCurrent();
        return;
    }

    PblVector3 pos = GetMatrix().GetTranslation();
    float dx = slidePos.x - mGlobalDest.x;
    float dz = slidePos.z - mGlobalDest.z;
    if (dz * dz + dx * dx > 1.0f)
    {
        float fDelay = 1.5f;
        float fx = pos.x - slidePos.x;
        float fz = pos.z - slidePos.z;
        if (fz * fz + fx * fx > 25.0f)
        {
            fDelay = 3.0f;
        }
        if (GameLoop::GetMissionTime() > mPrevMeleeTime + fDelay)
        {
            mPrevMeleeTime = GameLoop::GetMissionTime();
            PblVector3 dest = slidePos;
            const GameObject* pTarget = mOwner->mOwner->GetAttackHelperVisibleTarget();
            if (pTarget)
            {
                dest += pTarget->GetVelocity();
            }
            NextInfantrySlidePos(dest);
            mGlobalDest = dest;
        }
    }
}

// FUNCTION: SWBF2 0x005ae7f0
bool NavigatorSlide::GetObjectSlideToPosition(PblVector3* pPos) const
{
    const GameObject* pObj = AIUtil::GetAlivePtr(mSlideObj, true);
    if (!pObj)
    {
        return false;
    }

    if (mUseAttackHelper)
    {
        if (pObj != mOwner->mOwner->GetAttackHelperTarget())
        {
            return false;
        }
        *pPos = mOwner->mOwner->GetAttackHelperTargetLastSeenPosition();
        return true;
    }

    *pPos = (GameObjectGetMatrix(pObj)).GetTranslation();
    const PblVector3& myPos = GetMatrix().GetTranslation();
    PblVector3 dir(myPos.x - pPos->x, myPos.y - pPos->y, myPos.z - pPos->z);
    const PblVector3 delta = mOffset * AIUtil::GetSafeNormalXZ(dir);
    pPos->x += delta.x;
    pPos->y += delta.y;
    pPos->z += delta.z;
    return true;
}

// FUNCTION: SWBF2 0x005ae900
void NavigatorSlide::NextVehicleSlidePos()
{
    PblVector3 pos = GetMatrix().GetTranslation();
    const float fRadius = GetMyRadius() * 2.0f;
    const int eProhibit = GetMyProhibit();
    PathFinder::GetClosestValidPoint(pos, pos, fRadius, eProhibit, (const Obstacle**)mIgnore, mIgnoreCnt);

    for (int i = 0; i < 5; ++i)
    {
        const Obstacle* pHit = 0;
        float fStep = GetGameObject()->GetMaxSpeed() * 1.5f;
        if (fStep > 30.0f)
        {
            fStep = 30.0f;
        }
        else if (fStep < 1.0f)
        {
            fStep = 1.0f;
        }

        const PblVector3 delta = mSlidePos - pos;
        PblVector3 dest = mSlidePos;
        if (SquaredLengthXZ(delta) > fStep * fStep)
        {
            dest = pos + AIUtil::GetSafeNormalXZ(delta) * fStep;
        }

        float fDistSq;
        if (!PathFinder::FindClosestObstacleIntersection(pos, dest, fRadius, eProhibit, &pHit, &fDistSq, false,
                                                         (const Obstacle**)mIgnore, mIgnoreCnt))
        {
            ++gClearVehicleSlideCount;
            break;
        }

        PblVector3 left;
        PblVector3 right;
        pHit->FindCorners(pos, dest, fRadius, &left, &right);
        if (mSlideDirection == SLIDE_NONE)
        {
            const float lx = left.x - pos.x;
            const float lz = left.z - pos.z;
            const float fLeftDistSq = lz * lz + lx * lx;
            const float rx = right.x - pos.x;
            const float rz = right.z - pos.z;
            const float fRightDistSq = rz * rz + rx * rx;
            mSlideDirection = (fLeftDistSq < fRightDistSq) ? SLIDE_LEFT : SLIDE_RIGHT;
        }
        if (mSlideDirection == SLIDE_LEFT)
        {
            mSlidePos = left;
        }
        else
        {
            mSlidePos = right;
        }
    }

    if (!(mSlidePathDest != mSlidePos))
    {
        ++gUnchangedVehicleSlideCount;
        return;
    }

    delete mPathFollower;
    mPathFollower = new PathFollower(mOwner->mOwner, pos, mSlidePos, (const Obstacle**)mIgnore, mIgnoreCnt, false);
    mSlidePathDest = mSlidePos;
}

// FUNCTION: SWBF2 0x005aec40
void NavigatorSlide::UpdateVehicleSlidePos()
{
    if (mPathFollower->mDone)
    {
        if (!(mPathFollower->mFinalPoint != mGlobalDest))
        {
            SetNextState(100);
            return;
        }
        mSlidePos = mGlobalDest;
    }
    NextVehicleSlidePos();
}
