#include <stdio.h>
#include <stdlib.h>
#include <new>

#include "WeaponMeleeThrow.h"
#include "WeaponMeleeInstance.h"
#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "SoldierAnimatorClass.h"
#include "RedEngineFL/Graphics/RedPose.h"

extern bool netIsLocalTurn;

namespace
{
    inline EntityClass* GetEntityClass(GameObject* pObject)
    {
        typedef EntityClass* (__thiscall *Fn)(GameObject*);
        return pObject->GetEntityClass();
    }
}

// SYNTHETIC: SWBF2 0x00407240
// `dynamic initializer for 'WeaponMeleeThrow::rttiHashWeaponMeleeThrow''
// WeaponMeleeThrow::rttiHashWeaponMeleeThrow

// GLOBAL: SWBF2 0x01f9a8fc
// WeaponMeleeThrow::rttiHashWeaponMeleeThrow
PblHash WeaponMeleeThrow::rttiHashWeaponMeleeThrow("WeaponMeleeThrow");
// GLOBAL: SWBF2 0x0078c1cc
// WeaponMeleeThrow::rttiNameWeaponMeleeThrow
const char WeaponMeleeThrow::rttiNameWeaponMeleeThrow[] = "WeaponMeleeThrow";

// FUNCTION: SWBF2 0x0068f610
WeaponMeleeThrow::WeaponMeleeThrow(const WeaponMeleeThrowClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    m_bFired = 0;
    m_bReturnedThisFrame = 0;
    m_pPrimaryWeapon = 0;

    int iMeleeIndex = pClass->m_iPrimaryWeaponIndex;
    EntitySoldier& soldier = *static_cast<EntitySoldier*>(mOwner->GetGameObject());
    for (int i = 0; i < soldier.GetWeaponCount(); ++i)
    {
        if (soldier.GetWeaponPtr(i) && soldier.GetWeaponPtr(i)->IsMelee())
        {
            m_pPrimaryWeapon = static_cast<WeaponMelee*>(soldier.GetWeaponPtr(i));
            if (iMeleeIndex-- == 0)
            {
                break;
            }
        }
    }

    const WeaponMeleeClass* pPrimaryClass =
        static_cast<const WeaponMeleeClass*>(m_pPrimaryWeapon->mClass);

    const unsigned char uiThrowAnim = static_cast<unsigned char>(pPrimaryClass->m_pCombo->m_uiThrowAnimIndex);
    SoldierAnimatorClass::sInstance->GetAnimationBoneMatrix(
        static_cast<SoldierAnimationBank::MAP>(m_pPrimaryWeapon->mSoldierAnimationMap),
        uiThrowAnim, 0x2B960099 /* "hp_weapons" */,
        static_cast<const WeaponMeleeThrowClass*>(mClass)->m_fInitialSalvoDelay, m_mThrow0Local);
}







// FUNCTION: SWBF2 0x0068f570
bool WeaponMeleeThrow::IsMeleeThrow() const
{
    return true;
}

// FUNCTION: SWBF2 0x0068f580
void WeaponMeleeThrow::SetOrdnance(Ordnance*  )
{
}

// FUNCTION: SWBF2 0x0068fca0
void WeaponMeleeThrow::NotifyThrow()
{
    m_bThrow = 1;
}

// FUNCTION: SWBF2 0x0068fcb0
void WeaponMeleeThrow::NotifyReturned()
{
    m_bThrow = 0;
    m_bReturnedThisFrame = 1;
}

// FUNCTION: SWBF2 0x0068f810
bool WeaponMeleeThrow::IsBusy() const
{
    return (mState == FIRE || mState == FIRE2) && (!m_bFired || m_bThrow);
}

