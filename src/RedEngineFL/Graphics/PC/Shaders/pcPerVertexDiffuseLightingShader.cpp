#include <string.h>
#include "pcPerVertexDiffuseLightingShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Graphics/RedSpotLight.h"

#include "PebbleFL/Common/PblChunk.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcPerVertexDiffuseLightingShader::s_uiID;
pcRedShader::GetStateFuncT pcPerVertexDiffuseLightingShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcPerVertexDiffuseLightingShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcPerVertexDiffuseLightingShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcPerVertexDiffuseLightingShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x00710560
pcPerVertexDiffuseLightingShader::pcPerVertexDiffuseLightingShader()
{
    memset(GetStateFuncArray, 0, sizeof(GetStateFuncArray));
    memset(BeginFuncArray, 0, sizeof(BeginFuncArray));
    memset(EndFuncArray, 0, sizeof(EndFuncArray));
    memset(RenderFuncArray, 0, sizeof(RenderFuncArray));

    s_uiID = _Register(s_uiRenderTypeHash, this);

    _InitPipelineFuncArray();
}

// FUNCTION: SWBF2 0x00710620
pcPerVertexDiffuseLightingShader::pcPerVertexDiffuseLightingShader(unsigned int uiFlags,
                                                                   const unsigned int* pParams,
                                                                   unsigned int uiNumParams,
                                                                   unsigned int uiMaterialFlags,
                                                                   const RedColor& color0,
                                                                   const RedColor& color1,
                                                                   float fSpecularPower)
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
    _Init(uiFlags, pParams, uiNumParams, uiMaterialFlags,
          color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x007106e0
void pcPerVertexDiffuseLightingShader::Begin_Fixed(unsigned int uiState, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    if (uiState == 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    else
    {
        RedRenderer::pcSetTexture(0, 0);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

// FUNCTION: SWBF2 0x00710830
void pcPerVertexDiffuseLightingShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RedRenderer::pcSetTexture(0, 0);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    m_pLightingStatePrev = 0;
}

// FUNCTION: SWBF2 0x00710880
void pcPerVertexDiffuseLightingShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x007108a0
void pcPerVertexDiffuseLightingShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x007108b0
void pcPerVertexDiffuseLightingShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x007108c0
unsigned int pcPerVertexDiffuseLightingShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x007108e0
RenderPhaseT pcPerVertexDiffuseLightingShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x007108f0
unsigned int pcPerVertexDiffuseLightingShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00710900
pcRedShaderDef* pcPerVertexDiffuseLightingShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x00710910
unsigned int pcPerVertexDiffuseLightingShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00710920
unsigned int pcPerVertexDiffuseLightingShader::GetState_Prog1(const RenderItem* pItem) const
{
    const unsigned char uiInverted = ~pItem->flags;
    return (uiInverted >> 2) & 4;
}

// FUNCTION: SWBF2 0x00710940
void pcPerVertexDiffuseLightingShader::InitFixedFunctionShaderDef() const
{
    // GLOBAL: SWBF2 0x007854a4
    static const pcRedShaderDef::FixedFunctionStateT aFFStates[2] =
    {
        { 1, { 0x21, 0x00, 0x00, 0x00 } },
        { 1, { 0x21, 0x00, 0x00, 0x00 } },
    };

    GetShaderDef()->InitFixedFunction(0x28, 2, aFFStates);
}

// FUNCTION: SWBF2 0x00710960
void pcPerVertexDiffuseLightingShader::Render_Fixed(const RenderItem*, unsigned int) const
{
}

static inline void SetOmniLightConstants(pcRedVertexShaderConstantRegister* pRegs, int k,
                                         const RedOmniLight* pLight)
{
    pRegs[25 + 2 * k].component[0] = pLight->m_color.r;
    pRegs[25 + 2 * k].component[1] = pLight->m_color.g;
    pRegs[25 + 2 * k].component[2] = pLight->m_color.b;
    pRegs[25 + 2 * k].component[3] = 1.0f;
    pRegs[26 + 2 * k].component[0] = pLight->m_pos.x;
    pRegs[26 + 2 * k].component[1] = pLight->m_pos.y;
    pRegs[26 + 2 * k].component[2] = pLight->m_pos.z;
    pRegs[26 + 2 * k].component[3] = pLight->m_fRadiusInvSq;
}

static inline void SetNullOmniLightConstants(pcRedVertexShaderConstantRegister* pRegs, int k)
{
    const RedColorValue& black = RedColorValue::BLACK;
    pRegs[25 + 2 * k].component[0] = black.r;
    pRegs[25 + 2 * k].component[1] = black.g;
    pRegs[25 + 2 * k].component[2] = black.b;
    pRegs[25 + 2 * k].component[3] = 1.0f;
    pRegs[26 + 2 * k].component[0] = 0.0f;
    pRegs[26 + 2 * k].component[1] = 0.0f;
    pRegs[26 + 2 * k].component[2] = 0.0f;
    pRegs[26 + 2 * k].component[3] = 1.0f;
}

// FUNCTION: SWBF2 0x00710970
void pcPerVertexDiffuseLightingShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    RedRenderer::pcLoadFormat(pItem->pPrim->m_vertexFormat);

    if (!pItem->uiShaderState)
    {
        pItem->pPrim->Draw();
        return;
    }

    const RedLightingState* pLighting = pItem->pLightingState;
    const bool bVertexLit = (pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF & 0x100) != 0;
    if (pLighting != m_pLightingStatePrev)
    {
        m_pLightingStatePrev = (void*)pLighting;
        const RedColorValue& black = RedColorValue::BLACK;

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(19, 6);
        if (!bVertexLit)
        {
            const RedAmbientLight* pAmbient = &pLighting->ambientLight;
            pRegs[19].component[0] = pAmbient->m_topColor.r;
            pRegs[19].component[1] = pAmbient->m_topColor.g;
            pRegs[19].component[2] = pAmbient->m_topColor.b;
            pRegs[19].component[3] = pAmbient->m_topColor.a;
            pRegs[20].component[0] = pAmbient->m_bottomColor.r;
            pRegs[20].component[1] = pAmbient->m_bottomColor.g;
            pRegs[20].component[2] = pAmbient->m_bottomColor.b;
            pRegs[20].component[3] = pAmbient->m_bottomColor.a;
        }
        else
        {
            pRegs[19].component[0] = black.r;
            pRegs[19].component[1] = black.g;
            pRegs[19].component[2] = black.b;
            pRegs[19].component[3] = black.a;
            pRegs[20].component[0] = black.r;
            pRegs[20].component[1] = black.g;
            pRegs[20].component[2] = black.b;
            pRegs[20].component[3] = black.a;
        }

        pRegs[21].SetFloats(pLighting->pDirectionalLights[0]->m_color.r,
                            pLighting->pDirectionalLights[0]->m_color.g,
                            pLighting->pDirectionalLights[0]->m_color.b,
                            pLighting->pDirectionalFactor[0]);
        pRegs[22].SetFloats(pLighting->pDirectionalLights[0]->m_vDirection.x,
                            pLighting->pDirectionalLights[0]->m_vDirection.y,
                            pLighting->pDirectionalLights[0]->m_vDirection.z);
        if (pLighting->pDirectionalLights[1])
        {
            pRegs[23].SetFloats(pLighting->pDirectionalLights[1]->m_color.r,
                                pLighting->pDirectionalLights[1]->m_color.g,
                                pLighting->pDirectionalLights[1]->m_color.b,
                                pLighting->pDirectionalFactor[1]);
            pRegs[24].SetFloats(pLighting->pDirectionalLights[1]->m_vDirection.x,
                                pLighting->pDirectionalLights[1]->m_vDirection.y,
                                pLighting->pDirectionalLights[1]->m_vDirection.z);
        }
        else
        {
            pRegs[23].component[0] = black.r;
            pRegs[23].component[1] = black.g;
            pRegs[23].component[2] = black.b;
            pRegs[23].component[3] = 0.0f;
            pRegs[24].component[0] = 0.0f;
            pRegs[24].component[1] = 0.0f;
            pRegs[24].component[2] = 0.0f;
            pRegs[24].component[3] = 1.0f;
        }
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        if (pLighting->pSpotLights[0])
        {
            pRegs = RedRenderer::pcLockVertexShaderConstants(29, 5);
            const RedSpotLight* pSpot = pLighting->pSpotLights[0];
            pRegs[29].SetFloats(pSpot->m_color.r, pSpot->m_color.g, pSpot->m_color.b, 1.0f);
            pRegs[30].SetFloats(pSpot->m_vPos.x, pSpot->m_vPos.y, pSpot->m_vPos.z,
                                pSpot->m_fRadiusInvSq);
            pRegs[31].SetFloats(pSpot->m_vDirection.x, pSpot->m_vDirection.y,
                                pSpot->m_vDirection.z, pSpot->m_bBidirectional ? 2.0f : 0.0f);
            pRegs[32].SetFloats(
                pSpot->m_fCosHalfOuterConeAngle, pSpot->m_fCosHalfInnerConeAngle,
                1.0f / (pSpot->m_fCosHalfInnerConeAngle - pSpot->m_fCosHalfOuterConeAngle),
                pSpot->m_fFalloff);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);

            pRegs = RedRenderer::pcLockVertexShaderConstants(35, 3);
            pRegs[35].SetFloats(pSpot->GetProjectionMatrix().m[0][0],
                                pSpot->GetProjectionMatrix().m[1][0],
                                pSpot->GetProjectionMatrix().m[2][0],
                                pSpot->GetProjectionMatrix().m[3][0]);
            pRegs[36].SetFloats(pSpot->GetProjectionMatrix().m[0][1],
                                pSpot->GetProjectionMatrix().m[1][1],
                                pSpot->GetProjectionMatrix().m[2][1],
                                pSpot->GetProjectionMatrix().m[3][1]);
            pRegs[37].SetFloats(pSpot->GetProjectionMatrix().m[0][3],
                                pSpot->GetProjectionMatrix().m[1][3],
                                pSpot->GetProjectionMatrix().m[2][3],
                                pSpot->GetProjectionMatrix().m[3][3]);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);

            RedRenderer::pcSetTexture(0, RedTexture::Find(PblHash("flames1")));
            RedRenderer::pcSetTexture(1, RedTexture::Find(PblHash("white")));
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &RedColorValue::WHITE.r, 1);
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &RedColorValue::BLACK.r, 1);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
            RedRenderer::pcSetTexture(1, RedShadowMap::GetShadowMapTexture());
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, RedShadowMap::GetShadowMapWrapMode());
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, RedShadowMap::GetShadowMapWrapMode());
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(
                2, &pLighting->pDirectionalLights[0]->m_color.r, 1);
        }

        if (pLighting->pOmniLights[0])
        {
            pRegs = RedRenderer::pcLockVertexShaderConstants(25, 8);
            if (pLighting->pOmniLights[3])
            {
                SetOmniLightConstants(pRegs, 0, pLighting->pOmniLights[0]);
                SetOmniLightConstants(pRegs, 1, pLighting->pOmniLights[1]);
                SetOmniLightConstants(pRegs, 2, pLighting->pOmniLights[2]);
                SetOmniLightConstants(pRegs, 3, pLighting->pOmniLights[3]);
            }
            else if (pLighting->pOmniLights[2])
            {
                SetOmniLightConstants(pRegs, 0, pLighting->pOmniLights[0]);
                SetOmniLightConstants(pRegs, 1, pLighting->pOmniLights[1]);
                SetOmniLightConstants(pRegs, 2, pLighting->pOmniLights[2]);
                SetNullOmniLightConstants(pRegs, 3);
            }
            else
            {
                SetOmniLightConstants(pRegs, 0, pLighting->pOmniLights[0]);
                if (pLighting->pOmniLights[1])
                {
                    SetOmniLightConstants(pRegs, 1, pLighting->pOmniLights[1]);
                }
                else
                {
                    SetNullOmniLightConstants(pRegs, 1);
                }
                SetNullOmniLightConstants(pRegs, 2);
                SetNullOmniLightConstants(pRegs, 3);
            }
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        }
    }

    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00711380
