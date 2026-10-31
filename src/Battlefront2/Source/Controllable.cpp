#include <d3dx9math.h>
#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <stdio.h>
#include <stdlib.h>

#include "Controllable.h"
#include "ControllerManager.h"
#include "AILowLevel.h"
#include "Character.h"
#include "LockOnManager.h"
#include "NetGame.h"
#include "UnitController.h"
#include "WeaponCannonInstance.h"
#include "WeaponShield.h"
#include "CollisionObject.h"
#include "GameObject.h"
#include "MedalsMgr.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Aimer.h"
#include "EntitySoldier.h"

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};




// FUNCTION: SWBF2 0x00484730
ControllableClass::ControllableClass()
    : mLocalNameID(0),
      mPilotParentNodeHash(0),
      mPilotOffsetMatrix(gMatrixIdentity),
      mPilotAnimation(SoldierAnimationBank::INVALID_ANIMATION),
      mAnimatedPilotNodeFlag(false),
      mPilotDeathType(PILOTDEATH_FALL),
      mPilotType(PILOT_VEHICLE),
      mIsPilotExposed(true),
      mIsPilotAnimation9Pose(false),
      mCanCapturePosts(false),
      mCanEnterVehicles(false)
{
    mCombatInterrupts = true;
    mIgnoreHintNodes = false;
    mShakeScale = 1.0f;
    mBobPath.numPoints = 0;
    mCockpitTension = 0.0f;
    mControlsUnit = false;
    mAnimatesUnitAsTurret = false;
    mForceMode = 0;
    mFirstPersonFOV = mThirdPersonFOV = GetDefaultFOV();
}

UnitController* UnitControllerPtr(Controller* pController);

extern int netTurnsPerSecond;

namespace AIUtil
{
GameObject* GetAlivePtr(PblHandle<GameObject> const& hObj, bool bIncludeBuildings);
}

static inline unsigned int GameObjectHandleId(const GameObject* pObject)
{
    return pObject->mHandleId;
}

// FUNCTION: SWBF2 0x004833b0
float Controllable::GetSoundRadius() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x00483510
GameObject* LockOnManager::GetLockedTarget() const
{
    return mLockedObj;
}

// FUNCTION: SWBF2 0x00483540
void LockOnManager::ForceNewTargetLock(GameObject* pObject)
{
    mForceNewLock = 5;
    mForceNewLockTarget = pObject;
}

// FUNCTION: SWBF2 0x00483580
unsigned int LockOnManager::GetTargetLockedChangedTurn() const
{
    return mLockedObjChangedTurn;
}

// FUNCTION: SWBF2 0x004835a0
Controllable::Controllable(ControllableClass* pClass, int iPlayerId)
    : mControlMove(0.0f),
      mControlStrafe(0.0f),
      mControlTurn(0.0f),
      mControlPitch(0.0f),
      mAIThread(this),
      mCtrl(0),
      mCharacter(0),
      mPilot((pClass->mPilotType == PILOT_SELF || pClass->mPilotType == PILOT_VEHICLESELF) ? this : 0),
      mPlayerId(iPlayerId),
      mInitialPlayerId(iPlayerId),
      mIndirectControlFlag(false),
      mEnemyLockedOnMeState(0.0f),
      mEnemyLockedOnMeTimestamp(0.0f),
      mEnemyLockedOnMeMissile(0),
      mEnemyLockedOnMeDistance(FLT_MAX),
      mPilotType(pClass->mPilotType)
{
    ClearControllable();
    Initialize();
    NetGame::AddLateFence(mPlayerId);
}

