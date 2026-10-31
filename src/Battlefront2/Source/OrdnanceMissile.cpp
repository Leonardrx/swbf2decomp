#include "OrdnanceMissile.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9math.h>

#include "AIUtil.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "EntityCarrier.h"
#include "FLEffect.h"
#include "GameObject.h"
#include "GameLoop.h"
#include "UnitController.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

class Entity;
class WeaponClass;

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

extern PblVector3 gVectorZero;

// SYNTHETIC: SWBF2 0x00405b40
// `dynamic initializer for 'OrdnanceMissile::rttiHashOrdnanceMissile''
// OrdnanceMissile::rttiHashOrdnanceMissile

// GLOBAL: SWBF2 0x01ef265c
// OrdnanceMissile::rttiHashOrdnanceMissile
PblHash OrdnanceMissile::rttiHashOrdnanceMissile("OrdnanceMissile");
// GLOBAL: SWBF2 0x0078a9ac
// OrdnanceMissile::rttiNameOrdnanceMissile
const char OrdnanceMissile::rttiNameOrdnanceMissile[] = "OrdnanceMissile";







inline void Controllable::SetEnemyLockedOnMeState(float fState, OrdnanceMissile* pMissile, float fDistance)
{
    mEnemyLockedOnMeState = fState;
    mEnemyLockedOnMeTimestamp = GameLoop::GetMissionTime();
    if (pMissile != mEnemyLockedOnMeMissile)
    {
        if (!mEnemyLockedOnMeMissile || mEnemyLockedOnMeDistance > fDistance)
        {
            mEnemyLockedOnMeMissile = pMissile;
            mEnemyLockedOnMeDistance = fDistance;
        }
    }
    else
    {
        mEnemyLockedOnMeDistance = fDistance;
    }
}

static inline float Distance(const PblVector3& a, const PblVector3& b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return sqrtf(dy * dy + dx * dx + dz * dz);
}

// FUNCTION: SWBF2 0x006049b0
OrdnanceMissile::OrdnanceMissile(OrdnanceMissileClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc),
      mTarget(desc.mTarget),
      mBodyID(desc.mBodyID),
      mWaverPitch(PblRandom::GetSim().GetFloat() * 6.2831855f),
      mWaverYaw(PblRandom::GetSim().GetFloat() * 6.2831855f)
{
    OrdnanceMissileClass* pMissileClass = (OrdnanceMissileClass*)mClass;
    mTargetDir = desc.mDirection;
    mVelocityDir = desc.mDirection;
    if (pMissileClass->mAcceleration > 9000.0f)
    {
        mSpeed = pMissileClass->mVelocity;
    }
    else
    {
        mSpeed = pMissileClass->mMinSpeed;
    }
    mVelocity = mVelocityDir * mSpeed;
    mGoalSpeed = pMissileClass->mVelocity;
    mScale = 0.0f;

    if (mTarget)
    {
        mLockedOnSoundCtrl.Play(&pMissileClass->mLockedOnSound, &mPosition, &mVelocity, true);
        for (PblList<Controllable>::Itor it(mTarget->GetControllableList()); it; ++it)
        {
            if (it->GetEnemyLockedOnMeState() < 2.0f || it->GetEnemyLockedOnMeState() > 0.2f)
            {
                it->SetEnemyLockedOnMeState(2.0f, this, Distance(mTarget->GetCollisionCenter(), mPosition));
            }
        }
    }
}

// FUNCTION: SWBF2 0x00604d60
OrdnanceMissile::~OrdnanceMissile()
{
    if (mLockedOnSoundCtrl.mFlags & GameSoundControllable::flags_playing)
    {
        mLockedOnSoundCtrl.Stop(true);
    }
    if (mTarget)
    {
        for (PblList<Controllable>::Itor it(mTarget->GetControllableList()); it; ++it)
        {
            Controllable* pControllable = it;
            if (pControllable->mEnemyLockedOnMeState > 0.2f)
            {
                pControllable->mEnemyLockedOnMeState = 0.0f;
                pControllable->mEnemyLockedOnMeTimestamp = GameLoop::GetMissionTime();
                pControllable->mEnemyLockedOnMeDistance = FLT_MAX;
            }
        }
    }
}

