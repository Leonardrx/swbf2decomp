#pragma once

#include "PebbleFL/Common/PblRandom.h"

class VarianceValue
{
public:

    // FUNCTION: SWBF2 0x00609c40
    float GetValue(PblRandom& rand) const
    {
        if (mVariance != 0.0f)
        {
            return rand.GetFloat() * mVariance + mValue;
        }
        return mValue;
    }

    void Setup(float fValue)
    {
        mValue = fValue;
        mVariance = 0.0f;
    }

    void Setup(float fMin, float fMax)
    {
        if (fMin == fMax)
        {
            Setup(fMin);
        }
        else if (fMin > fMax)
        {
            mValue = fMax;
            mVariance = fMin - fMax;
        }
        else
        {
            mValue = fMin;
            mVariance = fMax - fMin;
        }
    }

    float GetMin() const { return mValue; }
    float GetMax() const { return mValue + mVariance; }

    float mValue;
    float mVariance;
};
