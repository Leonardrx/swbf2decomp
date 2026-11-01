#include "FLJoystick.h"

#include <math.h>
#include <string.h>

#include "FLInputManager.h"
#include "LuaHelper.h"
#include "RedEngineFL/input/RedJoystick.h"
#include "GameLoop.h"
#include "PebbleFL/Common/PblMath.h"

extern bool gAttractMode;

// FUNCTION: SWBF2 0x0052b490
void RawControllerInputs::ClearInputsToOff()
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 76; j++)
        {
            if ((j >= 66 && j <= 75) || j <= 4u)
            {
                mLockedRawInputBool[i][j] = fabsf(mRawInputFloat[i][j]) >= 0.125f;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0052b620
ProcessedInputs::ProcessedInputs()
{
    memset(mFloatInputs, 0, sizeof(mFloatInputs));
    bits.mBoolInputs = 0;
    bits.mMouseMode = 2;
}

// FUNCTION: SWBF2 0x0052b880
bool ProcessedInputs::GetDigital(ePROCESSEDINPUT_TYPE eType) const
{
    return (bits.mBoolInputs & (1 << eType)) != 0;
}

// FUNCTION: SWBF2 0x0052b8b0
void ProcessedInputs::Clear()
{
    bits.mBoolInputs = 0;
    memset(mFloatInputs, 0, sizeof(mFloatInputs));
}

// FUNCTION: SWBF2 0x0052b920
void GUIInputs::Clear()
{
    memcpy(mLastGUIInputs, mGUIInputs, sizeof(mGUIInputs));
    memset(mGUIInputs, 0, sizeof(mGUIInputs));
    memset(mProcessedGUIInputs, 0, sizeof(mProcessedGUIInputs));
}

// FUNCTION: SWBF2 0x0052b9a0
void GUIInputs::Process()
{
    for (int i = 0; i < eGUIINPUT_MAX; i++)
    {
        float fAbs = mGUIInputs[i] < 0.0f ? -mGUIInputs[i] : mGUIInputs[i];
        bool bRepeat = false;
        switch (i)
        {
        case eGUIINPUT_Up:
        case eGUIINPUT_Down:
        case eGUIINPUT_Left:
        case eGUIINPUT_Right:
        case eGUIINPUT_yAxis:
        case eGUIINPUT_xAxis:
            bRepeat = true;
            break;
        default:
            break;
        }

        lua_pushstring(LuaHelper::mState, "gbWantMousedownEvents");
        lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
        bool bSend = bRepeat;
        if (!lua_isnil(LuaHelper::mState, -1))
        {
            bSend = true;
        }
        lua_pop(LuaHelper::mState, 1);

        if (i != eGUIINPUT_FreeEyeUp && i != eGUIINPUT_FreeEyeDown)
        {
            if (mGUIInputs[i] == 0.0f)
            {
                continue;
            }
            if (!bSend && mGUIInputs[i] == mLastGUIInputs[i])
            {
                continue;
            }
            if (i == eGUIINPUT_xAxis || i == eGUIINPUT_yAxis)
            {
                mProcessedGUIInputs[i] = mGUIInputs[i];
                continue;
            }
        }
        mProcessedGUIInputs[i] = fAbs > mGUIInputs[i] ? fAbs : mGUIInputs[i];
    }
}

// FUNCTION: SWBF2 0x0052b8c0
float ProcessedInputs::IsPrimarySwitchButtonPressed() const
{
    if (bits.mBoolInputs & 0x2000)
    {
        return 1.0f;
    }
    if (bits.mBoolInputs & 0x4000)
    {
        return -1.0f;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x0052b8f0
float ProcessedInputs::IsSecondarySwitchButtonPressed() const
{
    if (bits.mBoolInputs & 0x8000)
    {
        return 1.0f;
    }
    if (bits.mBoolInputs & 0x10000)
    {
        return -1.0f;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x0052b5d0
bool RawControllerInputs::IsDefaultBindings(int iMode)
{
    return memcmp(&mRawInputStorage[0x1acc - 4 + iMode * sizeof(sDefaultBindings[0])],
                  sDefaultBindings[iMode], sizeof(sDefaultBindings[0])) == 0;
}

// FUNCTION: SWBF2 0x0052b510
float RawControllerInputs::AdjustValueForSensitivity(float fValue, unsigned char ucSensitivity)
{
    float fBelowKneeScale = (ucSensitivity + 1.0f) * 0.2f;
    float fKneePoint = 0.85f;
    if (fBelowKneeScale > 1.0f && fBelowKneeScale * fKneePoint > 1.0f)
    {
        fKneePoint = 1.0f / fBelowKneeScale;
    }

    float fScale;
    if (fabsf(fValue) < fKneePoint)
    {
        fScale = fBelowKneeScale;
    }
    else
    {
        fScale = Clamp((fabsf(fValue) - fKneePoint) / (1.0f - fKneePoint), 0.0f, 1.0f);
        fScale = fBelowKneeScale * (1.0f - fScale) + fScale;
    }
    return Clamp(fScale * fValue, -1.0f, 1.0f);
}

// FUNCTION: SWBF2 0x0052bab0
void Rumble::StopIntermittent()
{
    if (mRumbleMode == eIntermittent)
    {
        mRumbleMode = eNone;
    }
}

// FUNCTION: SWBF2 0x0052b640
void ProcessedInputs::Set(ePROCESSEDINPUT_TYPE eType, bool& bLocked, float fValue, float fLastValue)
{
    if (eType == ePROCESSEDINPUT_NONE)
    {
        return;
    }

    if (bLocked == true)
    {
        if (fabsf(fValue) < 0.125f)
        {
            bLocked = false;
        }
        else
        {
            fValue = 0.0f;
        }
    }

    if ((unsigned int)(eType - ePROCESSEDINPUT_BOOLCOUNT) <= 3)
    {
        float& fAxis = mFloatInputs[eType - ePROCESSEDINPUT_BOOLCOUNT];
        const float fOldAxis = fAxis;
        const float fAbsAxis = fOldAxis < 0.0f ? -fOldAxis : fOldAxis;
        const float fAbsValue = fValue < 0.0f ? -fValue : fValue;
        if (!(fAbsAxis > fAbsValue))
        {
            fAxis = fValue;
        }
        return;
    }

    float fCurrent;
    switch (eType)
    {
    case ePROCESSEDINPUT_STRAFE_POS:
        fCurrent = mFloatInputs[0];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[0] = fValue;
        }
        break;
    case ePROCESSEDINPUT_STRAFE_NEG:
        fCurrent = mFloatInputs[0];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[0] = -fValue;
        }
        break;
    case ePROCESSEDINPUT_MOVE_POS:
        fCurrent = mFloatInputs[1];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[1] = fValue;
        }
        break;
    case ePROCESSEDINPUT_MOVE_NEG:
        fCurrent = mFloatInputs[1];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[1] = -fValue;
        }
        break;
    case ePROCESSEDINPUT_TURN_POS:
        fCurrent = mFloatInputs[2];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[2] = fValue;
        }
        break;
    case ePROCESSEDINPUT_TURN_NEG:
        fCurrent = mFloatInputs[2];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[2] = -fValue;
        }
        break;
    case ePROCESSEDINPUT_PITCH_POS:
        fCurrent = mFloatInputs[3];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[3] = fValue;
        }
        break;
    case ePROCESSEDINPUT_PITCH_NEG:
        fCurrent = mFloatInputs[3];
        if (fValue > (fCurrent < 0.0f ? -fCurrent : fCurrent))
        {
            mFloatInputs[3] = -fValue;
        }
        break;

    case ePROCESSEDINPUT_useButtonPressed:
    case ePROCESSEDINPUT_primaryNextButtonPressed:
    case ePROCESSEDINPUT_primaryPrevButtonPressed:
    case ePROCESSEDINPUT_secondaryNextButtonPressed:
    case ePROCESSEDINPUT_secondaryPrevButtonPressed:
    case ePROCESSEDINPUT_playerList:
    case ePROCESSEDINPUT_chat:
    case ePROCESSEDINPUT_friendChat:
    case ePROCESSEDINPUT_Comm_Chat_0:
    case ePROCESSEDINPUT_Comm_Chat_1:
    case ePROCESSEDINPUT_Comm_Chat_2:
    case ePROCESSEDINPUT_Comm_Chat_3:
    case ePROCESSEDINPUT_Comm_Chat_4:
    case ePROCESSEDINPUT_Comm_Chat_5:
    case ePROCESSEDINPUT_Comm_Chat_6:
    case ePROCESSEDINPUT_Comm_Chat_7:
    case ePROCESSEDINPUT_Map:
        if (fValue >= 0.125f && fLastValue < 0.125f)
        {
            bits.mBoolInputs |= 1 << eType;
        }
        break;

    case ePROCESSEDINPUT_primaryFireButtonDown:
    case ePROCESSEDINPUT_secondaryFireButtonDown:
    case ePROCESSEDINPUT_sprintButtonDown:
    case ePROCESSEDINPUT_jumpButtonDown:
    case ePROCESSEDINPUT_crouchButtonDown:
    case ePROCESSEDINPUT_zoomButtonDown:
    case ePROCESSEDINPUT_viewButtonDown:
    case ePROCESSEDINPUT_reloadButtonDown:
    case ePROCESSEDINPUT_squadCommandButtonPressed:
    case ePROCESSEDINPUT_acceptHeroPressed:
    case ePROCESSEDINPUT_declineHeroPressed:
    case ePROCESSEDINPUT_lockTargetButtonPressed:
    case ePROCESSEDINPUT_TakeScreenshot:
        if (fValue >= 0.125f)
        {
            bits.mBoolInputs |= 1 << eType;
        }
        break;

    default:
        if (fValue >= 0.125f)
        {
            bits.mBoolInputs |= 1 << eType;
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0052b960
void GUIInputs::Set(eGUIINPUT_TYPE eType, float fValue)
{
    if (eType >= 0 && eType < eGUIINPUT_MAX)
    {
        if (eType != eGUIINPUT_xAxis && eType != eGUIINPUT_yAxis)
        {
            const float fCurrent = mGUIInputs[eType];
            if (!((fValue < 0.0f ? -fValue : fValue) > (fCurrent < 0.0f ? -fCurrent : fCurrent)))
            {
                fValue = fCurrent;
            }
        }
        mGUIInputs[eType] = fValue;
    }
}

// FUNCTION: SWBF2 0x0052bac0
void Rumble::StopAllRumble()
{
    mRumbleMode = eNone;
    mDelayLight = 0.0f;
    mDelayHeavy = 0.0f;
    mLight = 0.0f;
    mHeavy = 0.0f;
    if (mJoystick)
    {
        mJoystick->SetLightEngine(0.0f);
        mJoystick->SetHeavyEngine(0.0f);
        mJoystick->Update(0.0f);
    }
}

// FUNCTION: SWBF2 0x0052bb20
void Rumble::Update(float fDeltaTime)
{
    if (GameLoop::IsGameOver())
    {
        mDelayLight = 0.0f;
        mDelayHeavy = 0.0f;
        mLight = 0.0f;
        mHeavy = 0.0f;
    }

    if (!mJoystick || !mJoystick->m_connected || !mEnabled || gAttractMode)
    {
        return;
    }

    switch (mRumbleMode)
    {
    case eRumble:
        mDelayLight -= fDeltaTime;
        mDelayHeavy -= fDeltaTime;
        if (mDelayLight <= 0.0f)
        {
            mLight -= mLightDecay * fDeltaTime;
            mTimeLeftLight -= fDeltaTime;
            if (mLight > 0.0f && mTimeLeftLight > 0.0f)
            {
                mJoystick->SetLightEngine(mLight);
            }
        }
        if (mDelayHeavy <= 0.0f)
        {
            mHeavy -= mHeavyDecay * fDeltaTime;
            mTimeLeftHeavy -= fDeltaTime;
            if (mHeavy > 0.0f && mTimeLeftHeavy > 0.0f)
            {
                mJoystick->SetHeavyEngine(mHeavy);
            }
        }
        if ((mLight <= 0.0f && mHeavy <= 0.0f) || (mTimeLeftHeavy <= 0.0f && mTimeLeftLight <= 0.0f))
        {
            mRumbleMode = eNone;
        }
        break;

    case eIntermittent:
        mTime += fDeltaTime;
        if (mTime > mInterval)
        {
            mDelayLight -= fDeltaTime;
            mDelayHeavy -= fDeltaTime;
            if (mDelayLight <= 0.0f)
            {
                mJoystick->SetLightEngine(mLight);
                mDelayLight = mLightDecay;
            }
            if (mDelayHeavy <= 0.0f)
            {
                mJoystick->SetHeavyEngine(mHeavy);
                mDelayHeavy = mHeavyDecay;
            }
            mTime -= mInterval;
        }
        break;

    default:
        mDelayLight = 0.0f;
        mDelayHeavy = 0.0f;
        mLight = 0.0f;
        mHeavy = 0.0f;
        break;
    }
}
