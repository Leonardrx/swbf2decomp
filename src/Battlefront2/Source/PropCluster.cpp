#include "PebbleFL/Common/PblMath.h"
#include "PropCluster.h"

#include <float.h>
#include <math.h>

#include "PropGenerator.h"
#include "PropModelMesh.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "Battlefront2Lod.h"
#include "CameraManager.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

namespace FLRenderer
{

unsigned int GetCurrentViewport();
}

// GLOBAL: SWBF2 0x007e903c

float sGrassDistanceScale = 1.0f;

// GLOBAL: SWBF2 0x01eaefc6
bool gTracePropClusterRenders;

// GLOBAL: SWBF2 0x01faa730
int gPropClusterRenderedObjectCount;

// GLOBAL: SWBF2 0x01faa734
int gPropClusterRenderCount;

extern NoLodData noloddata;

// FUNCTION: SWBF2 0x006295c0
PropCluster::PropCluster()
    : RedSceneObject(&noloddata)
{
}

// FUNCTION: SWBF2 0x006296d0
void PropCluster::Init(unsigned char id, float fSize)
{
    mId = id;
    mWasRenderedFlag = 0;
    mMapIndex = -1;
    mSize = fSize;

    for (int i = 0; i < 4; i++)
    {
        while (mLayers[i].mObjects.GetCount() > 0)
        {
            mLayers[i].mObjects.Remove(mLayers[i].mObjects.GetFirst());
        }
    }
}

// FUNCTION: SWBF2 0x00629740
PropCluster::~PropCluster()
{
    DeleteObjects();
}

// FUNCTION: SWBF2 0x006297a0
void SetPropDistanceScale(float fValue)
{
    sGrassDistanceScale = fValue;
}

