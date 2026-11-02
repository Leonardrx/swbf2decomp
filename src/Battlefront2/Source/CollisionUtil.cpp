#include <math.h>
#include <d3dx9math.h>

#include "CollisionUtil.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblMatrix.h"

// FUNCTION: SWBF2 0x00469060
bool CollisionUtil::CollideSphereAABB(const PblVector3& rCenter, float fRadius,
                                      const PblVector3& rMin, const PblVector3& rMax)
{
    float fDist2 = 0.0f;
    if (rCenter.x < rMin.x)
    {
        fDist2 += (rCenter.x - rMin.x) * (rCenter.x - rMin.x);
    }
    else if (rCenter.x > rMax.x)
    {
        fDist2 += (rCenter.x - rMax.x) * (rCenter.x - rMax.x);
    }
    if (rCenter.y < rMin.y)
    {
        fDist2 += (rCenter.y - rMin.y) * (rCenter.y - rMin.y);
    }
    else if (rCenter.y > rMax.y)
    {
        fDist2 += (rCenter.y - rMax.y) * (rCenter.y - rMax.y);
    }
    if (rCenter.z < rMin.z)
    {
        fDist2 += (rCenter.z - rMin.z) * (rCenter.z - rMin.z);
    }
    else if (rCenter.z > rMax.z)
    {
        fDist2 += (rCenter.z - rMax.z) * (rCenter.z - rMax.z);
    }
    return fRadius * fRadius >= fDist2;
}