// FUNCTION: SWBF2 0x004837c0
void Controllable::ClearControllable()
{
    memset(mReticuleTarget, 0, sizeof(mReticuleTarget));
    memset(mReticuleAffiliation, 0, sizeof(mReticuleAffiliation));

    mControlMove = 0.0f;
    mControlStrafe = 0.0f;
    mControlTurn = 0.0f;
    mControlPitch = 0.0f;
    for (int i = 0; i < 2; ++i)
    {
        mControlSwitch[i] = 0.0f;
        mTriggerControlSwitch[i].Clear();
        mControlFire[i].Clear();
    }
    mControlReload.Clear();
    mControlSprint.Clear();
    mControlJump.Clear();
    mControlCrouch.Clear();
    mControlUse.Clear();
    mControlZoom.Clear();
    mControlView.Clear();
    mControlLockTarget.Clear();
    mControlSquadCommand.Clear();
    mControlSpecialMove.Clear();
    mControlThrustFwd.Clear();
    mControlThrustBack.Clear();
    mControlStrafeLeft.Clear();
    mControlStrafeRight.Clear();

    mWpnDistFactor = 1.0f;
    mWpnChannel = 0;
    mAnimNameId = 0;
    mTurnBuildup = 0.0f;
    mPitchBuildup = 0.0f;
    mTargetLockedObj.mObject = 0;
    mTargetLockedObj.mSavedHandleId = 0;
    mAllowedLockTypes = 0;
    mTurnAdjusted = 2.0f;
    mPitchAdjusted = 2.0f;
    mTurnAuto = 0.0f;
    mPitchAuto = 0.0f;
    mLockBreakTimer = 0.0f;
    mUsingTurnAdjusted = false;
    mCollisionObj.mObject = 0;
    mCollisionObj.mSavedHandleId = 0;
    mEnemyLockedOnMeState = 0.0f;
    mEnemyLockedOnMeTimestamp = 0.0f;
    mEnemyLockedOnMeMissile = 0;
    mEnemyLockedOnMeDistance = FLT_MAX;

    if (UnitControllerPtr(mCtrl))
    {
        UnitControllerPtr(mCtrl)->mLowLevel.ClearControllable();
    }
}

