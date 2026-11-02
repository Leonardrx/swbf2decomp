#include <d3dx9math.h>

#include "EntityHover.h"
#include "EntityWalker.h"
#include "UnitAttackPatterns.h"
#include "AIDifficulty.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

class GameObject;


extern PblVector3 gVectorZero;

namespace AIUtil
{

    int GetRandomIntPerSecond(int iMin, int iMax);

    void Slide(GameObject* pMe, const GameObject* pTarget, bool bUseAttackHelper,
               bool* pbIsPathing, float* pfPathTime, float fMinDist, float fOffset);
    bool DirectLineToTarget(const GameObject* pObj, const PblVector3& vPos);
}

namespace CombatUtil
{
    float GetWeaponOptimalRange(const Controllable* pMe);
    float GetWeaponMaxRange(const Controllable* pMe);
    float GetWeaponMinRange(const Controllable* pMe);
}

namespace VisionManager
{

    float MaxVisibleDist(const GameObject* pSeer, const GameObject* pTarget);
}

namespace
{


    inline bool IsRtti(const GameObject* pObj, unsigned int uRtti)
    {
        return reinterpret_cast<const Entity*>(pObj)->IsRtti(uRtti);
    }

    inline CollisionObject* GetCollisionObject(GameObject* pObj)
    {
        return pObj->GetCollisionObject();
    }

    PblVector3 GetTargetPoint(const GameObject* pObj, const PblVector3& v0, const PblVector3& v1, int i)
    {
        PblVector3 vResult;
        vResult = pObj->GetTargetPoint(v0, v1, i);
        return vResult;
    }

    inline const CollisionObject* GetCollisionObject(const GameObject* pObj)
    {
        return pObj->GetCollisionObject();
    }

    inline const CollisionObject* GetGeometryCollision(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj);
    }

    inline float GetMaxSpeed(GameObject* pObj)
    {
        return pObj->GetMaxSpeed();
    }

    float GetMaxStrafe(GameObject* pObj)
    {
        return pObj->GetMaxStrafe();
    }


    inline const EntitySoldierClass* GetSoldierClass(const Entity* pSoldier)
    {
        return static_cast<const EntitySoldier*>(pSoldier)->mClass;
    }

    const void* GetAcklayData(const Entity* pSoldier)
    {
        return static_cast<const EntitySoldier*>(pSoldier)->mAcklayData;
    }

    inline float GetJetFuel(const Entity* pSoldier)
    {
        return static_cast<const EntitySoldier*>(pSoldier)->mJetFuel;
    }

}

// SYNTHETIC: SWBF2 0x0065f1c0
// AttackPatterns::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0065f200
AttackPatterns::AttackPatterns(UnitAgent* pOwner)
    : HelperFsm(STATE_START, STATE_START)
    , mOwner(pOwner)
    , mCoverHelper(pOwner)
    , mVehicleCoverHelper(pOwner)
    , mJetJumpHintHelper(this)
{
}

// FUNCTION: SWBF2 0x0065f240
void AttackPatterns::Kill()
{
    mCoverHelper.Kill();
    mVehicleCoverHelper.Kill();
    mJetJumpHintHelper.Kill();
    Fsm::Kill();
}

// FUNCTION: SWBF2 0x0065f270
void AttackPatterns::EnterState()
{
    mSwitchToCoverTimer = -1.0f;
    mSwitchToVehicleCoverTimer = -1.0f;
    mSwitchToJetJumpTimer = AIUtil::GetRandomInt(3, 8) + GameLoop::GetMissionTime();
    mSwitchToJetJumpHintTimer = AIUtil::GetRandomInt(3, 8) + GameLoop::GetMissionTime();

    switch (mCurState)
    {
    case STATE_STATIONARY:      InitStationary(); break;
    case STATE_SIMPLE:          InitSimple(); break;
    case STATE_SERPENTINE:      InitSerpentine(); break;
    case STATE_FLANK:           InitFlank(); break;
    case STATE_CRITICALHIT:     InitCriticalHit(); break;
    case STATE_MELEE:           InitMelee(); break;
    case STATE_GLANCE:          InitGlance(); break;
    case STATE_RETREAT:         InitRetreat(); break;
    case STATE_COVER:           InitCover(); break;
    case STATE_VEHICLECOVER:    InitVehicleCover(); break;
    case STATE_JETJUMP:         InitJetJump(); break;
    case STATE_JETJUMPHINT:     InitJetJumpHint(); break;
    case STATE_JETJUMPCHARGE:   InitJetJumpCharge(); break;
    case STATE_LOCATE:          InitLocate(); break;
    case STATE_FLY_STRAFE:      InitFlyStrafe(); break;
    case STATE_FLY_DIRECT:      InitFlyDirect(); break;
    }
}

// FUNCTION: SWBF2 0x0065f3b0
void AttackPatterns::ExitState()
{
    mOwner->GetLowLevel().SetHeight(HEIGHT_STAND);

    switch (mCurState)
    {
    case STATE_LOCATE:          CleanLocate(); break;
    case STATE_FLY_STRAFE:      CleanFlyStrafe(); break;
    case STATE_FLY_DIRECT:      CleanFlyDirect(); break;
    case STATE_MELEE:           CleanMelee(); break;
    case STATE_GLANCE:          CleanGlance(); break;
    case STATE_COVER:           CleanCover(); break;
    case STATE_VEHICLECOVER:    CleanVehicleCover(); break;
    case STATE_JETJUMP:         CleanJetJump(); break;
    case STATE_JETJUMPHINT:     CleanJetJumpHint(); break;
    case STATE_JETJUMPCHARGE:   CleanJetJumpCharge(); break;
    }
}

