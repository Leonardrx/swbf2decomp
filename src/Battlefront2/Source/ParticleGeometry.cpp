#include <d3dx9math.h>
#include "ParticleGeometry.h"

#include "RedEngineFL/Graphics/RedModel.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"
#include "PebbleFL/Common/PblMath.h"
#include <math.h>

namespace ParticleSystem
{
PblBox& GetBoundingBox();
}
namespace ParticleSystem
{
float GetSizeMultiplier();
}
namespace ParticleSystem
{
float GetMinParticleSize();
}
namespace ParticleSystem
{
float GetLodAlpha(int iLod);
}
namespace ParticleSystem
{
void SetupParticleCache(unsigned int uTexture, RedBlendMode eBlend, float fBlurValue, int iBlurParam);
}
namespace ParticleSystem
{
void CacheParticle(const PblVector3& vPos, const RedColorValue& color, float fSize, float fRotation);
}

// GLOBAL: SWBF2 0x01eaed64
int ParticleGeometry::mParticleCount;

// FUNCTION: SWBF2 0x0060c930
ParticleGeometry::ParticleGeometry()
{
    mTextureHash = 0;
    mTextureWidthLog = 0;
    mTextureHeightLog = 0;
    mBlendMode = BLEND_NORMAL;
    mType = PT_PARTICLE;
    mEmitterClass = 0;
    mBlurValue = 0.0f;
    mBlurParam = 0;
    mBlurRes = 0.0f;
    mSparkLength = 0.0f;
    mFrameSizeLog = 5.0f;
    mTimePerFrame = 2.0f;
    mTotalFrames = 4.0f;
    mLooping = true;
}

// FUNCTION: SWBF2 0x0060c900
void ParticleGeometry::Setup()
{
    if (mType == PT_GEOMETRY)
    {
        m_pModel = RedModel::_HashTable.Find(mTextureHash);
    }
}

// FUNCTION: SWBF2 0x0060c9a0
void HSVtoRGB(float& r, float& g, float& b, float h, float s, float v)
{
    const float fSector = h * 6.0f;
    if (s == 0.0f)
    {
        b = v;
        g = v;
        r = v;
        return;
    }
    const int iSector = (int)fSector;
    const float f = fSector - (float)iSector;
    const float p = (1.0f - s) * v;
    const float q = (1.0f - f * s) * v;
    const float t = (1.0f - (1.0f - f) * s) * v;
    switch (iSector)
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    default:
        r = v;
        g = p;
        b = q;
        break;
    }
}

