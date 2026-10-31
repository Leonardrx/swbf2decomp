#include "UnitStationaryController.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityBuilding.h"

namespace
{

    inline Entity* AsEntity(GameObject* pObj)
    {
        return reinterpret_cast<Entity*>(pObj);
    }
}

// SYNTHETIC: SWBF2 0x00406d20
// `dynamic initializer for 'UnitStationaryController::rttiHashUnitStationaryController''
// UnitStationaryController::rttiHashUnitStationaryController

// GLOBAL: SWBF2 0x01f9a24c
// UnitStationaryController::rttiHashUnitStationaryController
PblHash UnitStationaryController::rttiHashUnitStationaryController("UnitStationaryController");
// GLOBAL: SWBF2 0x0078b930
// UnitStationaryController::rttiNameUnitStationaryController
const char UnitStationaryController::rttiNameUnitStationaryController[] = "UnitStationaryController";







// FUNCTION: SWBF2 0x006697a0
UnitStationaryController::UnitStationaryController(Controllable* pOwner)
    : UnitController(pOwner)
{
    if (AsEntity(pOwner->GetGameObject())->GetEntity()->IsRtti(EntityBuilding::rttiHashEntityBuilding))
    {
        mLookAheadOnly = true;
    }
}

// SYNTHETIC: SWBF2 0x00669770
// UnitStationaryController::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00669830
void UnitStationaryController::ApplyNextState()
{
    switch (GetCurCmd()->GetCommand())
    {
    case CMD_COMBAT:
        SetNextState(STATE_COMBAT);
        break;
    case CMD_UNBOARD:
        SetNextState(STATE_UNBOARD);
        break;
    case CMD_WAIT:
    case CMD_CAPTURECP:
    case CMD_CTFOFFENSE:
    case CMD_DEATHMATCH:
    case CMD_DEFEND:
    case CMD_DESTROY:
        SetNextState(STATE_WAIT);
        break;
    }
}
