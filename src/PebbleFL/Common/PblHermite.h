#pragma once

#include "PblVector.h"

class PblHermiteVector3
{
public:

    PblHermiteVector3() {}

    PblHermiteVector3(const PblVector3& vP0, const PblVector3& vP1, const PblVector3& vT0,
                      const PblVector3& vT1, float fScale);

    void Set(const PblVector3& vP0, const PblVector3& vP1, const PblVector3& vT0, const PblVector3& vT1,
             float fScale);

    // FUNCTION: SWBF2 0x00424b60
    float GetLengthApproximation(int iSteps) const
    {
        float fLength = 0.0f;
        for (int i = 0; i < iSteps; i++)
        {
            PblVector3 v0 = getValue((float)i / (float)iSteps);
            PblVector3 v1 = getValue((float)(i + 1) / (float)iSteps);
            fLength += Length(v1 - v0);
        }
        return fLength;
    }

    PblVector3 getValue(float fT) const
    {
        return ((_a * fT + _b) * fT + _c) * fT + _d;
    }
    PblVector3 getSlope(float fT) const
    {
        return (fT * _a * 3.0f + _b * 2.0f) * fT + _c;
    }

    PblVector3 _a;
    PblVector3 _b;
    PblVector3 _c;
    PblVector3 _d;
    float _fApproximateLength;
};
 
