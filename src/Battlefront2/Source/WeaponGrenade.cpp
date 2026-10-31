#include "AmmoCounter.h"
#include <d3dx9math.h>
#include <stdio.h>
#include <stdlib.h>

#include "WeaponGrenade.h"
#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "SoldierAnimatorClass.h"
#include "Trigger.h"


extern bool netIsLocalTurn;



namespace GameLoop
{
extern float sClientDeltaTime;
}

// SYNTHETIC: SWBF2 0x00407190
// `dynamic initializer for 'WeaponGrenadeClass::rttiHashWeaponGrenadeClass''
// WeaponGrenadeClass::rttiHashWeaponGrenadeClass

// GLOBAL: SWBF2 0x01f9a8cc
// WeaponGrenadeClass::rttiHashWeaponGrenadeClass
PblHash WeaponGrenadeClass::rttiHashWeaponGrenadeClass("WeaponGrenadeClass");
// GLOBAL: SWBF2 0x0078bff4
// WeaponGrenadeClass::rttiNameWeaponGrenadeClass
const char WeaponGrenadeClass::rttiNameWeaponGrenadeClass[] = "WeaponGrenadeClass";

// FUNCTION: SWBF2 0x00685770
WeaponGrenadeClass::WeaponGrenadeClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mLockOnAngle = 0.0f;
    mReloadTime = 0.6f;

    mShotDelay = 0.125f;
    mInitialSalvoDelay[0] = 0.5f;
    mInitialSalvoDelay[1] = 0.36f;
    mInitialSalvoDelay[2] = 0.584f;
    mShotElevate = 0.3f;
    mMaxPressedTime = 0.0f;

    mHideOnFire = true;
    mForcedFireAnim = -1;

    memset(m_avFireOffset, 0, sizeof(m_avFireOffset));
}







// FUNCTION: SWBF2 0x00685840
WeaponGrenadeClass::WeaponGrenadeClass(WeaponGrenadeClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mShotDelay = pBase->mShotDelay;
    memcpy(mInitialSalvoDelay, pBase->mInitialSalvoDelay, sizeof(mInitialSalvoDelay));
    mShotElevate = pBase->mShotElevate;
    mMaxPressedTime = pBase->mMaxPressedTime;
    mHideOnFire = pBase->mHideOnFire;
    mForcedFireAnim = pBase->mForcedFireAnim;
    memcpy(m_avFireOffset, pBase->m_avFireOffset, sizeof(m_avFireOffset));
}

// FUNCTION: SWBF2 0x006858f0
WeaponGrenadeClass::~WeaponGrenadeClass()
{
}

// SYNTHETIC: SWBF2 0x00685810
// WeaponGrenadeClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00685900
void WeaponGrenadeClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xD9356908:
    {
        sscanf(pszValue, "%f", &mShotDelay);
        if (mShotDelay < 0.01f)
        {
            mShotDelay = 0.01f;
        }
            break;
    }
    case 0xC8388D9D:
    {
        sscanf(pszValue, "%f", &mInitialSalvoDelay[1]);
        mInitialSalvoDelay[0] = mInitialSalvoDelay[1];
        mInitialSalvoDelay[2] = mInitialSalvoDelay[1];
            break;
    }
    case 0x2A708F20:
    {
        sscanf(pszValue, "%f", &mInitialSalvoDelay[0]);
            break;
    }
    case 0x36F0BFCD:
    {
        sscanf(pszValue, "%f", &mInitialSalvoDelay[2]);
            break;
    }
    case 0xAE3DB60D:
    {
        sscanf(pszValue, "%f", &mShotElevate);
            break;
    }
    case 0x2819FFA0:
    {
        sscanf(pszValue, "%f", &mMaxPressedTime);
            break;
    }
    case 0x5578D44C:
    {
        mHideOnFire = atoi(pszValue) != 0;
            break;
    }
    case 0x079C1D86:
    {
        mForcedFireAnim = (char)atoi(pszValue);
        if (mForcedFireAnim < 0 || mForcedFireAnim > 1)
        {
            mForcedFireAnim = -1;
        }
            break;
    }
    default:
    {
        WeaponClass::SetProperty(uProperty, pszValue);
            break;
    }
    }
}

// FUNCTION: SWBF2 0x00685a50
WeaponClass* WeaponGrenadeClass::Derive(unsigned int uHash)
{
    return new WeaponGrenadeClass(this, uHash);
}

// FUNCTION: SWBF2 0x00685ac0
Weapon* WeaponGrenadeClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponGrenade(this, desc);
}

