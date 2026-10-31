#include <stdlib.h>

#include "WeaponRemote.h"
#include "Controllable.h"
#include "Damageable.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Trigger.h"

// SYNTHETIC: SWBF2 0x00407270
// `dynamic initializer for 'WeaponRemoteClass::rttiHashWeaponRemoteClass''
// WeaponRemoteClass::rttiHashWeaponRemoteClass

// GLOBAL: SWBF2 0x01f9a904
// WeaponRemoteClass::rttiHashWeaponRemoteClass
PblHash WeaponRemoteClass::rttiHashWeaponRemoteClass("WeaponRemoteClass");
// GLOBAL: SWBF2 0x0078c208
// WeaponRemoteClass::rttiNameWeaponRemoteClass
const char WeaponRemoteClass::rttiNameWeaponRemoteClass[] = "WeaponRemoteClass";

// SYNTHETIC: SWBF2 0x00407260
// `dynamic initializer for 'WeaponRemote::rttiHashWeaponRemote''
// WeaponRemote::rttiHashWeaponRemote

// GLOBAL: SWBF2 0x01f9a908
// WeaponRemote::rttiHashWeaponRemote
PblHash WeaponRemote::rttiHashWeaponRemote("WeaponRemote");
const char WeaponRemote::rttiNameWeaponRemote[] = "WeaponRemote";







// FUNCTION: SWBF2 0x006903b0
WeaponRemoteClass::WeaponRemoteClass(unsigned int uHash)
    : WeaponDispenserClass(uHash)
{
    mSwitchImmediately = false;
    mAllowDestruct = true;
}

// SYNTHETIC: SWBF2 0x006903e0
// WeaponRemoteClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00690410
WeaponRemoteClass::WeaponRemoteClass(WeaponRemoteClass* pBase, unsigned int uHash)
    : WeaponDispenserClass(pBase, uHash)
{
    mSwitchImmediately = pBase->mSwitchImmediately;
    mAllowDestruct = pBase->mAllowDestruct;
}

// FUNCTION: SWBF2 0x00690450
WeaponRemoteClass::~WeaponRemoteClass()
{
}

// FUNCTION: SWBF2 0x00690460
void WeaponRemoteClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xC43B77DD: /* "SwitchImmediately" */
        mSwitchImmediately = atoi(pszValue) != 0;
        break;

    case 0xED422F60: /* "AllowDestruct" */
        mAllowDestruct = atoi(pszValue) != 0;
        break;

    default:
        WeaponDispenserClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x006904c0
WeaponDispenserClass* WeaponRemoteClass::Derive(unsigned int uHash)
{
    return new WeaponRemoteClass(this, uHash);
}

// FUNCTION: SWBF2 0x00690530
WeaponDispenser* WeaponRemoteClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponRemote(this, desc);
}







// FUNCTION: SWBF2 0x0068ff60
WeaponRemote::WeaponRemote(const WeaponRemoteClass* pClass, const WeaponDesc& desc)
    : WeaponDispenser(pClass, desc)
{
}

// FUNCTION: SWBF2 0x006900b0
bool WeaponRemote::Update(float fDeltaTime)
{
    if (mSelectedFlag && mItem && !(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        Controllable* pPilot;
        if (mTrigger->IsPressed() && (pPilot = mOwner->mPilot) != 0)
        {
            if (mOwner->mPlayerId >= 0)
            {
                pPilot->EnterRemote(mItem);
            }
        }
        else if (mReload && mReload->IsPressed() && ((const WeaponRemoteClass*)mClass)->mAllowDestruct)
        {
            Controllable* pOwner = mOwner;
            Character* pCharacter = pOwner->mCharacter;
            DamageOwner owner(pCharacter, pOwner->GetGameObject(), mItem->GetTeam(),
                              mItem->GetEntityClass(), mClass, 1.0f);
            mItem->InstantDeath(owner, false);
        }
    }
    return WeaponDispenser::Update(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x0068ffa0
// WeaponRemote::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068ffd0
WeaponRemote::~WeaponRemote()
{
    if (mItem)
    {
        mItem->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
    }
}

// FUNCTION: SWBF2 0x00690220
bool WeaponRemote::IsBusy() const
{
    return WeaponDispenser::IsBusy();
}

// FUNCTION: SWBF2 0x00690230
bool WeaponRemote::CheckCharge() const
{
    if (mItem)
    {
        return false;
    }
    return WeaponDispenser::CheckCharge();
}

// FUNCTION: SWBF2 0x00690270
bool WeaponRemote::CheckFire() const
{
    if (mItem)
    {
        return false;
    }
    return WeaponDispenser::CheckFire();
}

// FUNCTION: SWBF2 0x006902b0
bool WeaponRemote::FireDone() const
{
    return mFired && mStateTimer >= mStateLimit;
}

// FUNCTION: SWBF2 0x006902d0
Entity* WeaponRemote::Fire()
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }

    Entity* pEntity = WeaponDispenser::Fire();
    if (pEntity)
    {
        if (GameObject* pObject = pEntity->GetGameObject())
        {
            if (!mItem)
            {
                mItem = pObject;
            }
            if (static_cast<const WeaponRemoteClass*>(mClass)->mSwitchImmediately && mItem)
            {
                if (Controllable* pPilot = mOwner->mPilot)
                {
                    pPilot->EnterRemote(mItem);
                }
            }
        }
    }
    return pEntity;
}