// FUNCTION: SWBF2 0x004839e0
void Controllable::SetSwitch(float fValue, int iIndex)
{
    if (mTriggerControlSwitch[iIndex].IsPressed())
    {
        mControlSwitch[iIndex] = fValue;
    }
    else
    {
        mControlSwitch[iIndex] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00483a10
Controllable* Controllable::GetEnteredRemoteOrVehicleIfNotMe()
{
    if (!mCharacter)
    {
        return 0;
    }
    if (mCharacter->mRemote)
    {
        return mCharacter->mRemote != this ? mCharacter->mRemote : 0;
    }
    if (mCharacter->mVehicle)
    {
        return mCharacter->mVehicle != this ? mCharacter->mVehicle : 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00483a40
void Controllable::ResetControls()
{
    mControlMove = 0.0f;
    mControlStrafe = 0.0f;
    mControlTurn = 0.0f;
    mControlPitch = 0.0f;
    mAnimNameId = 0;
    mWpnDistFactor = 1.0f;
    if (UnitControllerPtr(mCtrl))
    {
        UnitControllerPtr(mCtrl)->mLowLevel.ResetControls();
    }
}

// FUNCTION: SWBF2 0x00483ab0
void Controllable::Initialize()
{
    if (UnitControllerPtr(mCtrl))
    {
        UnitControllerPtr(mCtrl)->mLowLevel.Initialize();
    }
    mPlayAnim = false;
}

// SYNTHETIC: SWBF2 0x004850ef
// ??_EControllable@@WBI@AEPAXI@Z
// FUNCTION: SWBF2 0x00483af0
Controllable::~Controllable()
{
    ReleaseController();
}

// FUNCTION: SWBF2 0x00483ba0
void Controllable::Kill()
{
    NetGame::AddLateFence(mPlayerId);
    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mPilot)
    {
        Controllable* pPilot = mPilot;
        PilotType eType = mPilotType;
        if (eType == PILOT_VEHICLESELF)
        {
            eType = (pPilot != this) ? PILOT_VEHICLE : PILOT_SELF;
        }
        switch (eType)
        {
        case PILOT_SELF:
            if (mCharacter)
            {
                if (mCharacter->mVehicle)
                {
                    mCharacter->mVehicle->SetPilot(0);
                    mCharacter->SetVehicle(0);
                }
                if (mCharacter->mRemote)
                {
                    ExitRemote();
                }
                mCharacter->Died();
                SetCharacter(0);
            }
            break;
        case PILOT_VEHICLE:
        {
            bool bAttached = false;
            if (GetGameObject())
            {
                if (GetGameObject()->GetCollisionObject())
                {
                    bAttached = ((CollisionObject*)(pPilot->GetGameObject()->GetCollisionObject()))->GetCollisionType() == COLL_ATTACHED;
                }
                GameObject* pObject = GetGameObject();
                pPilot->GetGameObject()->InstantDeath(pObject->mDamageOwner, false);
            }
            pPilot->Kill();
            if (!bAttached && !(NetGame::IsEnabled() && netEnabledNext && netClientActive))
            {
                delete pPilot;
            }
            break;
        }
        case PILOT_REMOTE:
            pPilot->ExitRemote();
            break;
        }
        mPilot = 0;
    }
    ClearControllable();
    ReleaseController();
}

// FUNCTION: SWBF2 0x00483d50
void Controllable::SetExitVehicle(const CommandPost* pPost)
{
    if (mPilot && mPilot != this)
    {
        mPilot->SetExitVehicle(pPost);
    }
    if (UnitControllerPtr(mCtrl))
    {
        UnitControllerPtr(mCtrl)->mLowLevel.SetExitVehicle(pPost);
    }
}

// FUNCTION: SWBF2 0x00483da0
void Controllable::AddController(Controller* pController, bool bIndirect)
{
    if (mCtrl)
    {
        return;
    }
    mCtrl = pController;
    SetIndirectControl(bIndirect);
    if (NetGame::IsEnabled() && NetGame::IsClient() && mPlayerId >= 0)
    {
        int iCount = GetWeaponCount();
        for (int i = 0; i < iCount; ++i)
        {
            Weapon* pWeapon = GetWeaponPtr(i);
            if (pWeapon)
            {
                pWeapon->ActivatePhysics();
            }
        }
    }
    ControllerManager::AddController(mCtrl);
}

// FUNCTION: SWBF2 0x00483e30
void Controllable::ReleaseController()
{
    if (mCtrl)
    {
        ControllerManager::RemoveController(mCtrl);
        mCtrl->KillController();
        delete mCtrl;
        mCtrl = 0;
        SetIndirectControl(false);
    }
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        int iCount = GetWeaponCount();
        for (int i = 0; i < iCount; ++i)
        {
            Weapon* pWeapon = GetWeaponPtr(i);
            if (pWeapon && !pWeapon->IsRtti(WeaponShield::GetRtti()))
            {
                pWeapon->DeactivatePhysics();
            }
        }
    }
}

// FUNCTION: SWBF2 0x00483ef0
void Controllable::SetIndirectControl(bool bIndirect)
{
    if (!mIndirectControlFlag && bIndirect)
    {
        mAIThread.ActivateThread(10, PER_TURN);
    }
    else if (mIndirectControlFlag && !bIndirect)
    {
        mAIThread.DeactivateThread();
    }
    mIndirectControlFlag = bIndirect;
}

// FUNCTION: SWBF2 0x00483f50
void Controllable::SetCurrentCollision(GameObject* pObject)
{
    mCollisionObj.mObject = pObject;
    mCollisionObj.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
}

// FUNCTION: SWBF2 0x00483f80
GameObject* Controllable::GetCurrentCollision() const
{
    return AIUtil::GetAlivePtr(mCollisionObj, true);
}

// FUNCTION: SWBF2 0x00483f90
void Controllable::SetWeaponIndex(int iChannel, int iIndex)
{
}

// FUNCTION: SWBF2 0x00483fa0
void Controllable::SetCharacter(Character* pCharacter)
{
    mCharacter = pCharacter;
    mPlayerId = pCharacter ? pCharacter->mPlayerId : -1;
    if (pCharacter)
    {
        mInitialPlayerId = mPlayerId;
    }
}

// FUNCTION: SWBF2 0x00483fd0
void Controllable::SetPilot(Controllable* pPilot)
{
    if (mPilot == pPilot)
    {
        return;
    }
    GameObject* pObject = GetGameObject();
    if (mPilotType == PILOT_VEHICLESELF && mPilot == this)
    {
        mPilot = 0;
    }
    if (mPilot)
    {
        GameObject* pPilotObject = mPilot->GetGameObject();
        if (pObject &&
            (IsPilotable() || IsRemoteVehicle()) &&
            pPilotObject)
        {
            pPilotObject->GetCollisionObject()->mParent = 0;
            if (pObject->GetControllable() == this &&
                ((pPilotObject->GetGameObjectClass()->GetSoldierType() == GameObjectClass::SOLDIER_PILOT &&
                  pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) && pObject->mMaxHealth > 1400.0f) ||
                 (mPilot->mCharacter && mPilot->mCharacter->mMedalsMgr &&
                  mPilot->mCharacter->mMedalsMgr->IsAwardAvailable(3))))
            {
                pObject->AddPilotSkillCount(-1);
            }
        }
    }
    ClearControllable();
    ReleaseController();
    mPilot = pPilot;
    if (pPilot)
    {
        GameObject* pPilotObject = pPilot->GetGameObject();
        if (pObject && pPilotObject)
        {
            int iTeam = pPilotObject->GetTeam();
            pObject->SetTeam(iTeam);
            pObject->mPerceivedTeam = iTeam;
        }
        SetCharacter(mPilot->mCharacter);
        SetupController();
        if (pObject &&
            (IsPilotable() || IsRemoteVehicle()) &&
            pPilotObject)
        {
            pPilotObject->GetCollisionObject()->mParent = pObject->GetCollisionObject();
            if (pObject->GetControllable() == this &&
                ((pPilotObject->GetGameObjectClass()->GetSoldierType() == GameObjectClass::SOLDIER_PILOT &&
                  pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) && pObject->mMaxHealth > 1400.0f) ||
                 (mPilot->mCharacter && mPilot->mCharacter->mMedalsMgr &&
                  mPilot->mCharacter->mMedalsMgr->IsAwardAvailable(3))))
            {
                pObject->AddPilotSkillCount(1);
            }
        }
    }
    else
    {
        if (pObject && pObject->GetGameObjectClass() && pObject->GetGameObjectClass()->IsNeutralWhenEmpty())
        {

            bool bOccupied = false;
            for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
            {
                Controllable* pControllable = it;
                if (pControllable->mPilotType == PILOT_SELF ||
                    (pControllable->mPilotType == PILOT_VEHICLESELF && pControllable->mPilot == pControllable))
                {
                    continue;
                }
                if (pControllable->mPilot)
                {
                    bOccupied = true;
                    break;
                }
            }
            if (!bOccupied)
            {
                pObject->SetTeam(0);
                pObject->mPerceivedTeam = 0;
            }
        }
        SetCharacter(0);
    }
    if (mPilotType == PILOT_VEHICLESELF && !mPilot)
    {
        mPilot = this;
        SetupController();
    }
}