// FUNCTION: SWBF2 0x004690f0
bool CollisionUtil::CollideSphereRay(const PblVector3& rCenter, float fRadius,
                                     const PblVector3& rFrom, const PblVector3& rDir, float fLength)
{
    PblVector3 vDelta = rCenter - rFrom;
    PblVector3 vDir = rDir;
    float fProj = DotProduct(vDelta, vDir);
    float fDist2 = DotProduct(vDelta, vDelta);
    float fRadius2 = fRadius * fRadius;

    if (fRadius2 > fDist2)
    {
        return true;
    }
    if (fProj < 0.0f && fDist2 > fRadius2)
    {
        return false;
    }
    float fPerp2 = fDist2 - fProj * fProj;
    if (fPerp2 > fRadius2)
    {
        return false;
    }
    float fHalfChord = (float)sqrt((double)(fRadius2 - fPerp2));
    float fHit = fDist2 > fRadius2 ? fProj - fHalfChord : fProj + fHalfChord;
    if (fHit < 0.0f)
    {
        return false;
    }
    if (fHit > fLength)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0046a810
bool CollisionUtil::CollideSpheres2(const PblVector3& rCenter1, float fRadius1,
                                    const PblVector3& rCenter2, float fRadius2)
{
    PblVector3 vDelta = rCenter2 - rCenter1;
    float fRadii = fRadius1 + fRadius2;
    float fDist2 = SquaredLength(vDelta);
    if (fDist2 >= fRadii * fRadii)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0046ae60
int FindAACenteredBoxRayIntersection(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                                     float fWidth, float fHeight, float fDepth, float& fHit)
{
    bool bInside = true;
    PblVector3 vExtent(fWidth, fHeight, fDepth);
    float aPlane[3];
    int aSide[3];
    int i;
    for (i = 0; i < 3; i++)
    {
        if (-vExtent[i] > vStart[i])
        {
            aSide[i] = -1;
            aPlane[i] = -vExtent[i];
            bInside = false;
        }
        else if (vStart[i] > vExtent[i])
        {
            aSide[i] = 1;
            aPlane[i] = vExtent[i];
            bInside = false;
        }
        else
        {
            aSide[i] = 0;
            aPlane[i] = vExtent[i];
        }
    }
    if (bInside)
    {
        return 0;
    }

    PblVector3 vRay = vDir * fLength;
    float aT[3];
    for (i = 0; i < 3; i++)
    {
        if (aSide[i] != 0 && fabsf(vRay[i]) > 0.0001f)
        {
            aT[i] = (aPlane[i] - vStart[i]) / vRay[i];
        }
        else
        {
            aT[i] = -1.0f;
        }
    }

    for (i = 0; i < 3; i++)
    {
        if (aT[i] > 0.0f)
        {
            PblVector3 vHit = vStart + vRay * aT[i];
            for (int j = 0; j < 3; j++)
            {
                if (j != i && (-vExtent[j] > vHit[j] || vHit[j] > vExtent[j]))
                {
                    aT[i] = -1.0f;
                    break;
                }
            }
        }
    }

    int nPlane = 0;
    if (aT[1] > 0.0f && (aT[nPlane] > aT[1] || aT[nPlane] < 0.0f))
    {
        nPlane = 1;
    }
    if (aT[2] > 0.0f && (aT[nPlane] > aT[2] || aT[nPlane] < 0.0f))
    {
        nPlane = 2;
    }
    if (aT[nPlane] < 0.0f)
    {
        return 0;
    }
    fHit = aT[nPlane];
    return aSide[nPlane] * (nPlane + 1);
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x00469670
bool IsPointInsideAxisAlignedCylinder(const PblVector3& vOffset, float fRadius, float fHalfHeight)
{
    float fY = vOffset.y;
    float fAbsY = fY < 0.0f ? -fY : fY;
    if (fAbsY > fHalfHeight)
    {
        return false;
    }
    return SquaredLengthXZ(vOffset) < fRadius * fRadius;
}
}

static inline PblVector3 ToLocal(const PblMatrix& m, const PblVector3& v)
{
    return PblVector3(m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
                      m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
                      m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z);
}

static inline PblVector3 ToWorld(const PblMatrix& m, float x, float y, float z)
{
    return PblVector3(m.m[0][0] * x + m.m[1][0] * y + m.m[2][0] * z,
                      m.m[0][1] * x + m.m[1][1] * y + m.m[2][1] * z,
                      m.m[0][2] * x + m.m[1][2] * y + m.m[2][2] * z);
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046a700
bool CollideSpheres(const PblVector3& vCenter, float fRadius, const PblVector3& vSphereCenter, float fSphereRadius,
                PblVector3& vNormal, float& fDepth)
{
    float fDx = vSphereCenter.x - vCenter.x;
    float fDy = vSphereCenter.y - vCenter.y;
    float fDz = vSphereCenter.z - vCenter.z;
    PblVector3 vDelta(fDx, fDy, fDz);
    float fRadii = fRadius + fSphereRadius;
    float fDist = SquaredLength(vDelta);
    if (fDist >= fRadii * fRadii)
    {
        return false;
    }
    if (fDist > 0.0f)
    {
        fDist = sqrtf(fDist);
    }
    fDepth = fRadii - fDist;
    if (fDist > 0.0f)
    {
        vNormal = vDelta / fDist;
    }
    else
    {
        vNormal.x = 0.0f;
        vNormal.y = 0.0f;
        vNormal.z = 1.0f;
    }
    return true;
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x00469e90
bool CollideBoxSphere(const PblMatrix& matrix, float fWidth, float fDepth, float fHeight, const PblVector3& vSphereCenter,
                float fSphereRadius, PblVector3& vNormal, float& fPenetration)
{
    PblVector3 vAxis[3] = {matrix.GetXAxis(), matrix.GetYAxis(), matrix.GetZAxis()};
    PblVector3 vOffset = vSphereCenter - matrix.GetTranslation();
    PblVector3 vLocal(DotProduct(vAxis[0], vOffset), DotProduct(vAxis[1], vOffset), DotProduct(vAxis[2], vOffset));
    bool bInside = true;
    int iSideX, iSideY, iSideZ;
    PblVector3 vClamp;

    if (-fWidth > vLocal.x)
    {
        iSideX = -1;
        vClamp.x = -fWidth;
        bInside = false;
    }
    else if (vLocal.x > fWidth)
    {
        iSideX = 1;
        vClamp.x = fWidth;
        bInside = false;
    }
    else
    {
        iSideX = 0;
    }
    if (-fHeight > vLocal.y)
    {
        iSideY = -1;
        vClamp.y = -fHeight;
        bInside = false;
    }
    else if (vLocal.y > fHeight)
    {
        iSideY = 1;
        vClamp.y = fHeight;
        bInside = false;
    }
    else
    {
        iSideY = 0;
    }
    if (-fDepth > vLocal.z)
    {
        iSideZ = -1;
        vClamp.z = -fDepth;
        bInside = false;
    }
    else if (vLocal.z > fDepth)
    {
        iSideZ = 1;
        vClamp.z = fDepth;
        bInside = false;
    }
    else
    {
        iSideZ = 0;
    }
    if (!bInside)
    {
        float fDZ = 0.0f;
        float fDY = 0.0f;
        float fDX = 0.0f;
        if (iSideX)
        {
            fDX = (float)fabs((double)(vClamp.x - vLocal.x));
        }
        if (iSideY)
        {
            fDY = (float)fabs((double)(vClamp.y - vLocal.y));
        }
        if (iSideZ)
        {
            fDZ = (float)fabs((double)(vClamp.z - vLocal.z));
        }
        float fDist2 = fDX * fDX + fDY * fDY + fDZ * fDZ;
        if (fSphereRadius * fSphereRadius > fDist2)
        {
            float fDist = (float)sqrt((double)fDist2);
            fPenetration = fSphereRadius - fDist;
            vNormal = vAxis[0] * ((float)iSideX * fDX) + vAxis[1] * ((float)iSideY * fDY) + vAxis[2] * ((float)iSideZ * fDZ);
            float fInv = 1.0f / fDist;
            vNormal = PblVector3(vNormal.x * fInv, vNormal.y * fInv, vNormal.z * fInv);
            return true;
        }
    }
    return false;
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x00469e20
bool CollideSphereBox(const PblVector3& vCenter, float fRadius, const PblMatrix& matrix, float fWidth, float fDepth,
                float fHeight, PblVector3& vNormal, float& fPenetration)
{
    bool bCollided = CollisionUtil::CollideBoxSphere(matrix, fWidth, fDepth, fHeight, vCenter, fRadius, vNormal, fPenetration);
    vNormal = PblVector3(vNormal.x * -1.0f, vNormal.y * -1.0f, vNormal.z * -1.0f);
    return bCollided;
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046a280
bool CollideCylinderSphere(const PblMatrix& matrix, float fRadius, float fHalfHeight, const PblVector3& vSphereCenter,
                float fSphereRadius, PblVector3& vNormal, float& fDepth)
{
    PblVector3 vXAxis = matrix.GetXAxis();
    PblVector3 vYAxis = matrix.GetYAxis();
    PblVector3 vZAxis = matrix.GetZAxis();
    PblVector3 vOffset(vSphereCenter.x - matrix.m[3][0], vSphereCenter.y - matrix.m[3][1],
                       vSphereCenter.z - matrix.m[3][2]);
    PblVector3 vLocal(vXAxis.x * vOffset.x + vXAxis.y * vOffset.y + vXAxis.z * vOffset.z,
                      vYAxis.x * vOffset.x + vYAxis.y * vOffset.y + vYAxis.z * vOffset.z,
                      vZAxis.x * vOffset.x + vZAxis.y * vOffset.y + vZAxis.z * vOffset.z);
    float fDist2 = vLocal.x * vLocal.x + vLocal.z * vLocal.z;

    if (!(vLocal.y > fHalfHeight || -fHalfHeight > vLocal.y))
    {

        float fDist = (float)sqrt((double)fDist2);
        fDepth = (fRadius + fSphereRadius) - fDist;
        if (!(fDepth > 0.0f))
        {
            return false;
        }
        if (fDist > 0.0001f)
        {
            float fInv = 1.0f / fDist;
            float fWorldX = fInv * vLocal.x;
            float fWorldY = fInv * 0.0f;
            float fWorldZ = vLocal.z * fInv;
            vNormal = PblVector3(vXAxis.x * fWorldX + vYAxis.x * fWorldY + vZAxis.x * fWorldZ,
                                 vXAxis.y * fWorldX + vYAxis.y * fWorldY + vZAxis.y * fWorldZ,
                                 vXAxis.z * fWorldX + vYAxis.z * fWorldY + vZAxis.z * fWorldZ);
        }
        else
        {
            fDepth = fSphereRadius > fRadius ? fSphereRadius : fRadius;
            vNormal = matrix.GetZAxis();
        }
        return true;
    }

    if (fRadius * fRadius > fDist2)
    {

        fDepth = fSphereRadius - ((float)fabs((double)vLocal.y) - fHalfHeight);
        if (!(fDepth > 0.0f))
        {
            return false;
        }
        if (0.0f > vLocal.y)
        {
            vNormal = PblVector3(matrix.m[1][0] * -1.0f, matrix.m[1][1] * -1.0f, matrix.m[1][2] * -1.0f);
        }
        else
        {
            vNormal = matrix.GetYAxis();
        }
        return true;
    }

    float fScale = fRadius / (float)sqrt((double)fDist2);
    PblVector3 vEdge(fScale * vLocal.x, fScale * 0.0f, vLocal.z * fScale);
    float fCapY = 0.0f > vLocal.y ? -fHalfHeight : fHalfHeight;
    float fX = vLocal.x - vEdge.x;
    float fZ = vLocal.z - vEdge.z;
    float fY = vLocal.y - fCapY;
    float fEdgeDist2 = fX * fX + fY * fY + fZ * fZ;
    if (!(fSphereRadius * fSphereRadius > fEdgeDist2))
    {
        return false;
    }
    float fEdgeDist = (float)sqrt((double)fEdgeDist2);
    fDepth = fSphereRadius - fEdgeDist;
    float fInv = 1.0f / fEdgeDist;
    float fWorldX = fX * fInv;
    float fWorldY = fY * fInv;
    float fWorldZ = fZ * fInv;
    vNormal = PblVector3(vXAxis.x * fWorldX + vYAxis.x * fWorldY + vZAxis.x * fWorldZ,
                         vXAxis.y * fWorldX + vYAxis.y * fWorldY + vZAxis.y * fWorldZ,
                         vXAxis.z * fWorldX + vYAxis.z * fWorldY + vZAxis.z * fWorldZ);
    return true;
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x004696b0
bool GetClosestPointInPolygon(const PblVector3& vPos, int iNumCorners, const PblVector3* pCorners, PblVector3& vOutPos,
                float fMaxDist)
{
    PblVector3 c0 = pCorners[0];
    const PblVector3& c1 = pCorners[1];
    const PblVector3& c2 = pCorners[2];
    float e0x = c1.x - c0.x;
    float e0y = c1.y - c0.y;
    float e0z = c1.z - c0.z;
    float e1x = c2.x - c1.x;
    float e1y = c2.y - c1.y;
    float e1z = c2.z - c1.z;
    float nx = e0y * e1z - e0z * e1y;
    float ny = e0z * e1x - e0x * e1z;
    float nz = e0x * e1y - e0y * e1x;
    float fNormal2 = nx * nx + ny * ny + nz * nz;
    if (fNormal2 < 0.0001f)
    {
        return false;
    }

    float fDist = nx * (vPos.x - c0.x) + ny * (vPos.y - c0.y) + nz * (vPos.z - c0.z);
    if (fDist < 0.0f)
    {
        return false;
    }
    float fMaxDist2 = fMaxDist * fMaxDist;
    if (fDist * fDist > fNormal2 * fMaxDist2)
    {
        return false;
    }

    float fT = fDist / fNormal2;
    float px = vPos.x - nx * fT;
    float py = vPos.y - ny * fT;
    float pz = vPos.z - nz * fT;

    float fFirstStart = e0x * (px - c0.x) + e0y * (py - c0.y) + e0z * (pz - c0.z);
    float fEnd = -(e0x * (px - c1.x) + e0y * (py - c1.y) + e0z * (pz - c1.z));
    if (fFirstStart >= 0.0f && fEnd >= 0.0f &&
        (ny * e0z - nz * e0y) * (px - c0.x) + (nz * e0x - nx * e0z) * (py - c0.y) +
                (nx * e0y - ny * e0x) * (pz - c0.z) < 0.0f)
    {
        float fS = fFirstStart / (e0x * e0x + e0y * e0y + e0z * e0z);
        float qx = e0x * fS + c0.x;
        float qy = e0y * fS + c0.y;
        float qz = e0z * fS + c0.z;
        if ((vPos.x - qx) * (vPos.x - qx) + (vPos.y - qy) * (vPos.y - qy) + (vPos.z - qz) * (vPos.z - qz) <=
            fMaxDist2)
        {
            vOutPos = PblVector3(qx, qy, qz);
            return true;
        }
        return false;
    }

    for (int i = 1; i < iNumCorners; ++i)
    {
        const PblVector3& a = pCorners[i];
        const PblVector3& b = pCorners[i + 1 == iNumCorners ? 0 : i + 1];
        float ex = b.x - a.x;
        float ey = b.y - a.y;
        float ez = b.z - a.z;
        float fStart = ex * (px - a.x) + ey * (py - a.y) + ez * (pz - a.z);
        float fNextEnd = -(ex * (px - b.x) + ey * (py - b.y) + ez * (pz - b.z));
        if (fStart >= 0.0f && fNextEnd >= 0.0f &&
            (ny * ez - nz * ey) * (px - a.x) + (nz * ex - nx * ez) * (py - a.y) + (nx * ey - ny * ex) * (pz - a.z) <
                0.0f)
        {
            float fS = fStart / (ex * ex + ey * ey + ez * ez);
            float qx = ex * fS + a.x;
            float qy = ey * fS + a.y;
            float qz = ez * fS + a.z;
            if ((vPos.x - qx) * (vPos.x - qx) + (vPos.y - qy) * (vPos.y - qy) + (vPos.z - qz) * (vPos.z - qz) >
                fMaxDist2)
            {
                return false;
            }
            vOutPos = PblVector3(qx, qy, qz);
            return true;
        }
        if (fEnd <= 0.0f && fStart <= 0.0f)
        {
            if ((vPos.x - a.x) * (vPos.x - a.x) + (vPos.y - a.y) * (vPos.y - a.y) + (vPos.z - a.z) * (vPos.z - a.z) >
                fMaxDist2)
            {
                return false;
            }
            vOutPos = a;
            return true;
        }
        fEnd = fNextEnd;
    }

    if (fEnd > 0.0f || fFirstStart > 0.0f)
    {
        vOutPos = PblVector3(px, py, pz);
        return true;
    }
    if ((vPos.x - c0.x) * (vPos.x - c0.x) + (vPos.y - c0.y) * (vPos.y - c0.y) + (vPos.z - c0.z) * (vPos.z - c0.z) >
        fMaxDist2)
    {
        return false;
    }
    vOutPos = c0;
    return true;
}
}

static inline float Fabs(float f)
{
    return (float)fabs((double)f);
}

static inline void GetAxis(const PblMatrix& m, int i, PblVector3& v)
{
    switch (i)
    {
    case 0:
        v = PblVector3(m.m[0][0], m.m[0][1], m.m[0][2]);
        break;
    case 1:
        v = PblVector3(m.m[1][0], m.m[1][1], m.m[1][2]);
        break;
    case 2:
        v = PblVector3(m.m[2][0], m.m[2][1], m.m[2][2]);
        break;
    }
}

// FUNCTION: SWBF2 0x00468810
bool CollisionUtil::CollideBoxes(const PblVector3& vVelocityA, const PblMatrix& mA, float fWidthA, float fHeightA,
                                 float fDepthA, const PblVector3& vVelocityB, const PblMatrix& mB, float fWidthB,
                                 float fHeightB, float fDepthB, PblVector3& vNormal, float& fDepth)
{
    fDepth = fWidthA + fHeightA + fDepthA + fWidthB + fHeightB + fDepthB;

    PblMatrix mInvA = mA.GetInverseFast();
    PblMatrix mRel;
    mRel = mB * mInvA;

    PblVector3 vRelVel(vVelocityB.x - vVelocityA.x, vVelocityB.y - vVelocityA.y, vVelocityB.z - vVelocityA.z);
    PblVector3 vRelDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vRelDir, (const D3DXVECTOR3*)&vRelVel);

    const float (*R)[4] = mRel.m;
    PblVector3 vAxis[3] = { PblVector3(R[0][0], R[0][1], R[0][2]), PblVector3(R[1][0], R[1][1], R[1][2]),
                            PblVector3(R[2][0], R[2][1], R[2][2]) };
    PblVector3 vOffset(R[3][0], R[3][1], R[3][2]);
    float fExtentA[3] = { fWidthA, fHeightA, fDepthA };
    float fExtentB[3] = { fWidthB, fHeightB, fDepthB };
    int i;

    for (i = 0; i < 3; ++i)
    {
        float fT = (&vOffset.x)[i];
        float fSep = Fabs((&vAxis[0].x)[i]) * fWidthB + Fabs((&vAxis[1].x)[i]) * fHeightB +
                     Fabs((&vAxis[2].x)[i]) * fDepthB + fExtentA[i] - Fabs(fT);
        if (0.0f > fSep)
        {
            return false;
        }
        if (fDepth > fSep)
        {
            fDepth = fSep;
            GetAxis(mA, i, vNormal);
            if (0.0f > fT)
            {
                vNormal = PblVector3(-vNormal.x, -vNormal.y, -vNormal.z);
            }
        }
    }

    PblVector3 v;
    for (i = 0; i < 3; ++i)
    {
        GetAxis(mRel, i, v);
        float fProj = R[3][0] * v.x + R[3][1] * v.y + R[3][2] * v.z;
        float fSep = Fabs(v.x) * fWidthA + Fabs(v.y) * fHeightA + Fabs(v.z) * fDepthA + fExtentB[i] - Fabs(fProj);
        if (0.0f > fSep)
        {
            return false;
        }
        if (fDepth > fSep)
        {
            fDepth = fSep;
            GetAxis(mB, i, vNormal);
            if (0.0f > fProj)
            {
                vNormal = PblVector3(-vNormal.x, -vNormal.y, -vNormal.z);
            }
        }
    }

    const float* T = R[3];

    if (0.0f > (Fabs(R[0][1]) * fDepthA + Fabs(R[0][2]) * fHeightA) +
                   (Fabs(R[1][0]) * fDepthB + Fabs(R[2][0]) * fHeightB) - Fabs(R[0][1] * T[2] - R[0][2] * T[1]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[1][1]) * fDepthA + Fabs(R[1][2]) * fHeightA) +
                   (Fabs(R[2][0]) * fWidthB + Fabs(R[0][0]) * fDepthB) - Fabs(R[1][1] * T[2] - R[1][2] * T[1]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[2][1]) * fDepthA + Fabs(R[2][2]) * fHeightA) +
                   (Fabs(R[1][0]) * fWidthB + Fabs(R[0][0]) * fHeightB) - Fabs(R[2][1] * T[2] - R[2][2] * T[1]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[0][2]) * fWidthA + Fabs(R[0][0]) * fDepthA) +
                   (Fabs(R[2][1]) * fHeightB + Fabs(R[1][1]) * fDepthB) - Fabs(R[0][2] * T[0] - R[0][0] * T[2]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[1][0]) * fDepthA + Fabs(R[1][2]) * fWidthA) +
                   (Fabs(R[0][1]) * fDepthB + Fabs(R[2][1]) * fWidthB) - Fabs(R[1][2] * T[0] - R[1][0] * T[2]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[2][0]) * fDepthA + Fabs(R[2][2]) * fWidthA) +
                   (Fabs(R[0][1]) * fHeightB + Fabs(R[1][1]) * fWidthB) - Fabs(R[2][2] * T[0] - R[2][0] * T[2]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[0][1]) * fWidthA + Fabs(R[0][0]) * fHeightA) +
                   (Fabs(R[2][2]) * fHeightB + Fabs(R[1][2]) * fDepthB) - Fabs(R[0][0] * T[1] - R[0][1] * T[0]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[1][0]) * fHeightA + Fabs(R[1][1]) * fWidthA) +
                   (Fabs(R[0][2]) * fDepthB + Fabs(R[2][2]) * fWidthB) - Fabs(R[1][0] * T[1] - R[1][1] * T[0]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[2][0]) * fHeightA + Fabs(R[2][1]) * fWidthA) +
                   (Fabs(R[0][2]) * fHeightB + Fabs(R[1][2]) * fWidthB) - Fabs(R[2][0] * T[1] - R[2][1] * T[0]))
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x00468740
bool CollisionUtil::CollideCylinderBox(const PblVector3& vVelocityA, const PblMatrix& mA, float fRadius, float fHeight,
                                       const PblVector3& vVelocityB, const PblMatrix& mB, float fWidth, float fDepthB,
                                       float fHeightB, PblVector3& vNormal, float& fDepth)
{
    return CollideBoxes(vVelocityA, mA, fRadius, fHeight, fRadius, vVelocityB, mB, fWidth, fHeightB, fDepthB, vNormal,
                        fDepth);
}

