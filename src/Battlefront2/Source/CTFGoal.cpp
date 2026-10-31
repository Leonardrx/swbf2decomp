#include "CTFGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

namespace
{


inline int GameObjectTeam(const GameObject* pObject)
{
    return pObject->mTeam;
}

inline Character* ItemOwner(const GameObject* pItem)
{
    return static_cast<const FlagItem*>(pItem)->mOwner;
}

}

// SYNTHETIC: SWBF2 0x004027a0
// `dynamic initializer for 'AI::CTFGoal::rttiHashCTFGoal''

// GLOBAL: SWBF2 0x01eb97ac
// AI::CTFGoal::rttiHashCTFGoal
PblHash AI::CTFGoal::rttiHashCTFGoal("CTFGoal");
// GLOBAL: SWBF2 0x00788de4
// AI::CTFGoal::rttiNameCTFGoal
const char AI::CTFGoal::rttiNameCTFGoal[] = "CTFGoal";







// SYNTHETIC: SWBF2 0x00487c70
// AI::CTFGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00487ca0
AI::CTFGoal::CTFGoal(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_CTFOFFENSE, uTeam, fWeight)
{
    mCapturePos = target.mPos;
    mFlag = static_cast<const GameObject*>(target.mGameObj);
    if (!static_cast<const GameObject*>(mFlag))
    {
        for (PblList<FlagItem>::Itor it = FlagItem::sList->Begin(); it; ++it)
        {
            const GameObject* pFlag = reinterpret_cast<const GameObject*>(static_cast<FlagItem*>(it));
            if (GameObjectTeam(pFlag) != uTeam)
            {
                mFlag = pFlag;
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00487d80
void AI::CTFGoal::Assign(Character* pChr)
{
    if (!static_cast<const GameObject*>(mFlag) || !pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (pCtrl)
    {
        UnitGoalAgentInfo info(
            CMD_CTFOFFENSE, reinterpret_cast<const FlagItem*>(static_cast<const GameObject*>(mFlag)), mCapturePos);
        pCtrl->ForceCommand(info);
    }
}

// FUNCTION: SWBF2 0x00487e40
void AI::CTFGoal::Update()
{
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pChr = Character::GetPtr(i);
        if (!pChr || pChr->mTeamNumber != mTeam)
        {
            continue;
        }
        const GameObject* pFlag = static_cast<const GameObject*>(mFlag);
        if (!pFlag || ItemOwner(pFlag) != pChr)
        {
            continue;
        }
        if (!pChr->GetControllable())
        {
            continue;
        }
        UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
        if (pCtrl && (pCtrl->GetCurCmdType() != CMD_CTFOFFENSE || pCtrl->IsAgentDone()) &&
            !pCtrl->IsNewCommand())
        {
            AddUnit(pChr);
            Assign(pChr);
        }
    }
    AIGoal::Update();
}
