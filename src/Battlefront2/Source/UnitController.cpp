#include "AmmoCounter.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblVector.h"

#include <float.h>
#include <new>

#include "AIGoal.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionObject.h"
#include "CommandPost.h"
#include "Controllable.h"
#include "ControllerManager.h"
#include "Damageable.h"
#include "Entity.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "CameraManager.h"
#include "SquadCommands.h"
#include "Team.h"
#include "Thread.h"
#include "RedEngineFL/Portals/Portal.h"
#include "UnitBoardAgent.h"
#include "UnitCTFOffenseAgent.h"
#include "UnitCombatAgent.h"
#include "UnitDeathmatchAgent.h"
#include "UnitDefendAgent.h"
#include "UnitDestroyAgent.h"
#include "UnitFollowAgent.h"
#include "UnitHoldAgent.h"
#include "UnitRandomAgent.h"
#include "UnitTrooperAgent.h"
#include "UnitUnBoardAgent.h"
#include "UnitWaitAgent.h"
#include "UnitWaitSecondaryAgent.h"
#include "EntityBuildingArmed.h"
#include "OrdnanceEmitter.h"
#include "WeaponDispenser.h"

// GLOBAL: SWBF2 0x007b28b0
// __xmm@3f800000400000004040000040800000

class EntitySoldier { public: static PblHash rttiHashEntitySoldier; };

class EntityWalker { public: static PblHash rttiHashEntityWalker; };
class EntityHover { public: static PblHash rttiHashEntityHover; };
class EntityFlyer { public: static PblHash rttiHashEntityFlyer; };
class WeaponInvisibility { public: static PblHash rttiHashWeaponInvisibility; };


 


namespace AudibleManager
{
void UpdateAudience(Controller* pCtrl);
}
namespace VisionManager
{
void RemoveRequests(UnitController* pCtrl);
void UpdatePotentiallyVisible(UnitController* pCtrl);
}

namespace AI
{
enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};
}

namespace ControllerManager
{

int GetUnitsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                    int iMax, const Team* pTeam, AI::AIAffil eAffil);
}

namespace AIUtil
{
bool __fastcall ShouldStayInVehicle(Controllable* pMe);
}

namespace AIUtil
{
bool __cdecl CanPotentiallySee(Controllable* pMe, const GameObject* pObject, float* pOut);
}

namespace AIDebug
{
bool SkipCameraForLODs();
}

// FUNCTION: SWBF2 0x004386a0
bool AIDebug::SkipCameraForLODs()
{
    return false;
}

// FUNCTION: SWBF2 0x00663d00
void UnitController::ResetFriendlyList()
{
    mFriendlyCount = 0;
}



namespace
{




    inline const PblVector3& GetPosition(const GameObject* pObject)
    {
    return static_cast<const CollisionObject*>(pObject)->GetPosition();
    }

    GameObject* Resolve(PblHandle<GameObject>& h)
    {
        GameObject* pObject = h.mObject;
        if (pObject)
        {
        if (pObject->mHandleId == h.mSavedHandleId)
        {
                return pObject;
        }
            h.mObject = 0;
            h.mSavedHandleId = 0;
        }
        return 0;
    }


}

// SYNTHETIC: SWBF2 0x00406c50
// `dynamic initializer for 'UnitController::rttiHashUnitController''
// UnitController::rttiHashUnitController

// GLOBAL: SWBF2 0x01f99384
// UnitController::rttiHashUnitController
PblHash UnitController::rttiHashUnitController("UnitController");
// GLOBAL: SWBF2 0x0078b7a8
// UnitController::rttiNameUnitController
const char UnitController::rttiNameUnitController[] = "UnitController";

// SYNTHETIC: SWBF2 0x00406c60
// `dynamic initializer for 'UnitController::sMemoryPool''
// SYNTHETIC: SWBF2 0x007696c0
// `dynamic atexit destructor for 'UnitController::sMemoryPool''

// GLOBAL: SWBF2 0x01f9a1f8
// UnitController::sMemoryPool
MemoryPool UnitController::sMemoryPool("UnitController", sizeof(UnitController));







// FUNCTION: SWBF2 0x006630a0
static int GetLODDistHigh()
{
    return AIUtil::IsUberMode() ? 5 : 25;
}

