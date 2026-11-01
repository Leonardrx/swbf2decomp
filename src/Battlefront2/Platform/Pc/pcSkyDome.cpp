#include <d3dx9math.h>
#include "Battlefront2/Source/SkyDome.h"
#include "PebbleFL/Common/PblVector.h"

#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcSkyFogShader.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "Battlefront2/Source/ParticleEmitterObject.h"
#include "Battlefront2/Source/SkyStars.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "pcSunFlare.h"

// SYNTHETIC: SWBF2 0x004020e0
// `dynamic initializer for 's_SkyFogPlane''
// SYNTHETIC: SWBF2 0x00766cf0
// `dynamic atexit destructor for 's_SkyFogPlane''

// GLOBAL: SWBF2 0x01eafd84
// s_SkyFogPlane
SkyFogRenderable s_SkyFogPlane;

// FUNCTION: SWBF2 0x004285e0
SkyFogRenderable::~SkyFogRenderable()
{
    if (PblRef* p = m_pPrimitive)
    {
        p->Release();
    }
    if (PblRef* p = m_pShader)
    {
        p->Release();
    }
}

// FUNCTION: SWBF2 0x00428690
void SkyFogRenderable::Init()
{
    m_pShader = pcRedShader::Create(0x890EFA1E /* "SkyFog" */, 0x202, 0, 0, 0,
                                    RedColor::WHITE, RedColor::BLACK, 32.0f);
    static_cast<pcSkyFogShader*>(m_pShader)->SetTexture(
        RedRenderer::pcGetBuffer(PC_BUFFER_SKY));

    m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x202, 0x14, 4,
                                          0, true);
    PblBox box(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
    m_pPrimitive->m_bbox = box;
    CompressedPos::SetBoundingBox(box);

    struct FogVertex
    {
        PblVector3 vPos;
        float fU;
        float fV;
    };
    FogVertex* pVertex = (FogVertex*)m_pPrimitive->BeginVertexData();
    if (!pVertex)
    {
        return;
    }

    pVertex[0].vPos = PblVector3(-1.0f, -1.0f, 1.0f);
    pVertex[0].fU = 0.0f;
    pVertex[0].fV = 1.0f;
    pVertex[1].vPos = PblVector3(1.0f, -1.0f, 1.0f);
    pVertex[1].fU = 1.0f;
    pVertex[1].fV = 1.0f;
    pVertex[2].vPos = PblVector3(-1.0f, 1.0f, 1.0f);
    pVertex[2].fU = 0.0f;
    pVertex[2].fV = 0.0f;
    pVertex[3].vPos = PblVector3(1.0f, 1.0f, 1.0f);
    pVertex[3].fU = 1.0f;
    pVertex[3].fV = 0.0f;

    m_pPrimitive->EndVertexData();

    Activate(9999);
}

// FUNCTION: SWBF2 0x00428950
void SkyDome::Init()
{
    s_SkyFogPlane.Init();
}

extern PblMatrix gMatrixIdentity;

// FUNCTION: SWBF2 0x004288f0
void SkyFogRenderable::Render(int)
{
    RedRenderer::pcRenderPrimitive(m_pPrimitive, m_pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
}




// FUNCTION: SWBF2 0x00428970
void SkyDome::Render(int value)
{
    const unsigned int uiFlags = (unsigned int)value;

    bool bFogEnable = RedRenderer::GetFogEnable();
    RedRenderer::SetFogEnable(false);
    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

    if (mNumDomeModels > 0)
    {
        PblMatrix mat = gMatrixIdentity;
        bool bNearClip = RedRenderer::pcGetUserNearClipPlaneEnable();
        RedRenderer::pcSetUserNearClipPlaneEnable(false);

        for (int i = 0; i < mNumDomeModels; ++i)
        {
            DomeModel& rDome = mDomeModels[i];
            if (!rDome.mModel)
            {
                continue;
            }

            mat = gMatrixIdentity;
            D3DXMatrixRotationAxis(reinterpret_cast<D3DXMATRIX*>(&mat), reinterpret_cast<const D3DXVECTOR3*>(&rDome.mRotationAxis),
                                   RedRenderer::GetTimeBase()->GetTime() * rDome.mRotationSpeed *
                                       6.2831855f);

            const PblMatrix& camMatrix = pCamera->GetMatrix();
            if (rDome.mType == STATIC_HEIGHT)
            {
                float fHeight = rDome.mOffset + mDomeHeight;
                PblVector3 pos = camMatrix.GetTranslation();
                pos.y = fHeight - (fHeight - pos.y) * rDome.mMovementScale;
                mat.SetTranslation(pos);
            }
            else
            {
                mat.SetTranslation(camMatrix.GetTranslation());
            }

            for (int j = 0; j < rDome.mNumEffectData; ++j)
            {
                EffectData& rData = rDome.mEffectData[j];
                if (rData.effect == 0)
                {
                    continue;
                }

                PblMatrix boneMat;
                rDome.mModel->GetBoneMatrix(rData.hardPoint, boneMat);
                PblMatrix product = boneMat * mat;
                boneMat = product;
                const PblVector3& vEffectPos = boneMat.GetTranslation();
                rData.pos = vEffectPos;

                const PblVector3& camPos = camMatrix.GetTranslation();
                const PblVector3& camZ = camMatrix.GetZAxis();
                const PblVector3 vToEffect = vEffectPos - camPos;
                if (camZ.y * vToEffect.y + vToEffect.x * camZ.x + camZ.z * vToEffect.z < 0.0f)
                {
                    rData.effect->Update(0.0f);
                    rData.effect->Render(0, 1.0f, 0);
                }
            }

            if (i == 0)
            {
                RedColor color = m_colorSkyDome;
                rDome.mModel->Render(&mat, 0, color, 0x20211, 0);
                if (m_pStars)
                {
                    m_pStars->Render(uiFlags);
                }
                if (uiFlags & 0x400000)
                {
                    RedRenderer::pcSetUserNearClipPlaneEnable(bNearClip);
                    RedRenderer::Flush();
                    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
                    RedRenderer::SetFogEnable(bFogEnable);
                    return;
                }
            }
            else
            {
                rDome.mModel->Render(&mat, 0, RedColor::WHITE, 0x20211, 0);
            }
        }

        RedRenderer::pcSetUserNearClipPlaneEnable(bNearClip);
        RedParticleRenderer::RenderAll();
        RedRenderer::Flush();
    }
    else if (m_pStars)
    {
        bool bNearClip = RedRenderer::pcGetUserNearClipPlaneEnable();
        RedRenderer::pcSetUserNearClipPlaneEnable(false);
        m_pStars->Render(uiFlags);
        RedRenderer::pcSetUserNearClipPlaneEnable(bNearClip);
        RedRenderer::Flush();
    }

    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    RedRenderer::SetFogEnable(bFogEnable);

    float fFarSceneRange = FLRenderer::GetFarSceneRange();
    if (!(FLRenderer::GetNearSceneFadeStart() >= fFarSceneRange))
    {
        RedTerrain::Render(uiFlags | 0x20000);
        RedRenderer::pcFlush(RENDERPHASE_OPAQUE);
        if (!(uiFlags & 0x200000))
        {
            RedWater::Render(uiFlags | 0x20000);
        }
    }
}

// FUNCTION: SWBF2 0x00428860
void SkyFogRenderable::Cleanup()
{
    Deactivate();

    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
        m_pPrimitive = 0;
    }
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x00428960
void SkyDome::Cleanup()
{
    s_SkyFogPlane.Cleanup();
}
