#pragma once

#include <math.h>

class PblAngle
{
public:

    // FUNCTION: SWBF2 0x0049e2e0
    PblAngle() : mCos(1.0f), mSin(0.0f) {}
    // FUNCTION: SWBF2 0x0048c8e0
    PblAngle(float fCos, float fSin) : mCos(fCos), mSin(fSin) {}

    explicit PblAngle(float fAngle) : mCos(cosf(fAngle)), mSin(sinf(fAngle)) {}

    PblAngle& operator-=(const PblAngle& a)
    {
        float fCos = mSin * a.mSin + mCos * a.mCos;
        float fSin = mSin * a.mCos - mCos * a.mSin;
        mCos = fCos;
        mSin = fSin;
        return *this;
    }

    void Normalize()
    {
        float fInv = 1.0f / sqrtf(mCos * mCos + mSin * mSin);
        mCos = mCos * fInv;
        mSin = mSin * fInv;
    }

    void Renormalize()
    {
        float fLength2 = mCos * mCos + mSin * mSin;
        float fHalf = (3.0f - fLength2) * 0.5f;
        float fScale = (3.0f - fHalf * fLength2 * fHalf) * (fHalf * 0.5f);
        mCos = fScale * mCos;
        mSin = fScale * mSin;
    }

    PblAngle& operator+=(const PblAngle& a)
    {
        float fSin = mCos * a.mSin + mSin * a.mCos;
        mCos = mCos * a.mCos - mSin * a.mSin;
        mSin = fSin;
        return *this;
    }

    // FUNCTION: SWBF2 0x0047a550
    float GetCos() const { return mCos; }
    // FUNCTION: SWBF2 0x0047a560
    float GetSin() const { return mSin; }

    float mCos;
    float mSin;
};

 
