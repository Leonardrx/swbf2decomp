#include "AmmoCounter.h"
#include <stdio.h>
#include <new>

#include "WeaponDestruct.h"
#include "Explosion.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntityDroid.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"

#include "Trigger.h"

extern bool netIsLocalTurn;



// SYNTHETIC: SWBF2 0x004070e0
// `dynamic initializer for 'WeaponDestruct::rttiHashWeaponDestruct''
// WeaponDestruct::rttiHashWeaponDestruct

// GLOBAL: SWBF2 0x01f9a8a4
// WeaponDestruct::rttiHashWeaponDestruct
PblHash WeaponDestruct::rttiHashWeaponDestruct("WeaponDestruct");
// GLOBAL: SWBF2 0x0078bee4
// WeaponDestruct::rttiNameWeaponDestruct
const char WeaponDestruct::rttiNameWeaponDestruct[] = "WeaponDestruct";







// SYNTHETIC: SWBF2 0x004070f0
// `dynamic initializer for 'WeaponDestructClass::rttiHashWeaponDestructClass''
// WeaponDestructClass::rttiHashWeaponDestructClass

// GLOBAL: SWBF2 0x01f9a8a8
// WeaponDestructClass::rttiHashWeaponDestructClass
PblHash WeaponDestructClass::rttiHashWeaponDestructClass("WeaponDestructClass");
// GLOBAL: SWBF2 0x0078befc
// WeaponDestructClass::rttiNameWeaponDestructClass
const char WeaponDestructClass::rttiNameWeaponDestructClass[] = "WeaponDestructClass";

// FUNCTION: SWBF2 0x00682190
WeaponDestructClass::WeaponDestructClass(unsigned int uHash)
    : WeaponClass(uHash), OrbitalStrikeClass(uHash)
{
    mTargetSides = 0;
    mTargetTypes = 0;
    mAITargetTypes = 0;

    mMaxPressedTime = 0.0f;
    mExplosionClass = 0;
}

// FUNCTION: SWBF2 0x00682220
WeaponDestructClass::WeaponDestructClass(WeaponDestructClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash), OrbitalStrikeClass(pBase, uHash)
{
    mMaxPressedTime = pBase->mMaxPressedTime;
    mExplosionClass = pBase->mExplosionClass;
}

// FUNCTION: SWBF2 0x00682280
WeaponDestructClass::~WeaponDestructClass()
{
}

// SYNTHETIC: SWBF2 0x006821f0
// WeaponDestructClass::`scalar deleting destructor'







// FUNCTION: SWBF2 0x00682290
void WeaponDestructClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x2819FFA0:
        sscanf(pszValue, "%f", &mMaxPressedTime);
        break;
    case 0x4C5AA5AA:
        sscanf(pszValue, "%f", &mChargeSoundPitch);
        break;
    case 0xCA0C1378:
        mExplosionClass = ExplosionClass::Find(PblHash(pszValue));
        break;
    default:
        if (!OrbitalStrikeClass::SetProperty(uProperty, pszValue))
        {
            WeaponClass::SetProperty(uProperty, pszValue);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00682320
WeaponClass* WeaponDestructClass::Derive(unsigned int uHash)
{
    return new WeaponDestructClass(this, uHash);
}

// FUNCTION: SWBF2 0x00682390
Weapon* WeaponDestructClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponDestruct(this, desc);
}

// FUNCTION: SWBF2 0x00681ab0
WeaponDestruct::WeaponDestruct(const WeaponDestructClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
}

// SYNTHETIC: SWBF2 0x00681af0
// WeaponDestruct::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00681b20
WeaponDestruct::~WeaponDestruct()
{
}

// FUNCTION: SWBF2 0x006820f0
bool WeaponDestruct::CheckCharge() const
{
    if (mState == IDLE)
    {
        if (mTrigger && mTrigger->IsPressed())
        {
            return true;
        }
        return false;
    }
    return mStateLimit > mStateTimer;
}

// FUNCTION: SWBF2 0x00682130
bool WeaponDestruct::CheckFire() const
{
    return mState == CHARGE && mStateTimer >= mStateLimit;
}

// FUNCTION: SWBF2 0x00681b30
bool WeaponDestruct::IsBusy() const
{
    return (mState == FIRE || mState == FIRE2) && mStrike.operator OrbitalStrike*() != 0;
}

// FUNCTION: SWBF2 0x00682150
bool WeaponDestruct::FireDone() const
{
    return mStrike.operator OrbitalStrike*() == 0;
}

