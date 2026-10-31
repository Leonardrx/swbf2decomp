#include <d3dx9math.h>
#include <float.h>
#include "PebbleFL/Common/PblVector.h"

#include "EntityDroideka.h"
#include "WeaponCannonInstance.h"
#include "Tracker.h"
#include "FirstPerson.h"
#include "EnergyBar.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include <math.h>
#include "Character.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "FLEffect.h"
#include "GameModel.h"
#include "NetGame.h"
#include "PlayerController.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include <stdlib.h>
#include "CameraManager.h"
#include "CollisionBody.h"
#include "CollisionPrimitive.h"
#include "Explosion.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "Zephyr/Source/ZephyrAnimJoint.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"
#include <new>
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Restrictor.h"
#include "TargetManager.h"
#include "UnitController.h"
#include "UnitMobileController.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "FoleyFXWalker.h"
#include "LockOnManager.h"
#include "MedalsMgr.h"
#include "TriggerRegionManager.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/RedWater.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

#include "StatusDisplay.h"

#include "FlagItem.h"
#include "GameLoop.h"
#include "PebbleFL/Common/PblRandom.h"

#include "CollisionModel.h"
#include "GameObject.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblMatrix.h"

// GLOBAL: SWBF2 0x007b2630
// __xmm@00000000000000003f80000000000000

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp,
                          const PblVector3& vPos);

// FUNCTION: SWBF2 0x004a6110
void EntityDroideka::DeactivatePhysics()
{
    mPostCollision.DeactivateThread();
    static_cast<Controllable*>(this)->DeactivateThread();
}

// FUNCTION: SWBF2 0x004a5f60
void EntityDroideka::SetupCameraZoomFactor(float& rZoom)
{
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0)
    {
        Weapon* pWeapon = mWeapon[mWeaponIndex[0]];
        if (pWeapon)
        {
            rZoom = pWeapon->mZoom;
            return;
        }
    }
    rZoom = 1.0f;
}

// FUNCTION: SWBF2 0x004a5ef0
void EntityDroideka::SetupCameraFOV(float& rFOV)
{
    if (IsAlive())
    {
        if (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView()))
        {
            rFOV = GetControllableClass()->mFirstPersonFOV;
        }
        else
        {
            rFOV = GetControllableClass()->mThirdPersonFOV;
        }
    }
    else
    {
        rFOV = GetDefaultFOV();
    }
}

// FUNCTION: SWBF2 0x004a5b40
void EntityDroideka::SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix)
{
    bool bFirstPerson = IsAlive() &&
        ((mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]] &&
          mWeapon[mWeaponIndex[0]]->ZoomFirstPerson()) ||
         (mTracker && mTracker->IsFirstPersonView()));
    if (bFirstPerson)
    {
        rMatrix = DirectionMatrix((-mEyeDir),
                                  mMatrix.GetYAxis(), mEyePoint);
        return;
    }
    PblVector3 vTarget = mEyePoint + mEyeDir * 65536.0f;
    GetCameraTrackSettings()->SetupCameraTrackMatrixHUDTargeting(rMatrix, mMatrix, vTarget);
}

// FUNCTION: SWBF2 0x004a5cb0
void EntityDroideka::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    bool bFirstPerson = IsAlive() &&
        ((mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]] &&
          mWeapon[mWeaponIndex[0]]->ZoomFirstPerson()) ||
         (mTracker && mTracker->IsFirstPersonView()));
    if (bFirstPerson)
    {
        rMatrix = DirectionMatrix((-mEyeDir),
                                  mMatrix.GetYAxis(), mEyePoint);
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateCamera(iFirstPerson, rMatrix);
        }
        return;
    }
    PblVector3 vTarget = mEyePoint + mEyeDir * 65536.0f;
    GetCameraTrackSettings()->SetupCameraTrackMatrix(this, rMatrix, fDeltaTime,
        mMatrix, vTarget, 0.9f, 2.0f, this);
}

// FUNCTION: SWBF2 0x004a5e80
void EntityDroideka::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix)
{
    if (IsAlive() &&
        ((mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]] &&
          mWeapon[mWeaponIndex[0]]->ZoomFirstPerson()) ||
         (mTracker && mTracker->IsFirstPersonView())))
    {
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateAimer(iFirstPerson, rMatrix);
        }
    }
}

// FUNCTION: SWBF2 0x004a6460
void EntityDroideka::SetFrameState(float fState)
{
    mZephyrPoseDyn.SetFrameNumFloat(fState);
    mZephyrPoseDyn.Update(0.0f);
}

// FUNCTION: SWBF2 0x004a6490
void EntityDroideka::ResetTurnState()
{
    mZephyrPoseDyn.SetFrameNumFloat(mTurnState);
    mZephyrPoseDyn.Update(0.0f);
}

// FUNCTION: SWBF2 0x004a64c0
float EntityDroideka::GetStateLimit()
{
    return (float)mZephyrPoseDyn.GetNumFrames();
}

// FUNCTION: SWBF2 0x00524070
bool EntityDroideka::IsRollingOrTransition() const
{
    return mState == Rolling || mState == IdleToRoll || mState == RollToIdle;
}

// FUNCTION: SWBF2 0x004a6430
float EntityDroideka::GetTurnState()
{
    mTurnState = (float)(mZephyrPoseDyn.m_kAnim.m_pkAnim->GetNumFrames() - 1) *
                 mZephyrPoseDyn.m_fCurT;
    return mTurnState;
}

// FUNCTION: SWBF2 0x004a64e0
void EntityDroideka::BuffOffense(float fTimer, float fMult)
{
    if (fTimer > mBuffOffenseTimer)
    {
        mBuffOffenseTimer = fTimer;
        mBuffOffenseMult = fMult;
    }
}

// FUNCTION: SWBF2 0x004a6500
void EntityDroideka::BuffDefense(float fTimer, float fMult)
{
    if (fTimer > mBuffDefenseTimer)
    {
        mBuffDefenseTimer = fTimer;
        mBuffDefenseMult = fMult;
    }
}

// FUNCTION: SWBF2 0x004a6520
void EntityDroideka::BuffHealth(float fTimer, float fRate)
{
    if (fTimer > mBuffHealthTimer)
    {
        mBuffHealthTimer = fTimer;
        mBuffHealthRate = fRate;
    }
}

// FUNCTION: SWBF2 0x004a6540
void EntityDroideka::DebuffDamage(float fTimer, float fRate, const DamageOwner& owner)
{
    if (fTimer > mDebuffDamageTimer)
    {
        mDebuffDamageTimer = fTimer;
        mDebuffDamageRate = fRate;
        mDebuffDamageOwner = owner;
    }
}

// GLOBAL: SWBF2 0x01e51d98
static int sWeaponSection;
// GLOBAL: SWBF2 0x01e51d9c
static int sCurrentCameraStance;
// GLOBAL: SWBF2 0x01e51da0
static int sCurrentCameraSplit;

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x004a70e0
EntityDroidekaClass::EntityDroidekaClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    m_bDynamicLighting = true;
    mHealthType = HEALTH_TYPE_DROID;
    mPilotType = PILOT_SELF;
    mCanCapturePosts = true;
    mLODClass = 2;
    mTurnOffTime = 0.1f;

    mUprightLowRes = 0;
    mBallLowRes = 0;
    mAnimObj = 0;
    mAnimations = 0;
    mAnimationCount = 0;
    mTimeTotals = 0;
    mAnimSpeeds = 0;
    mAnimRates = 0;
    mAcceleration = 5.0f;
    mMaxSpeed = 5.0f;
    mMaxTurnSpeed = 2.0f;
    mMaxYawSpeed = 1.0f;
    mMaxPitchSpeed = 0.5f;
    mUnrollTime = 1.67f;
    mRollTime = 0.5f;
    mMouseMaxTurnSpeed = 20.0f;
    mMouseMaxYawSpeed = 20.0f;
    mMouseMaxPitchSpeed = 20.0f;
    mMouseTurnDecay = 5.0f;
    mMinYaw = 0.0f;
    mMaxYaw = 0.0f;
    mMinPitch = -0.5f;
    mMaxPitch = 0.5f;
    mCollisionThreshold[0] = 7.5f;
    mCollisionThreshold[1] = 2.5f;
    mCollisionThreshold[2] = 0.0f;
    mCollisionScale[0] = 1.0f;
    mCollisionScale[1] = 0.0f;
    mCollisionScale[2] = 0.0f;
    mCollisionInflict = 1.0f;

    sWeaponSection = 0;

    for (int i = 0; i < 4; ++i)
    {
        mWeaponClass[i] = 0;
    }
    for (int i = 0; i < 4; ++i)
    {
        mWeaponAmmo[i].uiAmmo = 0;
        mWeaponAmmo[i].uiHeat = 0;
        mWeaponAmmo[i].bFlagAmmoInfinite = 0;
        mWeaponAmmo[i].bFlagShareHeat = 0;
        mWeaponAmmo[i].bFlagShareAmmo = 0;
    }
    for (int i = 0; i < 4; ++i)
    {
        mWeaponAmmo[i].bFlagAmmoInfinite = 1;
    }
    for (int i = 0; i < 4; ++i)
    {
        mWeaponChannel[i] = i;
    }

    mWeaponCount = 0;
    mStompEffect = 0;
    mVehicleType = 9;
    mNoiseFactor = 5.0f;
    mAnimRateMove = 1.0f;
    mAnimRateTurn = 1.0f;
    mTurnThreshold = 0.52359879f;
    mMaxTerrainAngle = 0.9f;

    mCameraSettings[0].mCameraEyePointOffset.x = 0.0f;
    mCameraSettings[0].mCameraEyePointOffset.y = 1.8f;
    mCameraSettings[0].mCameraEyePointOffset.z = 0.0f;
    mCameraSettings[0].mCameraTrackCenter.x = 0.0f;
    mCameraSettings[0].mCameraTrackCenter.y = 2.3f;
    mCameraSettings[0].mCameraTrackCenter.z = 0.0f;
    mCameraSettings[0].mCameraTrackOffset.x = 0.0f;
    mCameraSettings[0].mCameraTrackOffset.y = 0.0f;
    mCameraSettings[0].mCameraTrackOffset.z = 4.5f;
    mCameraSettings[0].mCameraMoveTensionL = 15.0f;
    mCameraSettings[0].mCameraMoveTensionR = 15.0f;
    mCameraSettings[0].mCameraMoveTensionU = 15.0f;
    mCameraSettings[0].mCameraMoveTensionD = 15.0f;
    mCameraSettings[0].mCameraMoveTensionF = 15.0f;
    mCameraSettings[0].mCameraMoveTensionB = 15.0f;
    mCameraSettings[0].mCameraAimTension = FLT_MAX;

    mCameraSettings[1].mCameraTrackCenter = mCameraSettings[1].mCameraEyePointOffset;
    mCameraSettings[1].mCameraTrackOffset = gVectorZero;
    mCameraSettings[1].mCameraMoveTensionL = FLT_MAX;
    mCameraSettings[1].mCameraMoveTensionR = FLT_MAX;
    mCameraSettings[1].mCameraMoveTensionU = FLT_MAX;
    mCameraSettings[1].mCameraMoveTensionD = FLT_MAX;
    mCameraSettings[1].mCameraMoveTensionF = FLT_MAX;
    mCameraSettings[1].mCameraMoveTensionB = FLT_MAX;
    mCameraSettings[1].mCameraAimTension = FLT_MAX;

    mCameraSettings[2].mCameraEyePointOffset.x = 0.0f;
    mCameraSettings[2].mCameraEyePointOffset.y = 1.2f;
    mCameraSettings[2].mCameraEyePointOffset.z = 0.0f;
    mCameraSettings[2].mCameraTrackCenter.x = 0.0f;
    mCameraSettings[2].mCameraTrackCenter.y = 1.3f;
    mCameraSettings[2].mCameraTrackCenter.z = 0.0f;
    mCameraSettings[2].mCameraTrackOffset.x = 0.0f;
    mCameraSettings[2].mCameraTrackOffset.y = 0.0f;
    mCameraSettings[2].mCameraTrackOffset.z = 3.5f;
    mCameraSettings[2].mCameraMoveTensionL = 15.0f;
    mCameraSettings[2].mCameraMoveTensionR = 15.0f;
    mCameraSettings[2].mCameraMoveTensionU = 15.0f;
    mCameraSettings[2].mCameraMoveTensionD = 15.0f;
    mCameraSettings[2].mCameraMoveTensionF = 15.0f;
    mCameraSettings[2].mCameraMoveTensionB = 15.0f;
    mCameraSettings[2].mCameraAimTension = FLT_MAX;

    mExplosion = 0;
    mNoDeathExplosions = 0;
    mDeathDustClassHash = 0;
    mDeathDustEffect = 0;
    mDeathDustDelay = 0.0f;
    mDeathDustOffset.x = 0.0f;
    mDeathDustOffset.y = 0.0f;
    mDeathDustOffset.z = 0.0f;
    mDeathShakeDelay = -1.0f;
    mDeathShakeForce = 1.0f;
    mDeathShakeDuration = 1.0f;
    mDeathShakeRadius = 1.0f;
    mRenderPilotFlag = false;
    mNumFeet = 0;

    for (int i = 0; i < 3; ++i)
    {
        mFootPrimitives[i] = -1;
    }

    mHydraulicSoundHeight = 0.0f;
    mAttachHash = 0;
    mAttachOffset.x = 0.0f;
    mAttachOffset.y = 0.0f;
    mAttachOffset.z = 0.0f;
    mWaterDamageInterval = 1.0f;
    mWaterDamageAmount = 0.0f;
    mBallRadius = 0.5f;
    mBallMoveSpeed = 5.0f;
    mBallStoppedTurnSpeed = 2.5f;
    mBallTurnSpeed = 2.0f;
    mBallSprintTurnSpeed = 1.0f;
    mBallSlippage = 1.0f;
    mBallMaxLean = 0.2f;
    mBallAcceleration = 100.0f;
    mBallRollingFriction = 2.0f;
    mMaxBallAngle = 0.7f;
    mSprintTimeForBowling = 0.5f;
    mBowlingPush = 8.0f;
    mBallSprintSpeedFactor = 2.0f;
    mBallJumpVerticalSpeed = 5.0f;
    mBallJumpForwardSpeedFactor = 1.25f;

    mEnergyBarClass.Init(100.0f, 20.0f);

    mEnergyRestore = 10.0f;
    mEnergyRestoreIdle = 15.0f;
    mEnergyDrainSprint = 30.0f;
    mEnergyCostJump = 30.0f;
    mEnergyCostBowling = 30.0f;

    for (int i = 0; i < 3; ++i)
    {
        mJointIdx[i] = -1;
    }

    for (int i = 0; i < 2; ++i)
    {
        mArmJointIdx[i] = -1;
    }

    mWakeEffect = 0;
    mFirstPersonModelHandle = -1;
    mUprightWaterDamageHeight = 1.5f;
    mBallWaterDamageHeight = 1.5f;
    mHealthGainEffect = 0x9D26C2F2u;
    mAmmoGainEffect = 0x2A72AE31u;
    mEnergyGainEffect = 0x9796A1C5u;
    mBuffHealthEffect = 0xBB3E84E9u;
    mBuffOffenseEffect = 0xC17AB11Au;
    mBuffDefenseEffect = 0x28B9F3ACu;
    mDebuffEffect = 0x9796A1C5u;

    mLockOnMinDist = 0.0f;
    mLockOnMaxDist = 15.0f;
    mLockOnMaxTrackDist = 500.0f;
}


namespace GameLoop
{
extern float sClientDeltaTime;
}

namespace
{
    inline EnergyBar* DroidekaEnergyBar(EntityDroideka* pDroideka)
    {
        return &pDroideka->mEnergyBar;
    }

    inline void DroidekaUpdateCollisionSphere(EntityDroideka* pDroideka)
    {
        PblVector3 center;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&center), reinterpret_cast<const D3DXVECTOR3*>(&pDroideka->mModel->_mModel->_BoundingSphere.GetCenter()),
                               reinterpret_cast<const D3DXMATRIX*>(&pDroideka->mMatrix));
        pDroideka->SetCollisionCenter(center);
        pDroideka->RecalculateAABB();
    }
}

// SYNTHETIC: SWBF2 0x00402a80
// `dynamic initializer for 'EntityDroideka::rttiHashEntityDroideka''
// EntityDroideka::rttiHashEntityDroideka

// GLOBAL: SWBF2 0x01ebbc58
// EntityDroideka::rttiHashEntityDroideka
PblHash EntityDroideka::rttiHashEntityDroideka("EntityDroideka");
// GLOBAL: SWBF2 0x0078917c
// EntityDroideka::rttiNameEntityDroideka
const char EntityDroideka::rttiNameEntityDroideka[] = "EntityDroideka";

// SYNTHETIC: SWBF2 0x00402a60
// `dynamic initializer for 'EntityDroideka::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f40
// `dynamic atexit destructor for 'EntityDroideka::sMemoryPool''

// GLOBAL: SWBF2 0x01ebbc08
// EntityDroideka::sMemoryPool
MemoryPool EntityDroideka::sMemoryPool("EntityDroideka", 0x1b70);







// FUNCTION: SWBF2 0x0049e3e0
void EntityDroideka::MoveForward(float fDistance)
{
    PblVector3 vPos = mMatrix.GetTranslation();
    if (mTurnOffset.mCos < 1.0f)
    {
        float fCos = mTurnOffset.mCos * mTurnOffset.mCos - mTurnOffset.mSin * mTurnOffset.mSin;
        float fSin = mTurnOffset.mCos * 2.0f * mTurnOffset.mSin;
        if (mState != Rolling)
        {
            float f4Cos2 = fCos * fCos * 4.0f;
            fCos = fCos * (f4Cos2 - 3.0f);
            fSin = (f4Cos2 - 1.0f) * fSin;
        }

        PblMatrix kTurn(PblVector3(fCos, 0.0f, -fSin), PblVector3(0.0f, 1.0f, 0.0f),
                        PblVector3(fSin, 0.0f, fCos), PblVector3(0.0f, 0.0f, 0.0f));
        kTurn *= mMatrix;

        PblVector3 vMove(kTurn.m[2][0] * fDistance, kTurn.m[2][1] * fDistance,
                         kTurn.m[2][2] * fDistance);
        vPos.x += vMove.x;
        vPos.y += vMove.y;
        vPos.z += vMove.z;
    }
    else
    {
        PblVector3 vMove(mMatrix.m[2][0] * fDistance, mMatrix.m[2][1] * fDistance,
                         mMatrix.m[2][2] * fDistance);
        vPos.x += vMove.x;
        vPos.y += vMove.y;
        vPos.z += vMove.z;
    }
    SetPosition(vPos);
}

// FUNCTION: SWBF2 0x0049e5d0
void EntityDroideka::RotateY(const PblAngle& rAngle)
{
    PblMatrix kRot;
    kRot.m[0][0] = rAngle.mCos;
    kRot.m[0][1] = 0.0f;
    kRot.m[0][2] = -rAngle.mSin;
    kRot.m[0][3] = 0.0f;
    kRot.m[1][0] = 0.0f;
    kRot.m[1][1] = 1.0f;
    kRot.m[1][2] = 0.0f;
    kRot.m[1][3] = 0.0f;
    kRot.m[2][0] = rAngle.mSin;
    kRot.m[2][1] = 0.0f;
    kRot.m[2][2] = rAngle.mCos;
    kRot.m[2][3] = 0.0f;
    kRot.m[3][0] = 0.0f;
    kRot.m[3][1] = 0.0f;
    kRot.m[3][2] = 0.0f;
    kRot.m[3][3] = 1.0f;

    PblMatrix kOut;
    kOut = kRot * mMatrix;
    mMatrix = kOut;
}