// FUNCTION: SWBF2 0x00685b30
bool WeaponGrenadeClass::PostLoadInit()
{
    if (!WeaponClass::PostLoadInit())
    {
        return false;
    }

    PblMatrix matrix;
    if (mIsOffhand)
    {
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY,
                0xb7ec1d31, mInitialSalvoDelay[0], matrix))
        {
            m_avFireOffset[0] = matrix.GetTranslation();
            m_avFireOffset[0].x = 0.0f;
        }
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY2,
                0xb7ec1d31, mInitialSalvoDelay[1], matrix))
        {
            m_avFireOffset[1] = matrix.GetTranslation();
            m_avFireOffset[1].x = 0.0f;
        }
    }
    else
    {
        PblVector3 v;
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY,
                0x2b960099, mInitialSalvoDelay[0], matrix))
        {
            D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&v), reinterpret_cast<const D3DXVECTOR3*>(&mFirePointOffset), reinterpret_cast<const D3DXMATRIX*>(&matrix));
            m_avFireOffset[0] = v;
        }
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY2,
                0x2b960099, mInitialSalvoDelay[1], matrix))
        {
            D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&v), reinterpret_cast<const D3DXVECTOR3*>(&mFirePointOffset), reinterpret_cast<const D3DXMATRIX*>(&matrix));
            m_avFireOffset[1] = v;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00684fe0
WeaponGrenade::WeaponGrenade(const WeaponGrenadeClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mNetFiredFlag = false;
    mFired = false;
    mStrength = 0.0f;
    m_iFirePos = 0;
}

// SYNTHETIC: SWBF2 0x00407180
// `dynamic initializer for 'WeaponGrenade::rttiHashWeaponGrenade''
// WeaponGrenade::rttiHashWeaponGrenade

// GLOBAL: SWBF2 0x01f9a8d0
// WeaponGrenade::rttiHashWeaponGrenade
PblHash WeaponGrenade::rttiHashWeaponGrenade("WeaponGrenade");
// GLOBAL: SWBF2 0x0078c018
// WeaponGrenade::rttiNameWeaponGrenade
const char WeaponGrenade::rttiNameWeaponGrenade[] = "WeaponGrenade";







