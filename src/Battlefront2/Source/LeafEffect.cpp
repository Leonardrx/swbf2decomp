#include <math.h>
#include <stddef.h>
#include <d3dx9math.h>

#include "LeafEffect.h"
#include "Battlefront2Lod.h"
#include "NetGame.h"
#include "CameraManager.h"
#include "WeatherEffect.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"

#include "RedEngineFL/Application/RedTerrain.h"

extern NoLodData noloddata;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static const float EPSILON = 0.0001f;

class GameObject;
class CollisionManager
{
public:
    static int GetObjectsInRect(const PblVector3& vMin, const PblVector3& vMax, GameObject** const ppObjects,
                                int iMaxObjects, const GameObject* pIgnore, int iMask);
};



// FUNCTION: SWBF2 0x0056b370
FallingLeafParticle::FallingLeafParticle()
    : RedSceneObject(&noloddata)
{
    mIsValid = false;
    mTextureHash = 0x2648CD55u;
    PlatformInit();
}

// SYNTHETIC: SWBF2 0x0056b3d0
// FallingLeafParticle::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056b400
FallingLeafParticle::~FallingLeafParticle()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x0056b450
void FallingLeafParticle::RespawnLeaf(const PblVector3& vPos, LeafSpawner* pSpawner, LeafState eState)
{
    mRenderTimer = 0.0f;
    mGroundHeight = RedTerrain::GetHoleHeight(vPos);
    mVelocity.SetAll(0.0f);
    mWindVelocity.SetAll(0.0f);
    mConstantWindVelocity.SetAll(0.0f);

    PblQuaternion q = PblQuaternion(gMatrixIdentity);
    PblVector3 vOmega(0.0f, PblRandom::_Global.GetFloat() * 6.2831855f, 0.0f);
    float fAngle = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
    }

    PblMatrix mNew(q, vPos);
    mMatrix = mNew;
    mInitMatrix = mNew;
    mIsValid = true;

    SetRenderRadius(0.125f);
    SetRenderPos(vPos);
    Activate();

    mFallTime = PblRandom::_Global.GetFloat() * 6.2831855f;
    mRandomness = PblRandom::_Global.GetFloat();
    if (eState == LEAF_NONE)
    {
        eState = (PblRandom::_Global.GetFloat() < 0.2f) ? LEAF_TUMBLE : LEAF_PERIODIC_FLUTTER;
    }
    mState = eState;
    mSpawner = pSpawner;

    LeafParticleManager::sInstance->mLeafFallSound.Play(&vPos, NULL, false, true);
}

// FUNCTION: SWBF2 0x0056b810
void FallingLeafParticle::SetConstantWindVelocity(const PblVector3& vVelocity)
{
    mConstantWindVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x0056b830
void FallingLeafParticle::DoFlutterUpdate(float fDeltaTime)
{

    // GLOBAL: SWBF2 0x01faaa58
    static PblVector3 sFlutterSinkVelocity(0.0f, -0.5f, 0.0f);

    // GLOBAL: SWBF2 0x01faaa68
    static float sFlutterMaxRockAngle = PblMath::fPi * 0.25f;

    float fSin = PblMath::Sin(mFallTime * 2.0f);
    PblVector3 vPos = mMatrix.GetTranslation();

    PblQuaternion q = PblQuaternion(mInitMatrix);
    float fNegSin = -fSin;
    PblVector3 vOmega(0.0f, 0.0f, -(mRandomness * sFlutterMaxRockAngle * fNegSin));
    float fAngle = Length(vOmega);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
    }
    if (mRandomness < 0.15f)
    {
        q = q * PblQuaternion(PblVector3(0.0f, 1.0f, 0.0f), (mRandomness - 1.0f) * 10.0f * mFallTime);
    }

    mMatrix = PblMatrix(q, vPos);

    float fSlide = fSin * -1.8f;
    PblVector3 vSlide(mMatrix.m[0][0] * fSlide, mMatrix.m[0][1] * fSlide, mMatrix.m[0][2] * fSlide);
    mVelocity = Interpolate(sFlutterSinkVelocity, vSlide, (float)fabs((double)(mRandomness * fSin)));

    mMatrix.SetTranslation(mVelocity * fDeltaTime + vPos);
}

// FUNCTION: SWBF2 0x0056bc10
void FallingLeafParticle::DoTumbleUpdate(float fDeltaTime)
{
    PblVector3 vPos = mMatrix.GetTranslation();

    PblQuaternion q = PblQuaternion(mInitMatrix);
    PblVector3 vOmega((mRandomness + 2.0f) * 6.2831855f * mFallTime, 0.0f, 0.0f);
    float fAngle = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
    }

    PblMatrix mNew(q, vPos);
    mMatrix = mNew;

    float fSpeed = mRandomness * 0.3f + 0.3f;
    vPos.x -= 0.0f;
    vPos.y -= (mRandomness + 1.0f) * fDeltaTime;
    vPos.z -= 0.0f;
    mMatrix.SetTranslation(PblVector3(mInitMatrix.m[2][0] * fSpeed * fDeltaTime + vPos.x,
                                      mInitMatrix.m[2][1] * fSpeed * fDeltaTime + vPos.y,
                                      mInitMatrix.m[2][2] * fSpeed * fDeltaTime + vPos.z));
    mMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0056be80
