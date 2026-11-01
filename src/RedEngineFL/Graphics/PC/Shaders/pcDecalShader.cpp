#include <string.h>
#include "pcDecalShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcDecalShader::s_uiID;
pcRedShader::GetStateFuncT pcDecalShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcDecalShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcDecalShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcDecalShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x0070efa0
pcDecalShader::pcDecalShader()
{
    memset(GetStateFuncArray, 0, sizeof(GetStateFuncArray));
    memset(BeginFuncArray, 0, sizeof(BeginFuncArray));
    memset(EndFuncArray, 0, sizeof(EndFuncArray));
    memset(RenderFuncArray, 0, sizeof(RenderFuncArray));

    s_uiID = _Register(s_uiRenderTypeHash, this);

    _InitPipelineFuncArray();
}

// FUNCTION: SWBF2 0x0070f060
pcDecalShader::pcDecalShader(unsigned int uiFlags,
                             const unsigned int* pParams,
                             unsigned int uiNumParams,
                             unsigned int uiMaterialFlags,
                             const RedColor& color0,
                             const RedColor& color1,
                             float fArg7)
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
    _Init(uiFlags, pParams, uiNumParams, uiMaterialFlags, color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x0070f120
void pcDecalShader::Begin_Fixed(unsigned int, unsigned int) const
{
    if (m_uiTextureHash)
    {
        RedRenderer::pcSetCullMode(PCREDCULL_NONE);
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_MODULATE2X);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 1);
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, 0x80808080);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

// FUNCTION: SWBF2 0x0070f290
void pcDecalShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_MODULATE2X);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 1);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

// FUNCTION: SWBF2 0x0070f370
void pcDecalShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x0070f390
void pcDecalShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070f3a0
void pcDecalShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070f3b0
unsigned int pcDecalShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x0070f3d0
RenderPhaseT pcDecalShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x0070f3e0
unsigned int pcDecalShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x0070f3f0
pcRedShaderDef* pcDecalShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x0070f400
unsigned int pcDecalShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x0070f410
unsigned int pcDecalShader::GetState_Prog1(const RenderItem*) const
{
    if (m_uiBumpHash)
    {
        return m_uiTextureHash ? 1 : 2;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0070f430
void pcDecalShader::InitFixedFunctionShaderDef() const
{
    // GLOBAL: SWBF2 0x00785370
    static const pcRedShaderDef::FixedFunctionStateT aFFStates[1] =
    {
        { 1, { 0x21, 0x00, 0x00, 0x00 } },
    };

    GetShaderDef()->InitFixedFunction(0x10, 1, aFFStates);
}

// FUNCTION: SWBF2 0x0070f450
void pcDecalShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    if (m_uiTextureHash)
    {
        RedRenderer::pcLoadFormat(m_pVertexFormat);
        RedRenderer::pcSetTexture(0, RedTexture::s_textureHashTable.Find(m_uiTextureHash));
        pItem->pPrim->Draw();
    }
}

// FUNCTION: SWBF2 0x0070f490
void pcDecalShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 1);
    const RedColorValue color(pItem->tweakColor);
    pRegs[39].SetFloats(color.r, color.g, color.b, color.a);
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    RedRenderer::pcLoadFormat(m_pVertexFormat);

    if (pItem->uiShaderState == 0)
    {
        RedRenderer::pcSetTexture(0, RedTexture::s_textureHashTable.Find(m_uiTextureHash));
    }
    else if (m_uiTextureHash)
    {
        RedRenderer::pcSetTexture(0, RedTexture::s_textureHashTable.Find(m_uiTextureHash));
        RedRenderer::pcSetTexture(1, RedTexture::s_textureHashTable.Find(m_uiBumpHash));
    }
    else
    {
        RedRenderer::pcSetTexture(0, RedTexture::s_textureHashTable.Find(m_uiBumpHash));
    }

    const RedColorValue& light = RedLight::m_sGlobalDirectionals[0]->m_color;
    RedColorValue afConstants[2];
    afConstants[0].r = 0.5f;
    afConstants[0].g = 0.5f;
    afConstants[0].b = 0.5f;
    afConstants[0].a = 0.5f;
    afConstants[1].r = light.r;
    afConstants[1].g = light.g;
    afConstants[1].b = light.b;
    afConstants[1].a = light.a;
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &afConstants[0].r, 2);

    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070f5e0
pcRedShader* pcDecalShader::_Create(unsigned int uiFlags,
                                    const unsigned int* pParams,
                                    unsigned int uiNumParams,
                                    unsigned int uiMaterialFlags,
                                    const RedColor& color0,
                                    const RedColor& color1,
                                    float fArg7) const
{
    return new pcDecalShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                             color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x0070f670
void pcDecalShader::_Init(unsigned int uiFlags,
                          const unsigned int* pParams,
                          unsigned int uiNumParams,
                          unsigned int uiMaterialFlags,
                          const RedColor&,
                          const RedColor&,
                          float)
{
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, uiFlags);
    m_uiTextureHash = uiNumParams ? pParams[0] : 0;
    m_uiBumpHash = uiNumParams ? pParams[1] : 0;
    m_uiMaterialFlags = uiMaterialFlags;
}

// FUNCTION: SWBF2 0x0070f6c0
void pcDecalShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcDecalShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcDecalShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcDecalShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcDecalShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcDecalShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcDecalShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcDecalShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcDecalShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x0070f720
void pcDecalShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x0070f800
void pcDecalShader::_StaticInit() const
{
}

// SYNTHETIC: SWBF2 0x0040a3f0
// `dynamic initializer for 'pcDecalShader::s_uiRenderTypeHash''

// GLOBAL: SWBF2 0x009ca6d0
// pcDecalShader::s_uiRenderTypeHash
unsigned int pcDecalShader::s_uiRenderTypeHash = PblHash("Decal");

// SYNTHETIC: SWBF2 0x0040a3b0
// `dynamic initializer for 'pcDecalShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769f60
// `dynamic atexit destructor for 'pcDecalShader::s_Factory''

// GLOBAL: SWBF2 0x009ca6a4
// pcDecalShader::s_Factory
pcDecalShader pcDecalShader::s_Factory;

// SYNTHETIC: SWBF2 0x0040a3d0
// `dynamic initializer for 'pcDecalShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769f80
// `dynamic atexit destructor for 'pcDecalShader::s_shaderDef''

// GLOBAL: SWBF2 0x009ca678
// pcDecalShader::s_shaderDef
pcRedShaderDef pcDecalShader::s_shaderDef("decal_shader.xml");

// SYNTHETIC: SWBF2 0x0040a450
// `dynamic initializer for 'kuiLinearRamp''

// GLOBAL: SWBF2 0x009ca66c
// kuiLinearRamp
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

// SYNTHETIC: SWBF2 0x0040a410
// PblMath::`dynamic initializer for 'fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x0040a430
// PblMath::`dynamic initializer for 'fOneOverTwoPi''

// GLOBAL: SWBF2 0x009ca668
// g_pcDecalShader
unsigned int g_pcDecalShader;
