#include "FollowGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

namespace
{



inline bool IsAlive(const GameObject* pObject)
{
    return static_cast<const Damageable*>(pObject)->mIsAlive;
}
}

// SYNTHETIC: SWBF2 0x00403b50
// `dynamic initializer for 'AI::FollowGoal::rttiHashFollowGoal''

// GLOBAL: SWBF2 0x01ec0bd8
// AI::FollowGoal::rttiHashFollowGoal
PblHash AI::FollowGoal::rttiHashFollowGoal("FollowGoal");
// GLOBAL: SWBF2 0x007898f8
// AI::FollowGoal::rttiNameFollowGoal
const char AI::FollowGoal::rttiNameFollowGoal[] = "FollowGoal";







// SYNTHETIC: SWBF2 0x0052d210
// AI::FollowGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052d240
AI::FollowGoal::FollowGoal(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_FOLLOW, uTeam, fWeight)
{
    mTarget = target;
}

// FUNCTION: SWBF2 0x0052d390
void AI::FollowGoal::Update()
{
    if (mEnabled)
    {
        if (mTarget.mType == GTT_GAMEOBJECT && static_cast<const GameObject*>(mTarget.mGameObj) &&
            !static_cast<const GameObject*>(mTarget.mGameObj)->IsAlive())
        {
            SetEnabled(false);
            return;
        }
        AIGoal::Update();
    }
    else if (mTarget.mType == GTT_GAMEOBJECT && static_cast<const GameObject*>(mTarget.mGameObj) &&
             static_cast<const GameObject*>(mTarget.mGameObj)->IsAlive())
    {
        SetEnabled(true);
    }
}

// FUNCTION: SWBF2 0x0052d2a0
void AI::FollowGoal::Assign(Character* pChr)
{
    if (!pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (!pCtrl)
    {
        return;
    }

    const GameObject* pObject = mTarget.GetGameObject();
    if (pObject)
    {
        pCtrl->ForceCommand(UnitFollowInfo(PblHandle<GameObject>(const_cast<GameObject*>(pObject)), PblHandle<GameObject>(), true));
    }
}