// FUNCTION: SWBF2 0x0049e690
const RedPose* EntityDroideka::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x0049e6a0
GameObject* EntityDroideka::GetGameObject()
{
    return this;
}

// FUNCTION: SWBF2 0x0049e6b0
const GameObject* EntityDroideka::GetGameObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x0049e6c0
Entity* EntityDroideka::GetEntity()
{
    return this;
}

// FUNCTION: SWBF2 0x0049e6d0
const Entity* EntityDroideka::GetEntity() const
{
    return this;
}

// FUNCTION: SWBF2 0x0049e6e0
const PblMatrix& EntityDroideka::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x0049e6f0
const PblVector3& EntityDroideka::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x0049e700
void EntityDroideka::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x0049e720
float EntityDroideka::GetMaxStrafe() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x0049e730
Trackable* EntityDroideka::GetTrackable()
{
    return this;
}

// FUNCTION: SWBF2 0x0049e750
const Controllable* EntityDroideka::GetControllable() const
{
    return this;
}

// FUNCTION: SWBF2 0x0049e760
Controllable* EntityDroideka::GetControllable()
{
    return this;
}

// FUNCTION: SWBF2 0x0049e770
const PlayableEntitySounds* EntityDroideka::GetPlayableEntitySounds() const
{
    return this;
}

// FUNCTION: SWBF2 0x0049e780
PlayableEntitySounds* EntityDroideka::GetPlayableEntitySounds()
{
    return this;
}

// FUNCTION: SWBF2 0x0049e790
int EntityDroideka::GetWeaponIndex(int iChannel) const
{
    return mWeaponIndex[iChannel];
}

// FUNCTION: SWBF2 0x004a0d60
float EntityDroideka::GetAimTurnRate() const
{
    return mClass->mMaxPitchSpeed;
}

// FUNCTION: SWBF2 0x004a0d70
void EntityDroideka::SetWeaponIndex(int iChannel, int iIndex)
{
    mWeaponIndex[iIndex] = iChannel;
}

// FUNCTION: SWBF2 0x004a0de0
PilotDeathType EntityDroideka::GetPilotDeathType()
{
    return mClass->mPilotDeathType;
}

// FUNCTION: SWBF2 0x004a2350
bool EntityDroideka::IsForcedThirdPerson() const
{
    return mState == Rolling || mState == IdleToRoll || mState == RollToIdle;
}

// FUNCTION: SWBF2 0x004a6040
const PblVector3& EntityDroideka::GetAttachOffset() const
{
    return mClass->mAttachOffset;
}

// FUNCTION: SWBF2 0x0049e7b0
Weapon* EntityDroideka::GetWeaponPtr(int iIndex) const
{
    return mWeapon[iIndex];
}

// FUNCTION: SWBF2 0x0049e7d0
const Aimer* EntityDroideka::GetAimer(int iIndex) const
{
    if (mState == IdleToRoll || mState == Rolling || mState == RollToIdle)
    {
        return 0;
    }
    return mAimer[iIndex];
}

// FUNCTION: SWBF2 0x0049e800
GameObjectType EntityDroideka::GetGameObjectType()
{
    return (GameObjectType)(GO_VEHICLE | GO_WALKER);
}

// FUNCTION: SWBF2 0x0049e810
EntityType EntityDroideka::GetEntityType() const
{
    return ENT_DROIDEKA;
}

// FUNCTION: SWBF2 0x0049e820
void EntityDroideka::SetPosition(const PblVector3& vPos)
{
    mMatrix.SetTranslation(vPos);
    DroidekaUpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x0049e8c0
void EntityDroideka::SetMatrix(const PblMatrix& mMatrixIn)
{
    mMatrix = mMatrixIn;
    DroidekaUpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x0049e970
bool EntityDroideka::IsFirstPersonAvailablePS2() const
{
    return true;
}

// FUNCTION: SWBF2 0x0049e990
EnergyBar* EntityDroideka::GetEnergyBar()
{
    return &mEnergyBar;
}

// FUNCTION: SWBF2 0x0049e9a0
bool EntityDroideka::PostCollision::Update(float fDeltaTime)
{
    return mOwner->PostCollisionUpdate(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x0049e9c0
// EntityDroideka::PostCollision::`scalar deleting destructor'



// SYNTHETIC: SWBF2 0x00402a90
// `dynamic initializer for 'EntityDroidekaClass::rttiHashEntityDroidekaClass''
// EntityDroidekaClass::rttiHashEntityDroidekaClass

// GLOBAL: SWBF2 0x01ebbc5c
// EntityDroidekaClass::rttiHashEntityDroidekaClass
PblHash EntityDroidekaClass::rttiHashEntityDroidekaClass("EntityDroidekaClass");
// GLOBAL: SWBF2 0x007891a8
// EntityDroidekaClass::rttiNameEntityDroidekaClass
const char EntityDroidekaClass::rttiNameEntityDroidekaClass[] = "EntityDroidekaClass";







// FUNCTION: SWBF2 0x0049ea60
int EntityDroidekaClass::GetVehicleType() const
{
    return mVehicleType;
}

// FUNCTION: SWBF2 0x0049ea70
int EntityDroidekaClass::GetWeaponCount() const
{
    if (!mWeaponClass[0])
    {
        return 0;
    }
    return mWeaponClass[1] ? 2 : 1;
}

// FUNCTION: SWBF2 0x0049ea90
const WeaponClass* EntityDroidekaClass::GetWeaponClass(int iIndex) const
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x0049eab0
const ControllableClass* EntityDroidekaClass::GetControllableClass() const
{
    return static_cast<const ControllableClass*>(this);
}

// FUNCTION: SWBF2 0x0049f470
float EntityDroideka::UpdateMouseDecay(float fDeltaTime, float fInput)
{
    PlayerController* pController = static_cast<PlayerController*>(mCtrl);
    float fBuildup = pController->GetMouseBuildup();
    if (fInput == 0.0f)
    {
        float fDecay = fDeltaTime * 15.0f;
        float fResult = 0.0f;
        if (fBuildup < 0.0f)
        {
            if (!(fBuildup > -fDecay))
            {
                fResult = fDecay + fBuildup;
            }
        }
        else if (!(fDecay > fBuildup))
        {
            fResult = fBuildup - fDecay;
        }
        pController->mMouseTurnBuildup = fResult;
        return fResult;
    }
    return fBuildup;
}

// FUNCTION: SWBF2 0x0049f4e0
void EntityDroideka::UpdateWalkingGroundNormal()
{
    float f4Cos2 = mTurnOffset.mCos * mTurnOffset.mCos * 4.0f;
    float fCos3 = (f4Cos2 - 3.0f) * mTurnOffset.mCos;
    float fSin3 = (f4Cos2 - 1.0f) * mTurnOffset.mSin;
    float fCos = fCos3 * fCos3 - fSin3 * fSin3;
    float fSin = fCos3 * 2.0f * fSin3;

    PblVector3 vFoot0;
    PblVector3 vFoot1;
    PblVector3 vFoot2;
    vFoot0.x = fCos * 0.7071f + fSin * 0.5f;
    vFoot0.y = 1.0f;
    vFoot0.z = fCos * 0.5f - fSin * 0.7071f;
    vFoot1.x = fSin * -0.9142f;
    vFoot1.y = 1.0f;
    vFoot1.z = fCos * -0.9142f;
    vFoot2.x = fSin * 0.5f - fCos * 0.7071f;
    vFoot2.y = 1.0f;
    vFoot2.z = fCos * 0.5f + fSin * 0.7071f;
    PblVector3 vWorld;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vWorld), reinterpret_cast<const D3DXVECTOR3*>(&vFoot0), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
    vFoot0 = vWorld;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vWorld), reinterpret_cast<const D3DXVECTOR3*>(&vFoot1), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
    vFoot1 = vWorld;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vWorld), reinterpret_cast<const D3DXVECTOR3*>(&vFoot2), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
    vFoot2 = vWorld;

    const GameObject* pIgnore = GetGameObject();
    CollisionObject* pHit;
    {
        PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
        float fHit = CollisionManager::RayHit(vFoot0, vDown, 2.0f, pHit, 0, &pIgnore, 1, 0x98, false);
        vFoot0.y = 1.0f - fHit * 2.0f;
    }
    {
        PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
        float fHit = CollisionManager::RayHit(vFoot1, vDown, 2.0f, pHit, 0, &pIgnore, 1, 0x98, false);
        vFoot1.y = 1.0f - fHit * 2.0f;
    }
    {
        PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
        float fHit = CollisionManager::RayHit(vFoot2, vDown, 2.0f, pHit, 0, &pIgnore, 1, 0x98, false);
        vFoot2.y = 1.0f - fHit * 2.0f;
    }

    PblVector3 vA(vFoot2.x - vFoot1.x, vFoot2.y - vFoot1.y, vFoot2.z - vFoot1.z);
    PblVector3 vB(vFoot0.x - vFoot1.x, vFoot0.y - vFoot1.y, vFoot0.z - vFoot1.z);
    PblVector3 vPlane(vA.y * vB.z - vA.z * vB.y, vA.z * vB.x - vA.x * vB.z, vA.x * vB.y - vA.y * vB.x);
    PblVector3 vNormal;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNormal), reinterpret_cast<const D3DXVECTOR3*>(&vPlane));

    float fRate = GameLoop::sClientDeltaTime * 5.0f;
    PblVector3 vCross(vNormal.y * mGroundNormal.z - vNormal.z * mGroundNormal.y,
                      vNormal.z * mGroundNormal.x - vNormal.x * mGroundNormal.z,
                      vNormal.x * mGroundNormal.y - vNormal.y * mGroundNormal.x);
    float fCross2 = vCross.x * vCross.x + vCross.y * vCross.y + vCross.z * vCross.z;
    if (fRate * fRate > fCross2)
    {
        mGroundNormal = vNormal;
    }
    else
    {
        float fT = sqrtf(fCross2) * fRate;
        float fKeep = 1.0f - fT;
        PblVector3 vBlend(mGroundNormal.x * fKeep + vNormal.x * fT,
                          mGroundNormal.y * fKeep + vNormal.y * fT,
                          mGroundNormal.z * fKeep + vNormal.z * fT);
        mGroundNormal = Normalize(vBlend);
    }
}