pcRedShader* pcPerVertexDiffuseLightingShader::_Create(unsigned int uiFlags,
                                                       const unsigned int* pParams,
                                                       unsigned int uiNumParams,
                                                       unsigned int uiMaterialFlags,
                                                       const RedColor& color0,
                                                       const RedColor& color1,
                                                       float fSpecularPower) const
{
    return new pcPerVertexDiffuseLightingShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                                color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x00711410
void pcPerVertexDiffuseLightingShader::_Init(unsigned int,
                                             const unsigned int*,
                                             unsigned int,
                                             unsigned int,
                                             const RedColor&,
                                             const RedColor&,
                                             float)
{
    m_pLightingStatePrev = 0;
}

// FUNCTION: SWBF2 0x00711420
void pcPerVertexDiffuseLightingShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcPerVertexDiffuseLightingShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcPerVertexDiffuseLightingShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcPerVertexDiffuseLightingShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcPerVertexDiffuseLightingShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcPerVertexDiffuseLightingShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcPerVertexDiffuseLightingShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcPerVertexDiffuseLightingShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcPerVertexDiffuseLightingShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00711480
void pcPerVertexDiffuseLightingShader::_Read(PblFileChunk& chunk) const
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x454d414e)
        {
            continue;
        }

        char acName[256];
        child.ReadString(acName, 256);
        const unsigned int uiHash = PblHash(acName);
        PblTraceP("pcRedShaderDef [%s]\tHash 0x%08x\n", acName, uiHash);

        pcRedShaderDef* pDef = GetShaderDef();
        if (pDef)
        {
            pDef->Read(chunk);
        }
        else
        {
            PblTraceP("Error: pcRedShaderDef 0x%08x not found\n", uiHash);
        }
    }
}

