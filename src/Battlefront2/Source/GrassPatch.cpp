#include <d3dx9math.h>
#include <math.h>
#include <stdio.h>

#include "GrassPatch.h"
#include "Battlefront2Lod.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "CameraManager.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"


class PblMatrix;
extern PblMatrix gMatrixIdentity;
extern NoLodData noloddata;

// GLOBAL: SWBF2 0x007eb9bc
PblList<GrassPatchClass> GrassPatchClass::sClasses;

// GLOBAL: SWBF2 0x007eb9e8
PblList<GrassPatch> GrassPatchClass::sObjects;

static bool bGrassPatchEnable = true;

// FUNCTION: SWBF2 0x00540d10
GrassPatchClass::GrassPatchClass(unsigned int uHash)
    : Thread()
    , EntityClass(uHash)
{
    mTransparentFlag = false;
    mParticle = 0;
    mNumParticles = 50;
    mBoundingRadius = 64.0f;
    mSizeMin = 10.0f;
    mSizeMax = 15.0f;
    mAlpha = 0.5f;
    mRenderData = 0;
    mTextureHash = 0xB26FAC7Du /* "Grass" */;
    mRadiusFadeMin = -1.0f;
    mRadiusFadeMax = -1.0f;
    mMaxDistance = 100.0f;
    mDarknessMin = 1.0f;
    mDarknessMax = 1.0f;
    mNumParts = 4;
    mBoxX = -1.0f;
    mBoxZ = -1.0f;
    mYOffset = 0.0f;
    mFlatHeightMin = 0.2f;
    mFlatHeightMax = 0.2f;
    mFlatSizeMultiplier = 2.0f;
    mFlatFaceFactor = 0.5f;
    mFlatShadowHeight = 0.0f;
    mFlatGrassSwingFlag = false;
    mFlatCount = 6;
    mSkinnyFactor = 0.3f;
    mMaxSkew = 0.1f;

    sClasses.AddLast(&mClassNode, this);
}

// FUNCTION: SWBF2 0x005409e0
GrassPatch::GrassPatch(GrassPatchClass* pClass, const PblMatrix& matrix)
    : RedSceneObject(&noloddata)
{
    mClass = pClass;
    mMatrix = gMatrixIdentity;

    mMatrix[1] = matrix[1];
    mMatrix.m[1][3] = 0.0f;

    mMatrix[0] = Normalize(CrossProduct(-mMatrix[2], mMatrix[1]));
    mMatrix.m[0][3] = 0.0f;

    mMatrix[2] = Normalize(CrossProduct(mMatrix[0], mMatrix[1]));
    mMatrix.m[2][3] = 0.0f;

    mMatrix[3] = matrix[3];
    mMatrix.m[3][3] = 1.0f;

    PblVector3 vCenter;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vCenter), reinterpret_cast<const D3DXVECTOR3*>(&mClass->mBoundingOffset), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
    SetRenderPos(vCenter);
    SetRenderRadius(mClass->mBoundingRadius);
    Activate();

    GrassPatchClass::sObjects.AddLast(&mObjectNode, this);
}

// FUNCTION: SWBF2 0x00540ca0
GrassPatch::~GrassPatch()
{
}

// SYNTHETIC: SWBF2 0x005421b5
// ??_EGrassPatch@@WFI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00540c70
// GrassPatch::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00540cc0
bool GrassPatch::IsEnabled()
{
    return bGrassPatchEnable;
}

// FUNCTION: SWBF2 0x00540ef0
GrassPatchClass::~GrassPatchClass()
{
    if (mParticle)
    {
        delete mParticle;
    }
    DestroyRenderData();
}

// SYNTHETIC: SWBF2 0x00540ec0
// GrassPatchClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005421bd
// ??_EGrassPatchClass@@WBI@AEPAXI@Z