// FUNCTION: SWBF2 0x0049f9f0
bool EntityDroideka::Jump()
{
    if (mState != Rolling || mFallTimer >= 0.1f ||
        !DroidekaEnergyBar(this)->SpendEnergy(mClass->mEnergyCostJump))
    {
        return false;
    }

    mFallTimer = 0.1f;

    const PblMatrix& rMatrix = GetMatrix();
    const PblVector3& rVelocity = GetVelocity();
    mClass->mSoundJump.Play((const PblVector3*)&rMatrix.m[3][0], &rVelocity, true, true);

    mVelocity.y += mClass->mBallJumpVerticalSpeed;
    float fForward = mVelocity.y * mMatrix.m[2][1] + mVelocity.x * mMatrix.m[2][0] +
                     mVelocity.z * mMatrix.m[2][2];
    if (fForward > 0.0f)
    {
        float fBoost = (mClass->mBallJumpForwardSpeedFactor - 1.0f) * fForward;
        mVelocity.x += mMatrix.m[2][0] * fBoost;
        mVelocity.y += mMatrix.m[2][1] * fBoost;
        mVelocity.z += mMatrix.m[2][2] * fBoost;
    }

    if (mClass->mStompEffect)
    {
        FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(mClass->mStompEffect);
        if (pEffectClass)
        {
            FLEffectObject* pEffect = pEffectClass->BuildEffect();
            if (pEffect)
            {
                pEffect->AttachEffectToMatrix(&mMatrix, false);
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0049fb90
bool EntityDroideka::Sprint()
{
    if (mState == Rolling && !DroidekaEnergyBar(this)->IsOverheating() &&
        (!mCharacter || !mCharacter->mCarriedFlagPtr))
    {
        m_bSprint = true;
        return true;
    }
    m_bSprint = false;
    return false;
}

// FUNCTION: SWBF2 0x0049fbd0
bool EntityDroideka::EndSprint()
{
    if (!m_bSprint)
    {
        return false;
    }
    m_bSprint = false;
    return true;
}

// FUNCTION: SWBF2 0x0049fbf0
bool EntityDroideka::RenderTrackable(unsigned int uView) const
{
    if (!IsAlive())
    {
        return true;
    }

    if (mTargetInfo.mIsAiming)
    {
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0)
        {
            Weapon* pWeapon = mWeapon[iIndex];
            if (pWeapon && pWeapon->ZoomFirstPerson() &&
                NetGame::GetJoystickIndex(uView) == (int)FLRenderer::GetCurrentViewport())
            {
                return false;
            }
        }
    }
    return Trackable::RenderTrackable(uView);
}

// FUNCTION: SWBF2 0x0049fc60
int EntityDroideka::GetFirstPersonIndex()
{
    if (mPlayerId < 0)
    {
        return -1;
    }
    if (!mTracker || !mTracker->IsFirstPersonView())
    {
        return -1;
    }
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return NetGame::GetJoystickIndex(mPlayerId);
    }
    return -1;
}



// FUNCTION: SWBF2 0x004a7940
EntityDroidekaClass::EntityDroidekaClass(EntityDroidekaClass* pSrc, unsigned int uHash)
    : GameObjectClass(pSrc, uHash),
      ControllableClass(pSrc),
      VehicleEngineClass(pSrc)
{

    sWeaponSection = 0;
    mLODClass = pSrc->mLODClass;

    mLodData = pSrc->mLodData;
    mUprightLowRes = pSrc->mUprightLowRes;
    mBallLowRes = pSrc->mBallLowRes;
    mAnimObj = pSrc->mAnimObj;
    mAnimations = pSrc->mAnimations;
    mAnimationCount = pSrc->mAnimationCount;
    mTimeTotals = pSrc->mTimeTotals;
    mAnimSpeeds = pSrc->mAnimSpeeds;
    mAnimRates = pSrc->mAnimRates;
    mAcceleration = pSrc->mAcceleration;
    mMaxSpeed = pSrc->mMaxSpeed;
    mMaxTurnSpeed = pSrc->mMaxTurnSpeed;
    mMaxYawSpeed = pSrc->mMaxYawSpeed;
    mMaxPitchSpeed = pSrc->mMaxPitchSpeed;
    mUnrollTime = pSrc->mUnrollTime;
    mRollTime = pSrc->mRollTime;
    mMouseMaxTurnSpeed = pSrc->mMouseMaxTurnSpeed;
    mMouseMaxYawSpeed = pSrc->mMouseMaxYawSpeed;
    mMouseMaxPitchSpeed = pSrc->mMouseMaxPitchSpeed;
    mMouseTurnDecay = pSrc->mMouseTurnDecay;
    mMinYaw = pSrc->mMinYaw;
    mMaxYaw = pSrc->mMaxYaw;
    mMinPitch = pSrc->mMinPitch;
    mMaxPitch = pSrc->mMaxPitch;
    memcpy(mCollisionThreshold, pSrc->mCollisionThreshold, sizeof(mCollisionThreshold));
    memcpy(mCollisionScale, pSrc->mCollisionScale, sizeof(mCollisionScale));
    mCollisionInflict = pSrc->mCollisionInflict;

    for (int i = 0; i < 4; ++i)
    {
        mAimerClass[i] = pSrc->mAimerClass[i];
    }
    memcpy(mWeaponClass, pSrc->mWeaponClass, sizeof(mWeaponClass));
    memcpy(mWeaponAmmo, pSrc->mWeaponAmmo, sizeof(mWeaponAmmo));
    memcpy(mWeaponChannel, pSrc->mWeaponChannel, sizeof(mWeaponChannel));
    mWeaponCount = pSrc->mWeaponCount;
    mStompEffect = pSrc->mStompEffect;
    mVehicleType = pSrc->mVehicleType;
    mNoiseFactor = pSrc->mNoiseFactor;
    mAnimRateMove = pSrc->mAnimRateMove;
    mAnimRateTurn = pSrc->mAnimRateTurn;
    mTurnThreshold = pSrc->mTurnThreshold;
    mMaxTerrainAngle = pSrc->mMaxTerrainAngle;

    memcpy(mCameraSettings, pSrc->mCameraSettings, sizeof(mCameraSettings));
    mExplosion = pSrc->mExplosion;
    mNoDeathExplosions = pSrc->mNoDeathExplosions;
    mDeathDustClassHash = pSrc->mDeathDustClassHash;
    mDeathDustEffect = pSrc->mDeathDustEffect;
    mDeathDustDelay = pSrc->mDeathDustDelay;
    mDeathDustOffset = pSrc->mDeathDustOffset;
    mDeathShakeDelay = pSrc->mDeathShakeDelay;
    mDeathShakeForce = pSrc->mDeathShakeForce;
    mDeathShakeDuration = pSrc->mDeathShakeDuration;
    mDeathShakeRadius = pSrc->mDeathShakeRadius;
    mRenderPilotFlag = pSrc->mRenderPilotFlag;
    mNumFeet = pSrc->mNumFeet;
    for (int i = 0; i < 3; ++i)
    {
        mFootPrimitives[i] = pSrc->mFootPrimitives[i];
        mFootstepSounds[i] = pSrc->mFootstepSounds[i];
    }
    mHydraulicSound = pSrc->mHydraulicSound;
    mHydraulicSoundHeight = pSrc->mHydraulicSoundHeight;
    mAttachHash = pSrc->mAttachHash;
    mAttachOffset = pSrc->mAttachOffset;
    mWaterDamageInterval = pSrc->mWaterDamageInterval;
    mWaterDamageAmount = pSrc->mWaterDamageAmount;
    mBallRadius = pSrc->mBallRadius;
    mBallMoveSpeed = pSrc->mBallMoveSpeed;
    mBallStoppedTurnSpeed = pSrc->mBallStoppedTurnSpeed;
    mBallTurnSpeed = pSrc->mBallTurnSpeed;
    mBallSprintTurnSpeed = pSrc->mBallSprintTurnSpeed;
    mBallSlippage = pSrc->mBallSlippage;
    mBallMaxLean = pSrc->mBallMaxLean;
    mBallAcceleration = pSrc->mBallAcceleration;
    mBallRollingFriction = pSrc->mBallRollingFriction;
    mMaxBallAngle = pSrc->mMaxBallAngle;
    mSprintTimeForBowling = pSrc->mSprintTimeForBowling;
    mBowlingPush = pSrc->mBowlingPush;
    mBallSprintSpeedFactor = pSrc->mBallSprintSpeedFactor;
    mBallJumpVerticalSpeed = pSrc->mBallJumpVerticalSpeed;
    mBallJumpForwardSpeedFactor = pSrc->mBallJumpForwardSpeedFactor;
    mEnergyBarClass.Init(pSrc->mEnergyBarClass.m_fEnergyMax, pSrc->mEnergyBarClass.m_fEnergyOverheat);
    mEnergyRestore = pSrc->mEnergyRestore;
    mEnergyRestoreIdle = pSrc->mEnergyRestoreIdle;
    mEnergyDrainSprint = pSrc->mEnergyDrainSprint;
    mEnergyCostJump = pSrc->mEnergyCostJump;
    mEnergyCostBowling = pSrc->mEnergyCostBowling;
    mCollisionBall = pSrc->mCollisionBall;
    mWakeEffect = pSrc->mWakeEffect;
    mSoundShield = pSrc->mSoundShield;
    mSoundShieldWearOff = pSrc->mSoundShieldWearOff;
    mSoundShieldOff = pSrc->mSoundShieldOff;
    mSoundJump = pSrc->mSoundJump;
    mSoundLand = pSrc->mSoundLand;
    mFirstPersonModelHandle = pSrc->mFirstPersonModelHandle;
    mUprightWaterDamageHeight = pSrc->mUprightWaterDamageHeight;
    mBallWaterDamageHeight = pSrc->mBallWaterDamageHeight;
    mHealthGainEffect = pSrc->mHealthGainEffect;
    mAmmoGainEffect = pSrc->mAmmoGainEffect;
    mEnergyGainEffect = pSrc->mEnergyGainEffect;
    mBuffHealthEffect = pSrc->mBuffHealthEffect;
    mBuffOffenseEffect = pSrc->mBuffOffenseEffect;
    mBuffDefenseEffect = pSrc->mBuffDefenseEffect;
    mDebuffEffect = pSrc->mDebuffEffect;
}

// FUNCTION: SWBF2 0x004a8080
EntityDroidekaClass::~EntityDroidekaClass()
{
    delete[] mAnimations;
    delete[] mTimeTotals;
    delete[] mAnimSpeeds;
    delete[] mAnimRates;
}

// SYNTHETIC: SWBF2 0x004a78a0
// EntityDroidekaClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004a8180
EntityClass* EntityDroidekaClass::Derive(unsigned int uHash)
{
    return new EntityDroidekaClass(this, uHash);
}

// FUNCTION: SWBF2 0x004a82a0
void EntityDroidekaClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x2FA0BA9D:
    case 0x9F8A0040:
        sscanf(pszValue, "%f", &mAcceleration);
        break;
    case 0x86DF0364:
        sscanf(pszValue, "%f", &mMaxSpeed);
        break;
    case 0x5433F589:
        sscanf(pszValue, "%f", &mMaxTurnSpeed);
        break;
    case 0x7335AA87:
        sscanf(pszValue, "%f", &mMaxYawSpeed);
        if (mMaxTurnSpeed > mMaxYawSpeed)
        {
            mMaxYawSpeed = mMaxTurnSpeed;
        }
        break;
    case 0xB5534BE0:
        sscanf(pszValue, "%f", &mMaxPitchSpeed);
        break;
    case 0x3209ADD4:
        sscanf(pszValue, "%f", &mUnrollTime);
        break;
    case 0xC817A0F7:
        sscanf(pszValue, "%f", &mRollTime);
        break;
    case 0x48616A7B:
        sscanf(pszValue, "%f", &mMouseTurnDecay);
        break;
    case 0x2C3E8078:
        sscanf(pszValue, "%f %f", &mMinYaw, &mMaxYaw);
        mMinYaw *= 0.017453292f;
        mMaxYaw *= 0.017453292f;
        break;
    case 0x3403B139:
        sscanf(pszValue, "%f %f", &mMinPitch, &mMaxPitch);
        mMinPitch *= 0.017453292f;
        mMaxPitch *= 0.017453292f;
        break;
    case 0x2CDB588A:
        sscanf(pszValue, "%f %f %f", &mCollisionThreshold[0], &mCollisionThreshold[1],
               &mCollisionThreshold[2]);
        break;
    case 0x1A87BACF:
        sscanf(pszValue, "%f %f %f", &mCollisionScale[0], &mCollisionScale[1], &mCollisionScale[2]);
        break;
    case 0x2EF03732:
        sscanf(pszValue, "%f", &mCollisionInflict);
        break;
    case 0x28ADDA49:
        mCombatInterrupts = atoi(pszValue) == 0;
        break;

    case 0xD0329E80:
        sWeaponSection = atoi(pszValue) - 1;
        break;
    case 0x6F332041:
    case 0xFBF47DBA:
    {
        mWeaponClass[sWeaponSection] = WeaponClass::Find(PblHash(pszValue));
        int iCount = sWeaponSection + 1;
        if (mWeaponCount > iCount)
        {
            iCount = mWeaponCount;
        }
        mWeaponCount = iCount;
        break;
    }
    case 0x16472328:
        sscanf(pszValue, "%d", &mWeaponChannel[sWeaponSection]);
        break;
    case 0xDC19BD13:
    {
        int iAmmo = 0;
        sscanf(pszValue, "%d", &iAmmo);
        mWeaponAmmo[sWeaponSection].bFlagShareAmmo = 0;
        mWeaponAmmo[sWeaponSection].bFlagAmmoInfinite = iAmmo <= 0;
        if (!mWeaponAmmo[sWeaponSection].bFlagAmmoInfinite)
        {
            mWeaponAmmo[sWeaponSection].uiAmmo = iAmmo;
        }
        break;
    }
    case 0x8585BB8A:
    {
        int iWeapon = 0;
        sscanf(pszValue, "%d", &iWeapon);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        if (iWeapon > 0 && iWeapon <= sWeaponSection)
        {
            ammo.bFlagAmmoInfinite = 0;
            ammo.bFlagShareAmmo = 1;
            ammo.uiAmmo = iWeapon - 1;
        }
        else
        {
            ammo.bFlagShareAmmo = 0;
            ammo.bFlagAmmoInfinite = 0;
            ammo.bFlagAmmoInfinite = 1;
        }
        break;
    }
    case 0x9455A446:
    {
        int iWeapon = 0;
        sscanf(pszValue, "%d", &iWeapon);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        if (iWeapon > 0 && iWeapon <= sWeaponSection)
        {
            ammo.bFlagShareHeat = 1;
            ammo.uiHeat = iWeapon - 1;
        }
        else
        {
            ammo.bFlagShareHeat = 0;
        }
        break;
    }

    case 0xBACA6BF1:
        switch (PblHash(pszValue))
        {
        case 0x94D7CDEC:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 1;
            break;
        case 0x4E4FA58B:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 2;
            break;
        case 0xD8AECCCD:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 0;
            break;
        case 0x1643517A:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 1;
            break;
        case 0xCA780CF9:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 2;
            break;
        default:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 0;
            break;
        }
        break;

    case 0x07FF5A07:
        mStompEffect = PblHash(pszValue);
        break;
    case 0x5233B817:
        mVehicleType = FindVehicleType(pszValue);
        break;
    case 0x46BBA6D5:
        sscanf(pszValue, "%f", &mTurnThreshold);
        mTurnThreshold *= 0.017453292f;
        break;
    case 0x47A95951:
        sscanf(pszValue, "%f", &mMaxTerrainAngle);
        mMaxTerrainAngle = (float)cos(mMaxTerrainAngle * 0.017453292f);
        break;

    case 0x02BB1FE0:
    case 0x9B30E763:
        mExplosion = ExplosionClass::Find(PblHash(pszValue));
        break;
    case 0x16181B74:
        if (*pszValue)
        {
            sscanf(pszValue, "%d", &mNoDeathExplosions);
        }
        break;
    case 0x7C8816FC:
        if (*pszValue)
        {
            mDeathDustClassHash = PblHash(pszValue);
        }
        break;
    case 0xAE640110:
        sscanf(pszValue, "%f", &mDeathDustDelay);
        break;
    case 0x060C07E2:
        if (*pszValue)
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            mDeathDustOffset.x = x;
            mDeathDustOffset.y = y;
            mDeathDustOffset.z = z;
        }
        break;
    case 0x0C13FC70:
        sscanf(pszValue, "%f", &mDeathShakeDelay);
        break;
    case 0x94B9538C:
        sscanf(pszValue, "%f", &mDeathShakeForce);
        break;
    case 0xBB80D985:
        sscanf(pszValue, "%f", &mDeathShakeDuration);
        break;
    case 0xB3AF758B:
        sscanf(pszValue, "%f", &mDeathShakeRadius);
        break;

    case 0x21F56ED0:
        mFootstepSounds[0].SetID(pszValue, true, false);
        break;
    case 0x937CEAA5:
        mFootstepSounds[1].SetID(pszValue, true, false);
        break;
    case 0x12B23362:
        mFootstepSounds[2].SetID(pszValue, true, false);
        break;
    case 0x069134E5:
        mHydraulicSound.SetID(pszValue, true, false);
        break;
    case 0x8AE91462:
        sscanf(pszValue, "%f", &mHydraulicSoundHeight);
        break;
    case 0x580DC4EC:
        mSoundJump.SetID(pszValue, true, false);
        break;
    case 0xAD3E89E3:
        mSoundLand.SetID(pszValue, true, false);
        break;

    case 0x18BE5123:
    {

        unsigned int uHash = PblTEMPHash(pszValue);
        mAttachHash = uHash;
        if (mModel)
        {
            const RedSkeleton* pSkeleton = mModel->GetRedModel(0)->GetSkeleton();
            if (pSkeleton)
            {
                for (int i = 0; i < 128; ++i)
                {
                    if (pSkeleton->GetBoneNameHash(i) == uHash)
                    {
                        const PblMatrix& mBone = pSkeleton->GetDefaultPoseMatrix(i);
                        mAttachOffset = mBone.GetTranslation();
                        break;
                    }
                }
            }
        }
        break;
    }
    case 0x5B491ACC:
        sscanf(pszValue, "%f", &mWaterDamageInterval);
        break;
    case 0x829299AD:
        sscanf(pszValue, "%f", &mWaterDamageAmount);
        break;
    case 0x9895BB7A:
        sscanf(pszValue, "%f", &mBallRadius);
        break;
    case 0xACACDB0C:
        sscanf(pszValue, "%f", &mBallMoveSpeed);
        break;
    case 0x12A7E73D:
        sscanf(pszValue, "%f", &mBallStoppedTurnSpeed);
        break;
    case 0x578F6AE6:
        sscanf(pszValue, "%f", &mBallTurnSpeed);
        break;
    case 0x8A5E79C2:
        sscanf(pszValue, "%f", &mBallSprintTurnSpeed);
        break;
    case 0xDC524DD9:
        sscanf(pszValue, "%f", &mBallSlippage);
        break;
    case 0xD2DA8F5E:
        sscanf(pszValue, "%f", &mBallMaxLean);
        mBallMaxLean *= 0.017453292f;
        break;
    case 0xDE6B8538:
        sscanf(pszValue, "%f", &mBallAcceleration);
        break;
    case 0x6560DE67:
        sscanf(pszValue, "%f", &mBallRollingFriction);
        break;
    case 0xE6F7BEFB:
        sscanf(pszValue, "%f", &mMaxBallAngle);
        mMaxBallAngle = (float)cos(mMaxBallAngle * 0.017453292f);
        break;
    case 0x81D52461:
        sscanf(pszValue, "%f", &mSprintTimeForBowling);
        break;
    case 0x4A6AC53F:
        sscanf(pszValue, "%f", &mBowlingPush);
        break;
    case 0xBEBB6ED6:
        sscanf(pszValue, "%f", &mBallSprintSpeedFactor);
        break;
    case 0x2DAA5A0B:
    {
        float fHeight = 0.0f;
        sscanf(pszValue, "%f", &fHeight);
        mBallJumpVerticalSpeed = (float)sqrt(fHeight * 2.0f * 9.8f);
        break;
    }
    case 0x388D3934:
        sscanf(pszValue, "%f", &mBallJumpForwardSpeedFactor);
        break;

    case 0x942ED301:
        sscanf(pszValue, "%f", &mEnergyRestore);
        if (mEnergyRestore < 0.0f)
        {
            mEnergyRestore = 0.0f;
        }
        if (mEnergyRestore > mEnergyRestoreIdle)
        {
            mEnergyRestoreIdle = mEnergyRestore;
        }
        break;
    case 0x082EAF37:
        sscanf(pszValue, "%f", &mEnergyRestoreIdle);
        if (mEnergyRestore > mEnergyRestoreIdle)
        {
            mEnergyRestoreIdle = mEnergyRestore;
        }
        break;
    case 0x134361B9:
        sscanf(pszValue, "%f", &mEnergyDrainSprint);
        if (mEnergyDrainSprint < 0.0f)
        {
            mEnergyDrainSprint = 0.0f;
        }
        break;
    case 0x0B4A7FD4:
        sscanf(pszValue, "%f", &mEnergyCostJump);
        break;
    case 0x2431E118:
        sscanf(pszValue, "%f", &mEnergyCostBowling);
        break;

    case 0x26D7A21C:
        mWakeEffect = PblHash(pszValue);
        break;
    case 0x6363D774:
        mFirstPersonModelHandle = FirstPerson::AddGeometry(pszValue);
        break;
    case 0xF01D5583:
        sscanf(pszValue, "%f", &mUprightWaterDamageHeight);
        break;
    case 0xAC229931:
        sscanf(pszValue, "%f", &mBallWaterDamageHeight);
        break;
    case 0x68C9C2D9:
        mHealthGainEffect = PblHash(pszValue);
        break;
    case 0x8D8743F9:
        mAmmoGainEffect = PblHash(pszValue);
        break;
    case 0x80A20AE3:
        mEnergyGainEffect = PblHash(pszValue);
        break;
    case 0x32839537:
        mBuffHealthEffect = PblHash(pszValue);
        break;
    case 0x5B84DF23:
        mBuffOffenseEffect = PblHash(pszValue);
        break;
    case 0x8B003201:
        mBuffDefenseEffect = PblHash(pszValue);
        break;
    case 0x1F3FEA32:
        mDebuffEffect = PblHash(pszValue);
        break;

    case 0x98557A5E:
        InitAnimations(pszValue);
        break;
    case 0x47C86B4A:
        GameObjectClass::SetProperty(0x47C86B4A, pszValue);
        mLodData = SoldierLodData(mModel ? mModel->GetRedModel(0) : 0, 0);
        break;
    case 0xF1F9F3E3:
        mUprightLowRes = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        mLodData = SoldierLodData(mModel ? mModel->GetRedModel(0) : 0, mUprightLowRes->GetRedModel(0));
        break;
    case 0x19D9AB85:
        mBallLowRes = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        break;

    default:
    {

        const RedModel* pModel = mModel ? mModel->GetRedModel(0) : 0;
        if (!mEnergyBarClass.SetProperty(uProperty, pszValue) &&
            !VehicleEngineClass::SetProperty(uProperty, pszValue) &&
            !PlayableEntitySoundsClass::SetProperty(uProperty, pszValue) &&
            !mAimerClass[sWeaponSection].SetProperty(uProperty, pszValue, pModel, 0) &&
            ((sCurrentCameraSplit && CameraManager::sInstance->m_iNumCam <= 1) ||
             !mCameraSettings[sCurrentCameraStance].SetProperty(uProperty, pszValue)) &&
            !ControllableClass::SetProperty(uProperty, pszValue, pModel))
        {
            GameObjectClass::SetProperty(uProperty, pszValue);
            if (uProperty == 0x47C86B4A)
            {
                if (mModel)
                {
                    mAttachOffset = PblVector3(0.0f, mModel->_mModel->_TotalBoundingBox._vMax.y, 0.0f);
                }
            }
            else if (uProperty == 0xAFE693CE)
            {
                int iMask = mCollisionModel.GetMaskId(PblHash(pszValue));
                if (iMask >= 0)
                {
                    mFootPrimitives[mNumFeet++] = iMask;
                }
            }
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x004a92a0
void EntityDroidekaClass::PostReadSetup()
{
    CollisionPrimitive primitive;
    PblMatrix matrix = gMatrixIdentity;
    primitive.SetSphere(mBallRadius);
    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = mBallRadius + 0.1f;
    matrix.m[3][2] = 0.2f;
    matrix.m[3][3] = 1.0f;
    mCollisionBall.AddBody(new CollisionBodyPrimitive(primitive, "coll_ball", matrix, 0));
    mCollisionBall.SetCollisionMask(COLL_TERRAIN, CollisionMask());
    mCollisionBall.SetCollisionMask(COLL_STATIC, CollisionMask());
    mCollisionBall.SetCollisionMask(COLL_RIGID, CollisionMask());
    mCollisionBall.SetCollisionMask(COLL_SOFT, CollisionMask());
    mCollisionBall.SetCollisionMask(COLL_ORDNANCE, CollisionMask());

    const float fWalkRate = mMaxSpeed / mAnimSpeeds[EntityDroideka::LeftToRight];
    mAnimRates[EntityDroideka::IdleToLeft] = fWalkRate;
    mAnimRates[EntityDroideka::LeftToRight] = fWalkRate;
    mAnimRates[EntityDroideka::RightToLeft] = fWalkRate;
    mAnimRates[EntityDroideka::LeftToIdle] = fWalkRate;
    mAnimRates[EntityDroideka::RightToIdle] = fWalkRate;
    const float fTurnRate = mMaxTurnSpeed * 0.51f / mAnimSpeeds[EntityDroideka::TurnLeft];
    mAnimRates[EntityDroideka::TurnLeft] = fTurnRate;
    mAnimRates[EntityDroideka::TurnRight] = fTurnRate;
    mAnimRates[EntityDroideka::RollToIdle] = mTimeTotals[EntityDroideka::RollToIdle] / mUnrollTime;
    mAnimRates[EntityDroideka::IdleToRoll] = mTimeTotals[EntityDroideka::IdleToRoll] / mRollTime;

    mCollisionModel.mDeathOnFlyerLand = true;
    mCollisionBall.mDeathOnFlyerLand = true;
    mEnergyBarClass.SetPropertyEnergyCost(mEnergyCostJump);
    mEnergyBarClass.SetPropertyEnergyCost(mEnergyCostBowling);
}

// FUNCTION: SWBF2 0x004a9580
void EntityDroidekaClass::InitAnimation(int iIndex, const char* pszName)
{
    RedAnimation* pAnimObj = mAnimObj;
    const ZephyrAnim* pAnim = (pszName && pAnimObj->_pZephyrAnimBank)
        ? pAnimObj->_pZephyrAnimBank->Find(PblTEMPHash(pszName))
        : 0;
    mAnimations[iIndex] = const_cast<ZephyrAnim*>(pAnim);
    mAnimRates[iIndex] = 1.0f;

    ZephyrAnim* pZAnim = mAnimations[iIndex];
    if (!pZAnim || pZAnim->m_u16NumFrames <= 1)
    {
        mAnimSpeeds[iIndex] = 0.0f;
        mTimeTotals[iIndex] = 0.0f;
    }
    else
    {
        int iLastFrame = pZAnim->m_u16NumFrames - 1;
        mTimeTotals[iIndex] = (float)iLastFrame * (1.0f / 30.0f);
        PblVector3 vTrans;
        pZAnim->m_pkJoints[0].GetTranslationUncached(vTrans, (unsigned short)iLastFrame);
        mAnimSpeeds[iIndex] = vTrans.z / mTimeTotals[iIndex];
    }
}

// FUNCTION: SWBF2 0x004a9660
bool EntityDroidekaClass::InitAnimations(const char* pszBank)
{
    mAnimObj = RedAnimation::Find(PblHash(pszBank));
    if (!mAnimObj)
    {
        return false;
    }

    mAnimationCount = EntityDroideka::StateCount;
    mAnimations = new ZephyrAnim*[mAnimationCount];
    mTimeTotals = new float[mAnimationCount];
    mAnimSpeeds = new float[mAnimationCount];
    mAnimRates = new float[mAnimationCount];

    InitAnimation(EntityDroideka::Idle, "idle");
    InitAnimation(EntityDroideka::IdleToLeft, "idle_to_leftfoot");
    InitAnimation(EntityDroideka::IdleToRight, 0);
    InitAnimation(EntityDroideka::LeftToRight, "walk_leftfoot_rightfoot");
    InitAnimation(EntityDroideka::RightToLeft, "walk_rightfoot_leftfoot");
    InitAnimation(EntityDroideka::LeftToIdle, "leftfoot_to_idle");
    InitAnimation(EntityDroideka::RightToIdle, "rightfoot_to_idle");
    InitAnimation(EntityDroideka::TurnLeft, "turnl");
    InitAnimation(EntityDroideka::TurnRight, "turnr");
    InitAnimation(EntityDroideka::Dying, "death01");
    InitAnimation(EntityDroideka::IdleToRoll, "fold");
    InitAnimation(EntityDroideka::Rolling, "roll");
    InitAnimation(EntityDroideka::RollToIdle, "unfold");
    InitAnimation(EntityDroideka::Dead, 0);

    mAnimSpeeds[EntityDroideka::TurnLeft] =
        0.52359879f / (mTimeTotals[EntityDroideka::TurnLeft] * 6.0f);
    mAnimSpeeds[EntityDroideka::TurnRight] =
        -0.52359879f / (mTimeTotals[EntityDroideka::TurnRight] * 6.0f);

    const ZephyrSkeletonShared* pSkel = mAnimObj->_pSkeletonShared;
    mSpineIdx = (int)pSkel->m_kHashCRC2Joint.Find(0x48FAD19F) - 1;
    mJointIdx[0] = (int)pSkel->m_kHashCRC2Joint.Find(0x6008D6E4) - 1;
    mJointIdx[1] = (int)pSkel->m_kHashCRC2Joint.Find(0x6D4BF03D) - 1;
    mJointIdx[2] = (int)pSkel->m_kHashCRC2Joint.Find(0x698AED8A) - 1;
    mArmJointIdx[0] = (int)pSkel->m_kHashCRC2Joint.Find(0xA5C0332D) - 1;
    mArmJointIdx[1] = (int)pSkel->m_kHashCRC2Joint.Find(0x9031BDCA) - 1;
    return true;
}

extern bool netIsLocalTurn;

// FUNCTION: SWBF2 0x0049eaf0
EntityDroideka::EntityDroideka(EntityDroidekaClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_SOFT, &pClass->mLodData)
    , Controllable(pClass, desc.mPlayerId)
    , PlayableEntitySounds(pClass)
    , VehicleEngine(pClass)
    , mTime(0.0f)
    , mFootState(0)
    , mState(Idle)
    , mDirection(None)
    , mPostCollision(this)
    , mSpeedPrev(0.0f)
    , mBallAngularVelocity(0.0f)
    , mStompTime(0.0f)
    , mFallTimer(0.0f)
    , mEnergyBar(&pClass->mEnergyBarClass)
    , mDebuffDamageOwner(0, 0, 0, 0, 0, 1.0f)
{

    mHealthGainEffectHandle = NULL;
    mAmmoGainEffectHandle = NULL;
    mEnergyGainEffectHandle = NULL;
    mBuffHealthEffectHandle = NULL;
    mBuffOffenseEffectHandle = NULL;
    mBuffDefenseEffectHandle = NULL;
    mDebuffEffectHandle = NULL;
    mCallUpdateWalkingGroundNormal = false;
    mUprightLowRes = pClass->mUprightLowRes;
    mBallLowRes = pClass->mBallLowRes;
    TargetManager::AddTarget(this, true);
    mControllableList.AddLast(&this->Controllable::mNode, this);
    mAnimObj = pClass->mAnimObj;
    mClass = pClass;
    AddToScene();

    mVelocity.z = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.x = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        mAimer[i] = 0;
        mWeapon[i] = 0;
    }
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (!mClass->mWeaponClass[i])
        {
            continue;
        }

        mAimer[i] = new Aimer(&pClass->mAimerClass[i], this, &mMatrix, &mPose, 0);

        WeaponDesc weaponDesc;
        weaponDesc.mOwner = this;
        weaponDesc.mAimer = mAimer[i];
        weaponDesc.mTrigger = &mControlFire[mClass->mWeaponChannel[i]];
        weaponDesc.mReload = &mControlReload;
        weaponDesc.mNumClips = 0;
        weaponDesc.m_pAmmoCounter = 0;
        weaponDesc.m_pEnergyBar = 0;

        const WeaponAmmoDesc& ammo = mClass->mWeaponAmmo[i];
        if (ammo.bFlagShareAmmo)
        {
            weaponDesc.m_pAmmoCounter = mWeapon[ammo.uiAmmo]->m_pAmmoCounter;
        }
        else if (ammo.bFlagAmmoInfinite)
        {
            weaponDesc.mNumClips = 0x7fffffff;
        }
        else
        {
            weaponDesc.mNumClips = ammo.uiAmmo;
        }
        if (ammo.bFlagShareHeat)
        {
            weaponDesc.m_pEnergyBar = mWeapon[ammo.uiHeat]->m_pEnergyBar;
        }

        mWeapon[i] = mClass->mWeaponClass[i]->Build(weaponDesc);
    }
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        mWeaponIndex[iChannel] = -1;
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (mWeapon[i] && mClass->mWeaponChannel[i] == iChannel)
            {
                mWeaponIndex[iChannel] = i;
                break;
            }
        }
    }
    ClearControllable();

    mZephyrSkeleton.Open(*pClass->mAnimObj->GetZephyrSkeletonShared());
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    mModel->GetRedModel(0)->SetupPose(mPose);

    mVelocity.y = 0.0f;
    mGroundNormal = gVectorUnitY;
    mIsDead = false;
    mPitchValue = 0.0f;
    mAILastCollisionNormal = gVectorZero;
    mEyeDir = mMatrix.GetZAxis();
    mEyePoint = GetCameraTrackSettings()->CalcEyePoint(mMatrix);
    _uiRenderFlags &= 3;
    NetGame::Created(this);
    mDoneDustCloud = false;
    mDeathExplosionTime = 0.0f;
    mDeathTime = 0.0f;
    mLastFootHeight[0] = 0.0f;
    mLastFootHeight[1] = 0.0f;
    mLastFootHeight[2] = 0.0f;
    mWaterDamageTimer = mClass->mWaterDamageInterval;
    mWakeEmitter = NULL;
    m_bSprint = false;
    m_bIsGrounded = false;
    mCanChangeClass = 0;
    mDoNotPlayCollisionEffect = 0;
    mBuffOffenseTimer = 0.0f;
    mBuffOffenseMult = 0.0f;
    mBuffDefenseTimer = 0.0f;
    mBuffDefenseMult = 0.0f;
    mBuffHealthTimer = 0.0f;
    mBuffHealthRate = 0.0f;
    mDebuffDamageTimer = 0.0f;
    mDebuffDamageRate = 0.0f;
}

