#include "Battlefront2/Source/MotionBlurEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

extern PblMatrix gMatrixIdentity;

// GLOBAL: SWBF2 0x01f9ae54
static pcClearShader* s_pClearShader;

// FUNCTION: SWBF2 0x00423bd0
void MotionBlurEffect::PlatformInit()
{
    s_pClearShader = (pcClearShader*)pcRedShader::Create(
        0x5c6e1222 /* "clear" */, 0x403, 0, 0, 4, RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x00423c00
void MotionBlurEffect::PlatformCleanup()
{
    if (s_pClearShader)
    {
        s_pClearShader->Release();
        s_pClearShader = 0;
    }
}

// FUNCTION: SWBF2 0x00423c40
void MotionBlurEffect::PlatformRender(unsigned int) const
{
    const unsigned int uiView = FLRenderer::GetCurrentViewport();
    if (m_curBlurAmount[uiView] == 0.0f)
    {
        return;
    }
    if (!(RedRenderer::pcGetDeviceCaps().DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES))
    {
        return;
    }
    if (!RedRenderer::pcGetRenderOptions().motionBlur)
    {
        return;
    }

    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x403, 32, 4, 0, false);
    PblVector4* pVerts = (PblVector4*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    float fMinX, fMinY, fMaxY, fMaxX;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);

    pVerts[0].Set(fMinX, fMinY, 0.0f, 1.0f);
    pVerts[1].Set(viewport.fOffsetX, viewport.fOffsetY, 0.0f, 0.0f);
    pVerts[2].Set(fMinX, fMaxY, 0.0f, 1.0f);
    pVerts[3].Set(viewport.fOffsetX, viewport.fHeight + viewport.fOffsetY, 0.0f, 1.0f);
    pVerts[4].Set(fMaxX, fMinY, 0.0f, 1.0f);
    pVerts[5].Set(viewport.fWidth + viewport.fOffsetX, viewport.fOffsetY, 1.0f, 0.0f);
    pVerts[6].Set(fMaxX, fMaxY, 0.0f, 1.0f);
    pVerts[7].Set(viewport.fWidth + viewport.fOffsetX, viewport.fHeight + viewport.fOffsetY, 1.0f, 1.0f);
    pPrimitive->EndVertexData();

    s_pClearShader->m_pTex = FLRenderer::pcGetPreviousFrameTexture();
    s_pClearShader->m_pAlphaMaskTex = RedTexture::Find(0xF745F270);

    RedColor color;
    color.b = RedColor::WHITE.b;
    color.g = RedColor::WHITE.g;
    color.r = RedColor::WHITE.r;
    color.a = (unsigned char)(m_curBlurAmount[uiView] * 255.0f);
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pClearShader, gMatrixIdentity,
                                   color, 0x200, 0, 0, 0);
    pPrimitive->Release();
}
