#include <d3dx9math.h>
#include <math.h>
#include <stdio.h>

#include "LeafPatch.h"
#include "Battlefront2Lod.h"
#include "CollisionManager.h"
#include "CollisionUtil.h"
#include "LeafEffect.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "CameraManager.h"
#include "NetGame.h"

#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"

// SYNTHETIC: SWBF2 0x0056dbd0
// LeafPatchClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056dcb0
LeafPatchClass::~LeafPatchClass()
{
    if (mParticle)
    {
        delete mParticle;
    }
    DestroyRenderData();
}

// SYNTHETIC: SWBF2 0x0056f41d
// ??_ELeafPatchClass@@WBI@AEPAXI@Z


// FUNCTION: SWBF2 0x0056dc00
void LeafPatchClass::CreateClient()
{
    for (PblList<LeafPatch>::Itor it(sObjects); (LeafPatch*)it; ++it)
    {
        it->Deactivate();
    }
    NetGame::EnterClient();
    for (PblList<LeafPatch>::Itor it(sObjects); (LeafPatch*)it; ++it)
    {
        it->Activate();
    }
    NetGame::LeaveClient();

    for (PblList<LeafPatchClass>::Itor it(sClasses); (LeafPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->DeactivateThread();
        }
    }
    NetGame::EnterClient();
    for (PblList<LeafPatchClass>::Itor it(sClasses); (LeafPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->ActivateThread(0, PER_FRAME);
        }
    }
    NetGame::LeaveClient();
}

// GLOBAL: SWBF2 0x007ebaec
PblList<LeafPatchClass> LeafPatchClass::sClasses;

// FUNCTION: SWBF2 0x0056d9b0
LeafPatchClass::LeafPatchClass(unsigned int uHash)
    : Thread()
    , EntityClass(uHash)
{
    mParticle = 0;
    mNumParticles = 50;
    mBoundingRadius = 10.0f;
    mSizeMin = 10.0f;
    mSizeMax = 15.0f;
    mAlpha = 0.5f;
    mRenderData = 0;
    mTextureHash = 0xB26FAC7Du;
    mRadius = 5.0f;
    mHeight = 5.0f;
    mMaxDistance = 100.0f;
    mDarknessMin = 1.0f;
    mDarknessMax = 1.0f;
    mMaxFallingLeaves = 0;
    mMaxScatterBirds = 0;
    mOffset.z = 0.0f;
    mOffset.y = 0.0f;
    mOffset.x = 0.0f;
    mHeightScale = 1.0f;
    mSeed = (int)PblRandom::GetGlobalInt();
    mNearFadeDist = 6.0f;
    mNumParts = 4;

    sClasses.AddLast(&mClassNode, this);

    mBoxSize.z = 0.0f;
    mBoxSize.y = 0.0f;
    mBoxSize.x = 0.0f;
    mConeHeight = 0.0f;
    mType = 0;
    mVineXSpread = 1.0f;
    mVineZSpread = 0.0f;
    mWiggleSpeed = 1.2f;
    mWiggleAmount = 0.11f;
    mVineLengthMin = 2;
    mVineLengthMax = 4;
    mVineSpread = 0.6f;
    mFadeInStep = 1.0f;
}

// GLOBAL: SWBF2 0x01e573d4
int sLeafParticleIndex;

