#include <d3dx9math.h>
#include <new>
#include <stdio.h>
#include <string.h>

#include "EntityCarrier.h"
#include "CommandFlyer.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "Character.h"
#include "CollisionObject.h"
#include "EntitySquadron.h"
#include "FirstPerson.h"
#include "Tracker.h"
#include "NetGame.h"
#include "WeaponCannonInstance.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "CollisionManager.h"
#include "GameLoop.h"
#include "MotionBlurEffect.h"
#include "Trackable.h"
#include <float.h>
float GetDefaultFOV();

// FUNCTION: SWBF2 0x004b3dd0
CameraTrackSetting* EntityFlyer::GetCameraTrackSettings()
{
    return &mClass->mCameraSettings;
}

// FUNCTION: SWBF2 0x004b5f00
void EntityFlyer::SetupCameraZoomFactor(float& rZoom)
{
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0)
    {
        Weapon* pWeapon = mWeapon[mWeaponIndex[0]];
        if (pWeapon)
        {
            rZoom = pWeapon->mZoomTurnScale;
            return;
        }
    }
    rZoom = 1.0f;
}



// SYNTHETIC: SWBF2 0x00496ea0
// EntityCarrier::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00497941
// ??_EEntityCarrier@@WCEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0049794c
// ??_EEntityCarrier@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497962
// ??_EEntityCarrier@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0049796d
// ??_EEntityCarrier@@WDMM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497978
// ??_EEntityCarrier@@WCFI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497983
// ??_EEntityCarrier@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0049798b
// ??_EEntityCarrier@@WCAA@AEPAXI@Z

// FUNCTION: SWBF2 0x00497060
EntityCarrier::~EntityCarrier()
{
}

// SYNTHETIC: SWBF2 0x00478579
// ?GetGameObject@EntityFlyer@@WBEA@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004785ad
// ?GetGameObject@EntityFlyer@@WM@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004785bd
// ?GetEntity@EntityFlyer@@WM@BEPBVEntity@@XZ
// SYNTHETIC: SWBF2 0x004785db
// ?GetControllable@EntityFlyer@@WCFI@AEPAVControllable@@XZ
// SYNTHETIC: SWBF2 0x004785e6
// ?GetControllable@EntityFlyer@@WCFI@BEPBVControllable@@XZ
// SYNTHETIC: SWBF2 0x004785fc
// ?GetMatrix@EntityFlyer@@WDMA@BEABVPblMatrix@@XZ

// FUNCTION: SWBF2 0x00496f60
EntityCarrier::EntityCarrier(EntityCarrierClass* pClass, const EntityDesc& desc)
    : EntityFlyer(pClass, desc)
{
    memset(mCargoArray, 0, sizeof(mCargoArray));
    mCargoCount = pClass->mCargoCount;
    static_cast<VehicleEngine*>(this)->TurnOn(
        mClass ? static_cast<VehicleEngineClass*>(mClass) : 0,
        mMatrix.GetTranslation(), mVelocity, 1.0f);
}

// SYNTHETIC: SWBF2 0x00497957
// ?Kill@EntityCarrier@@WBAA@AEXXZ

// FUNCTION: SWBF2 0x00497110
void EntityCarrier::Kill()
{
    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iPlayer = NetGame::GetJoystickIndex(mPlayerId);
        if (iPlayer >= 0)
        {
            FirstPerson::Activate(iPlayer, false);
            FirstPerson::Update(iPlayer, TYPE_COCKPIT, -2, 0, 0);
        }
    }
    for (int i = 0; i < mCargoCount; ++i)
    {
        GameObject* pObject = mCargoArray[i].mObject;
        if (pObject)
        {
            Controllable* pControllable = pObject->GetControllable();
            if (pControllable)
            {
                pControllable->Kill();
            }
            DetachCargo(i);
        }
    }
    EntityFlyer::Kill();
}