// FUNCTION: SWBF2 0x00468790
bool CollisionUtil::CollideBoxCylinder(const PblVector3& vVelocityA, const PblMatrix& mA, float fWidth, float fDepthA,
                                       float fHeightA, const PblVector3& vVelocityB, const PblMatrix& mB, float fRadius,
                                       float fHeight, PblVector3& vNormal, float& fDepth)
{
    return CollideBoxes(vVelocityA, mA, fWidth, fHeightA, fDepthA, vVelocityB, mB, fRadius, fHeight, fRadius, vNormal,
                        fDepth);
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x004687e0
bool CollideCylinders(const PblMatrix& m1, float fRadius1, float fHalfHeight1, const PblMatrix& m2, float fRadius2,
                float fHalfHeight2, PblVector3& vNormal, float& fDepth)
{
    return CollisionUtil::CollideCapsules(m1, fRadius1, fHalfHeight1, m2, fRadius2, fHalfHeight2, vNormal, fDepth);
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x00469200
bool CollideCapsules(const PblMatrix& m1, float fRadius1, float fHalfHeight1, const PblMatrix& m2, float fRadius2,
                float fHalfHeight2, PblVector3& vNormal, float& fDepth)
{
    PblVector3 vStart1, vEnd1, vStart2, vEnd2;
    vStart1 = m1.GetTranslation() - fHalfHeight1 * m1.GetYAxis();
    vEnd1 = m1.GetTranslation() + fHalfHeight1 * m1.GetYAxis();
    vStart2 = m2.GetTranslation() - fHalfHeight2 * m2.GetYAxis();
    vEnd2 = m2.GetTranslation() + fHalfHeight2 * m2.GetYAxis();
    PblVector3 vPoint1, vPoint2, vDelta;
    CollisionUtil::GetClosestPoints_SegmentSegment(vStart1, vEnd1, vStart2, vEnd2, vPoint1, vPoint2);

    vDelta = vPoint2 - vPoint1;
    float fDist2 = SquaredLength(vDelta);
    if (0.0001f > fDist2)
    {
        vNormal.x = 1.0f;
        vNormal.y = 0.0f;
        vNormal.z = 0.0f;
        if (0.0001f > 1.0f - (vNormal.x * m1.m[1][0] + vNormal.y * m1.m[1][1] + vNormal.z * m1.m[1][2]))
        {
            vNormal.x = 0.0f;
            vNormal.y = 0.0f;
            vNormal.z = 1.0f;
        }
        fDepth = fRadius1 > fRadius2 ? fRadius1 : fRadius2;
        return true;
    }
    float fDist = (float)sqrt((double)fDist2);
    fDepth = fRadius1 + fRadius2 - fDist;
    if (fDepth > 0.0001f)
    {
        float fInv = 1.0f / fDist;
        vNormal = vDelta * fInv;
        return true;
    }
    return false;
}
}

// FUNCTION: SWBF2 0x004694d0
bool CollisionUtil::CollideAxisAlignedCylinders(PblVector3& vDelta, float fRadiusA, float fRadiusB, float fHeightA,
                                                float fHeightB, float& fDepth)
{
    float fAbsY = fabsf(vDelta.y);
    float fDistXZ = PblMath::Sqrt(vDelta.x * vDelta.x + vDelta.z * vDelta.z);
    float fRadii = fRadiusA + fRadiusB;
    if (fDistXZ >= fRadii)
    {
        return false;
    }
    float fHalfHeight = (fHeightA + fHeightB) * 0.5f;
    if (fAbsY >= fHalfHeight)
    {
        return false;
    }

    float fHorizontalOverlap = fRadii - fDistXZ;
    float fVerticalOverlap = fHalfHeight - fAbsY;
    if (fHorizontalOverlap < fVerticalOverlap)
    {
        vDelta.y = 0.0f;
        fDepth = fDistXZ;
        if (fDistXZ != 0.0f)
        {
            float fInv = 1.0f / fDistXZ;
            vDelta.x = vDelta.x * fInv;
            vDelta.y = fInv * vDelta.y;
            vDelta.z = vDelta.z * fInv;
        }
        else
        {
            vDelta.z = -1.0f;
        }
        fDepth = fRadii - fDepth;
    }
    else
    {
        vDelta.x = 0.0f;
        vDelta.z = 0.0f;
        fDepth = fAbsY;
        if (fAbsY != 0.0f)
        {
            float fInv = 1.0f / fAbsY;
            vDelta.x = vDelta.x * fInv;
            vDelta.y = fInv * vDelta.y;
            vDelta.z = vDelta.z * fInv;
        }
        else
        {
            vDelta.y = 1.0f;
        }
        fDepth = fHalfHeight - fDepth;
    }
    return true;
}

// FUNCTION: SWBF2 0x0046a210
bool CollisionUtil::CollideSphereCylinder(const PblVector3& vCenter, float fSphereRadius, const PblMatrix& matrix,
                                          float fRadius, float fHalfHeight, PblVector3& vNormal, float& fDepth)
{
    bool bHit = CollisionUtil::CollideCylinderSphere(matrix, fRadius, fHalfHeight, vCenter, fSphereRadius, vNormal, fDepth);
    vNormal = PblVector3(vNormal.x * -1.0f, vNormal.y * -1.0f, vNormal.z * -1.0f);
    return bHit;
}

// FUNCTION: SWBF2 0x0046a850
float CollisionUtil::RayHitSphere(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                                  const PblVector3& vCenter, float fRadius, PblVector3* pNormal)
{
    if (fRadius != 0.0f)
    {
        PblVector3 vToCenter = vCenter - vStart;
        float fProj = DotProduct(vDir, vToCenter);
        if (-fRadius > fProj)
        {
            return 1.0f;
        }
        if (fProj > fLength + fRadius)
        {
            return 1.0f;
        }

        PblVector3 vNear = vStart + vDir * fProj;
        PblVector3 vNearToCenter = vNear - vCenter;
        float fDist2 = vNearToCenter.x * vNearToCenter.x + vNearToCenter.y * vNearToCenter.y +
                       vNearToCenter.z * vNearToCenter.z;
        float fRadius2 = fRadius * fRadius;
        if (fDist2 > fRadius2)
        {
            return 1.0f;
        }

        float fHalfChord2 = fRadius2 - fDist2;
        if (fHalfChord2 > fProj * fProj)
        {
            if (pNormal)
            {
                float fLen2 = vToCenter.x * vToCenter.x + vToCenter.y * vToCenter.y + vToCenter.z * vToCenter.z;
                if (fLen2 > 0.0001f)
                {
                    float fLen = (float)sqrt((double)fLen2);
                    if (fLen > 0.0001f)
                    {
                        float fInv = 1.0f / fLen;
                        *pNormal = PblVector3(fInv * vToCenter.x, fInv * vToCenter.y, fInv * vToCenter.z);
                        return 0.0f;
                    }
                }
                *pNormal = vDir;
            }
            return 0.0f;
        }

        if (0.0f > fProj)
        {
            return 1.0f;
        }
        float fHalfChord = (float)sqrt((double)fHalfChord2);
        float fHit = fProj - fHalfChord;
        if (fHit > fLength)
        {
            return 1.0f;
        }
        if (pNormal)
        {
            *pNormal = (vNear - vDir * fHalfChord - vCenter) * (1.0f / fRadius);
        }
        return fHit / fLength;
    }
    return 1.0f;
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046ab20
float RayHitBubble(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 const PblVector3& vSize, PblVector3* pNormal)
{
    PblVector3 vSizeInv(1.0f / vSize.x, 1.0f / vSize.y, 1.0f / vSize.z);
    PblVector3 vStartPosRel = vStart - matrix.GetTranslation();
    PblVector3 vStartPosLocal(DotProduct(matrix.GetXAxis(), vStartPosRel) * vSizeInv.x,
                             DotProduct(vStartPosRel, matrix.GetYAxis()) * vSizeInv.y,
                             DotProduct(matrix.GetZAxis(), vStartPosRel) * vSizeInv.z);
    float fDistSqr = SquaredLength(vStartPosLocal);
    if (fDistSqr < 1.0f)
    {
        return 1.0f;
    }

    PblVector3 vRayLocal(DotProduct(matrix.GetXAxis(), vDir) * (vSizeInv.x * fLength),
                        DotProduct(matrix.GetYAxis(), vDir) * (vSizeInv.y * fLength),
                        DotProduct(matrix.GetZAxis(), vDir) * (vSizeInv.z * fLength));
    float fRayDotR = DotProduct(vRayLocal, vStartPosLocal);
    if (fRayDotR >= 0.0f)
    {
        return 1.0f;
    }

    float fQ = fRayDotR * fRayDotR - (fDistSqr - 1.0f) * SquaredLength(vRayLocal);
    if (fQ <= 0.0f)
    {
        return 1.0f;
    }
    float fRaySqr = SquaredLength(vRayLocal);
    float fEnd = fRaySqr + fRayDotR;
    if (fEnd < 0.0f && fEnd * fEnd > fQ)
    {
        return 1.0f;
    }

    float fT = (-fRayDotR - sqrtf(fQ)) / fRaySqr;
    if (pNormal)
    {
        PblVector3 vNormal = (vStartPosLocal + vRayLocal * fT) * vSizeInv;
        *pNormal = Normalize(matrix.ApplyRotation(vNormal));
    }
    return fT;
}
}


static inline PblVector3 InverseRotate(const PblMatrix& m, float x, float y, float z)
{
    return PblVector3(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
                      m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
                      m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046b130
float RayHitBox(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 float fWidth, float fDepth, float fHeight, PblVector3* pNormal)
{
    const PblVector3& vPos = matrix.GetTranslation();
    PblVector3 vLocalStart = InverseRotate(matrix, vStart.x - vPos.x, vStart.y - vPos.y, vStart.z - vPos.z);
    PblVector3 vLocalDir = InverseRotate(matrix, vDir.x, vDir.y, vDir.z);

    float fHit = 0.0f;
    int face = FindAACenteredBoxRayIntersection(vLocalStart, vLocalDir, fLength, fWidth, fHeight, fDepth, fHit);
    if (face == 0)
    {
        return 1.0f;
    }
    if (pNormal)
    {
        switch (face)
        {
        case 1: *pNormal = matrix.GetXAxis(); break;
        case 2: *pNormal = matrix.GetYAxis(); break;
        case 3: *pNormal = matrix.GetZAxis(); break;
        case -1:
            *pNormal = PblVector3(matrix.m[0][0] * -1.0f, matrix.m[0][1] * -1.0f, matrix.m[0][2] * -1.0f);
            break;
        case -2:
            *pNormal = PblVector3(matrix.m[1][0] * -1.0f, matrix.m[1][1] * -1.0f, matrix.m[1][2] * -1.0f);
            break;
        case -3:
            *pNormal = PblVector3(matrix.m[2][0] * -1.0f, matrix.m[2][1] * -1.0f, matrix.m[2][2] * -1.0f);
            break;
        default:
            pNormal->x = 0.0f;
            pNormal->y = 1.0f;
            pNormal->z = 0.0f;
            break;
        }
    }
    return fHit;
}
}

// FUNCTION: SWBF2 0x0046b3e0
float RayHitAACylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblVector3& vCenter,
                 float fRadius, float fHalfHeight, PblVector3* pNormal)
{
    PblVector3 vOffset(vStart.x - vCenter.x, vStart.y - vCenter.y, vStart.z - vCenter.z);
    if (CollisionUtil::IsPointInsideAxisAlignedCylinder(vOffset, fRadius, fHalfHeight))
    {
        if (pNormal)
        {
            if (vStart.y > vCenter.y)
            {
                pNormal->x = 0.0f;
                pNormal->y = 1.0f;
                pNormal->z = 0.0f;
            }
            else
            {
                pNormal->x = 0.0f;
                pNormal->y = -1.0f;
                pNormal->z = 0.0f;
            }
        }
        return 1.0f;
    }

    float fClipped = fLength;
    float fEndY = vDir.y * fLength + vStart.y;
    float fBottom = vCenter.y - fHalfHeight;
    float fTop = vCenter.y + fHalfHeight;
    if (fBottom > fEndY)
    {
        fClipped = (vStart.y - fBottom) / (vStart.y - fEndY) * fLength;
    }
    else if (fEndY > fTop)
    {
        fClipped = (vStart.y - fTop) / (vStart.y - fEndY) * fLength;
    }

    PblVector3 vFlat(vDir.x * fClipped, 0.0f, vDir.z * fClipped);
    float fFlatLen = Length(vFlat);
    if (fFlatLen < 0.0001f)
    {
        return 1.0f;
    }
    PblVector3 vFlatDir = vFlat * (1.0f / fFlatLen);

    float fProj = DotProduct(vCenter - vStart, vFlatDir);
    if (fProj < 0.0f || fProj > fFlatLen + fRadius)
    {
        return 1.0f;
    }
    float fDx = vStart.x + vFlatDir.x * fProj - vCenter.x;
    float fDz = vStart.z + vFlatDir.z * fProj - vCenter.z;
    float fDist2 = fDx * fDx + fDz * fDz;
    if (fDist2 > fRadius * fRadius)
    {
        return 1.0f;
    }
    float fEnter = fProj - sqrtf(fRadius * fRadius - fDist2);
    if (fEnter > fFlatLen)
    {
        return 1.0f;
    }

    float fHit = fEnter / fFlatLen * fClipped;
    if (pNormal)
    {
        *pNormal = vStart + vDir * fHit - vCenter;
        pNormal->y = 0.0f;
        PblVector3 vUnit;
        D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)pNormal);
        *pNormal = vUnit;
    }
    return fHit / fLength;
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046b710
float RayHitAxisAlignedCylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblVector3& vCenter,
                 float fRadius, float fHalfHeight, PblVector3* pNormal)
{
    float fEndY = vDir.y * fLength + vStart.y;
    float fTop = vCenter.y + fHalfHeight;
    if (vStart.y > fTop)
    {
        if (fTop > fEndY)
        {
            float t = (vStart.y - fTop) / (vStart.y - fEndY);
            float dx = t * fLength * vDir.x + vStart.x - vCenter.x;
            float dz = t * fLength * vDir.z + vStart.z - vCenter.z;
            if (fRadius * fRadius > dx * dx + dz * dz)
            {
                if (pNormal)
                {
                    pNormal->x = 0.0f;
                    pNormal->y = 1.0f;
                    pNormal->z = 0.0f;
                }
                return t;
            }

            t += 0.0001f;
            float fStep = t * fLength;
            PblVector3 vS(fStep * vDir.x + vStart.x, fStep * vDir.y + vStart.y, fStep * vDir.z + vStart.z);
            float fRest = 1.0f - t;
            float fHit = RayHitAACylinder(vS, vDir, fRest * fLength, vCenter, fRadius, fHalfHeight, pNormal);
            if (fHit < 1.0f)
            {
                return fRest * fHit + t;
            }
        }
        return 1.0f;
    }

    float fBottom = vCenter.y - fHalfHeight;
    if (fBottom > vStart.y)
    {
        if (fEndY > fBottom)
        {
            float t = (vStart.y - fBottom) / (vStart.y - fEndY);
            float dx = t * fLength * vDir.x + vStart.x - vCenter.x;
            float dz = t * fLength * vDir.z + vStart.z - vCenter.z;
            if (fRadius * fRadius > dx * dx + dz * dz)
            {
                if (pNormal)
                {
                    pNormal->x = 0.0f;
                    pNormal->y = -1.0f;
                    pNormal->z = 0.0f;
                }
                return t;
            }

            t += 0.0001f;
            float fStep = t * fLength;
            PblVector3 vS(fStep * vDir.x + vStart.x, fStep * vDir.y + vStart.y, fStep * vDir.z + vStart.z);
            float fRest = 1.0f - t;
            float fHit = RayHitAACylinder(vS, vDir, fRest * fLength, vCenter, fRadius, fHalfHeight, pNormal);
            if (fHit < 1.0f)
            {
                return fRest * fHit + t;
            }
        }
        return 1.0f;
    }
    return RayHitAACylinder(vStart, vDir, fLength, vCenter, fRadius, fHalfHeight, pNormal);
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046b9c0
float RayHitCylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 float fRadius, float fHalfHeight, PblVector3* pNormal)
{
    const PblVector3& vPos = matrix.GetTranslation();
    PblVector3 vLocalStart = InverseRotate(matrix, vStart.x - vPos.x, vStart.y - vPos.y, vStart.z - vPos.z);
    PblVector3 vLocalDir = InverseRotate(matrix, vDir.x, vDir.y, vDir.z);
    PblVector3 vCenter(0.0f, 0.0f, 0.0f);
    PblVector3 vLocalNormal;

    float fHit = CollisionUtil::RayHitAxisAlignedCylinder(vLocalStart, vLocalDir, fLength, vCenter, fRadius, fHalfHeight, &vLocalNormal);
    if (fHit < 1.0f && pNormal)
    {
        *pNormal = PblVector3(
            matrix.m[0][0] * vLocalNormal.x + matrix.m[1][0] * vLocalNormal.y + matrix.m[2][0] * vLocalNormal.z,
            matrix.m[0][1] * vLocalNormal.x + matrix.m[1][1] * vLocalNormal.y + matrix.m[2][1] * vLocalNormal.z,
            matrix.m[0][2] * vLocalNormal.x + matrix.m[1][2] * vLocalNormal.y + matrix.m[2][2] * vLocalNormal.z);
    }
    return fHit;
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046bf30
void GetClosestPoint_SegmentPoint(const PblVector3& vSegStart, const PblVector3& vSegEnd, const PblVector3& vPoint,
                PblVector3& vClosest)
{
    PblVector3 vDelta = vSegEnd - vSegStart;
    PblVector3 vOffset = vPoint - vSegStart;
    float fProj = DotProduct(vOffset, vDelta);
    if (fProj <= 0.0f)
    {
        vClosest = vSegStart;
        return;
    }
    float fLen2 = DotProduct(vDelta, vDelta);
    if (fProj >= fLen2)
    {
        vClosest = vSegEnd;
        return;
    }
    float fT = fProj / fLen2;
    vClosest = vSegStart + fT * vDelta;
}
}