void FallingLeafParticle::DoGroundedUpdate(float)
{
    PblVector3 vPos = mMatrix.GetTranslation();
    mGroundHeight = RedTerrain::GetHoleHeight(vPos);
    vPos.y = mGroundHeight + 0.1f;
    *(PblVector3*)mMatrix.m[3] = vPos;
    mMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0056bf10
bool FallingLeafParticle::Update(float fDeltaTime)
{
    static const float kTwoPi = 6.2831855f;

    if (!mIsValid)
    {
        return true;
    }

    mRenderTimer += fDeltaTime;
    if (mState == LEAF_PERIODIC_FLUTTER)
    {
        DoFlutterUpdate(fDeltaTime);
    }
    else if (mState == LEAF_TUMBLE)
    {
        DoTumbleUpdate(fDeltaTime);
    }

    mFallTime += fDeltaTime;
    if (mFallTime > kTwoPi)
    {
        mFallTime -= kTwoPi;
    }

    PblVector3 vPos = mMatrix.GetTranslation();
    if (Length(mWindVelocity + mConstantWindVelocity) > 0.1f)
    {
        PblQuaternion q = PblQuaternion(mMatrix);

        PblVector3 vSwirl = (mWindVelocity + mConstantWindVelocity * 16.0f * (1.0f - mFallTime / kTwoPi)) * kTwoPi;

        PblVector3 vOmega(vSwirl.y * 0.0f - vSwirl.z,
                          vSwirl.z * 0.0f - vSwirl.x * 0.0f,
                          vSwirl.x - vSwirl.y * 0.0f);
        PblVector3 vLocal;
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vOmega, (const D3DXMATRIX*)&mMatrix.GetInverseFast());

        float fAngle = Length(vOmega);
        if (fAngle > EPSILON)
        {
            float fInv = 1.0f / fAngle;
            q = q * PblQuaternion(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
        }

        vPos += (mWindVelocity + mConstantWindVelocity) * fDeltaTime;

        PblMatrix mNew(q, vPos);
        mMatrix = mNew;

        mWindVelocity -= mWindVelocity * (fDeltaTime * 2.0f);
        if (Length(mWindVelocity) <= 0.1f)
        {
            mGroundHeight = RedTerrain::GetHoleHeight(vPos);
        }
    }

    if (mState == LEAF_GROUNDED)
    {
        DoGroundedUpdate(fDeltaTime);
        vPos = mMatrix.GetTranslation();
    }

    SetRenderPos(vPos);
    if (mGroundHeight - 0.5f > vPos.y || mRenderTimer > 3.0f)
    {
        Deactivate();
        mIsValid = false;
        if (mSpawner)
        {
            mSpawner->DeactivateLeaf();
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0056c470
void FallingLeafParticle::AddWindVelocity(const PblVector3& vVelocity)
{
    float fSpeed = PblMath::Sqrt(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z);
    mWindVelocity += vVelocity * 0.9f;
    mWindVelocity += PblVector3(0.0f, fSpeed * 0.4f, 0.0f);
}

// GLOBAL: SWBF2 0x01e573d0
LeafParticleManager* LeafParticleManager::sInstance;

// FUNCTION: SWBF2 0x0056c570
LeafParticleManager::LeafParticleManager()
    : Thread()
{
    mNextFreeLeaf = mLeafParticles;
    mGroundLeavesTimer = 0.0f;
}

// SYNTHETIC: SWBF2 0x0056c5f0
// LeafParticleManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056c620
LeafParticleManager::~LeafParticleManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x0056c690
bool LeafParticleManager::Create()
{
    if (sInstance)
    {
        return false;
    }
    sInstance = new LeafParticleManager;
    sInstance->ActivateThread(0, PER_FRAME);
    return true;
}

// FUNCTION: SWBF2 0x0056c710
bool LeafParticleManager::Destroy()
{
    if (sInstance)
    {
        delete sInstance;
        sInstance = NULL;
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0056c730
FallingLeafParticle* LeafParticleManager::GetFreeLeaf()
{
    if (!sInstance)
    {
        return NULL;
    }
    FallingLeafParticle* pLeaf = sInstance->mNextFreeLeaf;
    if (pLeaf)
    {
        for (int i = 0; i < 100; ++i)
        {
            FallingLeafParticle* pOther = &sInstance->mLeafParticles[i];
            if (!pOther->IsValid() && pOther != pLeaf)
            {
                sInstance->mNextFreeLeaf = pOther;
            }
        }
    }
    return pLeaf;
}

// FUNCTION: SWBF2 0x0056c770
bool LeafParticleManager::CreateClient()
{
    if (sInstance)
    {
        for (int i = 0; i < 100; ++i)
        {
            if (sInstance->mLeafParticles[i].IsValid())
            {
                sInstance->mLeafParticles[i].Deactivate();
            }
        }
        NetGame::EnterClient();
        for (int i = 0; i < 100; ++i)
        {
            if (sInstance->mLeafParticles[i].IsValid())
            {
                sInstance->mLeafParticles[i].Activate();
            }
        }
        NetGame::LeaveClient();

        sInstance->DeactivateThread();
        NetGame::EnterClient();
        sInstance->ActivateThread(0, PER_FRAME);
        NetGame::LeaveClient();
    }
    return true;
}

// FUNCTION: SWBF2 0x0056c810
bool LeafParticleManager::Update(float fDeltaTime)
{
    for (int i = 0; i < 100; ++i)
    {
        mLeafParticles[i].Update(fDeltaTime);
        if (!mLeafParticles[i].IsValid())
        {
            mNextFreeLeaf = &mLeafParticles[i];
        }
    }

    mGroundLeavesTimer += fDeltaTime;
    if (mGroundLeavesTimer > 30.0f)
    {
        int iNumCameras = CameraManager::Instance()->GetNumCameras();
        for (int iCam = 0; iCam < iNumCameras; ++iCam)
        {
            PblMatrix mCamera = CameraManager::Instance()->GetCamera(iCam)->GetMatrix();
            const PblVector3& vDir = *(const PblVector3*)&mCamera.m[2][0];
            const PblVector3& vPos = *(const PblVector3*)&mCamera.m[3][0];

            PblVector3 vWind;
            D3DXVec3Normalize((D3DXVECTOR3*)&vWind, (const D3DXVECTOR3*)&FogCloudEffect::Instance()->GetVelocity());

            PblVector3 vBack(vPos.x - (vDir.x + vWind.x) * 30.0f + 0.0f,
                             vPos.y - (vDir.y + vWind.y) * 30.0f + 1024.0f,
                             vPos.z - (vDir.z + vWind.z) * 30.0f + 0.0f);
            PblVector3 vMin(vBack.x > vPos.x ? vPos.x : vBack.x,
                            vBack.y > vPos.y ? vPos.y : vBack.y,
                            vBack.z > vPos.z ? vPos.z : vBack.z);
            PblVector3 vMax(vPos.x > vBack.x ? vPos.x : vBack.x,
                            vPos.y > vBack.y ? vPos.y : vBack.y,
                            vPos.z > vBack.z ? vPos.z : vBack.z);

            GameObject* pObject;
            if (CollisionManager::GetObjectsInRect(vMin, vMax, &pObject, 1, NULL, 0x200) > 0)
            {
                for (int i = 0; i < 10; ++i)
                {
                    FallingLeafParticle* pLeaf = GetFreeLeaf();
                    if (pLeaf)
                    {
                        float fWindDist = PblRandom::GetGlobalFloatRange(30.0f, 30.0f);
                        float fDirDist = PblRandom::GetGlobalFloatRange(30.0f, 30.0f);
                        PblVector3 vSpawn;
                        vSpawn.x = vPos.x - vDir.x * fDirDist - fWindDist * vWind.x;
                        vSpawn.z = vPos.z - vDir.z * fDirDist - vWind.z * fWindDist;
                        vSpawn.y = RedTerrain::GetInterpolatedHeight(vSpawn.x, vSpawn.z);
                        PblVector3 vDrop(vSpawn.x + 0.0f,
                                         PblRandom::_Global.GetFloat() + 1.0f + vSpawn.y,
                                         vSpawn.z + 0.0f);
                        pLeaf->RespawnLeaf(vDrop, NULL, LEAF_GROUNDED);
                        pLeaf->AddWindVelocity(PblVector3(vWind.x * 5.0f, vWind.y * 5.0f, vWind.z * 5.0f));
                        pLeaf->SetConstantWindVelocity(PblVector3(vWind.x * 5.0f, vWind.y * 5.0f, vWind.z * 5.0f));
                    }
                    pLeaf = GetFreeLeaf();
                    if (pLeaf)
                    {
                        float fWindDist = PblRandom::GetGlobalFloatRange(30.0f, 30.0f);
                        float fDirDist = PblRandom::GetGlobalFloatRange(30.0f, 30.0f);
                        PblVector3 vSpawn;
                        vSpawn.x = vPos.x - vDir.x * fDirDist - fWindDist * vWind.x;
                        vSpawn.z = vPos.z - vDir.z * fDirDist - vWind.z * fWindDist;
                        vSpawn.y = RedTerrain::GetInterpolatedHeight(vSpawn.x, vSpawn.z);
                        PblVector3 vDrop(vSpawn.x + 0.0f,
                                         PblRandom::_Global.GetFloat() + 1.0f + vSpawn.y,
                                         vSpawn.z + 0.0f);
                        pLeaf->RespawnLeaf(vDrop, NULL, LEAF_GROUNDED);
                        pLeaf->AddWindVelocity(PblVector3(vWind.x * 5.0f, vWind.y * 5.0f, vWind.z * 5.0f));
                        pLeaf->SetConstantWindVelocity(PblVector3(vWind.x * 5.0f, vWind.y * 5.0f, vWind.z * 5.0f));
                    }
                }
                mGroundLeavesTimer = 0.0f;
            }
        }
    }
    return true;
}
