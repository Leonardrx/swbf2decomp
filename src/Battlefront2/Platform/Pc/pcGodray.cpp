#include "Battlefront2/Source/Godray.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcParticleShader.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// FUNCTION: SWBF2 0x00415cf0
void GodrayManager::PlatformInit()
{
    if (m_pShader)
    {
        m_pShader->Release();
    }

    if (RedRenderer::pcGetGeometryMode())
    {
        m_pShader = pcRedShader::Create(
            0x8a8743bf /* "Particle" */, 0x9282,
            &m_uiTextureHash, 1, 0x10044, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        m_pShader = pcRedShader::Create(
            0x8a8743bf /* "Particle" */, 0x282,
            &m_uiTextureHash, 1, 0x10044, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
}

// FUNCTION: SWBF2 0x00415d70
void GodrayManager::PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}









struct GodrayVertex
{
    struct VertexC
    {
        CompressedPos pos;
        RedColor color;
        CompressedTexCoord uv;

        void SetPos(const PblVector3& v) { pos.SetPos(v); }
        void SetColor(const RedColor& c) { color = c; }
        void SetUV(float fU, float fV)
        {
            uv.SetUV(fU, fV);
        }
    };

    struct VertexU
    {
        PblVector3 pos;
        RedColor color;
        float u;
        float v;

        void SetPos(const PblVector3& vPos) { pos = vPos; }
        void SetColor(const RedColor& c) { color = c; }
        void SetUV(float fU, float fV) { u = fU; v = fV; }
    };
};
 
 

// FUNCTION: SWBF2 0x00415dc0
void GodrayManager::PlatformRender(unsigned int uiRenderFlags) const
{
    // GLOBAL: SWBF2 0x00796510
    
    static const unsigned short kuiRayIndices[20] =
    {
        5, 1, 4, 0, 3, 2, 2, 6, 3, 7, 4, 8, 5, 9, 9, 8, 11, 7, 10, 6
    };


    if (uiRenderFlags & 0x200000)
    {
        return;
    }
    if (!m_uiNumVisibleGodrays)
    {
        return;
    }

    unsigned int uiNumVertices = 0;
    unsigned int uiNumIndices = 0;
    unsigned int i;
    for (i = 0; i < m_uiNumVisibleGodrays; i++)
    {
        if (m_aVisibleGodrays[i].fAlpha > 0.0f)
        {
            uiNumVertices += 12 * m_aVisibleGodrays[i].uiNumRays;
            uiNumIndices += 22 * m_aVisibleGodrays[i].uiNumRays;
        }
    }
    if (uiNumIndices <= 2)
    {
        return;
    }
    uiNumIndices -= 2;

    const PblVector3& vEye = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    const float fBoxRadius = m_fMaxViewRadius + m_fMaxGodrayLength;
    const PblBox bbox(vEye.x - fBoxRadius, vEye.y - fBoxRadius, vEye.z - fBoxRadius,
                     vEye.x + fBoxRadius, vEye.y + fBoxRadius, vEye.z + fBoxRadius);

    pcRedPrimitive* pPrim;
    if (RedRenderer::pcGetGeometryMode())
    {
        pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, sizeof(GodrayVertex::VertexC),
                                            uiNumVertices, uiNumIndices, false);
        CompressedPos::SetBoundingBox(bbox);
    }
    else
    {
        pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, sizeof(GodrayVertex::VertexU),
                                            uiNumVertices, uiNumIndices, false);
    }
    pPrim->m_bbox = bbox;
    unsigned short* pIndex = pPrim->BeginIndexData();
    void* pVertexData = pPrim->BeginVertexData();
    if (!pVertexData)
    {
        return;
    }

    unsigned short uiNumVerticesWritten = 0;
    const float fTexCoordScale = 8.0f / (m_fTexScaleL * m_fMaxGodrayLength);
    ((pcParticleShader*)m_pShader)->SetTextureScale(1.0f / fTexCoordScale);

    for (i = 0; i < m_uiNumVisibleGodrays; i++)
    {
        const VisibleGodray* pVisible = &m_aVisibleGodrays[i];
        if (!(pVisible->fAlpha > 0.0f))
        {
            continue;
        }

        const Godray* pGodray = pVisible->pGodray;
        const float fMaxRadius = pVisible->fLengthClipped * 0.33f;
        float fAlphaClipped = 1.0f;
        float fLengthAlphaFade = pGodray->fRadius * 0.5f;
        if (fLengthAlphaFade > fMaxRadius)
        {
            fAlphaClipped = fMaxRadius / fLengthAlphaFade;
            fLengthAlphaFade = fMaxRadius;
        }

        RedColor colorEdge = pGodray->color;
        colorEdge.a = 0;
        const PblVector3 vLengthAlphaFade(m_vLengthDir.x * fLengthAlphaFade, m_vLengthDir.y * fLengthAlphaFade,
                                 m_vLengthDir.z * fLengthAlphaFade);

        for (unsigned int uiRay = 0; uiRay < pVisible->uiNumRays; uiRay++)
        {
            const VisibleGodray::Ray& ray = pVisible->aRays[uiRay];

            const float fWidthOffset = ray.fPosZ * m_vWidthDir.z + ray.fPosX * m_vWidthDir.x;
            const PblVector3 vWidth(ray.fRadius * m_vWidthDir.x, m_vWidthDir.y * ray.fRadius,
                                    m_vWidthDir.z * ray.fRadius);
            const float fHalf = ray.fRadius * 0.5f;
            const PblVector3 vWidthAlphaFade(m_vWidthDir.x * fHalf, m_vWidthDir.y * fHalf, m_vWidthDir.z * fHalf);
            const PblVector3 vWidthOffset(m_vWidthDir.x * fWidthOffset, fWidthOffset * m_vWidthDir.y,
                                     fWidthOffset * m_vWidthDir.z);

            const float fTexStartL = pVisible->bStartClipped
                                      ? (pGodray->fLength - pVisible->fLengthClipped) * m_fTexScaleL
                                      : 0.0f;
            float fTexOffsetL = ray.fTexOffsetL0 + m_fTexOffsetL + fTexStartL;
            float fTexOffsetW = ray.fRadius0 * m_fTexOffsetAngle * 0.5f * m_fTexScaleW
                     + (ray.fTexOffsetW0 + m_fTexOffsetW) + m_fTexScaleW * fWidthOffset;
            fTexOffsetL -= (float)(int)(fTexOffsetL + 8.0f);
            fTexOffsetW -= (float)(int)(fTexOffsetW + 8.0f);

            const PblVector3 vLength1(pVisible->vStartPosClipped.x + vLengthAlphaFade.x,
                                      pVisible->vStartPosClipped.y + vLengthAlphaFade.y,
                                      pVisible->vStartPosClipped.z + vLengthAlphaFade.z);
            const PblVector3 vLength2(pVisible->vEndPosClipped.x - vLengthAlphaFade.x,
                                    pVisible->vEndPosClipped.y - vLengthAlphaFade.y,
                                    pVisible->vEndPosClipped.z - vLengthAlphaFade.z);
            const PblVector3 avWidth[4] =
            {
                vWidthOffset - vWidth,
                vWidthOffset - vWidthAlphaFade,
                vWidthOffset + vWidthAlphaFade,
                vWidthOffset + vWidth
            };

            const float fWidthW = m_fTexScaleW * ray.fRadius;
            float afU[4];
            afU[0] = fTexOffsetL * fTexCoordScale;
            afU[1] = (m_fTexScaleL * fLengthAlphaFade + fTexOffsetL) * fTexCoordScale;
            afU[2] = ((pVisible->fLengthClipped - fLengthAlphaFade) * m_fTexScaleL + fTexOffsetL) * fTexCoordScale;
            afU[3] = (pVisible->fLengthClipped * m_fTexScaleL + fTexOffsetL) * fTexCoordScale;
            float afV[4];
            afV[0] = (fWidthW + fTexOffsetW) * fTexCoordScale;
            afV[1] = (fWidthW * 0.5f + fTexOffsetW) * fTexCoordScale;
            afV[2] = (fTexOffsetW - fWidthW * 0.5f) * fTexCoordScale;
            afV[3] = (fTexOffsetW - fWidthW) * fTexCoordScale;

            RedColor color = pGodray->color;
            color.a = (unsigned char)(int)(ray.fAlpha * pVisible->fAlpha * fAlphaClipped * 255.0f);

            if (RedRenderer::pcGetGeometryMode())
            {
                GodrayVertex::VertexC* pVertex = (GodrayVertex::VertexC*)pVertexData;
                pVertex[0].SetPos(pVisible->vStartPosClipped + avWidth[1]);
                pVertex[0].SetColor(colorEdge);
                pVertex[0].SetUV(afU[0], afV[1]);
                pVertex[1].SetPos(pVisible->vStartPosClipped + avWidth[2]);
                pVertex[1].SetColor(colorEdge);
                pVertex[1].SetUV(afU[0], afV[2]);
                pVertex[2].SetPos(vLength1 + avWidth[0]);
                pVertex[2].SetColor(colorEdge);
                pVertex[2].SetUV(afU[1], afV[0]);
                pVertex[3].SetPos(vLength1 + avWidth[1]);
                pVertex[3].SetColor(color);
                pVertex[3].SetUV(afU[1], afV[1]);
                pVertex[4].SetPos(vLength1 + avWidth[2]);
                pVertex[4].SetColor(color);
                pVertex[4].SetUV(afU[1], afV[2]);
                pVertex[5].SetPos(vLength1 + avWidth[3]);
                pVertex[5].SetColor(colorEdge);
                pVertex[5].SetUV(afU[1], afV[3]);
                pVertex[6].SetPos(vLength2 + avWidth[0]);
                pVertex[6].SetColor(colorEdge);
                pVertex[6].SetUV(afU[2], afV[0]);
                pVertex[7].SetPos(vLength2 + avWidth[1]);
                pVertex[7].SetColor(color);
                pVertex[7].SetUV(afU[2], afV[1]);
                pVertex[8].SetPos(vLength2 + avWidth[2]);
                pVertex[8].SetColor(color);
                pVertex[8].SetUV(afU[2], afV[2]);
                pVertex[9].SetPos(vLength2 + avWidth[3]);
                pVertex[9].SetColor(colorEdge);
                pVertex[9].SetUV(afU[2], afV[3]);
                pVertex[10].SetPos(pVisible->vEndPosClipped + avWidth[1]);
                pVertex[10].SetColor(colorEdge);
                pVertex[10].SetUV(afU[3], afV[1]);
                pVertex[11].SetPos(pVisible->vEndPosClipped + avWidth[2]);
                pVertex[11].SetColor(colorEdge);
                pVertex[11].SetUV(afU[3], afV[2]);
                pVertexData = pVertex + 12;
            }
            else
            {
                GodrayVertex::VertexU* pVertex = (GodrayVertex::VertexU*)pVertexData;
                pVertex[0].SetPos(pVisible->vStartPosClipped + avWidth[1]);
                pVertex[0].SetColor(colorEdge);
                pVertex[0].SetUV(afU[0], afV[1]);
                pVertex[1].SetPos(pVisible->vStartPosClipped + avWidth[2]);
                pVertex[1].SetColor(colorEdge);
                pVertex[1].SetUV(afU[0], afV[2]);
                pVertex[2].SetPos(vLength1 + avWidth[0]);
                pVertex[2].SetColor(colorEdge);
                pVertex[2].SetUV(afU[1], afV[0]);
                pVertex[3].SetPos(vLength1 + avWidth[1]);
                pVertex[3].SetColor(color);
                pVertex[3].SetUV(afU[1], afV[1]);
                pVertex[4].SetPos(vLength1 + avWidth[2]);
                pVertex[4].SetColor(color);
                pVertex[4].SetUV(afU[1], afV[2]);
                pVertex[5].SetPos(vLength1 + avWidth[3]);
                pVertex[5].SetColor(colorEdge);
                pVertex[5].SetUV(afU[1], afV[3]);
                pVertex[6].SetPos(vLength2 + avWidth[0]);
                pVertex[6].SetColor(colorEdge);
                pVertex[6].SetUV(afU[2], afV[0]);
                pVertex[7].SetPos(vLength2 + avWidth[1]);
                pVertex[7].SetColor(color);
                pVertex[7].SetUV(afU[2], afV[1]);
                pVertex[8].SetPos(vLength2 + avWidth[2]);
                pVertex[8].SetColor(color);
                pVertex[8].SetUV(afU[2], afV[2]);
                pVertex[9].SetPos(vLength2 + avWidth[3]);
                pVertex[9].SetColor(colorEdge);
                pVertex[9].SetUV(afU[2], afV[3]);
                pVertex[10].SetPos(pVisible->vEndPosClipped + avWidth[1]);
                pVertex[10].SetColor(colorEdge);
                pVertex[10].SetUV(afU[3], afV[1]);
                pVertex[11].SetPos(pVisible->vEndPosClipped + avWidth[2]);
                pVertex[11].SetColor(colorEdge);
                pVertex[11].SetUV(afU[3], afV[2]);
                pVertexData = pVertex + 12;
            }

            if (uiNumVerticesWritten)
            {
                pIndex[0] = pIndex[-1];
                pIndex[1] = uiNumVerticesWritten + 5;
                pIndex += 2;
            }
            for (unsigned int uiIndex = 0; uiIndex < 20; uiIndex++)
            {
                *pIndex++ = uiNumVerticesWritten + kuiRayIndices[uiIndex];
            }
            uiNumVerticesWritten += 12;
        }
    }

    pPrim->EndIndexData();
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, m_pShader,
                                   gMatrixIdentity,
                                   m_color, 0x1000010, 0, 0, 0);
    pPrim->Release();
}
