#include "AmmoCounter.h"
#include <stdio.h>

#include "WeaponRepair.h"

#include "Aimer.h"
#include "Character.h"
#include "CommandFlyer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntityItem.h"
#include "EntitySoldier.h"
#include "EventManager_Object.h"
#include "GameObject.h"
#include "MedalsMgr.h"
#include "NetGame.h"
#include "PowerupUtility.h"
#include "StatusDisplay.h"
#include "Trigger.h"

namespace GameLoop
{
extern float sClientDeltaTime;
}


class EntityWalker { public: static PblHash rttiHashEntityWalker; };
class EntityHover { public: static PblHash rttiHashEntityHover; };
class EntityMine { public: static PblHash rttiHashEntityMine; };
class CommandWalker { public: static PblHash rttiHashCommandWalker; };

namespace EventManager
{
namespace ObjectEvent
{
extern Event<GameObject, DamageOwner> mRepair;
extern Event<GameObject, DamageOwner> mHack;
}
}

// SYNTHETIC: SWBF2 0x00407290
// `dynamic initializer for 'WeaponRepairClass::rttiHashWeaponRepairClass''
// WeaponRepairClass::rttiHashWeaponRepairClass

// GLOBAL: SWBF2 0x01f9a90c
// WeaponRepairClass::rttiHashWeaponRepairClass
PblHash WeaponRepairClass::rttiHashWeaponRepairClass("WeaponRepairClass");
// GLOBAL: SWBF2 0x0078c254
// WeaponRepairClass::rttiNameWeaponRepairClass
const char WeaponRepairClass::rttiNameWeaponRepairClass[] = "WeaponRepairClass";

// FUNCTION: SWBF2 0x006910c0
WeaponRepairClass::WeaponRepairClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mTriggerSingle = false;
    mTargetSides = 6;
    mTargetTypes = 0;
    mAITargetTypes = 0;

    mShotDelay = 0.125f;
    memset(mAddHealth, 0, sizeof(mAddHealth));
    memset(mAddAmmo, 0, sizeof(mAddAmmo));
}

// FUNCTION: SWBF2 0x00691160
WeaponRepairClass::WeaponRepairClass(WeaponRepairClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mShotDelay = pBase->mShotDelay;
    memcpy(mAddHealth, pBase->mAddHealth, sizeof(mAddHealth));
    memcpy(mAddAmmo, pBase->mAddAmmo, sizeof(mAddAmmo));
}

// FUNCTION: SWBF2 0x006911d0
WeaponRepairClass::~WeaponRepairClass()
{
}

// SYNTHETIC: SWBF2 0x00691130
// WeaponRepairClass::`scalar deleting destructor'







// FUNCTION: SWBF2 0x006911e0
void WeaponRepairClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float fValue;

    switch (uProperty)
    {
    case 0xd9356908:
        sscanf(pszValue, "%f", &mShotDelay);
        return;
    case 0xf51ad206:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[0] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 1;
            mAITargetTypes |= 1;
        }
        return;
    case 0x60d746d7:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[1] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 2;
            mAITargetTypes |= 2;
        }
        return;
    case 0x9622c73b:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[2] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 4;
            mAITargetTypes |= 4;
        }
        return;
    case 0xf33b41df:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[0] = fValue;
        mAddHealth[2] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 5;
            mAITargetTypes |= 5;
        }
        return;
    case 0x14e381c9:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[3] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 8;
            mAITargetTypes |= 8;
        }
        return;
    case 0xa6c59e9f:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[4] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 0x10;
            mAITargetTypes |= 0x10;
        }
        return;
    case 0x03ec6220:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[5] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 0x20;
            mAITargetTypes |= 0x20;
        }
        return;
    case 0x09cc7f53:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[6] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 0x40;
            mAITargetTypes |= 0x40;
        }
        return;
    case 0xf5ed2064:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[7] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 0x80;
            mAITargetTypes |= 0x80;
        }
        return;
    case 0x738f412e:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[0] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 1;
            mAITargetTypes |= 1;
        }
        return;
    case 0xbfe8e397:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[1] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 2;
            mAITargetTypes |= 2;
        }
        return;
    case 0x0fda4dbb:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[2] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 4;
            mAITargetTypes |= 4;
        }
        return;
    case 0xc3da529f:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[0] = fValue;
        mAddAmmo[2] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 5;
            mAITargetTypes |= 5;
        }
        return;
    case 0xe728d955:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[3] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 8;
            mAITargetTypes |= 8;
        }
        return;
    case 0xa25f865f:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[4] = fValue;
        if (fValue != 0.0f)
        {
            mTargetTypes |= 0x10;
            mAITargetTypes |= 0x10;
        }
        return;
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        return;
    }
}

// FUNCTION: SWBF2 0x00691620
WeaponClass* WeaponRepairClass::Derive(unsigned int uHash)
{
    return new WeaponRepairClass(this, uHash);
}