namespace CollisionUtil
{

// FUNCTION: SWBF2 0x0046bc00
void GetClosestPoints_SegmentSegment(const PblVector3& vSStart, const PblVector3& vSEnd, const PblVector3& vTStart,
                const PblVector3& vTEnd, PblVector3& vSPoint, PblVector3& vTPoint)
{
    const float SMALL = 0.0001f;
    PblVector3 u = vSEnd - vSStart;
    PblVector3 v = vTEnd - vTStart;
    PblVector3 w = vSStart - vTStart;
    float a = DotProduct(u, u);
    float b = DotProduct(v, u);
    float c = DotProduct(v, v);
    float d = DotProduct(w, u);
    float e = DotProduct(w, v);
    float D = c * a - b * b;
    float sN, sD = D, tN, tD = D;

    if (D < SMALL)
    {
        sN = 0.0f;
        sD = 1.0f;
        tN = e;
        tD = c;
    }
    else
    {
        sN = e * b - d * c;
        tN = e * a - d * b;
        if (sN < 0.0f)
        {
            sN = 0.0f;
            tN = e;
            tD = c;
        }
        else if (sN > sD)
        {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0f)
    {
        tN = 0.0f;
        if (-d < 0.0f)
        {
            sN = 0.0f;
        }
        else if (-d > a)
        {
            sN = sD;
        }
        else
        {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD)
    {
        tN = tD;
        if (b - d < 0.0f)
        {
            sN = 0.0f;
        }
        else if (b - d > a)
        {
            sN = sD;
        }
        else
        {
            sN = b - d;
            sD = a;
        }
    }

    float sc = fabsf(sN) < SMALL ? 0.0f : sN / sD;
    float tc = fabsf(tN) < SMALL ? 0.0f : tN / tD;
    vSPoint = vSStart + u * sc;
    vTPoint = vTStart + v * tc;
}
}
