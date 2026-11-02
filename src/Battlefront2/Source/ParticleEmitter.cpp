#include "ParticleEmitter.h"
#include "Particle.h"
#include "ParticleEmitterObject.h"
#include "Battlefront2Lod.h"
#include "PebbleFL/Common/PblRandom.h"

extern NoLodData noloddata;

extern SmallEffectLodData smallEffectLodData;

extern Particle sParticleCache[128];
extern ParticleEmitterObject sPEO;

namespace ParticleSystem
{
bool IsLodActive(int iLod);
}

// SYNTHETIC: SWBF2 0x00405c60
// `dynamic initializer for 'ParticleEmitter::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769210
// `dynamic atexit destructor for 'ParticleEmitter::sMemoryPool''

// GLOBAL: SWBF2 0x01ef26c8
// ParticleEmitter::sMemoryPool
MemoryPool ParticleEmitter::sMemoryPool("ParticleEmitter", sizeof(ParticleEmitter));

// FUNCTION: SWBF2 0x00609ed0
ParticleEmitterClass::ParticleEmitterClass()
{
    mType = 1;
    mNext = 0;
    mLodData = &smallEffectLodData;
    mNoRegisterStep = false;
    mWorldAlignedFlag = false;
    mRed.Setup(1.0f);
    mGreen.Setup(1.0f);
    mBlue.Setup(1.0f);
    mAlpha.Setup(1.0f);
    mSize.Setup(1.0f);
    mNextSeed = 0;
    mMinDistLOD = 10.0f;
    mMaxDistLOD = 50.0f;
    mBoundingRadius = 5.0f;
}

// FUNCTION: SWBF2 0x00609fa0
ParticleEmitterClass::~ParticleEmitterClass()
{
    delete mNext;
}

// SYNTHETIC: SWBF2 0x00609ea0
// ParticleEmitterClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0060a010
FLEffectObject* ParticleEmitterClass::BuildEffect()
{
    ParticleEmitterObject* pObject = ParticleEmitterObject::Create(this);
    if (pObject)
    {
        pObject->ActivateEffect();
    }
    return pObject;
}

// FUNCTION: SWBF2 0x0060a030
ParticleEmitter::ParticleEmitter(const ParticleEmitterClass* pClass)
{
    mFirstInfo = 0;
    mNext = 0;
    mSoundStarted = 0;
    Reset(pClass);
}

// FUNCTION: SWBF2 0x00609d00
ParticleEmitter* ParticleEmitter::Create(const ParticleEmitterClass* pClass)
{
    if (ParticleEmitter::sMemoryPool.mUsed >= ParticleEmitter::sMemoryPool.mCount)
    {
        return 0;
    }
    return new ParticleEmitter(pClass);
}


// FUNCTION: SWBF2 0x0060a130
void ParticleEmitter::Reset(const ParticleEmitterClass* pClass)
{
    while (mFirstInfo)
    {
        ParticleEmitterInfoData* pNext = mFirstInfo->mNext;
        mFirstInfo->Release();
        mFirstInfo = pNext;
    }
    delete mNext;

    mClass = (ParticleEmitterClass*)pClass;
    unsigned int uSeed = pClass->mNextSeed;
    if (uSeed == 0)
    {
        uSeed = PblRandom::GetGlobalInt();
    }

    PblRandom rand(uSeed ^ 0xC71C2830u);
    mSeed = uSeed;
    mMaxParticles = (short)mClass->mMaxParticles.GetValue(rand);
    if (mMaxParticles > 128)
    {
        mMaxParticles = 128;
    }
    mSpawnDelay = mClass->mStartDelay.GetValue(rand);
    mFirstInfo = 0;
    mParticleCount = 0;
    mStopTime = 10000.0f;
    mColor.r = mClass->mRed.GetValue(rand);
    mColor.g = mClass->mGreen.GetValue(rand);
    mColor.b = mClass->mBlue.GetValue(rand);
    mColor.a = mClass->mAlpha.GetValue(rand);
    mSize = mClass->mSize.GetValue(rand);
    mInheritVelocityFactor = 0.0f;
    mSoundStarted = 0;

    if (pClass->mNext)
    {
        mNext = ParticleEmitter::Create(pClass->mNext);
    }
    else
    {
        mNext = 0;
    }

    mSpawnerSeed = PblRandom::GetGlobalInt();
}

