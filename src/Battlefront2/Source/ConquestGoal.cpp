#include "ConquestGoal.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "ConnectivityGraph.h"
#include "Controllable.h"
#include "AIUtil.h"
#include "GameObject.h"
#include "Team.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

namespace
{
inline int GameObjectTeam(const GameObject* pObject)
{
    return pObject->mTeam;
}
}

// SYNTHETIC: SWBF2 0x00402760
// `dynamic initializer for 'AI::ConquestGoal::rttiHashConquestGoal''

// GLOBAL: SWBF2 0x01eb97a0
// AI::ConquestGoal::rttiHashConquestGoal
PblHash AI::ConquestGoal::rttiHashConquestGoal("ConquestGoal");
// GLOBAL: SWBF2 0x00788d4c
// AI::ConquestGoal::rttiNameConquestGoal
const char AI::ConquestGoal::rttiNameConquestGoal[] = "ConquestGoal";







// SYNTHETIC: SWBF2 0x00482a10
// AI::ConquestGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00482a40
AI::ConquestGoal::ConquestGoal(unsigned char uTeam, float fWeight)
    : AIGoal(AIGOAL_CONQUEST, uTeam, fWeight)
{
}

// FUNCTION: SWBF2 0x00482a60
void AI::ConquestGoal::Update()
{
    AIGoal::Update();

    for (PblList<Character>::Itor it(mUnitList); it; ++it)
    {
        if (!it->GetControllable())
        {
            continue;
        }

        UnitController* pCtrl = UnitControllerPtr(it->GetControllable()->mCtrl);
        if (!pCtrl || pCtrl->GetCurCmdType() == CMD_SC_FOLLOW)
        {
            continue;
        }

        const CommandPost* pPost = it->GetAssignedCP();
        if (pPost && pPost->GetGameObject() && GameObjectTeam(pPost->GetGameObject()) > 0 &&
            Team::sTeams[GameObjectTeam(pPost->GetGameObject())] &&
            Team::sTeams[GameObjectTeam(pPost->GetGameObject())]->GetAffiliation(it->mTeamNumber) > 0 &&
            !pPost->GetStrategicDefendValue(it->mTeamPtr->GetSide()))
        {
            sAssign(it, 0);
            continue;
        }

        if (pCtrl->GetCurCmdType() == CMD_NONE || pCtrl->GetCurCmdType() == CMD_WAIT)
        {
            Assign(it);
        }
    }
}

// FUNCTION: SWBF2 0x00482c10
void AI::ConquestGoal::Assign(Character* pChr)
{
    sAssign(pChr, 0);
}

// FUNCTION: SWBF2 0x00482c20
void AI::ConquestGoal::sAssign(Character* pChr, const CommandPost* pPost)
{
    if (pChr->mPlayerId >= 0)
    {
        return;
    }

    if (sIsPilotingVehicle(pChr))
    {
        const CommandPost* pNewPost = sGenerateCP(pChr, pPost, false);
        sIssueCmd(pChr, pNewPost);

        Controllable* pSelf = pChr->GetControllable();
        for (PblList<Controllable>::Itor it(pChr->mVehicle->GetGameObject()->GetControllableList()); it; ++it)
        {
            Controllable* pPassenger = it;
            if (pPassenger->mPlayerId < 0 && pPassenger != pSelf && pPassenger->mCharacter)
            {
                sIssueCmd(pPassenger->mCharacter, pNewPost);
            }
        }
    }
    else
    {
        const CommandPost* pNewPost = 0;
        if (sIsPassenger(pChr))
        {
            const Controllable* pVehicle = pChr->mVehicle;
            const Controllable* pPilot = pVehicle->GetGameObject()->GetControllable();
            if (pPilot && pPilot->mCharacter && pPilot->mPlayerId < 0)
            {
                pNewPost = pPilot->mCharacter->GetAssignedCP();
            }
        }
        if (!pNewPost)
        {
            pNewPost = sGenerateCP(pChr, pPost, false);
        }
        sIssueCmd(pChr, pNewPost);
    }
}

// FUNCTION: SWBF2 0x00482d10
void AI::ConquestGoal::sIssueCmd(Character* pChr, const CommandPost* pPost)
{
    if (pChr->GetControllable())
    {
        Controller* pCtrl = pChr->GetControllable()->mCtrl;
        if (pCtrl)
        {
            UnitCaptureCPInfo info(pPost);
            UnitControllerPtr(pCtrl)->ForceCommand(info);
        }
    }
    pChr->SetAssignedCP(pPost);
}

