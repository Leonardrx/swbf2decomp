#include "UnitCTFOffenseAgent.h"
#include "PebbleFL/Common/PblHashTable.h"

#include "EventInfo.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "CollisionObject.h"
#include "ControllerManager.h"
#include "DefendHelper.h"
#include "AIUtil.h"
#include "AILowLevel.h"
#include "AttackHelper.h"
#include "Character.h"
#include "EntityItem.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "Team.h"
#include "Controllable.h"
#include "Character.h"
#include "RedEngineFL/Util/RedRegion.h"

// FUNCTION: SWBF2 0x00664810
float UnitCTFOffenseAgent::GetUpdateRate() const
{
    switch (mCurState)
    {
    case 3:
    case 8:
        return 0.8f;
    case 4:
        return 1.0f;
    case 2:
    case 5:
        return 2.0f;
    case 6:
    case 7:
        return mDefendHelper.GetUpdateRate();
    default:
        return UnitAgent::GetUpdateRate();
    }
}

// SYNTHETIC: SWBF2 0x00406c80
// `dynamic initializer for 'UnitCTFOffenseAgent::rttiHashUnitCTFOffenseAgent''
// UnitCTFOffenseAgent::rttiHashUnitCTFOffenseAgent

// GLOBAL: SWBF2 0x01f99388
// UnitCTFOffenseAgent::rttiHashUnitCTFOffenseAgent
PblHash UnitCTFOffenseAgent::rttiHashUnitCTFOffenseAgent("UnitCTFOffenseAgent");
// GLOBAL: SWBF2 0x0078b7c4
// UnitCTFOffenseAgent::rttiNameUnitCTFOffenseAgent
const char UnitCTFOffenseAgent::rttiNameUnitCTFOffenseAgent[] = "UnitCTFOffenseAgent";

// FUNCTION: SWBF2 0x00664730
UnitCTFOffenseAgent::UnitCTFOffenseAgent(UnitController* pCtrl, const FlagItem* pFlag, const PblVector3& vPos)
    : UnitAgent(pCtrl), mCombatHelper(this), mDefendHelper(this)
{
    mCurState = 1;
    mNextState = 1;
    mFlag.mObject = reinterpret_cast<const GameObject*>(pFlag);
    mFlag.mSavedHandleId = pFlag ? pFlag->mHandleId : 0;
    mCapturePos = vPos;

    mDefendCarrierOffset = AIUtil::GetRandomFloat(1.0f, 6.0f);
}



// FUNCTION: SWBF2 0x006647e0
void UnitCTFOffenseAgent::Kill()
{
    mDefendHelper.Kill();
    mCombatHelper.Kill();
    UnitAgent::Kill();
}

// FUNCTION: SWBF2 0x006646f0
int GetFormationType(const UnitController* pCtrl)
{
    if (pCtrl)
    {
        const Controllable* pOwner = pCtrl->mOwner;
        if (pOwner)
        {
            const Character* pData = pOwner->mCharacter;
            if (pData)
            {
                const Team* pTeam = pData->mTeamPtr;
                if (pTeam)
                {
                    const unsigned int value = pTeam->mSide;
                    return (value >= 2 && value <= 3) ? 2 : 3;
                }
            }
        }
    }
    return 0;
}







// FUNCTION: SWBF2 0x00665390
const char* UnitCTFOffenseAgent::NFINAL_GetStateString(int) const
{
    return 0;
}

namespace
{

    const GameObject* GetFlag(PblHandle<const GameObject>& handle)
    {
        const GameObject* pObject = handle.mObject;
        if (pObject)
        {
            if (pObject->mHandleId ==
                handle.mSavedHandleId)
            {
                return pObject;
            }
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
        }
        return 0;
    }

    inline const CollisionObject* GetCollisionObject(const GameObject* pObject)
    {
        return static_cast<const CollisionObject*>(pObject);
    }
}