// FUNCTION: SWBF2 0x0049fce0
int EntityDroideka::GetJoystickIndex()
{
    if (mPlayerId < 0)
    {
        return -1;
    }
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return NetGame::GetJoystickIndex(mPlayerId);
    }
    return -1;
}

// FUNCTION: SWBF2 0x0049fd50
void EntityDroideka::Init()
{
    GameObject::Init();
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->Init();
        }
    }
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        int iIndex = mWeaponIndex[iChannel];
        if (iIndex >= 0 && mWeapon[iIndex])
        {
            mWeapon[iIndex]->Select(iChannel, false);
        }
    }
    NextState(0);
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        for (PblList<Controllable>::Itor it(GetControllableList()); it; ++it)
        {
            it->SetupController();
        }
    }
}

// FUNCTION: SWBF2 0x0049fe20
void EntityDroideka::SetupController()
{
    Controller* pController;
    if (mPlayerId >= 0)
    {
        pController = new PlayerController(this);
        if (NetGame::IsEnabled() && NetGame::IsClient())
        {
            int iSpectator = NetGame::GetSpectator(NetGame::GetLocalIndex(0));
            if (iSpectator != -1)
            {
                Track(iSpectator);
            }
            else
            {
                Track(mPlayerId);
            }
        }
        else
        {
            Track(mPlayerId);
        }
    }
    else
    {
        if (!mPilot)
        {
            return;
        }
        if (NetGame::IsEnabled() && NetGame::IsClient())
        {
            return;
        }
        pController = new UnitMobileController(this);
    }
    if (pController)
    {
        AddController(pController, mPlayerId < 0);
    }
}

// FUNCTION: SWBF2 0x004a0d90
int EntityDroideka::GetWeaponCount() const
{
    return mClass->mWeaponCount;
}

// FUNCTION: SWBF2 0x004a0da0
const ControllableClass* EntityDroideka::GetControllableClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x004a0dc0
ControllableClass* EntityDroideka::GetControllableClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x004a0df0
float EntityDroideka::GetSmallestCollisionDim() const
{
    return mClass->mBallRadius;
}



// FUNCTION: SWBF2 0x004a0e00
bool EntityDroideka::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                       Restrictor* pRestrictor)
{
    if (mState == Rolling && pOther->GetGameObject() && pOther->GetGameObject()->GetEntityType() == ENT_SOLDIER &&
        !(mClass->mSprintTimeForBowling > mStompTime) &&
        DroidekaEnergyBar(this)->SpendEnergy(mClass->mEnergyCostBowling))
    {
        GameObject* pObject = static_cast<GameObject*>(pOther->GetEntity());
        float fOverlap = pResult->mOverlap;
        PblVector3 vForward(fOverlap * mMatrix.m[2][0], mMatrix.m[2][1] * fOverlap, mMatrix.m[2][2] * fOverlap);
        const PblVector3& vMine = GetCollisionCenter();
        const PblVector3& vTheirs = pObject->GetCollisionCenter();
        PblVector3 vDir(vTheirs.x - vMine.x + vForward.x, vTheirs.y - vMine.y + vForward.y,
                        vTheirs.z - vMine.z + vForward.z);
        if (vDir.y < 0.0f)
        {
            vDir.y = 0.0f;
        }
        float fAlong = (mMatrix.m[2][1] * vDir.y + vDir.x * mMatrix.m[2][0] + mMatrix.m[2][2] * vDir.z) * 0.5f;
        PblVector3 vSide(vDir.x - fAlong * mMatrix.m[2][0], vDir.y - mMatrix.m[2][1] * fAlong,
                         vDir.z - mMatrix.m[2][2] * fAlong);
        PblVector3 vPush;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vPush), reinterpret_cast<const D3DXVECTOR3*>(&vSide));
        pObject->ApplyPush(vPush,
                           ((mMatrix.m[2][1] * vPush.y + vPush.x * mMatrix.m[2][0] + mMatrix.m[2][2] * vPush.z) +
                            1.0f) *
                               0.5f * mClass->mBowlingPush,
                           false);
        mStompTime = mClass->mSprintTimeForBowling * 0.5f;
    }

    if (pOther->GetCollisionType() == COLL_ITEM)
    {
        return false;
    }

    const PblVector3& vSeparation = pResult->mSeparationNormal;
    if (AIUtil::ShouldSoldierDodgeThisType(pOther->GetCollisionType()) && fabs(vSeparation.y) < 0.3f)
    {
        mAILastCollisionNormal.x += vSeparation.x;
        mAILastCollisionNormal.y += vSeparation.y;
        mAILastCollisionNormal.z += vSeparation.z;
    }

    if (pOther->GetCollisionType() >= GetCollisionType())
    {
        PblVector3 vRelVel = mVelocity;
        if (pOther->GetGameObject())
        {
            const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
            vRelVel.x -= vOtherVel.x;
            vRelVel.y -= vOtherVel.y;
            vRelVel.z -= vOtherVel.z;
        }
        float fApproach = vSeparation.y * vRelVel.y + vRelVel.x * vSeparation.x + vSeparation.z * vRelVel.z;
        if (fApproach < 0.0f)
        {
            float fUp = mMatrix.m[1][1] * vSeparation.y + mMatrix.m[1][0] * vSeparation.x +
                        mMatrix.m[1][2] * vSeparation.z;
            float fFront = mMatrix.m[2][1] * vSeparation.y + mMatrix.m[2][0] * vSeparation.x +
                           mMatrix.m[2][2] * vSeparation.z;
            float fThreshold = mClass->mCollisionThreshold[1] * fUp + mClass->mCollisionThreshold[0] +
                               mClass->mCollisionThreshold[2] * fFront;

            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
            PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);
            VehicleEngine::Collision(mClass, mMatrix.GetTranslation(), mVelocity);

            if (-fThreshold > fApproach)
            {
                float fExcess = fThreshold + fApproach;
                float fDamage = (mClass->mCollisionScale[1] * fUp + mClass->mCollisionScale[0] +
                                 mClass->mCollisionScale[2] * fFront) *
                                fExcess * fExcess;
                if (pOther->GetGameObject())
                {
                    fDamage = pOther->GetGameObject()->GetGameObjectClass()->GetCollisionInflict() * fDamage;
                }
                if (fDamage > 0.0f)
                {
                    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
                    if (pOther->GetGameObject() && pOther->GetGameObject()->GetControllable())
                    {
                        owner.mCharacter = pOther->GetGameObject()->GetControllable()->mCharacter;
                        owner.mGameObject = pOther->GetGameObject();
                        owner.mTeam = pOther->GetGameObject()->GetTeam();
                        owner.mClass = pOther->GetGameObject()->GetGameObjectClass();
                    }
                    Damageable::ApplyDamage(DamageDesc(owner, fDamage), 0xffffffff);
                }
            }
        }
    }

    int eType = pOther->GetCollisionType();
    if (eType != COLL_RIGID && eType != COLL_STATIC && eType != COLL_TERRAIN)
    {
        return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    }

    if (mState == Rolling)
    {
        PblVector3 vNormal = vSeparation;
        float fOverlap = pResult->mOverlap;
        if (vNormal.y > 0.6f)
        {
            float fLift = fOverlap / vNormal.y;
            SetPosition(PblVector3(mMatrix.m[3][0] + fLift * gVectorUnitY.x, mMatrix.m[3][1] + fLift * gVectorUnitY.y,
                                   mMatrix.m[3][2] + fLift * gVectorUnitY.z));
        }
        else
        {
            SetPosition(PblVector3(mMatrix.m[3][0] + vNormal.x * fOverlap, mMatrix.m[3][1] + fOverlap * vNormal.y,
                                   mMatrix.m[3][2] + fOverlap * vNormal.z));
        }

        float fSpeed = mVelocity.y * vNormal.y + mVelocity.x * vNormal.x + mVelocity.z * vNormal.z;
        if (-fSpeed > 4.0f)
        {
            float fBounce = (-fSpeed - 4.0f) * 0.5f;
            float fScale = fBounce - fSpeed;
            mVelocity.x += vNormal.x * fScale;
            mVelocity.y += fScale * vNormal.y;
            mVelocity.z += fScale * vNormal.z;
            mBallLean -= PblAngle((mVelocity.y * mMatrix.m[0][1] + mVelocity.x * mMatrix.m[0][0] +
                                   mVelocity.z * mMatrix.m[0][2]) *
                                  0.05f);
            if (mClass->mStompEffect && !mDoNotPlayCollisionEffect)
            {
                mDoNotPlayCollisionEffect = 15;
                if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mStompEffect))
                {
                    if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                    {
                        pEffect->AttachEffectToMatrix(&mMatrix, false);
                    }
                }
            }
            fSpeed = fBounce;
        }
        else if (-fSpeed > 0.0f)
        {
            mVelocity.x += -fSpeed * vNormal.x;
            mVelocity.y += -fSpeed * vNormal.y;
            mVelocity.z += -fSpeed * vNormal.z;
            fSpeed = 0.0f;
        }

        if (vNormal.y > mClass->mMaxBallAngle)
        {
            if (fSpeed > 0.01f)
            {
                return true;
            }

            float fArm = -(mClass->mBallRadius / mClass->mBallSlippage);
            PblVector3 vArm(fArm * vNormal.x, fArm * vNormal.y, fArm * vNormal.z);
            PblVector3 vSpin(mBallAngularVelocity * mMatrix.m[0][0], mMatrix.m[0][1] * mBallAngularVelocity,
                             mMatrix.m[0][2] * mBallAngularVelocity);
            m_bIsGrounded = true;
            mVelocity.x = vSpin.z * vArm.y - vSpin.y * vArm.z;
            mVelocity.y = vArm.z * vSpin.x - vSpin.z * vArm.x;
            mVelocity.z = vSpin.y * vArm.x - vArm.y * vSpin.x;
            mGroundNormal = vSeparation;
            return true;
        }
        if (vNormal.y < 0.0f)
        {

            PblVector3 vFlat = vSeparation;
            float fDist = pResult->mOverlap;
            float fDot = mGroundNormal.y * vFlat.y + mGroundNormal.x * vFlat.x + mGroundNormal.z * vFlat.z;
            vFlat.x -= fDot * mGroundNormal.x;
            vFlat.y -= mGroundNormal.y * fDot;
            vFlat.z -= mGroundNormal.z * fDot;
            float fLength = sqrtf(vFlat.x * vFlat.x + vFlat.y * vFlat.y + vFlat.z * vFlat.z);
            if (fLength > 0.0f)
            {
                float fInv = 1.0f / fLength;
                vFlat.x = fInv * vFlat.x;
                vFlat.y = vFlat.y * fInv;
                vFlat.z = vFlat.z * fInv;
                fDist = pResult->mOverlap / fLength;
            }
            PblVector3 vDir = pRestrictor->RestrictMovement(vFlat, fDist);
            if (fDist > 5.0f)
            {
                fDist = 0.0f;
                DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
                if (pOther->GetGameObject() && pOther->GetGameObject()->GetControllable())
                {
                    owner.mCharacter = pOther->GetGameObject()->GetControllable()->mCharacter;
                    owner.mGameObject = pOther->GetGameObject();
                    owner.mTeam = pOther->GetGameObject()->GetTeam();
                    owner.mClass = pOther->GetGameObject()->GetGameObjectClass();
                }
                Damageable::InstantDeath(owner, true);
            }
            PblVector3 vMove = pRestrictor->GetRestrictedMovement(vDir, fDist);
            if (vMove.y > 0.0f)
            {
                vMove.y -= 0.02f;
                if (!(vMove.y > 0.0f))
                {
                    vMove.y = 0.0f;
                }
            }
            SetPosition(PblVector3(mMatrix.m[3][0] + vMove.x, mMatrix.m[3][1] + vMove.y, mMatrix.m[3][2] + vMove.z));
            if (pRestrictor)
            {
                pRestrictor->AddRestriction(vFlat);
            }
            return true;
        }
        if (pRestrictor)
        {
            pRestrictor->AddRestriction(vSeparation);
        }
        return true;
    }

    PblVector3 vNormal = vSeparation;
    if (vNormal.y > 0.0f)
    {
        if (vNormal.y > 0.7f)
        {

            mVelocity.y = 0.0f;
            float fLift = pResult->mOverlap / vNormal.y;
            SetPosition(PblVector3(mMatrix.m[3][0] + fLift * gVectorUnitY.x, mMatrix.m[3][1] + fLift * gVectorUnitY.y,
                                   mMatrix.m[3][2] + fLift * gVectorUnitY.z));
            pRestrictor->AddRestriction(vSeparation);
            m_bIsGrounded = true;
            mCallUpdateWalkingGroundNormal = true;
            return true;
        }

        PblVector3 vFlat(vNormal.x, 0.0f, vNormal.z);
        float fLength = sqrtf(vFlat.x * vFlat.x + 0.0f + vFlat.z * vFlat.z);
        float fInv = 1.0f / fLength;
        vFlat.x = fInv * vFlat.x;
        vFlat.y = fInv * 0.0f;
        vFlat.z = vFlat.z * fInv;
        float fDist = pResult->mOverlap / fLength;
        SetPosition(PblVector3(mMatrix.m[3][0] + vFlat.x * fDist, mMatrix.m[3][1] + vFlat.y * fDist,
                               mMatrix.m[3][2] + vFlat.z * fDist));
        pRestrictor->AddRestriction(vFlat);
        return true;
    }

    float fDist = pResult->mOverlap;
    PblVector3 vDir = pRestrictor->RestrictMovement(vSeparation, fDist);
    if (fDist > 5.0f)
    {
        fDist = 0.0f;
        DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
        if (pOther->GetGameObject() && pOther->GetGameObject()->GetControllable())
        {
            owner.mCharacter = pOther->GetGameObject()->GetControllable()->mCharacter;
            owner.mGameObject = pOther->GetGameObject();
            owner.mTeam = pOther->GetGameObject()->GetTeam();
            owner.mClass = pOther->GetGameObject()->GetGameObjectClass();
        }
        Damageable::InstantDeath(owner, true);
    }
    PblVector3 vMove = pRestrictor->GetRestrictedMovement(vDir, fDist);
    SetPosition(PblVector3(mMatrix.m[3][0] + vMove.x, mMatrix.m[3][1] + vMove.y, mMatrix.m[3][2] + vMove.z));
    pRestrictor->AddRestriction(vSeparation);
    return true;
}

