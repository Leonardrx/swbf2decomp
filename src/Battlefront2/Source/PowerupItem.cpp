#include "PowerupItem.h"

#include <stdio.h>

#include "Damageable.h"
#include "EntitySoldier.h"
#include "NetGame.h"
#include "PowerupUtility.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// FUNCTION: SWBF2 0x00623cc0
PowerupItemClass::PowerupItemClass(unsigned int uHash)
    : EntityItemClass(uHash)
{
    memset(mAddHealth, 0, sizeof(mAddHealth));
    memset(mAddAmmo, 0, sizeof(mAddAmmo));
    memset(mAddEnergy, 0, sizeof(mAddEnergy));
    mBuffOffenseTimer = 0.0f;
    mBuffOffenseMult = 0.0f;
    mBuffDefenseTimer = 0.0f;
    mBuffDefenseMult = 0.0f;
    mBuffHealthTimer = 0.0f;
    mBuffHealthRate = 0.0f;
    mDebuffDamageTimer = 0.0f;
    mDebuffDamageRate = 0.0f;
}

// SYNTHETIC: SWBF2 0x00405f90
// `dynamic initializer for 'PowerupItemClass::rttiHashPowerupItemClass''
// PowerupItemClass::rttiHashPowerupItemClass

// GLOBAL: SWBF2 0x01efa1d0
// PowerupItemClass::rttiHashPowerupItemClass
PblHash PowerupItemClass::rttiHashPowerupItemClass("PowerupItemClass");
// GLOBAL: SWBF2 0x0078abe0
// PowerupItemClass::rttiNamePowerupItemClass
const char PowerupItemClass::rttiNamePowerupItemClass[] = "PowerupItemClass";







// SYNTHETIC: SWBF2 0x00623d80
// PowerupItemClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00623db0
PowerupItemClass::PowerupItemClass(PowerupItemClass* pBase, unsigned int uHash)
    : EntityItemClass(pBase, uHash)
{
    memcpy(mAddHealth, pBase->mAddHealth, sizeof(mAddHealth));
    memcpy(mAddAmmo, pBase->mAddAmmo, sizeof(mAddAmmo));
    memcpy(mAddEnergy, pBase->mAddEnergy, sizeof(mAddEnergy));
    mBuffOffenseTimer = pBase->mBuffOffenseTimer;
    mBuffOffenseMult = pBase->mBuffOffenseMult;
    mBuffDefenseTimer = pBase->mBuffDefenseTimer;
    mBuffDefenseMult = pBase->mBuffDefenseMult;
    mBuffHealthTimer = pBase->mBuffHealthTimer;
    mBuffHealthRate = pBase->mBuffHealthRate;
    mDebuffDamageTimer = pBase->mDebuffDamageTimer;
    mDebuffDamageRate = pBase->mDebuffDamageRate;
    mPowerupSound = pBase->mPowerupSound;
    mAmbientSound = pBase->mAmbientSound;
}

// FUNCTION: SWBF2 0x00623f00
PowerupItemClass::~PowerupItemClass()
{
}

// FUNCTION: SWBF2 0x00623f10
void PowerupItemClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float f;
    switch (uProperty)
    {
    case 0xf33b41df:
        sscanf(pszValue, "%f", &f);
        mAddHealth[HEALTH_TYPE_PERSON] = f;
        mAddHealth[HEALTH_TYPE_DROID] = f;
        break;
    case 0xf51ad206:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_PERSON]);
        break;
    case 0x60d746d7:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x9622c73b:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_DROID]);
        break;
    case 0x14e381c9:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xa6c59e9f:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_BUILDING]);
        break;
    case 0xc3da529f:
        sscanf(pszValue, "%f", &f);
        mAddAmmo[HEALTH_TYPE_PERSON] = f;
        mAddAmmo[HEALTH_TYPE_DROID] = f;
        break;
    case 0x738f412e:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_PERSON]);
        break;
    case 0xbfe8e397:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x0fda4dbb:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_DROID]);
        break;
    case 0xe728d955:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xa25f865f:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_BUILDING]);
        break;
    case 0x0adc5145:
        sscanf(pszValue, "%f", &f);
        mAddEnergy[HEALTH_TYPE_PERSON] = f;
        mAddEnergy[HEALTH_TYPE_DROID] = f;
        break;
    case 0x84622134:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_PERSON]);
        break;
    case 0xfb0507ad:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x3fe43479:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_DROID]);
        break;
    case 0x56238a9b:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xda2c3905:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_BUILDING]);
        break;
    case 0x70b204af:
        sscanf(pszValue, "%f", &mBuffOffenseTimer);
        break;
    case 0xce532e64:
        sscanf(pszValue, "%f", &mBuffOffenseMult);
        break;
    case 0x7705fcc1:
        sscanf(pszValue, "%f", &mBuffDefenseTimer);
        break;
    case 0xb25fb62e:
        sscanf(pszValue, "%f", &mBuffDefenseMult);
        break;
    case 0x460b143b:
        sscanf(pszValue, "%f", &mBuffHealthTimer);
        break;
    case 0xb6639618:
        sscanf(pszValue, "%f", &mBuffHealthRate);
        break;
    case 0x805b51e7:
        sscanf(pszValue, "%f", &mDebuffDamageTimer);
        break;
    case 0x9834a7b4:
        sscanf(pszValue, "%f", &mDebuffDamageRate);
        break;
    case 0xa2a034ea:
        mPowerupSound.SetID(pszValue, false, false);
        break;
    case 0x65a920ba:
        mAmbientSound.SetID(pszValue, false, false);
        break;
    default:
        EntityItemClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x006243a0