// FUNCTION: SWBF2 0x0060a090
ParticleEmitter::~ParticleEmitter()
{
    while (mFirstInfo)
    {
        ParticleEmitterInfoData* pNext = mFirstInfo->mNext;
        mFirstInfo->Release();
        mFirstInfo = pNext;
    }

    delete mNext;
}

// FUNCTION: SWBF2 0x0060a530
void ParticleEmitter::StopAndFinish(float fStopTime)
{
    if (mStopTime > fStopTime)
    {
        mStopTime = fStopTime;
    }
    if (mNext)
    {
        mNext->StopAndFinish(fStopTime);
    }
    mAmbientSound.Stop(true);
}

// FUNCTION: SWBF2 0x0060a560
void ParticleEmitter::SetColor(const RedColorValue& color)
{
    mColor = color;
    if (mNext)
    {
        mNext->SetColor(color);
    }
}

// FUNCTION: SWBF2 0x0060a590
ParticleEmitter* ParticleEmitter::DeleteAndReturnNext()
{
    ParticleEmitter* pNext = mNext;
    mNext = 0;
    delete this;
    return pNext;
}

// FUNCTION: SWBF2 0x0060a5c0
bool ParticleEmitter::Update(float fTime)
{
    if (mNext && !mNext->Update(fTime))
    {
        mNext = mNext->DeleteAndReturnNext();
    }

    if (!mFirstInfo)
    {
        return true;
    }

    const float fLifeTime = mClass->mTransformer.mLifeTime;
    if (mFirstInfo->mStartTime + mSpawnDelay > mStopTime)
    {
        return false;
    }

    if (mAmbientSound.GetPlaying())
    {
        mAmbientSound.Update(&mClass->mAmbientSound, &mFirstInfo->mMatrix.GetTranslation(),
                             &mFirstInfo->mVelocity);
    }
    else if (!mSoundStarted)
    {
        mAmbientSound.Play(&mClass->mAmbientSound, &mFirstInfo->mMatrix.GetTranslation(),
                           &mFirstInfo->mVelocity, true);
        mSoundStarted = 1;
    }

    ParticleEmitterInfoData* pInfo = mFirstInfo;
    while (fTime > pInfo->mStartTime + mSpawnDelay + fLifeTime)
    {
        PblRandom rand(mSeed);
        rand.GetInt();
        mSeed = rand.GetInt();
        mSpawnDelay += mClass->mBurstDelay.GetValue(rand);
        int iCount = (int)(mClass->mBurstCount.GetValue(rand) + 0.5f);
        if (iCount > 128)
        {
            iCount = 128;
        }
        mParticleCount += (short)iCount;
        if (mMaxParticles != -1 && mParticleCount >= mMaxParticles)
        {
            return false;
        }

        while (mSpawnDelay > mFirstInfo->mLifeTime)
        {
            ParticleEmitterInfoData* pNext = mFirstInfo->mNext;
            if (!pNext)
            {
                break;
            }
            pNext->mPrev = mFirstInfo->mPrev;
            if (mFirstInfo->mPrev)
            {
                mFirstInfo->mPrev->mNext = pNext;
            }
            mSpawnDelay -= mFirstInfo->mLifeTime;
            mFirstInfo->Release();
            mFirstInfo = pNext;
        }
        pInfo = mFirstInfo;
    }
    return true;
}