// FUNCTION: SWBF2 0x00540f70
void GrassPatchClass::CreateClient()
{
    for (PblList<GrassPatch>::Itor it(sObjects); (GrassPatch*)it; ++it)
    {
        it->Deactivate();
    }
    NetGame::EnterClient();
    for (PblList<GrassPatch>::Itor it(sObjects); (GrassPatch*)it; ++it)
    {
        it->Activate();
    }
    NetGame::LeaveClient();

    for (PblList<GrassPatchClass>::Itor it(sClasses); (GrassPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->DeactivateThread();
        }
    }
    NetGame::EnterClient();
    for (PblList<GrassPatchClass>::Itor it(sClasses); (GrassPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->ActivateThread(0, PER_FRAME);
        }
    }
    NetGame::LeaveClient();
}

// FUNCTION: SWBF2 0x00541020
void GrassPatchClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xD133F7DA:
        if (sscanf(pszValue, "%i", &mNumParticles))
        {
            mNumParticles /= CameraManager::Instance()->GetNumCameras();
        }
        break;
    case 0xEAEBCF6E:
        sscanf(pszValue, "%f", &mSizeMin);
        break;
    case 0x80BA1308:
        sscanf(pszValue, "%f", &mSizeMax);
        break;
    case 0x3D5D0AEF:
        sscanf(pszValue, "%f", &mYOffset);
        break;
    case 0x5D8B6DAB:
        sscanf(pszValue, "%f", &mAlpha);
        break;
    case 0xE6915CF6:
        sscanf(pszValue, "%f", &mMaxDistance);
        break;
    case 0x3C6468F4:
        mTextureHash = PblHash(pszValue);
        break;
    case 0x80887A6F:
        sscanf(pszValue, "%f", &mRadiusFadeMin);
        break;
    case 0x6E74D661:
        sscanf(pszValue, "%f", &mRadiusFadeMax);
        break;
    case 0x13FAC25E:
        sscanf(pszValue, "%f", &mDarknessMin);
        break;
    case 0x25E5C3FC:
        sscanf(pszValue, "%f", &mDarknessMax);
        break;
    case 0x5AC02207:
        sscanf(pszValue, "%i", &mNumParts);
        break;
    case 0x35A4393B:
        sscanf(pszValue, "%f %f", &mBoxX, &mBoxZ);
        break;
    case 0x231FCC79:
        sscanf(pszValue, "%f %f", &mFlatHeightMin, &mFlatHeightMax);
        break;
    case 0xA9B14958:
        sscanf(pszValue, "%f", &mFlatSizeMultiplier);
        break;
    case 0xF6582B9C:
        sscanf(pszValue, "%f", &mFlatFaceFactor);
        break;
    case 0xC75263E1:
        sscanf(pszValue, "%f", &mFlatShadowHeight);
        break;
    case 0x97133EEB:
        sscanf(pszValue, "%f", &mMaxSkew);
        break;
    case 0x4B6D5171:
        sscanf(pszValue, "%i", &mFlatCount);
        break;
    case 0xFF7BB39A:
        sscanf(pszValue, "%f", &mSkinnyFactor);
        break;
    case 0x604DB82E:
        mFlatGrassSwingFlag = true;
        break;
    case 0x9C3F7A19:
        mTransparentFlag = true;
        break;
    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00541280
EntityClass* GrassPatchClass::Derive(unsigned int uHash)
{
    return new GrassPatchClass(uHash);
}

