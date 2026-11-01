#include "DeathmatchGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

// SYNTHETIC: SWBF2 0x004027c0
// `dynamic initializer for 'AI::DeathmatchGoal::rttiHashDeathmatchGoal''

// GLOBAL: SWBF2 0x01eb97b4
// AI::DeathmatchGoal::rttiHashDeathmatchGoal
PblHash AI::DeathmatchGoal::rttiHashDeathmatchGoal("DeathmatchGoal");
// GLOBAL: SWBF2 0x00788e54
// AI::DeathmatchGoal::rttiNameDeathmatchGoal
const char AI::DeathmatchGoal::rttiNameDeathmatchGoal[] = "DeathmatchGoal";







// SYNTHETIC: SWBF2 0x0048a9d0
// AI::DeathmatchGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048aa00
AI::DeathmatchGoal::DeathmatchGoal(unsigned char uTeam, float fWeight)
    : AIGoal(AIGOAL_DEATHMATCH, uTeam, fWeight)
{
}

// FUNCTION: SWBF2 0x0048aa20
void AI::DeathmatchGoal::Assign(Character* pChr)
{
    if (!pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (pCtrl)
    {
        CmdInfo cmd = UnitCmdInfo(CMD_DEATHMATCH, NULL, PblVector3(0.0f, 0.0f, 0.0f), NULL);
        pCtrl->ForceCommand(cmd);
    }
}