// FUNCTION: SWBF2 0x006630c0
static int GetLODDistLow()
{
    return AIUtil::IsUberMode() ? 20 : 100;
}

// FUNCTION: SWBF2 0x006630e0
UnitController* UnitControllerPtr(Controller* pController)
{
    if (pController && pController->IsRtti(UnitController::rttiHashUnitController))
    {
        return (UnitController*)pController;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663100
const UnitController* UnitControllerPtr(const Controller* pController)
{
    if (pController && pController->IsRtti(UnitController::rttiHashUnitController))
    {
        return (const UnitController*)pController;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663120
UnitController* UnitControllerPtr(Controllable* pControllable)
{
    if (pControllable && pControllable->mCtrl &&
        pControllable->mCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        return (UnitController*)pControllable->mCtrl;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663150
const UnitController* UnitControllerPtr(const Controllable* pControllable)
{
    if (pControllable && pControllable->mCtrl &&
        pControllable->mCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        return (const UnitController*)pControllable->mCtrl;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663180
UnitController::UnitController(Controllable* pOwner)
    : Controller(pOwner), mLowLevel(this)
{

    mOwner = pOwner;
    mAgent = 0;
    mLodHumanPlayer = 0;
    mCrowdCheckTime = -100.0f;
    if (mOwner->GetEntity()->IsRtti(EntityWalker::rttiHashEntityWalker) ||
        mOwner->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover) ||
        mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        mJustEnteredVehicleGoToPickupAgent = true;
    }
    else
    {
        mJustEnteredVehicleGoToPickupAgent = false;
    }
    mLookAheadOnly = false;
    SetRandomSeed(AIUtil::GetRandomInt(0, 100));
    const PblMatrix& rOwnerMatrix = mOwner->GetMatrix();
    mOriginalAheadDir = rOwnerMatrix.GetZAxis();
    mSkip = false;
    mFriendlyCount = 0;
    mUpdateRate = 0.5f;
    mLodState = 3;
    mLowLevelUpdateSkips = 0;
    mLowLevelUpdateSkipDt = 0.0f;
    mVisionSkip = 0;
    mLastUpdateTime = GameLoop::GetMissionTime();
    mNextUpdateTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x006633d0
UnitController::~UnitController()
{
    DeleteAgent();
}

// SYNTHETIC: SWBF2 0x006633a0
// UnitController::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006645cb
// ??_EUnitController@@OHI@AEPAXI@Z

// SYNTHETIC: SWBF2 0x006645d3
// ??_EUnitController@@OBME@AEPAXI@Z

// FUNCTION: SWBF2 0x006634a0
void UnitController::KillController()
{
    if (mAgent)
    {
        mAgent->Kill();
    }
    Fsm::Kill();
}

// FUNCTION: SWBF2 0x006634c0
void UnitController::DeleteAgent()
{
    if (mAgent)
    {
        delete mAgent;
        mAgent = 0;
    }
}

// FUNCTION: SWBF2 0x006634e0
float UnitController::GetUpdateRate() const
{
    float fAgentRate = 1.0f;
    if (mAgent)
    {
        fAgentRate = mAgent->GetUpdateRate();
    }
    float fIntervals[5] = { 4.0f, 3.0f, 2.0f, 1.0f, 0.25f };
    return fIntervals[mLodState] * fAgentRate;
}

// FUNCTION: SWBF2 0x00663540
void UnitController::EnterCombat()
{
    mAgent = new UnitCombatAgent(this, *static_cast<const UnitCombatInfo*>(GetCurCmd()));
}

// FUNCTION: SWBF2 0x00663030
PblHandle<GameObject> UnitFollowInfo::GetWho() const
{
    return mWho;
}

// FUNCTION: SWBF2 0x00663580
void UnitController::EnterState()
{
    switch (mCurState)
    {
    case STATE_WAIT:
        mAgent = new UnitWaitAgent(this);
        break;
    case STATE_COMBAT:
        EnterCombat();
        return;
    case STATE_CAPTURECP:
    {
        const CommandPost* pPost =
            static_cast<const UnitCaptureCPInfo*>(GetCurCmd())->GetCommandPost();
        if (!pPost)
        {
            return;
        }
        mAgent = new UnitTrooperAgent(this, pPost);
        break;
    }
    case STATE_WAIT_SECONDARY:
    {

        mAgent = new UnitWaitSecondaryAgent(this, GetCurCmd()->GetParam() / 1000.0f,
                                            (GetCurCmd()->GetParam() & 1) != 0);
        break;
    }
    case STATE_BOARD:
    {
        const UnitBoardInfo* pInfo = static_cast<const UnitBoardInfo*>(GetCurCmd());
        GameObject* pVehicle = pInfo->GetVehicle();
        mAgent = new UnitBoardAgent(this, pVehicle, pInfo->GetVOResponseHandle());
        break;
    }
    case STATE_UNBOARD:
        mAgent = new UnitUnBoardAgent(this);
        break;
    case STATE_SC_HOLD:
        mAgent = new UnitHoldAgent(this);
        break;
    case STATE_SC_FOLLOW:
    {
        const UnitFollowInfo* pInfo = static_cast<const UnitFollowInfo*>(GetCurCmd());
        mAgent = new UnitFollowAgent(this, PblHandle<const GameObject>(pInfo->GetWho()),
                                     pInfo->GetParam() != 0);
        break;
    }
    case STATE_RANDOM:
        mAgent = new UnitRandomAgent(this);
        break;
    case STATE_CTFOFFENSE:
    {
        const UnitGoalAgentInfo* pInfo = static_cast<const UnitGoalAgentInfo*>(GetCurCmd());
        mAgent = new UnitCTFOffenseAgent(this, pInfo->GetFlag(), pInfo->GetPos());
        break;
    }
    case STATE_DEATHMATCH:
        mAgent = new UnitDeathmatchAgent(this);
        break;
    case STATE_DEFEND:
    {
        AI::GoalTarget target;
        static_cast<const UnitGoalAgentInfo*>(GetCurCmd())->GetGoalTarget(target);
        mAgent = new UnitDefendAgent(this, target);
        break;
    }
    case STATE_DESTROY:
    {
        AI::GoalTarget target;
        static_cast<const UnitGoalAgentInfo*>(GetCurCmd())->GetGoalTarget(target);
        mAgent = new UnitDestroyAgent(this, target);
        break;
    }
    }
}

// FUNCTION: SWBF2 0x00663920
void UnitController::ExitState()
{
    if (mAgent)
    {
        mAgent->Kill();
    }
    DeleteAgent();
}

// FUNCTION: SWBF2 0x00663940
void UnitController::UpdateState(float dt)
{
    if (mAgent)
    {
        mCommandDone = mAgent->Update(dt, false);
    }
}

// FUNCTION: SWBF2 0x00663970
void UnitController::UpdateHighLevel()
{
    float fTime = GameLoop::GetMissionTime();
    float dt = fTime - mLastUpdateTime;
    mLastUpdateTime = fTime;

    mOwner->GetGameObject();
    const GameObject* pObj = mOwner->GetGameObject();
    if (!pObj->IsAlive())
    {
        mNextUpdateTime = fTime + 100000.0f;
        return;
    }

    mOwner->ResetControls();

    if (mJustEnteredVehicleGoToPickupAgent)
    {
        AI::EventInfo info(AI::EVT_Vehicle_WaitForPickUp, 4.0f, 0);
        if (SendEvent(&info))
        {
            mJustEnteredVehicleGoToPickupAgent = false;
        }
    }

    UpdateLodState();
    UpdateFreeSpace();
    AudibleManager::UpdateAudience(this);

    if (Character* pCharacter = mOwner->mCharacter)
    {
        pCharacter->mAdrenaline.Update(dt, mOwner->GetGameObject());
    }

    mThreatManager.Update(dt, this);
    CommandFsm::FSMUpdate(dt, false);
    UpdateInvisibility();
    UseBuffs();
    UseAutoTurrets();
    mOwner->SetCurrentCollision(0);

    if (mOwner->GetGameObject() && AIUtil::IsEndGame(0) &&
        !AIUtil::ShouldStayInVehicle(mOwner))
    {
        mLowLevel.SetExitVehicle(0);
    }

    VisionManager::RemoveRequests(this);
    VisionManager::UpdatePotentiallyVisible(this);
    mNextUpdateTime = GetUpdateRate() + fTime;
}

// FUNCTION: SWBF2 0x00663af0
void UnitController::UpdateLowLevel(float dt)
{
    mLowLevelUpdateSkipDt = dt + mLowLevelUpdateSkipDt;
    if (--mLowLevelUpdateSkips >= 0)
    {
        mLowLevel.UpdateSkip();
        return;
    }

    switch (mLodState)
    {
    case 0:
    case 1:
        mLowLevelUpdateSkips = 4;
        break;
    case 2:
        mLowLevelUpdateSkips = 2;
        break;
    default:
        mLowLevelUpdateSkips = 0;
        break;
    }

    mOwner->GetGameObject();
    UpdateTargetBubbleSize(mLowLevelUpdateSkipDt);
    UpdateTotalVisFactor();
    mLowLevel.Update(mLowLevelUpdateSkipDt);
    mLowLevelUpdateSkipDt = 0.0f;
}

// FUNCTION: SWBF2 0x00663b90
bool UnitController::IsInCombat() const
{
    return mCurState == STATE_COMBAT;
}

// FUNCTION: SWBF2 0x00663ba0
bool UnitController::IsAgentDone()
{
    return mAgent && mAgent->IsDone();
}

// FUNCTION: SWBF2 0x00663bc0
CmdInfo UnitController::CopyCurCmd() const
{
    CmdInfo cmd;
    if ((unsigned int)GetCurCmd()->GetCommand() <= CMD_SC_END)
    {
        cmd = *GetCurCmd();
    }
    return cmd;
}

// FUNCTION: SWBF2 0x00663c00
const UnitCmdInfo* UnitController::GetCurCmd() const
{
    return static_cast<const UnitCmdInfo*>(&mCurCmd);
}

// FUNCTION: SWBF2 0x00663c10
const SquadCommands::SquadCommandOrder& UnitController::GetLastSquadCommand()
{
    return mLastSquadCommand;
}

// FUNCTION: SWBF2 0x00663c20
const GameObject* UnitController::GetAttackHelperTarget() const
{
    if (!mAgent)
    {
        return 0;
    }
    const GameObject* obj = mAgent->mAttackHelper.GetTarget();
    return AIUtil::GetAlivePtr(obj, false);
}

// FUNCTION: SWBF2 0x00663c70
const GameObject* UnitController::GetAttackHelperVisibleTarget() const
{
    if (!mAgent)
    {
        return 0;
    }
    const GameObject* obj = mAgent->mAttackHelper.GetVisibleTarget();
    return AIUtil::GetAlivePtr(obj, false);
}

// FUNCTION: SWBF2 0x00663cc0
const PblVector3& UnitController::GetAttackHelperTargetLastSeenPosition() const
{
    if (!mAgent)
    {
        return gMatrixIdentity.GetTranslation();
    }
    return mAgent->mAttackHelper.GetLastSeenTargetPos();
}

// FUNCTION: SWBF2 0x00663ce0
void UnitController::ForceSetAttackHelperTarget(const GameObject* pTarget)
{
    if (mAgent)
    {
        mAgent->mAttackHelper.SetVisibleTarget(pTarget);
    }
}

// FUNCTION: SWBF2 0x00663dd0
void UnitController::UpdateLodState()
{
    if (GameLoop::GetMissionTime() < mLodStateUpdateTime + 0.5f)
    {
        return;
    }
    mLodStateUpdateTime = GameLoop::GetMissionTime();

    int iLod = 0;
    const PblVector3& vMyPos = mOwner->GetMatrix().GetTranslation();
    float fClosestDistSq = FLT_MAX;
    mLodHumanPlayer = 0;

    for (int i = 0; i < Character::sCharacterLimit; i++)
    {
        Character* pCharacter = Character::GetPtr(i);
        if (pCharacter->mPlayerId < 0 || !pCharacter->GetControllable())
        {
            continue;
        }
        const Character& rCharacter = *pCharacter;

        int iCharLod = 3;
        const PblVector3& vPos = pCharacter->GetControllable()->GetMatrix().GetTranslation();
        float dx = vPos.x - vMyPos.x;
        float dy = vPos.y - vMyPos.y;
        float dz = vPos.z - vMyPos.z;
        float fDistSq = dx * dx + dy * dy + dz * dz;
        int iHigh = GetLODDistHigh();
        if ((float)(iHigh * iHigh) > fDistSq)
        {
            iCharLod = 4;
        }
        else
        {
            int iLow = GetLODDistLow();
            if (fDistSq > (float)(iLow * iLow))
            {
                iCharLod = 2;
            }
        }

        if (!AIUtil::CanPotentiallySee(mOwner, rCharacter.GetControllable()->GetGameObject(), 0))
        {
            iCharLod--;
        }

        const GameObject* pSceneObj = rCharacter.GetControllable()->GetGameObject();
        if (pSceneObj->PortalActive() && !pSceneObj->IsActive(pvs::GetCurrentFrame_()))
        {
            iCharLod--;
        }

        if (iLod > iCharLod)
        {
            iCharLod = iLod;
        }
        iLod = iCharLod;
        if (fClosestDistSq > fDistSq)
        {
            fClosestDistSq = fDistSq;
            mLodHumanPlayer = pCharacter;
        }
        if (iLod == 4)
        {
            break;
        }
    }

    if (!AIDebug::SkipCameraForLODs() && iLod != 4)
    {
        float fRadius = AIUtil::GetCollisionRadius(mOwner->GetGameObject()) * 3.0f;
        if (CameraManager::sInstance->IsInsideAnyCamera(vMyPos, fRadius, (float)GetLODDistLow()))
        {
            iLod = 3;
            if (CameraManager::sInstance->IsInsideAnyCamera(vMyPos, fRadius, (float)GetLODDistHigh()))
            {
                iLod = 4;
            }
        }
    }

    mLodState = iLod;
}

// FUNCTION: SWBF2 0x006640b0
int UnitController::GetRandomSeed() const
{
    return mSeed;
}

// FUNCTION: SWBF2 0x006640c0
void UnitController::SetRandomSeed(int iSeed)
{
    mSeed = iSeed;
}

// FUNCTION: SWBF2 0x006640d0
int UnitController::SendEvent(AI::EventInfo* pInfo)
{
    int iResult = 0;
    if (mAgent)
    {
        iResult = mAgent->EventHandler(pInfo);
        if (pInfo->mNextCmd.GetCmdType() != CMD_NONE)
        {
            if (mAgent->ShouldQueueCurCmd(pInfo->mNextCmd))
            {
                CmdInfo curCmd = mAgent->mCtrl->CopyCurCmd();
                mAgent->mCtrl->PrependQueueCommands(curCmd, pInfo->mNextCmd);
            }
            else
            {
                mAgent->mCtrl->PrependQueueCommand(pInfo->mNextCmd);
            }

            if (mLodState >= 3)
            {
                ControllerManager::RequestControllerUpdateASAP(this);
            }
        }
    }
    return iResult;
}

// FUNCTION: SWBF2 0x00664170
int UnitController::SendEvent(AI::EventInfo* pInfo) const
{
    if (mAgent)
    {
        return mAgent->EventHandler(pInfo);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00664190
void UnitController::SetLastShotDist(float fDistSq)
{
    mLastShotDistSq = fDistSq;
    mLastShotDistPos = mOwner ? mOwner->GetMatrix().GetTranslation() : gMatrixIdentity.GetTranslation();
    mLastShotDistTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x006641e0
bool UnitController::IsTargetLOSPossiblyBlocked()
{
    if (GameLoop::GetMissionTime() - 2.0f > mLastShotDistTime)
    {
        return false;
    }

    const GameObject* pTarget = GetAttackHelperTarget();
    if (!pTarget || !mOwner)
    {
        return false;
    }

    const PblVector3& vMyPos = mOwner->GetMatrix().GetTranslation();
    if (SquaredLength(vMyPos - mLastShotDistPos) > 1.0f)
    {
        return false;
    }

    float fDistSq = SquaredLength(GetPosition(pTarget) - vMyPos);
    if (fDistSq < 4.0f)
    {
        return false;
    }
    if (fDistSq > mLastShotDistSq * 4.0f)
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006642d0
void UnitController::UpdateInvisibility()
{
    if (!mOwner || !mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }
    if (mThreatManager.GetVisibleObject())
    {
        return;
    }

    for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
    {
        if (mOwner->GetWeaponPtr(i)->IsRtti(WeaponInvisibility::rttiHashWeaponInvisibility))
        {
            GetLowLevel().SetNextWeapon(i);
            GetLowLevel().SetFireRequest(true, false);
        }
    }
}

// FUNCTION: SWBF2 0x00664360
void UnitController::UseBuffs()
{
    if (!mOwner || !mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }
    if (!mThreatManager.GetVisibleObject())
    {
        return;
    }

    for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = mOwner->GetWeaponPtr(i);
        const WeaponClass* pClass = pWeapon->mClass;
        if (pWeapon->CheckOverheat())
        {
            continue;
        }
        if (pClass->mOrdnanceClass &&
            pClass->mOrdnanceClass->IsRtti(OrdnanceEmitterClass::rttiHashOrdnanceEmitterClass) &&
            (pClass->mTargetSides & 4) && (pClass->mAITargetTypes & 1))
        {
            GameObject* apUnits[3];
            Controllable* pOwner = mOwner;
            const Team* pTeam = Team::GetPtr(pOwner->GetGameObject()->GetTeam());
            if (ControllerManager::GetUnitsInRange(pOwner->GetMatrix().GetTranslation(), pClass->mMaxRange, apUnits, 3,
                                                   pTeam, AI::AFFIL_ALLYNEUTRAL) < 3)
            {
                return;
            }
            mLowLevel.SetNextWeapon(i);
            mLowLevel.SetFireRequest(true, false);
        }
    }
}

// FUNCTION: SWBF2 0x00664490
void UnitController::UseAutoTurrets()
{
    if (!mOwner || !mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }

    const GameObject* pTarget = mThreatManager.GetVisibleObject();
    const Controllable* pTargetCtl = pTarget ? pTarget->GetControllable() : 0;
    Controller* pTargetCtrl = pTargetCtl ? pTargetCtl->mCtrl : 0;
    if (!pTargetCtrl || !pTargetCtrl->IsInCrowd())
    {
        return;
    }
    if (!pTarget->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }

    for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = mOwner->GetWeaponPtr(i);
        const WeaponClass* pClass = pWeapon->mClass;
        if (!pClass || pWeapon->CheckOverheat())
        {
            continue;
        }
        const AmmoCounter* pAmmo = pWeapon->m_pAmmoCounter;
        float fAmmoPerRound = pAmmo->m_pClass->mAmmoPerRound;
        if (fAmmoPerRound > pAmmo->mCurClip && fAmmoPerRound > pAmmo->mNumClips)
        {
            continue;
        }
        if (pClass->IsRtti(WeaponDispenserClass::rttiHashWeaponDispenserClass))
        {
            EntityClass* pDispense = static_cast<const WeaponDispenserClass*>(pClass)->mDispenseClass;
            if (pDispense && pDispense->IsRtti(EntityBuildingArmedClass::rttiHashEntityBuildingArmedClass) &&
                (pClass->mAITargetTypes & 1))
            {
                AILowLevel* pLowLevel = &mLowLevel;
                pLowLevel->SetNextWeapon(i);
                pLowLevel->SetFireRequest(true, false);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00663d60
int UnitController::GetFriendlyFireCount() const
{
    return mFriendlyCount;
}

// FUNCTION: SWBF2 0x00663d10
void UnitController::AddFriendlyFire(const GameObject* pObject)
{
    if (mFriendlyCount < 8)
    {
        mFriendly[mFriendlyCount++] = pObject;
    }
}

// FUNCTION: SWBF2 0x00663d70
const GameObject* UnitController::GetFriendlyFire(int iIndex)
{
    if (iIndex < mFriendlyCount)
    {
        return ((PblHandle<const GameObject>*)mFriendly)[iIndex];
    }
    return 0;
}

// FUNCTION: SWBF2 0x00664090
bool UnitController::IsLookAheadOnly() const
{
    return mLookAheadOnly;
}

// FUNCTION: SWBF2 0x006640a0
const PblVector3& UnitController::GetOriginalAheadDir() const
{
    return mOriginalAheadDir;
}