// FUNCTION: SWBF2 0x00484290
bool Controllable::Update(float fDeltaTime)
{
    UpdateFirstThirdPerson(mPlayerId);
    return true;
}

// FUNCTION: SWBF2 0x004842b0
const Aimer* Controllable::GetCurAimer() const
{
    int iIndex = GetWeaponIndex(mWpnChannel);
    if (iIndex < 0)
    {
        return 0;
    }
    return GetAimer(iIndex);
}

// FUNCTION: SWBF2 0x004842e0
const Weapon* Controllable::GetCurWpn() const
{
    int iIndex = GetWeaponIndex(mWpnChannel);
    if (iIndex < 0)
    {
        return 0;
    }
    return GetWeaponPtr(iIndex);
}

// FUNCTION: SWBF2 0x00484310
Weapon* Controllable::GetCurWpn()
{
    int iIndex = GetWeaponIndex(mWpnChannel);
    if (iIndex < 0)
    {
        return 0;
    }
    return GetWeaponPtr(iIndex);
}

// FUNCTION: SWBF2 0x00484340
int Controllable::GetWpnChannel() const
{
    return mWpnChannel;
}

// FUNCTION: SWBF2 0x00484350
float Controllable::GetWpnDistFactor() const
{
    return mWpnDistFactor;
}

// FUNCTION: SWBF2 0x00484360
void Controllable::SetWpnDistFactor(float fFactor)
{
    mWpnDistFactor = fFactor;
}

// FUNCTION: SWBF2 0x00484370
float Controllable::GetTurnAdjusted()
{
    if (mTurnAdjusted < 1.5f)
    {
        return mTurnAdjusted;
    }
    return mControlTurn;
}

