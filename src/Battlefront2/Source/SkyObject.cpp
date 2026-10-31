#include <d3dx9math.h>
#include "SkyObject.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "PebbleFL/Common/PblRandom.h"

#include <math.h>


PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// GLOBAL: SWBF2 0x01eaf068
int SkyObjectManager::mNumObjectClasses;

// GLOBAL: SWBF2 0x01eaf06c
SkyObjectManager::SkyObjectClass* SkyObjectManager::mObjectClasses[10];

namespace SkyManager
{
void Activate();
}

// FUNCTION: SWBF2 0x00638d30
void SkyManager::Activate()
{
}

// FUNCTION: SWBF2 0x00638d40
void SkyObjectManager::Clear()
{
    mNumObjectClasses = 0;
}

// FUNCTION: SWBF2 0x00638d50
void SkyObjectManager::Load(PblConfig& config)
{
    mObjectClasses[mNumObjectClasses++] = new SkyObjectClass(config);
}

// FUNCTION: SWBF2 0x00638de0
void ReadVarianceData(PblConfig::Data& data, float& fMean, float& fVariance)
{
    fMean = data.GetFloatArg(0);
    if (data.GetNumArgs() > 1)
    {
        fVariance = data.GetFloatArg(1) - fMean;
    }
    else
    {
        fVariance = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00638e10
SkyObjectManager::SkyObjectClass::SkyObjectClass(PblConfig& config)
    : Renderable(0)
{
    mNumObjects = 1;
    mVelocityX = 0.0f;
    mVelocityVarianceX = 0.0f;
    mVelocityY = 0.0f;
    mVelocityVarianceY = 0.0f;
    mVelocityZ = 0.0f;
    mVelocityVarianceZ = 0.0f;
    mHeight = 70.0f;
    mHeightVariance = 30.0f;
    mDistance = 300.0f;
    mInDirectionFactor = 3.0f;
    mMidPos.x = 0.0f;
    mMidPos.y = 0.0f;
    mMidPos.z = 0.0f;
    mAcceleration.x = 0.0f;
    mAcceleration.y = 0.0f;
    mAcceleration.z = 0.0f;
    mMaxLifeTime = 1000.0f;
    mModel = 0;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0xB6C8887Eu:
            ReadVarianceData(data, mVelocityX, mVelocityVarianceX);
            break;
        case 0xB7C88A11u:
            ReadVarianceData(data, mVelocityY, mVelocityVarianceY);
            break;
        case 0xB4C88558u:
            ReadVarianceData(data, mVelocityZ, mVelocityVarianceZ);
            break;
        case 0xD5BDBB42u:
            ReadVarianceData(data, mHeight, mHeightVariance);
            break;
        case 0xC91C0E2Cu:
            mInDirectionFactor = data.GetFloatArg(0);
            break;
        case 0x2EB31462u:
            mDistance = data.GetFloatArg(0);
            break;
        case 0x2A6D8591u:
            mModel = RedModel::_HashTable.Find(PblHash(data.GetStringArg(0)));
            break;
        case 0x2FA0BA9Du:
            mAcceleration.x = data.GetFloatArg(0);
            mAcceleration.y = data.GetFloatArg(1);
            mAcceleration.z = data.GetFloatArg(2);
            break;
        case 0x307926C1u:
            mNumObjects = (int)data.GetFloatArg(0);
            break;
        case 0x5212ABEEu:
            mMaxLifeTime = data.GetFloatArg(0);
            break;
        }
    }

    if (!mModel)
    {
        mNumObjects = 0;
        mObjects = 0;
    }
    else
    {
        mObjects = new SkyObject[mNumObjects];
    }

    float fMinDist = mDistance + 10.0f;
    mMaxDist = (mMaxDist > fMinDist) ? mMaxDist : fMinDist;

    for (int i = 0; i < mNumObjects; i++)
    {
        Spawn(i);
    }

    if (mNumObjects > 0)
    {
        Activate(0);
        ActivateThread(0, PER_FRAME);
    }
}

