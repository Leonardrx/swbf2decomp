#include <math.h>

#include "MuzzleFlash.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

// GLOBAL: SWBF2 0x01e579f0
MuzzleFlashRenderer* sMuzzleFlashRenderer;

class Aimer
{
public:

    char mUnmodeledPrefix[0x88];
    PblVector3 mFirePos;
};

// FUNCTION: SWBF2 0x005a9f70
MuzzleFlashRenderer::MuzzleFlashRenderer()
{
    mNumMuzzleFlashes = 0;
    mCurrentBuffer = 0;
    mOldNumMuzzleFlashes = 0;
    Activate(250);
}

MuzzleFlashRenderer::~MuzzleFlashRenderer()
{
    Deactivate();
}

// SYNTHETIC: SWBF2 0x005aa670
// MuzzleFlashRenderer::`scalar deleting destructor'

inline void MuzzleFlashRenderer::RenderFlash(MuzzleFlashData const& data, RedColor const& color) const
{
    const PblVector3 vPos = data.m_pAimer->mFirePos;
    const PblVector3 vMin(vPos.x - 50.0f, vPos.y - 50.0f, vPos.z - 50.0f);
    const PblVector3 vMax(vPos.x + 50.0f, vPos.y + 50.0f, vPos.z + 50.0f);
    RedParticleRenderer::SubmitBounds(vMin, vMax);
    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;
    RedParticleRenderer::SubmitParticle(PIT_PARTICLE, vPos, color, data.mRadius, 1.0f, texCoords,
                                        gVectorUnitX, gVectorUnitY);
}

// FUNCTION: SWBF2 0x005a9fe0
void MuzzleFlashRenderer::RenderStreak(MuzzleFlashData const& data, RedColor const& color) const
{
    const PblVector3 vPos = data.m_pAimer->mFirePos;
    RedParticleRenderer::SubmitBounds(PblVector3(vPos.x - 50.0f, vPos.y - 50.0f, vPos.z - 50.0f),
                                      PblVector3(vPos.x + 50.0f, vPos.y + 50.0f, vPos.z + 50.0f));
    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;
    RedParticleRenderer::SubmitParticle(PIT_STREAK_HEAD, vPos, color, 0.15f, 0.0f,
                                        texCoords, gVectorUnitX, gVectorUnitY);
    const float fHalfLength = data.mLength * 0.5f;
    RedParticleRenderer::SubmitParticle(PIT_STREAK, vPos + data.mDir * fHalfLength, color, data.mWidth, 0.5f,
                                        texCoords, gVectorUnitX, gVectorUnitY);
    RedParticleRenderer::SubmitParticle(PIT_STREAK_TAIL, vPos + data.mDir * data.mLength, color, data.mWidth, 1.0f,
                                        texCoords, gVectorUnitX, gVectorUnitY);
}

// FUNCTION: SWBF2 0x005aa190
void MuzzleFlashRenderer::Render(int)
{
    if (mNumMuzzleFlashes == 0 && mOldNumMuzzleFlashes == 0)
    {
        return;
    }

    const PblMatrix mCamera = RedRenderer::GetCamera()->GetMatrix();
    RedParticleRenderer::SubmitTexture(0x504865B4u, BLEND_ADDITIVE, 0);

    for (int i = 0; i < mNumMuzzleFlashes; i++)
    {
        const float fDot = mMuzzleFlashData[mCurrentBuffer][i].mDir.y * mCamera.m[2][1] +
                           mMuzzleFlashData[mCurrentBuffer][i].mDir.x * mCamera.m[2][0] +
                           mMuzzleFlashData[mCurrentBuffer][i].mDir.z * mCamera.m[2][2];
        RedColor color = mMuzzleFlashData[mCurrentBuffer][i].mColor;
        const float fAbsDot = fDot < 0.0f ? -fDot : fDot;
        color.a = (unsigned char)(int)((1.0f - fAbsDot * 0.3f) * 255.0f);
        if (color.a)
        {
            RenderStreak(mMuzzleFlashData[mCurrentBuffer][i], color);
        }
    }
    for (int i = 0; i < mOldNumMuzzleFlashes; i++)
    {
        const float fDot = mMuzzleFlashData[1 - mCurrentBuffer][i].mDir.y * mCamera.m[2][1] +
                           mMuzzleFlashData[1 - mCurrentBuffer][i].mDir.x * mCamera.m[2][0] +
                           mMuzzleFlashData[1 - mCurrentBuffer][i].mDir.z * mCamera.m[2][2];
        RedColor color = mMuzzleFlashData[1 - mCurrentBuffer][i].mColor;
        const float fAbsDot = fDot < 0.0f ? -fDot : fDot;
        color.a = (unsigned char)(int)((1.0f - fAbsDot * 0.3f) * 128.0f);
        if (color.a)
        {
            RenderStreak(mMuzzleFlashData[1 - mCurrentBuffer][i], color);
        }
    }

    RedParticleRenderer::SubmitTexture(0xCFCCCBCEu, BLEND_ADDITIVE, 0);

    for (int i = 0; i < mNumMuzzleFlashes; i++)
    {
        RenderFlash(mMuzzleFlashData[mCurrentBuffer][i], mMuzzleFlashData[mCurrentBuffer][i].mColor);
    }
    for (int i = 0; i < mOldNumMuzzleFlashes; i++)
    {
        RedColor color = mMuzzleFlashData[1 - mCurrentBuffer][i].mColor;
        color.a >>= 1;
        RenderFlash(mMuzzleFlashData[1 - mCurrentBuffer][i], color);
    }

    mCurrentBuffer = 1 - mCurrentBuffer;
    mOldNumMuzzleFlashes = mNumMuzzleFlashes;
    mNumMuzzleFlashes = 0;
}

// FUNCTION: SWBF2 0x005aa750
bool MuzzleFlash::Destroy()
{
    if (!sMuzzleFlashRenderer)
    {
        return false;
    }

    delete sMuzzleFlashRenderer;
    sMuzzleFlashRenderer = 0;
    return true;
}

namespace MuzzleFlash
{

// FUNCTION: SWBF2 0x005aa6e0
bool Init()
{
    if (sMuzzleFlashRenderer)
    {
        return false;
    }
    sMuzzleFlashRenderer = new MuzzleFlashRenderer;
    return true;
}

}