// FUNCTION: SWBF2 0x0056dd30
void LeafPatchClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xD133F7DA:
        if (sscanf(pszValue, "%i", &mNumParticles))
        {
            mNumParticles /= CameraManager::Instance()->GetNumCameras();
        }
        mNumVisible = mNumParticles;
        break;
    case 0x1ABD7877:
        sscanf(pszValue, "%i", &mNumVisible);
        mFadeInStep = 1.0f / ((float)mNumVisible * 0.2f);
        break;
    case 0x0DBA4CB3:
        sscanf(pszValue, "%f", &mRadius);
        break;
    case 0xCAAA3778:
        sscanf(pszValue, "%f", &mHeightScale);
        break;
    case 0xD5BDBB42:
        sscanf(pszValue, "%f", &mHeight);
        break;
    case 0x5045BCAC:
        sscanf(pszValue, "%i", &mSeed);
        break;
    case 0x14C8D3CA:
    {
        float fX, fY, fZ;
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mOffset.x = fX;
        mOffset.y = fY;
        mOffset.z = fZ;
        break;
    }
    case 0xEAEBCF6E:
        sscanf(pszValue, "%f", &mSizeMin);
        break;
    case 0x80BA1308:
        sscanf(pszValue, "%f", &mSizeMax);
        break;
    case 0x5D8B6DAB:
        sscanf(pszValue, "%f", &mAlpha);
        break;
    case 0xE6915CF6:
        sscanf(pszValue, "%f", &mMaxDistance);
        break;
    case 0xEAAAA7A9:
        sscanf(pszValue, "%f", &mConeHeight);
        break;
    case 0x35A4393B:
    {
        float fX, fY, fZ;
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mBoxSize.x = fX;
        mBoxSize.y = fY;
        mBoxSize.z = fZ;
        mType = 2;
        break;
    }
    case 0x3C6468F4:
        mTextureHash = PblHash(pszValue);
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
    case 0xFD7FAE90:
    {
        int iCount;
        sscanf(pszValue, "%i", &iCount);
        mMaxFallingLeaves = (unsigned char)(iCount > 50 ? 50 : iCount);
        break;
    }
    case 0x3B1948F3:
    {
        int iCount;
        sscanf(pszValue, "%i", &iCount);
        mMaxScatterBirds = (unsigned char)(iCount > 5 ? 5 : iCount);
        break;
    }
    case 0xCDBC383D:
        sscanf(pszValue, "%f %f", &mVineXSpread, &mVineZSpread);
        mType = 3;
        break;
    case 0xA8C0D41D:
        sscanf(pszValue, "%i %i", &mVineLengthMin, &mVineLengthMax);
        break;
    case 0x45328DC6:
        sscanf(pszValue, "%f", &mVineSpread);
        break;
    case 0xD4CBD35B:
        sscanf(pszValue, "%f", &mWiggleSpeed);
        break;
    case 0x7CBB796C:
        sscanf(pszValue, "%f", &mWiggleAmount);
        break;
    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x0056e110
EntityClass* LeafPatchClass::Derive(unsigned int uHash)
{
    return new LeafPatchClass(uHash);
}

// FUNCTION: SWBF2 0x0056e180
void LeafPatchClass::CreateVineBranch(PblVector3 vPos, PblVector3 vDir, int iCount, PblRandom& rand)
{
    int iLeft = iCount - 1;
    if (iLeft < 0)
    {
        return;
    }

    mParticle[sLeafParticleIndex].mPos = vPos;
    float fScale = ((float)iLeft + 1.0f) * 0.25f;
    float fSize = (mSizeMax - mSizeMin) * rand.GetFloat() + mSizeMin;
    fScale = Min(fScale, 1.0f);
    fSize = fScale * fSize;
    mParticle[sLeafParticleIndex++].mSize = fSize;

    int iLength = Min((int)((float)(mVineLengthMax - mVineLengthMin) * rand.GetFloat() + (float)mVineLengthMin), iLeft);

    for (; iLength != 0; --iLength)
    {
        const PblVector3 vWander((rand.GetFloat() - rand.GetFloat()) * 0.5f * mVineXSpread, 0.2f,
                                 (rand.GetFloat() - rand.GetFloat()) * 0.5f * mVineZSpread);
        vDir += vWander;
        vDir = Normalize(vDir);

        vPos += vDir * fSize * mVineSpread;

        fScale = ((float)iLeft + 1.0f) * 0.25f;
        fSize = (mSizeMax - mSizeMin) * rand.GetFloat() + mSizeMin;
        fScale = Min(fScale, 1.0f);
        --iLeft;
        fSize = fScale * fSize;
        mParticle[sLeafParticleIndex].mPos = vPos;
        mParticle[sLeafParticleIndex].mSize = fSize;
        ++sLeafParticleIndex;
    }

    if (iLeft > 0)
    {
        PblVector3 vBranch;
        vBranch = Interpolate(vDir,
                              PblVector3((rand.GetFloat() - rand.GetFloat()) * mVineXSpread, 0.0f,
                                         (rand.GetFloat() - rand.GetFloat()) * mVineZSpread),
                              0.8f);
        vBranch = Normalize(vBranch);
        const int iSplit = (int)((rand.GetFloat() * 0.5f + 0.25f) * (float)iLeft);
        CreateVineBranch(vBranch * fSize * mVineSpread + vPos, vBranch, iSplit, rand);

        vBranch = Interpolate(vDir,
                              PblVector3((rand.GetFloat() - rand.GetFloat()) * mVineXSpread, 0.0f,
                                         (rand.GetFloat() - rand.GetFloat()) * mVineZSpread),
                              0.8f);
        vBranch = Normalize(vBranch);
        CreateVineBranch(vBranch * fSize * mVineSpread + vPos, vBranch, iLeft - iSplit, rand);
    }
}

