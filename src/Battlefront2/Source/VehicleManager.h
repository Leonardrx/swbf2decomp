#pragma once

#include "ListPool.h"
#include "PebbleFL/Common/PblHandle.h"

class GameObject;
class PblVector3;

namespace VehicleManager
{
    void Initialize();
    void EnterClient();
    void LeaveClient();
    void Add(GameObject* pObject);
    void Remove(GameObject* pObject);

    void Update();

    bool sIsFull(const GameObject* pObject);
    int sGameObjSortByDistance(const void* pA, const void* pB);
    int RequestVehiclesInRange(const PblVector3& vPos, GameObject** ppOut, float fRadius, int iMax);

    extern ListPool<PblHandle<GameObject>, int>* sVehicles;
}