// FUNCTION: SWBF2 0x004a1eb0
void EntityDroideka::SetTerrainHeight(int, float)
{
}

// FUNCTION: SWBF2 0x004a2030
void EntityDroideka::UpdatePilot()
{
    bool bUnroll = mPlayerId >= 0 && mControlCrouch.IsPressed() && mState == Rolling;
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        bUnroll |= mPlayerId < 0 && mState == Rolling &&
                   (!UnitControllerPtr(mCtrl) || UnitControllerPtr(mCtrl)->GetLowLevel().mFoldStatus == FOLD_UNFOLD);
    }
    if (bUnroll && CanUnroll())
    {
        NextState(0);
    }
    else
    {
        bool bRoll = mPlayerId >= 0 && mControlCrouch.IsPressed() && mState != Rolling && mState != IdleToRoll &&
                     mState != RollToIdle;
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            bRoll |= mPlayerId < 0 && !mIsDead && mState != Rolling && mState != IdleToRoll && mState != RollToIdle &&
                     UnitControllerPtr(mCtrl) && UnitControllerPtr(mCtrl)->GetLowLevel().mFoldStatus == FOLD_FOLD;
        }
        if (bRoll)
        {
            NextState(7);
        }
    }
    if (static_cast<unsigned int>(mState - IdleToRoll) <= static_cast<unsigned int>(RollToIdle - IdleToRoll))
    {
        mControlFire[0].Clear();
    }
}

// FUNCTION: SWBF2 0x004a21d0
void EntityDroideka::UpdatePlayer(float fDeltaTime)
{
    if (mPlayerId < 0 || !IsAlive())
    {
        return;
    }

    if (mControlView.IsPressed() && netIsLocalTurn && !IsForcedThirdPerson() && !IsForcedFirstPerson() &&
        mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
    {
        SwitchControlView(mPlayerId);
    }

    if (mState == Rolling || mState == IdleToRoll)
    {
        mTargetInfo.mIsAiming = false;
    }
    else
    {
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mClass->mZoomMax > 1.0f)
        {
            if (mControlZoom.IsPressed())
            {
                mTargetInfo.mIsAiming = mWeapon[iIndex]->CycleZoom(mTargetInfo.mIsAiming);
            }
            if (mWeapon[0]->mClass->mZoomRate != 0.0f)
            {
                mWeapon[0]->UpdateZoom(fDeltaTime, mControlMove);
            }
        }
    }

    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0)
        {
            FirstPerson::Update(iFirstPerson, TYPE_DROIDEKA, mClass->mFirstPersonModelHandle, this, 0);
        }
    }
}

// FUNCTION: SWBF2 0x004a5a00
bool EntityDroideka::UpdateIndirect(float fDeltaTime)
{
    AILowLevel& lowLevel = UnitControllerPtr(mCtrl)->GetLowLevel();
    if (!lowLevel.UpdateIndirect(fDeltaTime) || !mCtrl)
    {
        return true;
    }

    ControllableHeading heading = lowLevel.mHeading;
    lowLevel.ProcessFire(fDeltaTime, &heading);
    float fPitch = GetPitchAdjusted();
    float fThrust = mControlMove;
    float fTurn = GetTurnAdjusted();
    float fStrafe;
    AIUtil::UpdatePhysics(this, fDeltaTime, lowLevel.mDest, heading, mClass->mMaxSpeed, 0.0f,
               lowLevel.mRolling ? mClass->mBallTurnSpeed : mClass->mMaxTurnSpeed, mClass->mMaxPitchSpeed,
               &fThrust, &fStrafe, &fTurn, &fPitch);
    mControlMove = fThrust;
    mControlTurn = fTurn;
    mControlPitch = fPitch;
    return true;
}

// FUNCTION: SWBF2 0x004a5fa0
float EntityDroideka::GetSoundRadius() const
{
    float fRadius = 0.0f;
    float fFeetRadius = 0.0f;
    unsigned int uNumFeet = mClass->mNumFeet;
    for (unsigned int i = 0; i < uNumFeet; ++i)
    {
        if (mFootState & (1 << i))
        {
            float fMute = mClass->mFootstepSounds[i].GetMuteDistance();
            if (fMute > fFeetRadius)
            {
                fFeetRadius = fMute;
                fRadius = fMute;
            }
        }
    }
    float fEngine = VehicleEngine::GetMuteDistance();
    if (fEngine > fFeetRadius)
    {
        fRadius = fEngine;
    }
    return mClass->mNoiseFactor * fRadius;
}

// FUNCTION: SWBF2 0x004a6050
CameraTrackSetting* EntityDroideka::GetCameraTrackSettings()
{
    int iCamera = mTargetInfo.mIsAiming;
    if (mState == Rolling || mState == IdleToRoll)
    {
        iCamera = 2;
    }
    return &mClass->mCameraSettings[iCamera];
}

// FUNCTION: SWBF2 0x004a60a0
void EntityDroideka::ActivatePhysics()
{
    ActivateThread(0, PER_TURN);
    mPostCollision.ActivateThread(-15, PER_TURN);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mAimer[i])
        {
            mAimer[i]->ActivatePhysics();
        }
    }
}

// FUNCTION: SWBF2 0x004a6140
float EntityDroideka::GetMaxSpeed() const
{
    return mClass->mMaxSpeed;
}