// SYNTHETIC: SWBF2 0x00605b26
// ??_EOrdnanceMissile@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00605b2e
// ??_EOrdnanceMissile@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00604920
// OrdnanceMissile::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00604ea0
bool OrdnanceMissile::Update(float fDeltaTime)
{
    const OrdnanceMissileClass* pClass = (OrdnanceMissileClass*)mClass;

    const float fScale = pClass->mScaleRate * fDeltaTime + mScale;
    mScale = fScale < 1.0f ? fScale : 1.0f;

    const OrdnanceMissileClass* pNext = (OrdnanceMissileClass*)pClass->mNextCharge;
    const float fTurnRate = ((pNext->mTurnRate - pClass->mTurnRate) * mRatio + pClass->mTurnRate) * mScale;
    if (fTurnRate > 0.0f)
    {
        const GameObject* pTarget = mTarget;
        if (pTarget)
        {
            PblVector3 vAim = pTarget->GetTargetPoint(mPosition, mVelocityDir, mBodyID) +
                              pTarget->GetTargetPointMissVector();
            if (mOwner)
            {
                if (UnitControllerPtr(mOwner->GetControllable()) &&
                    !pTarget->IsRtti(EntityFlyer::rttiHashEntityFlyer))
                {
                    vAim += AIUtil::CalculateBubbleOffset(mOwner->GetControllable(), vAim, pTarget,
                                                          mOwner->GetControllable()->GetCurWpn()->mClass);
                }
            }

            mTargetDir = Normalize(vAim - mPosition);

            for (PblList<Controllable>::Itor it(mTarget->GetControllableList()); it; ++it)
            {
                if (it->GetEnemyLockedOnMeState() < 2.0f || it->GetEnemyLockedOnMeState() > 0.2f)
                {
                    it->SetEnemyLockedOnMeState(2.0f, this, Distance(static_cast<const CollisionObject*>(pTarget)->GetCollisionCenter(), mPosition));
                }
            }
        }

        PblVector3 vAxis = CrossProduct(mVelocityDir, mTargetDir);
        const float fLengthSq = vAxis.y * vAxis.y + vAxis.x * vAxis.x + vAxis.z * vAxis.z;
        if (fLengthSq > 1.0e-8f)
        {
            const float fLength = sqrtf(fLengthSq);
            const float fInv = 1.0f / fLength;
            vAxis *= fInv;
            const float fMaxTurn = fTurnRate * fDeltaTime;
            D3DXMATRIX mRotation;
            D3DXMatrixRotationAxis(&mRotation, (const D3DXVECTOR3*)&vAxis,
                                   fMaxTurn > fLength ? fLength : fMaxTurn);
            PblVector3 vDir;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocityDir, &mRotation);
            mVelocityDir = vDir;
        }
    }

    const OrdnanceMissileClass* pWaverClass = (OrdnanceMissileClass*)mClass;
    const OrdnanceMissileClass* pWaverNext = (OrdnanceMissileClass*)pWaverClass->mNextCharge;
    const float fWaverTurn =
        ((pWaverNext->mWaverTurn - pWaverClass->mWaverTurn) * mRatio + pWaverClass->mWaverTurn) * mScale;
    if (fWaverTurn > 0.0f)
    {
        PblVector3 vAxis;
        {
            const PblMatrix mDir = DirectionMatrix(mVelocityDir, gVectorUnitY, gVectorZero);
            const float fSinYaw = PblMath::Sin(mWaverYaw);
            const PblVector3 vUp(mDir.m[1][0] * fSinYaw, mDir.m[1][1] * fSinYaw, mDir.m[1][2] * fSinYaw);
            const float fSinPitch = PblMath::Sin(mWaverPitch);
            vAxis = PblVector3(mDir.m[0][0] * fSinPitch + vUp.x, mDir.m[0][1] * fSinPitch + vUp.y,
                               mDir.m[0][2] * fSinPitch + vUp.z);
        }
        const float fLengthSq = vAxis.y * vAxis.y + vAxis.x * vAxis.x + vAxis.z * vAxis.z;
        if (fLengthSq > 1.0e-8f)
        {
            const float fLength = sqrtf(fLengthSq);
            const float fInv = 1.0f / fLength;
            vAxis *= fInv;
            D3DXMATRIX mRotation;
            D3DXMatrixRotationAxis(&mRotation, (const D3DXVECTOR3*)&vAxis, fWaverTurn * fDeltaTime * fLength);
            PblVector3 vDir;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocityDir, &mRotation);
            mVelocityDir = vDir;
        }

        const OrdnanceMissileClass* pRateClass = (OrdnanceMissileClass*)mClass;
        const OrdnanceMissileClass* pRateNext = (OrdnanceMissileClass*)pRateClass->mNextCharge;
        const float fWaverRate =
            ((pRateNext->mWaverRate - pRateClass->mWaverRate) * mRatio + pRateClass->mWaverRate) * fDeltaTime * 2.0f;
        mWaverPitch += PblRandom::GetSim().GetFloat() * fWaverRate;
        mWaverYaw += PblRandom::GetSim().GetFloat() * fWaverRate;
    }

    mSpeed += ((OrdnanceMissileClass*)mClass)->mAcceleration * fDeltaTime;
    if (mSpeed > mGoalSpeed)
    {
        mSpeed = mGoalSpeed;
    }

    mVelocityDir = Normalize(mVelocityDir);
    mVelocity = mVelocityDir * mSpeed;

    if (mLockedOnSoundCtrl.mFlags & GameSoundControllable::flags_playing)
    {
        mLockedOnSoundCtrl.Update(&((OrdnanceMissileClass*)mClass)->mLockedOnSound, &mPosition, &mVelocity);
    }

    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->SetOrientation(mVelocity);
    }

    return OrdnanceBullet::Update(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x00405b50
// `dynamic initializer for 'OrdnanceMissileClass::rttiHashOrdnanceMissileClass''
// OrdnanceMissileClass::rttiHashOrdnanceMissileClass

// GLOBAL: SWBF2 0x01ef2658
// OrdnanceMissileClass::rttiHashOrdnanceMissileClass
PblHash OrdnanceMissileClass::rttiHashOrdnanceMissileClass("OrdnanceMissileClass");
// GLOBAL: SWBF2 0x0078a990
// OrdnanceMissileClass::rttiNameOrdnanceMissileClass
const char OrdnanceMissileClass::rttiNameOrdnanceMissileClass[] = "OrdnanceMissileClass";

// FUNCTION: SWBF2 0x00605790
OrdnanceMissileClass::OrdnanceMissileClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mLifeSpan = 3.0f;
    mModel = 0;
    mStraightBullet = false;
    mVelocity = 35.0f;

    mScaleRate = 1.0f;
    mTurnRate = 1.0f;
    mWaverRate = 0.0f;
    mWaverTurn = 0.0f;
    mMinSpeed = 0.1f;
    mAcceleration = 10000.0f;
}

