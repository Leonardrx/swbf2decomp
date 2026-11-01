#include <math.h>
#include <stddef.h>
#include <string.h>
#include <d3dx9math.h>

#include "FishEffect.h"
#include "Battlefront2Lod.h"
#include "CameraManager.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblHashTable.h"

#include "RedEngineFL/Application/RedTerrain.h"

extern NoLodData noloddata;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static const float EPSILON = 0.0001f;
static const float fHalfPi = PblMath::fPi * 0.5f;





// FUNCTION: SWBF2 0x00522380
FishParticle::FishParticle()
    : RedSceneObject(&noloddata)
{
    mIsValid = false;
}

// SYNTHETIC: SWBF2 0x005223a0
// FishParticle::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005223d0
FishParticle::~FishParticle()
{
}

// FUNCTION: SWBF2 0x005223e0
void FishParticle::RespawnFish(const PblVector3& vPos, const FishParticleType& type, FishState eState)
{
    mType = type;

    PblQuaternion q = PblQuaternion(gMatrixIdentity);
    {
        PblVector3 initomega = PblVector3(0.0f, PblRandom::_Global.GetFloat() * PblMath::fTwoPi, 0.0f);
        float initAngle = Length(initomega);
        if (initAngle > EPSILON)
        {
            q *= PblQuaternion(initomega / initAngle, initAngle);
        }
    }
    mMatrix = PblMatrix(q, vPos);
    mState = eState;
    mBaseMatrix = mMatrix;
    mPhase = PblRandom::_Global.GetFloat() * PblMath::fTwoPi;

    if (mState == FISH_FLOAT)
    {
        q = PblQuaternion(mMatrix);
        {
            PblVector3 initomega = PblVector3(0.0f, 0.0f, fHalfPi);
            float initAngle = Length(initomega);
            if (initAngle > EPSILON)
            {
                q *= PblQuaternion(initomega / initAngle, initAngle);
            }
        }
        mMatrix = PblMatrix(q, vPos);
        mBaseMatrix = mMatrix;
        mVelocity = PblVector3(0.0f, PblRandom::_Global.GetFloat() * 0.15f + 0.15f, 0.0f);
    }
    else if (mState == FISH_SWIM)
    {
        mVelocity = mBaseMatrix.GetZAxis() * 3.0f;
    }

    mIsValid = true;
    mUnderwater = true;
    mTimeOffCamera = 0.0f;
    mNextTurnAngle = 0.0f;
    mTotalStraightTime = 3.0f;
    SetRenderRadius(mType.mSize);
    SetRenderPos(vPos);
    Activate();
}

