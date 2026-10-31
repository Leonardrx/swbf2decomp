#include <float.h>

#include "AttackHelper.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Aimer.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "UnitAgent.h"
#include "UnitAttackPatterns.h"
#include "UnitController.h"

#include "GameObject.h"

#include "UnitThreatManager.h"

extern PblVector3 gVectorZero;

namespace AI
{

enum VOType
{
    VOT_AcquiredTarget = 1
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}

}

namespace AIUtil
{

    PblVector3 GetHisVisionTargetPos(const Controllable* pMe, const GameObject* pObj);

    bool CanSeePosition(Controllable* pMe, const PblVector3& vPos, const GameObject* pObj);
}

namespace CombatUtil
{

    bool SelectCombatResponse(Controllable* pMe, const GameObject* pTarget,
                              AttackPatterns::State* pState, float* pTime,
                              bool* pbFlag, bool bA, bool bB);
}

namespace VisionManager
{

    int GetVisualPriority(const Controllable* pMe, const GameObject* pObj);
}

namespace CombatUtil
{
bool TestWpnConstraintsPos(Controllable* pMe, const PblVector3& vAim, const PblVector3& vPos);
}

namespace
{
    inline unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }

    inline AILowLevel& LowLevelOf(Controller* pCtrl)
    {
        return UnitControllerPtr(pCtrl)->mLowLevel;
    }
}

// FUNCTION: SWBF2 0x00447a80
AttackHelper::AttackHelper(UnitAgent* pAgent)
    : mAgent(pAgent)
{
    Reset();
}

// FUNCTION: SWBF2 0x00447ab0
void AttackHelper::Reset()
{
    mTargetHandle = 0;
    mWaitingForTarget = false;
    mTargetVisible = false;
    mLastTimeSeen = -1000.0f;

    mCurPriority = (float)60000;
    ResetWaitDelay();
    mAllowEnemyUnseen = false;
}

// FUNCTION: SWBF2 0x00447af0
void AttackHelper::SetTarget(GameObject const* pTarget)
{
    mTargetHandle.mObject = pTarget;
    mTargetHandle.mSavedHandleId = pTarget ? GameObjectHandleId(pTarget) : 0;
    if (pTarget)
    {
        AI::VOHelper::Play(AI::VOT_AcquiredTarget, GetMe()->GetGameObject(), 0, pTarget, 0.0f);
    }
}

// FUNCTION: SWBF2 0x00447b40
void AttackHelper::SetTargetInfo(GameObject const* pTarget, PblVector3 const& vPos)
{
    mTargetVisible = true;
    mLastKnownTargetPos = vPos;
    mLastTimeSeen = GameLoop::GetMissionTime();
    mCurPriority = (float)VisionManager::GetVisualPriority(GetMe(), pTarget);
    mWaitingForTarget = false;
}

// FUNCTION: SWBF2 0x00447b90
bool AttackHelper::UpdateTarget()
{
    const GameObject* pVisible = mAgent->mCtrl->mThreatManager.GetVisibleObject();
    const GameObject* pOldTarget = AIUtil::GetAlivePtr(mTargetHandle, false);
    const GameObject* pTarget = pOldTarget;
    if (pTarget && !GetMe()->GetGameObject()->IsMyEnemy(pTarget))
    {
        SetTarget(pVisible);
        pTarget = AIUtil::GetAlivePtr(mTargetHandle, false);
    }
    if (!pTarget || pTarget != pVisible)
    {
        SetTarget(pVisible);
        pTarget = AIUtil::GetAlivePtr(mTargetHandle, false);
    }
    if (pTarget && pTarget == pVisible)
    {
        SetTargetInfo(pTarget, mAgent->mCtrl->mThreatManager.GetVisibleObjectPos());
    }
    else
    {
        mTargetVisible = false;
    }
    return pOldTarget != AIUtil::GetAlivePtr(mTargetHandle, false);
}