// FUNCTION: SWBF2 0x006651b0
int UnitCTFOffenseAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
        return AI::EVT_NotHandled;

    case AI::EVT_Grenade:
    case AI::EVT_Damaged:
    case AI::EVT_Vision_CanSeeEnemy:
    case AI::EVT_Audible_CanHearEnemy:
    case AI::EVT_EndInvisibility:
        if (GetFlag(mFlag))
        {
            if (mCurState == 2)
            {
                const PblMatrix& flagMat = GetCollisionObject(GetFlag(mFlag))->GetMatrix();
                const PblMatrix& myMat = mCtrl->mOwner->GetMatrix();
                float dx = myMat.m[3][0] - flagMat.m[3][0];
                float dz = myMat.m[3][2] - flagMat.m[3][2];
                if (dx * dx + dz * dz < 100.0f)
                {
                    return AI::EVT_NotHandled;
                }
            }
            if (mCurState == 3 || mCurState == 8 || mCurState == 4)
            {
                DamagedEventPlayerCentricness(pInfo);
                return AI::EVT_NotHandled;
            }
        }
        if (!UnitAgent::EventHandler(pInfo))
        {
            return AI::EVT_NotHandled;
        }
        mCombatCmd = pInfo->mNextCmd;
        SetNextState(8);
        pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                                      PblHandle<GameObject>());
        return AI::EVT_Handled;

    case AI::EVT_CTF_FlagKill:
        if (mCurState != 3 && mCurState != 4 && mCurState != 6 && mCurState != 7)
        {
            return AI::EVT_NotHandled;
        }
        SetNextState(1);
        ControllerManager::RequestControllerUpdateASAP(mCtrl);
        return AI::EVT_Handled;

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

enum FormationType
{
    FT_RANDOM = 0,
    FT_SINGLEFILE = 1,
    FT_DOUBLEFILE = 2,
    FT_DOUBLEFILE_RAND = 3,
    FT_CIRCLE = 4,
    FT_CONVOY = 5,
    FT_FLYINGV = 6
};

namespace ReserveManager
{
    int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);
    PblVector3 ReserveOffset(const GameObject* pTarget, const GameObject* pSelf,
                             FormationType eFormation);
    void UnreserveReserver(const GameObject* pReserver, ReserveType eType);

    void UnreserveObject(const GameObject* pObj, ReserveType eType);
}

// FUNCTION: SWBF2 0x00664cc0
void UnitCTFOffenseAgent::ExitState()
{
    switch (mCurState)
    {
    case 3:
        ReserveManager::UnreserveObject(mCtrl->mOwner->GetGameObject(), RT_FORMATION);
        break;
    case 4:
        ReserveManager::UnreserveReserver(mCtrl->mOwner->GetGameObject(), RT_FORMATION);
        break;
    case 6:
    case 7:
        mDefendHelper.Clean();
        break;
    case 8:
        mCombatHelper.Clean();
        break;
    }
}

// FUNCTION: SWBF2 0x00664880
bool UnitCTFOffenseAgent::IsFlagNeutral() const
{
    const GameObject* pFlag = mFlag;
    return pFlag && !static_cast<const EntityItem*>(pFlag)->GetOwner();
}

// FUNCTION: SWBF2 0x006648c0
bool UnitCTFOffenseAgent::IsFlagInMyHands() const
{
    const GameObject* pFlag = mFlag;
    return pFlag && static_cast<const EntityItem*>(pFlag)->GetOwner() == mCtrl->mOwner->mCharacter;
}

// FUNCTION: SWBF2 0x00664910
bool UnitCTFOffenseAgent::IsFlagInAlliedHandsButNotMine() const
{
    const GameObject* pFlag = mFlag;
    if (!mFlag)
    {
        return false;
    }
    const Character* pCarrier = static_cast<const EntityItem*>(pFlag)->GetOwner();
    if (!pCarrier)
    {
        return false;
    }
    const Character* pMe = mCtrl->mOwner->mCharacter;
    if (pCarrier == pMe)
    {
        return false;
    }
    return pMe->mTeamPtr->GetAffiliation(pCarrier->mTeamNumber) > 0;
}