// FUNCTION: SWBF2 0x005412f0
void GrassPatchClass::Init()
{
    if (mParticle)
    {
        return;
    }

    mParticle = new GrassParticle[mNumParticles];

    PblVector3 vMin(100.0f, 100.0f, 100.0f);
    PblVector3 vMax(-100.0f, -100.0f, -100.0f);

    for (int i = 0; i < mNumParticles; ++i)
    {
        float fFade;

        if (mBoxX > 0.0f || mBoxZ > 0.0f)
        {
            const float fZ = (PblRandom::_Global.GetFloat() - 0.5f) * mBoxZ;
            const float fX = (PblRandom::_Global.GetFloat() - 0.5f) * mBoxX;
            mParticle[i].mPos = PblVector3(fX, 0.0f, fZ);
            fFade = 1.0f;
        }
        else
        {
            fFade = 0.0f;
            while (fFade == 0.0f)
            {
                const float fZ = (PblRandom::_Global.GetFloat() * 2.0f - 1.0f) * mRadiusFadeMax;
                const float fX = (PblRandom::_Global.GetFloat() * 2.0f - 1.0f) * mRadiusFadeMax;
                mParticle[i].mPos = PblVector3(fX, 0.0f, fZ);

                const float fDist = PblMath::Sqrt(mParticle[i].mPos.x * mParticle[i].mPos.x
                                          + mParticle[i].mPos.y * mParticle[i].mPos.y
                                          + mParticle[i].mPos.z * mParticle[i].mPos.z);
                if (fDist < mRadiusFadeMin)
                {
                    fFade = 1.0f;
                }
                else if (fDist < mRadiusFadeMax)
                {
                    fFade = 1.0f - (fDist - mRadiusFadeMin) / (mRadiusFadeMax - mRadiusFadeMin);
                }
            }
        }

        mParticle[i].mSize = (mSizeMax - mSizeMin) * PblRandom::_Global.GetFloat() + mSizeMin;

        const float fDarkness = (mDarknessMax - mDarknessMin) * PblRandom::_Global.GetFloat() + mDarknessMin;
        mParticle[i].mColor.a = mAlpha;
        mParticle[i].mColor.r = fDarkness;
        mParticle[i].mColor.g = fDarkness;
        mParticle[i].mColor.b = fDarkness;

        float fA, fB;
        do
        {
            fA = PblRandom::_Global.GetFloat() - 0.5f;
            fB = PblRandom::_Global.GetFloat() - 0.5f;
        } while (fA * fA + fB * fB == 0.0f);

        mParticle[i].mTime = PblRandom::_Global.GetFloat() * 10.0f;
        mParticle[i].mOffset = 0.0f;
        mParticle[i].mLODPriority = 0.0f;
        mParticle[i].mFlatType = false;
        mParticle[i].mSkew = (PblRandom::_Global.GetFloat() - 0.5f) * (mMaxSkew * 2.0f);

        if (mNumParts == 3 || mNumParts > 4)
        {
            bool bFlat = false;
            if (mNumParts == 3)
            {
                mParticle[i].mTexture = (unsigned char)(i % (mFlatCount + 2));
                if (mParticle[i].mTexture > 1)
                {
                    mParticle[i].mTexture = 2;
                    bFlat = true;
                }
            }
            else
            {
                mParticle[i].mTexture = (unsigned char)(i % (mFlatCount + 4));
                while (mParticle[i].mTexture > 5)
                {
                    mParticle[i].mTexture -= 2;
                }
                bFlat = mParticle[i].mTexture > 3;
            }

            mParticle[i].mPos.y = mYOffset;
            if (bFlat)
            {
                mParticle[i].mHeight = mParticle[i].mSize * mFlatSizeMultiplier;
                mParticle[i].mSize = mFlatSizeMultiplier * mParticle[i].mSize;
                mParticle[i].mUpFactor = PblRandom::_Global.GetFloat() * 0.1f + 0.05f;
                mParticle[i].mPos.y += ((mFlatHeightMax - mFlatHeightMin) * PblRandom::_Global.GetFloat()
                                    * (1.0f - fFade) + mFlatHeightMin) * mSizeMin;
                mParticle[i].mFlatType = true;

                do
                {
                    const float fZ = PblRandom::_Global.GetFloat() - 0.5f;
                    const float fX = PblRandom::_Global.GetFloat() - 0.5f;
                    mParticle[i].mUp = PblVector3(fX, 0.0f, fZ);
                } while (fabsf(mParticle[i].mUp.x) < 0.01f && fabsf(mParticle[i].mUp.z) < 0.01f);
                D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&mParticle[i].mUp), reinterpret_cast<const D3DXVECTOR3*>(&mParticle[i].mUp));
                mParticle[i].mUp.y = (PblRandom::_Global.GetFloat() - 0.5f) * 0.3f;
                D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&mParticle[i].mUp), reinterpret_cast<const D3DXVECTOR3*>(&mParticle[i].mUp));
            }
            else
            {
                mParticle[i].mHeight = mParticle[i].mSize;
                mParticle[i].mSize = mSkinnyFactor * mParticle[i].mSize;
                mParticle[i].mUpFactor = PblRandom::_Global.GetFloat() * 0.2f + 0.8f;
                mParticle[i].mLODPriority = 10000.0f;
            }
        }
        else
        {
            mParticle[i].mUpFactor = 1.0f;
            mParticle[i].mHeight = mParticle[i].mSize * 0.9f;
            mParticle[i].mSize = mParticle[i].mSize * 1.1f;
            mParticle[i].mTexture = (unsigned char)(i % 4);
            mParticle[i].mPos.y = mYOffset;
            mParticle[i].mPos.y += (1.0f - fFade) * (mSizeMin * -0.5f);
        }

        const float fHalfSize = mParticle[i].mSize * 0.5f;
        vMin.x = Min(vMin.x, mParticle[i].mPos.x - fHalfSize);
        vMin.y = Min(vMin.y, mParticle[i].mPos.y - mParticle[i].mSize);
        vMin.z = Min(vMin.z, mParticle[i].mPos.z - fHalfSize);
        vMax.x = Max(vMax.x, mParticle[i].mPos.x + fHalfSize);
        vMax.y = Max(vMax.y, mParticle[i].mPos.y + mParticle[i].mSize);
        vMax.z = Max(vMax.z, mParticle[i].mPos.z + fHalfSize);
    }

    mBoundingOffset.x = (vMax.x + vMin.x) * 0.5f;
    mBoundingOffset.y = (vMax.y + vMin.y) * 0.5f;
    mBoundingOffset.z = (vMax.z + vMin.z) * 0.5f;
    mBoundingRadius = 0.0f;
    mBoundingBoxRadius.x = vMax.x - mBoundingOffset.x;
    mBoundingBoxRadius.y = vMax.y - mBoundingOffset.y;
    mBoundingBoxRadius.z = vMax.z - mBoundingOffset.z;

    for (int i = 0; i < mNumParticles; ++i)
    {
        const PblVector3& vPos = mParticle[i].mPos;
        const float fX = vPos.x - mBoundingOffset.x;
        const float fY = vPos.y - mBoundingOffset.y;
        const float fZ = vPos.z - mBoundingOffset.z;
        const float fRadius = PblMath::Sqrt(fX * fX + fY * fY + fZ * fZ) + mParticle[i].mSize;
        mBoundingRadius = Max(fRadius, mBoundingRadius);
    }

    ActivateThread(0, PER_FRAME);
    CreateRenderData();
}