// FUNCTION: SWBF2 0x0056e970
void LeafPatchClass::Init()
{
    if (mParticle)
    {
        return;
    }

    mParticle = new LeafParticle[mNumParticles];

    const float fInvHeightScale = 1.0f / mHeightScale;
    const PblVector3 vOffset(mOffset.x, mOffset.y * fInvHeightScale, mOffset.z);

    PblRandom rand(mSeed ^ 0xC71C2830u);
    rand.GetInt();

    char cQuadrant = 0;

    if (mType == 3)
    {
        sLeafParticleIndex = 0;
        CreateVineBranch(PblVector3(0.0f, 0.0f, 0.0f), PblVector3(0.0f, 1.0f, 0.0f), mNumParticles, rand);
    }

    PblVector3 vMin(100.0f, 100.0f, 100.0f);
    PblVector3 vMax(-100.0f, -100.0f, -100.0f);

    for (int i = 0; i < mNumParticles; ++i)
    {
        if (mType != 3)
        {
            if (mType != 2)
            {
                do
                {
                    const float fZ = rand.GetFloat() * mRadius * 1.5f;
                    const float fY = rand.GetFloat() * mRadius;
                    const float fX = rand.GetFloat() * mRadius * 1.5f;
                    PblVector3& vPos = mParticle[i].mPos;
                    vPos.x = fX;
                    vPos.y = fY;
                    vPos.z = fZ;
                    if (cQuadrant < 2)
                    {
                        vPos.x = -vPos.x;
                    }
                    if (cQuadrant == 1 || cQuadrant == 2)
                    {
                        vPos.z = -vPos.z;
                    }
                    cQuadrant = (char)((cQuadrant + 1) % 4);
                } while (mParticle[i].mPos.x * mParticle[i].mPos.x + mParticle[i].mPos.y * mParticle[i].mPos.y
                             + mParticle[i].mPos.z * mParticle[i].mPos.z
                         > mRadius * mRadius);

                mParticle[i].mPos.y = fInvHeightScale * mHeight / mRadius * mParticle[i].mPos.y;
                if (mConeHeight > 0.0f && mParticle[i].mPos.y > mHeight - mConeHeight)
                {
                    const float fPinch = (mHeight - mParticle[i].mPos.y) / mConeHeight;
                    mParticle[i].mPos.x *= fPinch;
                    mParticle[i].mPos.z *= fPinch;
                }
            }
            else
            {
                const float fZ = (rand.GetFloat() - 0.5f) * mBoxSize.z;
                const float fY = (rand.GetFloat() - 0.5f) * mBoxSize.y;
                const float fX = (rand.GetFloat() - 0.5f) * mBoxSize.x;
                PblVector3& vPos = mParticle[i].mPos;
                vPos.x = fX;
                vPos.y = fY;
                vPos.z = fZ;
            }
            mParticle[i].mSize = (mSizeMax - mSizeMin) * rand.GetFloat() + mSizeMin;
        }

        const float fDarknessRange = mDarknessMax - mDarknessMin;
        const RedDirectionalLight* pLight = RedLight::m_sGlobalDirectionals[0];
        const float fDarkness = rand.GetFloat() * fDarknessRange * 0.5f + mDarknessMin;
        PblVector3 vNormal;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNormal), reinterpret_cast<const D3DXVECTOR3*>(&mParticle[i].mPos));
        const float fShade = fDarkness
                             - (pLight->m_vDirection.y * vNormal.y + vNormal.x * pLight->m_vDirection.x
                                + pLight->m_vDirection.z * vNormal.z)
                                   * fDarknessRange * 0.5f;
        RedColorValue& color = mParticle[i].mColor;
        color.a = mAlpha;
        color.r = fShade;
        color.g = fShade;
        color.b = fShade;
        mParticle[i].mTime = rand.GetFloat() * 100.0f;
        mParticle[i].mTexture = (unsigned char)(i % 4);
        PblVector3& vPos = mParticle[i].mPos;
        vPos.x += vOffset.x;
        vPos.y += vOffset.y;
        vPos.z += vOffset.z;

        const float fHalfSize = mParticle[i].mSize * 0.5f;
        const float fLowX = mParticle[i].mPos.x - fHalfSize;
        vMin.x = Min(vMin.x, fLowX);
        const float fLowY = (mParticle[i].mPos.y - fHalfSize) * mHeightScale;
        vMin.y = Min(vMin.y, fLowY);
        const float fLowZ = mParticle[i].mPos.z - fHalfSize;
        vMin.z = Min(vMin.z, fLowZ);
        vMax.x = Max(vMax.x, fHalfSize + mParticle[i].mPos.x);
        vMax.y = Max(vMax.y, (fHalfSize + mParticle[i].mPos.y) * mHeightScale);
        vMax.z = Max(vMax.z, fHalfSize + mParticle[i].mPos.z);
    }

    mBoundingOffset = PblVector3((vMax.x + vMin.x) * 0.5f, (vMax.y + vMin.y) * 0.5f, (vMax.z + vMin.z) * 0.5f);
    mBoundingRadius = 0.0f;
    mBoundingBoxRadius = PblVector3(vMax.x - mBoundingOffset.x, vMax.y - mBoundingOffset.y,
                                    vMax.z - mBoundingOffset.z);

    for (int i = 0; i < mNumParticles; ++i)
    {
        const float fX = mParticle[i].mPos.x - mBoundingOffset.x;
        const float fY = mParticle[i].mPos.y * mHeightScale - mBoundingOffset.y;
        const float fZ = mParticle[i].mPos.z - mBoundingOffset.z;
        const float fHalfSize = mParticle[i].mSize * 0.5f;
        const float fDist = PblMath::Sqrt(fX * fX + fY * fY + fZ * fZ);
        const float fScaled = mHeightScale * fHalfSize;
        const float fRadius = fDist + Max(fHalfSize, fScaled);
        mBoundingRadius = Max(fRadius, mBoundingRadius);
    }

    ActivateThread(0, PER_FRAME);
    CreateRenderData();
}

