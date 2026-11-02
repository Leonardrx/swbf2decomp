#include <math.h>
#include <d3dx9math.h>

#include "MapCamera.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/RedCamera.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);



static PblVector3 HermiteSpline(float t, const PblVector3& p0, const PblVector3& m0,
                                const PblVector3& p1, const PblVector3& m1)
{
    float t2 = t * t;
    float t3 = t2 * t;
    float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h10 = t3 - 2.0f * t2 + t;
    float h01 = 3.0f * t2 - 2.0f * t3;
    float h11 = t3 - t2;
    return PblVector3(h00 * p0.x + h10 * m0.x + h01 * p1.x + h11 * m1.x,
                      h00 * p0.y + h10 * m0.y + h01 * p1.y + h11 * m1.y,
                      h00 * p0.z + h10 * m0.z + h01 * p1.z + h11 * m1.z);
}

static PblVector3 Scale(const PblVector3& v, float f) { return PblVector3(v.x * f, v.y * f, v.z * f); }

// GLOBAL: SWBF2 0x007ebba8
PblVector3 MapCamera::sOffset[NUM_ZOOM_LEVELS] = {
    PblVector3(0.0f, 400.0f, 100.0f),
    PblVector3(0.0f, 100.0f, 150.0f),
    PblVector3(0.0f, 10.0f, 25.0f),
};

// GLOBAL: SWBF2 0x01e579b0
float MapCamera::sPitch[NUM_ZOOM_LEVELS];

// FUNCTION: SWBF2 0x005a1c70
PblVector3 MapCamera::GetVelocity()
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x005a1c90
MapCamera::MapCamera(CameraManager* pManager, int iView)
    : GameCamera(pManager, iView),
      mMatrix(gMatrixIdentity), mTarget(gMatrixIdentity), mVelocity(gVectorZero),
      mMoveRatio(1.0f), mMoveCurPos(gVectorZero), mMoveOldPos(gVectorZero),
      mZoomDir(0), mZoomLevel(ZOOM_NORMAL), mZoomRatio(1.0f),
      mZoomOldDir(gVectorZero), mZoomOldPos(gVectorZero),
      mZoomNewDir(gVectorZero), mZoomNewPos(gVectorZero), mZoomDist(0.0f)
{
}

// SYNTHETIC: SWBF2 0x005a1e10
// MapCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005a1e40
void MapCamera::SetMatrix(const PblMatrix& rMatrix)
{
    mTarget = rMatrix;
    mMoveRatio = 0.0f;
    mMoveOldPos = mMoveCurPos;
}

// FUNCTION: SWBF2 0x005a1ea0
void MapCamera::SetZoomLevel(ZoomLevel eLevel)
{
    int iOldDir = mZoomDir;
    mZoomDir = eLevel - mZoomLevel;
    mZoomLevel = eLevel;
    if (iOldDir * mZoomDir < 0)
    {
        mZoomRatio = 1.0f - mZoomRatio;
        mZoomOldDir = mZoomNewDir;
        mZoomOldPos = mZoomNewPos;
    }
    else
    {
        mZoomRatio = 0.0f;
        mZoomOldDir = mMatrix[2];
        mZoomOldPos = mMatrix[3];
    }
}

// FUNCTION: SWBF2 0x005a1f60
void MapCamera::Update(float fDeltaTime)
{
    PblVector3 vOldPos = mMatrix[3];

    mMoveRatio += fDeltaTime * 2.0f;
    if (mMoveRatio < 1.0f)
    {
        float fT = 0.5f - PblMath::Cos(mMoveRatio * PblMath::fPi) * 0.5f;
        mMoveCurPos = Interpolate(mMoveOldPos, mTarget[3], fT);
    }
    else
    {
        mMoveCurPos = mTarget[3];
    }

    switch (mZoomLevel)
    {
    case ZOOM_OUT:
        mZoomNewDir = Normalize(sOffset[ZOOM_OUT]);
        mZoomNewPos = sOffset[mZoomLevel];
        break;
    case ZOOM_NORMAL:
        mZoomNewDir = Normalize(sOffset[ZOOM_NORMAL]);
        mZoomNewPos = sOffset[mZoomLevel] + mMoveCurPos;
        break;
    default:
        mZoomNewDir = mTarget[2];
        mZoomNewPos = mMoveCurPos;
        break;
    }

    PblVector3 vAxis = Normalize(CrossProduct(gVectorUnitY, mZoomNewDir));
    D3DXMATRIX mRotation;
    D3DXMatrixRotationAxis(&mRotation, (const D3DXVECTOR3*)&vAxis, sPitch[mZoomLevel]);
    PblVector3 vDir;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mZoomNewDir, &mRotation);
    mZoomNewDir = vDir;

    if (mZoomRatio == 0.0f)
    {
        mZoomDist = Length(mZoomNewPos - mZoomOldPos);
    }

    mZoomRatio += fDeltaTime * 0.5f;
    if (mZoomRatio < 1.0f)
    {
        float fT = 0.5f - PblMath::Cos(mZoomRatio * PblMath::fPi) * 0.5f;
        float fDir = (float)mZoomDir;
        PblVector3 vOldTangent = mZoomOldDir * fDir;
        PblVector3 vNewTangent = mZoomNewDir * fDir;
        float fTangent = -(mZoomDist * fDir);
        float t2 = fT * fT;
        float t3 = t2 * fT;
        PblVector3 vZoomDir = mZoomOldDir * (2.0f * t3 - 3.0f * t2 + 1.0f) + vOldTangent * (t3 - 2.0f * t2 + fT)
                            + mZoomNewDir * (3.0f * t2 - 2.0f * t3) + vNewTangent * (t3 - t2);
        PblVector3 vZoomPos = mZoomOldPos * (2.0f * t3 - 3.0f * t2 + 1.0f)
                            + mZoomOldDir * fTangent * (t3 - 2.0f * t2 + fT)
                            + mZoomNewPos * (3.0f * t2 - 2.0f * t3) + mZoomNewDir * fTangent * (t3 - t2);
        mMatrix = DirectionMatrix(vZoomDir, gVectorUnitY, vZoomPos);
    }
    else
    {
        mMatrix = DirectionMatrix(mZoomNewDir, gVectorUnitY, mZoomNewPos);
        mZoomDir = 0;
        mZoomRatio = 1.0f;
    }

    if (fDeltaTime > 0.0f)
    {
        mVelocity = (mMatrix[3] - vOldPos) * (1.0f / fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x005a2700
void MapCamera::SetupCamera(RedCamera* pCamera, float fDeltaTime)
{
    pCamera->SetMatrix(mMatrix);
    pCamera->SetZoom(1.0f);
}