// FUNCTION: SWBF2 0x004971d0
bool EntityCarrier::Update(float fDeltaTime)
{
    if (!EntityFlyer::Update(fDeltaTime))
    {
        return false;
    }

    for (int i = 0; i < mCargoCount; ++i)
    {
        GameObject* pObject = mCargoArray[i].mObject;
        if (!pObject)
        {
            continue;
        }

        PblMatrix matrix = mMatrix;
        PblVector3 position;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&position), reinterpret_cast<const D3DXVECTOR3*>(&mCargoArray[i].mOffset), reinterpret_cast<const D3DXMATRIX*>(&matrix));
        matrix.SetTranslation(position);
        pObject->SetMatrix(matrix);
        pObject->SetVelocity(mVelocity);
    }
    return true;
}


namespace
{

    inline void EntityFlyerUpdateCollisionSphere(EntityFlyer* pFlyer)
    {
        const PblMatrix& matrix = pFlyer->mMatrix;
        const GameModel* pModel = pFlyer->mModel;
        PblVector3 center;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&center), reinterpret_cast<const D3DXVECTOR3*>(&pModel->_mModel->_BoundingSphere.GetCenter()), reinterpret_cast<const D3DXMATRIX*>(&matrix));
        CollisionObject* pCollision = static_cast<CollisionObject*>(pFlyer);
        pCollision->SetCollisionCenter(center);
        pCollision->RecalculateAABB();
    }
}