// FUNCTION: SWBF2 0x0056f240
Entity* LeafPatchClass::Build(const EntityDesc& desc)
{
    if (!mParticle)
    {
        Init();
    }
    return new LeafPatch(this, desc.mMatrix);
}

// FUNCTION: SWBF2 0x0056f2d0
bool LeafPatchClass::Update(float fDeltaTime)
{
    for (int i = 0; i < mNumParticles; ++i)
    {
        mParticle[i].mTime += mWiggleSpeed * fDeltaTime;
        mParticle[i].mRotation = PblMath::Sin(mParticle[i].mTime) * mWiggleAmount;
    }
    return true;
}

class PblMatrix;
extern PblMatrix gMatrixIdentity;
extern NoLodData noloddata;

// FUNCTION: SWBF2 0x0056cf20
LeafPatch::LeafPatch(LeafPatchClass* pClass, const PblMatrix& matrix)
    : RedSceneObject(&noloddata)
    , CollisionObject(COLL_LEAFPATCH, 0, 0, 0)
{
    mClass = pClass;
    PblMatrix& rMatrix = mMatrix;
    rMatrix = gMatrixIdentity;

    rMatrix[1] = matrix[1];
    rMatrix.m[1][3] = 0.0f;

    rMatrix[0] = Normalize(CrossProduct(-mMatrix[2], mMatrix[1]));
    rMatrix.m[0][3] = 0.0f;

    rMatrix[2] = Normalize(CrossProduct(rMatrix[0], mMatrix[1]));
    rMatrix.m[2][3] = 0.0f;

    rMatrix.SetTranslation(matrix.GetTranslation());

    SetCollisionCenter(mMatrix.GetTranslation());
    SetCollisionRadius(mClass->mRadius);

    {
        PblVector3 vCenter;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vCenter), reinterpret_cast<const D3DXVECTOR3*>(&mClass->mBoundingOffset), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
        SetRenderPos(vCenter);
    }
    SetRenderRadius(mClass->mBoundingRadius);
    static_cast<RedSceneObject*>(this)->Activate();

    mLod = 0.0f;
    mNumFallingLeaves = 0;
    const float fShade = PblRandom::_Global.GetFloat() * 0.2f;
    const float fBlue = PblRandom::_Global.GetFloat() * 0.1f + fShade + 0.7f;
    const float fGreen = PblRandom::_Global.GetFloat() * 0.1f + fShade + 0.7f;
    const float fRed = PblRandom::_Global.GetFloat() * 0.1f + fShade + 0.7f;
    mColor.r = fRed;
    mColor.g = fGreen;
    mColor.b = fBlue;
    mColor.a = 1.0f;

    if (mClass->mType != 3 && mClass->mMaxFallingLeaves > 0)
    {
        CollisionManager::AddLeafPatchBody(this);
    }

    mMaxDistance = mClass->mMaxDistance;
    LeafPatchClass::sObjects.AddLast(&mObjectNode, this);
}