// FUNCTION: SWBF2 0x0060a860
void ParticleEmitter::Render(float fTime)
{
    if (mClass->mGeometry.mType == ParticleGeometry::PT_EMITTER)
    {
        RenderAsEmitter(fTime);
        return;
    }
    if (!mFirstInfo)
    {
        return;
    }

    ParticleEmitterInfoData* pInfo = mFirstInfo;
    unsigned int uSeed = mSeed;
    float fDelay = mSpawnDelay;
    short sCount = mParticleCount;

    PblMatrix mat = pInfo->mMatrix;
    mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fDelay,
                                   mat.m[3][1] + pInfo->mVelocity.y * fDelay,
                                   mat.m[3][2] + pInfo->mVelocity.z * fDelay));
    while (fDelay > pInfo->mLifeTime && pInfo->mNext)
    {
        fDelay -= pInfo->mLifeTime;
        pInfo = pInfo->mNext;
        mat = pInfo->mMatrix;
        mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fDelay,
                                       mat.m[3][1] + pInfo->mVelocity.y * fDelay,
                                       mat.m[3][2] + pInfo->mVelocity.z * fDelay));
    }

    PblMatrix matFirst;
    if (mClass->mNoRegisterStep)
    {
        ParticleEmitterInfoData* pLast = mFirstInfo;
        while (pLast->mNext)
        {
            pLast = pLast->mNext;
        }
        matFirst = pLast->mMatrix;
    }
    else
    {
        matFirst = mat;
    }

    mClass->mGeometry.Begin();
    int iNum = 0;
    bool bLastBurst = false;
    for (;;)
    {
        const float fStart = pInfo->mStartTime + fDelay;
        if (fTime < fStart || !(mStopTime > fStart))
        {
            break;
        }
        if (mMaxParticles != -1 && sCount >= mMaxParticles)
        {
            break;
        }
        if (iNum >= 127)
        {
            break;
        }

        PblRandom rand(uSeed);
        rand.GetInt();
        uSeed = rand.GetInt();
        ParticleEmitterClass* const pClass = mClass;
        const float fBurstDelay = pClass->mBurstDelay.GetValue(rand);
        int iBurst = (int)(pClass->mBurstCount.GetValue(rand) + 0.5f);
        if (iBurst > 128)
        {
            iBurst = 128;
        }
        if (mMaxParticles != -1 && mMaxParticles - sCount < iBurst)
        {
            iBurst = mMaxParticles - sCount;
        }

        unsigned int uParticleSeed = uSeed;
        PblRandom spawnRand(mSpawnerSeed ^ 0xC71C2830u);
        for (int i = 0; i < iBurst; ++i, ++sCount)
        {
            if (iNum >= 127)
            {
                break;
            }
            PblRandom particleRand(uParticleSeed);
            particleRand.GetInt();
            uParticleSeed = particleRand.GetInt();
            if (pClass->mGeometry.mType != ParticleGeometry::PT_STREAK && !ParticleSystem::IsLodActive(sCount & 3))
            {
                continue;
            }

            Particle& particle = sParticleCache[iNum];
            const float fAge = fTime - (pInfo->mStartTime + fDelay);
            if (pClass->mNoRegisterStep)
            {
                mat = matFirst;
            }
            if (pClass->mWorldAlignedFlag)
            {
                for (int r = 0; r < 3; ++r)
                {
                    mat[r] = gMatrixIdentity[r];
                    mat.m[r][3] = 0.0f;
                }
            }
            pClass->mSpawner.SetupParticle(particle, mat, spawnRand.GetInt(), sCount, fAge, mSize, pInfo->mVelocity, mInheritVelocityFactor, pClass->mNoRegisterStep);
            particle.mInfoData = pInfo;
            sParticleCache[i].mFrameIndex = pClass->mGeometry.mType == ParticleGeometry::PT_ANIMATED ? 0 : -1;
            if (pClass->mGeometry.mType == ParticleGeometry::PT_STREAK)
            {
                particle.mRotation[0] *= fTime;
            }
            const float fBorn = pInfo->mStartTime + fDelay;
            if (pClass->mSpawner.mFadeInTime > fBorn)
            {
                const float fFade = fBorn / pClass->mSpawner.mFadeInTime;
                for (int k = 0; k < pClass->mSpawner.mNumColors; ++k)
                {
                    particle.mColor[k].a *= fFade;
                }
            }
            ++iNum;
        }
        if (bLastBurst)
        {
            break;
        }

        fDelay += fBurstDelay;
        mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fBurstDelay,
                                       mat.m[3][1] + pInfo->mVelocity.y * fBurstDelay,
                                       mat.m[3][2] + pInfo->mVelocity.z * fBurstDelay));
        while (fDelay > pInfo->mLifeTime && pInfo->mNext)
        {
            fDelay -= pInfo->mLifeTime;
            pInfo = pInfo->mNext;
            mat = pInfo->mMatrix;
            mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fDelay,
                                           mat.m[3][1] + pInfo->mVelocity.y * fDelay,
                                           mat.m[3][2] + pInfo->mVelocity.z * fDelay));
        }
        if (fDelay > pInfo->mLifeTime && !pInfo->mNext)
        {
            bLastBurst = true;
            fDelay = 0.0f;
            mat = pInfo->mMatrix;
            mat.m[3][3] = 1.0f;
        }
    }

    for (int i = 0; i < iNum; ++i)
    {
        Particle& particle = sParticleCache[i];
        mClass->mTransformer.Update(particle, mSize);
        if (mClass->mGeometry.mType == ParticleGeometry::PT_ANIMATED)
        {
            const int iTotal = (int)mClass->mGeometry.mTotalFrames;
            const int iFrame = (int)(particle.mLifeTime / mClass->mGeometry.mTimePerFrame);
            if (mClass->mGeometry.mLooping)
            {
                particle.mFrameIndex = (char)(iFrame % iTotal);
            }
            else
            {
                particle.mFrameIndex = iFrame < iTotal ? (char)iFrame : -1;
            }
        }
        else
        {
            particle.mFrameIndex = -1;
        }
    }

    for (int i = 0; i < iNum; ++i)
    {
        for (int k = 0; k < mClass->mSpawner.mNumColors; ++k)
        {
            RedColorValue& color = sParticleCache[i].mColor[k];
            color.r *= mColor.r;
            color.g *= mColor.g;
            color.b *= mColor.b;
            color.a *= mColor.a;
        }
    }

    if (mClass->mGeometry.mType != ParticleGeometry::PT_STREAK || iNum >= 3)
    {
        for (int i = 0; i < iNum; ++i)
        {
            mClass->mGeometry.Render(sParticleCache[i], i == iNum - 1);
        }
    }
}

