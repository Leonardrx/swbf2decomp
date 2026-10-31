#include "AmmoCounter.h"
#include <stdio.h>

#include "WeaponLaser.h"
#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "Trigger.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include <new>

extern bool netIsLocalTurn;
namespace GameLoop
{
extern float sClientDeltaTime;
}

namespace CombatUtil
{
void SendDamageEvent(const Controllable* pOwner, const GameObject* pTarget, const Trigger* pTrigger);
}



namespace
{
    inline EntityClass* GetEntityClass(GameObject* pObject)
    {
        typedef EntityClass* (__thiscall *Fn)(GameObject*);
        return pObject->GetEntityClass();
    }
}

// SYNTHETIC: SWBF2 0x004071c0
// `dynamic initializer for 'WeaponLaser::rttiHashWeaponLaser''
// WeaponLaser::rttiHashWeaponLaser

// GLOBAL: SWBF2 0x01f9a8e0
// WeaponLaser::rttiHashWeaponLaser
PblHash WeaponLaser::rttiHashWeaponLaser("WeaponLaser");
// GLOBAL: SWBF2 0x0078c06c
// WeaponLaser::rttiNameWeaponLaser
const char WeaponLaser::rttiNameWeaponLaser[] = "WeaponLaser";







// FUNCTION: SWBF2 0x00686ae0
WeaponLaser::WeaponLaser(const WeaponLaserClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mSalvoTimer = 0.0f;
}

// FUNCTION: SWBF2 0x00686b10
WeaponLaser::~WeaponLaser()
{
}

// SYNTHETIC: SWBF2 0x00686a60
// WeaponLaser::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00686b20
bool WeaponLaser::IsBusy() const
{
    if (mState == FIRE || mState == FIRE2)
    {
        if (mSalvoTimer - GameLoop::sClientDeltaTime > 0.0f ||
            mStateLimit > mStateTimer + GameLoop::sClientDeltaTime)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00686b70
void WeaponLaser::EnterFire()
{
    Weapon::EnterFire();
    const WeaponLaserClass* pClass = static_cast<const WeaponLaserClass*>(mClass);
    mStateTimer = 0.0f;
    mStateLimit = pClass->mShotDelay;
    mSalvoTimer = pClass->mSalvoTime;
    if (pClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x00686bc0
bool WeaponLaser::UpdateFire(float fDeltaTime)
{
    float fRemaining = fDeltaTime;
    if (!mSelectedFlag)
    {
        mSalvoTimer = 0.0f;
    }
    while (fRemaining > 0.0f)
    {
        float fStep = mSalvoTimer;
        if (fStep <= 0.0f)
        {
            const float fLimit = mStateLimit;
            float fWait = fLimit - mStateTimer;
            if (fWait > fRemaining)
            {
                fWait = fRemaining;
            }
            fRemaining -= fWait;
            mStateTimer += fWait;
            if (!(mStateTimer >= fLimit) || mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty())
            {
                mSoundControl.Stop(true);
                break;
            }
            mStateTimer -= mStateLimit;
            if (static_cast<const WeaponLaserClass*>(mClass)->mShotDelay > 0.0f)
            {
                Weapon::ExitFire();
                Weapon::EnterFire();
            }
            mSalvoTimer = static_cast<const WeaponLaserClass*>(mClass)->mSalvoTime;
        }
        else
        {
            if (fStep > fRemaining)
            {
                fStep = fRemaining;
            }
            Aimer* pFirst = mAimer;
            do
            {
                if (mAimer->WeaponShouldFire())
                {
                    Fire(fStep);
                    const float fShare = fStep / static_cast<const WeaponLaserClass*>(mClass)->mSalvoTime;
                    m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot * fShare);
                    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot * fShare);
                }
            } while (!mAimer->UpdateFire(mAimer, true) || mAimer != pFirst);
            fRemaining -= fStep;
            mSalvoTimer -= fStep;
        }
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00686de0
void WeaponLaser::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00686df0
bool WeaponLaser::CheckFire() const
{
    if (!mTrigger)
    {
        return false;
    }
    if (CheckOverheat())
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00686e10
bool WeaponLaser::FireDone() const
{
    return mSalvoTimer <= 0.0f && mStateTimer >= mStateLimit &&
           (mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty());
}

// FUNCTION: SWBF2 0x00686e80
bool WeaponLaser::Fire(float fTime)
{
    OrdnanceDesc desc;
    desc.mPosition = mAimer->mFirePos;
    desc.mDirection = mAimer->mDirection;
    desc.mStrength = fTime / static_cast<const WeaponLaserClass*>(mClass)->mSalvoTime;
    desc.mOwner = mOwner->GetGameObject();
    desc.mDamageOwner.mCharacter = mOwner->mCharacter;
    desc.mDamageOwner.mGameObject = PblHandle<GameObject>(mOwner->GetGameObject());
    desc.mDamageOwner.mDamageMultiplier = mOwner->GetOffenseMultiplier();
    desc.mDamageOwner.mTeam = mOwner->GetGameObject()->GetTeam();
    desc.mDamageOwner.mClass = mOwner->GetGameObject()->GetEntityClass();
    desc.mDamageOwner.mWeapon = mClass;
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
    CombatUtil::SendDamageEvent(mOwner, static_cast<const GameObject*>(mTarget), mTrigger);
    return true;
}

// FUNCTION: SWBF2 0x006870a0
void WeaponLaser::EnterEmpty()
{
    Weapon::EnterEmpty();
    mStateLimit = static_cast<const WeaponLaserClass*>(mClass)->mShotDelay;
}

// SYNTHETIC: SWBF2 0x004071d0
// `dynamic initializer for 'WeaponLaserClass::rttiHashWeaponLaserClass''
// WeaponLaserClass::rttiHashWeaponLaserClass

// GLOBAL: SWBF2 0x01f9a8dc
// WeaponLaserClass::rttiHashWeaponLaserClass
PblHash WeaponLaserClass::rttiHashWeaponLaserClass("WeaponLaserClass");
// GLOBAL: SWBF2 0x0078c07c
// WeaponLaserClass::rttiNameWeaponLaserClass
const char WeaponLaserClass::rttiNameWeaponLaserClass[] = "WeaponLaserClass";

// FUNCTION: SWBF2 0x006870c0
WeaponLaserClass::WeaponLaserClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mShotDelay = 1.0f;
    mSalvoTime = 0.5f;
}







// SYNTHETIC: SWBF2 0x006870f0
// WeaponLaserClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00687120
WeaponLaserClass::WeaponLaserClass(WeaponLaserClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mShotDelay = pBase->mShotDelay;
    mSalvoTime = pBase->mSalvoTime;
}

// FUNCTION: SWBF2 0x00687160
WeaponLaserClass::~WeaponLaserClass()
{
}

// FUNCTION: SWBF2 0x00687170
void WeaponLaserClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x4225631D: /* "SalvoTime" */
        sscanf(pszValue, "%f", &mSalvoTime);
        break;

    case 0xD9356908: /* "ShotDelay" */
        sscanf(pszValue, "%f", &mShotDelay);
        break;

    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x006871c0
WeaponClass* WeaponLaserClass::Derive(unsigned int uHash)
{
    return new WeaponLaserClass(this, uHash);
}

// FUNCTION: SWBF2 0x00687230
Weapon* WeaponLaserClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponLaser(this, desc);
}