// FUNCTION: SWBF2 0x0056d430
LeafPatch::~LeafPatch()
{
}

// SYNTHETIC: SWBF2 0x0056f415
// ??_ELeafPatch@@WFI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0056f425
// ??_ELeafPatch@@WGA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0056d400
// LeafPatch::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056cef0
void LeafPatch::DeactivateLeaf()
{
    --mNumFallingLeaves;
}

// FUNCTION: SWBF2 0x0056cf00
const PblVector3& LeafPatch::GetPosition() const
{
    return mMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x0056cf10
const PblMatrix& LeafPatch::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x0056d4a0
void LeafPatch::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xE6915CF6)
    {
        sscanf(pszValue, "%f", &mMaxDistance);
    }
}

// FUNCTION: SWBF2 0x0056d6f0
bool LeafPatch::RayTest(const PblVector3& vStart, const PblVector3& vDir, float fMaxT)
{
    const float fRadiusSq = GetCollisionRadius() * GetCollisionRadius();
    const PblVector3 vEnd = vStart + vDir * fMaxT;
    const PblVector3& vCenter = GetCollisionCenter();
    float dx = vEnd.x - vCenter.x;
    float dy = vEnd.y - vCenter.y;
    float dz = vEnd.z - vCenter.z;
    if (dy * dy + dx * dx + dz * dz > fRadiusSq)
    {
        return false;
    }
    const PblVector3& vCenter2 = GetCollisionCenter();
    dx = vStart.x - vCenter2.x;
    dy = vStart.y - vCenter2.y;
    dz = vStart.z - vCenter2.z;
    if (fRadiusSq > dy * dy + dx * dx + dz * dz)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0056d4d0
float LeafPatch::RayHit(const PblVector3& vStart, const PblVector3& vDir, float fMaxT,
                        PblVector3* pNormal, int* pMaterial, bool)
{
    const PblMatrix inverse = mMatrix.GetInverseFast();
    PblVector3 vLocalStart;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vLocalStart), reinterpret_cast<const D3DXVECTOR3*>(&vStart), reinterpret_cast<const D3DXMATRIX*>(&inverse));
    PblVector3 vLocalDir;
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vLocalDir), reinterpret_cast<const D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXMATRIX*>(&inverse));
    if (0.0f >= mClass->mRadius || 0.0f >= mClass->mHeight)
    {
        return 1.0f;
    }
    vLocalStart.x /= mClass->mRadius;
    vLocalStart.y /= mClass->mHeight;
    vLocalStart.z /= mClass->mRadius;
    vLocalDir.x /= mClass->mRadius;
    vLocalDir.y /= mClass->mHeight;
    vLocalDir.z /= mClass->mRadius;
    const float fLength = (float)sqrt((double)(vLocalDir.x * vLocalDir.x + vLocalDir.y * vLocalDir.y +
                                               vLocalDir.z * vLocalDir.z));
    const float fInv = 1.0f / fLength;
    vLocalDir = vLocalDir * fInv;
    const float fHit = CollisionUtil::RayHitSphere(vLocalStart, vLocalDir, fLength * fMaxT,
                                                   PblVector3(0.0f, 0.0f, 0.0f), 1.0f, pNormal);
    if (pMaterial)
    {
        *pMaterial = 0;
    }
    if (pNormal)
    {
        pNormal->x *= mClass->mRadius;
        pNormal->y *= mClass->mHeight;
        pNormal->z *= mClass->mRadius;
        PblVector3 vNormal;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNormal), reinterpret_cast<const D3DXVECTOR3*>(pNormal));
        *pNormal = vNormal;
    }
    return fHit;
}