// FUNCTION: SWBF2 0x00482d90
const CommandPost* AI::ConquestGoal::sGenerateCP(Character* pChr, const CommandPost* pPost, bool bSecondPass)
{
    CommandPostItor it = CommandPostManager::Instance()->GetItor();
    float fTotal = 0.0f;
    bool bValid[16] = {};
    const int iTeam = pChr->mTeamNumber;

    for (int i = 0; it; ++it, ++i)
    {
        CommandPost* pCP = *it;
        if (!bSecondPass && pCP->GetGameObject() &&
            pChr->mTeamPtr->GetAffiliation(pCP->GetGameObject()->GetTeam()) > 0)
        {
            continue;
        }

        bValid[i] = true;
        if (sIsPilotingVehicle(pChr) &&
            (pChr->mVehicle->GetGameObject()->GetGameObjectClass()->mAIType & pCP->GetStrategicBan()))
        {
            bValid[i] = false;
        }
        if (pCP == pPost)
        {
            bValid[i] = false;
        }

        float fImportance = sGetImportance(pCP, iTeam);
        if (fImportance <= 0.0f)
        {
            bValid[i] = false;
        }

        UnitController* pCtrl = pChr->GetControllable() ? UnitControllerPtr(pChr->GetControllable()->mCtrl) : 0;
        if (!bValid[i])
        {
            continue;
        }

        if (pCtrl && ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1) &&
            !ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1)
                 ->CanGetThereFromHere_QuickButNotSoPerfectlyAccurateTest(pCtrl, AIUtil::GetCommandPostPos(pCP)))
        {
            bValid[i] = false;
            continue;
        }
        fTotal += fImportance;
    }

    if (fTotal > 0.0f)
    {
        float fPick = AIUtil::GetRandomFloat(0.0f, fTotal);
        float fSum = 0.0f;
        int i = 0;
        for (it = CommandPostManager::Instance()->GetItor(); it; ++it, ++i)
        {
            if (!bValid[i])
            {
                continue;
            }
            CommandPost* pCP = *it;
            fSum += sGetImportance(pCP, iTeam);
            if (fSum >= fPick)
            {
                return pCP;
            }
        }
    }

    if (!bSecondPass)
    {
        return sGenerateCP(pChr, pPost, true);
    }

    CommandPost* pPosts[16];
    int iCount = 0;
    for (it = CommandPostManager::Instance()->GetItor(); it; ++it)
    {
        pPosts[iCount++] = *it;
    }
    if (iCount <= 1)
    {
        return 0;
    }

    CommandPost* pPick;
    do
    {
        pPick = pPosts[AIUtil::GetRandomInt(0, iCount - 1)];
    } while (pPick == pPost);
    return pPick;
}

// FUNCTION: SWBF2 0x00483070
bool AI::ConquestGoal::sIsPilotingVehicle(Character* pChr)
{
    const Controllable* pVehicle = pChr->mVehicle;
    if (pVehicle)
    {
        return pVehicle->GetGameObject()->GetControllable() == pVehicle;
    }
    return false;
}

// FUNCTION: SWBF2 0x004830a0
bool AI::ConquestGoal::sIsPassenger(Character* pChr)
{
    const Controllable* pVehicle = pChr->mVehicle;
    if (pVehicle)
    {
        return pVehicle->GetGameObject()->GetControllable() != pVehicle;
    }
    return false;
}

// FUNCTION: SWBF2 0x004830d0
float AI::ConquestGoal::sGetImportance(const CommandPost* pPost, int iTeam)
{
    const Team* pTeam = Team::sTeams[iTeam];
    bool bFriendly = pPost->GetGameObject() && pTeam->GetAffiliation(pPost->GetGameObject()->GetTeam()) > 0;
    bool bEnemy = pPost->GetGameObject() && pTeam->GetAffiliation(pPost->GetGameObject()->GetTeam()) < 0;
    if (bFriendly)
    {
        return (float)pPost->GetStrategicDefendValue(pTeam->GetSide());
    }
    if (bEnemy)
    {
        return (float)pPost->GetStrategicAttackValue(pTeam->GetSide());
    }
    return ((float)pPost->GetStrategicAttackValue(pTeam->GetSide()) +
            (float)pPost->GetStrategicDefendValue(pTeam->GetSide())) * 0.5f;
}