// SYNTHETIC: SWBF2 0x00639110
// SkyObjectManager::SkyObjectClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00639170
bool SkyObjectManager::SkyObjectClass::Update(float fDeltaTime)
{
    for (int i = 0; i < mNumObjects; i++)
    {
        if (mObjects[i].mAlpha >= 0.0f)
        {
            float fDX = mObjects[i].mMatrix.m[3][0] - mMidPos.x;
            float fDZ = mObjects[i].mMatrix.m[3][2] - mMidPos.z;
            if (mMaxDist * mMaxDist < fDX * fDX + fDZ * fDZ || mObjects[i].mLifeTime > mMaxLifeTime)
            {
                mObjects[i].mAlpha = -1.0f;
            }
        }
        if (!mObjects[i].Update(fDeltaTime))
        {
            Spawn(i);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00639220
void SkyObjectManager::SkyObjectClass::Render(int uiFlags)
{
    uiFlags |= 0x200;
    RedRenderer::pcFlush(0);
    bool bFogEnable = RedRenderer::GetFogEnable();
    RedRenderer::SetFogEnable(false);
    for (int i = 0; i < mNumObjects; i++)
    {
        SkyObject& obj = mObjects[i];
        RedColor color;
        color.b = 0xFF;
        color.g = 0xFF;
        color.r = 0xFF;
        color.a = (unsigned char)(int)(fabsf(obj.mAlpha) * 255.0f);
        if (mModel)
        {
            mModel->Render(&obj.mMatrix, 0, color, uiFlags | 0x20001, 0);
        }
    }
    RedRenderer::pcFlush(0);
    RedRenderer::SetFogEnable(bFogEnable);
}

// FUNCTION: SWBF2 0x006392e0
bool SkyObjectManager::SkyObjectClass::SkyObject::Update(float fDeltaTime)
{
    float fOldAlpha = mAlpha;
    mLifeTime = fDeltaTime + mLifeTime;
    float fAlpha = fDeltaTime * 0.25f + fOldAlpha;
    mAlpha = (fAlpha < 1.0f) ? fAlpha : 1.0f;
    if (mAlpha >= 0.0f && fOldAlpha < 0.0f)
    {
        return false;
    }

    mVelocity += mAcceleration * fDeltaTime;

    float fX = fDeltaTime * mVelocity.x;
    float fY = mVelocity.y * fDeltaTime;
    float fZ = mVelocity.z * fDeltaTime;
    *(PblVector3*)&mMatrix.m[3][0] = mMatrix.GetTranslation() + mMatrix.GetXAxis() * fX + mMatrix.GetYAxis() * fY + mMatrix.GetZAxis() * fZ;
    mMatrix.m[3][3] = 1.0f;
    return true;
}

// FUNCTION: SWBF2 0x00639440
void SkyObjectManager::SkyObjectClass::Spawn(int i)
{
    mObjects[i].mAlpha = 0.0f;

    PblVector3 vDir;
    vDir.y = 0.0f;
    do
    {
        vDir.z = PblRandom::_Global.GetFloat() - 0.5f;
        vDir.x = PblRandom::_Global.GetFloat() - 0.5f;
    } while (vDir.x == 0.0f && vDir.z == 0.0f);
    PblVector3 vNorm = Normalize(vDir);
    vDir = vNorm;

    PblVector3 vHeading(vDir.z - vDir.y * 0.0f, vDir.x * 0.0f - vDir.z * 0.0f, vDir.y * 0.0f - vDir.x);
    float fHeight = PblRandom::_Global.GetFloat() * mHeightVariance + mHeight;
    PblVector3 vPos(mMidPos.x + mDistance * vDir.x, mMidPos.y + fHeight, mMidPos.z + mDistance * vDir.z);
    if (PblRandom::_Global.GetFloat() > 0.5f)
    {
        vHeading = PblVector3(-vHeading.x, -vHeading.y, -vHeading.z);
    }
    if (mInDirectionFactor > 0.0f)
    {
        float fIn = PblRandom::_Global.GetFloat() * mInDirectionFactor * 0.5f + mInDirectionFactor * 0.5f;
        vHeading.x += -vNorm.x * fIn;
        vHeading.y += -vNorm.y * fIn;
        vHeading.z += -vNorm.z * fIn;
    }
    PblVector3 vNorm2 = Normalize(vDir);
    vDir = vNorm2;

    mObjects[i].mMatrix = DirectionMatrix(vHeading, PblVector3(0.0f, 1.0f, 0.0f), vPos);
    mObjects[i].mVelocity = PblVector3(PblRandom::_Global.GetFloat() * mVelocityVarianceX + mVelocityX,
                               PblRandom::_Global.GetFloat() * mVelocityVarianceY + mVelocityY,
                               PblRandom::_Global.GetFloat() * mVelocityVarianceZ + mVelocityZ);
    mObjects[i].mAcceleration = mAcceleration;
    mObjects[i].mLifeTime = (PblRandom::_Global.GetFloat() * 0.2f + 0.8f) * mMaxLifeTime;
}
