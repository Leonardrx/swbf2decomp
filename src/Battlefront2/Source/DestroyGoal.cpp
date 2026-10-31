#include "DestroyGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

// SYNTHETIC: SWBF2 0x00402800
// `dynamic initializer for 'AI::DestroyGoal::rttiHashDestroyGoal''

// GLOBAL: SWBF2 0x01eb980c
// AI::DestroyGoal::rttiHashDestroyGoal
PblHash AI::DestroyGoal::rttiHashDestroyGoal("DestroyGoal");
// GLOBAL: SWBF2 0x00788ea4
// AI::DestroyGoal::rttiNameDestroyGoal
const char AI::DestroyGoal::rttiNameDestroyGoal[] = "DestroyGoal";







// SYNTHETIC: SWBF2 0x0048c700
// AI::DestroyGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048c730
AI::DestroyGoal::DestroyGoal(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_DESTROY, uTeam, fWeight)
{
    mTarget = target;
}

// FUNCTION: SWBF2 0x0048c790
void AI::DestroyGoal::Assign(Character* pChr)
{
    if (!pChr || !pChr->GetControllable())
    {
        return;
    }
    UnitController* pController = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (pController)
    {
        UnitGoalAgentInfo info(CMD_DESTROY, &mTarget);
        pController->ForceCommand(info);
    }
}

// FUNCTION: SWBF2 0x0048c810
void AI::DestroyGoal::Update()
{
    if (mEnabled)
    {
        if (mTarget.mType == GTT_GAMEOBJECT && mTarget.mGameObj && !mTarget.mGameObj->IsAlive())
        {
            SetEnabled(false);
            return;
        }
        AIGoal::Update();
    }
    else if (mTarget.mType == GTT_GAMEOBJECT && mTarget.mGameObj && mTarget.mGameObj->IsAlive())
    {
        SetEnabled(true);
    }
}
