#include "UnitFlyAgent.h"

#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "Damageable.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "HintManager.h"
#include "UnitController.h"
#include "VehicleSpawn.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "EntityCarrier.h"
#include "PathManager.h"
#include "WeaponCannonInstance.h"
#include "WeaponTowCable.h"
#include "PebbleFL/Common/PblRandom.h"

#include "UnitThreatManager.h"

// SYNTHETIC: SWBF2 0x00406cc0
// `dynamic initializer for 'UnitFlyAgent::rttiHashUnitFlyAgent''
// UnitFlyAgent::rttiHashUnitFlyAgent

// GLOBAL: SWBF2 0x01f993bc
// UnitFlyAgent::rttiHashUnitFlyAgent
PblHash UnitFlyAgent::rttiHashUnitFlyAgent("UnitFlyAgent");
// GLOBAL: SWBF2 0x0078b844
// UnitFlyAgent::rttiNameUnitFlyAgent
const char UnitFlyAgent::rttiNameUnitFlyAgent[] = "UnitFlyAgent";

namespace VehicleManager
{
int RequestVehiclesInRange(const PblVector3& vPos, GameObject** ppOut, float fRadius, int iMax);
}

// FUNCTION: SWBF2 0x00666790
UnitFlyAgent::UnitFlyAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl)
{
    mHint = 0;
    mNextHintCheckTime = -100.0f;
    mCurState = 1;
    mNextState = 1;
}

// SYNTHETIC: SWBF2 0x00666740
// UnitFlyAgent::`scalar deleting destructor'







// FUNCTION: SWBF2 0x006667d0
void UnitFlyAgent::EnterState()
{
    const PblVector3 vPos = mCtrl->mOwner->GetMatrix().GetTranslation();

    switch (mCurState)
    {
    case 1:
        mGotoNonOwnedCP = true;
        if (GetFlyer()->GetState() != EntityFlyer::LANDED)
        {
            SetNextState(4);
        }
        else
        {
            GetLowLevel().SetNavigator_Stationary();
        }
        break;

    case 2:
    {
        PblVector3 vFwd = mCtrl->mOwner->GetMatrix().GetZAxis();
        vFwd.y = 0.0f;
        PblVector3 vDest = vFwd * 200.0f;
        vDest += vPos;
        vDest.y += 1.0f;

        AIPath* pPath = new AIPath();
        if (pPath)
        {
            pPath->Prepend(vDest, 0);
            pPath->Prepend(mCtrl->mOwner->GetMatrix().GetTranslation(), 0);
            GetLowLevel().SetNavigator_FollowPath(pPath, false, true, true);
            pPath->Release();
        }

        EntityFlyer* pFlyer = static_cast<EntityFlyer*>(mCtrl->mOwner->GetEntity());
        if (pFlyer->GetState() != EntityFlyer::FLYING)
        {
            pFlyer->TakeOff();
        }
        break;
    }

    case 3:
        if (IsTransport() && mGotoNonOwnedCP)
        {
            mWaitTime = GameLoop::GetMissionTime() + 10.0f;
            GetLowLevel().SetNavigator_Stationary();
        }
        else
        {
            mWaitTime = 4.0f;
            GetLowLevel().SetNavigator_Stationary();
        }
        break;

    case 4:
    {
        int iTries = 0;
        const int iTeam = mCtrl->mOwner->GetGameObject()->GetTeam();
        bool bGood;
        PblVector3 vDest;
        float fDistSq;
        do
        {
            CommandPostManager* pManager = CommandPostManager::Instance();
            const CommandPost* pPost =
                pManager->FindPostFromIndex(AIUtil::GetRandomInt(0, pManager->GetCount() - 1));
            if (IsTransport())
            {
                if (mGotoNonOwnedCP)
                {
                    bGood = !pPost->CanTeamSpawnHere(iTeam);
                }
                else
                {
                    bGood = pPost->CanTeamSpawnHere(iTeam);
                }
            }
            else
            {
                bGood = true;
            }
            vDest = AIUtil::GetCommandPostPos(pPost);
            fDistSq = SquaredLength(vDest - vPos);
            ++iTries;
        } while (!bGood && fDistSq < 10000.0f && iTries < 10);

        PblVector3 vGoto = vDest;
        vGoto.y += 20.0f;
        GetLowLevel().SetNavigator_Goto(vGoto, 0, false, false);
        if (GetFlyer()->GetState() != EntityFlyer::FLYING)
        {
            GetFlyer()->TakeOff();
        }
        break;
    }

    case 5:
    {
        mHint->Reserve(mCtrl->mOwner->GetGameObject());
        PblMatrix mLand = mHint->BuildMatrix();
        VehicleSpawn::gCalcBeginLandPos(*GetFlyer()->mClass, &mLand);
        const PblVector3 vLand = mLand.GetTranslation();

        const float fDist = GetFlyer()->GetMaxSpeed() * 2.0f;
        PblVector3 vBack = AIUtil::GetSafeNormalXZ(mHint->GetZAxis());
        vBack.y = -0.2f;
        const PblVector3 vApproach = vLand - vBack * fDist;

        AIPath* pPath = new AIPath();
        if (pPath)
        {
            pPath->Prepend(vLand, 0);
            pPath->Prepend(vApproach, 0);
            pPath->Prepend(mCtrl->mOwner->GetMatrix().GetTranslation(), 0);
            GetLowLevel().SetNavigator_FollowPath(pPath, false, true, true);
            pPath->Release();
        }
        GetLowLevel().mLandAtDest = true;
        break;
    }

    case 6:
        if (mGotoNonOwnedCP)
        {
            AIUtil::DeployCargo(mCtrl->mOwner->GetGameObject(), 0);
            if (!IsTransport())
            {
                mHint->SetReserveDelay(mCtrl->mOwner->GetGameObject(), 20.0f);
                mCtrl->mOwner->SetExitVehicle(mHint->GetCommandPost());
                mCtrl->mOwner->GetCharacter()->SetVehicleBlockTime(GameLoop::GetMissionTime() + 15.0f);
                return;
            }
        }
        mGotoNonOwnedCP = !mGotoNonOwnedCP;
        SetNextState(3);
        break;
    }
}

