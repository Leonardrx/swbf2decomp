#include "UnitFlyController.h"
#include "UnitFlyAgent.h"

// SYNTHETIC: SWBF2 0x00406cd0
// `dynamic initializer for 'UnitFlyController::rttiHashUnitFlyController''
// UnitFlyController::rttiHashUnitFlyController

// GLOBAL: SWBF2 0x01f993e4
// UnitFlyController::rttiHashUnitFlyController
PblHash UnitFlyController::rttiHashUnitFlyController("UnitFlyController");
// GLOBAL: SWBF2 0x0078b874
// UnitFlyController::rttiNameUnitFlyController
const char UnitFlyController::rttiNameUnitFlyController[] = "UnitFlyController";

// FUNCTION: SWBF2 0x00667d70
UnitFlyController::UnitFlyController(Controllable* pOwner)
    : UnitController(pOwner)
{
}







// FUNCTION: SWBF2 0x00667da0
void UnitFlyController::ApplyNextState()
{
    const UnitCmdInfo* pCmd = GetCurCmd();
    switch (pCmd->GetCommand())
    {
    case CMD_WAIT_SECONDARY:
        SetNextState(STATE_WAIT_SECONDARY);
        break;
    case CMD_COMBAT:
        SetNextState(STATE_COMBAT);
        break;
    case CMD_CAPTURECP:
    case CMD_CTFOFFENSE:
    case CMD_DEATHMATCH:
    case CMD_DEFEND:
    case CMD_DESTROY:
        if (mCurState != STATE_FLY)
        {
            if (mJustEnteredVehicleGoToPickupAgent)
            {
                mJustEnteredVehicleGoToPickupAgent = false;
                PrependQueueCommands(*pCmd, UnitWaitSecondaryInfo(4.0f, false));
                ApplyNextCommand();
                mForceCommand = false;
                ApplyNextState();
            }
            else
            {
                SetNextState(STATE_FLY);
            }
        }
        break;
    case CMD_SC_HOLD:
        SetNextState(STATE_SC_HOLD);
        break;
    }
}

// FUNCTION: SWBF2 0x00667e80
void UnitFlyController::EnterState()
{
    if (mCurState != STATE_FLY)
    {
        UnitController::EnterState();
    }
    else
    {
        mAgent = new UnitFlyAgent(this);
    }
}

// FUNCTION: SWBF2 0x00667ec0
void UnitFlyController::ExitState()
{
    UnitController::ExitState();
}

// FUNCTION: SWBF2 0x00667ed0
void UnitFlyController::UpdateState(float dt)
{
    UnitController::UpdateState(dt);
}
