#include "UnitFollowAgent.h"

#include "EntityDroideka.h"
#include "GameObject.h"

#include "EventInfo.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "Controllable.h"
#include "PlayerController.h"
#include "AILowLevel.h"
#include "AttackHelper.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "Character.h"
#include "CollisionObject.h"
#include "AIUtil.h"
#include "GameLoop.h"
#include "PathFinder.h"
#include "PebbleFL/Common/PblRandom.h"

#include "UnitThreatManager.h"

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

    PblVector3 ReserveOffset(const GameObject* pTarget, const GameObject* pSelf, FormationType eFormation);

    int GetReserveOrder(const GameObject* pTarget, const GameObject* pSelf, ReserveType eType);

    void UnreserveReserver(const GameObject* pReserver, ReserveType eType);
}

namespace SquadCommands
{
bool IsSquadCommand(int iCmd);
}

namespace AI
{
enum VOType
{
    VOT_SC_StopFollowResponse = 25
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}
}

namespace
{

    // FUNCTION: SWBF2 0x00667fc0
    FormationType GetFormationType(const UnitController* pCtrl)
    {
        return pCtrl->mOpenRadius > 10.0f ? FT_FLYINGV : FT_DOUBLEFILE;
    }

    template <typename TYPE>
    inline TYPE* ResolveHandle(PblHandle<TYPE>& handle)
    {
        return static_cast<TYPE*>(handle);
    }

    GameObject* ResolveHandleAt(void* pHandleAddr)
    {
        return pHandleAddr ? ResolveHandle(*reinterpret_cast<PblHandle<GameObject>*>(pHandleAddr)) : 0;
    }

    PblHandle<GameObject> BuildHandle(GameObject* pObject)
    {
        PblHandle<GameObject> h;
        h.mObject = pObject;
        h.mSavedHandleId = pObject ? pObject->mHandleId : 0;
        return h;
    }


    inline Controllable* SeatOf(const GameObject* pTarget)
    {
        return pTarget->mControllableList.GetFirst()->_pObject;
    }

    inline const PblVector3* GetTargetPosition(const GameObject* pTarget)
    {
        return &static_cast<const CollisionObject*>(pTarget)->GetPosition();
    }
}

// SYNTHETIC: SWBF2 0x00406ce0
// `dynamic initializer for 'UnitFollowAgent::rttiHashUnitFollowAgent''
// UnitFollowAgent::rttiHashUnitFollowAgent

// GLOBAL: SWBF2 0x01f993e8
// UnitFollowAgent::rttiHashUnitFollowAgent
PblHash UnitFollowAgent::rttiHashUnitFollowAgent("UnitFollowAgent");
// GLOBAL: SWBF2 0x0078b8a0
// UnitFollowAgent::rttiNameUnitFollowAgent
const char UnitFollowAgent::rttiNameUnitFollowAgent[] = "UnitFollowAgent";







// SYNTHETIC: SWBF2 0x00667f50
// UnitFollowAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00667fe0
UnitFollowAgent::UnitFollowAgent(UnitController* pCtrl, PblHandle<GameObject const> hTarget, bool bTightFollow)
    : UnitAgent(pCtrl)
    , mCoverHelper(this)
    , mCombatHelper(this)
{
    mTightFollowMode = bTightFollow;
    mTarget = hTarget;
    mCurState = 1;
    mNextState = mTightFollowMode ? 2 : 5;
}

// FUNCTION: SWBF2 0x00668070
void UnitFollowAgent::Kill()
{
    mCoverHelper.Kill();
    mCombatHelper.Kill();
    UnitAgent::Kill();
}

// FUNCTION: SWBF2 0x006680a0
float UnitFollowAgent::GetUpdateRate() const
{
    if (mCurState == 4)
    {
        return mCombatHelper.GetUpdateRate();
    }
    return UnitAgent::GetUpdateRate();
}

// FUNCTION: SWBF2 0x006680d0
void UnitFollowAgent::EnterState()
{
    switch (mCurState)
    {
    case 2:
        if (!mTarget)
        {
            SetNextState(100);
            return;
        }
        return;

    case 4:
        if (mCombatCmd.mWhat == 0)
        {
            mCombatCmd = UnitCombatInfo(UnitCombatInfo::ENGAGE);
        }
        mCombatHelper.Init(reinterpret_cast<const UnitCombatInfo&>(mCombatCmd));
        return;

    case 5:
    {
        if (!mTarget)
        {
            SetNextState(100);
            return;
        }

        unsigned int uTries = 0;
        bool bValid;
        PblVector3 vDest;
        do
        {
            const GameObject* pTargetA = mTarget;
            const CollisionObject& rTargetA = *pTargetA;
            const GameObject* pTargetB = mTarget;

            const PblVector3 vRandom = PblRandom::GetGlobalVector3();
            const PblVector3 vPush = (vRandom * 20.0f) * 0.2f;

            const PblVector3 vRow = (rTargetA.GetMatrix()[2] * 20.0f) * 0.5f;

            const PblVector3& rTargetPos = pTargetB->GetPosition();
            ++uTries;
            vDest = PblVector3(rTargetPos.x - vRow.x + vPush.x, rTargetPos.y - vRow.y + vPush.y,
                               rTargetPos.z - vRow.z + vPush.z);
            bValid = PathFinder::IsPointValid(vDest, 0, mCtrl->mOwner->GetGameObject());
        } while (!bValid && uTries < 20);

        if (uTries < 20)
        {
            mCtrl->mLowLevel.SetNavigator_Goto(vDest, 0, false, false);
            return;
        }
        SetNextState(100);
        return;
    }

    case 1:
    case 100:
        ReserveManager::UnreserveReserver(mCtrl->mOwner->GetGameObject(), RT_FORMATION);
        return;

    default:
        return;
    }
}

