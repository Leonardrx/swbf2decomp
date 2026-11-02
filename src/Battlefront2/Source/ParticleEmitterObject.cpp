#include "ParticleEmitterObject.h"
#include "PebbleFL/Common/PblVector.h"
#include "ParticleEmitter.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblRandom.h"
#include "Particle.h"
#include "ParticleSystem.h"
#include "Battlefront2Lod.h"

#include <math.h>

static const float EPSILON = 0.0001f;

extern PblVector3 gVectorZero;

PblVector3 Min(const PblVector3& a, const PblVector3& b);
PblVector3 Max(const PblVector3& a, const PblVector3& b);

// GLOBAL: SWBF2 0x01eaed56
bool ParticleEmitterObject::sForceRegisterStep;



extern Particle sParticleCache[128];
extern ParticleEmitterObject sPEO;

namespace ParticleSystem
{
bool IsLodActive(int iLod);
}

// SYNTHETIC: SWBF2 0x00405cc0
// `dynamic initializer for 'ParticleEmitterObject::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769240
// `dynamic atexit destructor for 'ParticleEmitterObject::sMemoryPool''

// GLOBAL: SWBF2 0x01ef5090
// ParticleEmitterObject::sMemoryPool
MemoryPool ParticleEmitterObject::sMemoryPool("ParticleEmitterObject", sizeof(ParticleEmitterObject));

// FUNCTION: SWBF2 0x0060b910
ParticleEmitterInfoData* ParticleEmitterInfoData::Create()
{
    if (sMemoryPool.mUsed >= sMemoryPool.mCount)
    {
        return 0;
    }
    return new ParticleEmitterInfoData;
}

// SYNTHETIC: SWBF2 0x0060b960
// ParticleEmitterInfoData::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0060ba30
ParticleEmitterObject* ParticleEmitterObject::Create(const ParticleEmitterClass* pClass)
{
    if (sMemoryPool.mUsed >= sMemoryPool.mCount)
    {
        return 0;
    }
    if (NetGame::IsEnabled() && !NetGame::IsClient() &&
        !(NetGame::IsOneWorld() && !(NetGame::IsHost() && NetGame::IsDedicated())))
    {
        return 0;
    }
    return new ParticleEmitterObject(pClass);
}

// FUNCTION: SWBF2 0x0060bb00
ParticleEmitterObject::ParticleEmitterObject(const ParticleEmitterClass* pClass)
    : RedSceneObject(pClass->mLodData)
{
    mLifeTime = 0.0f;
    mEmitter = ParticleEmitter::sMemoryPool.mUsed >= ParticleEmitter::sMemoryPool.mCount
                   ? 0
                   : new ParticleEmitter(pClass);
    mEmitterInfoData = 0;
    ActivateThread(0, PER_FRAME);
    mRegisterTime = 0.0f;
    mStopFlag = false;
    mIsChildPEO = false;
    if (mEmitter)
    {
        mEmitterBoundingRadius = pClass->mBoundingRadius;
        _pLodData = mEmitter->mClass->mLodData;
    }
    m_uiAttachFlags |= UPDATE_REQUIRED;
    bIsEmitterEmitter = false;
    mbAddedToScene = true;
}

// SYNTHETIC: SWBF2 0x0060b8f5
// ??_EParticleEmitterObject@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0060b8fd
// ??_EParticleEmitterObject@@WIA@AEPAXI@Z
// FUNCTION: SWBF2 0x0060bc60
ParticleEmitterObject::~ParticleEmitterObject()
{
    delete mEmitter;
    mEmitter = 0;
}

// FUNCTION: SWBF2 0x0060be10
void ParticleEmitterObject::StopAndFinish()
{
    mStopFlag = true;
    if (mEmitter)
    {
        mEmitter->StopAndFinish(mLifeTime);
    }
}

// FUNCTION: SWBF2 0x0060be40
void ParticleEmitterObject::StopNow()
{
    while (mEmitter)
    {
        mEmitter = mEmitter->DeleteAndReturnNext();
    }
}

// FUNCTION: SWBF2 0x006371c0
void ParticleEmitterObject::PermaRemoveParticleEffect()
{
    RedSceneObject::RemoveFromScene();
    mbAddedToScene = false;
}

// FUNCTION: SWBF2 0x0060c400
bool ParticleEmitterObject::IsEffectActive() const
{
    return mEmitter != 0;
}

// FUNCTION: SWBF2 0x0060c410
void ParticleEmitterObject::ActivateEffect()
{
    if (mEmitter)
    {
        FLEffectObject::ActivateEffect();
    }
}

// FUNCTION: SWBF2 0x0060c420
void ParticleEmitterObject::DeactivateEffect()
{
    if (mEmitter)
    {
        StopAndFinish();
        return;
    }
    m_uiAttachFlags &= ~UPDATE_REQUIRED;
    FLEffectObject::DeactivateEffect();
}

// FUNCTION: SWBF2 0x0060c690
void ParticleEmitterObject::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    Render_(ucViewport, fDist, uiFlags);
}