// FUNCTION: SWBF2 0x004a6a20
void EntityDroideka::TriggerBuffHealthEffect()
{
    if (mBuffHealthTimer > 0.0f)
    {
        if (mClass->mBuffHealthEffect && mBuffHealthEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mBuffHealthEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mBuffHealthEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mBuffHealthEffectHandle)
    {
        mBuffHealthEffectHandle->StopAndFinish();
        mBuffHealthEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004a6b20
void EntityDroideka::TriggerBuffOffenseEffect()
{
    if (mBuffOffenseTimer > 0.0f)
    {
        if (mClass->mBuffOffenseEffect && mBuffOffenseEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mBuffOffenseEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mBuffOffenseEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mBuffOffenseEffectHandle)
    {
        mBuffOffenseEffectHandle->StopAndFinish();
        mBuffOffenseEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004a6c20
void EntityDroideka::TriggerBuffDefenseEffect()
{
    if (mBuffDefenseTimer > 0.0f)
    {
        if (mClass->mBuffDefenseEffect && mBuffDefenseEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mBuffDefenseEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mBuffDefenseEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mBuffDefenseEffectHandle)
    {
        mBuffDefenseEffectHandle->StopAndFinish();
        mBuffDefenseEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004a6d20
void EntityDroideka::TriggerDebuffEffect()
{
    if (mDebuffDamageTimer > 0.0f)
    {
        if (mClass->mDebuffEffect && mDebuffEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mDebuffEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mDebuffEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mDebuffEffectHandle)
    {
        mDebuffEffectHandle->StopAndFinish();
        mDebuffEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004a81f0
Entity* EntityDroidekaClass::Build(const EntityDesc& desc)
{
    if (EntityDroideka::sMemoryPool.IsFull())
    {
        return 0;
    }
    EntityDroideka* pDroideka = new EntityDroideka(this, desc);
    if (pDroideka)
    {
        pDroideka->AddSoftToCollisionManager();
        pDroideka->ActivatePhysics();
        pDroideka->BuildEffects();
    }
    return pDroideka;
}

// FUNCTION: SWBF2 0x0049e980
bool EntityDroideka::CollideTrisWithAllPrimitives() const
{
    return true;
}

// SYNTHETIC: SWBF2 0x004a98ec
// ?GetMatrix@EntityDroideka@@WCDE@BEABVPblMatrix@@XZ

// SYNTHETIC: SWBF2 0x004a9902
// ?GetEntity@EntityDroideka@@WM@AEPAVEntity@@XZ

// SYNTHETIC: SWBF2 0x004a9912
// ?GetGameObject@EntityDroideka@@WBEA@AEPAVGameObject@@XZ

// SYNTHETIC: SWBF2 0x004a9928
// ?GetGameObject@EntityDroideka@@WCFI@AEPAVGameObject@@XZ

// SYNTHETIC: SWBF2 0x004a9933
// ?GetGameObject@EntityDroideka@@WCFI@BEPBVGameObject@@XZ

// SYNTHETIC: SWBF2 0x004a993e
// ?GetGameObject@EntityDroideka@@WM@AEPAVGameObject@@XZ

// SYNTHETIC: SWBF2 0x004a994e
// ?GetEntity@EntityDroideka@@WCEA@AEPAVEntity@@XZ

// SYNTHETIC: SWBF2 0x004a98ce
// ??_EEntityDroideka@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004a98d6
// ??_EEntityDroideka@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004a98f7
// ??_EEntityDroideka@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004a990a
// ?GetEntity@EntityDroideka@@WM@BEPBVEntity@@XZ
// SYNTHETIC: SWBF2 0x004a991d
// ?GetGameObject@EntityDroideka@@WBEA@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004a9946
// ?GetGameObject@EntityDroideka@@WM@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004a9964
// ?GetControllable@EntityDroideka@@WCFI@AEPAVControllable@@XZ
// SYNTHETIC: SWBF2 0x004a996f
// ?GetControllable@EntityDroideka@@WCFI@BEPBVControllable@@XZ
// SYNTHETIC: SWBF2 0x004a997a
// ?Kill@EntityDroideka@@WBAA@AEXXZ
// SYNTHETIC: SWBF2 0x004a9985
// ??_EEntityDroideka@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004a9990
// ??_EEntityDroideka@@WDMM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004a999b
// ??_EEntityDroideka@@WCFI@AEPAXI@Z

// SYNTHETIC: SWBF2 0x004a9959
// ?GetEntity@EntityDroideka@@WCEA@BEPBVEntity@@XZ


extern bool netFrameLock;
extern bool netIsLateTurn;

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// GLOBAL: SWBF2 0x007e6340
EntityDroideka::StateTableEntry EntityDroideka::sStateTable[StateCount] = {
    {{Idle, Idle, IdleToLeft, LeftToIdle, TurnLeft, TurnRight, Dying, IdleToRoll}},
    {{Idle, Idle, IdleToLeft, LeftToIdle, Idle, Idle, Dying, IdleToRoll}},
    {{Idle, Idle, IdleToLeft, RightToIdle, Idle, Idle, Dying, IdleToRoll}},
    {{Dead, Dead, Dead, Dead, Dead, Dead, Dead, IdleToRoll}},
    {{Dead, Dead, Dead, Dead, Dead, Dead, Dead, IdleToRoll}},
    {{LeftToIdle, Idle, LeftToRight, RightToIdle, LeftToIdle, LeftToIdle, Dying, IdleToRoll}},
    {{RightToIdle, Idle, RightToLeft, LeftToIdle, RightToIdle, RightToIdle, Dying, IdleToRoll}},
    {{RightToIdle, IdleToLeft, RightToLeft, RightToLeft, RightToIdle, RightToIdle, Dying, IdleToRoll}},
    {{LeftToIdle, LeftToRight, LeftToRight, LeftToRight, LeftToIdle, LeftToIdle, Dying, IdleToRoll}},
    {{Idle, IdleToLeft, IdleToLeft, RightToLeft, TurnLeft, TurnRight, Dying, IdleToRoll}},
    {{Idle, LeftToRight, IdleToLeft, LeftToRight, TurnLeft, TurnRight, Dying, IdleToRoll}},
    {{Rolling, Rolling, Rolling, Rolling, Rolling, Rolling, Dead, Rolling}},
    {{RollToIdle, RollToIdle, Rolling, Rolling, Rolling, Rolling, Dead, Rolling}},
    {{Idle, Idle, Idle, Idle, Idle, Idle, Dead, Idle}},
};

// FUNCTION: SWBF2 0x0049ffb0
void EntityDroideka::DoFootImpactEffects()
{
    if (mTracker && mTracker->IsFirstPersonView())
    {
        if (mStompTime > 0.1f)
        {
            if (FoleyFX* pFoley = GetFoleyFX())
            {
                static_cast<FoleyFXWalker*>(pFoley)->mFootstep[0].Play(&mMatrix.GetTranslation(), 0, false, true);
            }
            mClass->mFootstepSounds[0].Play(&mMatrix.GetTranslation(), 0, false, true);
            mStompTime = 0.0f;
        }
        return;
    }

    const PblMatrix* pFoot0 = mPose.Find(0xF63B8713 /* "bone_l_leg2" */);
    const PblMatrix* pFoot1 = mPose.Find(0xC3CA09F4 /* "bone_r_leg2" */);
    const PblMatrix* pFoot2 = mPose.Find(0x758BC3C3 /* "bone_b_leg2" */);
    float fHeight[3];
    fHeight[0] = pFoot0 ? pFoot0->m[3][1] : 0.0f;
    fHeight[1] = pFoot1 ? pFoot1->m[3][1] : 0.0f;
    fHeight[2] = pFoot2 ? pFoot2->m[3][1] : 0.0f;

    for (unsigned int i = 0; i < 3; ++i)
    {
        float fFoot = fHeight[i];
        bool bLanded = fFoot < 0.48f && mLastFootHeight[i] > 0.48f;
        unsigned int uHydraulicBit = 1 << (i + 8);
        if (fFoot > mLastFootHeight[i])
        {
            if (fFoot > mClass->mHydraulicSoundHeight && !(mFootState & uHydraulicBit))
            {
                mClass->mHydraulicSound.Play(&mMatrix.GetTranslation(), 0, false, true);
                mFootState |= uHydraulicBit;
            }
        }
        else
        {
            mFootState &= ~uHydraulicBit;
        }
        mLastFootHeight[i] = fFoot;
        if (bLanded)
        {
            if (FoleyFX* pFoley = GetFoleyFX())
            {
                static_cast<FoleyFXWalker*>(pFoley)->mFootstep[i].Play(&mMatrix.GetTranslation(), 0, false, true);
            }
            mClass->mFootstepSounds[i].Play(&mMatrix.GetTranslation(), 0, false, true);
            mFootState |= 1 << i;
        }
        else
        {
            mFootState &= ~(1 << i);
        }
    }
}

// FUNCTION: SWBF2 0x004a2370
void EntityDroideka::UpdateState(float fDeltaTime, float fMove, float fStrafe, float fTurn)
{
    bool bMoving = fMove * fMove + fStrafe * fStrafe > 0.01f;
    float fDistance = 0.0f;

    float fTurnSpeed;
    if (mPlayerId >= 0 && !NetGame::IsMouseMode(mPlayerId))
    {
        fTurnSpeed = (mClass->mMaxTurnSpeed - mClass->mMaxPitchSpeed) * mTurnBuildup + mClass->mMaxPitchSpeed;
    }
    else
    {
        fTurnSpeed = mClass->mMaxTurnSpeed;
    }
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]] && mPlayerId >= 0)
    {
        fTurnSpeed /= mWeapon[mWeaponIndex[0]]->mZoomTurnScale;
    }
    AdjustRotation(fDeltaTime * fTurn * fTurnSpeed);

    bool bForward = false;
    if (mState == Idle)
    {
        float fTarget = 0.0f;
        if (bMoving)
        {
            fTarget = fMove >= 0.0f ? atan2f(-fStrafe, fMove) : atan2f(fStrafe, -fMove);
        }
        float fDiff = fTarget - atan2f(mTurnOffset.mSin, mTurnOffset.mCos) * 6.0f;
        if (fDiff > mClass->mTurnThreshold)
        {
            NextState(4);
        }
        else if (-mClass->mTurnThreshold > fDiff)
        {
            NextState(5);
        }
        else if (bMoving)
        {
            bForward = fMove >= 0.0f;
            if (bForward)
            {
                NextState(2);
            }
            else
            {
                NextState(3);
            }
        }
        mStompTime = 0.0f;
    }
    else
    {
        if (mState != TurnLeft && mState != TurnRight)
        {
            float fTarget = 0.0f;
            if (bMoving)
            {
                float fLimit = mTurnOffset.mCos * fMove - mTurnOffset.mSin * fStrafe > 0.0f ? -0.3f : 0.3f;
                if (fMove >= fLimit)
                {
                    bForward = true;
                    fTarget = atan2f(-fStrafe, fMove);
                }
                else
                {
                    fTarget = atan2f(fStrafe, -fMove);
                }
            }
            float fCurrent = atan2f(mTurnOffset.mSin, mTurnOffset.mCos) * 6.0f;
            float fDiff = fTarget - fCurrent;
            if (fDiff > 3.1415927f)
            {
                fDiff -= 6.2831855f;
            }
            else if (-3.1415927f > fDiff)
            {
                fDiff = 6.2831855f + fDiff;
            }
            if (fDiff > 1.4f || -1.4f > fDiff)
            {
                bForward = false;
                bMoving = false;
            }
            float fMax = mClass->mMaxTurnSpeed * fDeltaTime;
            if (fDiff > fMax)
            {
                fDiff = fMax;
            }
            else if (-fMax > fDiff)
            {
                fDiff = -fMax;
            }
            PblAngle kTurn((fDiff + fCurrent) * (1.0f / 6.0f));
            mTurnOffset.mCos = kTurn.mCos;
            mTurnOffset.mSin = kTurn.mSin;
        }
        mStompTime += fDeltaTime;
        DoFootImpactEffects();
    }

    if (!(fDeltaTime > 0.0f))
    {
        return;
    }

    if (bMoving)
    {
        if (bForward)
        {
            if (mDirection == Backward)
            {
                mZephyrPoseDyn.PlayAnimForward();
                mDirection = Forward;
            }
        }
        else if (mDirection == Forward)
        {
            mZephyrPoseDyn.PlayAnimBackward();
            mDirection = Backward;
        }
    }

    float fRate = mClass->mAnimRates[mState];
    float fSign;
    float fRemain;
    if (mDirection == Backward)
    {
        fSign = -1.0f;
        fRemain = mTime;
    }
    else
    {
        fSign = 1.0f;
        fRemain = mClass->mTimeTotals[mState] - mTime;
    }
    float fStep = fRate * fDeltaTime;
    if (fStep >= fRemain)
    {

        float fLeft = fDeltaTime - fRemain / fRate;
        mTime = fRemain * fSign + mTime;
        mZephyrPoseDyn.Update(fRemain);
        switch (mState)
        {
        case Idle:
        case Dying:
        case Dead:
        case Rolling:
            break;
        case TurnLeft:
        case TurnRight:
            mTurnOffset += PblAngle(mClass->mAnimSpeeds[mState] * fRemain);
            break;
        default:
        {
            float fStride = mClass->mAnimSpeeds[mState] * fRemain * fSign;
            fDistance += fStride;
            MoveForward(fStride);
            break;
        }
        }

        if (mIsDead)
        {
            NextState(6);
        }
        else if (bMoving)
        {
            if (bForward)
            {
                NextState(2);
            }
            else
            {
                NextState(3);
            }
        }
        else
        {
            NextState(0);
        }
        if (mState == Rolling)
        {
            return;
        }

        fSign = mDirection == Backward ? -1.0f : 1.0f;
        fStep = mClass->mAnimRates[mState] * fLeft;
    }

    if (mDirection != Backward || mTime >= fStep)
    {
        mTime = fStep * fSign + mTime;
        mZephyrPoseDyn.Update(fStep);
    }
    switch (mState)
    {
    case Idle:
    case Dying:
    case Dead:
    case Rolling:
        break;
    case TurnLeft:
    case TurnRight:
        mTurnOffset += PblAngle(mClass->mAnimSpeeds[mState] * fStep);
        break;
    default:
    {
        float fStride = mClass->mAnimSpeeds[mState] * fStep * fSign;
        fDistance += fStride;
        MoveForward(fStride);
        break;
    }
    }
    mVelocity.x = mMatrix.m[2][0] * (fDistance / fDeltaTime);
    mVelocity.z = mMatrix.m[2][2] * (fDistance / fDeltaTime);
}

// FUNCTION: SWBF2 0x004a2b70
void EntityDroideka::UpdateStateRolling(float fDeltaTime, float fMove, float fTurn)
{
    float fThrust = fMove;
    if (mPlayerId >= 0 && fTurn * fTurn > 0.1f * 0.1f)
    {
        if (fMove >= 0.0f && fMove < 0.2f)
        {
            fThrust = 0.2f;
        }
        else if (fMove < 0.0f && -fMove < 0.2f)
        {
            fThrust = -0.2f;
        }
    }

    float fTurnRate;
    if (m_bSprint)
    {
        fTurnRate = mClass->mBallSprintTurnSpeed;
        mStompTime += fDeltaTime;
    }
    else
    {
        fTurnRate = ((float)fabs((double)fMove) * (mClass->mBallTurnSpeed - mClass->mBallStoppedTurnSpeed) +
                     mClass->mBallStoppedTurnSpeed) /
                    1.53f;
        if (mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z >
            mClass->mBallMoveSpeed * 1.1f * mClass->mBallMoveSpeed)
        {
            mStompTime += fDeltaTime;
        }
        else if (mStompTime > 0.0f)
        {
            mStompTime = 0.0f;
        }
    }

    RotateY(PblAngle(-(fTurnRate * fTurn * fDeltaTime)));

    float fHalfStep = fDeltaTime * 0.5f;
    float fLean = (fThrust > 0.0f ? 1.0f : -1.0f) * fTurn * mClass->mBallMaxLean - mBallLean.mSin;
    if (fLean > fHalfStep)
    {
        mBallLean.mSin = mBallLean.mSin + fHalfStep;
    }
    else if (-fLean > fHalfStep)
    {
        mBallLean.mSin = mBallLean.mSin - fHalfStep;
    }
    else
    {
        mBallLean.mSin = mBallLean.mSin + fLean;
    }
    mBallLean.mCos = 1.0f;
    mBallLean.Normalize();

    float fAccel;
    if (mControlJump.IsPressed())
    {
        EndSprint();
        Jump();
        fAccel = 0.0f;
    }
    else
    {
        bool bSprint;
        if (m_bSprint)
        {
            bSprint = !mControlSprint.Tapped() && !mControlSprint.HeldAndReleased() && !(fThrust < 0.9f);
        }
        else
        {
            bSprint = mControlThrustFwd.TappedAndPressed() ||
                      ((mControlSprint.Tapped() || mControlSprint.Held()) && fThrust >= 0.9f);
        }
        if (bSprint && Sprint())
        {
            fAccel = mClass->mBallAcceleration * fThrust * mClass->mBallSprintSpeedFactor;
        }
        else
        {
            EndSprint();
            fAccel = mClass->mBallAcceleration * fThrust;
        }
    }

    mBallAngularVelocity = fAccel * fDeltaTime + mBallAngularVelocity;
    if (m_bIsGrounded)
    {
        float fFriction = mClass->mBallRollingFriction * fDeltaTime;
        if (mBallAngularVelocity > fFriction)
        {
            mBallAngularVelocity -= fFriction;
        }
        else if (-fFriction > mBallAngularVelocity)
        {
            mBallAngularVelocity += fFriction;
        }
        else
        {
            mBallAngularVelocity = 0.0f;
        }
    }
    float fMaxSpin = mClass->mBallSlippage * mClass->mBallMoveSpeed / mClass->mBallRadius;
    if (m_bSprint)
    {
        fMaxSpin = mClass->mBallSprintSpeedFactor * fMaxSpin;
    }
    if (mBallAngularVelocity > fMaxSpin)
    {
        mBallAngularVelocity = fMaxSpin;
    }
    else if (-fMaxSpin > mBallAngularVelocity)
    {
        mBallAngularVelocity = -fMaxSpin;
    }

    PblAngle kOrientation(mBallOrientation.mCos - mBallAngularVelocity * mBallOrientation.mSin * fHalfStep,
                          mBallAngularVelocity * mBallOrientation.mCos * fHalfStep + mBallOrientation.mSin);
    kOrientation.Normalize();
    mBallOrientation.mCos = kOrientation.mCos;
    mBallOrientation.mSin = kOrientation.mSin;
}

// FUNCTION: SWBF2 0x004a3060
void EntityDroideka::SetState(int iState)
{
    if (iState == Dead)
    {
        mState = iState;
        return;
    }

    switch (iState)
    {
    case Idle:
    case TurnLeft:
    case TurnRight:
    case Dying:
    case Dead:
    case IdleToRoll:
    case Rolling:
    case RollToIdle:
        mDirection = None;
        break;
    default:
        break;
    }

    float fTime = 0.0f;
    if (ZephyrAnim* pAnim = mClass->mAnimations[iState])
    {
        mZephyrPoseDyn.SetLoop(0);
        if (mDirection == Backward)
        {
            fTime = 1.0f;
            mTime = mClass->mTimeTotals[iState];
            mZephyrPoseDyn.PlayAnimBackward();
        }
        else
        {
            mTime = 0.0f;
            mZephyrPoseDyn.PlayAnimForward();
        }
        mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);

        mZephyrPoseDyn.SetCurTime(fTime);
        mZephyrPoseDyn.Update(0.0f);
    }

    if (mState != iState)
    {
        if (iState == IdleToRoll)
        {
            for (int i = 0; i < 2; ++i)
            {
                int iIndex = mWeaponIndex[i];
                if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->IsSelected())
                {
                    mWeapon[iIndex]->Deselect(i, false);
                }
            }
        }
        else if (iState == Idle)
        {
            if (mState == RollToIdle)
            {
                for (int i = 0; i < 2; ++i)
                {
                    int iIndex = mWeaponIndex[i];
                    if (iIndex >= 0 && mWeapon[iIndex] && !mWeapon[iIndex]->IsSelected())
                    {
                        mWeapon[iIndex]->Select(i, false);
                    }
                }
            }
        }
        if (iState == Rolling)
        {
            SetCollisionModel(&mClass->mCollisionBall);
            mStompTime = 0.0f;
        }
        else if (mState == Rolling)
        {
            SetCollisionModel(&mClass->mCollisionModel);
        }
    }
    BuildWorldMatrices();
    mState = iState;
}

// FUNCTION: SWBF2 0x004a3240
void EntityDroideka::NextState(int iInput)
{
    switch (iInput)
    {
    case 0:
        if (mDirection == Backward)
        {
            iInput = 1;
        }
        break;
    case 2:
        mDirection = Forward;
        break;
    case 3:
        mDirection = Backward;
        break;
    }

    if (mState == Rolling)
    {
        mVelocity.z = 0.0f;
        mVelocity.y = 0.0f;
        mVelocity.x = 0.0f;
        mTurnOffset.mCos = 1.0f;
        mTurnOffset.mSin = 0.0f;
        mGroundNormal = PblVector3(0.0f, 1.0f, 0.0f);
    }
    SetState(sStateTable[mState].next[iInput]);
    if (mState == Rolling)
    {
        mTurnOffset.mCos = 1.0f;
        mTurnOffset.mSin = 0.0f;
        mBallOrientation.mCos = 1.0f;
        mBallOrientation.mSin = 0.0f;
        mBallAngularVelocity = 0.0f;
    }
}

// FUNCTION: SWBF2 0x004a3350
void EntityDroideka::AdjustRotation(float fAngle)
{
    RotateY(PblAngle(-fAngle));
    mTurnOffset += PblAngle(fAngle / 6.0f);
}

// FUNCTION: SWBF2 0x004a3430
void EntityDroideka::BuildWorldMatrices()
{
    ZephyrPoseStatic<32> kPose;
    mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
    kPose.Open(mZephyrSkeleton);
    kPose.Set(mZephyrPoseDyn, false);
    PblBitVector<32> kDirty;
    kDirty.SetAll();

    if (mState == Rolling)
    {
        PblQuaternion qLean;
        qLean._fX = 0.0f;
        qLean._fY = 0.0f;
        qLean._fZ = mBallLean.mSin;
        qLean._fS = mBallLean.mCos;
        PblQuaternion qRoll;
        qRoll._fX = mBallOrientation.mSin;
        qRoll._fY = 0.0f;
        qRoll._fZ = 0.0f;
        qRoll._fS = mBallOrientation.mCos;
        PblQuaternion qSpin = qLean * qRoll;

        PblQuaternion qUnit;
        PblMatrix kSpin;
        D3DXMatrixRotationQuaternion(reinterpret_cast<D3DXMATRIX*>(&kSpin), D3DXQuaternionNormalize(reinterpret_cast<D3DXQUATERNION*>(&qUnit), reinterpret_cast<const D3DXQUATERNION*>(&qSpin)));
        kSpin.SetTranslation(gVectorZero);

        mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity, mClass->mSpineIdx, kDirty);
        PblMatrix& rSpine = mZephyrSkeleton.m_pWorldMatrices[mClass->mSpineIdx];
        PblMatrix kSpine;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&kSpine), reinterpret_cast<const D3DXMATRIX*>(&kSpin), reinterpret_cast<const D3DXMATRIX*>(&rSpine));
        rSpine = kSpine;

        PblVector3 vCentre(0.0f - rSpine.m[3][0], mClass->mBallRadius - rSpine.m[3][1], 0.0f - rSpine.m[3][2]);
        PblVector3 vRotated;
        D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vRotated), reinterpret_cast<const D3DXVECTOR3*>(&vCentre), reinterpret_cast<const D3DXMATRIX*>(&rSpine));
        mTranslationOffset =
            PblVector3(vCentre.x - vRotated.x, vCentre.y - vRotated.y, vCentre.z - vRotated.z);
        rSpine.m[3][0] = mTranslationOffset.x + rSpine.m[3][0];
        rSpine.m[3][1] = rSpine.m[3][1] + mTranslationOffset.y;
        rSpine.m[3][2] = rSpine.m[3][2] + mTranslationOffset.z;
    }
    else
    {

        float fCos3 = (mTurnOffset.mCos * mTurnOffset.mCos * 4.0f - 3.0f) * mTurnOffset.mCos;
        float fSin3 = (mTurnOffset.mCos * mTurnOffset.mCos * 4.0f - 1.0f) * mTurnOffset.mSin;
        float fCos6 = fCos3 * fCos3 - fSin3 * fSin3;
        float fSin6 = fCos3 * 2.0f * fSin3;

        PblVector3 vUp(mGroundNormal.y * mMatrix.m[0][1] + mGroundNormal.x * mMatrix.m[0][0] +
                           mGroundNormal.z * mMatrix.m[0][2],
                       mMatrix.m[1][1] * mGroundNormal.y + mGroundNormal.x * mMatrix.m[1][0] +
                           mMatrix.m[1][2] * mGroundNormal.z,
                       mGroundNormal.y * mMatrix.m[2][1] + mGroundNormal.x * mMatrix.m[2][0] +
                           mGroundNormal.z * mMatrix.m[2][2]);
        float fAlong = vUp.x * fSin6 + vUp.y * 0.0f + vUp.z * fCos6;
        PblVector3 vFlat(vUp.y * fSin6 - fAlong * gVectorUnitY.x, vUp.y * 0.0f - fAlong * gVectorUnitY.y,
                         vUp.y * fCos6 - fAlong * gVectorUnitY.z);
        PblVector3 vFront;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vFront), reinterpret_cast<const D3DXVECTOR3*>(&vFlat));
        PblVector3 vRight(vUp.y * vFront.z - vUp.z * vFront.y, vUp.z * vFront.x - vUp.x * vFront.z,
                          vUp.x * vFront.y - vUp.y * vFront.x);

        PblMatrix kTilt(vRight, vUp, vFront, gVectorZero);

        mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity, mClass->mSpineIdx, kDirty);
        PblMatrix& rSpine = mZephyrSkeleton.m_pWorldMatrices[mClass->mSpineIdx];
        PblMatrix kSpine;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&kSpine), reinterpret_cast<const D3DXMATRIX*>(&kTilt), reinterpret_cast<const D3DXMATRIX*>(&rSpine));
        rSpine = kSpine;

        float fYaw = atan2f(fSin6, fCos6);
        float fPitch = asinf(vFront.y);
        float fRoll = asinf(kTilt.m[0][1]);
        float fHalfYaw = fYaw * 0.5f;

        PblMatrix kLegs;
        if (fabs(fRoll) > 0.04f)
        {
            PblVector3 vAxis(fSin6, 0.0f, fCos6);
            kLegs.SetTranslation(gVectorZero);
            D3DXMatrixRotationAxis(reinterpret_cast<D3DXMATRIX*>(&kLegs), reinterpret_cast<const D3DXVECTOR3*>(&vAxis), fRoll * -0.3333f);
        }
        else
        {
            kLegs = gMatrixIdentity;
        }
        PblMatrix kRot;
        if (fabs(fPitch) > 0.04f)
        {
            PblVector3 vAxis(-fCos6, 0.0f, fSin6);
            kRot.SetTranslation(gVectorZero);
            D3DXMatrixRotationAxis(reinterpret_cast<D3DXMATRIX*>(&kRot), reinterpret_cast<const D3DXVECTOR3*>(&vAxis), fPitch * -0.3333f);
            kLegs *= kRot;
        }
        if (fabs(fHalfYaw) > 0.04f)
        {
            kLegs.RotateY(fHalfYaw * -0.3333f);
        }

        for (int i = 0; i < 3; ++i)
        {
            mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity, mClass->mJointIdx[i], kDirty);
            PblMatrix& rJoint = mZephyrSkeleton.m_pWorldMatrices[mClass->mJointIdx[i]];
            PblVector3 vZ;
            PblVector3 vY;
            PblVector3 vX;
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vZ), reinterpret_cast<const D3DXVECTOR3*>(&rJoint.GetZAxis()), reinterpret_cast<const D3DXMATRIX*>(&kLegs));
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vY), reinterpret_cast<const D3DXVECTOR3*>(&rJoint.GetYAxis()), reinterpret_cast<const D3DXMATRIX*>(&kLegs));
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vX), reinterpret_cast<const D3DXVECTOR3*>(&rJoint.GetXAxis()), reinterpret_cast<const D3DXMATRIX*>(&kLegs));
            PblMatrix kJoint;
            kJoint.m[0][0] = vX.x;
            kJoint.m[0][1] = vX.y;
            kJoint.m[0][2] = vX.z;
            kJoint.m[0][3] = 0.0f;
            kJoint.m[1][0] = vY.x;
            kJoint.m[1][1] = vY.y;
            kJoint.m[1][2] = vY.z;
            kJoint.m[1][3] = 0.0f;
            kJoint.m[2][0] = vZ.x;
            kJoint.m[2][1] = vZ.y;
            kJoint.m[2][2] = vZ.z;
            kJoint.m[2][3] = 0.0f;
            kJoint.m[3][0] = rJoint.m[3][0];
            kJoint.m[3][1] = rJoint.m[3][1];
            kJoint.m[3][2] = rJoint.m[3][2];
            kJoint.m[3][3] = 1.0f;
            rJoint = kJoint;
        }

        PblMatrix kArms;
        kArms.SetTranslation(gVectorZero);
        kArms.SetRotateY(-fHalfYaw);
        for (int i = 0; i < 2; ++i)
        {
            mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity, mClass->mArmJointIdx[i], kDirty);
            PblMatrix& rJoint = mZephyrSkeleton.m_pWorldMatrices[mClass->mArmJointIdx[i]];
            PblVector3 vZ;
            PblVector3 vY;
            PblVector3 vX;
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vZ), reinterpret_cast<const D3DXVECTOR3*>(&rJoint.GetZAxis()), reinterpret_cast<const D3DXMATRIX*>(&kArms));
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vY), reinterpret_cast<const D3DXVECTOR3*>(&rJoint.GetYAxis()), reinterpret_cast<const D3DXMATRIX*>(&kArms));
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vX), reinterpret_cast<const D3DXVECTOR3*>(&rJoint.GetXAxis()), reinterpret_cast<const D3DXMATRIX*>(&kArms));
            PblMatrix kJoint;
            kJoint.m[0][0] = vX.x;
            kJoint.m[0][1] = vX.y;
            kJoint.m[0][2] = vX.z;
            kJoint.m[0][3] = 0.0f;
            kJoint.m[1][0] = vY.x;
            kJoint.m[1][1] = vY.y;
            kJoint.m[1][2] = vY.z;
            kJoint.m[1][3] = 0.0f;
            kJoint.m[2][0] = vZ.x;
            kJoint.m[2][1] = vZ.y;
            kJoint.m[2][2] = vZ.z;
            kJoint.m[2][3] = 0.0f;
            kJoint.m[3][0] = rJoint.m[3][0];
            kJoint.m[3][1] = rJoint.m[3][1];
            kJoint.m[3][2] = rJoint.m[3][2];
            kJoint.m[3][3] = 1.0f;
            rJoint = kJoint;
        }
    }

    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity, kDirty);
}

