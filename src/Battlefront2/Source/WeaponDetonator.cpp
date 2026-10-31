#include "WeaponDetonator.h"
#include "Aimer.h"
#include "Character.h"
#include "Controllable.h"
#include "GameObject.h"
#include "EntityItem.h"
#include "EntityPortableTurret.h"
#include "UnitController.h"
#include "UnitStationaryController.h"
#include "EventManager_Character.h"
#include "Trigger.h"
#include "PebbleFL/Common/PblVector.h"


// SYNTHETIC: SWBF2 0x00407110
// `dynamic initializer for 'WeaponDetonatorClass::rttiHashWeaponDetonatorClass''
// WeaponDetonatorClass::rttiHashWeaponDetonatorClass

// GLOBAL: SWBF2 0x01f9a8ac
// WeaponDetonatorClass::rttiHashWeaponDetonatorClass
PblHash WeaponDetonatorClass::rttiHashWeaponDetonatorClass("WeaponDetonatorClass");
// GLOBAL: SWBF2 0x0078bf14
// WeaponDetonatorClass::rttiNameWeaponDetonatorClass
const char WeaponDetonatorClass::rttiNameWeaponDetonatorClass[] = "WeaponDetonatorClass";

// SYNTHETIC: SWBF2 0x00407100
// `dynamic initializer for 'WeaponDetonator::rttiHashWeaponDetonator''
// WeaponDetonator::rttiHashWeaponDetonator

// GLOBAL: SWBF2 0x01f9a8b0
// WeaponDetonator::rttiHashWeaponDetonator
PblHash WeaponDetonator::rttiHashWeaponDetonator("WeaponDetonator");
// GLOBAL: SWBF2 0x0078bf30
// WeaponDetonator::rttiNameWeaponDetonator
const char WeaponDetonator::rttiNameWeaponDetonator[] = "WeaponDetonator";





// FUNCTION: SWBF2 0x00682600
unsigned int WeaponDetonator::GetNameHash() const
{
    const WeaponDetonatorClass* pClass = static_cast<const WeaponDetonatorClass*>(mClass);
    if (pClass && HasItemsDispensed() && pClass->mPlantedHash)
    {
        return pClass->mPlantedHash;
    }
    return Weapon::GetNameHash();
}


// FUNCTION: SWBF2 0x00682aa0
WeaponDetonatorClass::WeaponDetonatorClass(unsigned int uHash)
    : WeaponDispenserClass(uHash)
{
    mPlantedHash = 0x2E4D0225u;
    mMaxItems = 1;
}

// SYNTHETIC: SWBF2 0x00682ad0
// WeaponDetonatorClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00682b00
WeaponDetonatorClass::WeaponDetonatorClass(WeaponDetonatorClass* pBase, unsigned int uHash)
    : WeaponDispenserClass(pBase, uHash)
{
    mPlantedHash = pBase->mPlantedHash;
}







// FUNCTION: SWBF2 0x00682b40
void WeaponDetonatorClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty != 0x83CF5063 /* "PlantedHash" */)
    {
        WeaponDispenserClass::SetProperty(uProperty, pszValue);
        return;
    }
    mPlantedHash = PblHash(pszValue);
}

// FUNCTION: SWBF2 0x00682b80
WeaponClass* WeaponDetonatorClass::Derive(unsigned int uHash)
{
    return new WeaponDetonatorClass(this, uHash);
}

// FUNCTION: SWBF2 0x00682bf0
WeaponDispenser* WeaponDetonatorClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponDetonator(this, desc);
}

// FUNCTION: SWBF2 0x00682b30
WeaponDetonatorClass::~WeaponDetonatorClass()
{
}



// FUNCTION: SWBF2 0x00682500
WeaponDetonator::WeaponDetonator(const WeaponDetonatorClass* pClass, const WeaponDesc& desc)
    : WeaponDispenser(pClass, desc)
{
}

