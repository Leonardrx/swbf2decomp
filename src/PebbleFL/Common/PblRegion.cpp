#include <d3dx9math.h>
#include "PblRegion.h"

#include <math.h>
#include <stddef.h>

// FUNCTION: SWBF2 0x00729a40
bool PblRegion::IsPointInside(const PblVector3& rPoint) const
{
    switch (mType)
    {
    case type_sphere:
    {
        const float fR2 = mSize.w * mSize.w;
        const float fDx = mMatrix.m[3][0] - rPoint.x;
        const float fDy = mMatrix.m[3][1] - rPoint.y;
        const float fDz = mMatrix.m[3][2] - rPoint.z;
        return fDy * fDy + fDx * fDx + fDz * fDz < fR2;
    }

    case type_box:
    {
        const PblVector3 vDiff = rPoint - mMatrix[3];
        return mSize.x > (float)fabs(DotProduct(vDiff, mMatrix[0]))
            && mSize.y > (float)fabs(DotProduct(vDiff, mMatrix[1]))
            && mSize.z > (float)fabs(vDiff.x * mMatrix.m[2][0] + vDiff.y * mMatrix.m[2][1] + vDiff.z * mMatrix.m[2][2]);
    }

    case type_cylinder:
    {
        const PblVector3 vDiff = rPoint - mMatrix[3];

        const float fAxial = (float)fabs(DotProduct(mMatrix[1], vDiff));
        if (fAxial >= mSize.y)
        {
            return false;
        }

        const float fRadial0 = DotProduct(mMatrix[0], vDiff);
        const float fRadial2 = DotProduct(mMatrix[2], vDiff);
        return mSize.x * mSize.x > fRadial2 * fRadial2 + fRadial0 * fRadial0;
    }

    case type_plane:
        return DotProduct(mMatrix[2], rPoint) < mSize.w;

    case type_box_aligned:
        return mSize.x > (float)fabs(rPoint.x - mMatrix.m[3][0])
            && mSize.y > (float)fabs(rPoint.y - mMatrix.m[3][1])
            && mSize.z > (float)fabs(rPoint.z - mMatrix.m[3][2]);

    case type_cylinder_xaligned:
    {
        const float fDx = rPoint.x - mMatrix.m[3][0];
        const float fDy = rPoint.y - mMatrix.m[3][1];
        const float fDz = rPoint.z - mMatrix.m[3][2];

        return mSize.y > (float)fabs(fDx)
            && mSize.x * mSize.x > fDz * fDz + fDy * fDy;
    }

    case type_cylinder_yaligned:
    {
        const PblVector3 vDiff = rPoint - mMatrix[3];
        return mSize.y > (float)fabs(vDiff.y)
            && mSize.x * mSize.x > vDiff.z * vDiff.z + vDiff.x * vDiff.x;
    }

    case type_cylinder_zaligned:
    {
        const PblVector3 vDiff = rPoint - mMatrix[3];
        return mSize.y > (float)fabs(vDiff.z)
            && mSize.x * mSize.x > vDiff.y * vDiff.y + vDiff.x * vDiff.x;
    }

    default:
        return false;
    }
}

// SYNTHETIC: SWBF2 0x0040aef0
// `dynamic initializer for 'PblRegion::sList''
// SYNTHETIC: SWBF2 0x0076a0d0
// `dynamic atexit destructor for 'PblRegion::sList''

// GLOBAL: SWBF2 0x007e2c9c
// PblRegion::sList
PblListSingle PblRegion::sList;

// FUNCTION: SWBF2 0x00727f00
PblRegion::PblRegion(const PblRegion& rOther)
{
    sList.AddLast(&mNode);
    mMatrix = rOther.mMatrix;
    mSize = rOther.mSize;
    mId = rOther.mId;
    mType = rOther.mType;
}