// FUNCTION: SWBF2 0x0068f7f0
bool WeaponMeleeThrow::Update(float fDeltaTime)
{
    return Weapon::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x0068f940
void WeaponMeleeThrow::EnterEmpty()
{
    Weapon::EnterEmpty();
}

// FUNCTION: SWBF2 0x0068fa20
bool WeaponMeleeThrow::ReloadDone() const
{
    return Weapon::ReloadDone();
}

// FUNCTION: SWBF2 0x0068f9b0
bool WeaponMeleeThrow::FireDone() const
{
    if (!m_bFired || m_bThrow)
    {
        return false;
    }
    return Weapon::FireDone() ? true : false;
}

// FUNCTION: SWBF2 0x0068f720
WeaponMeleeThrow::~WeaponMeleeThrow()
{
}

// SYNTHETIC: SWBF2 0x0068f590
// WeaponMeleeThrow::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068f730
void WeaponMeleeThrow::Render(const PblMatrix& matrix, const RedPose* pPose,
    const RedColor& color, unsigned int uiFlags, bool bRenderModel) const
{
    uiFlags |= 1;
    if (m_bThrow && pPose)
    {
        const PblMatrix* pThrowMatrix = pPose->Find(0x2B960099);
        if (pThrowMatrix)
        {
            RedPose* pMutablePose = const_cast<RedPose*>(pPose);
            pMutablePose->Remove(0x2B960099);
            m_pPrimaryWeapon->Render(matrix, pPose, color, uiFlags, bRenderModel);
            pMutablePose->Store(0x2B960099, pThrowMatrix);
            return;
        }
    }
    m_pPrimaryWeapon->Render(matrix, pPose, color, uiFlags, bRenderModel);
}

// FUNCTION: SWBF2 0x0068f840
void WeaponMeleeThrow::EnterFire()
{
    Weapon::EnterFire();
    mStateTimer = 0.0f;
    float fEnergyDrain = mClass->mEnergyDrain;
    EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (pEnergy)
    {
        pEnergy->SpendEnergy(fEnergyDrain);
    }
    m_bFired = 0;
    if (mClass->mInstantPlayFireAnim)
    {
        mFiredFlag = 1;
    }
}

// FUNCTION: SWBF2 0x0068f920
void WeaponMeleeThrow::ExitFire()
{
    Weapon::ExitFire();
    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
}

// FUNCTION: SWBF2 0x0068f950
bool WeaponMeleeThrow::CheckFire() const
{
    if (!mTrigger)
    {
        return false;
    }
    const float fEnergyDrain = mClass->mEnergyDrain;
    EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (pEnergy && pEnergy->IsOverheating() && !(fEnergyDrain <= 0.0f))
    {
        return false;
    }
    if (Weapon::CheckOverheat())
    {
        return false;
    }
    return mTrigger->IsPressed();
}

// FUNCTION: SWBF2 0x0068f9d0
bool WeaponMeleeThrow::CheckEmpty() const
{
    const float fEnergyDrain = mClass->mEnergyDrain;
    EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (!pEnergy || !pEnergy->IsOverheating() || fEnergyDrain <= 0.0f)
    {
        return Weapon::CheckEmpty();
    }
    return true;
}

// FUNCTION: SWBF2 0x0068f8a0
bool WeaponMeleeThrow::UpdateFire(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (!m_bFired)
    {
        if (static_cast<const WeaponMeleeThrowClass*>(mClass)->m_fInitialSalvoDelay > mStateTimer)
        {
            return true;
        }
        if (!mClass->mInstantPlayFireAnim)
        {
            mFiredFlag = 1;
        }
        Fire();
        m_bFired = 1;
        mStateTimer = 0.0f;
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x0068fa30
void WeaponMeleeThrow::Fire()
{
    OrdnanceDesc desc;
    desc.mPosition = mAimer->mFirePos;
    desc.mDirection = mAimer->mDirection;
    desc.mStrength = 1.0f;
    desc.mOwner = mOwner->GetGameObject();
    desc.mDamageOwner.mCharacter = mOwner->mCharacter;
    desc.mDamageOwner.mGameObject = PblHandle<GameObject>(mOwner->GetGameObject());
    desc.mDamageOwner.mDamageMultiplier = mOwner->GetOffenseMultiplier();
    desc.mDamageOwner.mTeam = mOwner->GetGameObject()->GetTeam();
    desc.mDamageOwner.mClass = mOwner->GetGameObject()->GetEntityClass();
    desc.mDamageOwner.mWeapon = mClass;
    desc.mTarget = IsLocked() ? const_cast<GameObject*>(static_cast<const GameObject*>(mTarget)) : 0;
    desc.mBodyID = IsLocked() ? mTargetBodyID : -1;
    desc.mFireWeapon = this;

    bool bFlash = true;
    if (NetGame::IsEnabled())
    {
        if (NetGame::IsClient())
        {
            if (!netIsLocalTurn || NetGame::GetJoystickIndex(mOwner->mPlayerId) < 0)
            {
                return;
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
        mClass->CreateMuzzleFlash(mAimer->GetFireMatrix());
    }
    Ordnance* pOrdnance = CreateOrdnance(desc);
    if (mClass->mInstantPlayFireAnim)
    {
        mFiredFlag = 0;
    }
    SetOrdnance(pOrdnance);
}

// FUNCTION: SWBF2 0x0068fc90
Ordnance* WeaponMeleeThrow::CreateOrdnance(OrdnanceDesc& desc)
{
    return mClass->mOrdnanceClass->Build(desc);
}

// SYNTHETIC: SWBF2 0x00407250
// `dynamic initializer for 'WeaponMeleeThrowClass::rttiHashWeaponMeleeThrowClass''
// WeaponMeleeThrowClass::rttiHashWeaponMeleeThrowClass

// GLOBAL: SWBF2 0x01f9a900
// WeaponMeleeThrowClass::rttiHashWeaponMeleeThrowClass
PblHash WeaponMeleeThrowClass::rttiHashWeaponMeleeThrowClass("WeaponMeleeThrowClass");
// GLOBAL: SWBF2 0x0078c1e4
// WeaponMeleeThrowClass::rttiNameWeaponMeleeThrowClass
const char WeaponMeleeThrowClass::rttiNameWeaponMeleeThrowClass[] = "WeaponMeleeThrowClass";

// FUNCTION: SWBF2 0x0068fcd0
WeaponMeleeThrowClass::WeaponMeleeThrowClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    m_iPrimaryWeaponIndex = 0;
    m_fInitialSalvoDelay = 0.0f;
}







// SYNTHETIC: SWBF2 0x0068fd00
// WeaponMeleeThrowClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068fd30
WeaponMeleeThrowClass::WeaponMeleeThrowClass(WeaponMeleeThrowClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    m_iPrimaryWeaponIndex = pBase->m_iPrimaryWeaponIndex;
    m_fInitialSalvoDelay = pBase->m_fInitialSalvoDelay;
}

// FUNCTION: SWBF2 0x0068fd70
WeaponMeleeThrowClass::~WeaponMeleeThrowClass()
{
}

// FUNCTION: SWBF2 0x0068fd80
void WeaponMeleeThrowClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xC8388D9D: /* "InitialSalvoDelay" */
        sscanf(pszValue, "%f", &m_fInitialSalvoDelay);
        break;
    case 0xF83F717B: /* "PrimaryWeaponIndex" */
        m_iPrimaryWeaponIndex = atoi(pszValue);
        break;
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x0068fde0
WeaponClass* WeaponMeleeThrowClass::Derive(unsigned int uHash)
{
    return new WeaponMeleeThrowClass(this, uHash);
}

// FUNCTION: SWBF2 0x0068fe50
Weapon* WeaponMeleeThrowClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponMeleeThrow(this, desc);
}