// FUNCTION: SWBF2 0x00691690
Weapon* WeaponRepairClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponRepair(this, desc);
}

// FUNCTION: SWBF2 0x00690640
WeaponRepair::WeaponRepair(const WeaponRepairClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
}

// SYNTHETIC: SWBF2 0x00407280
// `dynamic initializer for 'WeaponRepair::rttiHashWeaponRepair''
// WeaponRepair::rttiHashWeaponRepair

// GLOBAL: SWBF2 0x01f9a910
// WeaponRepair::rttiHashWeaponRepair
PblHash WeaponRepair::rttiHashWeaponRepair("WeaponRepair");
// GLOBAL: SWBF2 0x0078c23c
// WeaponRepair::rttiNameWeaponRepair
const char WeaponRepair::rttiNameWeaponRepair[] = "WeaponRepair";







// FUNCTION: SWBF2 0x00690830
bool WeaponRepair::IsBusy() const
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

// FUNCTION: SWBF2 0x00690810
bool WeaponRepair::Update(float fDeltaTime)
{
    return Weapon::Update(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x00690660
// WeaponRepair::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00690690
WeaponRepair::~WeaponRepair()
{
    StatusDisplay::Instance(NetGame::GetJoystickIndex(mOwner->mInitialPlayerId))->SetRepairWeapon(0);
}

// FUNCTION: SWBF2 0x006906f0
bool WeaponRepair::IsLocked() const
{
    GameObject* pTarget = const_cast<GameObject*>(static_cast<const GameObject*>(mTarget));
    if (!pTarget)
    {
        pTarget = mOwner->GetGameObject();
    }
    if (pTarget)
    {
        float fHealth = static_cast<const WeaponRepairClass*>(mClass)->mAddHealth[pTarget->mHealthType];
        float fAmmo = static_cast<const WeaponRepairClass*>(mClass)->mAddAmmo[pTarget->mHealthType];
        int iPilotType;
        int bHack = pTarget->GetControllable() &&
                    ((iPilotType = pTarget->GetControllable()->mPilotType) == PILOT_VEHICLE ||
                     iPilotType == PILOT_VEHICLESELF ||
                     pTarget->GetControllable()->IsRemoteVehicle()) &&
                    mOwner->GetGameObject() && mOwner->GetGameObject()->IsMyEnemy(pTarget);

        int bAmmo = PowerupUtility::CheckAmmo(pTarget, fAmmo, false);
        int bHealth = PowerupUtility::CheckHealth(pTarget, fHealth);
        if (bHealth | bAmmo | bHack)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00690870
void WeaponRepair::EnterFire()
{
    Weapon::EnterFire();
    mStateTimer = 0.0f;
    mStateLimit = static_cast<const WeaponRepairClass*>(mClass)->mShotDelay;
    mFired = false;
    if (mClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x006908c0
bool WeaponRepair::UpdateFire(float fDeltaTime)
{
    if (mStateLimit <= 0.0f)
    {
        mFired = true;
        if (Fire(fDeltaTime))
        {
            m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot * fDeltaTime);
            m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot * fDeltaTime);
            mAimer->UpdateFire(mAimer, true);
        }
    }
    else
    {
        float fTimeLeft = fDeltaTime;
        while (fTimeLeft > 0.0f)
        {
            if (mFired)
            {
                float fStep = mStateLimit - mStateTimer;
                if (fStep > fTimeLeft)
                {
                    fStep = fTimeLeft;
                }
                fTimeLeft -= fStep;
                mStateTimer += fStep;
                if (!(mStateTimer >= mStateLimit) || mClass->mTriggerSingle || !CheckFire() ||
                    CheckReload() || CheckEmpty())
                {
                    break;
                }
                mStateTimer -= mStateLimit;
                Weapon::ExitFire();
                Weapon::EnterFire();
                mFired = false;
            }
            else
            {
                mFired = true;
                if (Fire(1.0f))
                {
                    m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot * fDeltaTime);
                    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot * fDeltaTime);
                    mAimer->UpdateFire(mAimer, true);
                }
            }
        }
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00690aa0
void WeaponRepair::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00690ab0
bool WeaponRepair::CheckFire() const
{
    if (NetGame::IsPreGame())
    {
        return false;
    }
    if (!mTrigger)
    {
        return false;
    }
    if (CheckOverheat())
    {
        return false;
    }
    if (!IsLocked())
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00690af0
bool WeaponRepair::FireDone() const
{
    return mFired && mStateTimer >= mStateLimit &&
           (mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty());
}

// FUNCTION: SWBF2 0x00690b50
bool WeaponRepair::Fire(float fDeltaTime)
{
    GameObject* pTarget = const_cast<GameObject*>(static_cast<const GameObject*>(mTarget));
    if (!pTarget)
    {
        pTarget = mOwner->GetGameObject();
    }

    mClass->CreateMuzzleFlash(mFirePointMatrix);

    if (!pTarget)
    {
        return false;
    }

    float fHealth = static_cast<const WeaponRepairClass*>(mClass)->mAddHealth[pTarget->mHealthType] * fDeltaTime;
    float fAmmo = static_cast<const WeaponRepairClass*>(mClass)->mAddAmmo[pTarget->mHealthType] * fDeltaTime;

    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    owner.mCharacter = mOwner->mCharacter;
    owner.mGameObject = PblHandle<GameObject>(mOwner->GetGameObject());
    owner.mTeam = mOwner->GetGameObject()->mTeam;
    owner.mClass = mOwner->GetGameObject()->GetEntityClass();
    owner.mWeapon = mClass;

    if (pTarget->IsRtti(EntityWalker::rttiHashEntityWalker) ||
        pTarget->IsRtti(EntityHover::rttiHashEntityHover) ||
        pTarget->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        if (mOwner->GetGameObject()->IsMyEnemy(pTarget) &&
            pTarget->GetEntityType() != ENT_TAUNTAUN &&
            !pTarget->IsRtti(CommandFlyer::rttiHashCommandFlyer) &&
            !pTarget->IsRtti(CommandWalker::rttiHashCommandWalker))
        {
            int iSoldiers = 0;
            const PblList<Controllable>& list = pTarget->GetControllableList();
            for (PblList<Controllable>::Itor it(list); (Controllable*)it; ++it)
            {
                if (it->GetPilot() && it->GetPilot()->GetGameObject() &&
                    it->GetPilot()->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
                {
                    ++iSoldiers;
                }
            }

            if (iSoldiers && pTarget->GetGameObjectClass()->GetTimeRequiredToEject() > 0.0f)
            {
                const float fEjectTimer = pTarget->GetEjectTimer();
                pTarget->SetEjectTimer(static_cast<const WeaponRepairClass*>(mClass)->mShotDelay + fEjectTimer);
                pTarget->ResetTimeSinceLastHack();
                if (pTarget->GetEjectTimer() > pTarget->GetGameObjectClass()->GetTimeRequiredToEject())
                {
                    EventManager::ObjectEvent::mHack.Trigger(*pTarget, owner);

                    for (PblList<Controllable>::Itor it(list); (Controllable*)it; ++it)
                    {
                        if (it->GetPilot() && it->GetPilot()->GetGameObject() &&
                            it->GetPilot()->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
                        {
                            EntitySoldier* pSoldier = static_cast<EntitySoldier*>(it->GetPilot());
                            Character* pCharacter = pSoldier->mCharacter;
                            if (pCharacter)
                            {
                                if (pCharacter->mRemote)
                                {
                                    pSoldier->ExitRemote();
                                }
                                if (pCharacter->GetVehicle())
                                {
                                    pSoldier->ExitVehicle(false, true);
                                }
                            }
                        }
                    }

                    pTarget->SetEjectTimer(0.0f);
                    pTarget->mDisableTime = pTarget->GetGameObjectClass()->GetReboardTime();
                    if (mClass->mScoreForMedalsType >= 0 && mOwner->mCharacter &&
                        mOwner->mCharacter->mMedalsMgr)
                    {
                        mOwner->mCharacter->mMedalsMgr->AddMedalPoints(
                            mClass->mScoreForMedalsType, false, false, (HealthType)pTarget->mHealthType,
                            true, false, 1);
                    }
                }
                return true;
            }
        }
    }
    else if (pTarget->IsRtti(EntityMine::rttiHashEntityMine))
    {
        EntityItem* pMine = static_cast<EntityItem*>(pTarget);
        if (pMine->GetLifeTimer() > 0.0f)
        {
            pMine->mLifeTimer += fHealth;
            return true;
        }
        return false;
    }

    bool bHealth = PowerupUtility::ApplyHealth(pTarget, fHealth, &owner, false);
    if (bHealth | PowerupUtility::ApplyAmmo(pTarget, fAmmo, false))
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && pTarget->mCurHealth == pTarget->mMaxHealth)
        {
            EventManager::ObjectEvent::mRepair.Trigger(*pTarget, owner);
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00691060
void WeaponRepair::Select(unsigned int uFlags, bool bOffhand)
{
    StatusDisplay::Instance(NetGame::GetJoystickIndex(mOwner->mPlayerId))->SetRepairWeapon(this);
    Weapon::Select(uFlags, bOffhand);
}

// FUNCTION: SWBF2 0x00691090
void WeaponRepair::Deselect(unsigned int uFlags, bool bOffhand)
{
    StatusDisplay::Instance(NetGame::GetJoystickIndex(mOwner->mPlayerId))->SetRepairWeapon(0);
    Weapon::Deselect(uFlags, bOffhand);
}