// FUNCTION: SWBF2 0x00605850
OrdnanceMissileClass::OrdnanceMissileClass(OrdnanceMissileClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mScaleRate = pBase->mScaleRate;
    mTurnRate = pBase->mTurnRate;
    mWaverRate = pBase->mWaverRate;
    mWaverTurn = pBase->mWaverTurn;
    mLockedOnSound = pBase->mLockedOnSound;
    mMinSpeed = pBase->mMinSpeed;
    mAcceleration = pBase->mAcceleration;
}







// FUNCTION: SWBF2 0x006049a0
float OrdnanceMissileClass::GetVelocity()
{
    return 3.0e8f;
}

// FUNCTION: SWBF2 0x00605910
void OrdnanceMissileClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x73E5A988:
        if (sscanf(pszValue, "%f", &mScaleRate))
        {
            mScaleRate = 1.0f / mScaleRate;
        }
        return;
    case 0x0F77C4E8:
        sscanf(pszValue, "%f", &mTurnRate);
        return;
    case 0x6D4A8840:
        sscanf(pszValue, "%f", &mWaverRate);
        return;
    case 0x129F08C1:
        sscanf(pszValue, "%f", &mWaverTurn);
        return;
    case 0x9371AECD:
        mLockedOnSound.SetID(pszValue, false, false);
        return;
    case 0xFBE2EB62:
        sscanf(pszValue, "%f", &mMinSpeed);

    case 0x2FA0BA9D:
        sscanf(pszValue, "%f", &mAcceleration);

    default:
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00605a20
OrdnanceClass* OrdnanceMissileClass::Derive(unsigned int uHash)
{
    return new OrdnanceMissileClass(this, uHash);
}

// FUNCTION: SWBF2 0x00605a90
Ordnance* OrdnanceMissileClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceMissile* pMissile = new OrdnanceMissile(this, desc);
    pMissile->ActivateThread(-40, PER_LOCAL_TURN);
    pMissile->AddOrdnanceToCollisionManager();
    return pMissile;
}

// SYNTHETIC: SWBF2 0x00605820
// OrdnanceMissileClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00605900
OrdnanceMissileClass::~OrdnanceMissileClass()
{
}