// FUNCTION: SWBF2 0x006649a0
bool UnitCTFOffenseAgent::IsFlagInEnemyHands() const
{
    const GameObject* pFlag = mFlag;
    if (pFlag)
    {
        const Character* pCarrier = static_cast<const EntityItem*>(pFlag)->GetOwner();
        if (pCarrier)
        {
            const int iCarrierTeam = pCarrier->mTeamNumber;
            return mCtrl->mOwner->mCharacter->mTeamPtr->GetAffiliation(iCarrierTeam) < 0;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00664d30
void UnitCTFOffenseAgent::FaceTargetIfClose(ControllableHeight eHeight)
{
    mCtrl->GetLowLevel().SetHeight(eHeight);
    const PblMatrix& myMat = mCtrl->mOwner->GetMatrix();
    const GameObject* pTarget = mAttackHelper.GetVisibleTarget();
    PblVector3 vLastSeen = mAttackHelper.HasTarget() ? mAttackHelper.GetLastSeenTargetPos() : PblVector3(0, 0, 0);

    if (mAttackHelper.GetVisibleTarget())
    {
        const PblVector3& vTarget = pTarget->GetPosition();
        if (SquaredLength(myMat.GetTranslation() - vTarget) < 625.0f)
        {
            mCtrl->GetLowLevel().SetHeading(mAttackHelper.GetVisibleTarget());
            return;
        }
    }

    if (mAttackHelper.HasTarget())
    {
        if (SquaredLength(myMat.GetTranslation() - vLastSeen) < 625.0f)
        {
            mCtrl->GetLowLevel().SetHeading(PblVector3(vLastSeen.x + 0.0f, vLastSeen.y + 1.6f, vLastSeen.z + 0.0f), 0.0f);
            return;
        }
    }

    mCtrl->GetLowLevel().SetHeadingDefault();
}

// FUNCTION: SWBF2 0x00664a10
void UnitCTFOffenseAgent::EnterState()
{
    const GameObject* pFlag = mFlag;
    if (!pFlag)
    {
        if (mCurState != 100)
        {
            SetNextState(100);
        }
        return;
    }

    switch (mCurState)
    {
    case 1:
        if (IsFlagNeutral())
        {
            if (reinterpret_cast<const FlagItem*>(pFlag)->GetAllowAIPickup())
            {
                SetNextState(2);
            }
            else
            {
                SetNextState(6);
            }
        }
        else if (IsFlagInMyHands())
        {
            SetNextState(3);
        }
        else if (IsFlagInAlliedHandsButNotMine())
        {
            Controllable* pCarrier = static_cast<const EntityItem*>(pFlag)->GetOwner()->GetControllable();
            if (pCarrier)
            {
                GameObject* pCarrierObj = pCarrier->GetGameObject();
                if (!pCarrierObj->GetGameObjectClass()->mHeroFlag)
                {
                    const PblMatrix& myMat = mCtrl->mOwner->GetMatrix();
                    const PblVector3& vCarrier = GetCollisionObject(pCarrierObj)->GetPosition();
                    float dx = vCarrier.x - myMat.m[3][0];
                    float dy = vCarrier.y - myMat.m[3][1];
                    float dz = vCarrier.z - myMat.m[3][2];
                    if (dx * dx + dy * dy + dz * dz < 2500.0f &&
                        ReserveManager::GetOpenReservableSlots(pCarrierObj, pCarrierObj->GetTeam(), RT_FORMATION) > 0)
                    {
                        SetNextState(4);
                        return;
                    }
                }
            }
            SetNextState(7);
        }
        else
        {
            SetNextState(5);
        }
        break;

    case 2:
        mCtrl->GetLowLevel().SetNavigator_Goto(mFlag->GetMatrix().GetTranslation(), 0, false, false);
        break;

    case 3:
        mCtrl->GetLowLevel().SetNavigator_Goto(mCapturePos, 0, false, false);
        break;

    case 4:
    case 5:
        mPathing = false;
        mPathTime = -100.0f;
        break;

    case 6:
    {
        AI::GoalTarget target(pFlag);
        mDefendHelper.Init(target);
        break;
    }

    case 7:
    {
        const RedRegion* pHome = reinterpret_cast<const FlagItem*>(pFlag)->GetHomeRegion();
        PblVector3 vPos;
        if (pHome)
        {
            vPos = pHome->mMatrix.GetTranslation();
        }
        else
        {
            vPos = GetCollisionObject(pFlag)->GetPosition();
        }
        AI::GoalTarget target(vPos);
        mDefendHelper.Init(target);
        break;
    }

    case 8:
        if (mCombatCmd.GetCmdType() == CMD_NONE)
        {
            mCombatCmd = UnitCombatInfo(UnitCombatInfo::ENGAGE);
        }
        mCombatHelper.Init(static_cast<const UnitCombatInfo&>(mCombatCmd));
        break;
    }
}

// FUNCTION: SWBF2 0x00664ee0
void UnitCTFOffenseAgent::UpdateStatePostVision(float dt)
{
    const GameObject* pFlag = mFlag;
    if (!pFlag)
    {
        if (mCurState != 100)
        {
            SetNextState(100);
        }
        return;
    }

    if (mCurState != 8)
    {
        mAttackHelper.Update();
    }

    switch (mCurState)
    {
    case 2:
        if (!IsFlagNeutral())
        {
            SetNextState(1);
            return;
        }
        if (mCtrl->GetLowLevel().IsNavigatorInProgress())
        {
            FaceTargetIfClose(HEIGHT_STAND);
            break;
        }
        SetNextState(1);
        return;

    case 3:
        if (!IsFlagInMyHands())
        {
            SetNextState(1);
            return;
        }
        FaceTargetIfClose(HEIGHT_STAND);
        break;

    case 4:
    {
        if (!IsFlagInAlliedHandsButNotMine())
        {
            SetNextState(1);
            return;
        }
        Character* pCarrier = static_cast<const EntityItem*>(pFlag)->GetOwner();
        if (!pCarrier || !pCarrier->GetControllable())
        {
            break;
        }
        GameObject* pCarrierObj = pCarrier->GetControllable()->GetGameObject();
        PblVector3 vFollow = ReserveManager::ReserveOffset(pCarrierObj, mCtrl->mOwner->GetGameObject(),
                                                           static_cast<FormationType>(GetFormationType(mCtrl))) * 2.0f;
        mCtrl->GetLowLevel().SetNavigator_FollowTarget(pCarrierObj, vFollow);
        FaceTargetIfClose(HEIGHT_STAND);
        break;
    }

    case 5:
    {
        if (!IsFlagInEnemyHands())
        {
            SetNextState(1);
            return;
        }
        GameObject* pCarrierObj = static_cast<const EntityItem*>(pFlag)->GetOwner()->GetControllable()->GetGameObject();
        AIUtil::Slide(mCtrl->mOwner->GetGameObject(), pCarrierObj, false, &mPathing, &mPathTime, 4.0f, 3.0f);
        FaceTargetIfClose(HEIGHT_STAND);
        break;
    }

    case 6:
        if (IsFlagInAlliedHandsButNotMine())
        {
            SetNextState(1);
            return;
        }
        FaceTargetIfClose(HEIGHT_STAND);
        mDefendHelper.FSMUpdate(dt, false);
        if (mDefendHelper.IsDone())
        {
            SetNextState(1);
        }
        break;

    case 7:
        if (IsFlagInMyHands() || IsFlagNeutral())
        {
            SetNextState(1);
            return;
        }
        FaceTargetIfClose(HEIGHT_STAND);
        mDefendHelper.FSMUpdate(dt, false);
        if (mDefendHelper.IsDone())
        {
            SetNextState(1);
        }
        break;

    case 8:
        if (IsFlagInMyHands() || IsFlagNeutral())
        {
            SetNextState(1);
            return;
        }
        mCombatHelper.FSMUpdate(dt, false);
        if (mCombatHelper.IsDone())
        {
            SetNextState(1);
        }
        break;
    }
}
