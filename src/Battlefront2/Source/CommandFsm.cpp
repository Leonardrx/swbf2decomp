#include "CommandFsm.h"
#include "UnitMobileController.h"
#include "PebbleFL/Common/PblMath.h"

// SYNTHETIC: SWBF2 0x00667ef2
// ??_EUnitFlyController@@WHI@AEPAXI@Z

// SYNTHETIC: SWBF2 0x00667ee7
// ??_EUnitFlyController@@WBME@AEPAXI@Z

// SYNTHETIC: SWBF2 0x00669317
// ??_EUnitMobileController@@WHI@AEPAXI@Z

// SYNTHETIC: SWBF2 0x0066930c
// ??_EUnitMobileController@@WBME@AEPAXI@Z

// SYNTHETIC: SWBF2 0x00669890
// ??_EUnitStationaryController@@WBME@AEPAXI@Z

// SYNTHETIC: SWBF2 0x0066989b
// ??_EUnitStationaryController@@WHI@AEPAXI@Z





// FUNCTION: SWBF2 0x00478780
CommandFsm::CommandFsm()
{
    mCurCmd = CmdInfo();
    mForceCommand = false;
    mCommandDone = true;
    mCmdCount = 0;
}

// SYNTHETIC: SWBF2 0x00478690
// CommandFsm::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00478700
void CommandFsm::ApplyNextCommand()
{
    --mCmdCount;
    mCurCmd = mCmdQueue[0];
    for (int i = 0; i < mCmdCount; i++)
    {
        mCmdQueue[i] = mCmdQueue[i + 1];
    }
    mCommandDone = false;
}

// FUNCTION: SWBF2 0x00478750
bool CommandFsm::IsNewCommand()
{
    bool bNew = mCommandDone && mCmdCount > 0;
    bNew |= mForceCommand;
    return bNew;
}

// FUNCTION: SWBF2 0x00478810
void CommandFsm::FSMUpdate(float dt, bool b)
{
    if (b)
    {
        if (!IsNewCommand())
        {
            Fsm::FSMUpdate(dt, true);
        }
        return;
    }

    do
    {
        if (IsNewCommand())
        {
            ApplyNextCommand();
            mForceCommand = false;
            ApplyNextState();
        }
        Fsm::FSMUpdate(dt, false);
    } while (IsNewCommand());
}

// FUNCTION: SWBF2 0x00478890
void CommandFsm::ForceCommand(const CmdInfo& cmd)
{
    mForceCommand = true;
    mCmdCount = 1;
    mCmdQueue[0] = cmd;
}

// FUNCTION: SWBF2 0x004788b0
void CommandFsm::QueueCommand(const CmdInfo& cmd)
{
    if (mCmdCount < 5)
    {
        mCmdQueue[mCmdCount++] = cmd;
    }
}

// FUNCTION: SWBF2 0x004788e0
void CommandFsm::RemoveQueuedCommands(int what)
{
    int kept = 0;
    int count = mCmdCount < 5 ? mCmdCount : 5;
    for (int i = 0; i < count; i++)
    {
        if (mCmdQueue[i].mWhat == 0)
        {
            break;
        }
        if (mCmdQueue[i].mWhat != what)
        {
            mCmdQueue[kept++] = mCmdQueue[i];
        }
    }
    mCmdCount = kept;
}

// FUNCTION: SWBF2 0x00478960
void CommandFsm::PrependQueueCommand(const CmdInfo& cmd)
{
    if (mForceCommand)
    {
        return;
    }

    mCmdCount = Min(mCmdCount + 1, 5);
    for (int i = mCmdCount - 1; i > 0; i--)
    {
        mCmdQueue[i] = mCmdQueue[i - 1];
    }
    mCmdQueue[0] = cmd;
    mForceCommand = true;
}

// FUNCTION: SWBF2 0x004789c0
void CommandFsm::PrependQueueCommands(const CmdInfo& cmdA, const CmdInfo& cmdB)
{
    if (mForceCommand)
    {
        return;
    }

    mCmdCount = Min(mCmdCount + 2, 5);
    for (int i = mCmdCount - 1; i > 1; i--)
    {
        mCmdQueue[i] = mCmdQueue[i - 2];
    }
    mCmdQueue[0] = cmdB;
    mCmdQueue[1] = cmdA;
    mForceCommand = true;
}