// FUNCTION: SWBF2 0x00484390
float Controllable::GetPitchAdjusted()
{
    if (mPitchAdjusted < 1.5f)
    {
        return mPitchAdjusted;
    }
    return mControlPitch;
}

// FUNCTION: SWBF2 0x004843b0
GameObject* Controllable::GetTargetLockedObj()
{
    if (mPlayerId >= 0 && !GetEnteredRemoteOrVehicleIfNotMe())
    {
        if (!NetGame::IsEnabled() || NetGame::IsClient() ||
            (NetGame::IsOneWorld() && NetGame::IsHost()))
        {
            int iLocal = NetGame::GetJoystickIndex(mPlayerId);
            if (iLocal >= 0)
            {
                LockOnManager* pManager = LockOnManager::GetByPlayerIndex(iLocal);
                if (pManager)
                {
                    GameObject* pLocked = pManager->GetLockedTarget();
                    mTargetLockedObj = pLocked;
                }
            }
        }
    }

    return mTargetLockedObj;
}

// FUNCTION: SWBF2 0x00484460
void Controllable::TargetTrackingForceClear()
{
    mTurnAdjusted = 2.0f;
    mPitchAdjusted = 2.0f;
    mTurnAuto = 0.0f;
    mPitchAuto = 0.0f;
    SetTargetLockedObj(0);
}

// FUNCTION: SWBF2 0x004844a0
void Controllable::SetTargetLockedObj(GameObject* pObject)
{
    if (mPlayerId < 0 || GetEnteredRemoteOrVehicleIfNotMe())
    {
        mTargetLockedObj = pObject;
        return;
    }

    if (pObject != mTargetLockedObj)
    {
        mTargetLockedObj = pObject;
    }

    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsOneWorld() && NetGame::IsHost()))
    {
        int iLocal = NetGame::GetJoystickIndex(mPlayerId);
        if (iLocal < 0)
        {
            return;
        }
        LockOnManager* pManager = LockOnManager::GetByPlayerIndex(iLocal);
        if (!pManager)
        {
            return;
        }
        GameObject* pLocked = pManager->GetLockedTarget();
        if (pLocked == pObject)
        {
            return;
        }

        if (NetGame::IsEnabled() && !(NetGame::IsOneWorld() && NetGame::IsHost()))
        {
            unsigned int uChangedTurn = pManager->mLockedObjChangedTurn;
            int iGrace = netTurnsPerSecond / 2;
            unsigned int uTurn = NetGame::GetClientAckedTurnNumber();
            if (uChangedTurn + (iGrace > 5 ? iGrace : 5) >= uTurn)
            {
                mTargetLockedObj = pLocked;
                return;
            }
        }

        if (pObject)
        {
            pManager->ForceNewTargetLock(pObject);
        }
        else
        {
            pManager->RemoveTargetLock();
        }
    }
}

// FUNCTION: SWBF2 0x00484800
ControllableClass::ControllableClass(ControllableClass* pBase)
{
    mPilotParentNodeHash = pBase->mPilotParentNodeHash;
    mPilotOffsetMatrix = pBase->mPilotOffsetMatrix;
    mPilotAnimation = pBase->mPilotAnimation;
    mIsPilotAnimation9Pose = pBase->mIsPilotAnimation9Pose;
    mAnimatedPilotNodeFlag = pBase->mAnimatedPilotNodeFlag;
    mPilotType = pBase->mPilotType;
    mIsPilotExposed = pBase->mIsPilotExposed;
    mCanCapturePosts = pBase->mCanCapturePosts;
    mCanEnterVehicles = pBase->mCanEnterVehicles;
    mCombatInterrupts = pBase->mCombatInterrupts;
    mIgnoreHintNodes = pBase->mIgnoreHintNodes;
    mShakeScale = pBase->mShakeScale;
    mBobPath = pBase->mBobPath;
    mCockpitTension = pBase->mCockpitTension;
    mControlsUnit = pBase->mControlsUnit;
    mAnimatesUnitAsTurret = pBase->mAnimatesUnitAsTurret;
    mLocalNameID = pBase->mLocalNameID;
    mForceMode = pBase->mForceMode;
    mThirdPersonFOV = pBase->mThirdPersonFOV;
    mFirstPersonFOV = pBase->mFirstPersonFOV;
}

