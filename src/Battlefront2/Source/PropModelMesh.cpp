#include "PropModelMesh.h"
#include "PropCluster.h"
#include "PropGenerator.h"
#include "CollisionBody.h"
#include "CollisionMesh.h"
#include "CollisionModel.h"
#include "NetGame.h"
#include "Entity.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblMath.h"

#include <d3dx9math.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "RedEngineFL/Application/RedTerrain.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;

namespace GameLoop
{
    // GLOBAL: SWBF2 0x01e56054
    extern float sDeltaTime;
}

namespace FLRenderer
{
float GetNearSceneFadeEnd();
}

extern bool flushClusterObjs_;

namespace PropGenerator
{
    void FlushClusterObjects();
}

// FUNCTION: SWBF2 0x0062b100
void PropGenerator::FlushClusterObjects()
{
    flushClusterObjs_ = true;
}

namespace PropGenerator
{
    void GetTerrainYAndNormal(float x, float z, float& y, PblVector3& vNormal);
}

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

// GLOBAL: SWBF2 0x007e903a
static bool bPropModelEnable = true;

// GLOBAL: SWBF2 0x007e9040
static float fDensity = 10.0f;

// GLOBAL: SWBF2 0x007e9044
static float fPropModelFadeAdj = 1.0f;

// GLOBAL: SWBF2 0x007e9048
static short sNumFreeObjects = -1;

// GLOBAL: SWBF2 0x01fa9270
static short sFreeObjectList[2000];

// SYNTHETIC: SWBF2 0x004060d0
// `dynamic initializer for 'sObjectCache''
// SYNTHETIC: SWBF2 0x007693d0
// `dynamic atexit destructor for 'sObjectCache''

// GLOBAL: SWBF2 0x01f03290
// sObjectCache
PropObjectMesh sObjectCache[2000];

// FUNCTION: SWBF2 0x0062c1b0
PblVector3 PropObjectMesh::GetPos()
{
    return mMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x0062c1d0
const PblMatrix& PropObjectMesh::GetMatrix()
{
    return mMatrix;
}

// SYNTHETIC: SWBF2 0x0062c1e0
// PropObjectMesh::~PropObjectMesh

// FUNCTION: SWBF2 0x0062c230
float PropModelMesh::GetDensity()
{
    return fDensity;
}

// FUNCTION: SWBF2 0x0062c240
void PropModelMesh::SetDensity(float fNewDensity)
{
    fDensity = fNewDensity;
}

bool PropModelMesh::IsEnabled()
{
    return bPropModelEnable;
}

// FUNCTION: SWBF2 0x0062c250
void PropModelMesh::SetEnable(bool bEnable)
{
    bPropModelEnable = bEnable;
}

float PropModelMesh::GetFadeAdj()
{
    return fPropModelFadeAdj;
}

// FUNCTION: SWBF2 0x0062c260
void PropModelMesh::SetFadeAdj(float fFadeAdj)
{
    fPropModelFadeAdj = fFadeAdj;
}

// FUNCTION: SWBF2 0x0062c270
PropCollisionObject::PropCollisionObject(PblMatrix& matrix, PropModelMesh* pModel)
    : CollisionObject(COLL_STATIC, pModel->mCollisionModel, 0, 0)
{
    mMatrix = matrix;

    {
        PblVector3 vCenter;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                               (const D3DXVECTOR3*)&pModel->mRenderObj.mGeometry->_BoundingSphere,
                               (const D3DXMATRIX*)&mMatrix);
        SetCollisionCenter(vCenter);
    }

    const float fRadius = pModel->mRenderObj.mGeometry->_BoundingSphere.GetRadius();
    SetCollisionRadius(fRadius);
    Set2DCollisionRadius(fRadius);

    mAABB = pModel->mRenderObj.mGeometry->_TotalBoundingBox.GetTransformedBox(mMatrix);
    AddStaticToCollisionManager();
}

void PropCollisionObject::RecalculateAABB(PblMatrix& matrix, const PblBox& box)
{
    mAABB = box.GetTransformedBox(matrix);
}