// FUNCTION: SWBF2 0x004a65f0
void EntityDroideka::UpdateBuffTimers(float fDeltaTime)
{
    if (!IsAlive())
    {
        TurnOffEffects();
        mBuffOffenseTimer = 0.0f;
        mBuffDefenseTimer = 0.0f;
        mBuffHealthTimer = 0.0f;
        mDebuffDamageTimer = 0.0f;
        return;
    }

    if (mCharacter && mCharacter->mMedalsMgr)
    {
        MedalsMgr* pMedals = mCharacter->mMedalsMgr;
        if (pMedals->IsAwardAvailable(MedalsMgr::WAR_HERO))
        {
            mBuffOffenseTimer = 340.0f;
            mBuffOffenseMult = 1.5f;
        }
        if (pMedals->IsAwardAvailable(MedalsMgr::GUARDIAN))
        {
            mBuffDefenseTimer = 340.0f;
            mBuffDefenseMult = 0.5f;
        }
    }

    if (mBuffOffenseTimer <= 300.0f)
    {
        mBuffOffenseTimer -= fDeltaTime;
    }
    if (mBuffOffenseTimer < 0.0f)
    {
        mBuffOffenseTimer = 0.0f;
    }
    TriggerBuffOffenseEffect();

    if (mBuffDefenseTimer <= 300.0f)
    {
        mBuffDefenseTimer -= fDeltaTime;
    }
    if (mBuffDefenseTimer < 0.0f)
    {
        mBuffDefenseTimer = 0.0f;
    }
    TriggerBuffDefenseEffect();

    if (mBuffHealthTimer <= 300.0f)
    {
        mBuffHealthTimer -= fDeltaTime;
    }
    if (mBuffHealthTimer < 0.0f)
    {
        mBuffHealthTimer = 0.0f;
    }
    else if (mBuffHealthTimer > 0.0f)
    {
        const float fMaxHealth = mMaxHealth;
        float fHealth = mBuffHealthRate * fDeltaTime + mCurHealth;
        SetCurHealth(fHealth > fMaxHealth ? fMaxHealth : fHealth, true, NULL);
    }
    TriggerBuffHealthEffect();

    if (mDebuffDamageTimer <= 300.0f)
    {
        mDebuffDamageTimer -= fDeltaTime;
    }
    if (mDebuffDamageTimer < 0.0f)
    {
        mDebuffDamageTimer = 0.0f;
    }
    else if (mDebuffDamageTimer > 0.0f)
    {
        DamageDesc desc(mDebuffDamageOwner, mDebuffDamageRate * fDeltaTime);
        Damageable::ApplyDamage(desc, 0xffffffff);
    }
    TriggerDebuffEffect();
}

// FUNCTION: SWBF2 0x004a50b0
bool EntityDroideka::PostCollisionUpdate(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    PblVector3 vLocal;
    vLocal.x = 0.0f;
    vLocal.y = sinf(mPitchValue);
    vLocal.z = cosf(mPitchValue);
    PblVector3 vEyeDir;
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vEyeDir), reinterpret_cast<const D3DXVECTOR3*>(&vLocal), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
    mEyeDir = vEyeDir;
    mEyePoint = GetCameraTrackSettings()->CalcEyePoint(mMatrix);

    if (mPlayerId >= 0 && mTracker)
    {
        if (mTargetInfo.mIsAiming || mTracker->IsFirstPersonView())
        {
            mTargetInfo.mAimStart = mEyePoint;
        }
        else
        {

            PblMatrix mLook =
                DirectionMatrix(PblVector3(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z), mMatrix.GetYAxis(), gVectorZero);
            PblQuaternion qLook = PblQuaternion(mLook);
            float fSin;
            float fCos;
            PblMath::SinCos(&fSin, &fCos, -GetCameraTrackSettings()->GetCameraTiltValue() * 0.5f);
            PblQuaternion qTilt;
            qTilt._fX = fSin * gVectorUnitX.x;
            qTilt._fY = fSin * gVectorUnitX.y;
            qTilt._fZ = fSin * gVectorUnitX.z;
            qTilt._fS = fCos;
            PblMatrix mCamera(qLook * qTilt, gVectorZero);

            mTargetInfo.mAimStart = GetCameraTrackSettings()->CalcCameraPoint(mMatrix, mCamera);
            float fAlong = DotProduct(mEyeDir, mEyePoint - mTargetInfo.mAimStart);
            mTargetInfo.mAimStart.x = fAlong * mEyeDir.x + mTargetInfo.mAimStart.x;
            mTargetInfo.mAimStart.y = mTargetInfo.mAimStart.y + mEyeDir.y * fAlong;
            mTargetInfo.mAimStart.z = mTargetInfo.mAimStart.z + mEyeDir.z * fAlong;
        }
        const GameObject* pIgnore = this;
        CollisionObject* pHit;
        float fDist =
            CollisionManager::RayHit(mTargetInfo.mAimStart, mEyeDir, 512.0f, pHit, 0, &pIgnore, 1, 0x9a, true) *
            512.0f;
        mTargetInfo.mAimPoint = mTargetInfo.mAimStart + mEyeDir * fDist;
    }
    else
    {
        mTargetInfo.mAimStart = mEyePoint;
        float fDist = 512.0f;
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mTarget)
        {
            PblVector3 vTarget = mWeapon[iIndex]->mTarget->GetTargetPoint(mEyePoint, mEyeDir, -1);
            fDist = sqrtf((vTarget.x - mEyePoint.x) * (vTarget.x - mEyePoint.x) +
                          (vTarget.y - mEyePoint.y) * (vTarget.y - mEyePoint.y) +
                          (vTarget.z - mEyePoint.z) * (vTarget.z - mEyePoint.z));
        }
        mTargetInfo.mAimPoint = mEyeDir * fDist + mEyePoint;
    }

    if (!mIsDead)
    {
        float fMoveX;
        float fMoveZ;
        float fMoveY = mVelocity.y * fDeltaTime;
        if (mState == Rolling)
        {
            fMoveX = mVelocity.x * fDeltaTime;
            fMoveZ = mVelocity.z * fDeltaTime;
        }
        else
        {
            fMoveX = 0.0f;
            fMoveZ = 0.0f;
        }
        SetPosition(PblVector3(mMatrix.m[3][0] + fMoveX, mMatrix.m[3][1] + fMoveY, mMatrix.m[3][2] + fMoveZ));
        mVelocity.y = fDeltaTime * -9.8f + mVelocity.y;
    }

    mClass->UpdatePilotAnimation(mPilot, &mPose, mMatrix, mVelocity, fDeltaTime, -1.0f, -1.0f, -1.0f);

    if (mState == Rolling)
    {
        PblVector3 vMin = GetModel()->_mModel->_TotalBoundingBox._vMin;
        FLEffectClass* pWake;
        if (mClass->mWakeEffect && (pWake = FLEffect::Get(mClass->mWakeEffect)) != 0 &&
            (mControlMove < 0.0f ? -mControlMove : mControlMove) + (mControlStrafe < 0.0f ? -mControlStrafe : mControlStrafe) > 0.01f)
        {
            PblMatrix mLocal = mCollisionModel->GetLocalMatrix(0, GetPose());
            PblMatrix mOffset = mMatrix;
            mOffset.m[3][0] = mLocal.m[3][0];
            mOffset.m[3][1] = vMin.y;
            mOffset.m[3][2] = mLocal.m[3][2];
            mOffset.m[3][3] = 1.0f;
            if (!mWakeEmitter)
            {
                mWakeEmitter = pWake->BuildEffect();
                if (mWakeEmitter)
                {
                    mWakeEmitter->AttachEffectToObject(this, mOffset, true);
                }
            }
        }
        else if (mWakeEmitter)
        {
            mWakeEmitter->DeactivateEffect();
            mWakeEmitter = NULL;
        }
    }
    else if (mWakeEmitter)
    {
        mWakeEmitter->DeactivateEffect();
        mWakeEmitter = NULL;
    }

    if (Smooth(fDeltaTime))
    {
        const PblVector3& vCenter = GetCollisionCenter();
        const PblVector3& vPos = GetPosition();
        const PblMatrix& mSmoothed = GetSmoothedMatrix();
        SetRenderPos(PblVector3(vCenter.x + mSmoothed.m[3][0] - vPos.x, mSmoothed.m[3][1] + vCenter.y - vPos.y,
                                mSmoothed.m[3][2] + vCenter.z - vPos.z));
    }

    if (!netFrameLock && NetGame::IsEnabled())
    {
        if (!NetGame::IsClient() || !netIsLocalTurn)
        {
            if (!NetGame::IsHost() || !NetGame::IsOneWorld() || netIsLateTurn)
            {
                return true;
            }
            if (mNetUniqueId >= 0)
            {
                mNetUniqueId = NetGame::GetHostWorldUniqueId(mNetUniqueId, this);
            }
        }
        gameObjectInterpolator.SetNext(mNetUniqueId, &GetSmoothedMatrix());
        gameObjectInterpolator.SetNextFloat(mNetUniqueId, 0, GetTurnState());
        gameObjectInterpolator.SetNextFloat(mNetUniqueId, 1, (float)mState);
        gameObjectInterpolator.SetNextFloat(mNetUniqueId, 2, atan2f(mBallOrientation.mSin, mBallOrientation.mCos));
    }
    return true;
}

// FUNCTION: SWBF2 0x004a3f90
bool EntityDroideka::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    Controllable::Update(fDeltaTime);
    if (mDoNotPlayCollisionEffect)
    {
        --mDoNotPlayCollisionEffect;
    }
    UpdateBuffTimers(fDeltaTime);
    mCanChangeClass = 0;

    if (!mIsDead)
    {
        int iJoystick = GetJoystickIndex();
        Weapon* pCurWeapon = GetCurWpn();
        bool bTargetHandled = false;
        if (iJoystick >= 0 &&
            (!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn) ||
             (NetGame::IsHost() && NetGame::IsOneWorld() && netIsLocalTurn)))
        {
            LockOnManager* pManager = LockOnManager::GetByPlayerIndex(iJoystick);
            if (mState == Rolling || mState == IdleToRoll || !pCurWeapon)
            {
                TargetTrackingForceClear();
                pManager->RemoveTargetLock();
                mAllowedLockTypes = 0;
                bTargetHandled = true;
            }
            else
            {
                int iTypes = 0;
                float fLockDist = 2000.0f;
                float fLoseDist = 2500.0f;
                for (int i = 0; i < 2; ++i)
                {
                    if (mWeaponIndex[i] != -1)
                    {
                        const WeaponClass* pClass = mWeapon[mWeaponIndex[i]]->mClass;
                        iTypes = pClass->mTargetTypes;
                        fLockDist = pClass->mTargetLockMaxDistance;
                        fLoseDist = pClass->mTargetLockMaxDistanceLose;
                        break;
                    }
                }
                pManager->SetAllowedTypes(iTypes);
                mAllowedLockTypes = iTypes;
                pManager->SetAutoLose(2.0f, true, 0.4f, fLoseDist);
                pManager->SetAutoLoseLocked(2.0f, true, 0.1f, fLoseDist);
                pManager->SetTarget(0.5f, 0.8f, fLockDist, true);
                pManager->SetAuto(true, false);
                pManager->SetNumAttackerTargets(0);
                pManager->SetNumNearestTargets(pManager->GetDisplayIconsEverything() ? 64 : 2);

                if (mControlLockTarget.IsPressed())
                {
                    pManager->ForceNewTargetLock(0);
                    bTargetHandled = true;
                }
                else if (mControlLockTarget.Held())
                {
                    pManager->RemoveTargetLock();
                    bTargetHandled = true;
                }
            }
        }
        else if (mPlayerId >= 0 && iJoystick < 0 && NetGame::IsEnabled() && !NetGame::IsClient())
        {

            int iTypes = 0;
            for (int i = 0; i < 2; ++i)
            {
                if (mWeaponIndex[i] != -1)
                {
                    iTypes = mWeapon[mWeaponIndex[i]]->mClass->mTargetTypes;
                    break;
                }
            }
            mAllowedLockTypes = iTypes;
        }

        if (mPlayerId >= 0 && !(NetGame::IsEnabled() && NetGame::IsClient()) && !bTargetHandled)
        {
            GameObject* pTarget = GetTargetLockedObj();
            if (pTarget &&
                !LockOnManager::IsTargetObjectTypeAllowed(pTarget, GetGameObject(), false, mAllowedLockTypes, 0, false))
            {
                pTarget = GetVehicleIfAttached(pTarget);
                if (pTarget &&
                    LockOnManager::IsTargetObjectTypeAllowed(pTarget, GetGameObject(), false, mAllowedLockTypes, 0,
                                                             false))
                {
                    SetTargetLockedObj(pTarget);
                }
                else
                {
                    SetTargetLockedObj(0);
                }
            }
        }

        if (mPlayerId >= 0)
        {

            for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
            {
                CommandPost* pPost = *itor;
                if (!(pPost->mHoldPlayerMask & (1 << mCharacter->mPlayerId)))
                {
                    continue;
                }
                const PblVector3& vMe = GetCollisionCenter();
                const PblVector3& vPost = pPost->GetGameObject()->GetCollisionCenter();
                PblVector3 vDir(vPost.x - vMe.x, vPost.y - vMe.y, vPost.z - vMe.z);
                float fDist = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
                if (fDist > 0.0001f)
                {
                    float fInv = 1.0f / fDist;
                    if (mEyeDir.x * (vDir.x * fInv) + mEyeDir.y * (vDir.y * fInv) + mEyeDir.z * (vDir.z * fInv) >
                            0.15f &&
                        pPost->CanTeamSpawnHere(GetTeam()))
                    {
                        mCanChangeClass = 1;
                        break;
                    }
                }
            }
            if (!mCanChangeClass)
            {
                GameObject* aObjects[64];
                int iCount =
                    CollisionManager::GetObjectsInRange(GetCollisionCenter(), 30.0f, aObjects, 64, this, 0x10);
                for (int i = 0; i < iCount; ++i)
                {
                    const PblVector3& vMe = GetPosition();
                    const PblVector3& vOther = aObjects[i]->GetPosition();
                    PblVector3 vDir(vOther.x - vMe.x, vOther.y - vMe.y, vOther.z - vMe.z);
                    float fDist = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
                    if (fDist > 0.0001f)
                    {
                        float fInv = 1.0f / fDist;
                        if (aObjects[i]->GetGameObjectClass()->mSwitchClassRadius > fDist &&
                            mEyeDir.x * (vDir.x * fInv) + mEyeDir.y * (vDir.y * fInv) + mEyeDir.z * (vDir.z * fInv) >
                                0.15f &&
                            aObjects[i]->mTeam == mTeam)
                        {
                            mCanChangeClass = 1;
                            break;
                        }
                    }
                }
            }
            if (iJoystick >= 0 && mControlUse.IsPressed() && mCanChangeClass)
            {
                SpawnDisplay::Instance(iJoystick)->ChangeClass(mCharacter);
            }
        }

        float fWaterHeight;
        float fDamageHeight = (mState == Rolling || mState == IdleToRoll) ? mClass->mBallWaterDamageHeight
                                                                           : mClass->mUprightWaterDamageHeight;
        if (RedWater::GetWaterHeight(GetPosition(), fWaterHeight) && fWaterHeight >= GetPosition().y + fDamageHeight)
        {
            mWaterDamageTimer -= fDeltaTime;
            if (0.0f > mWaterDamageTimer)
            {
                Damageable::ApplyDamage(DamageDesc(DamageOwner(0, 0, 0, 0, 0, 1.0f), mClass->mWaterDamageAmount),
                                        0xffffffff);
            }
        }
        else
        {
            mWaterDamageTimer = mClass->mWaterDamageInterval;
        }

        if (IsAlive())
        {
            if (!(NetGame::IsEnabled() && NetGame::IsClient()))
            {
                PblVector3 vPos = mMatrix.GetTranslation();
                if (-500.0f > vPos.y || TriggerRegionManager::sInstance->mSinkHeight > vPos.y)
                {
                    Kill();
                    return false;
                }
            }
            UpdateStatus(fDeltaTime);
        }
        else
        {
            Die();
        }
    }

    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        int iIndex = mWeaponIndex[iChannel];
        if (iIndex < 0 || mControlSwitch[iChannel] == 0.0f)
        {
            continue;
        }

        int iNew = iIndex;
        do
        {
            if (mControlSwitch[iChannel] < 0.0f)
            {
                if (--iNew < 0)
                {
                    iNew = mClass->mWeaponCount - 1;
                }
            }
            else
            {
                if (++iNew > mClass->mWeaponCount - 1)
                {
                    iNew = 0;
                }
            }
        } while (iNew != mWeaponIndex[iChannel] && mClass->mWeaponChannel[iNew] != iChannel);

        if (iNew != iIndex)
        {
            mWeapon[iIndex]->Deselect(iChannel, false);
            mWeaponIndex[iChannel] = iNew;
            mWeapon[iNew]->Select(iChannel, false);
            mTargetInfo.mIsAiming = false;
        }
        else if (GetJoystickIndex() >= 0)
        {
            WeaponClass::sWeaponChangeUnableSound.Play(&mMatrix.GetTranslation(), 0, false, true);
        }
    }

    UpdatePilot();
    UpdatePlayer(fDeltaTime);

    float fMove;
    float fStrafe;
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]] &&
        mWeapon[mWeaponIndex[0]]->mClass->mZoomRate != 0.0f)
    {
        fMove = 0.0f;
        fStrafe = 0.0f;
    }
    else
    {
        fMove = mControlMove;
        fStrafe = mControlStrafe;
    }
    float fTurn = GetTurnAdjusted();
    float fPitch = GetPitchAdjusted();
    if (mPlayerId >= 0)
    {
        if (NetGame::IsMouseMode(mPlayerId) && (mState == Rolling || mState == IdleToRoll))
        {
            fTurn = UpdateMouseDecay(fDeltaTime, fTurn);
        }
        if (mState == IdleToRoll)
        {
            fTurn = 0.0f;
            static_cast<PlayerController*>(mCtrl)->SetMouseBuildup(0.0f);
        }
    }

    float fPitchSpeed = mClass->mMaxTurnSpeed;
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
    {
        fPitchSpeed /= mWeapon[mWeaponIndex[0]]->mZoomTurnScale;
    }
    mPitchValue = fPitchSpeed * fPitch * fDeltaTime + mPitchValue;

    if (mIsDead)
    {
        if (mIsDead && mState == Dead)
        {
            if (mClass->mExplosion)
            {
                DamageOwner owner(mCharacter, this, GetTeam(), mClass, 0, 1.0f);
                mClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
            }
            CreateChunkCluster(1.0f);
            Kill();
            if (!(NetGame::IsEnabled() && NetGame::IsClient()))
            {
                return false;
            }
        }
        NextState(6);
    }

    if (m_bIsGrounded)
    {
        if (mFallTimer > 0.1f)
        {
            mClass->mSoundLand.Play(&GetMatrix().GetTranslation(), &GetVelocity(), false, true);
        }
        mFallTimer = 0.0f;
    }
    else
    {
        mFallTimer = fDeltaTime + mFallTimer;
    }

    if (mState == Rolling)
    {
        UpdateStateRolling(fDeltaTime, fMove, fTurn);
    }
    else
    {
        EndSprint();
        UpdateState(fDeltaTime, fMove, fStrafe, fTurn);
    }

    if (m_bSprint)
    {
        DroidekaEnergyBar(this)->SpendEnergy(mClass->mEnergyDrainSprint * fDeltaTime);
    }
    else if (mFallTimer < 0.1f)
    {
        float fMultiplier = mCharacter && mCharacter->mMedalsMgr &&
                                    mCharacter->mMedalsMgr->IsAwardAvailable(MedalsMgr::ENDURANCE)
                                ? 1.5f
                                : 1.0f;
        float fRestore = fMultiplier * fDeltaTime;
        if (1.0f > mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z)
        {
            fRestore *= mClass->mEnergyRestoreIdle;
        }
        else
        {
            fRestore *= mClass->mEnergyRestore;
        }
        DroidekaEnergyBar(this)->AddEnergy(fRestore);
    }

    if (VehicleEngine::IsOn())
    {
        if ((mState != Rolling && mState != IdleToRoll) || mState == RollToIdle)
        {
            VehicleEngine::TurnOff(mClass, mMatrix.GetTranslation(), mVelocity);
        }
    }
    else if (mState == Rolling || mState == IdleToRoll)
    {
        float fHealth = 0.0f;
        if (mMaxHealth != 0.0f)
        {
            fHealth = mCurHealth / mMaxHealth;
        }
        VehicleEngine::TurnOn(mClass, mMatrix.GetTranslation(), mVelocity, fHealth);
    }

    if (fDeltaTime > 0.0f && VehicleEngine::IsOn())
    {
        float fSpeed;
        float fAcceleration;
        if (m_bIsGrounded)
        {
            fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z) /
                     mClass->mBallMoveSpeed;
            if (mSpeedPrev > fSpeed)
            {
                fSpeed = (fSpeed - mSpeedPrev) * (fDeltaTime * 4.0f) + mSpeedPrev;
            }
            fAcceleration = (mSpeedPrev - fSpeed) / fDeltaTime;
        }
        else
        {
            fAcceleration = 0.0f;
            fSpeed = (0.5f - mSpeedPrev) * fDeltaTime + mSpeedPrev;
        }
        mSpeedPrev = fSpeed;
        float fHealth = 0.0f;
        if (mMaxHealth != 0.0f)
        {
            fHealth = mCurHealth / mMaxHealth;
        }
        VehicleEngine::Update(mClass, fDeltaTime, mMatrix.GetTranslation(), mVelocity, fSpeed, fAcceleration, fHealth,
                              0.0f, 0.0f, 0.0f, 0.0f, false, false, 0.0f);
    }
    else
    {
        mSpeedPrev = 0.0f;
    }

    if (mClass->mMinPitch > mPitchValue)
    {
        mPitchValue = mClass->mMinPitch;
    }
    else if (mPitchValue > mClass->mMaxPitch)
    {
        mPitchValue = mClass->mMaxPitch;
    }

    if (!mIsDead)
    {
        float fMoveX;
        float fMoveZ;
        float fMoveY = mVelocity.y * fDeltaTime;
        if (mState == Rolling)
        {
            fMoveX = mVelocity.x * fDeltaTime;
            fMoveZ = mVelocity.z * fDeltaTime;
        }
        else
        {
            fMoveX = 0.0f;
            fMoveZ = 0.0f;
        }
        SetPosition(PblVector3(mMatrix.m[3][0] + fMoveX, mMatrix.m[3][1] + fMoveY, mMatrix.m[3][2] + fMoveZ));
        mVelocity.y = fDeltaTime * -9.8f + mVelocity.y;
    }
    m_bIsGrounded = false;
    return true;
}

