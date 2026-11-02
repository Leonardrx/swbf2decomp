#include "Battlefront2/Source/FlashyTextElement.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include <string.h>

// FUNCTION: SWBF2 0x00414720
void FlashyTextElement::PlatformConstructor()
{
    m_pInterfaceBitmapShader = 0;
}

// FUNCTION: SWBF2 0x00414730
void FlashyTextElement::Init()
{
    m_pInterfaceBitmapShader = (pcInterfaceShader*)pcRedShader::Create(
        0x84ea5130, 0x202, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);

    m_pInterfaceBitmapShader->m_mode = pcInterfaceShader::INTERFACE_MODE_BITMAP;
}

// FUNCTION: SWBF2 0x00414770
void FlashyTextElement::Cleanup()
{
    if (m_pInterfaceBitmapShader)
    {
        m_pInterfaceBitmapShader->Release();
        m_pInterfaceBitmapShader = 0;
    }
}

// FUNCTION: SWBF2 0x004147c0
void FlashyTextElement::RenderBitmap(const PblMatrix& mContext, const RedColor& color,
                                     unsigned int uHash, float fX0, float fY0,
                                     float fX1, float fY1)
{
    // GLOBAL: SWBF2 0x007ec3f0
    static FlashyVertexT vertices[4] = {
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f },
    };

    m_pInterfaceBitmapShader->SetTexture(uHash);
    vertices[0].x = fX0;
    vertices[0].y = fY0;
    vertices[1].x = fX0;
    vertices[1].y = fY1;
    vertices[2].x = fX1;
    vertices[2].y = fY0;
    vertices[3].x = fX1;
    vertices[3].y = fY1;
    m_pInterfaceBitmapShader->m_materialFlags &= ~0x40u;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x202, sizeof(FlashyVertexT), 4, 0, false);

    void* pVertexData = pPrimitive->BeginVertexData();
    if (!pVertexData)
    {
        return;
    }

    memcpy(pVertexData, vertices, sizeof(vertices));

    pPrimitive->EndVertexData();

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pInterfaceBitmapShader,
                                   mContext, color, 0x200, 0, 0, 0);

    pPrimitive->Release();
}
