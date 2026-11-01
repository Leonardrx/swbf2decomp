#include "UnitWaitAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"

// SYNTHETIC: SWBF2 0x00406d50
// `dynamic initializer for 'UnitWaitAgent::rttiHashUnitWaitAgent''
// UnitWaitAgent::rttiHashUnitWaitAgent

// GLOBAL: SWBF2 0x01f9a258
// UnitWaitAgent::rttiHashUnitWaitAgent
PblHash UnitWaitAgent::rttiHashUnitWaitAgent("UnitWaitAgent");
// GLOBAL: SWBF2 0x0078b9b8
// UnitWaitAgent::rttiNameUnitWaitAgent
const char UnitWaitAgent::rttiNameUnitWaitAgent[] = "UnitWaitAgent";







// FUNCTION: SWBF2 0x0066b3b0
UnitWaitAgent::UnitWaitAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl)
{
    mCurState = 1;
    mNextState = 2;
}

// FUNCTION: SWBF2 0x0066b3e0
void UnitWaitAgent::EnterState()
{
    switch (mCurState)
    {
    case 2:
        mTargetTimer = GameLoop::GetMissionTime();
        SetNewSearchHeading();
        break;
    }
}

// FUNCTION: SWBF2 0x0066b400
void UnitWaitAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x0066b410
void UnitWaitAgent::SetNewSearchHeading()
{
    float fRange = 1.3089969f;
    if (mCtrl->IsLookAheadOnly())
    {
        fRange = 0.52359879f;
    }
    float fAngle = AIUtil::GetRandomFloat(-fRange, fRange);

    PblVector3 vRotated;
    {
        const PblVector3& vAhead = mCtrl->GetOriginalAheadDir();
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, fAngle);
        vRotated = PblVector3(vAhead.x * fCos - vAhead.z * fSin,
                              vAhead.y,
                              vAhead.z * fCos + vAhead.x * fSin);
    }

    mSearchPos = GetMe()->GetMatrix().GetTranslation() + vRotated * 1000.0f;
}

// FUNCTION: SWBF2 0x0066b500
void UnitWaitAgent::UpdateStatePostVision(float dt)
{
    AttackHelper& attack = mAttackHelper;
    attack.Update();
    switch (mCurState)
    {
    case 2:
    {
        if (AIUtil::GetRandomIntPerSecond(1, 9) == 2)
        {
            SetNewSearchHeading();
        }

        GameObject* pObj = GetMe()->GetGameObject();
        if (pObj && pObj->GetMaxSpeed() == 0.0f)
        {
            if (attack.HasTarget())
            {
                mTargetTimer = GameLoop::GetMissionTime();
            }
            else if (GameLoop::GetMissionTime() > mTargetTimer + AIUtil::gStationaryWaitTime)
            {
                GetLowLevel().SetExitVehicle(0);
                return;
            }
        }

        if (attack.GetVisibleTarget())
        {
            GetLowLevel().SetHeading(attack.GetVisibleTarget());
        }
        else if (attack.HasTarget())
        {
            const PblVector3& vLastSeen = attack.GetLastSeenTargetPos();
            GetLowLevel().SetHeading(PblVector3(vLastSeen.x + 0.0f, vLastSeen.y + 1.6f, vLastSeen.z + 0.0f), 0.0f);
        }
        else
        {
            GetLowLevel().SetHeading(PblVector3(mSearchPos.x + 0.0f, mSearchPos.y + 1.6f, mSearchPos.z + 0.0f), 0.0f);
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0066b660
const char* UnitWaitAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// SYNTHETIC: SWBF2 0x0066b380
// UnitWaitAgent::`scalar deleting destructor'