// FUNCTION: SWBF2 0x0060b0f0
void ParticleEmitter::RenderAsEmitter(float fTime)
{
    if (!mFirstInfo)
    {
        return;
    }

    ParticleEmitterInfoData* pInfo = mFirstInfo;
    unsigned int uSeed = mSeed;
    float fDelay = mSpawnDelay;
    short sCount = mParticleCount;

    PblMatrix mat = pInfo->mMatrix;
    mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fDelay,
                                   mat.m[3][1] + pInfo->mVelocity.y * fDelay,
                                   mat.m[3][2] + pInfo->mVelocity.z * fDelay));
    while (fDelay > pInfo->mLifeTime && pInfo->mNext)
    {
        fDelay -= pInfo->mLifeTime;
        pInfo = pInfo->mNext;
        mat = pInfo->mMatrix;
        mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fDelay,
                                       mat.m[3][1] + pInfo->mVelocity.y * fDelay,
                                       mat.m[3][2] + pInfo->mVelocity.z * fDelay));
    }

    PblMatrix matFirst;
    if (mClass->mNoRegisterStep)
    {
        ParticleEmitterInfoData* pLast = mFirstInfo;
        while (pLast->mNext)
        {
            pLast = pLast->mNext;
        }
        matFirst = pLast->mMatrix;
    }

    mClass->mGeometry.Begin();
    int iNum = 0;
    for (;;)
    {
        const float fStart = pInfo->mStartTime + fDelay;
        if (!(fTime >= fStart))
        {
            break;
        }
        if (mMaxParticles != -1 && sCount >= mMaxParticles)
        {
            break;
        }
        if (!(mStopTime > fStart) || sCount >= 31)
        {
            break;
        }

        PblRandom rand(uSeed);
        rand.GetInt();
        uSeed = rand.GetInt();
        ParticleEmitterClass* const pClass = mClass;
        const float fBurstDelay = pClass->mBurstDelay.GetValue(rand);
        int iBurst = (int)(pClass->mBurstCount.GetValue(rand) + 0.5f);
        if (iBurst > 128)
        {
            iBurst = 128;
        }
        if (mMaxParticles != -1 && mMaxParticles - sCount < iBurst)
        {
            iBurst = mMaxParticles - sCount;
        }

        unsigned int uParticleSeed = uSeed;
        PblRandom spawnRand(mSpawnerSeed ^ 0xC71C2830u);
        for (int i = 0; i < iBurst && sCount < 31; ++i)
        {
            if (iNum >= 128)
            {
                continue;
            }
            PblRandom particleRand(uParticleSeed);
            particleRand.GetInt();
            uParticleSeed = particleRand.GetInt();
            const float fAge = fTime - (pInfo->mStartTime + fDelay);
            if (mClass->mNoRegisterStep)
            {
                mat = matFirst;
            }
            if (mClass->mWorldAlignedFlag)
            {
                for (int r = 0; r < 3; ++r)
                {
                    mat[r] = gMatrixIdentity[r];
                    mat.m[r][3] = 0.0f;
                }
            }
            if (!mClass->mGeometry.mEmitterClass)
            {
                return;
            }
            mClass->mGeometry.mEmitterClass->SetNextSeed(uParticleSeed);

            ParticleEmitterObject* pChild = &sPEO;
            pChild->Reset(mClass->mGeometry.mEmitterClass, mSpawnerSeed + sCount * 10);
            pChild->SetChildFlag();

            Particle particle;
            Particle step;
            float fStepTime = 0.0f;
            float fStepDelta = 0.3f;
            mClass->mSpawner.SetupParticle(particle, mat, spawnRand.GetInt(), sCount, fAge, mSize, pInfo->mVelocity, mInheritVelocityFactor, mClass->mNoRegisterStep);
            particle.mInfoData = pInfo;
            if (fAge >= 0.0f)
            {
                do
                {
                    step = particle;
                    step.mLifeTime = fStepTime;
                    mClass->mTransformer.Update(step, mSize);
                    pChild->AttachEffectToVector(&step.mPosition, false);
                    ParticleEmitterObject::sForceRegisterStep = true;
                    pChild->Update(fStepDelta > fStepTime ? fStepTime : fStepDelta);
                    ParticleEmitterObject::sForceRegisterStep = false;
                    if (fStepTime == fAge)
                    {
                        break;
                    }
                    fStepTime += 0.3f;
                    if (fStepTime > fAge)
                    {
                        fStepDelta -= fStepTime - fAge;
                        fStepTime = fAge;
                    }
                } while (fAge >= fStepTime);
            }

            for (int k = 0; k < mClass->mSpawner.mNumColors; ++k)
            {
                RedColorValue& color = step.mColor[k];
                color.r *= mColor.r;
                color.g *= mColor.g;
                color.b *= mColor.b;
                color.a *= mColor.a;
            }
            pChild->SetEmitterColor(step.mColor[0]);
            pChild->Render_(0, 1.0f, 0);
            ++iNum;
            ++sCount;
        }

        fDelay += fBurstDelay;
        mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fBurstDelay,
                                       mat.m[3][1] + pInfo->mVelocity.y * fBurstDelay,
                                       mat.m[3][2] + pInfo->mVelocity.z * fBurstDelay));
        while (fDelay > pInfo->mLifeTime && pInfo->mNext)
        {
            fDelay -= pInfo->mLifeTime;
            pInfo = pInfo->mNext;
            mat = pInfo->mMatrix;
            mat.SetTranslation(PblVector3(mat.m[3][0] + pInfo->mVelocity.x * fDelay,
                                           mat.m[3][1] + pInfo->mVelocity.y * fDelay,
                                           mat.m[3][2] + pInfo->mVelocity.z * fDelay));
        }
    }
}