// FUNCTION: SWBF2 0x0065f440
void AttackPatterns::UpdateState(float dt)
{
    mOverrideFire = false;

    switch (mCurState)
    {
    case STATE_STATIONARY:      DoStationary(); break;
    case STATE_SIMPLE:          DoSimple(); break;
    case STATE_SERPENTINE:      DoSerpentine(); break;
    case STATE_FLANK:           DoFlank(); break;
    case STATE_CRITICALHIT:     DoCriticalHit(); break;
    case STATE_MELEE:           DoMelee(); break;
    case STATE_GLANCE:          DoGlance(); break;
    case STATE_RETREAT:         DoRetreat(); break;
    case STATE_COVER:           DoCover(dt); break;
    case STATE_VEHICLECOVER:    DoVehicleCover(dt); break;
    case STATE_JETJUMP:         DoJetJump(dt); break;
    case STATE_JETJUMPHINT:     DoJetJumpHint(dt); break;
    case STATE_JETJUMPCHARGE:   DoJetJumpCharge(dt); break;
    case STATE_LOCATE:          DoLocate(); break;
    case STATE_FLY_STRAFE:      DoFlyStrafe(); break;
    case STATE_FLY_DIRECT:      DoFlyDirect(); break;
    }
}

// FUNCTION: SWBF2 0x0065f540
void AttackPatterns::HELPERSetNextState(int state, float fTime)
{
    mCoverPercent = fTime;
    mVehicleCoverPercent = fTime;
    mReturnState = 100;
    HelperFsm::SetNextState(state);
    FSMChangeState();
}

// FUNCTION: SWBF2 0x0065f580
void AttackPatterns::HELPERClean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x0065f590
void AttackPatterns::ToggleToReturnState()
{
    int iReturnState = mReturnState;
    mReturnState = 100;
    HelperFsm::SetNextState(iReturnState);
}

// FUNCTION: SWBF2 0x0065f5b0
float AttackPatterns::GetNear(const Controllable* pMe)
{
    float fOptimal = CombatUtil::GetWeaponOptimalRange(pMe);
    float fNear = CombatUtil::GetWeaponMinRange(pMe) + 3.0f;
    return fOptimal > fNear ? fNear : fOptimal;
}

// FUNCTION: SWBF2 0x0065f5f0
float AttackPatterns::GetNear() const
{
    return GetNear(GetMe());
}

// FUNCTION: SWBF2 0x0065f600
float AttackPatterns::GetFar() const
{
    float fOptimal = CombatUtil::GetWeaponOptimalRange(GetMe());
    float fMax = CombatUtil::GetWeaponMaxRange(GetMe());
    float fVisible = VisionManager::MaxVisibleDist(GetMe()->GetGameObject(), GetAttackHelper()->GetTarget());

    float fFar = fMax - 10.0f > fOptimal ? fMax - 10.0f : fOptimal;
    return fFar > fVisible - 10.0f ? fVisible - 10.0f : fFar;
}

// FUNCTION: SWBF2 0x0065f690
float AttackPatterns::GetOptimalish() const
{
    float fOptimal = CombatUtil::GetWeaponOptimalRange(GetMe());
    float fMin = CombatUtil::GetWeaponMinRange(GetMe());
    float fMax = CombatUtil::GetWeaponMaxRange(GetMe());
    return AIUtil::GetRandomFloat(fOptimal - (fOptimal - fMin) * 0.2f, fOptimal + (fMax - fOptimal) * 0.2f);
}

// FUNCTION: SWBF2 0x0065f710
float AttackPatterns::CalculateAngle(const Controllable* pCtrl) const
{
    PblVector3 vTargetPoint = GetMe()->GetGameObject()->GetTargetPoint(gVectorZero, gVectorUnitZ, -1);

    PblVector3 vEyePoint;
    vEyePoint = pCtrl->mEyePoint;
    PblVector3 vEyeDir;
    vEyeDir = pCtrl->mEyeDir;
    vEyeDir.y = 0.0f;

    PblVector3 vToMe;
    vToMe.x = vTargetPoint.x - vEyePoint.x;
    vToMe.y = 0.0f;
    vToMe.z = vTargetPoint.z - vEyePoint.z;

    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vToMe);
    return vEyeDir.x * vDir.x + vDir.y * vEyeDir.y + vDir.z * vEyeDir.z;
}

// FUNCTION: SWBF2 0x0065f7d0
bool AttackPatterns::IsInEnemySights(float fAngle, const Controllable** ppEnemy)
{
    *ppEnemy = NULL;

    const GameObject* pTarget = GetAttackHelper()->GetVisibleTarget();
    if (pTarget)
    {
        float fBest = -1.1f;
        for (PblList<Controllable>::Itor it(pTarget->GetControllableList()); it; ++it)
        {
            if (it->mCtrl)
            {
                float f = CalculateAngle(it);
                if (f > fBest)
                {
                    fBest = f;
                    *ppEnemy = it;
                }
            }
        }

        if (fBest > PblMath::Cos(fAngle))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0065f880
bool AttackPatterns::GetInRange()
{
    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();

    PblVector3 vDiff;
    vDiff.x = vPos.x - vTarget.x;
    vDiff.y = 0.0f;
    vDiff.z = vPos.z - vTarget.z;
    PblVector3 vDir;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vDiff));

    PblVector3 vDest;
    float fDistSq = vDiff.z * vDiff.z + vDiff.x * vDiff.x;
    if (GetNear() * GetNear() > fDistSq)
    {
        PblVector3 vStep = vDir * 15.0f;
        vDest.x = vPos.x + vStep.x;
        vDest.y = vPos.y + vStep.y;
        vDest.z = vPos.z + vStep.z;
    }
    else
    {
        fDistSq = vDiff.z * vDiff.z + vDiff.x * vDiff.x;
        if (fDistSq > GetFar() * GetFar())
        {
            PblVector3 vStep = vDir * 15.0f;
            vDest.x = vPos.x - vStep.x;
            vDest.y = vPos.y - vStep.y;
            vDest.z = vPos.z - vStep.z;
        }
        else
        {
            mOwner->GetLowLevel().SetNavigator_Stationary();
            return true;
        }
    }

    mOwner->GetLowLevel().SetNavigator_Slide(vDest, NULL);
    return false;
}