// FUNCTION: SWBF2 0x0062c3b0
const PblVector3& PropCollisionObject::GetPosition() const
{
    return mMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x0062c3c0
const PblMatrix& PropCollisionObject::GetMatrix() const
{
    return mMatrix;
}

// SYNTHETIC: SWBF2 0x0062c3d0
// PropCollisionObject::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0062c400
void PropObjectMesh::ClearCache()
{
    for (short i = 0; i < 2000; i++)
    {
        sFreeObjectList[i] = i;
        sObjectCache[i].mCacheId = i;
    }
    sNumFreeObjects = 2000;
}

// FUNCTION: SWBF2 0x0062c430
PropObjectMesh* PropObjectMesh::Create(PropModelMesh* pModel, float x, float z, float rotation)
{
    if (sNumFreeObjects <= 0)
    {
        if (sNumFreeObjects == -1)
        {
            ClearCache();
        }
        else
        {
            PropGenerator::FlushClusterObjects();
            if (sNumFreeObjects == 0)
            {
                return 0;
            }
        }
    }

    PropObjectMesh* pObject = &sObjectCache[sFreeObjectList[--sNumFreeObjects]];
    pObject->Init(pModel, x, z, rotation);
    return pObject;
}

// FUNCTION: SWBF2 0x0062c4a0
void PropObjectMesh::Release()
{
    sFreeObjectList[sNumFreeObjects++] = mCacheId;
    mSound.Stop(true);
}

// FUNCTION: SWBF2 0x0062c4d0
PropObjectMesh::PropObjectMesh()
{
}

// SYNTHETIC: SWBF2 0x0062c4f0
// PropObjectMesh::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0062c550
void PropObjectMesh::Init(PropModelMesh* pModel, float x, float z, float rotation)
{
    mModel = pModel;
    mRotation = rotation;
    mMatrix = gMatrixIdentity;
    mMatrix.SetTranslation(x, 0.0f, -z, 1.0f);
    mSetupFlag = 0;
    mSound.Stop(true);
    mElapsedTime = 0.0f;
    mNextTriggerTime = 0.0f;
}

// FUNCTION: SWBF2 0x0062c5e0
void PropObjectMesh::CalculateNextTriggerTime()
{
    mNextTriggerTime = PblRandom::_Global.GetFloat() * mModel->mSoundTriggerTimeDev +
                       mModel->mSoundTriggerTime;
}

// FUNCTION: SWBF2 0x0062c640
float GetPropDistanceScale(PropModelMesh::Type eType)
{
    if (eType == PropModelMesh::PMM_GRASS)
    {
        return sGrassDistanceScale;
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x0062c660
void PropObjectMesh::Render(const RedCamera* pCamera, float fDist, unsigned int uiFlags)
{
    if (!mModel)
    {
        return;
    }
    if (!bPropModelEnable)
    {
        return;
    }

    const float fScale = GetPropDistanceScale(mModel->mType);
    const float fMaxDist = GetDist() * fPropModelFadeAdj * fScale;
    const float fFade = fPropModelFadeAdj * 1.45f * fScale;

    if (fMaxDist > fDist)
    {
        if (!mSetupFlag)
        {
            Setup();
        }

        const PblMatrix& camMatrix = pCamera->GetMatrix();
        const float dx = mMatrix.m[3][0] - camMatrix.m[3][0];
        const float dz = mMatrix.m[3][2] - camMatrix.m[3][2];
        float fCamDist = PblMath::Sqrt(dx * dx + dz * dz);

        if (fMaxDist > fCamDist)
        {
            switch (mModel->mType)
            {
            case PropModelMesh::PMM_MESH:
            {
                RedModel* pGeometry = mModel->mRenderObj.mGeometry;
                if (!pGeometry)
                {
                    return;
                }
                if (mModel->mGeometryMaxDist > 0.0f &&
                    fCamDist > GetPropDistanceScale(mModel->mType) * (mModel->mGeometryMaxDist * fFade))
                {
                    return;
                }

                const PblSphere sphere = pGeometry->_BoundingSphere;
                PblSphere worldSphere = sphere;
                D3DXVec3TransformCoord((D3DXVECTOR3*)&worldSphere, (const D3DXVECTOR3*)&sphere,
                                       (const D3DXMATRIX*)&mMatrix);
                const RedCamera::VisibilityT eVis =
                    pCamera->ClassifySphere(worldSphere.GetCenter(), worldSphere.GetRadius(), -1.0f);
                if (eVis == RedCamera::VISIBILITY_NONE)
                {
                    return;
                }

                float fAlpha = 1.0f;
                const float fGeometryMaxDist = mModel->mGeometryMaxDist;
                if (fGeometryMaxDist > 0.0f &&
                    fCamDist > GetPropDistanceScale(mModel->mType) * fGeometryMaxDist)
                {
                    fAlpha = 1.0f - (fCamDist - fGeometryMaxDist) /
                                        (fGeometryMaxDist * fFade - fGeometryMaxDist);
                }
                else if (mModel->mGeometryMinDist > 0.0f)
                {
                    const float fMinDist = GetPropDistanceScale(mModel->mType) * mModel->mGeometryMinDist;
                    if (fMinDist > fCamDist)
                    {
                        float fRatio = fCamDist / fMinDist;
                        if (!(fRatio > 0.1f))
                        {
                            fRatio = 0.1f;
                        }
                        fAlpha = fRatio * fRatio;
                    }
                }
                mColor.a = (unsigned char)(int)(fAlpha * 255.0f);

                unsigned int uiRenderFlags = uiFlags;
                if (eVis == RedCamera::VISIBILITY_PARTIAL)
                {
                    uiRenderFlags |= 1;
                }
                else
                {
                    uiRenderFlags &= ~1u;
                }
                if (!mModel->m_bDynamicLighting)
                {
                    uiRenderFlags |= 0x80;
                }
                else
                {
                    uiRenderFlags &= ~0x80u;
                }

                mModel->mRenderObj.mGeometry->Render(&mMatrix, 0, mColor, uiRenderFlags | 0x1000000, 0);
                break;
            }

            case PropModelMesh::PMM_GRASS:
            {
                GrassPatchClass* pGrass = mModel->mRenderObj.mGrassClass;
                if (!pGrass)
                {
                    return;
                }

                PblVector3 vCenter;
                D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                                       (const D3DXVECTOR3*)&pGrass->mBoundingOffset,
                                       (const D3DXMATRIX*)&mMatrix);
                const RedCamera::VisibilityT eVis = pCamera->ClassifySphere(
                    vCenter, mModel->mRenderObj.mGrassClass->mBoundingRadius, -1.0f);
                if (eVis == RedCamera::VISIBILITY_NONE)
                {
                    return;
                }

                unsigned int uiRenderFlags = uiFlags;
                if (eVis == RedCamera::VISIBILITY_PARTIAL)
                {
                    uiRenderFlags |= 1;
                }
                else
                {
                    uiRenderFlags &= ~1u;
                }

                const RedColorValue color(mColor);
                PblMatrix matrix(reinterpret_cast<const _D3DXMATRIXA16*>(&mMatrix));

                mModel->mRenderObj.mGrassClass->RenderMaxDist(
                    matrix, GetPropDistanceScale(mModel->mType) * mModel->mGeometryMaxDist, color,
                    uiRenderFlags);
                break;
            }

            case PropModelMesh::PMM_LEAF:
            {
                LeafPatchClass* pLeaf = mModel->mRenderObj.mLeafClass;
                if (!pLeaf)
                {
                    return;
                }

                PblVector3 vCenter;
                D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                                       (const D3DXVECTOR3*)&pLeaf->mBoundingOffset,
                                       (const D3DXMATRIX*)&mMatrix);
                const RedCamera::VisibilityT eVis = pCamera->ClassifySphere(
                    vCenter, mModel->mRenderObj.mLeafClass->mBoundingRadius, -1.0f);
                if (eVis == RedCamera::VISIBILITY_NONE)
                {
                    return;
                }

                unsigned int uiRenderFlags = uiFlags;
                if (eVis == RedCamera::VISIBILITY_PARTIAL)
                {
                    uiRenderFlags |= 1;
                }
                else
                {
                    uiRenderFlags &= ~1u;
                }

                const RedColorValue color(mColor);
                PblMatrix matrix(reinterpret_cast<const _D3DXMATRIXA16*>(&mMatrix));

                mModel->mRenderObj.mLeafClass->RenderMaxDist(
                    matrix, fCamDist, uiRenderFlags,
                    GetPropDistanceScale(mModel->mType) * mModel->mGeometryMaxDist, color);
                break;
            }
            }
        }
    }

    if (mElapsedTime > mNextTriggerTime)
    {
        mSound.Stop(true);
        mSound.Play(&mModel->mSound, (const PblVector3*)&mMatrix.m[3][0], 0, true);
        CalculateNextTriggerTime();
        mElapsedTime = 0.0f;
    }
    else
    {
        mElapsedTime += GameLoop::sDeltaTime;
    }
}

// FUNCTION: SWBF2 0x0062ca60
void PropObjectMesh::Setup()
{
    mModel->SetupMatrix(mMatrix, mRotation);
    mSetupFlag = 1;

    const PropModelMesh* pModel = mModel;
    float fLight = RedTerrain::GetInterpolatedLight(mMatrix.m[3][0], mMatrix.m[3][2]);
    if (pModel->mType == PropModelMesh::PMM_MESH)
    {
        fLight += 0.5f;
    }
    float fBrightness = fLight * 0.7f + 0.6f;
    if (!(1.0f > fBrightness))
    {
        fBrightness = 1.0f;
    }
    fBrightness *= 255.0f;

    union
    {
        float f;
        unsigned int u;
    } seed;
    seed.f = mMatrix.m[3][2] * mMatrix.m[3][0];
    PblRandom rand(seed.u ^ 0xC71C2830u);
    rand.GetInt();

    const float fVariation = pModel->mColorVariation;
    const unsigned char b = (unsigned char)(int)((1.0f - rand.GetFloat() * fVariation) * fBrightness);
    const unsigned char g = (unsigned char)(int)((1.0f - rand.GetFloat() * fVariation) * fBrightness);
    const unsigned char r = (unsigned char)(int)((1.0f - rand.GetFloat() * fVariation) * fBrightness);
    mColor.a = 0xff;
    mColor.b = b;
    mColor.g = g;
    mColor.r = r;

    mSound.Stop(true);
    mElapsedTime = 0.0f;
    CalculateNextTriggerTime();
}

// FUNCTION: SWBF2 0x0062cc20
float PropObjectMesh::GetDist()
{
    return FLRenderer::GetNearSceneFadeEnd();
}

// FUNCTION: SWBF2 0x0062cc30
PropModelMesh::PropModelMesh()
{
    mRenderObj.mGeometry = 0;
    mCollisionModel = 0;
    mAIVisFactorCrouch = 1.0f;
    mAIVisFactorStand = 1.0f;
}

// SYNTHETIC: SWBF2 0x0062cc70
// PropModelMesh::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0062cca0
PropModelMesh::~PropModelMesh()
{
    switch (mType)
    {
    case PMM_MESH:
        if (mRenderObj.mGeometry)
        {
            delete mRenderObj.mGeometry;
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0062cd00
void PropModelMesh::Read(PblFileChunk& chunk)
{
    unsigned int uiFileHash = 0;
    bool bUseCollision = false;

    mHash = 0;
    mRenderObj.mGeometry = 0;
    mGeometryMaxDist = 1000.0f;
    mGeometryMinDist = 0.0f;
    mScale = 1.0f;
    mYOffset = 0.0f;
    mColorVariation = 0.0f;
    mCollisionModel = 0;
    m_bDynamicLighting = false;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }

        const unsigned int uiHash = child._pFile->Read32();
        const int iVersion = child._pFile->Read8();

        switch (uiHash)
        {
        case 0x6a6fb399:
        {
            mHash = child._pFile->Read32();
            const float fDist = (iVersion > 1) ? child._pFile->ReadFloat() : 1000.0f;
            mGeometryMaxDist = fDist;
            if (fDist > 1000.0f)
            {
                mGeometryMaxDist = 1000.0f;
            }
            mType = PMM_LEAF;
            break;
        }

        case 0x156b70a1:
        {
            mHash = child._pFile->Read32();
            const float fDist = (iVersion > 1) ? child._pFile->ReadFloat() : 1000.0f;
            mGeometryMaxDist = fDist;
            if (fDist > 1000.0f)
            {
                mGeometryMaxDist = 1000.0f;
            }
            mType = PMM_GRASS;
            break;
        }

        case 0xaaea5743:
            uiFileHash = child._pFile->Read32();
            if (mHash)
            {
                if (iVersion > 1)
                {
                    child._pFile->ReadFloat();
                }
            }
            else
            {
                const float fDist = (iVersion > 1) ? child._pFile->ReadFloat() : 1000.0f;
                mGeometryMaxDist = fDist;
                if (fDist > 1000.0f)
                {
                    mGeometryMaxDist = 1000.0f;
                }
            }
            break;

        case 0x0e0d9594:
            mSound.SetID(child._pFile->Read32(), false, false);
            if (iVersion == 3)
            {
                mSoundTriggerTime = child._pFile->ReadFloat();
                mSoundTriggerTimeDev = child._pFile->ReadFloat();
            }
            else
            {
                mSoundTriggerTime = 0.0f;
                mSoundTriggerTimeDev = FLT_MAX;
            }
            break;

        case 0xc28f0c96:
            mSoundCollision.SetID(child._pFile->Read32(), false, false);
            break;

        case 0x08be1de3:
            mGeometryMinDist = child._pFile->ReadFloat();
            break;

        case 0x13fc0734:
            mAIVisFactorCrouch = child._pFile->ReadFloat();
            mAIVisFactorStand = child._pFile->ReadFloat();
            break;

        case 0x26edc40c:
            bUseCollision = true;
            break;

        case 0x827eb1c9:
        {
            char acLighting[256] = { 0 };
            child.ReadString(acLighting, 256);
            const size_t uiLen = strlen(acLighting);
            if (uiLen)
            {
                if (!_strnicmp(acLighting, "dynamic", uiLen))
                {
                    m_bDynamicLighting = true;
                }
                else if (!_strnicmp(acLighting, "static", uiLen))
                {
                    m_bDynamicLighting = false;
                }
            }
            break;
        }

        case 0x82971c71:
            mScale = child._pFile->ReadFloat();
            break;

        case 0xbbce475d:
            mColorVariation = child._pFile->ReadFloat();
            break;

        default:
            ReadData(uiHash, iVersion, child);
            break;
        }
    }

    if (!uiFileHash || mHash)
    {
        mMaxDist = mGeometryMaxDist;
        return;
    }

    mRenderObj.mGeometry = RedModel::_HashTable.Find(uiFileHash);
    mYOffset = 0.0f;
    mType = PMM_MESH;
    mMaxDist = mRenderObj.mGeometry->_BoundingSphere.GetRadius() * mScale +
               mGeometryMaxDist * 1.45f;

    if (bUseCollision)
    {
        const CollisionMesh* pMesh = CollisionMesh::_HashTable.Find(uiFileHash);
        if (pMesh)
        {
            mCollisionModel = new CollisionModel;
            mCollisionModel->AddBody(new CollisionBodyMesh(
                pMesh, "CollisionMesh", gMatrixIdentity, 0));

            mCollisionModel->SetCollisionMask(COLL_RIGID, CollisionMask(0));
            mCollisionModel->SetCollisionMask(COLL_ORDNANCE, CollisionMask(0));
        }
    }
}

// FUNCTION: SWBF2 0x0062d1e0
PropObject* PropModelMesh::CreateObject(float x, float z, float rotation)
{
    if (mHash)
    {
        if (mType == PMM_GRASS)
        {
            mRenderObj.mGrassClass = static_cast<GrassPatchClass*>(EntityClass::Find(mHash));
            mRenderObj.mGrassClass->Init();
            mMaxDist = mRenderObj.mGrassClass->mBoundingRadius * mScale + mGeometryMaxDist * 1.45f;
            mYOffset = 0.0f;
        }
        else if (mType == PMM_LEAF)
        {
            mRenderObj.mLeafClass = static_cast<LeafPatchClass*>(EntityClass::Find(mHash));
            mRenderObj.mLeafClass->Init();
            mMaxDist = mRenderObj.mLeafClass->mBoundingRadius * mScale + mGeometryMaxDist * 1.45f;
            mYOffset = 0.0f;
        }
        mHash = 0;
    }

    float fChance = mMaxDist / 100.0f;
    fChance = util::Min_(fChance, 3.0f);
    fChance = util::Max_(fChance, 0.3f);
    if (GetDensity() * fChance > (float)(rand() % 0x7fff) / 32767.0f)
    {
        return PropObjectMesh::Create(this, x, z, rotation);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0062d330
void PropModelMesh::SetupMatrix(PblMatrix& matrix, float rotation)
{
    const PblVector3 vOrigin = matrix.GetTranslation();
    const float x = vOrigin.x;
    const float z = vOrigin.z;
    PblVector3 vNormal(0.0f, 1.0f, 0.0f);
    float y;

    if (mType == PMM_MESH)
    {
        PropGenerator::GetTerrainYAndNormal(x, z, y, vNormal);
    }
    else
    {
        float y0 = 0.0f;
        float y1 = 0.0f;
        float y2 = 0.0f;
        const PblVector3 v0(x, 0.0f, z + 4.0f);
        PropGenerator::GetTerrainY(v0.x, v0.z, y0);
        const PblVector3 v1(x - 3.0f, 0.0f, z - 3.0f);
        PropGenerator::GetTerrainY(v1.x, v1.z, y1);
        const PblVector3 v2(x + 3.0f, 0.0f, z - 3.0f);
        PropGenerator::GetTerrainY(v2.x, v2.z, y2);

        y = (y1 + y0 + y2) / 3.0f;

        const float ax = x - x;
        const float ay = y0 - (y2 + y1) * 0.5f;
        const float az = v0.z - v1.z;
        const float bx = v2.x - v1.x;
        const float by = y2 - y1;
        const float bz = v1.z - v1.z;
        const PblVector3 vCross(bz * ay - by * az, az * bx - bz * ax, by * ax - ay * bx);
        PblVector3 vUnit;
        D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)&vCross);
        vNormal = vUnit;
    }

    matrix.SetTranslation(PblVector3(x, y, z));

    const float fStiffness = mStiffness;
    if (fStiffness > 0.0f)
    {
        const float fFlex = 1.0f - fStiffness;
        vNormal = PblVector3(fFlex * vNormal.x + fStiffness * 0.0f, vNormal.y * fFlex + fStiffness,
                             vNormal.z * fFlex + fStiffness * 0.0f);
    }

    const float fScale = mScale;
    if (mType == PMM_MESH)
    {
        const PblVector3 vPos(x - mYOffset * vNormal.x, y - vNormal.y * mYOffset,
                              z - vNormal.z * mYOffset);
        PblVector3 vDir;
        vDir.x = PblMath::Sin(rotation);
        vDir.y = 0.0f;
        vDir.z = -PblMath::Cos(rotation);

        PblMatrix scale;
        scale.m[0][0] = fScale;
        scale.m[0][1] = 0.0f;
        scale.m[0][2] = 0.0f;
        scale.m[0][3] = 0.0f;
        scale.m[1][0] = 0.0f;
        scale.m[1][1] = fScale;
        scale.m[1][2] = 0.0f;
        scale.m[1][3] = 0.0f;
        scale.m[2][0] = 0.0f;
        scale.m[2][1] = 0.0f;
        scale.m[2][2] = fScale;
        scale.m[2][3] = 0.0f;
        scale.m[3][0] = 0.0f;
        scale.m[3][1] = 0.0f;
        scale.m[3][2] = 0.0f;
        scale.m[3][3] = 1.0f;

        const PblMatrix frame = NormalMatrix(vDir, vNormal, vPos);
        PblMatrix result;
        result = scale * frame;
        matrix = result;
        return;
    }

    matrix = gMatrixIdentity;
    matrix[1] = vNormal;
    matrix.m[1][3] = 0.0f;

    PblVector3 vOut;
    const float bx = -matrix.m[2][0];
    const float by = -matrix.m[2][1];
    const float bz = -matrix.m[2][2];
    PblVector3 vIn(matrix.m[1][2] * by - matrix.m[1][1] * bz,
                   matrix.m[1][0] * bz - matrix.m[1][2] * bx,
                   matrix.m[1][1] * bx - matrix.m[1][0] * by);
    D3DXVec3Normalize((D3DXVECTOR3*)&vOut, (const D3DXVECTOR3*)&vIn);
    matrix[0] = vOut;
    matrix.m[0][3] = 0.0f;

    vIn = PblVector3(matrix.m[0][1] * matrix.m[1][2] - matrix.m[0][2] * matrix.m[1][1],
                     matrix.m[0][2] * matrix.m[1][0] - matrix.m[0][0] * matrix.m[1][2],
                     matrix.m[0][0] * matrix.m[1][1] - matrix.m[0][1] * matrix.m[1][0]);
    D3DXVec3Normalize((D3DXVECTOR3*)&vOut, (const D3DXVECTOR3*)&vIn);
    matrix[2] = vOut;
    matrix.m[2][3] = 0.0f;

    matrix.m[3][0] = x - mYOffset * vNormal.x;
    matrix.m[3][1] = y - vNormal.y * mYOffset;
    matrix.m[3][2] = z - mYOffset * vNormal.z;
    matrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0062d900
void PropModelMesh::CreateCollisionObject(float x, float z, float rotation)
{
    if (!mCollisionModel)
    {
        return;
    }

    if (NetGame::IsEnabled())
    {
        mType = PMM_INVALID;
        return;
    }

    PblMatrix matrix = gMatrixIdentity;
    matrix.m[3][0] = x;
    matrix.m[3][1] = 0.0f;
    matrix.m[3][2] = -z;
    matrix.m[3][3] = 1.0f;
    SetupMatrix(matrix, rotation);
    new PropCollisionObject(matrix, this);
}
