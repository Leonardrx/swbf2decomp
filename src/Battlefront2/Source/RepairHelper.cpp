#include "AmmoCounter.h"
#include <float.h>
#include <math.h>

#include "RepairHelper.h"

#include "AILowLevel.h"
#include "AIUtil.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "PowerupItem.h"
#include "Team.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "WeaponDispenser.h"
#include "WeaponRepair.h"
#include "PebbleFL/Common/PblMatrix.h"

namespace AI
{

enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};

enum VOType
{
    VOT_RepairStart = 16,
    VOT_RepairEnd = 17,
    VOT_GivePowerup = 18
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}
}

namespace TeamManager
{

int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);
}

namespace AIUtil
{

bool DirectLineToTarget(const GameObject* pObj, const PblVector3& vPos);
void Slide(GameObject* pMe, const GameObject* pTarget, bool bUseAttackHelper,
           bool* pbIsPathing, float* pfPathTime, float fMinDist, float fOffset);
}

namespace CombatUtil
{

float GetWeaponMaxRange(const Controllable* pMe);

bool IsTargetLinedUpObj(const Controllable* pMe, const GameObject* pTarget);
}

namespace ReserveManager
{

int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);

void Reserve(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);
}



namespace
{

    inline const CollisionObject* AsCollisionObject(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj);
    }

    inline HealthType GetHealthType(const GameObject* pObj)
    {
        return (HealthType)pObj->mHealthType;
    }

    inline float GetCurHealth(const GameObject* pObj)
    {
        return pObj->mCurHealth;
    }

    inline float GetMaxHealth(const GameObject* pObj)
    {
        return pObj->mMaxHealth;
    }

    inline const CollisionObject* GetCollisionObject(const GameObject* pObj)
    {
        return pObj->GetCollisionObject();
    }

    inline const Controllable* GetControllable(const GameObject* pObj)
    {
        return pObj->GetControllable();
    }

    float GetSoldierInvisibilityFactor(const GameObject* pObj)
    {
        return static_cast<const EntitySoldier*>(pObj)->GetInvisibilityFactor();
    }

    int GetSoldierPlayerId(const GameObject* pObj)
    {
        return static_cast<const EntitySoldier*>(pObj)->mPlayerId;
    }

    void AddAidTypes(const float* pAddHealth, const float* pAddAmmo, unsigned int* pHealth, unsigned int* pAmmo)
    {
        for (int i = 0; i < NUM_HEALTH_TYPES; ++i)
        {
            if (pAddHealth[i] > 0.0f)
            {
                *pHealth |= 1 << i;
            }
            if (pAddAmmo[i] > 0.0f)
            {
                *pAmmo |= 1 << i;
            }
        }
    }
}





// FUNCTION: SWBF2 0x0062e2c0
RepairHelper::RepairHelper(UnitAgent* pAgent)
    : mAgent(pAgent)
{
    mCheckForRecipientTime = 0.0f;
}

// FUNCTION: SWBF2 0x0062e2f0
RepairHelper::~RepairHelper()
{
}

