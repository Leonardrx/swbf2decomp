#include "AmmoCounter.h"
#include <d3dx9math.h>
#include <math.h>
#include <stdlib.h>

#include "WeaponCatapult.h"

#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntityBuildingArmed.h"
#include "GameObject.h"
#include "MountedTurret.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "Trigger.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include <new>

extern bool netIsLocalTurn;




namespace
{
    inline EntityClass* GetEntityClass(GameObject* pObject)
    {
        typedef EntityClass* (__thiscall *Fn)(GameObject*);
        return pObject->GetEntityClass();
    }
}

// SYNTHETIC: SWBF2 0x004070c0
// `dynamic initializer for 'WeaponCatapult::rttiHashWeaponCatapult''
// WeaponCatapult::rttiHashWeaponCatapult

// GLOBAL: SWBF2 0x01f9a89c
// WeaponCatapult::rttiHashWeaponCatapult
PblHash WeaponCatapult::rttiHashWeaponCatapult("WeaponCatapult");
// GLOBAL: SWBF2 0x0078beb0
// WeaponCatapult::rttiNameWeaponCatapult
const char WeaponCatapult::rttiNameWeaponCatapult[] = "WeaponCatapult";







// FUNCTION: SWBF2 0x00681450
bool WeaponCatapult::CheckReload() const
{
    return true;
}

// FUNCTION: SWBF2 0x00680c80
WeaponCatapult::WeaponCatapult(const WeaponCatapultClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mCatapultClass = const_cast<WeaponCatapultClass*>(pClass);
    float fCockedAngle = pClass->mCockedAngle;
    mLastArmAngle = fCockedAngle;
    mNextArmAngle = fCockedAngle;
    mCatapultBuilding = 0;
    Entity* pEntity = mOwner->GetGameObject()->GetEntity();
    if (pEntity && pEntity->IsRtti(EntityBuildingArmed::rttiHashEntityBuildingArmed))
    {
        mCatapultBuilding = static_cast<EntityBuildingArmed*>(pEntity);
        SetArmAngle(mLastArmAngle);
    }
    mCurrArmAngle = mLastArmAngle;
    mArmAccel = 0.0f;
    mArmSpeed = 0.0f;
    mShowOrdnance = true;
}

// FUNCTION: SWBF2 0x00680d90
WeaponCatapult::~WeaponCatapult()
{
}

// SYNTHETIC: SWBF2 0x00680d60
// WeaponCatapult::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00680db0
void WeaponCatapult::SetArmAngle(float fAngle)
{
    if (!mCatapultBuilding)
    {
        return;
    }
    const PblMatrix* pArm = mCatapultBuilding->mPose.Find(mCatapultClass->mArmHash);
    if (!pArm)
    {
        return;
    }
    PblMatrix* pOut = &mArmPoseMatrix;
    if (pArm != pOut)
    {
        mCatapultBuilding->mPose.Remove(mCatapultClass->mArmHash);
        mCatapultBuilding->mPose.Store(mCatapultClass->mArmHash, pOut);
    }

    const PblVector3 vX = pArm->GetXAxis();
    const PblVector3 vUp(0.0f, 1.0f, 0.0f);
    (*pOut)[0] = pArm->GetXAxis();
    pOut->m[0][3] = 0.0f;
    (*pOut)[1] = PblVector3(vUp.y * vX.z - vUp.z * vX.y, vUp.z * vX.x - vUp.x * vX.z,
                            vUp.x * vX.y - vUp.y * vX.x);
    pOut->m[1][3] = 0.0f;
    (*pOut)[2] = vUp;
    pOut->m[2][3] = 0.0f;

    PblQuaternion qArm(*pOut);
    const PblVector3 vOmega(fAngle, 0.0f, 0.0f);
    const float fLength = PblMath::Sqrt(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fLength > 0.0001f)
    {
        const float fInv = 1.0f / fLength;
        const PblVector3 vAxis(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv);
        qArm = qArm * PblQuaternion(vAxis, fLength);
    }

    PblMatrix mPose;
    D3DXMatrixRotationQuaternion(reinterpret_cast<D3DXMATRIX*>(&mPose), D3DXQuaternionNormalize(reinterpret_cast<D3DXQUATERNION*>(&qArm), reinterpret_cast<const D3DXQUATERNION*>(&qArm)));
    mPose.m[3][0] = pArm->m[3][0];
    mPose.m[3][1] = pArm->m[3][1];
    mPose.m[3][2] = pArm->m[3][2];
    mPose.m[3][3] = 1.0f;
    *pOut = mPose;
}

