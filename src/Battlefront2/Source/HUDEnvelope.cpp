#include "HUDEnvelope.h"

#include <float.h>
#include <string.h>

// FUNCTION: SWBF2 0x0055d900
HUD::Envelope::Envelope()
    : mTime(0.0f),
      mPhase(phase_Inactive)
{
    memset(mPhaseTime, 0, sizeof(mPhaseTime));
    mPhaseTime[phase_Hold] = FLT_MAX;
}

// FUNCTION: SWBF2 0x0055d930
void HUD::Envelope::Start()
{
    switch (mPhase)
    {
    case phase_Inactive:
        mTime = 0.0f;
        mPhase = phase_Attack;
        break;
    case phase_Attack:
        break;
    case phase_Hold:
        mTime = 0.0f;
        break;
    case phase_Sustain:
        mTime = 0.0f;
        mPhase = phase_Hold;
        break;
    case phase_Release:
        mTime = mPhaseTime[phase_Release] != 0.0f
                    ? mTime / mPhaseTime[phase_Release] * mPhaseTime[phase_Attack]
                    : 0.0f;
        mPhase = phase_Attack;
        break;
    }
}

// FUNCTION: SWBF2 0x0055d9b0
void HUD::Envelope::Stop()
{
    switch (mPhase)
    {
    case phase_Attack:
        mTime = mPhaseTime[phase_Attack] != 0.0f
                    ? mTime / mPhaseTime[phase_Attack] * mPhaseTime[phase_Release]
                    : 0.0f;
        mPhase = phase_Release;
        break;
    case phase_Hold:
        mTime = 0.0f;
        mPhase = phase_Sustain;
        break;
    }
}

// FUNCTION: SWBF2 0x0055d9f0
HUD::Envelope::Phase HUD::Envelope::Update(float fTime)
{
    while (mPhase > phase_Inactive &&
           (mPhaseTime[mPhase] == 0.0f || mTime > mPhaseTime[mPhase]))
    {
        mTime -= mPhaseTime[mPhase];
        mPhase = (Phase)(mPhase + 1);
        if (mPhase > phase_Release)
        {
            mPhase = phase_Inactive;
        }
    }
    mTime += fTime;
    return mPhase;
}

// FUNCTION: SWBF2 0x0055da70
float HUD::Envelope::CalculateValue() const
{
    switch (mPhase)
    {
    case phase_Inactive:
        return 0.0f;
    case phase_Attack:
        if (mTime < mPhaseTime[phase_Attack])
        {
            return mTime / mPhaseTime[phase_Attack];
        }
        return 1.0f;
    case phase_Hold:
    case phase_Sustain:
        return 1.0f;
    case phase_Release:
        if (mTime < mPhaseTime[phase_Release])
        {
            return 1.0f - mTime / mPhaseTime[phase_Release];
        }
        return 0.0f;
    default:
        return 0.0f;
    }
}

// FUNCTION: SWBF2 0x0055db00
HUD::Envelope& HUD::Envelope::operator=(const Envelope& other)
{
    memcpy(mPhaseTime, other.mPhaseTime, sizeof(mPhaseTime));
    return *this;
}

// FUNCTION: SWBF2 0x0055da60
HUD::Envelope::Phase HUD::Envelope::GetPhase() const { return mPhase; }

// FUNCTION: SWBF2 0x0055daf0
float HUD::Envelope::GetPhaseTime(Phase ePhase) const { return mPhaseTime[ePhase]; }

// FUNCTION: SWBF2 0x0055dae0
void HUD::Envelope::SetPhaseTime(Phase ePhase, float fTime) { mPhaseTime[ePhase] = fTime; }