// FUNCTION: SWBF2 0x0056d800
void LeafPatch::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!RedSceneManager::InTransparentPhase())
    {
        RedSceneManager::AddTransparentObject(this);
        return;
    }
    if (!mClass->RenderMaxDist(mMatrix, fDist, uiFlags, FLRenderer::GetNearSceneFadeEnd(), mColor))
    {
        return;
    }
    if (mNumFallingLeaves >= mClass->mMaxFallingLeaves)
    {
        return;
    }
    int iLeavesLeft = mClass->mMaxFallingLeaves - mNumFallingLeaves;
    if (iLeavesLeft > 0)
    {
        do
        {
            FallingLeafParticle* pLeaf = LeafParticleManager::GetFreeLeaf();
            if (!pLeaf)
            {
                continue;
            }
            PblVector3 vPos = mMatrix.GetTranslation();
            const float fX = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mClass->mRadius;
            const float fY = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mClass->mHeight * mClass->mHeightScale;
            const float fZ = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mClass->mRadius;
            vPos.x += fX;
            vPos.y += fY;
            vPos.z += fZ;
            pLeaf->RespawnLeaf(vPos, this, LEAF_NONE);
            ++mNumFallingLeaves;
        } while (--iLeavesLeft != 0);
    }
}

// FUNCTION: SWBF2 0x0056f350
void LeafPatchClass::Reset()
{
    for (PblList<LeafPatchClass>::Itor it(sClasses); (LeafPatchClass*)it; ++it)
    {
        it->DestroyRenderData();
    }

    sClasses.RemoveAll();
    sObjects.RemoveAll();
}

// TEMPLATE: SWBF2 0x0056f3d0
// PblList<LeafPatchClass>::RemoveAll
template void PblList<LeafPatchClass>::RemoveAll();

// GLOBAL: SWBF2 0x007ebad8
PblList<LeafPatch> LeafPatchClass::sObjects;