// FUNCTION: SWBF2 0x00666cf0
void UnitFlyAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x00667520
const char* UnitFlyAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00667460
const EntityFlyer* UnitFlyAgent::GetFlyer() const
{
    if (mCtrl->mOwner)
    {
        return static_cast<const EntityFlyer*>(mCtrl->mOwner);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00667480
EntityFlyer* UnitFlyAgent::GetFlyer()
{
    if (mCtrl->mOwner)
    {
        return static_cast<EntityFlyer*>(mCtrl->mOwner);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00666e80
bool UnitFlyAgent::IsTransport() const
{
    const GameObject* pResult = GetMe()->GetGameObject();
    return pResult->GetGameObjectClass()->GetVehicleType() == 5 ? true : false;
}

// FUNCTION: SWBF2 0x00667440
float UnitFlyAgent::GetUpdateRate() const
{
    if (GetFlyer()->mbCirclingPoint)
    {
        return 0.5f;
    }
    return UnitAgent::GetUpdateRate();
}

// FUNCTION: SWBF2 0x00666ea0
bool UnitFlyAgent::IsNearLandingPt()
{
    if (GameLoop::GetMissionTime() > mNextHintCheckTime)
    {
        mNextHintCheckTime = GameLoop::GetMissionTime() + 3.0f;
        mHint = 0;

        const PblMatrix& mat = mCtrl->mOwner->GetMatrix();
        const BaseHint* candidates[1];
        const int iCount = HintManager::RequestLandHints(mCtrl, mat.GetTranslation(), 80, 1, candidates,
                                                         mGotoNonOwnedCP);
        if (iCount > 0)
        {
            mHint = const_cast<BaseHint*>(candidates[AIUtil::GetRandomInt(0, iCount - 1)]);
            mHint->Reserve(mCtrl->mOwner->GetGameObject());
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00666d00
void UnitFlyAgent::UpdateStatePostVision(float dt)
{
    if (GetLowLevel().IsStuck() == 2)
    {
        mCtrl->mOwner->GetGameObject()->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
    }

    mAttackHelper.Update();

    switch (mCurState)
    {
    case 1:
        SetNextState(3);
        break;
    case 2:
        if (!IsRoofOverhead())
        {
            SetNextState(4);
        }
        break;
    case 3:
        if (GameLoop::GetMissionTime() > mWaitTime)
        {
            if (IsRoofOverhead())
            {
                SetNextState(2);
            }
            else
            {
                SetNextState(4);
            }
        }
        break;
    case 4:
        if (IsNearLandingPt())
        {
            SetNextState(5);
        }
        else if (GetLowLevel().IsAtDest())
        {
            SetNextState(4);
        }
        else if (!IsNearATAT())
        {
            GetFlyer()->mbCirclingPoint = false;
        }
        break;
    case 5:
        mHint->Reserve(mCtrl->mOwner->GetGameObject());
        if (GetFlyer()->mState == 0)
        {
            SetNextState(6);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00666f30
bool UnitFlyAgent::IsNearATAT()
{
    if (AIUtil::CountUnitsInVehicle(mCtrl->mOwner) != 2)
    {
        return false;
    }

    EntityFlyer* pFlyer = GetFlyer();
    if (!pFlyer->mbCirclingPoint)
    {
        if (pFlyer->mAIEng.ai.mReactionThinkTime > 0.0f)
        {
            return false;
        }
        pFlyer->mAIEng.ai.mReactionThinkTime = 4.75f;
        const float fFrequency = pFlyer->mClass->mCircleATATFrequency;
        if (fFrequency < PblRandom::GetGlobalFloat())
        {
            return false;
        }
    }

    const PblVector3 vPos = mCtrl->mOwner->GetMatrix().GetTranslation();
    GameObject* pNearby[20];
    const int iCount = VehicleManager::RequestVehiclesInRange(vPos, pNearby, 200.0f, 20);

    bool bFree = true;
    for (int i = 0; i < iCount; ++i)
    {
        Controllable* pATAT = pNearby[i]->GetControllable();
        if (!pATAT || !AIUtil::IsATAT(pATAT))
        {
            continue;
        }

        bFree = true;
        for (int j = 0; j < iCount; ++j)
        {
            GameObject* pObject = pNearby[j];
            if (!pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
            {
                continue;
            }
            EntityFlyer* pOther = static_cast<EntityFlyer*>(pObject->GetEntity());
            if (pOther != GetFlyer() && pOther->mTowCableWalker &&
                pOther->GetTowCableWalker()->GetGameObject() == pATAT->GetGameObject())
            {
                bFree = false;
            }
        }

        PblVector3 vATAT = pATAT->GetMatrix().GetTranslation();
        if (pFlyer->mTowCableWalker)
        {
            const PblVector3 vCircle = pFlyer->mPointToCircle;
            const float dx = vCircle.x - vATAT.x;
            const float dy = vCircle.y - vATAT.y;
            const float dz = vCircle.z - vATAT.z;
            if (dx * dx + dy * dy + dz * dz > 2500.0f)
            {
                bFree = false;
            }
        }

        if (bFree)
        {
            const PblVector3& vFwd = pATAT->GetMatrix().GetZAxis();
            const bool bWasCircling = pFlyer->mbCirclingPoint;
            vATAT += vFwd * 0.0f;
            pFlyer->mbCirclingPoint = true;
            pFlyer->mPointToCircle = vATAT;
            if (!bWasCircling)
            {
                pFlyer->EntityPathFollower::Reset(pFlyer->mClass);
                pFlyer->CirclePoint(40.0f);
                return true;
            }

            for (PblList<Controllable>::Itor it = pFlyer->GetControllableList(); it; ++it)
            {
                for (int k = 0; k < it->GetWeaponCount(); ++k)
                {
                    if (!it->GetWeaponPtr(k)->IsRtti(WeaponTowCable::rttiHashWeaponTowCable))
                    {
                        continue;
                    }
                    UnitController* pGunner = UnitControllerPtr(it->mCtrl);
                    if (pGunner)
                    {
                        pGunner->mThreatManager.AddThreat(pATAT->GetGameObject());
                        pGunner->ForceSetAttackHelperTarget(pATAT->GetGameObject());
                    }
                }
            }
            return true;
        }
    }

    if (!GetFlyer()->mbCirclingPoint || !GetFlyer()->GetTowCableWalker() || !bFree)
    {
        GetFlyer()->mbCirclingPoint = false;
    }
    return false;
}

// FUNCTION: SWBF2 0x006673b0
bool UnitFlyAgent::IsRoofOverhead() const
{
    PblVector3 vStart;
    vStart = mCtrl->mOwner->GetMatrix().GetTranslation();
    const PblVector3 vEnd(vStart.x + 0.0f, vStart.y + 100.0f, vStart.z + 0.0f);
    const GameObject* pIgnore = GetMe()->GetGameObject();
    return CollisionManager::RayTest(vStart, vEnd, &pIgnore, 1, 0x9a);
}

// FUNCTION: SWBF2 0x006674a0
int UnitFlyAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
    case AI::EVT_Grenade:
    case AI::EVT_Audible_CanHearEnemy:
        return 0;
    case AI::EVT_Damaged:
        PlayDamageEventVO(pInfo);
        return 0;
    case AI::EVT_Vision_CanSeeEnemy:
        if (mCurState == 5 || mCurState == 2 || IsTransport())
        {
            return 0;
        }
        return UnitAgent::EventHandler(pInfo);
    default:
        return UnitAgent::EventHandler(pInfo);
    }
}