// FUNCTION: SWBF2 0x00541f80
Entity* GrassPatchClass::Build(const EntityDesc& desc)
{
    if (!mParticle)
    {
        Init();
    }
    return new GrassPatch(this, desc.mMatrix);
}

// FUNCTION: SWBF2 0x00542000
bool GrassPatchClass::Update(float fDeltaTime)
{
    for (int i = 0; i < mNumParticles; ++i)
    {
        mParticle[i].mTime += ((float)(i % 5) * 0.2f + 1.0f) * (fDeltaTime * 0.8f);
        mParticle[i].mOffset = PblMath::Sin(mParticle[i].mTime);
    }
    return true;
}

// FUNCTION: SWBF2 0x005420c0
void GrassPatchClass::Render(const PblMatrix& matrix, const GrassPatch*, unsigned int uiFlags) const
{
    const GrassPatchClass* pClass = (const GrassPatchClass*)this;
    const RedColorValue color(1.0f, 1.0f, 1.0f, 1.0f);
    pClass->RenderMaxDist(matrix, pClass->mMaxDistance, color, uiFlags);
}

// FUNCTION: SWBF2 0x00540cd0
void GrassPatch::Render(unsigned char, float, unsigned int uiFlags)
{
    if (!RedSceneManager::InTransparentPhase())
    {
        RedSceneManager::AddTransparentObject(this);
        return;
    }
    ((GrassPatchClass*)mClass)->Render(mMatrix, this, uiFlags);
}

// FUNCTION: SWBF2 0x005420f0
void GrassPatchClass::Reset()
{
    for (PblList<GrassPatchClass>::Itor it(sClasses); (GrassPatchClass*)it; ++it)
    {
        it->DestroyRenderData();
    }

    sClasses.RemoveAll();
    sObjects.RemoveAll();
}

// TEMPLATE: SWBF2 0x00542170
// PblList<GrassPatchClass>::RemoveAll
template void PblList<GrassPatchClass>::RemoveAll();