// SYNTHETIC: SWBF2 0x0062e290
// RepairHelper::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0062e300
bool RepairHelper::CheckForRecipientAndInit()
{
    int iSoldierType = GetMe()->GetGameObject()->GetGameObjectClass()->GetSoldierType();
    if (iSoldierType != GameObjectClass::SOLDIER_REPAIR && iSoldierType != GameObjectClass::SOLDIER_PILOT)
    {
        return false;
    }

    if (mCheckForRecipientTime > GameLoop::GetMissionTime())
    {
        return false;
    }
    mCheckForRecipientTime = GameLoop::GetMissionTime() + 5.0f;

    GameObject* pMe = GetMe()->GetGameObject();
    int iMyTeam = pMe->GetTeam();
    if (AIUtil::IsEndGame(iMyTeam))
    {
        return false;
    }

    unsigned int health = 0;
    unsigned int ammo = 0;
    CalcMyAidTypes(&health, &ammo);

    PblVector3 vMyPos = pMe->GetPosition();
    GameObject* objects[50];
    int iCount = TeamManager::GetObjectsInRange(vMyPos, 75.0f, objects, 50, pMe, AI::AFFIL_ALLYNEUTRAL);

    GameObject* pBest = 0;
    AidType bestAid;
    float fBestDistSq = FLT_MAX;
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObj = objects[i];
        if (pObj->GetGameObjectClass()->GetAINoRepair())
        {
            continue;
        }

        PblVector3 vHisPos = pObj->GetPosition();
        if (!pObj->IsAlive() && pObj->mHealthType != HEALTH_TYPE_BUILDING_DEAD &&
            pObj->mHealthType != HEALTH_TYPE_BUILDING_UNBUILT)
        {
            continue;
        }
        if (ReserveManager::GetOpenReservableSlots(pObj, iMyTeam, RT_REPAIR) < 1)
        {
            continue;
        }
        int iOwningTeam = pObj->mOwningTeam;
        if (iOwningTeam && Team::GetPtr(iOwningTeam)->GetAffiliation(iMyTeam) <= 0)
        {
            continue;
        }
        if (pObj->GetEntityType() == ENT_SOLDIER && static_cast<const EntitySoldier*>(pObj)->GetInvisibilityFactor() > 0.0f)
        {
            continue;
        }

        AidType aid;
        if (!FindAidNeed(pObj, health, ammo, &aid, false))
        {
            continue;
        }

        PblVector3 vDiff(vMyPos.x - vHisPos.x, vMyPos.y - vHisPos.y, vMyPos.z - vHisPos.z);
        float fDistSq = vDiff.y * vDiff.y + vDiff.x * vDiff.x + vDiff.z * vDiff.z;
        PblVector3 vVel = pObj->GetVelocity();
        if (pObj->GetControllable() && pObj->GetControllable()->GetPlayerId() >= 0 && fDistSq < 900.0f)
        {
            const PblMatrix& matrix = pObj->GetMatrix();
            const PblVector3& vDir = AIUtil::GetSafeNormal(vDiff);
            if (vVel.y * vVel.y + vVel.x * vVel.x + vVel.z * vVel.z < 0.25f ||
                matrix.m[2][1] * vDir.y + matrix.m[2][0] * vDir.x + matrix.m[2][2] * vDir.z > (float)cos(15.0))
            {
                pBest = pObj;
                bestAid = aid;
                break;
            }
        }

        if (pObj->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
            static_cast<const EntitySoldier*>(pObj)->mPlayerId < 0)
        {
            continue;
        }
        if (vVel.y * vVel.y + vVel.x * vVel.x + vVel.z * vVel.z < 0.25f && fDistSq < fBestDistSq)
        {
            pBest = pObj;
            bestAid = aid;
            fBestDistSq = fDistSq;
        }
    }

    if (!pBest)
    {
        return false;
    }

    bool bChanged = false;
    mAidType = bestAid;
    if (!SetAidWpn(&bChanged))
    {
        return false;
    }

    mAidTarget = pBest;
    ReserveManager::Reserve(pBest, pMe, RT_REPAIR);
    mContactTime = FLT_MAX;
    mSearchTime = GameLoop::GetMissionTime();
    mPathing = false;
    mPathTime = -100.0f;
    GetCtrl()->mLowLevel.SetNavigator_Stationary();
    AI::VOHelper::Play(AI::VOT_RepairStart, GetMe()->GetGameObject(), 0, pBest, 0.0f);
    return true;
}

