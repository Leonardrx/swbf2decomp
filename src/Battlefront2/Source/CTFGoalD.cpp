#include "CTFGoalD.h"
#include "Character.h"
#include "Controllable.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "RedEngineFL/Util/RedRegion.h"



namespace
{


inline int GameObjectTeam(const GameObject* pObject)
{
    return pObject->mTeam;
}


inline const RedRegion* FlagHomeRegion(const GameObject* pFlag)
{
    return static_cast<const FlagItem*>(pFlag)->mHomeRegion;
}

inline const PblVector3& RegionPosition(const RedRegion* pRegion)
{
    return pRegion->mMatrix.GetTranslation();
}
}

// SYNTHETIC: SWBF2 0x004027b0
// `dynamic initializer for 'AI::CTFGoalD::rttiHashCTFGoalD''

// GLOBAL: SWBF2 0x01eb97b0
// AI::CTFGoalD::rttiHashCTFGoalD
PblHash AI::CTFGoalD::rttiHashCTFGoalD("CTFGoalD");
// GLOBAL: SWBF2 0x00788dfc
// AI::CTFGoalD::rttiNameCTFGoalD
const char AI::CTFGoalD::rttiNameCTFGoalD[] = "CTFGoalD";







// SYNTHETIC: SWBF2 0x00488090
// AI::CTFGoalD::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00487fb0
AI::GoalTarget::GoalTarget(const PblVector3& vPos)
    : mType(GTT_POS), mPos(vPos), mChr(0)
{
}

// FUNCTION: SWBF2 0x00487ff0
AI::GoalTarget::GoalTarget(const GameObject* pObject)
    : mType(GTT_GAMEOBJECT), mPos(0, 0, 0), mChr(0), mGameObj(pObject)
{
}

// FUNCTION: SWBF2 0x004880c0
AI::CTFGoalD::CTFGoalD(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_CTFDEFENSE, uTeam, fWeight)
{
    mFlag = static_cast<const GameObject*>(target.mGameObj);
    if (!static_cast<const GameObject*>(mFlag))
    {
        for (PblList<FlagItem>::Itor it(*FlagItem::sList); it; ++it)
        {
            const GameObject* pFlag = reinterpret_cast<const GameObject*>(static_cast<FlagItem*>(it));
            if (GameObjectTeam(pFlag) == uTeam)
            {
                mFlag = pFlag;
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00488190
void AI::CTFGoalD::Assign(Character* pChr)
{
    if (!mFlag || !pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (!pCtrl)
    {
        return;
    }

    GoalTarget target(RegionPosition(FlagHomeRegion(mFlag)));
    if (pCtrl->GetRandomSeed() % 2 == 1)
    {
        target = GoalTarget(mFlag);
    }
    UnitGoalAgentInfo command(CMD_DEFEND, &target);
    pCtrl->ForceCommand(command);
}
