#include "UnitDeathmatchAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "CombatUtil.h"
#include "ConnectivityGraph.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "Team.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include <d3dx9math.h>
#include "PebbleFL/Common/PblHashTable.h"

// SYNTHETIC: SWBF2 0x00406c90
// `dynamic initializer for 'UnitDeathmatchAgent::rttiHashUnitDeathmatchAgent''
// UnitDeathmatchAgent::rttiHashUnitDeathmatchAgent

// GLOBAL: SWBF2 0x01f9938c
// UnitDeathmatchAgent::rttiHashUnitDeathmatchAgent
PblHash UnitDeathmatchAgent::rttiHashUnitDeathmatchAgent("UnitDeathmatchAgent");
// GLOBAL: SWBF2 0x0078b7ec
// UnitDeathmatchAgent::rttiNameUnitDeathmatchAgent
const char UnitDeathmatchAgent::rttiNameUnitDeathmatchAgent[] = "UnitDeathmatchAgent";







// FUNCTION: SWBF2 0x00665b50
const char* UnitDeathmatchAgent::NFINAL_GetStateString(int) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00665a50
int UnitDeathmatchAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_Grenade:
    case AI::EVT_Damaged:
    case AI::EVT_Vision_CanSeeEnemy:
    case AI::EVT_Audible_CanHearEnemy:
    case AI::EVT_EndInvisibility:
        if (mCurState != 4 && UnitAgent::EventHandler(pInfo) == 1 &&
            pInfo->mNextCmd.GetCmdType() == CMD_COMBAT)
        {
            mCombatCmd = pInfo->mNextCmd;
            SetNextState(4);
            pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, NULL, PblVector3(0, 0, 0), NULL);
            return 1;
        }
        return 0;

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// SYNTHETIC: SWBF2 0x006653f0
// UnitDeathmatchAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00665450
UnitDeathmatchAgent::UnitDeathmatchAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl),
      mCombatHelper(this),
      mNextJumpTime(0.0f)
{
    mCurState = STATE_START;
    mNextState = STATE_RunPoint;
}

// FUNCTION: SWBF2 0x006654a0
void UnitDeathmatchAgent::Kill()
{
    mCombatHelper.Kill();
    UnitAgent::Kill();
}

// FUNCTION: SWBF2 0x006654c0
void UnitDeathmatchAgent::EnterState()
{
    switch (mCurState)
    {
    case STATE_START:
        if (ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1))
        {
            SetNextState((PblRandom::GetGlobalInt() & 1) ? STATE_RunPoint : STATE_RunEnemy);
        }
        else
        {
            SetNextState(STATE_Stationary);
        }
        break;

    case STATE_RunPoint:
    {
        const PblVector3 vGoal = AIUtil::GetRandomAccessiblePoint(mCtrl);
        mCtrl->GetLowLevel().SetNavigator_Goto(vGoal, 0, false, false);
        break;
    }

    case STATE_RunEnemy:
    {
        PblVector3 vDir;
        Character* pChar = mCtrl->mOwner->mCharacter;
        const Team* pTeam = Team::sTeams[pChar ? pChar->GetTeamNumber() : 3];
        int iEnemyTeam = 0;
        if (pTeam->mAffiliation[1] < 0)
        {
            iEnemyTeam = 1;
        }
        if (pTeam->mAffiliation[2] < 0)
        {
            iEnemyTeam = iEnemyTeam ? (int)(PblRandom::GetGlobalInt() & 1) + 1 : 2;
        }

        Character* pEnemy = Team::sTeams[iEnemyTeam]->GetRandomLiveCharacter();
        if (!pEnemy || !pEnemy->GetControllable())
        {
            break;
        }
        auto pOwnerCtrl = mCtrl->mOwner->mCtrl;
        if (!CombatUtil::EngageInCombat(UnitControllerPtr(pOwnerCtrl),
                                        pEnemy->GetControllable()->GetGameObject(), true))
        {
            SetNextState(STATE_RunPoint);
            break;
        }

        PblVector3 vDest = pEnemy->GetControllable()->GetGameObject()->GetPosition();
        if (GetMe()->GetGameObject()->GetGameObjectClass()->mAIType == ConnectivityGraph::CGT_FLAG_HUGE)
        {
            Controllable* pMe = GetMe();
            const PblVector3& vMyPos = pMe->GetMatrix().GetTranslation();
            const PblVector3 vDelta = vDest - vMyPos;
            D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vDelta);
            float fRadius = pMe->GetGameObject()->GetCollisionRadius();
            vDest -= vDir * fRadius;
        }
        else
        {
            PblVector3 vPush = PblRandom::GetGlobalVector3() * 10.0f + vDest;
            PathFinder::PushValidPoint(vDest, vPush, GetMe()->GetGameObject());
            vDest = vPush;
        }
        mCtrl->GetLowLevel().SetNavigator_Goto(vDest, 0, false, false);
        break;
    }

    case STATE_Combat:
        if (mCombatCmd.GetCmdType() == CMD_NONE)
        {
            mCombatCmd = UnitCombatInfo(UnitCombatInfo::ENGAGE);
        }
        mCombatHelper.Init(static_cast<const UnitCombatInfo&>(mCombatCmd));
        break;

    case STATE_Stationary:
        mCtrl->GetLowLevel().SetNavigator_Stationary();
        break;
    }
}