// FUNCTION: SWBF2 0x00727f70
PblRegion::PblRegion()
    : mId(0), mType(type_invalid)
{
    sList.AddLast(&mNode);
    mMatrix.Reset();
    mSize.x = mSize.y = mSize.z = mSize.w = 0.0f;
}

// FUNCTION: SWBF2 0x00727fd0
PblRegion::~PblRegion()
{
    mNode.Remove();
}


// FUNCTION: SWBF2 0x00728ef0
bool PblRegion::DoesSphereIntersect(const PblVector3& rCenter, float fRadius) const
{
    PblVector3 v;
    switch (mType)
    {
    case type_sphere:
    {
        const float fR = mSize.w + fRadius;
        const PblVector3 vDiff(rCenter.x - mMatrix.m[3][0], rCenter.y - mMatrix.m[3][1], rCenter.z - mMatrix.m[3][2]);
        return DotProduct(vDiff, vDiff) < fR * fR;
    }

    case type_box:
    {
        const PblVector3 vLocal = mMatrix.GetInverseFast().ApplyTransform(rCenter);
        return DoesSphereIntersectBox(mSize, vLocal, fRadius);
    }

    case type_cylinder:
    {
        const PblVector3 vLocal = mMatrix.GetInverseFast().ApplyTransform(rCenter);
        return DoesSphereIntersectYCylinder(mSize, vLocal, fRadius);
    }

    case type_plane:
        return DotProduct(rCenter, mMatrix[2]) < mSize.w + fRadius;

    case type_box_aligned:
        v.x = rCenter.x - mMatrix.m[3][0];
        v.y = rCenter.y - mMatrix.m[3][1];
        v.z = rCenter.z - mMatrix.m[3][2];
        return DoesSphereIntersectBox(mSize, v, fRadius);

    case type_cylinder_xaligned:
        v.x = rCenter.z - mMatrix.m[3][2];
        v.y = rCenter.x - mMatrix.m[3][0];
        v.z = rCenter.y - mMatrix.m[3][1];
        return DoesSphereIntersectYCylinder(mSize, v, fRadius);

    case type_cylinder_yaligned:
        v.x = rCenter.x - mMatrix.m[3][0];
        v.y = rCenter.y - mMatrix.m[3][1];
        v.z = rCenter.z - mMatrix.m[3][2];
        return DoesSphereIntersectYCylinder(mSize, v, fRadius);

    case type_cylinder_zaligned:
        v.x = rCenter.y - mMatrix.m[3][1];
        v.y = rCenter.z - mMatrix.m[3][2];
        v.z = rCenter.x - mMatrix.m[3][0];
        return DoesSphereIntersectYCylinder(mSize, v, fRadius);
    }
    return false;
}

// FUNCTION: SWBF2 0x00729130
bool PblRegion::DoesSphereIntersectBox(const PblVector3& vSize, const PblVector3& vPoint, float fRadius)
{
    const float fAx = (float)fabs(vPoint.x);
    const float fAy = (float)fabs(vPoint.y);
    const float fAz = (float)fabs(vPoint.z);
    const float fDz = fAz > vSize.z ? fAz - vSize.z : 0.0f;
    const float fDy = fAy > vSize.y ? fAy - vSize.y : 0.0f;
    const float fDx = fAx > vSize.x ? fAx - vSize.x : 0.0f;
    const float fDistSq = fDx * fDx + fDy * fDy + fDz * fDz;
    return fDistSq < fRadius * fRadius;
}

