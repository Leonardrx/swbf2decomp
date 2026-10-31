#include "UnitCombatAgent.h"
#include "PebbleFL/Common/PblHashTable.h"

// SYNTHETIC: SWBF2 0x00406c40
// `dynamic initializer for 'UnitCombatAgent::rttiHashUnitCombatAgent''
// UnitCombatAgent::rttiHashUnitCombatAgent

// GLOBAL: SWBF2 0x01f9935c
// UnitCombatAgent::rttiHashUnitCombatAgent
PblHash UnitCombatAgent::rttiHashUnitCombatAgent("UnitCombatAgent");
// GLOBAL: SWBF2 0x0078b778
// UnitCombatAgent::rttiNameUnitCombatAgent
const char UnitCombatAgent::rttiNameUnitCombatAgent[] = "UnitCombatAgent";







// FUNCTION: SWBF2 0x00663020
const char* UnitCombatAgent::NFINAL_GetStateString(int) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00662e60
bool UnitCombatAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return false;
}

// SYNTHETIC: SWBF2 0x00662e70
// UnitCombatAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00662ed0
UnitCombatAgent::UnitCombatAgent(UnitController* pCtrl, const UnitCombatInfo& info)
    : UnitAgent(pCtrl), mCombatInfo(info), mCombatHelper(this)
{
    mCurState = STATE_START;
    mNextState = STATE_Combat;
}

// FUNCTION: SWBF2 0x00662f10
void UnitCombatAgent::Kill()
{
    mCombatHelper.Kill();
    UnitAgent::Kill();
}

// FUNCTION: SWBF2 0x00662f30
float UnitCombatAgent::GetUpdateRate() const
{
    if (mCurState == STATE_Combat)
    {
        return mCombatHelper.GetUpdateRate();
    }
    return UnitAgent::GetUpdateRate();
}

// FUNCTION: SWBF2 0x00662f60
void UnitCombatAgent::EnterState()
{
    switch (mCurState)
    {
    case STATE_Combat:
        mCombatHelper.Init(mCombatInfo);
        break;
    }
}

// FUNCTION: SWBF2 0x00662f80
void UnitCombatAgent::ExitState()
{
    switch (mCurState)
    {
    case STATE_Combat:
        mCombatHelper.Clean();
        break;
    }
}

// FUNCTION: SWBF2 0x00662fa0
void UnitCombatAgent::UpdateStatePostVision(float dt)
{
    switch (mCurState)
    {
    case STATE_Combat:
        mCombatHelper.FSMUpdate(dt, false);
        if (mCombatHelper.IsDone())
        {
            SetNextState(100);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00662ff0
int UnitCombatAgent::EventHandler(AI::EventInfo* pInfo)
{
    if (mCurState == STATE_Combat)
    {
        int iResult = mCombatHelper.EventHandler(pInfo);
        if (iResult)
        {
            return iResult;
        }
    }
    return UnitAgent::EventHandler(pInfo);
}