// FUNCTION: SWBF2 0x0060a4f0
void ParticleEmitter::Register(ParticleEmitterInfoData* pInfo)
{
    ParticleEmitter* p = this;
    do
    {
        pInfo->AddRef();
        if (!p->mFirstInfo)
        {
            p->mFirstInfo = pInfo;
        }
        p = p->mNext;
    } while (p);
}

// FUNCTION: SWBF2 0x00609d80
ParticleEmitterObject::ParticleEmitterObject()
    : RedSceneObject(&noloddata)
{
    mbAddedToScene = true;
    mReflectionEnable = true;
}

// FUNCTION: SWBF2 0x00609e60
bool ParticleEmitterObject::IsReflected() const
{
    return mReflectionEnable;
}

// SYNTHETIC: SWBF2 0x00609e70
// ParticleEmitterObject::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00405c80
// `dynamic initializer for 'sPEO''
// SYNTHETIC: SWBF2 0x00769220
// `dynamic atexit destructor for 'sPEO''

// GLOBAL: SWBF2 0x01ef4730
// sPEO
ParticleEmitterObject sPEO;

// FUNCTION: SWBF2 0x0060b8f0
void ParticleEmitter::Cleanup()
{
    sPEO.DeleteEmitter();
}

// FUNCTION: SWBF2 0x00609cb0
void ParticleEmitterClass::SetNextSeed(unsigned int uSeed)
{
    mNextSeed = uSeed;
    if (mNext)
    {
        mNext->SetNextSeed(uSeed);
    }
}

// SYNTHETIC: SWBF2 0x00405be0
// `dynamic initializer for 'sParticleCache''

// GLOBAL: SWBF2 0x01ef2730
// sParticleCache
Particle sParticleCache[128];

// SYNTHETIC: SWBF2 0x00405c20
// `dynamic initializer for 'sEmitterCache''

// GLOBAL: SWBF2 0x01ef4840
// sEmitterCache
Particle sEmitterCache[32];