// FUNCTION: SWBF2 0x00477720
void EntityFlyer::SetPosition(const PblVector3& vPos)
{
    PblMatrix& matrix = mMatrix;
    matrix.SetTranslation(vPos);
    EntityFlyerUpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x004777c0
void EntityFlyer::SetMatrix(const PblMatrix& mMatrix)
{
    this->mMatrix = mMatrix;
    EntityFlyerUpdateCollisionSphere(this);
}



// FUNCTION: SWBF2 0x004778d0
Trackable* EntityFlyer::GetTrackable()
{
    return this;
}

// FUNCTION: SWBF2 0x004779a0
unsigned int EntityFlyer::GetAllegiance()
{
    const Character* pCharacter = mCharacter;
    return pCharacter ? pCharacter->mTeamNumber : 0;
}

// FUNCTION: SWBF2 0x004779c0
bool EntityFlyer::CanJoinSquadron() const
{
    return !EntitySquadron::InSquadron() && GetFollowingPath();
}

// FUNCTION: SWBF2 0x00477b00
int EntityFlyerClass::GetVehicleType() const
{
    return mVehicleType;
}

// FUNCTION: SWBF2 0x00477b10
int EntityFlyerClass::GetWeaponCount() const
{
    return mWeaponCount;
}

// FUNCTION: SWBF2 0x00477b20
const WeaponClass* EntityFlyerClass::GetWeaponClass(int iIndex) const
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x00477b40
float EntityFlyerClass::GetTimeRequiredToEject() const
{
    return mTimeRequiredToEject;
}

// FUNCTION: SWBF2 0x00477b50
float EntityFlyerClass::GetReboardTime() const
{
    return mTimeTilReboard;
}

// FUNCTION: SWBF2 0x00477b60
const ControllableClass* EntityFlyerClass::GetControllableClass() const
{
    return this ? static_cast<const ControllableClass*>(this) : 0;
}

// SYNTHETIC: SWBF2 0x00402960
// `dynamic initializer for 'EntityCarrier::rttiHashEntityCarrier''
// EntityCarrier::rttiHashEntityCarrier

// GLOBAL: SWBF2 0x01ebb8ec
// EntityCarrier::rttiHashEntityCarrier
PblHash EntityCarrier::rttiHashEntityCarrier("EntityCarrier");
// GLOBAL: SWBF2 0x007890b4
// EntityCarrier::rttiNameEntityCarrier
const char EntityCarrier::rttiNameEntityCarrier[] = "EntityCarrier";

// SYNTHETIC: SWBF2 0x00402940
// `dynamic initializer for 'EntityCarrier::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f10
// `dynamic atexit destructor for 'EntityCarrier::sMemoryPool''

// GLOBAL: SWBF2 0x01ebb8f0
// EntityCarrier::sMemoryPool
MemoryPool EntityCarrier::sMemoryPool("EntityCarrier", 0x1df0);







// SYNTHETIC: SWBF2 0x00402970
// `dynamic initializer for 'EntityCarrierClass::rttiHashEntityCarrierClass''
// EntityCarrierClass::rttiHashEntityCarrierClass

// GLOBAL: SWBF2 0x01ebb8e8
// EntityCarrierClass::rttiHashEntityCarrierClass
PblHash EntityCarrierClass::rttiHashEntityCarrierClass("EntityCarrierClass");
// GLOBAL: SWBF2 0x007890a0
// EntityCarrierClass::rttiNameEntityCarrierClass
const char EntityCarrierClass::rttiNameEntityCarrierClass[] = "EntityCarrierClass";







// FUNCTION: SWBF2 0x00497300
bool EntityCarrier::AttachCargo(int, GameObject* pGameObject)
{
    if (mCargoArray[0].mObject)
    {
        return false;
    }

    EntityCarrierClass* pClass = GetClass();
    mCargoArray[0].mOffset = pClass->mCargoInfo[0].mOffset - pGameObject->GetAttachOffset();
    mCargoArray[0].mObject = pGameObject;
    mCargoArray[0].mObject->mParent = static_cast<CollisionObject*>(this);

    UpdateLandedHeight();

    GetClass()->mSoundCargoPickup.Play(&mMatrix.GetTranslation(), &GetVelocity(), true, true);
    return true;
}

// FUNCTION: SWBF2 0x00497410
bool EntityCarrier::DetachCargo(int iSlot)
{
    if (!mCargoArray[iSlot].mObject)
    {
        return false;
    }

    mCargoArray[iSlot].mObject->mParent = 0;
    mCargoArray[iSlot].mObject = 0;

    UpdateLandedHeight();

    GetClass()->mSoundCargoDropoff.Play(&mMatrix.GetTranslation(), &GetVelocity(), true, true);
    return true;
}

// FUNCTION: SWBF2 0x004974b0
void EntityCarrier::UpdateLandedHeight()
{
    mLandedHeight = GetClass()->mLandedHeight;

    for (int i = 0; i < mCargoCount; ++i)
    {
        CargoSlot* pSlot = &mCargoArray[i];
        if (!pSlot->mObject.mObject)
        {
            continue;
        }

        if (pSlot->mObject.mObject->mHandleId == pSlot->mObject.mSavedHandleId)
        {
            const GameModel* pModel = pSlot->mObject.mObject->GetModel();
            if (pModel)
            {
                float fCandidate = -pSlot->mOffset.y - pModel->_mModel->_TotalBoundingBox._vMin.y;
                mLandedHeight = mLandedHeight > fCandidate ? mLandedHeight : fCandidate;
            }
        }
        else
        {
            pSlot->mObject.mObject = 0;
            pSlot->mObject.mSavedHandleId = 0;
        }
    }
}

// FUNCTION: SWBF2 0x00497560
void EntityCarrier::ActivatePhysics()
{
    ActivateThread(-1, PER_TURN);
}

// FUNCTION: SWBF2 0x00497580
EntityCarrierClass::EntityCarrierClass(unsigned int uHash)
    : EntityFlyerClass(uHash)
{
    mPilotType = PILOT_NONE;

    memset(mCargoInfo, 0, sizeof(mCargoInfo));

    mCargoCount = 0;
}

// SYNTHETIC: SWBF2 0x004975e0
// EntityCarrierClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004976a0
EntityCarrierClass::~EntityCarrierClass()
{
}

// SYNTHETIC: SWBF2 0x004780b0
// CommandFlyerClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00478160
CommandFlyerClass::~CommandFlyerClass()
{
}

// SYNTHETIC: SWBF2 0x004785f1
// ??_ECommandFlyerClass@@WBAKA@AEPAXI@Z

// FUNCTION: SWBF2 0x00497610
EntityCarrierClass::EntityCarrierClass(EntityCarrierClass* pSrc, unsigned int uHash)
    : EntityFlyerClass(pSrc, uHash)
{
    memcpy(mCargoInfo, pSrc->mCargoInfo, sizeof(mCargoInfo));
    mCargoCount = pSrc->mCargoCount;
}

// FUNCTION: SWBF2 0x004976b0
void EntityCarrierClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x759C8F54u:
        mSoundCargoPickup.SetID(pszValue, true, false);
        break;

    case 0x3E2C4DA4u:
        mCargoInfo[mCargoCount].mHash = PblTEMPHash(pszValue);

        if (mModel)
        {
            const RedSkeleton* pSkeleton = mModel->GetRedModel(0)->GetSkeleton();
            if (pSkeleton)
            {
                for (int i = 0; i < 128; ++i)
                {
                    if (pSkeleton->GetBoneNameHash(i) == mCargoInfo[mCargoCount].mHash)
                    {

                        const PblMatrix& mBone = pSkeleton->GetDefaultPoseMatrix(i);
                        mCargoInfo[mCargoCount].mOffset = mBone.GetTranslation();
                        ++mCargoCount;
                        break;
                    }
                }
            }
        }
        break;

    case 0x8897DB28u:
        mSoundCargoDropoff.SetID(pszValue, true, false);
        break;

    default:
        EntityFlyerClass::SetProperty(uProperty, pszValue);
        break;

    case 0x910A89FCu:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mCargoInfo[mCargoCount].mHash = 0;
        PblVector3& vOffset = mCargoInfo[mCargoCount].mOffset;
        vOffset.x = x;
        vOffset.y = y;
        vOffset.z = z;
        ++mCargoCount;
        break;
    }
    }
}