// FUNCTION: SWBF2 0x007291d0
bool PblRegion::DoesSphereIntersectYCylinder(const PblVector3& vSize, const PblVector3& vPoint, float fRadius)
{
    const float fAxial = (float)fabs(vPoint.y);
    const float fAxialMax = vSize.y + fRadius;
    if (fAxial >= fAxialMax)
    {
        return false;
    }
    const float fRadial2 = vPoint.x * vPoint.x + vPoint.z * vPoint.z;
    const float fOuter = (vSize.x + fRadius) * (vSize.x + fRadius);
    if (fRadial2 >= fOuter)
    {
        return false;
    }
    if (vSize.y > fAxial)
    {
        return fOuter > fRadial2;
    }
    if (vSize.x * vSize.x >= fRadial2)
    {
        return fAxialMax > fAxial;
    }
    const float fDy = fAxial - vSize.y;
    const float fDr = (float)sqrt((double)fRadial2) - vSize.x;
    return fDr * fDr + fDy * fDy < fRadius * fRadius;
}

// FUNCTION: SWBF2 0x00728010
bool PblRegion::DoesAABoxIntersect(const PblVector3& vCenter, const PblVector3& vExtents) const
{
    switch (mType)
    {
    case type_sphere:
    {
        const PblVector3 vDiff = vCenter - mMatrix[3];
        const float fAx = (float)fabs(vDiff.x);
        const float fAy = (float)fabs(vDiff.y);
        const float fAz = (float)fabs(vDiff.z);
        const float fDx = fAx > vExtents.x ? fAx - vExtents.x : 0.0f;
        const float fDy = fAy > vExtents.y ? fAy - vExtents.y : 0.0f;
        const float fDz = fAz > vExtents.z ? fAz - vExtents.z : 0.0f;
        return mSize.w * mSize.w > fDy * fDy + fDx * fDx + fDz * fDz;
    }

    case type_box:
    {
        const PblVector3 vAbs0((float)fabs(mMatrix.m[0][0]), (float)fabs(mMatrix.m[0][1]), (float)fabs(mMatrix.m[0][2]));
        const PblVector3 vAbs1((float)fabs(mMatrix.m[1][0]), (float)fabs(mMatrix.m[1][1]), (float)fabs(mMatrix.m[1][2]));
        const PblVector3 vAbs2((float)fabs(mMatrix.m[2][0]), (float)fabs(mMatrix.m[2][1]), (float)fabs(mMatrix.m[2][2]));
        const PblVector3 vDiff = vCenter - mMatrix[3];
        if ((float)fabs(vDiff.x) >= mSize.y * vAbs1.x + vAbs0.x * mSize.x + vAbs2.x * mSize.z)
        {
            return false;
        }
        if ((float)fabs(vDiff.y) >= mSize.y * vAbs1.y + vAbs0.y * mSize.x + vAbs2.y * mSize.z)
        {
            return false;
        }
        if ((float)fabs(vDiff.z) >= mSize.y * vAbs1.z + vAbs0.z * mSize.x + vAbs2.z * mSize.z)
        {
            return false;
        }
        const PblVector3 v = mMatrix.GetInverseFast().ApplyTransform(vCenter);
        if ((float)fabs(v.x) >= DotProduct(vExtents, vAbs0))
        {
            return false;
        }
        if ((float)fabs(v.y) >= DotProduct(vExtents, vAbs1))
        {
            return false;
        }
        if ((float)fabs(v.z) >= DotProduct(vExtents, vAbs2))
        {
            return false;
        }
        return true;
    }

    case type_plane:
    {
        const float fDist = DotProduct(vCenter, mMatrix[2]);
        if (mSize.w > fDist)
        {
            return true;
        }
        return mSize.w > fDist - ((float)fabs(mMatrix.m[2][1]) * vExtents.y
                                  + (float)fabs(mMatrix.m[2][0]) * vExtents.x
                                  + (float)fabs(mMatrix.m[2][2]) * vExtents.z);
    }

    case type_box_aligned:
        return mSize.x + vExtents.x > (float)fabs(vCenter.x - mMatrix.m[3][0])
            && mSize.y + vExtents.y > (float)fabs(vCenter.y - mMatrix.m[3][1])
            && mSize.z + vExtents.z > (float)fabs(vCenter.z - mMatrix.m[3][2]);

    case type_cylinder_xaligned:
    {
        if ((float)fabs(vCenter.x - mMatrix.m[3][0]) >= mSize.y + vExtents.y)
        {
            return false;
        }
        const float fA = (float)fabs(vCenter.z - mMatrix.m[3][2]);
        if (fA >= vExtents.x + mSize.x)
        {
            return false;
        }
        const float fB = (float)fabs(vCenter.y - mMatrix.m[3][1]);
        if (fB >= vExtents.z + mSize.x)
        {
            return false;
        }
        if (vExtents.x > fA && vExtents.z > fB)
        {
            return true;
        }
        const float fDa = fA - vExtents.x;
        const float fDb = fB - vExtents.z;
        return mSize.x * mSize.x > fDb * fDb + fDa * fDa;
    }

    case type_cylinder_yaligned:
    {
        if ((float)fabs(vCenter.y - mMatrix.m[3][1]) >= mSize.y + vExtents.y)
        {
            return false;
        }
        const float fA = (float)fabs(vCenter.x - mMatrix.m[3][0]);
        if (fA >= vExtents.x + mSize.x)
        {
            return false;
        }
        const float fB = (float)fabs(vCenter.z - mMatrix.m[3][2]);
        if (fB >= vExtents.z + mSize.x)
        {
            return false;
        }
        if (vExtents.x > fA && vExtents.z > fB)
        {
            return true;
        }
        const float fDa = fA - vExtents.x;
        const float fDb = fB - vExtents.z;
        return mSize.x * mSize.x > fDb * fDb + fDa * fDa;
    }

    case type_cylinder_zaligned:
    {
        if ((float)fabs(vCenter.z - mMatrix.m[3][2]) >= mSize.y + vExtents.y)
        {
            return false;
        }
        const float fA = (float)fabs(vCenter.y - mMatrix.m[3][1]);
        if (fA >= vExtents.x + mSize.x)
        {
            return false;
        }
        const float fB = (float)fabs(vCenter.x - mMatrix.m[3][0]);
        if (fB >= vExtents.z + mSize.x)
        {
            return false;
        }
        if (vExtents.x > fA && vExtents.z > fB)
        {
            return true;
        }
        const float fDa = fA - vExtents.x;
        const float fDb = fB - vExtents.z;
        return mSize.x * mSize.x > fDb * fDb + fDa * fDa;
    }
    }

    return false;
}