// FUNCTION: SWBF2 0x006297b0
void PropCluster::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    const RedCamera* pCamera = RedRenderer::GetCamera();

    int iNumObjects = 0;
    const PblMatrix& mCamera = pCamera->GetMatrix();
    const PblVector3 vForward = -mCamera.GetZAxis();
    const PblVector3 vCamPos = mCamera.GetTranslation();
    const PblVector3 vToCluster(_Sphere.x - vCamPos.x, 0.0f, _Sphere.z - vCamPos.z);
    float fCamDist = DotProduct(vToCluster, vForward);
    float fScale = 1.0f;
    if (!(1.0f > sGrassDistanceScale))
    {
        fScale = sGrassDistanceScale;
    }

    if (fScale * fCamDist > -mSize)
    {
        for (int i = 0; i < 4; i++)
        {
            float fLayerDist = PropGenerator::GetLayerMaxDist(i) * fScale;

            if (!(mSize + fLayerDist > fCamDist))
            {
                if (fCamDist > fLayerDist * 1.05f + mSize)
                {
                    UncreateObjects(i);
                }
            }
            else
            {
                if (mLayers[i].mCreateObjectsFlag)
                {
                    PropGenerator::AddObjectsFromClusterIndex(mMapIndex, i, mLayers[i].mObjects);
                    if (mLayers[i].mObjects.GetCount() > 0)
                    {
                        mLayers[i].mCreateObjectsFlag = false;
                    }
                }

                if (mLayers[i].mObjects.GetCount() > 0)
                {
                    CameraManager* pCameraManager = CameraManager::sInstance;
                    mWasRenderedFlag |= 1 << pCameraManager->GetCameraIdForView(FLRenderer::GetCurrentViewport());
                    for (PblList<PropObject>::Itor it(mLayers[i].mObjects); (PropObject*)it; ++it)
                    {
                        it->Render(pCamera, fCamDist - mSize, uiFlags);
                    }
                }
            }

            iNumObjects += mLayers[i].mObjects.GetCount();
        }
    }

    if (gTracePropClusterRenders)
    {
        // GLOBAL: SWBF2 0x01fac1ac
        static const RedCamera* s_pLastCamera = pCamera;
        ++gPropClusterRenderCount;
        gPropClusterRenderedObjectCount += iNumObjects;
        if (s_pLastCamera != pCamera)
        {
            s_pLastCamera = pCamera;
            gPropClusterRenderCount = 0;
            gPropClusterRenderedObjectCount = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006299e0
void PropCluster::UncreateObjects(int iLayer)
{
    while (mLayers[iLayer].mObjects.GetCount())
    {
        mLayers[iLayer].mObjects.GetFirst()->_pObject->Release();
        mLayers[iLayer].mObjects.Remove(mLayers[iLayer].mObjects.GetFirst());
    }
    mLayers[iLayer].mCreateObjectsFlag = true;
}

// FUNCTION: SWBF2 0x00629a40
void PropCluster::Setup(int iMapIndex, float fX, float fZ)
{
    mMapIndex = iMapIndex;

    float fY;
    PropGenerator::GetTerrainY(fX, -fZ, fY);
    float fMinY = fY;

    float fHalf = mSize * 0.5f;
    PropGenerator::GetTerrainY(fX - fHalf, -(fZ - fHalf), fY);
    if (fMinY > fY)
    {
        fMinY = fY;
    }
    fHalf = mSize * 0.5f;
    PropGenerator::GetTerrainY(fX + fHalf, -(fZ - fHalf), fY);
    if (fMinY > fY)
    {
        fMinY = fY;
    }
    fHalf = mSize * 0.5f;
    PropGenerator::GetTerrainY(fX - fHalf, -(fZ + fHalf), fY);
    if (fMinY > fY)
    {
        fMinY = fY;
    }
    fHalf = mSize * 0.5f;
    PropGenerator::GetTerrainY(fX + fHalf, -(fZ + fHalf), fY);
    if (fMinY > fY)
    {
        fMinY = fY;
    }

    _pLodData = const_cast<RedLodData*>(PropGenerator::GetLodData(mMapIndex));
    if (_pLodData)
    {
        PblVector3 vPos(fX, mSize * 0.4f + fMinY, -fZ);
        SetRenderPos(vPos);
        SetRenderRadius(mSize);
        Activate();
        for (int i = 0; i < 4; i++)
        {
            mLayers[i].mCreateObjectsFlag = true;
        }
    }
}

// FUNCTION: SWBF2 0x00629c10
void PropCluster::DeleteObjects()
{
    if (_bActive)
    {
        Deactivate();
    }
    for (int i = 0; i < 4; i++)
    {
        UncreateObjects(i);
    }
    mMapIndex = -1;
}

// FUNCTION: SWBF2 0x00629c40
const PropObject* PropCluster::FindPropObject(const PblVector3& vPos, bool) const
{
    float fBest = FLT_MAX;
    const PropObject* pBest = 0;

    for (unsigned int i = 0; i < 4; i++)
    {
        for (PblList<PropObject>::Itor it(mLayers[i].mObjects); (PropObject*)it; ++it)
        {
            PropObjectMesh* pObject = (PropObjectMesh*)(PropObject*)it;
            switch (pObject->mModel->mType)
            {
            case PropModelMesh::PMM_MESH:
            {
                PblSphere sphere = pObject->mModel->mRenderObj.mGeometry->_BoundingSphere;
                const PblVector3& vTrans = pObject->GetMatrix().GetTranslation();
                if (sphere.GetRadius() > Length(vPos - (vTrans + sphere.GetCenter())))
                {
                    float fVis = pObject->mModel->mAIVisFactorCrouch +
                                 pObject->mModel->mAIVisFactorStand;
                    if (fBest > fVis)
                    {
                        fBest = fVis;
                        pBest = pObject;
                    }
                }
                break;
            }

            case PropModelMesh::PMM_GRASS:
            {
                const PblVector3& vTrans = pObject->GetMatrix().GetTranslation();
                GrassPatchClass* pClass = pObject->mModel->mRenderObj.mGrassClass;
                int iCount = pClass->mNumParticles;
                if (pClass->mBoundingRadius > Length(vPos - vTrans) && iCount)
                {
                    GrassPatchClass::GrassParticle* pParticle = pClass->mParticle;
                    do
                    {
                        if (pParticle->mSize > Length(vPos - (vTrans + pParticle->mPos)))
                        {
                            float fVis = pObject->mModel->mAIVisFactorCrouch +
                                         pObject->mModel->mAIVisFactorStand;
                            if (fBest > fVis)
                            {
                                fBest = fVis;
                                pBest = pObject;
                            }
                        }
                        ++pParticle;
                    } while (--iCount);
                }
                break;
            }

            case PropModelMesh::PMM_LEAF:
            {
                const PblVector3& vTrans = pObject->GetMatrix().GetTranslation();
                LeafPatchClass* pClass = pObject->mModel->mRenderObj.mLeafClass;
                int iCount = pClass->mNumParticles;
                if (pClass->mBoundingRadius > Length(vPos - vTrans) && iCount)
                {
                    LeafPatchClass::LeafParticle* pParticle = pClass->mParticle;
                    do
                    {
                        if (pParticle->mSize > Length(vPos - (pParticle->mPos + vTrans)))
                        {
                            float fVis = pObject->mModel->mAIVisFactorCrouch +
                                         pObject->mModel->mAIVisFactorStand;
                            if (fBest > fVis)
                            {
                                fBest = fVis;
                                pBest = pObject;
                            }
                        }
                        ++pParticle;
                    } while (--iCount);
                }
                break;
            }

            default:
                break;
            }
        }
    }
    return pBest;
}

// SYNTHETIC: SWBF2 0x006296a0
// PropCluster::`scalar deleting destructor'
