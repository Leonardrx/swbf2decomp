#include "VehicleManager.h"
#include "Controllable.h"
#include "AIUtil.h"
#include "Character.h"
#include "CommandFlyer.h"
#include "EntityBuildingArmed.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "Team.h"
#include "UnitController.h"
#include <stdlib.h>
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

class Character;
class FlagItem;

namespace
{
    inline const Controllable* GetControllable(const GameObject* pObject)
    {
        return pObject->GetControllable();
    }

    const FlagItem* CarriedFlag(const Character* pCharacter)
    {
        return pCharacter->mCarriedFlagPtr;
    }

    inline const PblVector3& GameObjectPosition(const GameObject* pObject)
    {
        return static_cast<const CollisionObject*>(pObject)->GetPosition();
    }
}

// SYNTHETIC: SWBF2 0x00406db0
// `dynamic initializer for 'sHostVehicles''
// SYNTHETIC: SWBF2 0x00769700
// `dynamic atexit destructor for 'sHostVehicles''

// GLOBAL: SWBF2 0x007ec3cc
// sHostVehicles
ListPool<PblHandle<GameObject>, int> sHostVehicles(1200);

// SYNTHETIC: SWBF2 0x00406e60
// `dynamic initializer for 'sClientVehicles''
// SYNTHETIC: SWBF2 0x00769730
// `dynamic atexit destructor for 'sClientVehicles''

// GLOBAL: SWBF2 0x007ec3dc
// sClientVehicles
ListPool<PblHandle<GameObject>, int> sClientVehicles(1200);

// GLOBAL: SWBF2 0x01eafcf4
ListPool<PblHandle<GameObject>, int>* VehicleManager::sVehicles;

// GLOBAL: SWBF2 0x01faa428
float sNotifyRadius;

// GLOBAL: SWBF2 0x01eafb43
bool sGameObjSortFlagPriority;

// GLOBAL: SWBF2 0x01f9a260
PblVector3 sGameObjSortFrom;

// FUNCTION: SWBF2 0x0066dca0
void VehicleManager::Initialize()
{
    sHostVehicles.Clear();
    sClientVehicles.Clear();
    sVehicles = &sHostVehicles;
    sNotifyRadius = 50.0f;
}

// FUNCTION: SWBF2 0x0066dce0
void VehicleManager::EnterClient()
{
    sVehicles = &sClientVehicles;
}

// FUNCTION: SWBF2 0x0066dcf0
void VehicleManager::LeaveClient()
{
    sVehicles = &sHostVehicles;
}

// FUNCTION: SWBF2 0x0066dd00
void VehicleManager::Add(GameObject* pObject)
{
    if (!pObject)
    {
        return;
    }

    PblHandle<GameObject> handle(pObject);
    for (int i = 0; i < sVehicles->GetLength(); ++i)
    {
        if ((*sVehicles)[i] == handle)
        {
            return;
        }
    }
    sVehicles->Append(pObject);
}

// FUNCTION: SWBF2 0x0066ddc0
void VehicleManager::Remove(GameObject* pObject)
{
    if (!pObject)
    {
        return;
    }

    sVehicles->RemoveObject(PblHandle<GameObject>(pObject));
}

// TEMPLATE: SWBF2 0x0066e460
// ListPool<PblHandle<GameObject>,int>::RemoveObject
template void ListPool<PblHandle<GameObject>, int>::RemoveObject(const PblHandle<GameObject>&);