EntityClass* PowerupItemClass::Derive(unsigned int uHash)
{
    return new PowerupItemClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x00623990
// PowerupItem::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x006244ba
// ??_EPowerupItem@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x006244c5
// ??_EPowerupItem@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x006244cd
// ??_EPowerupItem@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x006244e3
// ??_EPowerupItem@@WJE@AEPAXI@Z

// SYNTHETIC: SWBF2 0x006244d8
// ??_EPowerupItem@@WCEA@AEPAXI@Z

// SYNTHETIC: SWBF2 0x00405f60
// `dynamic initializer for 'PowerupItem::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769340
// `dynamic atexit destructor for 'PowerupItem::sMemoryPool''

// GLOBAL: SWBF2 0x01efa1d8
// PowerupItem::sMemoryPool
MemoryPool PowerupItem::sMemoryPool("PowerupItem", sizeof(PowerupItem));

// SYNTHETIC: SWBF2 0x00405f80
// `dynamic initializer for 'PowerupItem::rttiHashPowerupItem''
// PowerupItem::rttiHashPowerupItem

// GLOBAL: SWBF2 0x01efa1d4
// PowerupItem::rttiHashPowerupItem
PblHash PowerupItem::rttiHashPowerupItem("PowerupItem");
// GLOBAL: SWBF2 0x0078ac00
// PowerupItem::rttiNamePowerupItem
const char PowerupItem::rttiNamePowerupItem[] = "PowerupItem";

class CollisionManager
{
public:
    static bool AddItemBody(CollisionObject* pObject);
};

// FUNCTION: SWBF2 0x00624410
Entity* PowerupItemClass::Build(const EntityDesc& desc)
{
    if (NetGame::IsPreGame())
    {
        return 0;
    }
    if (PowerupItem::sMemoryPool.mUsed >= PowerupItem::sMemoryPool.mCount)
    {
        return 0;
    }

    PowerupItem* pItem = new PowerupItem(this, desc);
    if (pItem)
    {
        pItem->AddItemToCollisionManager();
        pItem->ActivatePhysics();
        pItem->BuildEffects();
    }
    return pItem;
}

// FUNCTION: SWBF2 0x006238f0
PowerupItem::PowerupItem(PowerupItemClass* pClass, const EntityDesc& desc)
    : EntityItem(pClass, desc)
{
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x00623870
EntityType PowerupItem::GetEntityType() const
{
    return ENT_POWERUP;
}







// FUNCTION: SWBF2 0x00623880
bool PowerupItem::AddToTeamManager() const
{
    return false;
}

// FUNCTION: SWBF2 0x006239c0
PowerupItem::~PowerupItem()
{
    NetGame::Destroyed(this);
    mAmbientSound.Stop(true);
}

// FUNCTION: SWBF2 0x00623a60
bool PowerupItem::Update(float fDeltaTime)
{
    const PblVector3* pPos = (const PblVector3*)mMatrix.m[3];
    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && pPos->y < -500.0f)
    {
        Kill();
        return false;
    }

    if (mAmbientSound.GetPlaying())
    {
        mAmbientSound.Update(&static_cast<const PowerupItemClass*>(mClass)->mAmbientSound, pPos, &mVelocity);
    }
    else
    {
        mAmbientSound.Play(&static_cast<const PowerupItemClass*>(mClass)->mAmbientSound, pPos, &mVelocity, true);
    }
    return EntityItem::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x00623b10
bool PowerupItem::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                    Restrictor* pRestrictor)
{
    if (IsAlive() && (mStationary || mClass->mLifeSpan - mLifeTimer > 0.5f))
    {
        GameObject* pObject = pOther->GetGameObject();
        if (pObject && pObject->IsAlive())
        {
            const PowerupItemClass* pClass = static_cast<const PowerupItemClass*>(mClass);
            int iHealthType = pObject->mHealthType;
            float fHealth = pClass->mAddHealth[iHealthType];
            float fAmmo = pClass->mAddAmmo[iHealthType];
            float fEnergy = pClass->mAddEnergy[iHealthType];

            bool bUsed = PowerupUtility::ApplyBuff(pObject, pClass->mBuffOffenseTimer, pClass->mBuffOffenseMult,
                                                   pClass->mBuffDefenseTimer, pClass->mBuffDefenseMult,
                                                   pClass->mBuffHealthTimer, pClass->mBuffHealthRate,
                                                   pClass->mDebuffDamageTimer, pClass->mDebuffDamageRate) |
                         PowerupUtility::ApplyHealth(pObject, fHealth, 0, false) |
                         PowerupUtility::ApplyAmmo(pObject, fAmmo, true) |
                         PowerupUtility::ApplyEnergy(pObject, fEnergy);
            if (bUsed)
            {
                const PblVector3& vPos = GetPosition();
                if (pObject->GetEntityType() != ENT_SOLDIER ||
                    !static_cast<EntitySoldier*>(pObject)->mMarkedForDeath)
                {
                    static_cast<const PowerupItemClass*>(mClass)->mPowerupSound.Play(&vPos, 0, false, true);
                }
                InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
                mIsAlive = false;
                return true;
            }
        }
    }
    return EntityItem::CollisionCallback(pResult, pOther, pRestrictor);
}