// FUNCTION: SWBF2 0x00447c50
void AttackHelper::Aim()
{
    const GameObject* pTarget = AIUtil::GetAlivePtr(mTargetHandle, false);
    if (GetVisibleTarget())
    {
        AILowLevel& lowLevel = LowLevelOf(GetMe()->mCtrl);
        lowLevel.SetTarget(pTarget);
    }
    else if (pTarget)
    {
        AILowLevel& lowLevelClear = LowLevelOf(GetMe()->mCtrl);
        lowLevelClear.SetTarget(0);
        AILowLevel& lowLevelAim = LowLevelOf(GetMe()->mCtrl);
        lowLevelAim.SetAimPoint(mLastKnownTargetPos);
    }
    else
    {
        AILowLevel& lowLevelClear = LowLevelOf(GetMe()->mCtrl);
        lowLevelClear.SetTarget(0);
    }
}

// FUNCTION: SWBF2 0x00447cd0
void AttackHelper::Fire()
{
    bool bImmediate = false;
    bool bUponAim = false;
    const GameObject* pTarget = AIUtil::GetAlivePtr(mTargetHandle, false);
    if (pTarget)
    {
        if (mTargetVisible)
        {
            bImmediate = false;
            bUponAim = true;
        }
        else
        {
            const Aimer* pAimer = GetMe()->GetCurAimer();
            PblVector3 vLead;
            vLead = mLastKnownTargetPos;
            if (pAimer)
            {
                pAimer->GetLeadPos(mLastKnownTargetPos, gVectorZero, &vLead);
            }
            if (CombatUtil::TestWpnConstraintsPos(GetMe(), vLead, mLastKnownTargetPos) && TargetIsHiding())
            {
                bImmediate = AIUtil::CanSeePosition(GetMe(), mLastKnownTargetPos, pTarget) ? true : false;
            }
        }
    }
    AILowLevel& lowLevel = LowLevelOf(GetMe()->mCtrl);
    lowLevel.SetFireRequest(bImmediate, bUponAim);
}