// FUNCTION: SWBF2 0x00484930
bool ControllableClass::SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel)
{
    float fX = 0.0f;
    float fY = 0.0f;
    float fZ = 0.0f;
    int iIndex = 0;
    char szBuffer[128];

    switch (uProperty)
    {
    case 0x11CE2025:
        mAnimatedPilotNodeFlag = true;
        mPilotParentNodeHash = PblTEMPHash(pszValue);
        return true;
    case 0x104CD1B8:
        sscanf(pszValue, "%f", &mCockpitTension);
        return true;
    case 0x20728C12:
        sscanf(pszValue, "%f", &mThirdPersonFOV);
        mThirdPersonFOV *= 0.017453292f;
        return true;
    case 0x4785D6B7:
        sscanf(pszValue, "%f", &mFirstPersonFOV);
        mFirstPersonFOV *= 0.017453292f;
        return true;
    case 0x325F7C20:
        mCanCapturePosts = atoi(pszValue) != 0;
        return true;
    case 0x2B60E217:
        mIsPilotExposed = atoi(pszValue) != 0;
        return true;
    case 0x44C77C95:
        mControlsUnit = atoi(pszValue) != 0;
        return true;
    case 0x83192029:
        mAnimatesUnitAsTurret = atoi(pszValue) != 0;
        return true;
    case 0x848819E7:
        mIgnoreHintNodes = atoi(pszValue) != 0;
        return true;
    case 0x939B9985:
        mCanEnterVehicles = atoi(pszValue) == 0;
        return true;
    case 0x72CCBDE1:
        sscanf(pszValue, "%f", &mShakeScale);
        return true;
    case 0x9D1F5707:
        sscanf(pszValue, "%d", &mForceMode);
        return true;
    case 0x8A301535:
        sscanf(pszValue, "%d", &mBobPath.numPoints);
        return true;
    case 0xEF20A0A0:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.Vec[iIndex].x = fX;
        mBobPath.Vec[iIndex].y = fY;
        mBobPath.Vec[iIndex].z = fZ;
        return true;
    case 0x49824403:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.Slope[iIndex].x = fX;
        mBobPath.Slope[iIndex].y = fY;
        mBobPath.Slope[iIndex].z = fZ;
        return true;
    case 0x35341815:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.Ore[iIndex].x = fX;
        mBobPath.Ore[iIndex].y = fY;
        mBobPath.Ore[iIndex].z = fZ;
        return true;
    case 0xD67D9BA2:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.RSlope[iIndex].x = fX;
        mBobPath.RSlope[iIndex].y = fY;
        mBobPath.RSlope[iIndex].z = fZ;
        return true;
    case 0xBADF3672:
        sscanf(pszValue, "%i %f", &iIndex, &fX);
        mBobPath.Time[iIndex] = fX;
        return true;
    case 0xA976D065:
        mIsPilotAnimation9Pose = true;

    case 0x6E4FC069:
        mPilotAnimation = SoldierAnimationBank::AddAnimation(pszValue);
        return true;
    case 0x51CA39A6:
    {

        unsigned int uBone = PblTEMPHash(pszValue);
        if (pModel->GetHierarchicalBoneMatrix(uBone, mPilotOffsetMatrix))
        {
            mPilotParentNodeHash = pModel->GetParentBone(uBone);
        }
        return true;
    }
    case 0xDA1B8C0C:
        sscanf(pszValue, "%s", szBuffer);
        mLocalNameID = PblHash(szBuffer);
        return true;
    case 0x91EE6929:
        switch (PblHash(pszValue))
        {
        case 0xADA7AFDB: mPilotType = PILOT_NONE; break;
        case 0x645BA277: mPilotType = PILOT_SELF; break;
        case 0x71CD882F: mPilotType = PILOT_VEHICLE; break;
        case 0xC78D7953: mPilotType = PILOT_REMOTE; break;
        case 0x1FE70361: mPilotType = PILOT_VEHICLESELF; break;
        }
        return true;
    case 0x81BCE0F9:
        switch (PblHash(pszValue))
        {
        case 0xBFF162E2: mPilotDeathType = PILOTDEATH_FLY; break;
        case 0xF85A0760: mPilotDeathType = PILOTDEATH_FALL; break;
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00484ed0
void ControllableClass::UpdatePilotAnimation(Controllable* pPilot, const RedPose* pPose,
                                             const PblMatrix& mMatrix, const PblVector3& vOffset,
                                             float fDeltaTime, float f1, float f2, float f3)
{
    if (!pPilot)
    {
        return;
    }

    if (mPilotAnimation == SoldierAnimationBank::INVALID_ANIMATION)
    {
        pPilot->SetPilotInfo(mMatrix, vOffset, SoldierAnimationBank::INVALID_ANIMATION, false,
                             f1, f2, f3, fDeltaTime);
    }
    else
    {
        const PblMatrix* pNode = 0;
        if (pPose && mPilotParentNodeHash)
        {
            pNode = pPose->Find(mPilotParentNodeHash);
        }
        if (mAnimatedPilotNodeFlag)
        {
            PblMatrix mPilot;
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mPilot), reinterpret_cast<const D3DXMATRIX*>(pNode), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
            pPilot->SetPilotInfo(mPilot, vOffset, mPilotAnimation, mIsPilotExposed,
                                 f1, f2, f3, fDeltaTime);
        }
        else
        {
            PblMatrix mPilot;
            if (pNode)
            {
                PblMatrix mSeat;
                D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mSeat), reinterpret_cast<const D3DXMATRIX*>(&mPilotOffsetMatrix), reinterpret_cast<const D3DXMATRIX*>(pNode));
                D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mPilot), reinterpret_cast<const D3DXMATRIX*>(&mSeat), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
            }
            else
            {
                D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mPilot), reinterpret_cast<const D3DXMATRIX*>(&mPilotOffsetMatrix), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
            }
            pPilot->SetPilotInfo(mPilot, vOffset, mPilotAnimation, mIsPilotExposed,
                                 f1, f2, f3, fDeltaTime);
        }
    }

    if (mControlsUnit || mAnimatesUnitAsTurret)
    {
        Aimer* pAimer = const_cast<Aimer*>(pPilot->GetAimer(0));
        if (pAimer)
        {
            pAimer->Update(fDeltaTime);
        }
    }
}