// FUNCTION: SWBF2 0x00681080
void WeaponCatapult::EnterFire()
{
    mNextArmAngle = mLastArmAngle;
    Entity* pEntity = mOwner->GetEntity();
    if (pEntity && pEntity->IsRtti(MountedTurret::rttiHashMountedTurret))
    {
        mNextArmAngle = static_cast<MountedTurret*>(pEntity)->mPitch + 1.5707964f;
    }
    mCurrArmAngle = mLastArmAngle;
    mArmSpeed = 0.0f;
    mStateTimer = 0.0f;
    mStateLimit = 1.0f;

    const PblVector3 vFirePos = mAimer->mFirePos;
    if (const PblMatrix* pArm = mCatapultBuilding->mPose.Find(mCatapultClass->mArmHash))
    {
        const PblVector3& vBase = mCatapultBuilding->GetPosition();
        const float fDist = Length(vBase + pArm->GetTranslation() - vFirePos);
        if (fDist > 0.0f)
        {
            const float fRate = mClass->mOrdnanceClass->GetVelocity() / fDist;
            if (fRate != 0.0f)
            {
                const float fSwing = (mNextArmAngle - mLastArmAngle) * 2.0f / fRate;
                if (fSwing != 0.0f)
                {
                    mArmAccel = fRate / fSwing;
                    mArmSpeed = 0.0f;
                    const double dSwingMagnitude = fabs((double)fSwing);
                    mStateLimit += (float)dSwingMagnitude;
                }
            }
        }
    }
    if (mClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x00681270
bool WeaponCatapult::UpdateFire(float fDeltaTime)
{
    mArmSpeed += mArmAccel * fDeltaTime;
    const float fLastAngle = mCurrArmAngle;
    mCurrArmAngle = mArmSpeed * fDeltaTime + fLastAngle;
    if (mCurrArmAngle > mNextArmAngle)
    {
        mCurrArmAngle = mNextArmAngle;
        if (mNextArmAngle > fLastAngle)
        {
            Fire();
        }
    }
    SetArmAngle(mCurrArmAngle);
    mStateTimer += fDeltaTime;
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00681310
void WeaponCatapult::EnterReload()
{
    mLastArmAngle = mNextArmAngle;
    mNextArmAngle = mCatapultClass->mCockedAngle;
    if (mClass->mReloadTime > 0.0f)
    {
        mArmSpeed = (mNextArmAngle - mLastArmAngle) / mClass->mReloadTime;
    }
    else
    {
        mArmSpeed = (mNextArmAngle - mLastArmAngle) * 0.25f;
    }
    mCurrArmAngle = mLastArmAngle;
    mArmAccel = 0.0f;
    Weapon::EnterReload();
}

// FUNCTION: SWBF2 0x00681380
bool WeaponCatapult::UpdateReload(float fDeltaTime)
{
    mArmSpeed += mArmAccel * fDeltaTime;
    mCurrArmAngle += mArmSpeed * fDeltaTime;
    if (mNextArmAngle > mCurrArmAngle)
    {
        mCurrArmAngle = mNextArmAngle;
    }
    SetArmAngle(mCurrArmAngle);
    return Weapon::UpdateReload(fDeltaTime);
}

// FUNCTION: SWBF2 0x00681400
void WeaponCatapult::ExitReload()
{
    mLastArmAngle = mNextArmAngle;
    mShowOrdnance = true;
    Weapon::ExitReload();
}

// FUNCTION: SWBF2 0x00681420
bool WeaponCatapult::CheckFire() const
{
    if (!mTrigger)
    {
        return false;
    }
    return mTrigger->m_bDown;
}

// FUNCTION: SWBF2 0x00681430
bool WeaponCatapult::FireDone() const
{
    return mStateTimer >= mStateLimit;
}

// FUNCTION: SWBF2 0x00681460
bool WeaponCatapult::Fire()
{
    mShowOrdnance = false;
    m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);

    OrdnanceDesc desc;
    desc.mPosition = mAimer->mFirePos;
    desc.mDirection = mAimer->mDirection;
    desc.mOwner = mOwner->GetGameObject();
    desc.mDamageOwner.mCharacter = mOwner->mCharacter;
    PblHandle<GameObject> hOwner(mOwner->GetGameObject());
    desc.mDamageOwner.mGameObject = hOwner;
    desc.mDamageOwner.mDamageMultiplier = mOwner->GetOffenseMultiplier();
    desc.mDamageOwner.mTeam = mOwner->GetGameObject()->GetTeam();
    desc.mDamageOwner.mClass = GetEntityClass(mOwner->GetGameObject());
    desc.mDamageOwner.mWeapon = mClass;
    desc.mTarget = const_cast<GameObject*>(static_cast<const GameObject*>(mTarget));
    desc.mBodyID = mTargetBodyID;

    if (NetGame::IsEnabled())
    {
        if (NetGame::IsClient())
        {
            if (!netIsLocalTurn || NetGame::GetJoystickIndex(mOwner->mPlayerId) < 0)
            {
                return true;
            }
        }
        else
        {
            NetGame::CreateEvent_CreateOrdnance(mClass->mOrdnanceClass, desc);
        }
    }
    mClass->mOrdnanceClass->Build(desc);
    return true;
}

// FUNCTION: SWBF2 0x00681630
void WeaponCatapult::Render(const PblMatrix& matrix, const RedPose* pPose, const RedColor& color,
                            unsigned int uFlags, bool bArg) const
{
    if (mCatapultClass->mOrdnanceModel && mShowOrdnance)
    {
        if (const PblMatrix* pArm = mCatapultBuilding->mPose.Find(mCatapultClass->mArmHash))
        {
            const PblVector3 vFirePos = mAimer->mFirePos;
            const PblVector3& vBase = mCatapultBuilding->GetPosition();
            const float fDist = Length(vBase + pArm->GetTranslation() - vFirePos);
            PblMatrix mOrdnance = *pArm * mCatapultBuilding->GetMatrix();
            mOrdnance.SetTranslation(mOrdnance.GetTranslation() + mOrdnance.GetYAxis() * fDist);
            mOrdnance.m[3][3] = 1.0f;
            mCatapultClass->mOrdnanceModel->Render(&mOrdnance, 0, color, uFlags, 0);
        }
    }
    Weapon::Render(matrix, pPose, color, uFlags, bArg);
}

// SYNTHETIC: SWBF2 0x004070d0
// `dynamic initializer for 'WeaponCatapultClass::rttiHashWeaponCatapultClass''
// WeaponCatapultClass::rttiHashWeaponCatapultClass

// GLOBAL: SWBF2 0x01f9a8a0
// WeaponCatapultClass::rttiHashWeaponCatapultClass
PblHash WeaponCatapultClass::rttiHashWeaponCatapultClass("WeaponCatapultClass");
// GLOBAL: SWBF2 0x0078bec0
// WeaponCatapultClass::rttiNameWeaponCatapultClass
const char WeaponCatapultClass::rttiNameWeaponCatapultClass[] = "WeaponCatapultClass";

// FUNCTION: SWBF2 0x006817d0
WeaponCatapultClass::WeaponCatapultClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mOrdnanceModel = 0;
    mArmHash = 0;
    mCockedAngle = -30.0f;
}

// FUNCTION: SWBF2 0x00681840
WeaponCatapultClass::WeaponCatapultClass(WeaponCatapultClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mOrdnanceModel = pBase->mOrdnanceModel;
    mArmHash = pBase->mArmHash;
    mCockedAngle = pBase->mCockedAngle;
}

// FUNCTION: SWBF2 0x00681890
WeaponCatapultClass::~WeaponCatapultClass()
{
}

// SYNTHETIC: SWBF2 0x00681810
// WeaponCatapultClass::`scalar deleting destructor'







// FUNCTION: SWBF2 0x006818a0
void WeaponCatapultClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x64A37D77:

        mCockedAngle = (float)atof(pszValue) * 3.1415927f / 180.0f;
        break;
    case 0x90491ED6:
        mArmHash = PblTEMPHash(pszValue);
        break;
    case 0xA6032172:
        mOrdnanceModel = RedModel::_HashTable.Find(PblHash(pszValue));
        break;
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00681930
WeaponClass* WeaponCatapultClass::Derive(unsigned int uHash)
{
    return new WeaponCatapultClass(this, uHash);
}

// FUNCTION: SWBF2 0x006819a0
Weapon* WeaponCatapultClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponCatapult(this, desc);
}