// FUNCTION: SWBF2 0x0066ddf0
bool VehicleManager::sIsFull(const GameObject* pObject)
{
    for (PblList<Controllable>::Itor it(pObject->GetControllableList()); (Controllable*)it; ++it)
    {
        const Controllable* pSeat = it;
        PilotType type = pSeat->mPilotType;
        if (type != PILOT_VEHICLE && type != PILOT_VEHICLESELF)
        {
            continue;
        }
        if (type == PILOT_SELF || (type == PILOT_VEHICLESELF && pSeat->mPilot == pSeat) || !pSeat->mPilot)
        {
            if (type != PILOT_SELF && type != PILOT_VEHICLESELF)
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0066de50
int VehicleManager::sGameObjSortByDistance(const void* pA, const void* pB)
{
    const GameObject* pObjectA = *(const GameObject* const*)pA;
    const GameObject* pObjectB = *(const GameObject* const*)pB;
    if (!pObjectA)
    {
        return 1;
    }
    if (!pObjectB)
    {
        return -1;
    }

    if (sGameObjSortFlagPriority)
    {
        if (pObjectA->GetControllable() && pObjectA->GetControllable()->mCharacter &&
            pObjectA->GetControllable()->mCharacter->mCarriedFlagPtr)
        {
            return 1;
        }
        if (pObjectB->GetControllable() && pObjectB->GetControllable()->mCharacter &&
            pObjectB->GetControllable()->mCharacter->mCarriedFlagPtr)
        {
            return -1;
        }
    }

    const PblVector3 vDeltaA = GameObjectPosition(pObjectA) - sGameObjSortFrom;
    float fDistA = SquaredLength(vDeltaA);
    const PblVector3 vDeltaB = GameObjectPosition(pObjectB) - sGameObjSortFrom;
    float fDistB = SquaredLength(vDeltaB);
    if (fDistB > fDistA)
    {
        return -1;
    }
    return fDistA > fDistB;
}

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
}

namespace TeamManager
{

int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);
}

// GLOBAL: SWBF2 0x007ec558
int sVehicleUpdateBucket = 30;

// FUNCTION: SWBF2 0x0066df90
void VehicleManager::Update()
{
    if (sVehicleUpdateBucket == 30)
    {
        sVehicleUpdateBucket = 0;
    }
    else
    {
        ++sVehicleUpdateBucket;
    }

    for (int i = 0; i < sVehicles->GetLength(); ++i)
    {
        if ((int)(*sVehicles)[i].GetHandleId() % 30 != sVehicleUpdateBucket)
        {
            continue;
        }

        GameObject* pObject = (*sVehicles)[i];
        if (pObject->IsRtti(EntityBuildingArmed::rttiHashEntityBuildingArmed) &&
            static_cast<EntityBuildingArmed*>(pObject)->mNoAIBoard)
        {
            continue;
        }
        if (pObject->IsRtti(CommandFlyer::rttiHashCommandFlyer) &&
            static_cast<CommandFlyer*>(pObject)->mbHasTakenOffBefore)
        {
            continue;
        }
        if (sIsFull(pObject))
        {
            continue;
        }

        const PblVector3& vVelocity = pObject->GetVelocity();
        if (vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z > 9.0f)
        {
            continue;
        }

        if (!pObject->GetEntity()->IsRtti(EntityBuilding::rttiHashEntityBuilding))
        {
            const Obstacle* pBarrier = AIUtil::GetGameObjectBarrier(pObject);
            if (!PathFinder::IsPointValid(pObject->GetMatrix().GetTranslation(), 2.0f, 1, 0,
                                          &pBarrier, 1))
            {
                continue;
            }
        }
        if (pObject->GetGameObjectClass()->GetVehicleType() != GameObjectClass::VEHICLE_UNKNOWN)
        {
            const Obstacle* pBarrier = AIUtil::GetGameObjectBarrier(pObject);
            int eProhibit = pObject->GetGameObjectClass()->mAIType;
            if (!PathFinder::IsPointValid(pObject->GetMatrix().GetTranslation(),
                                          AIUtil::GetCollisionRadius(pObject), eProhibit, 0, &pBarrier, 1))
            {
                continue;
            }
        }

        GameObject* apObjects[30];
        int iCount = TeamManager::GetObjectsInRange(pObject->GetMatrix().GetTranslation(),
                                                    sNotifyRadius, apObjects, 30, pObject,
                                                    AI::AFFIL_ALLYNEUTRAL);
        if (iCount > 1)
        {
            sGameObjSortFlagPriority = pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer);
            sGameObjSortFrom = pObject->GetMatrix().GetTranslation();
            qsort(apObjects, iCount, sizeof(GameObject*), sGameObjSortByDistance);
        }

        for (int j = 0; j < iCount; ++j)
        {
            GameObject* pOther = apObjects[j];
            if (pObject->IsReservedForPlayer() &&
                Team::GetPtr(pOther->GetTeam())->GetAffiliation(AIUtil::IsPlayerTeam(1) ? 1 : 2) >= 0)
            {
                continue;
            }

            const PblList<Controllable>& seats = pOther->GetControllableList();
            for (PblList<Controllable>::Itor it(seats); (Controllable*)it != (Controllable*)seats.End(); ++it)
            {
                Controllable* pSeat = it;
                const ControllableClass* pClass = pSeat->GetControllableClass();
                Controller* pController = pSeat->mCtrl;
                Character* pCharacter = pSeat->mCharacter;
                if (pCharacter && GameLoop::GetMissionTime() < pCharacter->GetVehicleBlockTime())
                {
                    continue;
                }
                if (!pController || !pClass || !pClass->mCanEnterVehicles)
                {
                    continue;
                }

                if (pObject->mOwningTeam &&
                    Team::GetPtr(pObject->mOwningTeam)->GetAffiliation(pOther->GetTeam()) <= 0)
                {
                    if (!pObject->GetControllable()->GetPilot())
                    {
                        continue;
                    }
                    if (pObject->GetControllable()->GetPilot()->GetPlayerId() < 0)
                    {
                        continue;
                    }
                }

                if (!AIUtil::IsEndGame(0) && pController->IsRtti(UnitController::rttiHashUnitController))
                {
                    AI::EventInfo info(AI::EVT_EmptyVehicle, pObject);
                    static_cast<UnitController*>(pController)->SendEvent(&info);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x0066e390
int VehicleManager::RequestVehiclesInRange(const PblVector3& vPos, GameObject** ppOut, float fRadius, int iMax)
{
    const float fRadiusSq = fRadius * fRadius;
    int iCount = 0;
    for (int i = 0; i < VehicleManager::sVehicles->GetLength(); ++i)
    {
        GameObject* pObj = static_cast<GameObject*>((*VehicleManager::sVehicles)[i]);
        const PblMatrix& m = static_cast<const CollisionObject*>(pObj)->GetMatrix();
        const float dx = m.m[3][0] - vPos.x;
        const float dy = m.m[3][1] - vPos.y;
        const float dz = m.m[3][2] - vPos.z;
        if (dx * dx + dy * dy + dz * dz < fRadiusSq)
        {
            ppOut[iCount++] = pObj;
            if (iCount >= iMax)
            {
                break;
            }
        }
    }
    return iCount;
}
