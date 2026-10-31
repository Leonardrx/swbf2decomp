#include <math.h>

#include "Restrictor.h"

// FUNCTION: SWBF2 0x00630230
void Restrictor::RestrictMovement(PblVector3& vMovement)
{
    for (int i = 0; i < mNumRestrictions; ++i)
    {
        float fNx = mRestrictions[i].x;
        float fNy = mRestrictions[i].y;
        float fNz = mRestrictions[i].z;
        float fX = vMovement.x;
        float fY = vMovement.y;
        float fZ = vMovement.z;
        float fDot = fNx * fX + fNy * fY + fNz * fZ;
        if (fDot < 0.0f)
        {
            vMovement.x = fX - fNx * fDot;
            vMovement.y = fY - fNy * fDot;
            vMovement.z = fZ - fNz * fDot;
        }
    }
}

// FUNCTION: SWBF2 0x006301f0
PblVector3 Restrictor::GetRestrictedMovement(const PblVector3& vDir, float fDist)
{
    return PblVector3(vDir.x * fDist, vDir.y * fDist, vDir.z * fDist);
}

// FUNCTION: SWBF2 0x006302d0
PblVector3 Restrictor::RestrictMovement(const PblVector3& vDir, float& fDist)
{
    if (mNumRestrictions == 0 || fDist > 5.0f)
    {
        return vDir;
    }

    PblVector3 vResult = vDir;
    for (int i = 0; i < mNumRestrictions; ++i)
    {
        const PblVector3& vNormal = mRestrictions[i];
        float fDot = vNormal.x * vResult.x + vNormal.y * vResult.y + vNormal.z * vResult.z;
        if (fDot < 0.0f)
        {
            vResult.x = vResult.x - vNormal.x * fDot;
            vResult.y = vResult.y - vNormal.y * fDot;
            vResult.z = vResult.z - vNormal.z * fDot;
            float fLength = sqrtf(vResult.x * vResult.x + vResult.y * vResult.y + vResult.z * vResult.z);
            if (fLength > 0.0f)
            {
                float fInv = 1.0f / fLength;
                vResult.x = fInv * vResult.x;
                vResult.y = fInv * vResult.y;
                vResult.z = fInv * vResult.z;
                fDist = fDist / fLength;
            }
        }
    }
    return vResult;
}