// FUNCTION: SWBF2 0x00447da0
bool AttackHelper::TargetIsHiding() const
{
    PblVector3 vPos = AIUtil::GetHisVisionTargetPos(GetMe(), AIUtil::GetAlivePtr(mTargetHandle, false));
    if (SquaredLength(mLastKnownTargetPos - vPos) < 9.0f)
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00447e10
void AttackHelper::SelectBestWeapon()
{
    if (!GetMe()->GetWeaponPtr(0))
    {
        return;
    }
    const GameObject* pTarget = AIUtil::GetAlivePtr(mTargetHandle, false);
    if (!pTarget)
    {
        return;
    }
    AttackPatterns::State eState;
    float fTime;
    bool bFlag;
    CombatUtil::SelectCombatResponse(GetMe(), pTarget, &eState, &fTime, &bFlag, true, false);
}

// FUNCTION: SWBF2 0x00447e60
void AttackHelper::Update()
{
    UpdateTarget();
    SelectBestWeapon();
    Aim();
    Fire();
}

// FUNCTION: SWBF2 0x00447e80
Controllable const* AttackHelper::GetMe() const
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x00447e90
Controllable* AttackHelper::GetMe()
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x00447ea0
bool AttackHelper::HasTarget() const
{
    const GameObject* target = AIUtil::GetAlivePtr(mTargetHandle, false);
    return target != 0;
}

// FUNCTION: SWBF2 0x00447eb0
GameObject const* AttackHelper::GetVisibleTarget() const
{
    return mTargetVisible ? AIUtil::GetAlivePtr(mTargetHandle, false) : 0;
}

// FUNCTION: SWBF2 0x00447ec0
PblVector3 const& AttackHelper::GetLastSeenTargetPos() const
{
    return mLastKnownTargetPos;
}

// FUNCTION: SWBF2 0x00447ed0
float AttackHelper::GetLastTimeSeen() const
{
    return mLastTimeSeen;
}

// FUNCTION: SWBF2 0x00447ee0
GameObject const* AttackHelper::GetTarget() const
{
    return AIUtil::GetAlivePtr(mTargetHandle, false);
}

// FUNCTION: SWBF2 0x00447ef0
void AttackHelper::SetVisibleTarget(GameObject const* pTarget)
{
    Reset();
    mTargetHandle.mObject = pTarget;
    mTargetHandle.mSavedHandleId = pTarget ? GameObjectHandleId(pTarget) : 0;
    PblVector3 vPos = AIUtil::GetHisVisionTargetPos(GetMe(), pTarget);
    SetTargetInfo(pTarget, vPos);
}

// FUNCTION: SWBF2 0x00447f40
void AttackHelper::SetWaitDelay(float fDelay)
{
    mWaitDelay = fDelay;
    if (mWaitingForTarget)
    {
        ResetWaitOverTime();
    }
}

// FUNCTION: SWBF2 0x00447f50
void AttackHelper::ResetWaitDelay()
{
    mWaitDelay = 3.0f;
    if (mWaitingForTarget)
    {
        ResetWaitOverTime();
    }
}

// FUNCTION: SWBF2 0x00447f70
void AttackHelper::AllowEnemyUnseen(bool bAllow)
{
    mAllowEnemyUnseen = bAllow;
    if (mWaitingForTarget)
    {
        ResetWaitOverTime();
    }
}

// FUNCTION: SWBF2 0x00447f90
void AttackHelper::ResetWaitOverTime()
{
    if (mAllowEnemyUnseen)
    {
        mWaitOverTime = FLT_MAX;
    }
    else
    {
        mWaitOverTime = GameLoop::GetMissionTime() + mWaitDelay;
    }
}

// GLOBAL: SWBF2 0x007888d0

extern const unsigned char kBase64DecodeValue[128] = {
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127, 62,127,127,127, 63,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,127,127,127, 64,127,127,
    127,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,127,127,127,127,127,
    127, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,127,127,127,127,127
};

// FUNCTION: SWBF2 0x00448490
int __cdecl DecodeBase64(unsigned char* pOutput, unsigned int uiOutputCapacity, unsigned int* pOutputSize,
                       const unsigned char* pSource, unsigned int uiSourceSize)
{
    // First pass: validate the input and count the output bytes.
    unsigned int i;
    unsigned int uiValid = 0;
    unsigned int uiPadding = 0;
    for (i = 0; i < uiSourceSize; ++i)
    {
        // Skip spaces before checking for an end of line.
        unsigned int uiSpaces = 0;
        while (i < uiSourceSize && pSource[i] == ' ')
        {
            ++i;
            ++uiSpaces;
        }

        // Spaces at the end of the buffer are fine.
        if (i == uiSourceSize)
        {
            break;
        }

        if (uiSourceSize - i >= 2 && pSource[i] == '\r' && pSource[i + 1] == '\n')
        {
            continue;
        }
        if (pSource[i] == '\n')
        {
            continue;
        }

        // A space inside a line is an error.
        if (uiSpaces != 0)
        {
            return -44;
        }
        if (pSource[i] == '=' && ++uiPadding > 2)
        {
            return -44;
        }
        if (pSource[i] > 127 || kBase64DecodeValue[pSource[i]] == 127)
        {
            return -44;
        }
        if (kBase64DecodeValue[pSource[i]] < 64 && uiPadding != 0)
        {
            return -44;
        }
        ++uiValid;
    }

    if (uiValid == 0)
    {
        *pOutputSize = 0;
        return 0;
    }

    const unsigned int uiNeeded = ((uiValid * 6 + 7) >> 3) - uiPadding;
    if (!pOutput || uiNeeded > uiOutputCapacity)
    {
        *pOutputSize = uiNeeded;
        return -42;
    }

    // Second pass: decode four characters at a time into three bytes.
    unsigned char* pWrite = pOutput;
    unsigned int nBytes = 3;
    unsigned int nQuartet = 0;
    unsigned int quartet = 0;
    for (; i > 0; --i, ++pSource)
    {
        if (*pSource == '\r' || *pSource == '\n' || *pSource == ' ')
        {
            continue;
        }

        nBytes -= (kBase64DecodeValue[*pSource] == 64);
        quartet = (quartet << 6) | (kBase64DecodeValue[*pSource] & 63);

        if (++nQuartet == 4)
        {
            nQuartet = 0;
            if (nBytes > 0)
            {
                *pWrite++ = (unsigned char)(quartet >> 16);
            }
            if (nBytes > 1)
            {
                *pWrite++ = (unsigned char)(quartet >> 8);
            }
            if (nBytes > 2)
            {
                *pWrite++ = (unsigned char)quartet;
            }
        }
    }
    *pOutputSize = (unsigned int)(pWrite - pOutput);
    return 0;
}