// FUNCTION: SWBF2 0x006657e0
void UnitDeathmatchAgent::ExitState()
{
    if (mCurState == STATE_Combat)
    {
        mCombatHelper.Clean();
    }
}

// FUNCTION: SWBF2 0x006657f0
void UnitDeathmatchAgent::DoJetJumpStuff()
{
    if (!GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }

    EntitySoldier* pSoldier = static_cast<EntitySoldier*>(GetMe()->GetEntity());
    if (pSoldier->mClass->mJetFuelRechargeRate > 0.0f)
    {
        if (pSoldier->GetJetFuel() > pSoldier->mClass->GetJetFuelMinBorder())
        {
            if (!mCtrl->GetLowLevel().mJetJumping)
            {
                if (GameLoop::GetMissionTime() > mNextJumpTime)
                {
                    mNextJumpTime += 5.0f;
                    if (PblRandom::GetGlobalInt() & 1)
                    {
                        PblVector3 vStart = GetMe()->GetMatrix().GetTranslation();
                        PblVector3 vEnd = vStart + PblVector3(0.0f, 40.0f, 0.0f);
                        const GameObject* pIgnore = GetMe()->GetGameObject();
                        if (!CollisionManager::RayTest(vStart, vEnd, &pIgnore, 1, 0x9a))
                        {
                            mCtrl->GetLowLevel().SetJetJump(PblVector3(0.0f, 0.0f, 0.0f));
                        }
                    }
                }
            }
        }
        else
        {
            const float fNow = GameLoop::GetMissionTime();
            mNextJumpTime = fNow + 5.0f + PblRandom::GetGlobalFloat() * 10.0f;
        }
    }
}

// FUNCTION: SWBF2 0x00665990
void UnitDeathmatchAgent::UpdateStatePostVision(float dt)
{
    if (mCurState != STATE_Combat)
    {
        mAttackHelper.Update();
    }

    switch (mCurState)
    {
    case STATE_RunPoint:
    case STATE_RunEnemy:
        mCtrl->GetLowLevel().SetHeadingDefault();
        mCtrl->GetLowLevel().SetHeight(HEIGHT_STAND);
        DoJetJumpStuff();
        if (!mCtrl->GetLowLevel().IsNavigatorInProgress())
        {
            SetNextState(STATE_START);
        }
        break;

    case STATE_Combat:
        mCombatHelper.FSMUpdate(dt, false);
        if (mCombatHelper.IsDone())
        {
            SetNextState(STATE_START);
        }
        break;
    }
}