// FUNCTION: SWBF2 0x006683d0
void UnitFollowAgent::ExitState()
{
    if (mCurState == 6)
    {
        mCoverHelper.HELPERClean();
    }
}

// FUNCTION: SWBF2 0x006683e0
bool UnitFollowAgent::CheckForForwardHintNodes()
{
    Controllable* pMe = mCtrl->mOwner;
    const GameObject* pTarget = ResolveHandle(mTarget);
    if (ReserveManager::GetReserveOrder(pTarget, pMe->GetGameObject(), RT_FORMATION) > 2)
    {
        return false;
    }
    return mCoverHelper.CheckForHintAndInit(COVERTYPE_ForwardCover, ResolveHandle(mTarget));
}

// FUNCTION: SWBF2 0x00668480
bool UnitFollowAgent::IsInLooseRange()
{
    Controllable* pMe = mCtrl->mOwner;
    const GameObject* pTarget = ResolveHandle(mTarget);
    const PblMatrix& mine = pMe->GetMatrix();
    const PblVector3& theirs = static_cast<const CollisionObject&>(*pTarget).GetPosition();
    float dx = theirs.x - mine.m[3][0];
    float dz = theirs.z - mine.m[3][2];
    return (dx * dx + dz * dz) < 400.0f;
}

// FUNCTION: SWBF2 0x00668500
void UnitFollowAgent::UpdateStatePostVision(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    mAttackHelper.Update();

    if (!mTarget)
    {
        SetNextState(100);
        return;
    }

    if (mTarget->GetControllable() && mTarget->GetControllable()->mCharacter)
    {
        Controllable* pDeep = mTarget->GetControllable()->mCharacter->mVehicle;
        if (pDeep)
        {
            Entity* pOwnerEntity = mCtrl->mOwner->GetEntity();
            if (!pOwnerEntity->IsRtti(EntityDroideka::rttiHashEntityDroideka))
            {
                GameObject* pDeepGameObject = static_cast<Trackable*>(pDeep)->GetGameObject();
                UnitCmdInfo cmdBoard = UnitBoardInfo(pDeepGameObject, -1);
                mCtrl->PrependQueueCommand(cmdBoard);
                SetNextState(100);
                return;
            }
            SetNextState(100);
            return;
        }
    }

    switch (mCurState)
    {
    case 2:
        if (!mTightFollowMode)
        {
            SetNextState(5);
            break;
        }
        {
            bool bIsSeatedSoldier;
            GameObject* pSelfObj = mCtrl->mOwner->GetGameObject();
            if (pSelfObj->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
                static_cast<EntitySoldier*>(mCtrl->mOwner->GetGameObject())->mState == SoldierState::JET_HOVER)
            {
                bIsSeatedSoldier = true;
            }
            else
            {
                bIsSeatedSoldier = false;
                Controllable* pMe = mCtrl->mOwner;
                const GameObject* pTarget = mTarget.operator->();
                const PblMatrix& mine = pMe->GetMatrix();
                const PblVector3& theirPos = pTarget->GetPosition();
                float dx = theirPos.x - mine.m[3][0];
                float dy = theirPos.y - mine.m[3][1];
                float dz = theirPos.z - mine.m[3][2];
                if (dx * dx + dy * dy + dz * dz < 100.0f && CheckForForwardHintNodes())
                {
                    SetNextState(6);
                    break;
                }
            }

            float fScale = 2.0f;
            const PblVector3& vTargetVel = mTarget->GetVelocity();
            if (vTargetVel.x * vTargetVel.x + vTargetVel.y * vTargetVel.y + vTargetVel.z * vTargetVel.z < 16.0f)
            {
                fScale = 2.2f;
            }
            if (bIsSeatedSoldier)
            {
                fScale = 1.7f;
            }

            GetLowLevel().SetNavigator_FollowTarget(
                mTarget, ReserveManager::ReserveOffset(mTarget, mCtrl->mOwner->GetGameObject(),
                                                       GetFormationType(mCtrl)) * fScale);

            if (!mAttackHelper.HasTarget())
            {
                if (mTarget->mControllableList.GetFirst()->_pObject)
                {
                    if (mTarget->mControllableList.GetFirst()->_pObject->mReticuleTarget[0])
                    {
                        if (GetMe()->GetGameObject()->GetAffiliationToMe(
                                mTarget->mControllableList.GetFirst()->_pObject->mReticuleTarget[0]) < 0)
                        {
                            AI::UnitThreatManager& rThreats = mCtrl->mThreatManager;
                            rThreats.AddThreat(const_cast<GameObject*>(
                                static_cast<const GameObject*>(mTarget->GetControllable()->mReticuleTarget[0])));
                        }
                    }
                }
            }
            FaceTarget();
        }
        break;
    case 3:
        if (mTightFollowMode == true)
        {
            SetNextState(2);
        }
        else if (!IsInLooseRange())
        {
            SetNextState(5);
        }
        else if (CheckForForwardHintNodes())
        {
            SetNextState(6);
        }
        else
        {
            mCtrl->mLowLevel.SetNavigator_Stationary();
        }
        break;
    case 4:
        if (!IsInLooseRange())
        {
            SetNextState(5);
        }
        else if (CheckForForwardHintNodes())
        {
            SetNextState(6);
        }
        break;
    case 5:
        if (!mCtrl->mLowLevel.IsNavigatorInProgress())
        {
            SetNextState(IsInLooseRange() ? 3 : 5);
        }
        else if (CheckForForwardHintNodes())
        {
            SetNextState(6);
        }
        else
        {
            FaceTarget();
        }
        break;
    case 6:
        FaceTarget(HEIGHT_STAND);
        mCoverHelper.FSMUpdate(dt, false);
        if (mCoverHelper.IsDone())
        {
            SetNextState(mTightFollowMode ? 2 : 5);
        }
        else
        {
            Controllable* pMe = mCtrl->mOwner;
            const GameObject* pTarget = mTarget.operator->();
            const PblMatrix& mine = pMe->GetMatrix();
            const PblVector3& theirs = pTarget->GetPosition();
            float dx = theirs.x - mine.m[3][0];
            float dy = theirs.y - mine.m[3][1];
            float dz = theirs.z - mine.m[3][2];
            if (dx * dx + dy * dy + dz * dz > 400.0f)
            {
                SetNextState(mTightFollowMode ? 2 : 5);
            }
        }
        break;
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00668a60
bool UnitFollowAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return !SquadCommands::IsSquadCommand(cmd.GetCommand());
}

// FUNCTION: SWBF2 0x00668a80
int UnitFollowAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
    case AI::EVT_SC_FollowChrLoose:
    case AI::EVT_EndInvisibility:
        return 0;

    case AI::EVT_Grenade:
        if (mCurState != 3 || UnitAgent::EventHandler(pInfo) != 1)
        {
            return 0;
        }
        mCombatCmd = pInfo->mNextCmd;
        SetNextState(4);
        pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, NULL, PblVector3(0, 0, 0), NULL);
        return 1;

    case AI::EVT_Damaged:
        if (mCurState == 3)
        {
            if (UnitAgent::HandleDamagedEvent(pInfo) != 1)
            {
                return 0;
            }
            mCombatCmd = pInfo->mNextCmd;
            SetNextState(4);
            pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, NULL, PblVector3(0, 0, 0), NULL);
            return 1;
        }
        UnitAgent::DamagedEventPlayerCentricness(pInfo);
        UnitAgent::PlayDamageEventVO(pInfo);
        return 0;

    case AI::EVQ_SC_IsFollowingChr:
        if (pInfo->mGameObject != mTarget)
        {
            return 0;
        }
        ++pInfo->mInt;
        return 1;

    case AI::EVT_SC_FollowChr:
        if (mTightFollowMode)
        {
            return 0;
        }
        mTightFollowMode = true;
        return 1;

    case AI::EVT_SC_StopFollowChr:
        if (pInfo->mGameObject == mTarget)
        {
            AI::VOHelper::Play(AI::VOT_SC_StopFollowResponse, mCtrl->mOwner->GetGameObject(), 0, 0,
                               AIUtil::GetRandomFloat(0.5f, 1.2f) + GameLoop::GetMissionTime());
            SetNextState(100);
            return 1;
        }
        return -1;

    case AI::EVT_Vision_CanSeeEnemy:
    case AI::EVT_Audible_CanHearEnemy:
        if (mCurState != 3 || UnitAgent::EventHandler(pInfo) != 1)
        {
            return 0;
        }
        mCombatCmd = pInfo->mNextCmd;
        SetNextState(4);
        pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, NULL, PblVector3(0, 0, 0), NULL);
        return 1;

    case AI::EVQ_GetSkillModifier:

        if (mTarget &&
            PlayerControllerPtr(reinterpret_cast<Controllable*>(mTarget->mControllableList.GetFirst()->_pObject)))
        {
            pInfo->mInt = 6;
            return 1;
        }
        pInfo->mInt = 0;
        return 0;

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// FUNCTION: SWBF2 0x00668d40
const char* UnitFollowAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}