// FUNCTION: SWBF2 0x005228f0
bool FishParticle::UpdateSwimmingFish(float fDeltaTime)
{
    PblVector3 vPos(mBaseMatrix.m[3][0], mBaseMatrix.m[3][1], mBaseMatrix.m[3][2]);
    PblVector3 vHead(vPos.x + mBaseMatrix.m[2][0] * mType.mSize,
                     vPos.y + mBaseMatrix.m[2][1] * mType.mSize,
                     vPos.z + mBaseMatrix.m[2][2] * mType.mSize);

    mPhase += fDeltaTime * 12.0f;
    if (mPhase > PblMath::fTwoPi)
    {
        mPhase -= PblMath::fTwoPi;
    }

    float fWaterHeight;
    bool bWater = RedWater::GetWaterHeight(vHead, fWaterHeight);
    float fGroundHeight = RedTerrain::GetInterpolatedHeight(vHead);
    float fHalfSize = mType.mSize * 0.5f;
    float fAboveGround = vHead.y - fHalfSize - fGroundHeight;
    float fBelowWater = fWaterHeight - (vHead.y + fHalfSize);
    if (fAboveGround < 0.0f || fBelowWater < 0.0f)
    {
        vPos.y = (fGroundHeight + fWaterHeight) * 0.5f;
    }

    if ((fBelowWater + fAboveGround >= 0.0f && bWater) ||
        (fGroundHeight <= RedTerrain::GetInterpolatedHeight(vPos) && bWater))
    {
        mTotalStraightTime += fDeltaTime;
    }
    else
    {
        if (mTotalStraightTime > 2.0f)
        {
            mNextTurnAngle = PblRandom::GetGlobalFloat() * 0.375f * fHalfPi + fHalfPi * 0.125f;
            if (PblRandom::GetGlobalFloat() < 0.5f)
            {
                mNextTurnAngle = -mNextTurnAngle;
            }
        }

        PblQuaternion q = PblQuaternion(mBaseMatrix);
        PblVector3 vOmega(0.0f, mNextTurnAngle, 0.0f);
        float fAngle = Length(vOmega);
        if (fAngle > EPSILON)
        {
            mTotalStraightTime = 0.0f;
            q *= PblQuaternion(vOmega / fAngle, fAngle);
        }
        mBaseMatrix = PblMatrix(q, vPos);

        float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z);
        mVelocity = PblVector3(mBaseMatrix.m[2][0] * fSpeed, mBaseMatrix.m[2][1] * fSpeed, mBaseMatrix.m[2][2] * fSpeed);
    }

    vPos.x = mVelocity.x * fDeltaTime + vPos.x;
    vPos.y = mVelocity.y * fDeltaTime + vPos.y;
    vPos.z = mVelocity.z * fDeltaTime + vPos.z;
    mBaseMatrix.m[3][0] = vPos.x;
    mBaseMatrix.m[3][1] = vPos.y;
    mBaseMatrix.m[3][2] = vPos.z;
    mBaseMatrix.m[3][3] = 1.0f;
    mMatrix = mBaseMatrix;

    PblQuaternion q = PblQuaternion(mMatrix);
    {
        PblVector3 initomega = PblVector3(0.0f, PblMath::Sin(mPhase) * (fHalfPi * 0.25f), 0.0f);
        float initAngle = Length(initomega);
        if (initAngle > EPSILON)
        {
            q *= PblQuaternion(initomega / initAngle, initAngle);
        }
    }
    float fWag = PblMath::Sin(mPhase) * (mType.mSize * 0.07f);
    vPos.x = mBaseMatrix.m[0][0] * fWag + vPos.x;
    vPos.y = mBaseMatrix.m[0][1] * fWag + vPos.y;
    vPos.z = mBaseMatrix.m[0][2] * fWag + vPos.z;
    mMatrix = PblMatrix(q, vPos);

    SetRenderPos(vPos);
    return true;
}

// FUNCTION: SWBF2 0x00523090
bool FishParticle::UpdateFloatingFish(float fDeltaTime)
{
    PblVector3 vPos = mMatrix.GetTranslation();

    mPhase += fDeltaTime * 2.0f;
    if (mPhase > PblMath::fTwoPi)
    {
        mPhase -= PblMath::fTwoPi;
    }

    float fWaterHeight;
    PblVector3 vWaterVelocity;
    if (!RedWater::GetWaterHeightAndVelocity(vPos, fWaterHeight, vWaterVelocity))
    {
        return true;
    }

    if (mUnderwater && fWaterHeight > vPos.y)
    {
        vPos = vPos + mVelocity * fDeltaTime;
        mMatrix.SetTranslation(vPos);
        SetRenderPos(vPos);
        return true;
    }

    if (mUnderwater)
    {
        mUnderwater = false;
        mVelocity.x = 0.0f;
        mVelocity.y = 0.0f;
        mVelocity.z = 0.0f;
        mOmega.x = (0.5f - PblRandom::GetGlobalFloat()) * 0.125f;
        mOmega.y = 0.0f;
        mOmega.z = 0.0f;
    }

    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vWaterVelocity);
    PblVector3 vAhead = vDir * mType.mSize + vPos;
    float fAheadWater;
    if (!RedWater::GetWaterHeight(vAhead, fAheadWater) || RedTerrain::GetInterpolatedHeight(vAhead) > fAheadWater)
    {
        mIsValid = false;
        Deactivate();
        return true;
    }

    vPos.y = fWaterHeight;
    vPos.y = PblMath::Sin(mPhase) * 0.01f * mType.mSize + vPos.y;
    vPos = vWaterVelocity * fDeltaTime + vPos;

    PblQuaternion q(mMatrix);
    float initAngle = Length(mOmega);
    if (initAngle > EPSILON)
    {
        q *= PblQuaternion(mOmega / initAngle, initAngle);
    }
    mMatrix = PblMatrix(q, vPos);
    mBaseMatrix = mMatrix;

    SetRenderPos(vPos);
    return true;
}

