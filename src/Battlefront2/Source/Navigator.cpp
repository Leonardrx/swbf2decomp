#include <float.h>
#include <math.h>
#include <d3dx9math.h>

#include "Navigator.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Character.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "HintManager.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "PathManager.h"
#include "PowerupItem.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

class CollisionManager
{
public:

    static int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppList,
                                 int iMax, const GameObject* pIgnore, int iFlags);
};

namespace PowerupUtility
{

    bool CheckHealth(GameObject* pObj, float fAmount);
    bool CheckAmmo(GameObject* pObj, float fAmount, bool bFlag);
}

namespace
{


    inline Entity* AsEntity(GameObject* pObj)
    {
        return reinterpret_cast<Entity*>(pObj);
    }

    inline CollisionObject* AsCollisionObject(GameObject* pObj)
    {
        return &static_cast<CollisionObject&>(*pObj);
    }

    inline int GetHealthType(const GameObject* pObj)
    {
        return pObj->mHealthType;
    }
}

// SYNTHETIC: SWBF2 0x004043a0
// `dynamic initializer for 'Navigator::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768d80
// `dynamic atexit destructor for 'Navigator::sMemoryPool''

// GLOBAL: SWBF2 0x01ecb200
// Navigator::sMemoryPool
MemoryPool Navigator::sMemoryPool("Navigator", sizeof(Navigator));

// SYNTHETIC: SWBF2 0x00404390
// `dynamic initializer for 'Navigator::rttiHashNavigator''
// Navigator::rttiHashNavigator

// GLOBAL: SWBF2 0x01ecb1f8
// Navigator::rttiHashNavigator
PblHash Navigator::rttiHashNavigator("Navigator");
// GLOBAL: SWBF2 0x0078a5dc
// Navigator::rttiNameNavigator
const char Navigator::rttiNameNavigator[] = "Navigator";







// SYNTHETIC: SWBF2 0x005ab310
// Navigator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ab340
ObstacleTest::ObstacleTest(const PblVector3& rSrc, const PblVector3& rDest)
    : mSrc(rSrc), mDest(rDest)
{
    mBoundCenter = ((mSrc + mDest) * 0.5f);
    mBoundRadius = Length(mBoundCenter - mSrc);
}

// FUNCTION: SWBF2 0x005ab410
Navigator::Navigator(AILowLevel* pOwner)
    : mOwner(pOwner),
      mPathFollower(0),
      mWallFollowTime(0.0f),
      mPowerUpTime(0.0f),
      mCoverNodeTime(0.0f),
      mFailedPathSearch(false)
{
}

// FUNCTION: SWBF2 0x005ab450
Navigator::~Navigator()
{
    if (mPathFollower)
    {
        delete mPathFollower;
        mPathFollower = 0;
    }
}

// FUNCTION: SWBF2 0x005ab4c0
const PblMatrix& Navigator::GetMatrix() const
{
    return mOwner->mOwner->mOwner->GetMatrix();
}