// FUNCTION: SWBF2 0x0060bf00
void ParticleEmitterObject::RegisterStep(float fDeltaTime)
{
    if (!mEmitterInfoData)
    {
        mEmitterInfoData = ParticleEmitterInfoData::sMemoryPool.mUsed >= ParticleEmitterInfoData::sMemoryPool.mCount
                               ? 0
                               : new ParticleEmitterInfoData;
        if (!mEmitterInfoData)
        {
            return;
        }
        mEmitterInfoData->mPrev = 0;
        mEmitterInfoData->mNext = 0;
        mEmitterInfoData->mStartTime = 0.0f;
        mEmitterInfoData->mLifeTime = 0.0f;
        mEmitterInfoData->mVelocity = gVectorZero;
        mEmitterInfoData->mInitialVelocity = gVectorZero;
        mEmitterInfoData->mMatrix = m_mWorldMatrix;
        mEmitter->Register(mEmitterInfoData);
        SetRenderPos(m_mWorldMatrix.GetTranslation());
        SetRenderRadius(mEmitterBoundingRadius);
        if (!bIsEmitterEmitter && mbAddedToScene)
        {
            AddToScene();
        }
        mEmitterInfoData->Release();
        return;
    }

    const PblMatrix mWorld = m_mWorldMatrix;
    ParticleEmitterInfoData* pPrev = mEmitterInfoData->mPrev;
    const PblVector3& vPos = mWorld.GetTranslation();
    const PblVector3& vLast = mEmitterInfoData->mMatrix.GetTranslation();
    const float fTime = mLifeTime - mEmitterInfoData->mStartTime;
    const float fDX = vPos.x - vLast.x;
    const float fDY = vPos.y - vLast.y;
    const float fDZ = vPos.z - vLast.z;
    PblVector3 vVelocity;
    if (fTime > 0.0f)
    {
        const float fInv = 1.0f / fTime;
        vVelocity = PblVector3(fDX * fInv, fDY * fInv, fDZ * fInv);
    }
    else
    {
        vVelocity = PblVector3(0.0f, 0.0f, 0.0f);
    }

    ParticleEmitterInfoData* pNew = 0;
    bool bRegister = false;
    mRegisterTime -= fDeltaTime;
    if (!pPrev || mRegisterTime < 0.0f)
    {
        mRegisterTime = 0.1f;
        pNew = ParticleEmitterInfoData::Create();
        if (pNew)
        {
            pNew->mStartTime = mLifeTime;
            pNew->mVelocity = gVectorZero;
            bRegister = sForceRegisterStep || !pPrev;
            if (!bRegister)
            {
                const float fVX = vVelocity.x - pPrev->mInitialVelocity.x;
                const float fVY = vVelocity.y - pPrev->mInitialVelocity.y;
                const float fVZ = vVelocity.z - pPrev->mInitialVelocity.z;
                bRegister = fVY * fVY + fVX * fVX + fVZ * fVZ > 0.5f ||
                            (fTime == 0.0f && (fDX != 0.0f || fDY != 0.0f || fDZ != 0.0f));
            }
        }
    }

    if (bRegister)
    {
        pNew->mMatrix = mWorld;
        pNew->mLifeTime = 0.0f;
        ParticleEmitterInfoData* pCur = mEmitterInfoData;
        if (pCur->mNext)
        {
            pCur->mNext->mPrev = pNew;
        }
        pNew->mNext = pCur->mNext;
        pNew->mPrev = pCur;
        pCur->mNext = pNew;
        mEmitterInfoData->mInitialVelocity = vVelocity;
        mEmitterInfoData->mVelocity = mEmitterInfoData->mInitialVelocity;
        mEmitterInfoData->mLifeTime = fTime;
        mEmitterInfoData = pNew;
        mEmitter->Register(pNew);
    }
    else if (pPrev)
    {
        pPrev->mLifeTime += fTime;
        mEmitterInfoData->mMatrix = mWorld;
        mEmitterInfoData->mStartTime = mLifeTime;
        const PblVector3& vNow = mEmitterInfoData->mMatrix.GetTranslation();
        const PblVector3& vThen = pPrev->mMatrix.GetTranslation();
        const float fInv = 1.0f / (pPrev->mLifeTime + EPSILON);
        pPrev->mVelocity = PblVector3((vNow.x - vThen.x) * fInv, (vNow.y - vThen.y) * fInv,
                                      (vNow.z - vThen.z) * fInv);
    }
    if (pNew)
    {
        pNew->Release();
    }
}