// FUNCTION: SWBF2 0x007286b0
bool PblRegion::DoesRayIntersect(const PblVector3& rStart, const PblVector3& rEnd) const
{
    switch (mType)
    {
    case type_sphere:
    {
        const PblVector3 vDir = rEnd - rStart;
        const PblVector3 vToCenter = mMatrix[3] - rStart;
        const float fR2 = mSize.w * mSize.w;
        const float fLen2 = SquaredLength(vDir);
        const float fT = DotProduct(vToCenter, vDir) / fLen2;
        const PblVector3 vOffset = vToCenter - vDir * fT;
        const float fDist2 = SquaredLength(vOffset);
        if (fDist2 > fR2)
        {
            return false;
        }
        if (0.0f > fT)
        {
            if (fR2 - fDist2 > fT * fT * fLen2)
            {
                return false;
            }
        }
        else if (fT > 1.0f)
        {
            if (fR2 - fDist2 > (1.0f - fT) * (1.0f - fT) * fLen2)
            {
                return false;
            }
        }
        return true;
    }

    case type_box:
    case type_box_aligned:
    {
        const float fHx = (rEnd.x - rStart.x) * 0.5f;
        const float fHy = (rEnd.y - rStart.y) * 0.5f;
        const float fHz = (rEnd.z - rStart.z) * 0.5f;
        const float fCx = (rStart.x + rEnd.x) * 0.5f - mMatrix.m[3][0];
        const float fCy = (rStart.y + rEnd.y) * 0.5f - mMatrix.m[3][1];
        const float fCz = (rStart.z + rEnd.z) * 0.5f - mMatrix.m[3][2];
        const float fE0 = (float)fabs(mMatrix.m[0][0] * fHx + mMatrix.m[0][1] * fHy + mMatrix.m[0][2] * fHz);
        if ((float)fabs(mMatrix.m[0][0] * fCx + mMatrix.m[0][1] * fCy + mMatrix.m[0][2] * fCz) > mSize.x + fE0)
        {
            return false;
        }
        const float fE1 = (float)fabs(mMatrix.m[1][0] * fHx + mMatrix.m[1][1] * fHy + mMatrix.m[1][2] * fHz);
        if ((float)fabs(mMatrix.m[1][0] * fCx + mMatrix.m[1][1] * fCy + mMatrix.m[1][2] * fCz) > mSize.y + fE1)
        {
            return false;
        }
        const float fE2 = (float)fabs(mMatrix.m[2][0] * fHx + mMatrix.m[2][1] * fHy + mMatrix.m[2][2] * fHz);
        if ((float)fabs(mMatrix.m[2][0] * fCx + mMatrix.m[2][1] * fCy + mMatrix.m[2][2] * fCz) > mSize.z + fE2)
        {
            return false;
        }

        const float fWx = fCz * fHy - fCy * fHz;
        const float fWy = fCx * fHz - fHx * fCz;
        const float fWz = fHx * fCy - fCx * fHy;
        if ((float)fabs(mMatrix.m[0][0] * fWx + mMatrix.m[0][1] * fWy + mMatrix.m[0][2] * fWz) >
            mSize.z * fE1 + mSize.y * fE2)
        {
            return false;
        }
        if ((float)fabs(mMatrix.m[1][0] * fWx + mMatrix.m[1][1] * fWy + mMatrix.m[1][2] * fWz) >
            mSize.z * fE0 + mSize.x * fE2)
        {
            return false;
        }
        if ((float)fabs(mMatrix.m[2][0] * fWx + mMatrix.m[2][1] * fWy + mMatrix.m[2][2] * fWz) >
            mSize.y * fE0 + mSize.x * fE1)
        {
            return false;
        }
        return true;
    }

    case type_cylinder:
    case type_cylinder_xaligned:
    case type_cylinder_yaligned:
    case type_cylinder_zaligned:
    {
        const float fStartY = mMatrix.m[0][1] * rStart.x + mMatrix.m[1][1] * rStart.y + mMatrix.m[2][1] * rStart.z
                            + mMatrix.m[3][1];
        const float fEndY = mMatrix.m[0][1] * rEnd.x + mMatrix.m[1][1] * rEnd.y + mMatrix.m[2][1] * rEnd.z
                          + mMatrix.m[3][1];
        const float fHalf = mSize.y;
        if (fStartY >= fHalf && fEndY >= fHalf)
        {
            return false;
        }
        const float fNegHalf = -fHalf;
        if (fNegHalf >= fStartY && fNegHalf >= fEndY)
        {
            return false;
        }
        const float fR2 = mSize.x * mSize.x;
        float fStartX = mMatrix.m[0][0] * rStart.x + mMatrix.m[1][0] * rStart.y + mMatrix.m[2][0] * rStart.z
                      + mMatrix.m[3][0];
        float fStartZ = mMatrix.m[0][2] * rStart.x + mMatrix.m[1][2] * rStart.y + mMatrix.m[2][2] * rStart.z
                      + mMatrix.m[3][2];
        float fEndX = mMatrix.m[0][0] * rEnd.x + mMatrix.m[1][0] * rEnd.y + mMatrix.m[2][0] * rEnd.z
                    + mMatrix.m[3][0];
        float fEndZ = mMatrix.m[0][2] * rEnd.x + mMatrix.m[1][2] * rEnd.y + mMatrix.m[2][2] * rEnd.z
                    + mMatrix.m[3][2];
        const float fDz = fEndZ - fStartZ;
        const float fDx = fEndX - fStartX;
        const float fDy = fEndY - fStartY;

        if (fStartY > fHalf)
        {
            const float fInv = 1.0f / fDy;
            const float fT = (fHalf - fStartY) * fInv;
            fStartZ = fT * fDz + fStartZ;
            fStartX = fT * fDx + fStartX;
            if (fNegHalf > fEndY)
            {
                const float fT2 = (fHalf + fEndY) * fInv;
                fEndZ = fEndZ - fT2 * fDz;
                fEndX = fEndX - fT2 * fDx;
            }
        }
        else if (fEndY > fHalf)
        {
            const float fInv = 1.0f / fDy;
            const float fT = (fEndY - fHalf) * fInv;
            fEndZ = fEndZ - fT * fDz;
            fEndX = fEndX - fT * fDx;
            if (fNegHalf > fStartY)
            {
                const float fT2 = (fNegHalf - fStartY) * fInv;
                fStartZ = fT2 * fDz + fStartZ;
                fStartX = fT2 * fDx + fStartX;
            }
        }
        if (fR2 > fStartZ * fStartZ + fStartX * fStartX)
        {
            return true;
        }
        if (fR2 > fEndZ * fEndZ + fEndX * fEndX)
        {
            return true;
        }
        const float fCross = fEndZ * fStartX - fEndX * fStartZ;
        const float fLen2 = fDz * fDz + fDx * fDx;
        if (fLen2 * fR2 > fCross * fCross)
        {
            return false;
        }
        return (fCross * fDz > fLen2 * fEndX) != (fCross * fDz > fLen2 * fStartX);
    }

    case type_plane:
        return mSize.w > DotProduct(rStart, mMatrix[2]) && DotProduct(mMatrix[2], rEnd) > mSize.w;
    }
    return false;
}