// FUNCTION: SWBF2 0x00523540
bool FishParticle::UpdateJumpingFish(float)
{
    return true;
}

// FUNCTION: SWBF2 0x00523550
bool FishParticle::Update(float fDeltaTime)
{
    if (!mIsValid)
    {
        return true;
    }

    mTimeOffCamera += fDeltaTime;
    if (mTimeOffCamera > 5.0f)
    {
        mIsValid = false;
        Deactivate();
        return true;
    }

    switch (mState)
    {
    case FISH_SWIM:
        return UpdateSwimmingFish(fDeltaTime);
    case FISH_FLOAT:
        return UpdateFloatingFish(fDeltaTime);
    case FISH_JUMP:
        return UpdateJumpingFish(fDeltaTime);
    }
    mIsValid = false;
    Deactivate();
    return true;
}

// GLOBAL: SWBF2 0x01e55f14
int FishParticleManager::sNumFishTypes;

// GLOBAL: SWBF2 0x01e55f18
FishParticleManager* FishParticleManager::sInstance;

// GLOBAL: SWBF2 0x01e55f20
FishParticleType FishParticleManager::sFishTypes[3];

// FUNCTION: SWBF2 0x005235b0
FishParticleManager::FishParticleManager()
    : Thread()
{
    mNextFreeFish = mFishParticles;
}

// SYNTHETIC: SWBF2 0x00523620
// FishParticleManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00523650
FishParticleManager::~FishParticleManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x005236c0
void FishParticleManager::SetNumFishTypes(int iNumTypes)
{
    if (iNumTypes > 3)
    {
        sNumFishTypes = 3;
    }
    else
    {
        sNumFishTypes = iNumTypes < 0 ? 0 : iNumTypes;
    }
}

// FUNCTION: SWBF2 0x005236e0
void FishParticleManager::SetFishType(int iType, float fSize, const char* pcTexture)
{
    if (iType >= 0 && iType < sNumFishTypes)
    {
        sFishTypes[iType].mSize = fSize;
        sFishTypes[iType].mTexture = PblHash(pcTexture);
    }
}

// FUNCTION: SWBF2 0x00523720
bool FishParticleManager::Create()
{
    if (sInstance)
    {
        return false;
    }
    sInstance = new FishParticleManager;
    sInstance->ActivateThread(0, PER_FRAME);
    memset(sFishTypes, 0, sizeof(sFishTypes));
    sNumFishTypes = 0;
    return true;
}