// FUNCTION: SWBF2 0x0060c440
bool ParticleEmitterObject::Update(float fDeltaTime)
{
    if (!mEmitter)
    {
        m_uiAttachFlags &= ~UPDATE_REQUIRED;
        FLEffectObject::DeactivateEffect();
        return false;
    }

    mLifeTime += fDeltaTime;
    if (FLEffectObject::Update(fDeltaTime))
    {
        if (!mStopFlag)
        {
            RegisterStep(fDeltaTime);
        }
        if (!mEmitter->Update(mLifeTime))
        {
            mEmitter = mEmitter->DeleteAndReturnNext();
            if (!mEmitter)
            {
                return false;
            }
        }
        if (mEmitterInfoData)
        {
            PblVector3 vMin = mEmitterInfoData->mMatrix.GetTranslation();
            PblVector3 vMax = vMin;
            for (ParticleEmitterInfoData* p = mEmitterInfoData->mPrev; p; p = p->mPrev)
            {
                vMin = Min(vMin, p->mMatrix.GetTranslation());
                vMax = Max(vMax, p->mMatrix.GetTranslation());
            }
            mMidPos = (vMax + vMin) * 0.5f;
            mMidSize.x = (float)fabs((double)(mMidPos.x - vMin.x));
            mMidSize.y = (float)fabs((double)(mMidPos.y - vMin.y));
            mMidSize.z = (float)fabs((double)(mMidPos.z - vMin.z));
            SetRenderPos(mMidPos);
            float fRadius = mMidSize.x > mMidSize.y ? mMidSize.x : mMidSize.y;
            fRadius = fRadius > mMidSize.z ? fRadius : mMidSize.z;
            SetRenderRadius(mEmitterBoundingRadius + fRadius);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0060be70
void ParticleEmitterObject::SetEmitterColor(const RedColorValue& color)
{
    if (mEmitter)
    {
        mEmitter->SetColor(color);
    }
}

// FUNCTION: SWBF2 0x0060bec0
void ParticleEmitterObject::ScaleSet(float fScale)
{
    if (mEmitter)
    {
        mEmitter->ScaleSet(fScale);
    }
}

// FUNCTION: SWBF2 0x0060bd00
void ParticleEmitterObject::Reset(const ParticleEmitterClass* pClass, unsigned int uSeed)
{
    mLifeTime = 0.0f;
    if (mEmitter)
    {
        mEmitter->Reset(pClass);
    }
    else
    {
        mEmitter = ParticleEmitter::sMemoryPool.mUsed >= ParticleEmitter::sMemoryPool.mCount
                       ? 0
                       : new ParticleEmitter(pClass);
    }
    mEmitterInfoData = 0;
    mRegisterTime = 0.0f;
    mStopFlag = false;
    mIsChildPEO = false;
    if (mEmitter)
    {
        mEmitter->SetSpawnerSeed(uSeed);
        mEmitterBoundingRadius = pClass->mBoundingRadius;
        _pLodData = mEmitter->mClass->mLodData;
    }
    m_uiAttachFlags |= UPDATE_REQUIRED;
    bIsEmitterEmitter = true;
}

// FUNCTION: SWBF2 0x0060be00
void ParticleEmitterObject::SetChildFlag()
{
    mIsChildPEO = true;
}

// FUNCTION: SWBF2 0x0060c6b0
void ParticleEmitterObject::Render_(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!mEmitter || RedParticleRenderer::IsFull())
    {
        return;
    }

    const PblVector3 vOld = m_mWorldMatrix.GetTranslation();
    InterpolateWorldMatrix();
    const PblVector3 vNew = m_mWorldMatrix.GetTranslation();
    for (ParticleEmitterInfoData* p = mEmitter->mFirstInfo; p; p = p->mNext)
    {
        p->mMatrix.SetTranslation(PblVector3(p->mMatrix.m[3][0] + (vNew.x - vOld.x),
                                             p->mMatrix.m[3][1] + (vNew.y - vOld.y),
                                             p->mMatrix.m[3][2] + (vNew.z - vOld.z)));
    }

    if (!mIsChildPEO)
    {
        PblVector3 vMin = mMidPos - mMidSize;
        vMin.x -= 450.0f;
        vMin.y -= 450.0f;
        vMin.z -= 450.0f;
        PblVector3 vMax = mMidPos + mMidSize;
        vMax.x += 450.0f;
        vMax.y += 450.0f;
        vMax.z += 450.0f;
        if (!ParticleSystem::PrepareForRender(mEmitter, GetRenderPos(), ucViewport, fDist, uiFlags, _Sphere.w, vMin,
                                              vMax, GetLodData()->mMaxDist))
        {
            return;
        }
    }

    for (ParticleEmitter* pEmitter = mEmitter; pEmitter; pEmitter = pEmitter->mNext)
    {
        pEmitter->Render(mLifeTime);
    }

    if (!mIsChildPEO)
    {
        ParticleSystem::FlushParticleCache();
        RedParticleRenderer::RenderAll();
    }
}

// FUNCTION: SWBF2 0x0060be90
void ParticleEmitterObject::ScaleMultiply(float fScale)
{
    if (mEmitter)
    {
        mEmitter->ScaleMultiply(fScale);
    }
}

// FUNCTION: SWBF2 0x0060bee0
void ParticleEmitterObject::SetInheritVelocity(float fVelocity)
{
    if (mEmitter)
    {
        mEmitter->SetInheritVelocityFactor(fVelocity);
    }
}

// FUNCTION: SWBF2 0x0060c8d0
void ParticleEmitterObject::DeleteEmitter()
{
    delete mEmitter;
    mEmitter = 0;
}