// SYNTHETIC: SWBF2 0x0040a550
// `dynamic initializer for 'pcPerVertexDiffuseLightingShader::s_uiRenderTypeHash''
unsigned int pcPerVertexDiffuseLightingShader::s_uiRenderTypeHash = PblHash("pervertexdiffuselighting");

// SYNTHETIC: SWBF2 0x0040a510
// `dynamic initializer for 'pcPerVertexDiffuseLightingShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769fc0
// `dynamic atexit destructor for 'pcPerVertexDiffuseLightingShader::s_Factory''
pcPerVertexDiffuseLightingShader pcPerVertexDiffuseLightingShader::s_Factory;

// SYNTHETIC: SWBF2 0x0040a530
// `dynamic initializer for 'pcPerVertexDiffuseLightingShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769fe0
// `dynamic atexit destructor for 'pcPerVertexDiffuseLightingShader::s_shaderDef''
pcRedShaderDef pcPerVertexDiffuseLightingShader::s_shaderDef("pervertexdiffuselighting_shader.xml");

// SYNTHETIC: SWBF2 0x0040a590
// PblMath::`dynamic initializer for 'fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x0040a570
// PblMath::`dynamic initializer for 'fOneOverHalfPi''

// GLOBAL: SWBF2 0x009ca874
// g_pcPerVertexDiffuseLightingShader
unsigned int g_pcPerVertexDiffuseLightingShader;