// FUNCTION: SWBF2 0x005237d0
bool FishParticleManager::Destroy()
{
    if (sInstance)
    {
        PlatformCleanup();
        delete sInstance;
        sInstance = NULL;
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00523800
FishParticle* FishParticleManager::GetFreeFish()
{
    if (!sInstance)
    {
        return NULL;
    }
    FishParticle* pFish = sInstance->mNextFreeFish;
    if (pFish)
    {
        for (int i = 0; i < 20; ++i)
        {
            FishParticle* pOther = &sInstance->mFishParticles[i];
            if (!pOther->IsValid() && pOther != pFish)
            {
                sInstance->mNextFreeFish = pOther;
            }
        }
    }
    return pFish;
}

// FUNCTION: SWBF2 0x00523840
bool FishParticleManager::CreateClient()
{
    return true;
}

// FUNCTION: SWBF2 0x00523850
bool FishParticleManager::Update(float fDeltaTime)
{
    if (sNumFishTypes)
    {
        int iNumFree = 0;
        for (int i = 0; i < 20; ++i)
        {
            mFishParticles[i].Update(fDeltaTime);
            if (!mFishParticles[i].IsValid())
            {
                ++iNumFree;
                mNextFreeFish = &mFishParticles[i];
            }
        }

        if (iNumFree > 10)
        {
            unsigned int uiNumCameras = CameraManager::Instance()->GetNumCameras();
            int iCam = PblRandom::GetGlobalInt() % uiNumCameras;
            PblMatrix mCamera = CameraManager::Instance()->GetCamera(iCam)->GetMatrix();
            const PblVector3& vAt = mCamera.GetZAxis();
            const PblVector3& vCamPos = mCamera.GetTranslation();

            const PblVector3 vUp(0.0f, 1.0f, 0.0f);
            PblVector3 vRight = Normalize(CrossProduct(vUp, vAt));
            PblVector3 vBack = Normalize(CrossProduct(vUp, vRight));
            PblVector3 vSpawn = vCamPos + vBack * 35.0f;

            SpawnSwimmingFish(vSpawn, 10.0f, 0);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00523a40
void FishParticleManager::SpawnFloatingFish(const PblVector3& vPos, float fRadius, int iNumFish)
{
    if (!sInstance || !sNumFishTypes)
    {
        return;
    }

    for (int n = 0; n < iNumFish; ++n)
    {
        FishParticle* pFish = GetFreeFish();
        if (!pFish)
        {
            continue;
        }

        const float fRandZ = 0.5f - PblRandom::_Global.GetFloat();
        PblVector3 vRand(0.5f - PblRandom::_Global.GetFloat(), 0.0f, fRandZ);
        PblVector3 vDir;
        D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vRand);

        float fPick = PblRandom::_Global.GetFloat();
        float fStep = 1.0f / sNumFishTypes;
        float fTotal = 0.0f;
        for (int i = 1; i <= sNumFishTypes; ++i)
        {
            fTotal += fStep;
            if (fTotal >= fPick)
            {
                float fDist = PblRandom::_Global.GetFloat() * fRadius;
                PblVector3 vSpawn = vPos + vDir * fDist;
                pFish->RespawnFish(vSpawn, sFishTypes[i - 1], FISH_FLOAT);
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00523c80
void FishParticleManager::SpawnSwimmingFish(const PblVector3& vPos, float fRadius, int)
{
    if (!sInstance || !sNumFishTypes)
    {
        return;
    }
    FishParticle* pFish = GetFreeFish();
    if (!pFish)
    {
        return;
    }

    const float fRandZ = 0.5f - PblRandom::_Global.GetFloat();
    PblVector3 vRand(0.5f - PblRandom::_Global.GetFloat(), 0.0f, fRandZ);
    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vRand);

    float fPick = PblRandom::_Global.GetFloat();
    float fStep = 1.0f / sNumFishTypes;
    float fTotal = 0.0f;
    for (int i = 0; i < sNumFishTypes; ++i)
    {
        fTotal += fStep;
        if (fTotal >= fPick)
        {
            float fDist = PblRandom::_Global.GetFloat() * fRadius;
            PblVector3 vSpawn(vPos.x + vDir.x * fDist, vPos.y + vDir.y * fDist, vPos.z + vDir.z * fDist);
            float fWaterHeight;
            if (RedWater::GetWaterHeight(vSpawn, fWaterHeight))
            {
                float fGroundHeight = RedTerrain::GetInterpolatedHeight(vSpawn);
                if (fWaterHeight > fGroundHeight)
                {
                    vSpawn.y = (fGroundHeight + fWaterHeight) * 0.5f;
                    pFish->RespawnFish(vSpawn, sFishTypes[i], FISH_SWIM);
                    return;
                }
            }
        }
    }
}
