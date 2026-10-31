#include "UnitRandomAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityDroideka.h"
#include "EntitySoldier.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

namespace
{


    inline const PblMatrix& GameObjectGetMatrix(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj)->GetMatrix();
    }

    char EntitySoldierAiGetLastCollisionDirection(const Entity* p)
    {
        return static_cast<const EntitySoldier*>(p)->mAILastCollisionDirection;
    }

    char EntityDroidekaAiGetLastCollisionDirection(const Entity* p)
    {
        return static_cast<const EntityDroideka*>(p)->mAILastCollisionDirection;
    }
}

// SYNTHETIC: SWBF2 0x00406d10
// `dynamic initializer for 'UnitRandomAgent::rttiHashUnitRandomAgent''
// UnitRandomAgent::rttiHashUnitRandomAgent

// GLOBAL: SWBF2 0x01f9a248
// UnitRandomAgent::rttiHashUnitRandomAgent
PblHash UnitRandomAgent::rttiHashUnitRandomAgent("UnitRandomAgent");
// GLOBAL: SWBF2 0x0078b914
// UnitRandomAgent::rttiNameUnitRandomAgent
const char UnitRandomAgent::rttiNameUnitRandomAgent[] = "UnitRandomAgent";







// FUNCTION: SWBF2 0x00669370
bool UnitRandomAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return false;
}

// SYNTHETIC: SWBF2 0x00669380
// UnitRandomAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006693b0
UnitRandomAgent::UnitRandomAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl)
{
    mStartPos = pCtrl->mOwner->GetMatrix().GetTranslation();
    mCurState = 1;
    mNextState = 2;
}

// FUNCTION: SWBF2 0x00669420
bool UnitRandomAgent::IsPointValid(const PblVector3& vPoint)
{
    const GameObject* pObj = GetMe()->GetGameObject();
    PblVector3 vPos = pObj->GetMatrix().GetTranslation();
    return PathFinder::IsPointValid(vPos, AIUtil::GetCollisionRadius(pObj), AIUtil::GetAIType(pObj), 0, 0, 0);
}

// FUNCTION: SWBF2 0x00669480
void UnitRandomAgent::PickRandomDest()
{
    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vRand = PblRandom::GetGlobalVector3();
    PblVector3 vDest = vPos + vRand * 10.0f;
    GetLowLevel().SetNavigator_GotoDirect(vDest, true, false);
    mRandTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x00669530
void UnitRandomAgent::EnterState()
{
    switch (mCurState)
    {
    case 2:
        PickRandomDest();
        break;
    }
}

// FUNCTION: SWBF2 0x00669540
void UnitRandomAgent::ExitState()
{
    mCtrl->mLowLevel.mFoldStatus = FOLD_FOLD;
}

// FUNCTION: SWBF2 0x00669550
void UnitRandomAgent::UpdateStatePostVision(float dt)
{
    mAttackHelper.Update();
    switch (mCurState)
    {
    case 2:
    {
        float fRange = 6.0f;
        GetLowLevel().mFoldStatus = FOLD_UNFOLD;
        if (AIUtil::GetEntityType(GetMe()->GetEntity()) == AIUtil::ENTITY_DROIDEKA)
        {
            fRange = 2.0f;
        }

        PblVector3 vPos = GetMe()->GetGameObject()->GetMatrix().GetTranslation();
        float fX = vPos.x - mStartPos.x;
        float fY = vPos.y - mStartPos.y;
        float fZ = vPos.z - mStartPos.z;
        if (fY * fY + fX * fX + fZ * fZ > fRange * fRange && IsPointValid(vPos))
        {
            SetNextState(100);
            FaceTarget(HEIGHT_STAND);
            return;
        }

        bool bCollided = false;
        if (GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            bCollided = static_cast<EntitySoldier*>(GetMe()->GetEntity())->mAILastCollisionDirection != 0;
        }
        if (GetMe()->GetEntity()->IsRtti(EntityDroideka::rttiHashEntityDroideka))
        {
            bCollided = static_cast<EntityDroideka*>(GetMe()->GetEntity())->mAILastCollisionDirection != 0;
        }

        if (bCollided || GameLoop::GetMissionTime() > mRandTime + 3.0f || GetLowLevel().IsAtDest())
        {
            PickRandomDest();
        }
        break;
    }
    }
    FaceTarget(HEIGHT_STAND);
}

// FUNCTION: SWBF2 0x006696f0
int UnitRandomAgent::EventHandler(AI::EventInfo* pInfo)
{
    if (pInfo->mType != AI::EVT_Path_PathFollowerStuck)
    {
        return UnitAgent::EventHandler(pInfo);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00669710
const char* UnitRandomAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}
