#include "UnitHoldAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "Controllable.h"
#include "EventInfo.h"
#include "SquadCommands.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

// SYNTHETIC: SWBF2 0x00406cf0
// `dynamic initializer for 'UnitHoldAgent::rttiHashUnitHoldAgent''
// UnitHoldAgent::rttiHashUnitHoldAgent

// GLOBAL: SWBF2 0x01f993ec
// UnitHoldAgent::rttiHashUnitHoldAgent
PblHash UnitHoldAgent::rttiHashUnitHoldAgent("UnitHoldAgent");
// GLOBAL: SWBF2 0x0078b8c8
// UnitHoldAgent::rttiNameUnitHoldAgent
const char UnitHoldAgent::rttiNameUnitHoldAgent[] = "UnitHoldAgent";







// FUNCTION: SWBF2 0x00668dd0
UnitHoldAgent::UnitHoldAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl)
{
    mCurState = 1;
    mInsubValue = 1.0f;
    mNextState = 2;
}

// FUNCTION: SWBF2 0x00668e00
void UnitHoldAgent::EnterState()
{
    switch (mCurState)
    {
    case 2:
        SetNewSearchHeading();
        break;
    }
}

// FUNCTION: SWBF2 0x00668e10
void UnitHoldAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x00668e20
void UnitHoldAgent::SetNewSearchHeading()
{
    if (mCtrl->IsLookAheadOnly())
    {
        float fAngle = AIUtil::GetRandomFloat(-0.52359879f, 0.52359879f);
        const PblVector3& vAhead = mCtrl->GetOriginalAheadDir();
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, fAngle);
        PblVector3 vOffset((vAhead.x * fCos - vAhead.z * fSin) * 10.0f,
                           vAhead.y * 10.0f,
                           (vAhead.z * fCos + vAhead.x * fSin) * 10.0f);

        const PblMatrix& mat = GetMe()->GetMatrix();
        mSearchPos = PblVector3(mat.m[3][0] + vOffset.x, mat.m[3][1] + vOffset.y, mat.m[3][2] + vOffset.z);
    }
    else
    {
        const PblVector3 vDir = PblRandom::GetSim().GetVector3();
        PblVector3 vOffset(vDir.x * 10.0f, 0.0f, vDir.z * 10.0f);

        const PblMatrix& mat = GetMe()->GetMatrix();
        mSearchPos = PblVector3(mat.m[3][0] + vOffset.x, mat.m[3][1] + vOffset.y, mat.m[3][2] + vOffset.z);
    }
}

// FUNCTION: SWBF2 0x00668f70
void UnitHoldAgent::UpdateStatePostVision(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    mAttackHelper.Update();

    const Character* pLeader = mCtrl->GetLastSquadCommand().mWhoGaveIt;
    if (!pLeader || !pLeader->GetControllable())
    {
        SetNextState(100);
        return;
    }

    const PblMatrix& mine = GetMe()->GetMatrix();
    const PblMatrix& theirs = pLeader->GetControllable()->GetMatrix();
    PblVector3 vDelta = theirs.GetTranslation() - mine.GetTranslation();
    if (SquaredLength(vDelta) > 400.0f)
    {
        mInsubValue -= 0.05f;
        if (mInsubValue <= 0.0f)
        {
            SetNextState(100);
            return;
        }
    }

    if (mCurState != 2)
    {
        return;
    }

    if (AIUtil::GetRandomIntPerSecond(1, 9) == 2)
    {
        SetNewSearchHeading();
    }

    UnitAgent::FaceTargetCommon(HEIGHT_COUNT, true, mSearchPos);
}

// FUNCTION: SWBF2 0x00669130
bool UnitHoldAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return !SquadCommands::IsSquadCommand(cmd.GetCommand());
}

// FUNCTION: SWBF2 0x00669150
int UnitHoldAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
        return 0;
    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// FUNCTION: SWBF2 0x00669170
const char* UnitHoldAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// SYNTHETIC: SWBF2 0x00668da0
// UnitHoldAgent::`scalar deleting destructor'