// FUNCTION: SWBF2 0x00497830
EntityClass* EntityCarrierClass::Derive(unsigned int uHash)
{
    return (EntityClass*)new EntityCarrierClass(this, uHash);
}




// FUNCTION: SWBF2 0x004978a0
Entity* EntityCarrierClass::Build(const EntityDesc& desc)
{
    if (EntityCarrier::sMemoryPool.IsFull())
    {
        return 0;
    }

    EntityCarrier* pCarrier = new EntityCarrier(this, desc);
    if (pCarrier)
    {
        (static_cast<CollisionObject*>(pCarrier))->AddRigidToCollisionManager();
        pCarrier->ActivatePhysics();
        static_cast<EntityGeometry*>(pCarrier)->BuildEffects();
    }
    return (Entity*)pCarrier;
}

inline void* EntityCarrier::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}


PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace FLRenderer
{
unsigned int GetViewportCount();
}

// FUNCTION: SWBF2 0x004b5010
void EntityFlyer::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    if (mClass->IsGuidedMissile() &&
        (!mPilot || mEnergyBar.GetEnergy() <= 0.0f || mCrashTimer != 0.0f))
    {
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0)
        {
            ChaseCamera* pCamera = CameraManager::sInstance->mChaseCamera[iJoystick];
            if (pCamera)
            {
                pCamera->ClearShake();
            }
        }
        PblVector3 vCamera = rMatrix.GetTranslation();
        const PblMatrix& mBody = GetMatrix();
        PblVector3 vBack(-mBody.m[2][0], -mBody.m[2][1], -mBody.m[2][2]);
        const PblVector3& vCenter = GetCollisionCenter();
        PblVector3 vOffset(vCamera.x - vCenter.x, vCamera.y - vCenter.y, vCamera.z - vCenter.z);
        if (vOffset.y * vBack.y + vOffset.x * vBack.x + vOffset.z * vBack.z -
                mClass->mGuidedMissileDeathCamMinDistance < 0.0f)
        {
            float fPullback = mClass->mGuidedMissileDeathCamPullbackSpeed * fDeltaTime;
            const PblMatrix& mNow = GetMatrix();
            PblVector3 vDir(-mNow.m[2][0], -mNow.m[2][1], -mNow.m[2][2]);
            const GameObject* pIgnore = GetGameObject();
            CollisionObject* pHit;
            if (CollisionManager::RayHit(vCamera, vDir, fPullback, pHit, 0, &pIgnore, 1, 0x90, false) >= 1.0f)
            {
                rMatrix.m[3][0] = vDir.x * fPullback + vCamera.x;
                rMatrix.m[3][1] = vDir.y * fPullback + vCamera.y;
                rMatrix.m[3][2] = vDir.z * fPullback + vCamera.z;
                rMatrix.m[3][3] = 1.0f;
            }
        }
        return;
    }

    const PblMatrix& mTarget = GetSmoothedMatrix();
    if (IsAlive() && (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())))
    {
        mTrickCameraInterpDir = 0.0f;
        mTrickCameraInterpDetach = 0.0f;
        const PblVector3 vBack(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z);
        rMatrix = DirectionMatrix(vBack, mTarget.GetYAxis(), mEyePoint);
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateCamera(iFirstPerson, rMatrix);
        }
    }
    else
    {
        PblMatrix mUnbank;
        mUnbank.SetRotateZ(-mBankRollAngle);
        mUnbank.RotateX(-mBankPitchAngle);
        PblMatrix mBody;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mBody), reinterpret_cast<const D3DXMATRIX*>(&mUnbank), reinterpret_cast<const D3DXMATRIX*>(&mTarget));

        if (mTowCableWalker != 0)
        {
            CameraTrackSetting& rSettings = mClass->mCameraSettings;
            const PblVector3 vSavedOffset = rSettings.mCameraTrackOffset;
            PblVector3 vPos = mBody.GetTranslation();
            const PblVector3& vWalker = mTowCableWalker->GetCollisionObject()->GetPosition();
            PblVector3 vToCamera(vPos.x - vWalker.x, vPos.y - vWalker.y, vPos.z - vWalker.z);
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vPos), reinterpret_cast<const D3DXVECTOR3*>(&vToCamera));
            const float fSide = vPos.x * mTarget.m[0][0] + vPos.y * mTarget.m[0][1] + vPos.z * mTarget.m[0][2];
            rSettings.mCameraTrackOffset.y = 0.5f;
            rSettings.mCameraTrackOffset.z = 8.0f;
            rSettings.mCameraTrackOffset.x = fSide > 0.0f ? 2.0f : -2.0f;
            const float fRadius = GetCollisionRadius();
            rSettings.mCameraTrackOffset.x *= fRadius;
            rSettings.mCameraTrackOffset.y = fRadius * rSettings.mCameraTrackOffset.y;
            rSettings.mCameraTrackOffset.z = fRadius * rSettings.mCameraTrackOffset.z;
            rSettings.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime, mBody, mEyeDir, 1.0f, 1.0f, this);
            rSettings.mCameraTrackOffset = vSavedOffset;
            mTrickCameraInterpDir = 0.0f;
            mTrickCameraInterpDetach = 0.0f;
        }
        else
        {
            CameraTrackSetting settings = mClass->mCameraSettings;
            if (mInFlip)
            {
                mTrickCameraInterpDir = 1.0f;
                mTrickCameraDetach = 0.2f;
                mTrickCameraInterpDetach = 1.0f;
            }
            else if (mSideRoll.x != 0.0f || mSideRoll.y != 0.0f)
            {
                mTrickCameraDetach = 0.48f;
                mTrickCameraInterpDetach = 1.0f;
            }
            else if (mState == CRASHING || mState == CRASHED)
            {
                mTrickCameraDetach = 0.25f;
                mTrickCameraInterpDetach = 1.0f;
            }

            if (mTrickCameraInterpDir > 0.0f)
            {
                settings.mCameraAimTension = (1.0f - mTrickCameraInterpDir) * 50.0f + mTrickCameraInterpDir * 20.0f;
            }
            else
            {
                settings.mCameraAimTension = FLT_MAX;
            }
            settings.mCameraTiltValue = 0.087665f;
            if (FLRenderer::GetViewportCount() == 2)
            {
                settings.mCameraMoveTensionU *= 1.5f;
                settings.mCameraMoveTensionD *= 1.5f;
                settings.mCameraTrackCenter.y *= 0.6666667f;
                settings.mCameraTrackOffset.y *= 0.6666667f;
            }

            const float fDetach = mTrickCameraDetach * mTrickCameraInterpDetach + (1.0f - mTrickCameraInterpDetach);
            if (fDetach < 0.99f)
            {
                int iJoystick = GetJoystickIndex();
                ChaseCamera* pCamera = iJoystick >= 0 ? CameraManager::sInstance->mChaseCamera[iJoystick] : 0;
                if (pCamera)
                {
                    pCamera->mTimer = fDetach;
                }
                if (NetGame::IsEnabled())
                {
                    for (unsigned int i = 0; i < GameLoop::GetNumCameras(); ++i)
                    {
                        if (NetGame::GetSpectator(i) == mPlayerId)
                        {
                            ChaseCamera* pSpectator = CameraManager::sInstance->mChaseCamera[i];
                            if (pSpectator)
                            {
                                pSpectator->mTimer = fDetach;
                            }
                        }
                    }
                }
            }

            const PblVector3 vAhead(0.0f, 0.0f, 5.0f);
            PblVector3 vAim;
            if (mTrickCameraInterpDir > 0.0f)
            {
                PblVector3 vPoint;
                D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPoint), reinterpret_cast<const D3DXVECTOR3*>(&vAhead), reinterpret_cast<const D3DXMATRIX*>(&mBody));
                PblVector3 vToPoint(vPoint.x - rMatrix.m[3][0], vPoint.y - rMatrix.m[3][1],
                                    vPoint.z - rMatrix.m[3][2]);
                PblVector3 vNormal;
                D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNormal), reinterpret_cast<const D3DXVECTOR3*>(&vToPoint));
                vAim = vNormal;
            }
            else
            {
                vAim = mEyeDir;
            }

            if (mGetSpeedSpeed > 0.0001f)
            {
                settings.SetupAmbientShift(mBody, mAmbientShift, mAmbientShiftTarget, mAmbientShiftSpeed,
                                           mAmbientShiftSpeedMax, fDeltaTime);
            }
            else
            {
                mAmbientShift.x = 0.0f;
                mAmbientShift.y = 0.0f;
                mAmbientShift.z = 0.0f;
            }

            const float fInterp = mTrickCameraInterpDir;
            const PblVector3 vBlendAim(vAim.x * fInterp + mEyeDir.x * (1.0f - fInterp),
                                       vAim.y * fInterp + mEyeDir.y * (1.0f - fInterp),
                                       vAim.z * fInterp + mEyeDir.z * (1.0f - fInterp));
            settings.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime, mBody, vBlendAim, 1.0f, 1.0f, this);

            const float fDecay = fDeltaTime * 0.9f;
            mTrickCameraInterpDir -= fDecay;
            if (mTrickCameraInterpDir < 0.0f)
            {
                mTrickCameraInterpDir = 0.0f;
            }
            mTrickCameraInterpDetach -= fDecay;
            if (mTrickCameraInterpDetach < 0.0f)
            {
                mTrickCameraInterpDetach = 0.0f;
            }
        }
    }

    if (!NetGame::IsEnabled())
    {
        const float fBlurStart = mClass->mBlurStartV;
        const float fBlurEnd = mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                                 : mClass->mBoostFlyerSpeed;
        const float fSpeed = mGetSpeedSpeed;
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0 && fSpeed > fBlurStart)
        {
            float fBlend = (fSpeed - fBlurStart) / (fBlurEnd - fBlurStart);
            if (fBlend < 0.0f)
            {
                fBlend = 0.0f;
            }
            else if (fBlend > 1.0f)
            {
                fBlend = 1.0f;
            }
            if (mClass->mBlurEffect * fBlend > 0.0f)
            {
                MotionBlurEffect::Start(0, mClass->mBlurEffect * fBlend, iJoystick);
            }
        }
    }
}