// FUNCTION: SWBF2 0x0065fa40
bool AttackPatterns::ToggleToCover(int iReturnState, float fPercentMult)
{
    if (GameLoop::GetMissionTime() > mSwitchToCoverTimer)
    {
        mSwitchToCoverTimer = AIUtil::GetRandomInt(2, 5) + GameLoop::GetMissionTime();
        float fRandom = AIUtil::GetRandomFloat(0.0f, 1.0f);
        if (AIDifficulty::GetCoverPercentage() * (mCoverPercent * fPercentMult) > fRandom &&
            mCoverHelper.CheckForHintAndInit(COVERTYPE_Combat, NULL))
        {
            mReturnState = iReturnState;
            HelperFsm::SetNextState(STATE_COVER);
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0065fae0
bool AttackPatterns::ToggleToVehicleCover(int iReturnState)
{
    if (GetMe()->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover) &&
        !(GetMe()->GetGameObject()->GetMaxStrafe() <= 0.0001f) &&
        GetAttackHelper()->GetVisibleTarget() &&
        !GetAttackHelper()->GetVisibleTarget()->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
        GameLoop::GetMissionTime() > mSwitchToVehicleCoverTimer)
    {
        mSwitchToVehicleCoverTimer = AIUtil::GetRandomInt(2, 5) + GameLoop::GetMissionTime();
        if (mVehicleCoverPercent != 0.0f && mVehicleCoverHelper.CheckForHintAndInit(NULL))
        {
            mReturnState = iReturnState;
            HelperFsm::SetNextState(STATE_VEHICLECOVER);
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0065fbe0
bool AttackPatterns::ToggleToJetJump(int iReturnState)
{
    if (GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        const Entity* pSoldier = GetMe()->GetEntity();
        const EntitySoldierClass* pClass = GetSoldierClass(pSoldier);
        if (pClass->mJetFuelRechargeRate > 0.0f && GetJetFuel(pSoldier) > pClass->mJetFuelMinBorder &&
            GameLoop::GetMissionTime() > mSwitchToJetJumpTimer)
        {
            mSwitchToJetJumpTimer = AIUtil::GetRandomInt(6, 12) + GameLoop::GetMissionTime();
            mReturnState = iReturnState;
            HelperFsm::SetNextState(STATE_JETJUMP);
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0065fca0
bool AttackPatterns::ToggleToJetJumpHint(int iReturnState)
{
    if (GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        const Entity* pSoldier = GetMe()->GetEntity();
        const EntitySoldierClass* pClass = GetSoldierClass(pSoldier);
        if (pClass->mJetFuelRechargeRate > 0.0f && GetJetFuel(pSoldier) > pClass->mJetFuelMinBorder)
        {
            const bool bHover = pClass->mJetTypeHover;
            if (!bHover && GameLoop::GetMissionTime() > mSwitchToJetJumpHintTimer)
            {
                mSwitchToJetJumpHintTimer = AIUtil::GetRandomInt(3, 8) + GameLoop::GetMissionTime();
                mJetJumpHint = (JetJumpHint*)mJetJumpHintHelper.HELPERFindHint();
                if (mJetJumpHint)
                {
                    mReturnState = iReturnState;
                    HelperFsm::SetNextState(STATE_JETJUMPHINT);
                    return true;
                }
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0065fd80
bool AttackPatterns::ToggleToLocate(int iReturnState)
{
    if (GameLoop::GetMissionTime() - GetAttackHelper()->GetLastTimeSeen() > 2.0f)
    {
        mReturnState = iReturnState;
        HelperFsm::SetNextState(STATE_LOCATE);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0065fdd0
void AttackPatterns::InitStationary()
{
    mOwner->GetLowLevel().SetNavigator_Stationary();
    mHeight = (GetCtrl()->GetRandomSeed() % 3 == 1) ? HEIGHT_CROUCH : HEIGHT_STAND;
}

// FUNCTION: SWBF2 0x0065fe10
void AttackPatterns::DoStationary()
{
    if (ToggleToCover(STATE_STATIONARY, 1.0f) ||
        ToggleToVehicleCover(STATE_STATIONARY) ||
        ToggleToJetJumpHint(STATE_STATIONARY) ||
        ToggleToJetJump(STATE_STATIONARY) ||
        ToggleToJetJumpCharge(STATE_STATIONARY))
    {
        return;
    }

    if ((GetMe()->GetGameObject()->IsRtti(EntityHover::rttiHashEntityHover) ||
         GetMe()->GetGameObject()->IsRtti(EntityWalker::rttiHashEntityWalker)) &&
        GetMaxSpeed(GetMe()->GetGameObject()) < 13.0f &&
        GetMe()->GetGameObject()->GetMaxStrafe() > 0.0001f &&
        AIUtil::GetRandomIntPerSecond(1, 13) == 1)
    {
        PblVector3 vDest = GetMe()->GetMatrix().GetTranslation();
        const PblMatrix& m = GetMe()->GetMatrix();
        float fSide = (float)(AIUtil::GetRandomInt(0, 1) * 2 - 1);
        PblVector3 vSide(m.m[0][0] * fSide, m.m[0][1] * fSide, m.m[0][2] * fSide);

        float fRadius = GetCollisionObject(GetMe()->GetGameObject())->GetCollisionRadius();
        float fDist = AIUtil::GetRandomFloat(fRadius, fRadius * 2.0f);
        PblVector3 vOffset = vSide * fDist;
        vDest += vOffset;
        mOwner->GetLowLevel().SetNavigator_Goto(vDest, NULL, false, false);
    }

    mOwner->GetLowLevel().mAllowHoverStrafe = true;
    mOwner->FaceTarget(mHeight);
}

// FUNCTION: SWBF2 0x00660080
void AttackPatterns::SetSimpleTarget()
{
    PblVector3 vRandom = PblRandom::GetSim().GetVector3();
    vRandom.y = 0.0f;
    vRandom.x += 0.0001f;
    vRandom = Normalize(vRandom);

    float fDist = GetOptimalish();
    mOwner->GetLowLevel().SetNavigator_Slide(GetAttackHelper()->GetLastSeenTargetPos() + vRandom * fDist, NULL);
}

// FUNCTION: SWBF2 0x00660060
void AttackPatterns::InitSimple()
{
    SetSimpleTarget();
    mSimpleStopped = false;
}

// FUNCTION: SWBF2 0x00660140
void AttackPatterns::DoSimple()
{
    if (ToggleToCover(STATE_SIMPLE, 1.0f) ||
        ToggleToJetJumpHint(STATE_SIMPLE) ||
        ToggleToJetJump(STATE_SIMPLE) ||
        ToggleToJetJumpCharge(STATE_SIMPLE) ||
        ToggleToLocate(STATE_SIMPLE))
    {
        return;
    }

    if (mSimpleStopped)
    {
        if (GameLoop::GetMissionTime() > mSimpleTimer)
        {
            mSimpleStopped = false;
            SetSimpleTarget();
        }
        else
        {
            mOwner->GetLowLevel().SetNavigator_Stationary();
        }
    }
    else
    {
        bool bStop = GetCtrl()->mLowLevel.IsAtDest();
        if (!bStop)
        {
            bStop = AIUtil::GetRandomIntPerSecond(1, 6) == 2;
        }
        if (bStop)
        {
            mSimpleStopped = true;
            mSimpleTimer = GameLoop::GetMissionTime() + 1.9f;
        }
    }

    mOwner->FaceTarget(HEIGHT_STAND);
    mOwner->GetLowLevel().mAllowHoverStrafe = true;
}

// FUNCTION: SWBF2 0x00660240
void AttackPatterns::CycleSerpDir()
{
    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    const PblVector3& vTarget = GetAttackHelper()->GetLastSeenTargetPos();
    PblVector3 vAhead = AIUtil::GetSafeNormalXZ((vTarget - vPos));

    if (mSerpStrafe == LEFT)
    {
        mSerpCur = RIGHT;
    }
    else
    {
        mSerpCur = LEFT;
    }
    mSerpStrafe = mSerpCur;

    PblVector3 vSide(vAhead.y * 0.0f - vAhead.z, vAhead.z * 0.0f - vAhead.x * 0.0f, vAhead.x - vAhead.y * 0.0f);
    float fSide = (mSerpStrafe == RIGHT) ? 30.0f : -30.0f;
    float fAhead = (mSerpAhead == FORWARD) ? 30.0f : -30.0f;

    PblVector3 vDest = vAhead * fAhead + vSide * fSide + vPos;
    mOwner->GetLowLevel().SetNavigator_Slide(vDest, NULL);
}

// FUNCTION: SWBF2 0x006603f0
void AttackPatterns::DoSerpentine()
{
    if (ToggleToCover(STATE_SERPENTINE, 1.0f) ||
        ToggleToJetJumpHint(STATE_SERPENTINE) ||
        ToggleToJetJump(STATE_SERPENTINE) ||
        ToggleToJetJumpCharge(STATE_SERPENTINE) ||
        ToggleToLocate(STATE_SERPENTINE))
    {
        return;
    }

    bool bWasSeen = mSerpEnemySee;
    const Controllable* pEnemy = NULL;
    mSerpEnemySee = IsInEnemySights(PblMath::fPi / 10.0f, &pEnemy);
    if (mSerpEnemySee)
    {
        bool bJustSeen = false;
        if (!bWasSeen)
        {
            mSerpTimer = 0.0f;
            bJustSeen = true;
        }

        const Controllable* pMe = GetMe();
        const PblVector3& vTarget = GetAttackHelper()->GetLastSeenTargetPos();
        const PblVector3& vPos = pMe->GetMatrix().GetTranslation();
        float fDistSq = SquaredLengthXZ(vPos - vTarget);

        bool bChangeAhead = false;
        if (GameLoop::GetMissionTime() > mSerpTimer)
        {
            if (GetNear() * GetNear() > fDistSq)
            {
                mSerpAhead = BACKWARD;
                bChangeAhead = true;
            }
            else if (fDistSq > GetFar() * GetFar())
            {
                mSerpAhead = FORWARD;
                bChangeAhead = true;
            }
        }

        if (bChangeAhead)
        {
            mSerpTimer = GameLoop::GetMissionTime() + 3.0f;
            CycleSerpDir();
        }
        else
        {
            bool bCycle = bJustSeen;
            if (!bCycle)
            {
                bCycle = AIUtil::GetRandomIntPerSecond(1, 2) == 1;
            }
            if (bCycle)
            {
                CycleSerpDir();
            }
        }
    }
    else
    {
        if (bWasSeen)
        {
            mSerpRangeTimer = 0.0f;
        }
        if (GameLoop::GetMissionTime() > mSerpRangeTimer)
        {
            bool bInRange = GetInRange();
            if (!bInRange)
            {
                mSerpRangeTimer = GameLoop::GetMissionTime() + 3.0f;
            }
            mSerpInRange = bInRange;
        }
    }

    mOwner->FaceTarget(HEIGHT_STAND);
}

// FUNCTION: SWBF2 0x00660630
void AttackPatterns::DoFlank()
{
    if (ToggleToCover(STATE_FLANK, 1.0f) ||
        ToggleToJetJumpHint(STATE_FLANK) ||
        ToggleToJetJump(STATE_FLANK) ||
        ToggleToJetJumpCharge(STATE_FLANK) ||
        ToggleToLocate(STATE_FLANK))
    {
        return;
    }

    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();
    PblVector3 vDiff(vPos.x - vTarget.x, 0.0f, vPos.z - vTarget.z);

    if (GameLoop::GetMissionTime() > mFlankTimer)
    {
        mFlankInView = false;
        PblVector3 vDir;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vDiff));

        bool bResetTimer;
        const Controllable* pEnemy = NULL;
        if (IsInEnemySights(PblMath::fPi / 3.0f, &pEnemy))
        {
            mFlankInView = true;
            PblVector3 vEyeDir = pEnemy->mEyeDir;

            PblVector3 vSideDir = CrossProduct(vDir, PblVector3(0.0f, 1.0f, 0.0f));
            float fSide = 1.0f;
            if (CrossProduct(vEyeDir, vDir).y > 0.0f)
            {
                fSide = -1.0f;
            }

            float fDist = CombatUtil::GetWeaponOptimalRange(GetMe()) * fSide;
            PblVector3 vStep = vSideDir * fDist;
            PblVector3 vDest;
            vDest.x = vTarget.x + vStep.x;
            vDest.y = vTarget.y + vStep.y;
            vDest.z = vTarget.z + vStep.z;
            mOwner->GetLowLevel().SetNavigator_Slide(vDest, NULL);
            bResetTimer = true;
        }
        else
        {
            bResetTimer = !GetInRange();
        }

        if (bResetTimer)
        {
            mFlankTimer = GameLoop::GetMissionTime() + 4.0f;
        }
    }

    mOwner->FaceTarget(HEIGHT_STAND);
}

// FUNCTION: SWBF2 0x00660890
void AttackPatterns::DoCriticalHit()
{
    if (ToggleToCover(STATE_CRITICALHIT, 0.25f) ||
        ToggleToJetJumpHint(STATE_CRITICALHIT) ||
        ToggleToJetJump(STATE_CRITICALHIT) ||
        ToggleToJetJumpCharge(STATE_CRITICALHIT) ||
        ToggleToLocate(STATE_CRITICALHIT))
    {
        return;
    }

    const GameObject* pTarget = GetAttackHelper()->GetVisibleTarget();
    if (pTarget && GameLoop::GetMissionTime() > mFlankTimer)
    {
        mFlankTimer = GameLoop::GetMissionTime() + 4.0f;

        float fDist = Min(30.0f, CombatUtil::GetWeaponOptimalRange(GetMe()));

        const CollisionObject* pTargetCol = GetGeometryCollision(pTarget);
        const PblMatrix& m = pTargetCol->GetMatrix();
        PblVector3 vBack(m.m[2][0] * fDist, m.m[2][1] * fDist, m.m[2][2] * fDist);
        const PblVector3& vTargetPos = pTargetCol->GetPosition();
        PblVector3 vDest(vTargetPos.x - vBack.x, vTargetPos.y - vBack.y, vTargetPos.z - vBack.z);

        const Obstacle* pIgnore[2];
        pIgnore[0] = AIUtil::GetGameObjectBarrier(pTarget);
        pIgnore[1] = AIUtil::GetGameObjectBarrier(GetMe()->GetGameObject());
        float fRadius = AIUtil::GetCollisionRadius(GetMe()->GetGameObject());
        int iAIType = AIUtil::GetAIType(GetMe()->GetGameObject());
        PblVector3 vNormal;
        PathFinder::PushValidPoint(pTargetCol->GetPosition(), vDest, vNormal, fRadius,
                                   iAIType, pIgnore, 2);
        mOwner->GetLowLevel().SetNavigator_Slide(vDest, NULL);
    }

    mOwner->FaceTarget(HEIGHT_STAND);
}

// FUNCTION: SWBF2 0x00660a70
void AttackPatterns::InitMelee()
{
    mIsPathing = false;
    mMeleePathTime = -100.0f;
    mJumpTimer = AIUtil::GetRandomFloat(0.0f, 3.0f) + GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x00660ac0
void AttackPatterns::DoMelee()
{
    if (ToggleToLocate(STATE_MELEE) ||
        ToggleToJetJump(STATE_MELEE) ||
        ToggleToJetJumpHint(STATE_MELEE) ||
        ToggleToJetJumpCharge(STATE_MELEE))
    {
        return;
    }

    mOwner->FaceTarget(HEIGHT_STAND);

    if (GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
        static_cast<const EntitySoldier*>(GetMe()->GetEntity())->mAcklayData)
    {
        const PblVector3 vDiff = GetMe()->GetMatrix().GetTranslation() -
                                 GetAttackHelper()->GetTarget()->GetMatrix().GetTranslation();
        float fRadius = GetMe()->GetGameObject()->GetCollisionRadius();
        if (fRadius * 2.25f * fRadius >= vDiff.x * vDiff.x + vDiff.y * vDiff.y + vDiff.z * vDiff.z)
        {
            return;
        }
    }
    else if (GameLoop::GetMissionTime() > mJumpTimer)
    {
        const PblVector3& vPos = GetMe()->GetMatrix().GetTranslation();
        const PblVector3& vTarget = GetAttackHelper()->GetLastSeenTargetPos();
        PblVector3 vJump;
        vJump = vTarget - vPos;
        vJump.y = 0.0f;
        const float dz = vJump.z;
        const float dx = vJump.x;
        vJump = Normalize(vJump);
        vJump.x *= 0.70710677f;
        vJump.y = 0.70710677f;
        vJump.z *= 0.70710677f;
        if (!(dx * dx + dz * dz > 25.0f))
        {
            int iPick = AIUtil::GetRandomInt(0, 2);
            switch (iPick)
            {
            case 0:
                vJump = PblVector3(vJump.z, vJump.y, vJump.x);
                break;
            case 1:
            {
                float fSin, fCos;
                PblMath::SinCos(&fSin, &fCos, PblMath::fPi / 2.0f);
                vJump = PblVector3(fCos * vJump.x - vJump.z * fSin, vJump.y, vJump.z * fCos + fSin * vJump.x);
                break;
            }
            case 2:
            {
                float fSin, fCos;
                PblMath::SinCos(&fSin, &fCos, -PblMath::fPi / 2.0f);
                vJump = PblVector3(fCos * vJump.x - vJump.z * fSin, vJump.y, vJump.z * fCos + fSin * vJump.x);
                break;
            }
            }
        }

        PblVector3 vLand(vJump.x, 0.0f, vJump.z);
        vLand = Normalize(vLand);
        PblVector3 vDest = vLand * 25.0f;
        vDest += GetMe()->GetMatrix().GetTranslation();

        if (AIUtil::DirectLineToTarget(GetMe()->GetGameObject(), vDest))
        {
            mIsPathing = false;
            mMeleePathTime = -100.0f;
            mOwner->GetLowLevel().SetNavigator_Stationary();
            mOwner->GetLowLevel().SetJump(vJump);
            mJumpTimer = AIUtil::GetRandomFloat(5.0f, 10.0f) + GameLoop::GetMissionTime();
        }
        else
        {
            mJumpTimer = GameLoop::GetMissionTime() + 4.0f;
        }
    }

    if (!mOwner->GetLowLevel().IsJumping())
    {
        AIUtil::Slide(GetMe()->GetGameObject(), GetAttackHelper()->GetTarget(), true,
                      &mIsPathing, &mMeleePathTime, 2.0f, 0.0f);
    }
}

// FUNCTION: SWBF2 0x00660f40
void AttackPatterns::SetGlanceDest()
{
    const GameObject* pTarget = GetAttackHelper()->GetTarget();
    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vFwd = GetMe()->GetMatrix().GetZAxis();
    float fSpeed = GetMaxSpeed(GetMe()->GetGameObject());

    if (mOnApproach)
    {
        PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();
        PblVector3 vDir = AIUtil::GetSafeNormalXZ(vTarget - vPos);
        float fDot = DotProduct(vFwd, vDir);
        PblVector3 vSide = AIUtil::GetSafeNormalXZ(vFwd - vDir * fDot);

        float fMyRadius = GetCollisionObject(GetMe()->GetGameObject())->GetCollisionRadius();
        float fDist = GetCollisionObject(pTarget)->GetCollisionRadius() + fMyRadius + 2.0f;
        mGlancePos = vSide * fDist + vTarget;

        mGlancePos += fSpeed * 0.5f * AIUtil::GetSafeNormalXZ(mGlancePos - vPos);
    }
    else
    {
        float fAhead = fSpeed * 3.0f;
        mGlancePos = fAhead * vFwd + vPos;
    }

    mOwner->GetLowLevel().SetNavigator_Slide(mGlancePos, NULL);

    mClosestXZSq = SquaredLengthXZ(mGlancePos - GetMe()->GetMatrix().GetTranslation());
    mClosestTime = GameLoop::GetMissionTime();
    mIsPathing = false;
}

// FUNCTION: SWBF2 0x00661260
bool AttackPatterns::IsGlanceMakingProgress()
{
    const PblVector3& vPos = GetMe()->GetMatrix().GetTranslation();
    float fXZSq = SquaredLengthXZ(mGlancePos - vPos);
    if (mClosestXZSq > fXZSq)
    {
        mClosestXZSq = fXZSq;
        mClosestTime = GameLoop::GetMissionTime();
        return true;
    }
    return !(GameLoop::GetMissionTime() > mClosestTime + 10.0f);
}

// FUNCTION: SWBF2 0x006612e0
bool AttackPatterns::GonePastGlancePos() const
{
    PblVector3 vLeg = mGlancePos - mGlanceSrc;
    const PblVector3& vPos = GetMe()->GetMatrix().GetTranslation();
    return DotProduct(vPos - mGlancePos, vLeg) > 0.0f;
}

// FUNCTION: SWBF2 0x006613e0
void AttackPatterns::DoGlance()
{
    if (GetCtrl()->mLowLevel.IsAtDest() || GonePastGlancePos())
    {
        mOnApproach = !mOnApproach;
        SetGlanceDest();
    }

    if (!mIsPathing && !IsGlanceMakingProgress())
    {
        mOwner->GetLowLevel().SetNavigator_Goto(GetAttackHelper()->GetLastSeenTargetPos(), NULL, false, false);
        mIsPathing = true;
    }

    mOwner->GetLowLevel().SetCombat();
}

// FUNCTION: SWBF2 0x006614a0
void AttackPatterns::PickRetreatSpot()
{
    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();
    PblVector3 vAway = AIUtil::GetSafeNormalXZ(PblVector3(vPos.x - vTarget.x, vPos.y - vTarget.y, vPos.z - vTarget.z));

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, AIUtil::GetRandomFloat(-PblMath::fPi / 4.0f, PblMath::fPi / 4.0f));
    float fRotX = vAway.x * fCos - vAway.z * fSin;
    float fRotZ = vAway.z * fCos + vAway.x * fSin;
    vAway = PblVector3(fRotX, vAway.y, fRotZ);

    PblVector3 vDest(vAway.x * 100.0f + vPos.x, vAway.y * 100.0f + vPos.y, vAway.z * 100.0f + vPos.z);
    mOwner->GetLowLevel().SetNavigator_Slide(vDest, NULL);
}

// FUNCTION: SWBF2 0x006615e0
void AttackPatterns::DoRetreat()
{
    if (ToggleToCover(STATE_RETREAT, 1.0f) ||
        ToggleToJetJumpHint(STATE_RETREAT) ||
        ToggleToJetJumpCharge(STATE_RETREAT) ||
        ToggleToJetJump(STATE_RETREAT))
    {
        return;
    }

    if (GameLoop::GetMissionTime() > mRetreatTimer)
    {
        mRetreatTimer = AIUtil::GetRandomFloat(3.0f, 6.0f) + GameLoop::GetMissionTime();
        PickRetreatSpot();
    }

    mOwner->FaceTarget(HEIGHT_STAND);
}

// FUNCTION: SWBF2 0x006603a0
void AttackPatterns::InitSerpentine()
{
    mSerpAhead = FORWARD;
    mSerpStrafe = AIUtil::GetRandomInt(0, 1) ? RIGHT : LEFT;
    mSerpCur = mSerpAhead;
    mSerpTimer = 0.0f;
    mSerpRangeTimer = 0.0f;
    CycleSerpDir();
}

// FUNCTION: SWBF2 0x00660610
void AttackPatterns::InitFlank()
{
    mFlankTimer = 0.0f;
    mOwner->GetLowLevel().SetNavigator_Stationary();
}

// FUNCTION: SWBF2 0x00660870
void AttackPatterns::InitCriticalHit()
{
    mFlankTimer = 0.0f;
    mOwner->GetLowLevel().SetNavigator_Stationary();
}

// FUNCTION: SWBF2 0x00661390
void AttackPatterns::InitGlance()
{
    mOnApproach = true;
    mGlanceSrc = GetMe()->GetMatrix().GetTranslation();
    SetGlanceDest();
    GetAttackHelper()->SetWaitDelay(12.0f);
}

// FUNCTION: SWBF2 0x00661470
void AttackPatterns::CleanGlance()
{
    GetAttackHelper()->ResetWaitDelay();
}

// FUNCTION: SWBF2 0x00661480
void AttackPatterns::InitRetreat()
{
    mOwner->GetLowLevel().SetNavigator_Stationary();
    mRetreatTimer = -1.0f;
}

void AttackPatterns::InitCover()
{
}

void AttackPatterns::InitVehicleCover()
{
}

void AttackPatterns::CleanMelee()
{
}

void AttackPatterns::CleanJetJump()
{
}

// FUNCTION: SWBF2 0x00661680
void AttackPatterns::DoCover(float dt)
{
    mCoverHelper.FSMUpdate(dt, false);
    if (!mCoverHelper.IsDone())
    {
        mOwner->FaceTarget(mCoverHelper.HELPERGetHeight());
    }
    else
    {
        ToggleToReturnState();
    }
}

// FUNCTION: SWBF2 0x006616d0
void AttackPatterns::CleanCover()
{
    mCoverHelper.HELPERClean();
}

// FUNCTION: SWBF2 0x006616e0
void AttackPatterns::DoVehicleCover(float dt)
{
    mVehicleCoverHelper.FSMUpdate(dt, false);
    if (!mVehicleCoverHelper.IsDone())
    {
        mOwner->FaceTarget();
    }
    else
    {
        ToggleToReturnState();
    }
}

// FUNCTION: SWBF2 0x00661720
void AttackPatterns::CleanVehicleCover()
{
    mVehicleCoverHelper.HELPERClean();
}

// FUNCTION: SWBF2 0x00661a20
void AttackPatterns::InitJetJumpHint()
{
    mJetJumpHintHelper.HELPERInit(mJetJumpHint);
}

// FUNCTION: SWBF2 0x00661a30
void AttackPatterns::DoJetJumpHint(float dt)
{
    mJetJumpHintHelper.FSMUpdate(dt, false);
    if (mJetJumpHintHelper.IsDone())
    {
        ToggleToReturnState();
    }
}

// FUNCTION: SWBF2 0x00661a60
void AttackPatterns::CleanJetJumpHint()
{
    mJetJumpHintHelper.HELPERClean();
}

// FUNCTION: SWBF2 0x00661a70
bool AttackPatterns::ToggleToJetJumpCharge(int iReturnState)
{
    if (!GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return false;
    }

    Entity* pSoldier = GetMe()->GetEntity();
    const EntitySoldierClass* pClass = GetSoldierClass(pSoldier);
    const bool bHover = pClass->mJetTypeHover;
    if (!(pClass->mJetFuelRechargeRate > 0.0f) || !bHover)
    {
        return false;
    }
    const GameObject* pTarget = GetAttackHelper()->GetVisibleTarget();
    if (!pTarget)
    {
        return false;
    }

    const GameObject* pMeObj = GetMe()->GetGameObject();
    const PblVector3& vMe = pMeObj->GetPosition();
    const PblVector3& vTarget = pTarget->GetPosition();
    float fDist = Length(vMe - vTarget);
    if (fDist < 20.0f)
    {
        return false;
    }
    float fSpeed = GetMaxSpeed(reinterpret_cast<GameObject*>(pSoldier));
    if (fSpeed <= 0.0f || pClass->mJetFuelCost <= 0.0f)
    {
        return false;
    }
    if ((GetJetFuel(pSoldier) - pClass->mJetFuelInitialCost) / pClass->mJetFuelCost < fDist / fSpeed)
    {
        return false;
    }
    if (PathFinder::TestTrivialStraightLineHit(vMe, vTarget, pMeObj) == true)
    {
        return false;
    }

    mReturnState = iReturnState;
    HelperFsm::SetNextState(STATE_JETJUMPCHARGE);
    return true;
}

// FUNCTION: SWBF2 0x00661730
void AttackPatterns::InitJetJump()
{
    for (int iTries = 10; --iTries >= 0;)
    {
        PblVector3 vOffset = PblRandom::GetGlobalVector3() * 40.0f;
        vOffset.y = 0.0f;
        if (vOffset.x * vOffset.x + vOffset.y * vOffset.y + vOffset.z * vOffset.z < 400.0f)
        {
            continue;
        }

        const PblVector3 vTarget = GetMe()->GetMatrix().GetTranslation() + vOffset;
        const GameObject* pMeObj = GetMe()->GetGameObject();
        if (!PathFinder::IsPointValid(vTarget, AIUtil::GetCollisionRadius(pMeObj), AIUtil::GetAIType(pMeObj),
                                      0, 0, 0))
        {
            continue;
        }

        const PblVector3 vAbove = vTarget + PblVector3(0.0f, 40.0f, 0.0f);
        const GameObject* pIgnore = GetMe()->GetGameObject();
        if (CollisionManager::RayTest(vTarget, vAbove, &pIgnore, 1, 0x9a))
        {
            continue;
        }

        if (!AIUtil::gAIAllowBlindJetJumps &&
            PathFinder::TestTrivialStraightLineHit(GetMe()->GetMatrix().GetTranslation(), vTarget,
                                                   AIUtil::GetCollisionRadius(pMeObj),
                                                   AIUtil::GetAIType(pMeObj), 0, 0, false) == true)
        {
            continue;
        }

        mOwner->GetLowLevel().SetNavigator_Goto(vTarget, 0, false, false);
        mOwner->GetLowLevel().SetJetJump(PblVector3(0.0f, 0.0f, 0.0f));
        mJumpStartTime = GameLoop::GetMissionTime();
        mJumping = true;
        return;
    }

    mJumpStartTime = -10.0f;
    ToggleToReturnState();
}

// FUNCTION: SWBF2 0x006619e0
void AttackPatterns::DoJetJump(float)
{
    if (GameLoop::GetMissionTime() > mJumpStartTime + 1.0f && !mOwner->GetLowLevel().mJetJumping)
    {
        mJumping = false;
        ToggleToReturnState();
    }
}

// FUNCTION: SWBF2 0x00661c00
void AttackPatterns::InitJetJumpCharge()
{
    const GameObject* pTarget = GetAttackHelper()->GetVisibleTarget();
    if (!pTarget)
    {
        return;
    }

    const PblMatrix& mMe = GetMe()->GetMatrix();
    const PblVector3& vTarget = GetGeometryCollision(pTarget)->GetPosition();
    const PblVector3& vMe = mMe.GetTranslation();
    PblVector3 vDiff;
    vDiff.x = vTarget.x - vMe.x;
    vDiff.y = vTarget.y - vMe.y;
    vDiff.z = vTarget.z - vMe.z;
    PblVector3 vDir;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vDiff));
    mOwner->GetLowLevel().SetJetJump((vDir + PblVector3(0.0f, 1.0f, 0.0f)) * 0.70710677f);
    mOwner->GetLowLevel().SetNavigator_Slide(pTarget, -3.0f, false, NULL);
}

// FUNCTION: SWBF2 0x00661d00
void AttackPatterns::DoJetJumpCharge(float dt)
{
    const GameObject* pTarget = GetAttackHelper()->GetVisibleTarget();
    if (!pTarget)
    {
        ToggleToReturnState();
        return;
    }

    const PblMatrix& mMe = GetMe()->GetMatrix();
    const PblVector3& vTarget = GetGeometryCollision(pTarget)->GetPosition();
    const PblVector3& vMe = mMe.GetTranslation();
    float fZ = vMe.z - vTarget.z;
    float fX = vMe.x - vTarget.x;
    float fDistSq = fX * fX + fZ * fZ;
    if (9.0f > fDistSq)
    {
        ToggleToReturnState();
        return;
    }
    if (GetJetFuel(GetMe()->GetEntity()) <= 0.0f)
    {
        ToggleToReturnState();
        return;
    }

    if (fDistSq > 100.0f)
    {
        mOwner->GetLowLevel().SetHeading(PblVector3(vTarget.x + 0.0f, vTarget.y + 10.0f, vTarget.z + 0.0f),
                                         0.0f);
    }
    else
    {
        mOwner->FaceTarget(HEIGHT_STAND);
    }
}

// FUNCTION: SWBF2 0x00661e00
void AttackPatterns::CleanJetJumpCharge()
{
    mOwner->GetLowLevel().mJetJumpStop = true;
}

// FUNCTION: SWBF2 0x00661e10
void AttackPatterns::InitLocate()
{
    GetAttackHelper()->SetWaitDelay(16.0f);
    mOwner->GetLowLevel().SetNavigator_Slide(GetAttackHelper()->GetLastSeenTargetPos(), NULL);
    mLocateAtDest = false;
}

// FUNCTION: SWBF2 0x00661e50
void AttackPatterns::DoLocate()
{
    if (GetAttackHelper()->GetVisibleTarget())
    {
        ToggleToReturnState();
        return;
    }

    if (GetCtrl()->mLowLevel.IsAtDest() && !mLocateAtDest)
    {
        mLocateAtDest = true;
        GetAttackHelper()->ResetWaitDelay();
        PblVector3 vRandom = PblRandom::GetGlobalVector3();
        const Controllable* pMe = GetMe();
        const PblVector3 vOffset = AIUtil::GetSafeNormalXZ(vRandom) * 100.0f;
        const PblVector3& vMe = pMe->GetMatrix().GetTranslation();
        PblVector3 vDest(vMe.x + vOffset.x, vMe.y + vOffset.y, vMe.z + vOffset.z);
        mOwner->GetLowLevel().SetHeading(vDest, 0.0f);
        return;
    }

    mOwner->FaceTarget(HEIGHT_STAND);
}

// FUNCTION: SWBF2 0x00661f70
void AttackPatterns::CleanLocate()
{
    GetAttackHelper()->ResetWaitDelay();
}