// FUNCTION: SWBF2 0x005ab4d0
GameObject* Navigator::GetGameObject()
{
    if (mOwner && mOwner->mOwner && mOwner->mOwner->mOwner)
    {
        return mOwner->mOwner->mOwner->GetGameObject();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005ab4f0
const GameObject* Navigator::GetGameObject() const
{
    if (mOwner && mOwner->mOwner && mOwner->mOwner->mOwner)
    {
        return mOwner->mOwner->mOwner->GetGameObject();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005ab510
float Navigator::GetMyRadius() const
{
    return AIUtil::GetCollisionRadius(GetGameObject());
}

// FUNCTION: SWBF2 0x005ab520
int Navigator::GetMyProhibit() const
{
    return AIUtil::GetAIType(GetGameObject());
}

// FUNCTION: SWBF2 0x005ab530
const Obstacle* Navigator::GetMyBarrier() const
{
    return AIUtil::GetGameObjectBarrier(GetGameObject());
}

// FUNCTION: SWBF2 0x005ab540
void Navigator::Update(float dt)
{
    FSMUpdate(dt, false);

    Entity* pEntity = mOwner->mOwner->mOwner->GetEntity();
    if (mOwner->mOwner->mLodState >= 3 && AIUtil::GetEntityType(pEntity) == AIUtil::ENTITY_SOLDIER &&
        static_cast<EntitySoldier*>(pEntity)->mAcklayData == 0)
    {
        float fTime = GameLoop::GetMissionTime();
        if (fTime > mPowerUpTime + 2.0f)
        {
            mPowerUpTime = fTime;
            if (LookForPowerUps(6.0f) == true)
            {
                mPowerUpTime += 4.0f;
            }
        }
        if (fTime > mWallFollowTime + 3.0f)
        {
            mWallFollowTime = fTime;
            if (AttemptWallFollow(2.0f, PblRandom::GetGlobalFloat() > 0.5f) == true)
            {
                mWallFollowTime += 4.0f;
            }
        }
        if (fTime > mCoverNodeTime + 2.0f)
        {
            mCoverNodeTime = fTime;
            if (LookForCoverNodes() == true)
            {
                mCoverNodeTime += 6.0f;
            }
        }
    }
}

// FUNCTION: SWBF2 0x005ab680
void Navigator::UpdateSkip()
{
    if (mPathFollower)
    {
        mPathFollower->UpdateControllable();
    }
}

// FUNCTION: SWBF2 0x005ab690
void Navigator::ReceivePathResult(AIPath*)
{
}

// FUNCTION: SWBF2 0x005ab6a0
PathFollower* Navigator::GetPathFollower()
{
    return mPathFollower;
}

const PathFollower* Navigator::GetPathFollower() const
{
    return mPathFollower;
}

// FUNCTION: SWBF2 0x005ab6b0
bool Navigator::IsStuck() const
{
    return mPathFollower && mPathFollower->mIsStuck;
}

// FUNCTION: SWBF2 0x005ab6d0
bool Navigator::IsFailedPathSearch() const
{
    return mFailedPathSearch;
}

// FUNCTION: SWBF2 0x005ab6e0
void Navigator::SetFailedPathSearch()
{
    mFailedPathSearch = true;
}

// FUNCTION: SWBF2 0x005ab6f0
const PblVector3& Navigator::GetGlobalDest() const
{
    if (mPathFollower)
    {
        return mPathFollower->mFinalPoint;
    }
    return mOwner->mOwner->mOwner->GetMatrix().GetTranslation();
}

// FUNCTION: SWBF2 0x005ab710
const PblVector3& Navigator::GetLocalDest() const
{
    if (!mPathFollower)
    {
        return mOwner->mOwner->mOwner->GetMatrix().GetTranslation();
    }
    return mPathFollower->mGotoPoint;
}

// FUNCTION: SWBF2 0x005ab730
bool Navigator::AttemptWallFollow(float fStep, bool bRight)
{
    if (!mPathFollower || mPathFollower->mDone)
    {
        return false;
    }
    Controllable* pMe = mOwner->mOwner->mOwner;
    if (pMe && pMe->mCharacter && pMe->mCharacter->mCarriedFlagPtr)
    {
        return false;
    }

    PblVector3 vStart = pMe->GetMatrix().GetTranslation();
    PblVector3 vPos = vStart;
    float fRadius = GetMyRadius();
    int eProhibit = GetMyProhibit();

    const PblVector3& vGoto = mPathFollower->mGotoPoint;
    float fGX = vPos.x - vGoto.x;
    float fGY = vPos.y - vGoto.y;
    float fGZ = vPos.z - vGoto.z;
    float fMinDistSq = fRadius * fRadius * 10.0f * 10.0f;
    if (fMinDistSq > fGX * fGX + fGY * fGY + fGZ * fGZ)
    {
        return false;
    }

    float fProbe = fRadius * 6.0f;
    PblVector3 vToGoto = vGoto - vPos;
    PblVector3 vDir = AIUtil::GetSafeNormalXZ(vToGoto);
    PblVector3 vSide(vDir.z, 0.0f, -vDir.x);
    if (bRight)
    {
        vSide = -vSide;
    }

    PblVector3 vProbe = vPos + vSide * fProbe;
    const Obstacle* pHit;
    float fDistSq;
    if (!PathFinder::FindClosestObstacleIntersection(vPos, vProbe, GetMyRadius(), GetMyProhibit(), &pHit,
                                                     &fDistSq, false, 0, 0))
    {
        vSide = -vSide;
        vProbe = vPos + vSide * fProbe;
        bool bHit = PathFinder::FindClosestObstacleIntersection(vPos, vProbe, GetMyRadius(), GetMyProhibit(),
                                                                &pHit, &fDistSq, false, 0, 0);
        bRight = !bRight;
        if (!bHit)
        {
            return false;
        }
    }

    float fDist = (float)sqrt(fDistSq);
    if (fRadius * 3.0f > fDist)
    {
        return false;
    }

    PblVector3 vWall = vPos + vSide * fDist;
    PblVector3 vFar = vPos + vSide * fProbe;
    PblVector3 vNormal, vCorner, vOther;
    pHit->IntersectionNewer(ObstacleTest(vPos, vFar), fRadius, &fDistSq, &vNormal, &vCorner, &vOther);
    if ((vOther.x - vCorner.x) * vDir.x + (vOther.y - vCorner.y) * vDir.y +
            (vOther.z - vCorner.z) * vDir.z > 0.0f)
    {
        vCorner = vOther;
    }

    float fCX = vCorner.x - vWall.x;
    float fCZ = vCorner.z - vWall.z;
    if (fMinDistSq > fCX * fCX + fCZ * fCZ)
    {
        return false;
    }
    PblVector3 vToCorner = vCorner - vWall;
    PblVector3 vCornerDir = AIUtil::GetSafeNormalXZ(vToCorner);
    if (0.95f > vCornerDir.x * vDir.x + vCornerDir.y * vDir.y + vCornerDir.z * vDir.z)
    {
        return false;
    }

    vStart.y += 1.0f;
    vPos = PblVector3(vWall.x + vCornerDir.x * fDist, vStart.y, vWall.z + vCornerDir.z * fDist);
    PblVector3 vSidePt = vPos + vCornerDir * fStep;
    float fAhead = fDist * 2.0f + fStep;
    PblVector3 vAhead = vStart + vDir * fAhead;

    float fAX = vStart.x - vAhead.x;
    float fAZ = vStart.z - vAhead.z;
    float fBX = vStart.x - vCorner.x;
    float fBZ = vStart.z - vCorner.z;
    if (fAX * fAX + fAZ * fAZ > fBX * fBX + fBZ * fBZ)
    {
        return false;
    }

    fRadius -= 0.1f;
    if (PathFinder::TestTrivialStraightLineHit(vStart, vPos, fRadius, eProhibit, 0, 0, false))
    {
        return false;
    }
    if (PathFinder::TestTrivialStraightLineHit(vPos, vSidePt, fRadius, eProhibit, 0, 0, false))
    {
        return false;
    }
    if (PathFinder::TestTrivialStraightLineHit(vSidePt, vAhead, fRadius, eProhibit, 0, 0, false))
    {
        return false;
    }

    AIPath* pPath = new AIPath;
    if (!pPath)
    {
        return false;
    }
    pPath->Append(vAhead, 0);
    unsigned char uWaitFlags = bRight ? AIPathNode::AIPN_WAITRIGHT : AIPathNode::AIPN_WAITLEFT;
    pPath->Append(vSidePt, uWaitFlags);
    pPath->Append(vPos, 0);
    pPath->Append(vStart, 0);
    mPathFollower->PrependPath(pPath);
    pPath->Release();
    return true;
}

// FUNCTION: SWBF2 0x005abea0
bool Navigator::LookForPowerUps(float fRadius)
{
    if (!mPathFollower || mPathFollower->mDone)
    {
        return false;
    }

    GameObject* objects[6];
    int iCount = CollisionManager::GetObjectsInRange(GetMatrix().GetTranslation(), fRadius, objects, 6, 0, 1);
    while (iCount--)
    {
        if (!AsEntity(objects[iCount])->GetEntity()->IsRtti(PowerupItem::GetRtti()))
        {
            continue;
        }

        const PowerupItemClass* pClass =
            reinterpret_cast<const PowerupItemClass*>(AsEntity(objects[iCount])->GetEntityClass());
        float fHealth = pClass->mAddHealth[GetHealthType(GetGameObject())];
        float fAmmo = pClass->mAddAmmo[GetHealthType(GetGameObject())];
        if (!PowerupUtility::CheckHealth(GetGameObject(), fHealth) &&
            !PowerupUtility::CheckAmmo(GetGameObject(), fAmmo, true))
        {
            continue;
        }

        const Obstacle* pBarrier = GetMyBarrier();
        if (PathFinder::TestTrivialStraightLineHit(GetMatrix().GetTranslation(),
                                                   objects[iCount]->GetMatrix().GetTranslation(),
                                                   GetMyRadius(), GetMyProhibit(), &pBarrier, 1, false))
        {
            continue;
        }

        PblVector3 vStart = GetMatrix().GetTranslation();
        PblVector3 vItem = objects[iCount]->GetMatrix().GetTranslation();
        PblVector3 vDelta(vItem.x - vStart.x, vItem.y - vStart.y, vItem.z - vStart.z);
        PblVector3 vNormal;
        D3DXVec3Normalize((D3DXVECTOR3*)&vNormal, (const D3DXVECTOR3*)&vDelta);
        vItem += vNormal;

        AIPath* pPath = new AIPath;
        if (pPath)
        {
            pPath->Append(vStart, 0);
            pPath->Append(vItem, 0);
            pPath->Append(vStart, 0);
            mPathFollower->PrependPath(pPath);
            pPath->Release();
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x005ac120
bool Navigator::LookForCoverNodes()
{
    if (!AsEntity(GetGameObject())->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return false;
    }
    if (!mPathFollower || mPathFollower->mDone)
    {
        return false;
    }
    Controllable* pMe = mOwner->mOwner->mOwner;
    if (pMe && pMe->mCharacter && pMe->mCharacter->mCarriedFlagPtr)
    {
        return false;
    }

    const PblVector3& vMyPos = GetMatrix().GetTranslation();
    const PblVector3& vGoto = mPathFollower->mGotoPoint;
    PblVector3 vToGoto(vGoto.x - vMyPos.x, vGoto.y - vMyPos.y, vGoto.z - vMyPos.z);
    PblVector3 vDir = AIUtil::GetSafeNormalXZ(vToGoto);

    const BaseHint* hints[10];
    int iCount = HintManager::RequestCoverHints(mOwner->mOwner, vMyPos, 5, 10, hints);
    GameObject* pObj = pMe->GetGameObject();
    float fBestDistSq = FLT_MAX;
    const BaseHint* pBest = 0;
    for (int i = 0; i < iCount; ++i)
    {
        if (!AIUtil::IsHintAvailable(pObj, hints[i]))
        {
            continue;
        }
        const PblVector3& vHint = hints[i]->GetCollisionCenter();
        float fX = vMyPos.x - vHint.x;
        float fY = vMyPos.y - vHint.y;
        float fZ = vMyPos.z - vHint.z;
        float fDistSq = fX * fX + fY * fY + fZ * fZ;
        if (fDistSq >= fBestDistSq)
        {
            continue;
        }

        PblVector3 vToHint(vHint.x - vMyPos.x, vHint.y - vMyPos.y, vHint.z - vMyPos.z);
        PblVector3 vHintDir = AIUtil::GetSafeNormalXZ(vToHint);
        const PblVector3& vForward = GetMatrix().GetZAxis();
        if (0.0f > vForward.x * vHintDir.x + vForward.y * vHintDir.y + vForward.z * vHintDir.z)
        {
            continue;
        }
        const PblVector3& vFacing = hints[i]->GetZAxis();
        if (0.5f > vFacing.x * vDir.x + vFacing.y * vDir.y + vFacing.z * vDir.z)
        {
            continue;
        }
        if (PathFinder::TestTrivialStraightLineHit(vMyPos, vHint, pObj) == true)
        {
            continue;
        }

        pBest = hints[i];
        fBestDistSq = fDistSq;
    }
    if (!pBest)
    {
        return false;
    }

    pBest->Reserve(pObj);
    PblVector3 vStart = GetMatrix().GetTranslation();
    PblVector3 vHint = pBest->GetCollisionCenter();
    float fAlong = DotProduct(vHint - vMyPos, vDir);
    PblVector3 vFoot = vMyPos + vDir * fAlong;
    if (PathFinder::TestTrivialStraightLineHit(vHint, vFoot, pObj) == true)
    {
        vFoot = vStart;
    }

    unsigned char uFlags = (AIUtil::GetRandomFloat(0.0f, 1.0f) > 0.5f ? 0 : AIPathNode::AIPN_WAITLONG) +
                           (AIPathNode::AIPN_WAITLEFT | AIPathNode::AIPN_WAITRIGHT);
    AIPath* pPath = new AIPath;
    if (pPath)
    {
        pPath->Append(vFoot, 0);
        pPath->Append(vHint, uFlags);
        pPath->Append(vStart, 0);
        mPathFollower->PrependPath(pPath);
        pPath->Release();
    }
    return true;
}