// SYNTHETIC: SWBF2 0x0049eac0
// EntityDroideka::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x004a98e1
// ??_EEntityDroideka@@WCEA@AEPAXI@Z

// FUNCTION: SWBF2 0x0049f270
EntityDroideka::~EntityDroideka()
{
    int iFirstPerson = -1;
    if ((!NetGame::IsEnabled() || NetGame::IsClient()) &&
        mInitialPlayerId >= 0 && mTracker && mTracker->IsFirstPersonView())
    {
        iFirstPerson = NetGame::GetJoystickIndex(mInitialPlayerId);
    }
    if (iFirstPerson >= 0)
    {
        FirstPerson::Activate(iFirstPerson, false);
        FirstPerson::Update(iFirstPerson, TYPE_DROIDEKA, -2, 0, 0);
    }
    TargetManager::RemoveTarget(this);
    NetGame::Destroyed(this);
    mControllableList.Remove(&this->Controllable::mNode);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        delete mWeapon[i];
        delete mAimer[i];
    }
    TurnOffEffects();
}

// FUNCTION: SWBF2 0x0049ffa0
FoleyFX* EntityDroideka::GetFoleyFX() const
{
    return FoleyFXCollider::GetFoleyFX(0x5e9b9948);
}

// FUNCTION: SWBF2 0x004a01d0
void EntityDroideka::Kill()
{
    mIsAlive = false;
    int iFirstPerson = GetFirstPersonIndex();
    if (iFirstPerson >= 0)
    {
        FirstPerson::Activate(iFirstPerson, false);
    }
    GameObject::Kill();
    Controllable::Kill();
    VehicleEngine::Stop();
}

// FUNCTION: SWBF2 0x004a1ec0
bool EntityDroideka::ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
                                 int iBodyPart, unsigned int uFlags)
{
    if (mPlayerId >= 0)
    {
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0)
        {
            PblVector3 vLocal;
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vLocal), reinterpret_cast<const D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXMATRIX*>(&mMatrix.GetInverseFast()));
            StatusDisplay::Instance(iJoystick)->SetDamageDirection(vLocal);
        }
    }
    return Damageable::ApplyDamage(damage, vDir, vPos, iBodyPart, uFlags);
}

// FUNCTION: SWBF2 0x004a1f50
void EntityDroideka::Die()
{
    mIsDead = true;
    CollisionManager::RemoveBody(this);
    AddStaticToCollisionManager();
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->DeactivateThread();
        }
    }
    if (mClass->mExplosion)
    {
        DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam, mClass, 0, 1.0f);
        mClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
    }
}

// FUNCTION: SWBF2 0x004a6150
bool EntityDroideka::CanUnroll()
{
    if (!m_bIsGrounded)
    {
        return false;
    }

    if (CollisionManager::RayTest(GetCollisionCenter(), PblVector3(0.0f, 1.0f, 0.0f), GetCollisionRadius() * 1.1f,
                                  0, 0, 0x18))
    {
        return false;
    }

    PblVector3 vDir(1.0f, 0.0f, 0.0f);
    const PblVector3& vTrans = mMatrix.GetTranslation();
    PblVector3 vPos(vTrans.x + 0.0f, vTrans.y + 1.5f, vTrans.z + 0.0f);

    {
        if (CollisionManager::RayTest(vPos, vDir, 1.5f, 0, 0, 0x18) &&
            CollisionManager::RayTest(vPos, -vDir, 1.5f, 0, 0, 0x18))
        {
            return false;
        }
    }
    {
        vDir.x = 0.0f;
        vDir.y = 0.0f;
        vDir.z = 1.0f;
        if (CollisionManager::RayTest(vPos, vDir, 1.5f, 0, 0, 0x18) &&
            CollisionManager::RayTest(vPos, -vDir, 1.5f, 0, 0, 0x18))
        {
            return false;
        }
    }
    {
        vDir = PblVector3(0.7f, 0.0f, 0.7f);
        if (CollisionManager::RayTest(vPos, vDir, 1.5f, 0, 0, 0x18) &&
            CollisionManager::RayTest(vPos, -vDir, 1.5f, 0, 0, 0x18))
        {
            return false;
        }
    }
    {
        vDir = PblVector3(-0.7f, 0.0f, 0.7f);
        if (CollisionManager::RayTest(vPos, vDir, 1.5f, 0, 0, 0x18) &&
            CollisionManager::RayTest(vPos, -vDir, 1.5f, 0, 0, 0x18))
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x004a65b0
float EntityDroideka::GetDamageMultiplier(int iBodyId) const
{
    if (mBuffDefenseTimer > 0.0f)
    {
        return GameObject::GetDamageMultiplier(iBodyId) * mBuffDefenseMult;
    }
    return GameObject::GetDamageMultiplier(iBodyId);
}

// FUNCTION: SWBF2 0x004a6850
float EntityDroideka::GetOffenseMultiplier() const
{
    if (mBuffOffenseTimer != 0.0f)
    {
        return mBuffOffenseMult;
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x004a6870
void EntityDroideka::TriggerHealthGainEffect()
{
    if (mClass->mHealthGainEffect && mHealthGainEffectHandle == 0)
    {
        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mHealthGainEffect))
        {
            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mHealthGainEffectHandle = pEffect;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004a6900
void EntityDroideka::TriggerAmmoGainEffect()
{
    if (mClass->mAmmoGainEffect && mAmmoGainEffectHandle == 0)
    {
        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mAmmoGainEffect))
        {
            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mAmmoGainEffectHandle = pEffect;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004a6990
void EntityDroideka::TriggerEnergyGainEffect()
{
    if (mClass->mEnergyGainEffect && mEnergyGainEffectHandle == 0)
    {
        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mEnergyGainEffect))
        {
            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mEnergyGainEffectHandle = pEffect;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004a6e20
void EntityDroideka::TurnOffEffects()
{
    if (mHealthGainEffectHandle)
    {
        mHealthGainEffectHandle->StopAndFinish();
        mHealthGainEffectHandle = NULL;
    }
    if (mAmmoGainEffectHandle)
    {
        mAmmoGainEffectHandle->StopAndFinish();
        mAmmoGainEffectHandle = NULL;
    }
    if (mEnergyGainEffectHandle)
    {
        mEnergyGainEffectHandle->StopAndFinish();
        mEnergyGainEffectHandle = NULL;
    }
    if (mBuffHealthEffectHandle)
    {
        mBuffHealthEffectHandle->StopAndFinish();
        mBuffHealthEffectHandle = NULL;
    }
    if (mBuffOffenseEffectHandle)
    {
        mBuffOffenseEffectHandle->StopAndFinish();
        mBuffOffenseEffectHandle = NULL;
    }
    if (mBuffDefenseEffectHandle)
    {
        mBuffDefenseEffectHandle->StopAndFinish();
        mBuffDefenseEffectHandle = NULL;
    }
    if (mDebuffEffectHandle)
    {
        mDebuffEffectHandle->StopAndFinish();
        mDebuffEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004a70a0
void EntityDroideka::HealthChangeCallback(bool bArg, float fArg)
{
    if (fArg < 0.0f && mBuffHealthTimer <= 0.0f)
    {
        TriggerHealthGainEffect();
    }
    Damageable::HealthChangeCallback(bArg, fArg);
}

// FUNCTION: SWBF2 0x004a0210
void EntityDroideka::Render(unsigned char aLod, float aFadeVal, unsigned int uiRenderFlags)
{
    if (!RenderTrackable(mPlayerId))
    {
        return;
    }

    if (mCallUpdateWalkingGroundNormal)
    {
        UpdateWalkingGroundNormal();
        mCallUpdateWalkingGroundNormal = false;
    }

    if (mClass->m_bDynamicLighting)
    {
        uiRenderFlags &= ~0x80;
    }
    else
    {
        uiRenderFlags |= 0x80;
    }

    PblMatrix smoothedMat = GetSmoothedMatrix();
    if (!netFrameLock && NetGame::IsEnabled())
    {
        gameObjectInterpolator.GetMatrix(mNetUniqueId, smoothedMat);
    }

    if (!aLod)
    {

        bool resetAnim = false;
        float turnRoll = (float)atan2((double)mBallOrientation.mSin, (double)mBallOrientation.mCos);
        if (!netFrameLock && NetGame::IsEnabled())
        {
            if (mState == Rolling || mState == IdleToRoll)
            {

                float f0 = gameObjectInterpolator.GetFloat(mNetUniqueId, 2);
                float frameRoll = gameObjectInterpolator.GetPrevFloat(mNetUniqueId, 2);
                if (frameRoll * turnRoll < 0.0f)
                {
                    float f1 = gameObjectInterpolator.GetCurFloat(mNetUniqueId, 2);
                    if (-1.5707964f > frameRoll)
                    {
                        frameRoll += 6.2831855f;
                        f0 = (f1 - frameRoll) * NetGame::GetTurnRatio() + frameRoll;
                    }
                    else if (frameRoll > 1.5707964f)
                    {
                        frameRoll -= 6.2831855f;
                        f0 = (f1 - frameRoll) * NetGame::GetTurnRatio() + frameRoll;
                    }
                }
                PblAngle frameAngle(f0);
                mBallOrientation = frameAngle;
                resetAnim = true;
            }
            else
            {
                bool isPrev = false;
                float legFloat = gameObjectInterpolator.GetLoopingFloat(
                    mNetUniqueId, 0, mDirection == Backward ? -1.0f : 1.0f, GetStateLimit(), isPrev);
                if (isPrev)
                {
                    int lastState = (int)(gameObjectInterpolator.GetPrevFloat(mNetUniqueId, 1) + 0.5f);
                    if (lastState != mState)
                    {
                        ZephyrAnim* anim = mClass->mAnimations[lastState];
                        if (anim)
                        {
                            mZephyrPoseDyn.SetAnimation(anim, 30.0f);
                            resetAnim = true;
                        }
                    }
                }
                SetFrameState(legFloat);
            }
        }

        BuildWorldMatrices();

        const PblMatrix invMat = smoothedMat.GetInverseFast();
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (mAimer[i])
            {
                mAimer[i]->SetupPose(invMat, mPose, true);
            }
        }

        SetupShadow(this, mModel, 0, uiRenderFlags, 0);
        mModel->Render(0, aFadeVal, &smoothedMat, &mPose, RedColor::WHITE, uiRenderFlags, 0);
        PblMatrix reflectionRenderMat;
        if (!(uiRenderFlags & 0x200000) &&
            FLRenderer::IsReflected(GetRenderPos(), _Sphere.w, reflectionRenderMat, false))
        {
            PblMatrix mat;
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mat), reinterpret_cast<const D3DXMATRIX*>(&smoothedMat), reinterpret_cast<const D3DXMATRIX*>(&reflectionRenderMat));
            reflectionRenderMat = mat;
            mModel->Render(0, aFadeVal, &reflectionRenderMat, &mPose, RedColor::WHITE,
                           (uiRenderFlags & ~0x10000) | 0x10000100, 0);
        }

        FlagItem* flag = GetCarriedFlagItem();
        if (flag)
        {
            const PblMatrix* spine = mPose.Find(flag->GetCarriedDroidekaBoneHash());
            if (spine)
            {
                PblMatrix bone(reinterpret_cast<const _D3DXMATRIXA16*>(spine));
                PblMatrix mat;
                D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mat), reinterpret_cast<const D3DXMATRIX*>(&bone), reinterpret_cast<const D3DXMATRIX*>(&smoothedMat));
                reinterpret_cast<GameObject*>(flag)->SetMatrix(mat);
                flag->RenderHelper(0, aFadeVal, uiRenderFlags);
            }
        }

        if (!netFrameLock && NetGame::IsEnabled())
        {
            if (mState == Rolling || mState == IdleToRoll)
            {
                PblAngle turnAngle(turnRoll);
                mBallOrientation = turnAngle;
            }
            else
            {
                if (resetAnim)
                {
                    ZephyrAnim* anim = mClass->mAnimations[mState];
                    if (anim)
                    {
                        mZephyrPoseDyn.SetAnimation(anim, 30.0f);
                    }
                }
                ResetTurnState();
            }
            BuildWorldMatrices();
        }
    }
    else
    {
        if (mState == Rolling || mState == IdleToRoll)
        {
            PblQuaternion orient;
            orient._fX = 0.0f;
            orient._fY = 0.0f;
            orient._fZ = mBallLean.mSin;
            orient._fS = mBallLean.mCos;
            PblQuaternion roll;
            roll._fX = mBallOrientation.mSin;
            roll._fY = 0.0f;
            roll._fZ = 0.0f;
            roll._fS = mBallOrientation.mCos;
            roll = orient * roll;

            PblVector3 center(0.0f, mClass->mBallRadius, 0.0f);
            PblVector3 rotated = roll.RotateVector(center);
            PblVector3 offset(center.x - rotated.x, center.y - rotated.y, center.z - rotated.z);

            PblQuaternion unit;
            PblMatrix mat;
            D3DXMatrixRotationQuaternion(reinterpret_cast<D3DXMATRIX*>(&mat), D3DXQuaternionNormalize(reinterpret_cast<D3DXQUATERNION*>(&unit), reinterpret_cast<const D3DXQUATERNION*>(&roll)));
            mat.SetTranslation(offset);
            mat *= mMatrix;

            mBallLowRes->Render(aLod, aFadeVal, &mat, &mPose, RedColor::WHITE, uiRenderFlags | 0x1000000, 0);
            FlagItem* flag = GetCarriedFlagItem();
            if (flag)
            {
                const PblMatrix* spine = mPose.Find(flag->GetCarriedDroidekaBoneHash());
                if (spine)
                {
                    PblMatrix bone(reinterpret_cast<const _D3DXMATRIXA16*>(spine));
                    PblMatrix mat2;
                    D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mat2), reinterpret_cast<const D3DXMATRIX*>(&bone), reinterpret_cast<const D3DXMATRIX*>(&mat));
                    reinterpret_cast<GameObject*>(flag)->SetMatrix(mat2);
                    flag->RenderHelper(aLod, aFadeVal, uiRenderFlags);
                }
            }
        }
        else
        {
            mUprightLowRes->Render(aLod, aFadeVal, &mMatrix, &mPose, RedColor::WHITE, uiRenderFlags | 0x1000000,
                                   0);
            FlagItem* flag = GetCarriedFlagItem();
            if (flag)
            {
                const PblMatrix* spine = mPose.Find(flag->GetCarriedDroidekaBoneHash());
                if (spine)
                {
                    PblMatrix bone(reinterpret_cast<const _D3DXMATRIXA16*>(spine));
                    PblMatrix mat2;
                    D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mat2), reinterpret_cast<const D3DXMATRIX*>(&bone), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
                    reinterpret_cast<GameObject*>(flag)->SetMatrix(mat2);
                    flag->RenderHelper(aLod, aFadeVal, uiRenderFlags);
                }
            }
        }
    }

    if (!mIsDead || mState == Dead)
    {
        return;
    }

    if (!mClass->mNoDeathExplosions)
    {
        mDeathExplosionTime += GameLoop::GetDeltaTime();
        if (mDeathExplosionTime > 0.4f)
        {
            mDeathExplosionTime = 0.0f;
            FLEffectClass* effect = 0;
            switch (PblRandom::GetGlobalInt() % 3)
            {
            case 0:
                effect = FLEffect::s_EffectClasses.Find(0x02BB1FE0);
                break;
            case 1:
                effect = FLEffect::s_EffectClasses.Find(0xE07F39B5);
                break;
            case 2:
                effect = FLEffect::s_EffectClasses.Find(0x9DB69EE4);
                break;
            }
            if (effect)
            {
                FLEffectObject* particleObj = effect->BuildEffect();
                if (particleObj)
                {
                    unsigned int parentName = mCollisionModel->GetParentNameFromMaskId(
                        PblRandom::GetGlobalInt() % mCollisionModel->GetNumBodies());
                    const PblMatrix* spine = GetPose()->Find(parentName);

                    if (spine)
                    {
                        particleObj->AttachEffectToObject(this, spine, true);
                    }
                    else
                    {
                        particleObj->AttachEffectToMatrix(&mMatrix, false);
                    }
                }
            }
        }
    }

    mDeathTime += GameLoop::GetDeltaTime();
    if (!mDoneDustCloud && mDeathTime > mClass->mDeathDustDelay)
    {
        mDoneDustCloud = true;
        if (mClass->mDeathDustClassHash && !mClass->mDeathDustEffect)
        {
            mClass->mDeathDustEffect = FLEffect::Get(mClass->mDeathDustClassHash);
        }
        if (mClass->mDeathDustEffect)
        {
            FLEffectObject* particleObj = mClass->mDeathDustEffect->BuildEffect();
            if (particleObj)
            {
                PblMatrix mat2 = gMatrixIdentity;
                mat2.SetTranslation(mClass->mDeathDustOffset);
                PblMatrix mat;
                D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mat), reinterpret_cast<const D3DXMATRIX*>(&mat2), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
                mat2 = mat;
                particleObj->AttachEffectToMatrix(&mat2, false);
            }
        }
    }
}