// FUNCTION: SWBF2 0x00483400
EnergyBar* Controllable::GetEnergyBar()
{
    return 0;
}

// FUNCTION: SWBF2 0x00485080
GameObject* Controllable::GetVehicleIfAttached(GameObject* gameObject)
{
    if (gameObject)
    {
        Controllable* controllable = gameObject->GetControllable();
        if (controllable)
        {
            Character* character = controllable->mCharacter;
            if (character)
            {
                Controllable* vehControllable = character->GetVehicle();
                if (vehControllable)
                {
                    return vehControllable->GetGameObject();
                }
            }
        }
    }
    return gameObject;
}

// FUNCTION: SWBF2 0x004833c0
float Controllable::GetAimTurnRate() const
{
    return 0.0f;

}

// FUNCTION: SWBF2 0x004833d0
int Controllable::GetWeaponCount() const
{
    return 0;

}

// FUNCTION: SWBF2 0x004833e0
int Controllable::GetWeaponIndex(int) const
{
    return 0;

}

// FUNCTION: SWBF2 0x004833f0
Weapon* Controllable::GetWeaponPtr(int) const
{
    return 0;

}

// FUNCTION: SWBF2 0x00483410
const Aimer* Controllable::GetAimer(int) const
{
    return 0;

}

// FUNCTION: SWBF2 0x00484660
void Controllable::GetPilotMatrix(const PblMatrix& mIn, PblMatrix& mOut)
{
    Controllable* pPilot = mPilot;
    if (pPilot)
    {
        const PblMatrix& mPilotMatrix = pPilot->GetMatrix();
        const PblMatrix mInverse = static_cast<EntitySoldier*>(pPilot)->mLastVehicleMatrix.GetInverseFast();
        PblMatrix mResult;
        PblMatrix mRelative;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mResult), reinterpret_cast<const D3DXMATRIX*>(&mPilotMatrix), reinterpret_cast<const D3DXMATRIX*>(&mInverse));
        mRelative = mResult;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mResult), reinterpret_cast<const D3DXMATRIX*>(&mRelative), reinterpret_cast<const D3DXMATRIX*>(&mIn));
        mOut = mResult;
    }
}