// FUNCTION: SWBF2 0x0062e7d0
bool RepairHelper::SetAidWpn(bool* pbChanged)
{
    for (int i = 0; i < GetMe()->GetWeaponCount(); ++i)
    {
        const WeaponClass* pClass = GetMe()->GetWeaponPtr(i)->mClass;
        int bGivesAid;
        if (pClass->IsRtti(WeaponRepairClass::rttiHashWeaponRepairClass))
        {
            const WeaponRepairClass* pRepair = static_cast<const WeaponRepairClass*>(pClass);
            bGivesAid = mAidType.ammo ? pRepair->mAddAmmo[mAidType.type] > 0.0f : pRepair->mAddHealth[mAidType.type] > 0.0f;
        }
        else if (pClass->IsRtti(WeaponDispenserClass::rttiHashWeaponDispenserClass))
        {
            EntityClass* pDispense = static_cast<const WeaponDispenserClass*>(pClass)->mDispenseClass;
            if (!pDispense || !pDispense->IsRtti(PowerupItemClass::rttiHashPowerupItemClass))
            {
                continue;
            }
            PowerupItemClass* pPowerup = static_cast<PowerupItemClass*>(pDispense);
            bGivesAid = mAidType.ammo ? pPowerup->mAddAmmo[mAidType.type] > 0.0f : pPowerup->mAddHealth[mAidType.type] > 0.0f;
        }
        else
        {
            continue;
        }

        if (bGivesAid)
        {
            if (GetMe()->GetWeaponIndex(GetMe()->GetWpnChannel()) != i)
            {
                UnitController* pUnit = UnitControllerPtr(GetMe()->mCtrl);
                pUnit->mLowLevel.SetNextWeapon(i);
                *pbChanged = true;
            }
            else
            {
                *pbChanged = false;
            }
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0062e910
void RepairHelper::CalcMyAidTypes(unsigned int* pHealth, unsigned int* pAmmo)
{
    // GLOBAL: SWBF2 0x007ec54c
    static int iii = 1;

    *pHealth = 0;
    *pAmmo = 0;
    for (int i = 0; i < GetMe()->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = GetMe()->GetWeaponPtr(i);
        if (pWeapon->m_pAmmoCounter->GetNumRounds() > 0)
        {
            if (pWeapon->IsRtti(WeaponRepair::rttiHashWeaponRepair))
            {
                const WeaponRepairClass* pClass = static_cast<const WeaponRepairClass*>(pWeapon->mClass);
                if (pClass->mAddHealth[0] > 0.0f)
                {
                    *pHealth |= 1;
                }
                if (pClass->mAddAmmo[0] > 0.0f)
                {
                    *pAmmo |= 1;
                }
                if (pClass->mAddHealth[1] > 0.0f)
                {
                    *pHealth |= 2;
                }
                if (pClass->mAddAmmo[1] > 0.0f)
                {
                    *pAmmo |= 2;
                }
                if (pClass->mAddHealth[2] > 0.0f)
                {
                    *pHealth |= 4;
                }
                if (pClass->mAddAmmo[2] > 0.0f)
                {
                    *pAmmo |= 4;
                }
                if (pClass->mAddHealth[3] > 0.0f)
                {
                    *pHealth |= 8;
                }
                if (pClass->mAddAmmo[3] > 0.0f)
                {
                    *pAmmo |= 8;
                }
                if (pClass->mAddHealth[4] > 0.0f)
                {
                    *pHealth |= 16;
                }
                if (pClass->mAddAmmo[4] > 0.0f)
                {
                    *pAmmo |= 16;
                }
                if (pClass->mAddHealth[5] > 0.0f)
                {
                    *pHealth |= 32;
                }
                if (pClass->mAddAmmo[5] > 0.0f)
                {
                    *pAmmo |= 32;
                }
                if (pClass->mAddHealth[6] > 0.0f)
                {
                    *pHealth |= 64;
                }
                if (pClass->mAddAmmo[6] > 0.0f)
                {
                    *pAmmo |= 64;
                }
                if (pClass->mAddHealth[7] > 0.0f)
                {
                    *pHealth |= 128;
                }
                if (pClass->mAddAmmo[7] > 0.0f)
                {
                    *pAmmo |= 128;
                }
            }
            else if (pWeapon->IsRtti(WeaponDispenser::rttiHashWeaponDispenser))
            {
                EntityClass* pDispense = static_cast<const WeaponDispenserClass*>(pWeapon->mClass)->mDispenseClass;
                if (pDispense && pDispense->IsRtti(PowerupItemClass::rttiHashPowerupItemClass))
                {
                    PowerupItemClass* pPowerup = static_cast<PowerupItemClass*>(pDispense);
                    if (pPowerup->mAddHealth[0] > 0.0f)
                    {
                        *pHealth |= 1;
                    }
                    if (pPowerup->mAddAmmo[0] > 0.0f)
                    {
                        *pAmmo |= 1;
                    }
                    if (pPowerup->mAddHealth[1] > 0.0f)
                    {
                        *pHealth |= 2;
                    }
                    if (pPowerup->mAddAmmo[1] > 0.0f)
                    {
                        *pAmmo |= 2;
                    }
                    if (pPowerup->mAddHealth[2] > 0.0f)
                    {
                        *pHealth |= 4;
                    }
                    if (pPowerup->mAddAmmo[2] > 0.0f)
                    {
                        *pAmmo |= 4;
                    }
                    if (pPowerup->mAddHealth[3] > 0.0f)
                    {
                        *pHealth |= 8;
                    }
                    if (pPowerup->mAddAmmo[3] > 0.0f)
                    {
                        *pAmmo |= 8;
                    }
                    if (pPowerup->mAddHealth[4] > 0.0f)
                    {
                        *pHealth |= 16;
                    }
                    if (pPowerup->mAddAmmo[4] > 0.0f)
                    {
                        *pAmmo |= 16;
                    }
                    if (pPowerup->mAddHealth[5] > 0.0f)
                    {
                        *pHealth |= 32;
                    }
                    if (pPowerup->mAddAmmo[5] > 0.0f)
                    {
                        *pAmmo |= 32;
                    }
                    if (pPowerup->mAddHealth[6] > 0.0f)
                    {
                        *pHealth |= 64;
                    }
                    if (pPowerup->mAddAmmo[6] > 0.0f)
                    {
                        *pAmmo |= 64;
                    }
                    if (pPowerup->mAddHealth[7] > 0.0f)
                    {
                        *pHealth |= 128;
                    }
                    if (pPowerup->mAddAmmo[7] > 0.0f)
                    {
                        *pAmmo |= 128;
                    }
                }
            }
        }
        else
        {
            ++iii;
        }
    }
}

// FUNCTION: SWBF2 0x0062ec20
void RepairHelper::FindPowerupsNearby(const GameObject* pObj, unsigned int* pHealth, unsigned int* pAmmo)
{
    GameObject* objects[10];
    PblVector3 vPos = *reinterpret_cast<const PblVector3*>(&pObj->GetMatrix().m[3][0]);
    int iCount = CollisionManager::GetObjectsInRange(vPos, 15.0f, objects, 10, 0, 1);
    for (int i = 0; i < iCount; ++i)
    {
        Entity* pEntity = reinterpret_cast<Entity*>(objects[i]);
        if (!pEntity->IsRtti(PowerupItem::rttiHashPowerupItem))
        {
            continue;
        }
        const PowerupItemClass* pClass = static_cast<const PowerupItemClass*>(pEntity->GetEntityClass());
        for (int iType = 0; iType < NUM_HEALTH_TYPES; ++iType)
        {
            if (pClass->mAddHealth[iType] > 0.0f)
            {
                *pHealth |= 1 << iType;
            }
            if (pClass->mAddAmmo[iType] > 0.0f)
            {
                *pAmmo |= 1 << iType;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0062edd0
void RepairHelper::FindAllAidNeeds(const GameObject* pObj, unsigned int health, unsigned int ammo,
                                   bool bUnused, bool* pbNeedHealth, bool* pbNeedAmmo)
{
    HealthType type = GetHealthType(pObj);
    unsigned int nearHealth = 0;
    unsigned int nearAmmo = 0;
    FindPowerupsNearby(pObj, &nearHealth, &nearAmmo);

    *pbNeedHealth = ((health >> type) & (~nearHealth >> type) & (pObj->mMaxHealth > GetCurHealth(pObj))) & 1;
    bool bCanAmmo = ((ammo >> type) & 1) != 0;
    bool bNoAmmoNear = ((~nearAmmo >> type) & 1) != 0;
    *pbNeedAmmo = false;

    if (GetControllable(pObj))
    {
        bool bLow = false;
        for (int i = 0; i < GetControllable(pObj)->GetWeaponCount(); ++i)
        {
            const Weapon* pWeapon = GetControllable(pObj)->GetWeaponPtr(i);
            if (pWeapon && pWeapon->m_pAmmoCounter->GetMaxClips() - 1.0f > pWeapon->m_pAmmoCounter->GetNumClips())
            {
                bLow = true;
            }
        }
        *pbNeedAmmo = bNoAmmoNear & bCanAmmo & bLow;
    }
}

// FUNCTION: SWBF2 0x0062eed0
bool RepairHelper::FindAidNeed(const GameObject* pObj, unsigned int health, unsigned int ammo,
                               AidType* pAid, bool b)
{
    bool bNeedHealth;
    bool bNeedAmmo;
    FindAllAidNeeds(pObj, health, ammo, b, &bNeedHealth, &bNeedAmmo);
    if (!bNeedHealth && !bNeedAmmo)
    {
        return false;
    }

    HealthType type = GetHealthType(pObj);
    *pAid = AidType(type, (bNeedHealth && bNeedAmmo) ? AIUtil::GetRandomInt(0, 1) == 1 : bNeedAmmo);
    return true;
}

// FUNCTION: SWBF2 0x0062ef50
bool RepairHelper::FireRepairWeapon(float dt, const GameObject* pTarget)
{
    const bool bLinedUp = CombatUtil::IsTargetLinedUpObj(GetMe(), pTarget);
    if (bLinedUp)
    {
        Weapon* pWeapon = GetMe()->GetCurWpn();
        if (pWeapon->IsRtti(WeaponRepair::rttiHashWeaponRepair))
        {
            if (mContactTime > GameLoop::GetMissionTime())
            {
                mContactTime = GameLoop::GetMissionTime();
                mHealth = GetCurHealth(pTarget);
            }
            UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().SetFireRequest(true, false);
        }
        else if (pWeapon->IsRtti(WeaponDispenser::rttiHashWeaponDispenser))
        {
            if (mContactTime > GameLoop::GetMissionTime())
            {
                mContactTime = GameLoop::GetMissionTime();
                mHealth = GetCurHealth(pTarget);
                UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().SetFireRequest(true, false);
                AI::VOHelper::Play(AI::VOT_GivePowerup, GetMe()->GetGameObject(), 0, pTarget, 0.0f);
            }
        }
        else
        {
            return false;
        }
    }
    return bLinedUp;
}

// FUNCTION: SWBF2 0x0062f080
bool RepairHelper::Update(float dt)
{
    // GLOBAL: SWBF2 0x007ec550
    static int iii = 1;

    GameObject* pTarget = AIUtil::GetAlivePtr(mAidTarget, true);
    if (!pTarget)
    {
        return false;
    }
    ReserveManager::Reserve(pTarget, GetMe()->GetGameObject(), RT_REPAIR);

    bool bChanged = false;
    SetAidWpn(&bChanged);
    if (bChanged)
    {
        return true;
    }

    Controllable* pMe = GetMe();
    UnitControllerPtr(pMe->mCtrl)->GetLowLevel().SetTarget(pTarget);
    const CollisionObject* pTargetCollision = static_cast<const CollisionObject*>(pTarget);
    const PblVector3 vHisPos = *reinterpret_cast<const PblVector3*>(&pTargetCollision->GetMatrix().m[3][0]);
    const PblVector3 vMyPos = pMe->GetMatrix().GetTranslation();
    const float fRange = CombatUtil::GetWeaponMaxRange(pMe);
    const float fDistSq = (vMyPos.x - vHisPos.x) * (vMyPos.x - vHisPos.x) + (vMyPos.z - vHisPos.z) * (vMyPos.z - vHisPos.z);
    const bool bTouching = pMe->GetCurrentCollision() == pTarget;
    const bool bTooClose = fDistSq < 1.0f;
    const bool bInRange = fDistSq < fRange * fRange;

    if ((bTouching || fDistSq < 25.0f) && bInRange && !bTooClose)
    {
        if (!bTouching ||
            AIUtil::DirectLineToTarget(GetMe()->GetGameObject(), pTargetCollision->GetMatrix()[3]))
        {
            GetCtrl()->GetLowLevel().SetNavigator_Stationary();
            mSearchTime = GameLoop::GetMissionTime();
            const bool bFired = FireRepairWeapon(dt, pTarget);

            unsigned int health = 0;
            unsigned int ammo = 0;
            CalcMyAidTypes(&health, &ammo);
            bool bNeedHealth;
            bool bNeedAmmo;
            FindAllAidNeeds(pTarget, health, ammo, false, &bNeedHealth, &bNeedAmmo);
            if ((mAidType.ammo && !bNeedAmmo) || (!mAidType.ammo && !bNeedHealth))
            {
                AI::VOHelper::Play(AI::VOT_RepairEnd, GetMe()->GetGameObject(), 0, pTarget, 0.0f);
                return false;
            }

            if (GameLoop::GetMissionTime() > mContactTime + 8.0f)
            {
                if (!(GetCurHealth(pTarget) > mHealth))
                {
                    return false;
                }
                mHealth = GetCurHealth(pTarget);
                mContactTime = GameLoop::GetMissionTime();
            }
            if (!bFired)
            {
                mContactTime = FLT_MAX;
            }
        }
        else
        {
            mContactTime = FLT_MAX;
        }
    }
    else
    {
        if (GameLoop::GetMissionTime() - mSearchTime > 20.0f)
        {
            if (!UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().HasPath())
            {
                return false;
            }
            if (UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().IsStuck())
            {
                return false;
            }
            ++iii;
        }
        AIUtil::Slide(GetMe()->GetGameObject(), pTarget, false, &mPathing, &mPathTime, 8.0f, 1.6f);
        mContactTime = FLT_MAX;
    }

    const PblMatrix& rMyMatrix = GetMe()->GetMatrix();
    const PblMatrix& rHisMatrix = pTargetCollision->GetMatrix();
    const float fX = rHisMatrix.m[3][0] - rMyMatrix.m[3][0];
    const float fY = rHisMatrix.m[3][1] - rMyMatrix.m[3][1];
    const float fZ = rHisMatrix.m[3][2] - rMyMatrix.m[3][2];
    const float fReach = GetCollisionObject(pTarget)->GetCollisionRadius() + 8.0f;
    if (fReach * fReach > fX * fX + fY * fY + fZ * fZ)
    {
        GetCtrl()->GetLowLevel().SetHeading(pTarget);
        GetCtrl()->GetLowLevel().SetHeight(HEIGHT_STAND);
        return true;
    }
    mAgent->FaceTarget(HEIGHT_STAND);
    return true;
}