// FUNCTION: SWBF2 0x007292b0
PblRegion* PblRegion::Find(unsigned int uId)
{
    for (PblListSingle::Itor it(sList); it.GetNode() != sList.GetTerminator(); ++it)
    {
        PblRegion* pRegion = (PblRegion*)it.GetNode()->GetObjectA(&sList, offsetof(PblRegion, mNode));
        if (uId == pRegion->mId)
        {
            return pRegion;
        }
    }
    return 0;
}


// FUNCTION: SWBF2 0x007292e0
void PblRegion::Initialize(const Description& desc)
{
    mMatrix = desc.mMatrix;
    mSize = desc.mSize;
    mId = desc.mNameId;
    mType = desc.mType;

    switch (mType)
    {
    case type_box:
    {
        const float afLen[3] = { Length(mMatrix[0]), Length(mMatrix[1]), Length(mMatrix[2]) };

        unsigned int auMask[3] = { 7, 7, 7 };
        int i;
        for (i = 0; i < 3; ++i)
        {
            if ((float)fabs(mMatrix.m[i][0]) > 0.02f)
            {
                auMask[i] &= 1;
            }
            if ((float)fabs(mMatrix.m[i][1]) > 0.02f)
            {
                auMask[i] &= 2;
            }
            if ((float)fabs(mMatrix.m[i][2]) > 0.02f)
            {
                auMask[i] &= 4;
            }
            if (!auMask[i])
            {
                break;
            }
        }

        if (i == 3 && !(auMask[0] & auMask[1] & auMask[2]))
        {
            mType = type_box_aligned;
            // i is 3 here (every axis was checked)
            for (int j = 0; j < i; ++j)
            {
                if (auMask[j] & 1)
                {
                    mSize.x = afLen[j] * (&desc.mSize.x)[j];
                }
                if (auMask[j] & 2)
                {
                    mSize.y = afLen[j] * (&desc.mSize.x)[j];
                }
                if (auMask[j] & 4)
                {
                    mSize.z = afLen[j] * (&desc.mSize.x)[j];
                }
            }
            mMatrix[0] = gVectorUnitX;
            mMatrix.m[0][3] = 0.0f;
            mMatrix[1] = gVectorUnitY;
            mMatrix.m[1][3] = 0.0f;
            mMatrix[2] = gVectorUnitZ;
            mMatrix.m[2][3] = 0.0f;
        }
        else
        {
            mMatrix[0] = (1.0f / afLen[0]) * mMatrix[0];
            mMatrix.m[0][3] = 0.0f;
            mMatrix[1] = (1.0f / afLen[1]) * mMatrix[1];
            mMatrix.m[1][3] = 0.0f;
            mMatrix[2] = (1.0f / afLen[2]) * mMatrix[2];
            mMatrix.m[2][3] = 0.0f;
            mSize.x *= afLen[0];
            mSize.y *= afLen[1];
            mSize.z *= afLen[2];
        }
        break;
    }

    case type_cylinder:
    {
        const float fLenX = Length(mMatrix[0]);
        const float fLenY = Length(mMatrix[1]);
        const float fLenZ = Length(mMatrix[2]);

        unsigned int uMask = 7;
        if ((float)fabs(mMatrix.m[1][0]) > 0.02f)
        {
            uMask &= 1;
        }
        if ((float)fabs(mMatrix.m[1][1]) > 0.02f)
        {
            uMask &= 2;
        }
        if ((float)fabs(mMatrix.m[1][2]) > 0.02f)
        {
            uMask &= 4;
        }

        mSize.y *= fLenY;
        const float fX = mSize.x * fLenX;
        const float fZ = mSize.z * fLenZ;
        const float fX2 = fX * fX;
        const float fRadius = (float)sqrt((double)(fX2 + fZ * fZ));
        mSize.x = fRadius;
        mSize.z = fRadius;

        if (!uMask)
        {
            mMatrix[0] = mMatrix[0] / fLenX;
            mMatrix.m[0][3] = 0.0f;
            mMatrix[1] = mMatrix[1] * (1.0f / fLenY);
            mMatrix.m[1][3] = 0.0f;
            mMatrix[2] = mMatrix[2] / fLenZ;
            mMatrix.m[2][3] = 0.0f;
        }
        else
        {
            switch (uMask)
            {
            case 1:
                mType = type_cylinder_xaligned;
                mMatrix[0] = gVectorUnitY;
                mMatrix.m[0][3] = 0.0f;
                mMatrix[1] = gVectorUnitZ;
                mMatrix.m[1][3] = 0.0f;
                mMatrix[2] = gVectorUnitX;
                mMatrix.m[2][3] = 0.0f;
                break;

            case 4:
                mType = type_cylinder_zaligned;
                mMatrix[0] = gVectorUnitZ;
                mMatrix.m[0][3] = 0.0f;
                mMatrix[1] = gVectorUnitX;
                mMatrix.m[1][3] = 0.0f;
                mMatrix[2] = gVectorUnitY;
                mMatrix.m[2][3] = 0.0f;
                break;

            default:
                mType = type_cylinder_yaligned;
                mMatrix[0] = gVectorUnitX;
                mMatrix.m[0][3] = 0.0f;
                mMatrix[1] = gVectorUnitY;
                mMatrix.m[1][3] = 0.0f;
                mMatrix[2] = gVectorUnitZ;
                mMatrix.m[2][3] = 0.0f;
                break;
            }
        }
        break;
    }

    case type_plane:
    {
        PblVector3& vNormal = mMatrix[2];
        vNormal = Normalize(vNormal);
        mMatrix.m[2][3] = 0.0f;
        mSize.w = mMatrix.m[3][0] * vNormal.x + mMatrix.m[3][1] * vNormal.y + mMatrix.m[3][2] * vNormal.z;
        break;
    }
    }
}

// FUNCTION: SWBF2 0x00729e10
void PblRegion::Reset()
{
    sList.Clear();
}