// FUNCTION: SWBF2 0x006827d0
void WeaponDetonator::NetAttachItem(Entity* pAttach)
{
    WeaponDetonator* pWeapon = this;
    EntityItem* pItem = static_cast<EntityItem*>(pAttach);
    WeaponDispenserClass* pClass = (WeaponDispenserClass*)pWeapon->mClass;
    if (!pClass->mDispenseClass || !pWeapon->mItemArray || !pWeapon->mItemCount || !pItem)
    {
        return;
    }
    GameObject* pObject = pItem->GetGameObject();
    if (!pObject)
    {
        return;
    }

    const float strength = (pClass->mMaxStrength - pClass->mMinStrength) *
                           pWeapon->mStrength + pClass->mMinStrength;
    GameObject* pOwnerObject = pWeapon->mOwner->GetGameObject();
    const PblVector3& ownerPosition = pOwnerObject->GetPosition();
    const PblVector3& direction = pWeapon->mAimer->GetDirection();
    const float scale = pClass->mVelocity * strength;
    PblVector3 firePosition(ownerPosition.x + direction.x * scale,
                            ownerPosition.y + direction.y * scale,
                            ownerPosition.z + direction.z * scale);
    pObject->SetPosition(firePosition);
    Character* pCharacter = pWeapon->mOwner->mCharacter;
    const int team = pCharacter->mTeamNumber;
    pObject->SetTeam(team);
    pObject->SetPerceivedTeam(team);
    Entity* pEntity = pObject->GetEntity();
    if (pEntity->IsRtti(EntityPortableTurret::rttiHashEntityPortableTurret))
    {
        static_cast<EntityPortableTurret*>(pObject)->mOwnerChar = pCharacter;
    }
    pObject->Init();

    if (pObject->IsRtti(EntityItem::rttiHashEntityItem))
    {
        pItem->mOwner = pCharacter;
        EventManager::CharacterEvent::mDispensePowerup.Trigger(*pCharacter, *pObject);
    }
    else
    {
        EventManager::CharacterEvent::mDispenseControllable.Trigger(*pCharacter, *pObject);
        Controllable* pControllable = pObject->GetControllable();
        if (pControllable && pControllable->IsSelfPilotable() && !pControllable->GetPilot())
        {
            Controller* pController = pControllable->mCtrl;
            if (pController && pController->IsRtti(UnitController::rttiHashUnitController) &&
                !pController->IsRtti(UnitStationaryController::rttiHashUnitStationaryController))
            {
                const GameObject* pTarget = pWeapon->mTarget;
                if (!pTarget)
                {
                    pTarget = pOwnerObject;
                }
                PblHandle<const GameObject> who(pTarget);
                PblHandle<const GameObject> owner(pOwnerObject);
                DroidRepairInfo info(who, owner);
                CommandFsm* pFsm = static_cast<UnitController*>(pController);
                pFsm->ForceCommand(info);
            }
        }
    }

    if (pClass->mMaxItems > 0)
    {
        const int index = (*pWeapon->mItemCount)++;
        (*pWeapon->mItemArray)[index] = pObject;
    }
}

// FUNCTION: SWBF2 0x00682550
WeaponDetonator::~WeaponDetonator()
{
}

// SYNTHETIC: SWBF2 0x00682520
// WeaponDetonator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00682560
bool WeaponDetonator::HasItemsDispensed() const
{
    Character* pCharacter = mOwner->mCharacter;
    if (!pCharacter)
    {
        return false;
    }
    const int iIndex = pCharacter - Character::sCharacters;
    for (WeaponDispenserClass* pClass = (WeaponDispenserClass*)mStart; pClass;
         pClass = (WeaponDispenserClass*)pClass->mNextCharge)
    {
        if (pClass->mTrackItemCount[iIndex] > 0)
        {
            return true;
        }
        if (pClass == pClass->mNextCharge || pClass->mNextCharge == mStart)
        {
            break;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006825c0
bool WeaponDetonator::CheckCharge() const
{
    if (HasItemsDispensed())
    {
        return false;
    }
    return WeaponDispenser::CheckCharge();
}

// FUNCTION: SWBF2 0x006825e0
bool WeaponDetonator::CheckFire() const
{
    if (HasItemsDispensed())
    {
        return false;
    }
    return WeaponDispenser::CheckFire();
}

// FUNCTION: SWBF2 0x00682630
bool WeaponDetonator::Update(float fDeltaTime)
{
    const bool bResult = WeaponDispenser::Update(fDeltaTime);
    if (mSelectedFlag && mTrigger && mTrigger->IsPressed() && HasItemsDispensed())
    {
        const int iIndex = mOwner->mCharacter - Character::sCharacters;
        for (WeaponDispenserClass* pClass = (WeaponDispenserClass*)mStart; pClass;)
        {
            for (int i = 0; i < pClass->mTrackItemCount[iIndex]; ++i)
            {
                if (pClass->mTrackItemArray[iIndex][i] && pClass->mTrackItemArray[iIndex][i]->IsAlive())
                {
                    pClass->mTrackItemArray[iIndex][i]->Kill();
                }
            }
            pClass->mTrackItemCount[iIndex] = 0;

            WeaponDispenserClass* pNext = (WeaponDispenserClass*)pClass->mNextCharge;
            if (pNext == pClass || pNext == (WeaponDispenserClass*)mStart)
            {
                break;
            }
            pClass = pNext;
        }
        mTrigger->ClearUntilPressed();
    }
    return bResult;
}