// FUNCTION: SWBF2 0x00685060
bool WeaponGrenade::IsBusy() const
{
    if (mState == FIRE || mState == FIRE2)
    {
        if (!mFired || mStateLimit > mStateTimer + GameLoop::sClientDeltaTime)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006850d0
void WeaponGrenade::EnterFire()
{
    const WeaponGrenadeClass* pClass = (const WeaponGrenadeClass*)mClass;

    if (pClass->mHideOnFire)
    {
        mHideWeapon = 1;
    }
    mFired = false;

    if (pClass->mMaxPressedTime > 0.0f)
    {
        mStrength = mStateTimer / pClass->mMaxPressedTime;
    }
    else
    {
        mStrength = 1.0f;
    }

    char iFireAnim = pClass->mForcedFireAnim;
    bool bStrongThrow = false;
    if (iFireAnim < 0)
    {
        if (mStrength > 0.4f)
        {
            bStrongThrow = true;
        }
        iFireAnim = bStrongThrow;
    }
    mState = (WeaponState)(FIRE + iFireAnim);

    if (m_iSoldierState == 2)
    {
        m_iFirePos = 2;
    }
    else
    {
        m_iFirePos = iFireAnim;
    }

    mStateTimer = 0.0f;
    mStateLimit = pClass->mInitialSalvoDelay[m_iFirePos] + pClass->mShotDelay;
    mFiredFlag = 1;
    mNetFiredFlag = true;

    if (pClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x00685330
bool WeaponGrenade::CheckCharge() const
{
    if (!mTrigger)
    {
        return false;
    }
    if (0.0f >= ((const WeaponGrenadeClass*)mClass)->mMaxPressedTime)
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00685350
bool WeaponGrenade::CheckFire() const
{
    if (!mTrigger)
    {
        return false;
    }
    if (reinterpret_cast<const Weapon*>(this)->CheckOverheat())
    {
        return false;
    }
    if (((const WeaponGrenadeClass*)mClass)->mMaxPressedTime > 0.0f)
    {
        return mTrigger->IsRelease();
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00685050
WeaponGrenade::~WeaponGrenade()
{
}

// SYNTHETIC: SWBF2 0x00685020
// WeaponGrenade::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006850a0
void WeaponGrenade::SignalFire()
{
    bool bFiredFlag = mFiredFlag != 0;
    Weapon::SignalFire();
    mFiredFlag = bFiredFlag;
}

// FUNCTION: SWBF2 0x006851d0
bool WeaponGrenade::UpdateFire(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (!mSelectedFlag)
    {
        mFired = true;
    }
    if (!mFired)
    {
        const WeaponGrenadeClass* pClass = (const WeaponGrenadeClass*)mClass;
        if (mStateTimer >= pClass->mInitialSalvoDelay[m_iFirePos])
        {
            if (m_pAmmoCounter->CanExpendRounds(pClass->mRoundsPerShot) &&
                (!m_pEnergyBar->m_bOverheat || pClass->mHeatPerShot <= 0.0f) && Fire())
            {
                m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
                m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
            }
            mNetFiredFlag = false;
            mFired = true;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00685290
void WeaponGrenade::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x006852a0
void WeaponGrenade::EnterCharge()
{
    mStateTimer = 0.0f;
    mStateLimit = ((const WeaponGrenadeClass*)mClass)->mMaxPressedTime;
    Weapon::EnterCharge();
}

// FUNCTION: SWBF2 0x006852c0
bool WeaponGrenade::UpdateCharge(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (mStateTimer >= mStateLimit)
    {
        mStateTimer = mStateLimit;
    }
    Weapon::UpdateCharge(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x00685310
void WeaponGrenade::ExitCharge()
{
    Weapon::ExitCharge();
}

// FUNCTION: SWBF2 0x00685320
void WeaponGrenade::ExitReload()
{
    mHideWeapon = 0;
    Weapon::ExitReload();
}

// FUNCTION: SWBF2 0x006853a0
bool WeaponGrenade::FireDone() const
{
    return mFired && mStateTimer >= mStateLimit
           && (mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty());
}

// FUNCTION: SWBF2 0x00685400
void WeaponGrenade::EnterEmpty()
{
    Weapon::EnterEmpty();
    mStateLimit = ((const WeaponGrenadeClass*)mClass)->mShotDelay;
}

// FUNCTION: SWBF2 0x00685420
bool WeaponGrenade::Fire()
{
    if (!NetGame::IsEnabled())
    {
        PlayFireSound(false);
    }

    OrdnanceDesc desc;
    if (mOwner)
    {
        PblVector3 vOffset = ((const WeaponGrenadeClass*)mClass)->m_avFireOffset[m_iFirePos];
        if (mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            float fScale = static_cast<EntitySoldier*>(mOwner->GetGameObject())->mClass->mSkeletonRootScale;
            if (fScale != 1.0f)
            {
                vOffset *= fScale;
            }
        }
        PblVector3 vPos;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPos), reinterpret_cast<const D3DXVECTOR3*>(&vOffset), reinterpret_cast<const D3DXMATRIX*>(&mOwner->GetMatrix()));
        desc.mPosition = vPos;
    }
    else
    {
        desc.mPosition = mAimer->mFirePos;
    }

    desc.mDirection = mAimer->mDirection;
    if (mOwner->mPlayerId >= 0)
    {
        desc.mDirection.y = desc.mDirection.y + ((const WeaponGrenadeClass*)mClass)->mShotElevate;
        PblVector3 vDir;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&desc.mDirection));
        desc.mDirection = vDir;
    }
    desc.mStrength = mStrength;
    desc.mOwner = mOwner->GetGameObject();
    desc.mDamageOwner.mCharacter = mOwner->mCharacter;
    desc.mDamageOwner.mGameObject = PblHandle<GameObject>(mOwner->GetGameObject());
    desc.mDamageOwner.mDamageMultiplier = mOwner->GetOffenseMultiplier();
    desc.mDamageOwner.mTeam = mOwner->GetGameObject()->GetTeam();
    desc.mDamageOwner.mClass = mOwner->GetGameObject()->GetEntityClass();
    desc.mDamageOwner.mWeapon = mClass;
    desc.mFireWeapon = this;
    desc.mTarget = const_cast<GameObject*>(static_cast<const GameObject*>(mTarget));
    desc.mBodyID = mTargetBodyID;

    bool bFlash = true;
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
            bFlash = NetGame::IsOneWorld() && !NetGame::IsDedicated();
            NetGame::CreateEvent_CreateOrdnance(mClass->mOrdnanceClass, desc);
        }
    }
    if (bFlash)
    {
        mClass->CreateMuzzleFlash(mFirePointMatrix);
    }

    mClass->mOrdnanceClass->Build(desc);
    if (((const WeaponGrenadeClass*)mClass)->mInitialSalvoDelay[m_iFirePos] > 0.0f)
    {
        mFiredFlag = 0;
    }
    return true;
}