// FUNCTION: SWBF2 0x0060cab0
void ParticleGeometry::Render(Particle& particle, bool bLast) const
{
    if (mType != ParticleGeometry::PT_STREAK && particle.mColor[0].a <= 0.0f)
    {
        return;
    }
    if (mType == ParticleGeometry::PT_PARTICLE && ParticleSystem::GetMinParticleSize() > particle.mSize[0])
    {
        return;
    }

    ParticleInfoType eInfo;
    switch (mType)
    {
    case ParticleGeometry::PT_STREAK:
        eInfo = PIT_STREAK;
        if (bLast)
        {
            eInfo = PIT_STREAK_TAIL;
        }
        break;
    case ParticleGeometry::PT_ANIMATED:
        eInfo = PIT_ANIMATED;
        break;
    case ParticleGeometry::PT_BILLBOARD:
        eInfo = PIT_FLAT;
        break;
    case ParticleGeometry::PT_GEOMETRY:
        eInfo = PIT_GEOMETRY;
        break;
    default:
        eInfo = PIT_PARTICLE;
        break;
    }

    if (mFirstTime)
    {
        if (eInfo != PIT_GEOMETRY)
        {
            if (mType == ParticleGeometry::PT_PARTICLE)
            {
                ParticleSystem::SetupParticleCache(mTextureHash, mBlendMode, mBlurValue,
                           mBlurParam);
            }
            else
            {
                ParticleSystem::FlushParticleCache();
                RedParticleRenderer::SubmitTexture(mTextureHash, mBlendMode, 0);
                RedParticleRenderer::SetBlurParams(mBlurValue, mBlurParam);
            }
        }
        mFirstTime = false;
        if (eInfo == PIT_STREAK)
        {
            eInfo = PIT_STREAK_HEAD;
        }
        else if (eInfo == PIT_STREAK_TAIL)
        {
            return;
        }
    }

    RedColorValue color;
    color.r = particle.mColor[0].r;
    color.g = particle.mColor[0].g;
    color.b = particle.mColor[0].b;
    const float fFade = 1.0f - particle.mColor[0].a;
    color.a = (1.0f - fFade * fFade + particle.mColor[0].a) * 0.5f;
    if (mBlendMode == BLEND_BLUR)
    {
        color.a *= 2.0f;
        if (!(1.0f > color.a))
        {
            color.a = 1.0f;
        }
    }
    if (mType != ParticleGeometry::PT_STREAK)
    {
        color.a *= ParticleSystem::GetLodAlpha(particle.mLod);
    }
    if (particle.mUseHSV)
    {
        HSVtoRGB(color.r, color.g, color.b, color.r, color.g, color.b);
    }

    const PblBox& box = ParticleSystem::GetBoundingBox();
    const PblVector3& vPos = particle.mPosition;
    if (!(vPos.x >= box._vMin.x && vPos.x <= box._vMax.x))
    {
        return;
    }
    if (!(vPos.y >= box._vMin.y && vPos.y <= box._vMax.y))
    {
        return;
    }
    if (!(vPos.z >= box._vMin.z && vPos.z <= box._vMax.z))
    {
        return;
    }

    if (mType == ParticleGeometry::PT_SPARK)
    {
        const float fSizeMultiplier = ParticleSystem::GetSizeMultiplier();
        const float fSize = particle.mSize[0] * fSizeMultiplier;
        const int iFrame = particle.mFrameIndex;
        const float fLength = mSparkLength * fSizeMultiplier;
        const PblVector3 vStreak(particle.mVelocity.x * fLength, particle.mVelocity.y * fLength,
                                 particle.mVelocity.z * fLength);
        RedParticleRenderer::TexCoords texCoords = { 0, 0, 255, 255 };
        if (iFrame >= 0)
        {
            const int iFrameSizeLog = (int)mFrameSizeLog;
            const int iUBits = mTextureWidthLog - iFrameSizeLog;
            const int iVBits = mTextureHeightLog - iFrameSizeLog;
            texCoords.u0 = (iFrame & ((1 << iUBits) - 1)) << (8 - iUBits);
            texCoords.v0 = (iFrame & (((1 << iVBits) - 1) << iUBits)) << (8 - iVBits - iUBits);
            texCoords.u1 = texCoords.u0 + (1 << (8 - iUBits)) - 1;
            texCoords.v1 = texCoords.v0 + (1 << (8 - iVBits)) - 1;
        }
        const unsigned char ucB = (unsigned char)(int)(color.b * 255.0f);
        const unsigned char ucG = (unsigned char)(int)(color.g * 255.0f);
        const unsigned char ucR = (unsigned char)(int)(color.r * 255.0f);
        const unsigned char ucA = (unsigned char)(int)(color.a * 255.0f);
        const PblVector3 vHead(vPos.x - vStreak.x, vPos.y - vStreak.y, vPos.z - vStreak.z);
        RedParticleRenderer::SubmitParticle(PIT_STREAK_HEAD, vHead, RedColor(ucR, ucG, ucB, ucA), fSize, 0.0f, texCoords,
                                            gVectorUnitX, gVectorUnitY);
        const PblVector3 vMid(vPos.x - vStreak.x * 0.5f, vPos.y - vStreak.y * 0.5f, vPos.z - vStreak.z * 0.5f);
        RedParticleRenderer::SubmitParticle(PIT_STREAK, vMid, RedColor(ucR, ucG, ucB, ucA), fSize, 0.5f, texCoords,
                                            gVectorUnitX, gVectorUnitY);
        RedParticleRenderer::SubmitParticle(PIT_STREAK_TAIL, vPos, RedColor(ucR, ucG, ucB, ucA), fSize, 1.0f, texCoords,
                                            gVectorUnitX, gVectorUnitY);
    }
    else if (eInfo == PIT_GEOMETRY)
    {
        if (!m_pModel)
        {
            return;
        }

        const float fCos = (float)cos((double)particle.mRotation[0]);
        const float fSin = (float)sin((double)particle.mRotation[0]);
        const float fSize = particle.mSize[0];

        PblMatrix mWorld = PblMatrix(PblVector3(fSize * fCos, 0.0f, -(fSize * fSin)), PblVector3(0.0f, fSize, 0.0f),
                                     PblVector3(fSize * fSin, 0.0f, fSize * fCos), PblVector3(0.0f, 0.0f, 0.0f)) *
                           particle.mInfoData->mMatrix;
        mWorld.SetTranslation(vPos);
        unsigned int uiFlags = 1;
        if (mBlendMode == BLEND_ADDITIVE)
        {
            uiFlags = 0x800001;
        }
        else if (mBlendMode == BLEND_BLUR)
        {
            uiFlags = 0x41;
        }
        RedColor redColor;
        redColor.b = (unsigned char)(int)(color.b * 255.0f);
        redColor.g = (unsigned char)(int)(color.g * 255.0f);
        redColor.r = (unsigned char)(int)(color.r * 255.0f);
        redColor.a = (unsigned char)(int)(color.a * 255.0f);
        m_pModel->Render(&mWorld, 0, redColor, uiFlags, 0);
    }
    else if (mType == ParticleGeometry::PT_PARTICLE)
    {
        ParticleSystem::CacheParticle(vPos, color, ParticleSystem::GetSizeMultiplier() * particle.mSize[0], particle.mRotation[0]);
    }
    else if (mType == ParticleGeometry::PT_ANIMATED)
    {
        const int iFrame = particle.mFrameIndex;
        if (iFrame >= 0)
        {
            const int iFrameSizeLog = (int)mFrameSizeLog;
            const int iUBits = mTextureWidthLog - iFrameSizeLog;
            const int iVBits = mTextureHeightLog - iFrameSizeLog;
            const int iU0 = (iFrame & ((1 << iUBits) - 1)) << (8 - iUBits);
            const int iV0 = (iFrame & (((1 << iVBits) - 1) << iUBits)) << (8 - iVBits - iUBits);
            RedParticleRenderer::TexCoords texCoords;
            texCoords.u0 = iU0;
            texCoords.v0 = iV0;
            texCoords.u1 = iU0 + (1 << (8 - iUBits)) - 1;
            texCoords.v1 = iV0 + (1 << (8 - iVBits)) - 1;
            RedColor redColor;
            redColor.b = (unsigned char)(int)(color.b * 255.0f);
            redColor.g = (unsigned char)(int)(color.g * 255.0f);
            redColor.r = (unsigned char)(int)(color.r * 255.0f);
            redColor.a = (unsigned char)(int)(color.a * 255.0f);
            RedParticleRenderer::SubmitParticle(eInfo, vPos, redColor, ParticleSystem::GetSizeMultiplier() * particle.mSize[0],
                                                particle.mRotation[0], texCoords, gVectorUnitX, gVectorUnitY);
        }
    }
    else if (mType == ParticleGeometry::PT_BILLBOARD)
    {
        const float fRotation = particle.mRotation[0];
        const int iFace = (int)fRotation;
        PblVector3 vRight(0.0f, 0.0f, 0.0f);
        PblVector3 vUp(0.0f, 0.0f, 0.0f);
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, (fRotation - (float)iFace) * 6.2831855f);
        switch (iFace)
        {
        case 0:
            vUp.y = fSin;
            vRight.x = 1.0f;
            vUp.z = fCos;
            break;
        case 1:
            vUp.x = fSin;
            vRight.y = 1.0f;
            vUp.z = fCos;
            break;
        case 2:
            vUp.x = fCos;
            vRight.z = 1.0f;
            vUp.y = fSin;
            break;
        case 3:
            vRight.x = fSin;
            vRight.y = fCos;
            vUp.x = fCos;
            vUp.y = -fSin;
            break;
        case 4:
            vRight.x = fSin;
            vRight.z = fCos;
            vUp.x = fCos;
            vUp.z = -fSin;
            break;
        default:
            vRight.y = fSin;
            vRight.z = fCos;
            vUp.z = -fSin;
            vUp.y = fCos;
            break;
        }
        if (particle.mNoRegisterStep)
        {
            PblVector3 v;
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&v), reinterpret_cast<const D3DXVECTOR3*>(&vRight), reinterpret_cast<const D3DXMATRIX*>(&particle.mInfoData->mMatrix));
            vRight = v;
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&v), reinterpret_cast<const D3DXVECTOR3*>(&vUp), reinterpret_cast<const D3DXMATRIX*>(&particle.mInfoData->mMatrix));
            vUp = v;
        }
        RedColor redColor;
        redColor.b = (unsigned char)(int)(color.b * 255.0f);
        redColor.g = (unsigned char)(int)(color.g * 255.0f);
        redColor.r = (unsigned char)(int)(color.r * 255.0f);
        redColor.a = (unsigned char)(int)(color.a * 255.0f);
        const RedParticleRenderer::TexCoords texCoords = { 0, 0, 255, 255 };
        RedParticleRenderer::SubmitParticle(eInfo, vPos, redColor, ParticleSystem::GetSizeMultiplier() * particle.mSize[0], 0.0f, texCoords,
                                            vRight, vUp);
    }
    else
    {
        RedColor redColor;
        redColor.b = (unsigned char)(int)(color.b * 255.0f);
        redColor.g = (unsigned char)(int)(color.g * 255.0f);
        redColor.r = (unsigned char)(int)(color.r * 255.0f);
        redColor.a = (unsigned char)(int)(color.a * 255.0f);
        const RedParticleRenderer::TexCoords texCoords = { 0, 0, 255, 255 };
        RedParticleRenderer::SubmitParticle(eInfo, vPos, redColor, ParticleSystem::GetSizeMultiplier() * particle.mSize[0],
                                            particle.mRotation[0], texCoords, gVectorUnitX, gVectorUnitY);
    }
    ++ParticleGeometry::mParticleCount;
}

// FUNCTION: SWBF2 0x0060d590
void ParticleGeometry::Begin() const
{
    mFirstTime = true;
}