// FUNCTION: SWBF2 0x00681b70
void WeaponDestruct::EnterCharge()
{
    Weapon::EnterCharge();
    mStateTimer = 0.0f;
    mStateLimit = ((WeaponDestructClass*)mClass)->mMaxPressedTime;
    m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
    if (mOwner->GetGameObject() && mOwner->GetGameObject()->mAddHealth < 0.0f)
    {
        mOwner->GetGameObject()->mAddHealth = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00681c00
bool WeaponDestruct::UpdateCharge(float fDeltaTime)
{
    if (mOwner->GetGameObject()->IsRtti(EntityDroid::rttiHashEntityDroid))
    {
        ((EntityDroid*)mOwner->GetGameObject())->FreezeMovement();
    }
    mStateTimer += fDeltaTime;
    return Weapon::UpdateCharge(fDeltaTime);
}

// FUNCTION: SWBF2 0x00681c60
void WeaponDestruct::EnterFire()
{
    Weapon::ExitCharge();

    if (mClass->mOrdnanceClass)
    {
        OrdnanceDesc desc;
        desc.mPosition = mOwner->GetGameObject()->GetCollisionObject()->GetCollisionCenter();
        desc.mOwner = mOwner->GetGameObject();
        desc.mDamageOwner.mCharacter = mOwner->mCharacter;
        desc.mDamageOwner.mGameObject = PblHandle<GameObject>(mOwner->GetGameObject());
        desc.mDamageOwner.mDamageMultiplier = mOwner->GetOffenseMultiplier();
        desc.mDamageOwner.mTeam = mOwner->GetGameObject()->GetTeam();
        desc.mDamageOwner.mClass = mOwner->GetGameObject()->GetEntityClass();
        desc.mDamageOwner.mWeapon = mClass;
        desc.mTarget = IsLocked() ? (GameObject*)GetLocked() : 0;
        desc.mBodyID = IsLocked() ? GetLockedTargetBodyId() : -1;
        desc.mFireWeapon = this;

        bool bBuild = true;
        bool bFlash = true;
        if (NetGame::IsEnabled())
        {
            if (NetGame::IsClient())
            {
                bBuild = netIsLocalTurn && NetGame::GetJoystickIndex(mOwner->mPlayerId) >= 0;
            }
            else
            {
                bFlash = netIsOneWorld && !netIsDedicated;
                NetGame::CreateEvent_CreateOrdnance(mClass->mOrdnanceClass, desc);
            }
        }
        if (bBuild)
        {
            if (bFlash)
            {
                mClass->CreateMuzzleFlash(mOwner->GetGameObject()->GetMatrix());
            }
            mClass->mOrdnanceClass->Build(desc);
        }
    }

    if (((WeaponDestructClass*)mClass)->mStrikeClass)
    {
        OrbitalStrikeDesc desc = { PblVector3(0.0f, 0.0f, 0.0f), DamageOwner(0, 0, 0, 0, 0, 1.0f) };
        desc.mPosition = mOwner->GetGameObject()->GetCollisionObject()->GetCollisionCenter();
        desc.mDamageOwner.mCharacter = mOwner->mCharacter;
        desc.mDamageOwner.mGameObject = PblHandle<GameObject>(mOwner->GetGameObject());
        desc.mDamageOwner.mDamageMultiplier = mOwner->GetOffenseMultiplier();
        desc.mDamageOwner.mTeam = mOwner->GetGameObject()->GetTeam();
        desc.mDamageOwner.mClass = mOwner->GetGameObject()->GetGameObjectClass();
        desc.mDamageOwner.mWeapon = mClass;
        mStrike = new OrbitalStrike((WeaponDestructClass*)mClass, desc);
    }
}

// FUNCTION: SWBF2 0x00681fc0
void WeaponDestruct::ExitFire()
{
    Weapon::ExitFire();
    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    owner.mCharacter = mOwner->mCharacter;
    owner.mGameObject = mOwner->GetGameObject();
    owner.mTeam = mOwner->GetGameObject()->GetTeam();
    owner.mClass = mOwner->GetGameObject()->GetEntityClass();
    owner.mWeapon = mClass;
    if (((WeaponDestructClass*)mClass)->mExplosionClass)
    {
        ((WeaponDestructClass*)mClass)->mExplosionClass->CreateExplosion(
            mOwner->GetGameObject()->GetCollisionObject()->GetCollisionCenter(), owner, false, COLL_UNKNOWN);
    }
    if (mOwner->GetGameObject()->IsRtti(EntityDroid::rttiHashEntityDroid))
    {
        ((EntityDroid*)mOwner->GetGameObject())->KillDelay(1.0f);
    }
    else
    {
        mOwner->GetGameObject()->InstantDeath(owner, false);
    }
}
