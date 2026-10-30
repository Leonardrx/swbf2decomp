#include <string.h>
#include "pcNormalMapAdderShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcNormalMapAdderShader::s_uiID;
pcRedShader::GetStateFuncT pcNormalMapAdderShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcNormalMapAdderShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcNormalMapAdderShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcNormalMapAdderShader::RenderFuncArray[MAX_PIPELINES];

// SYNTHETIC: SWBF2 0x0040a150
// `dynamic initializer for 'pcNormalMapAdderShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769ef0
// `dynamic atexit destructor for 'pcNormalMapAdderShader::s_shaderDef''

// GLOBAL: SWBF2 0x009ca1b0
// pcNormalMapAdderShader::s_shaderDef
pcRedShaderDef pcNormalMapAdderShader::s_shaderDef("normalmapadder_shader.xml");

// SYNTHETIC: SWBF2 0x0040a1d0
// `dynamic initializer for 'kuiHemisphereNormalizationMap''

// GLOBAL: SWBF2 0x009ca1e4
// kuiHemisphereNormalizationMap
static unsigned int kuiHemisphereNormalizationMap = PblHash("hemisphere_normalizationmap");

// FUNCTION: SWBF2 0x0070b4e0
pcNormalMapAdderShader::pcNormalMapAdderShader()
{

    memset(GetStateFuncArray, 0, sizeof(GetStateFuncArray));
    memset(BeginFuncArray, 0, sizeof(BeginFuncArray));
    memset(EndFuncArray, 0, sizeof(EndFuncArray));
    memset(RenderFuncArray, 0, sizeof(RenderFuncArray));

    s_uiID = _Register(s_uiRenderTypeHash, this);

    _InitPipelineFuncArray();
}

// FUNCTION: SWBF2 0x0070b5b0
pcNormalMapAdderShader::pcNormalMapAdderShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x0070b680
void pcNormalMapAdderShader::Begin_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070b690
void pcNormalMapAdderShader::Begin_Prog1(unsigned int uiBlend, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    if (!uiBlend)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    RedRenderer::pcSetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
}

// FUNCTION: SWBF2 0x0070b770
void pcNormalMapAdderShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x0070b790
void pcNormalMapAdderShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070b7a0
void pcNormalMapAdderShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070b7b0
unsigned int pcNormalMapAdderShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x0070b7d0
RenderPhaseT pcNormalMapAdderShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x0070b7e0
unsigned int pcNormalMapAdderShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x0070b7f0
pcRedShaderDef* pcNormalMapAdderShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x0070b800
unsigned int pcNormalMapAdderShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x0070b810
unsigned int pcNormalMapAdderShader::GetState_Prog1(const RenderItem*) const
{
    return m_iShaderState;
}

// FUNCTION: SWBF2 0x0070b820
void pcNormalMapAdderShader::InitFixedFunctionShaderDef() const
{
    // GLOBAL: SWBF2 0x00785134
    static const pcRedShaderDef::FixedFunctionStateT aFFStates[1] =
    {
        { 1, { 0x00, 0x00, 0x00, 0x00 } },
    };

    GetShaderDef()->InitFixedFunction(0x10, 1, aFFStates);
}

// FUNCTION: SWBF2 0x0070b840
void pcNormalMapAdderShader::Render_Fixed(const RenderItem*, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070b850
void pcNormalMapAdderShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    RedRenderer::pcSetTexture(0, m_normalMap1);
    if (m_normalMap2)
    {
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTexture(1, m_normalMap2);
        const D3DXVECTOR4 blend(m_blend, m_blend, m_blend, m_blend);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(3, &blend.x, 1);
    }
    else
    {
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTexture(1, 0);
        RedRenderer::pcSetTexture(2, RedTexture::Find(kuiHemisphereNormalizationMap));
    }

    if (m_iShaderState == 0)
    {
        pcRedVertexShaderConstantRegister* pRegs =
            RedRenderer::pcLockVertexShaderConstants(40, 4);
        pRegs[40].SetFloats(m_textureTransform1(0, 0), m_textureTransform1(1, 0), 0.0f, m_textureTransform1(2, 0));
        pRegs[41].SetFloats(m_textureTransform1(0, 1), m_textureTransform1(1, 1), 0.0f, m_textureTransform1(2, 1));
        pRegs[42].SetFloats(m_textureTransform2(0, 0), m_textureTransform2(1, 0), 0.0f, m_textureTransform2(2, 0));
        pRegs[43].SetFloats(m_textureTransform2(0, 1), m_textureTransform2(1, 1), 0.0f, m_textureTransform2(2, 1));
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    }

    RedRenderer::pcLoadFormat(m_pVertexFormat[m_iShaderState]);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070bac0
pcRedShader* pcNormalMapAdderShader::_Create(unsigned int uiFlags,
                                             const unsigned int* pParams,
                                             unsigned int uiNumParams,
                                             unsigned int uiMaterialFlags,
                                             const RedColor& color0,
                                             const RedColor& color1,
                                             float fArg7) const
{
    return new pcNormalMapAdderShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                      color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x0070bb50
void pcNormalMapAdderShader::_Init(unsigned int uiFlags,
                                   const unsigned int*,
                                   unsigned int,
                                   unsigned int,
                                   const RedColor&,
                                   const RedColor&,
                                   float)
{
    m_pVertexFormat[0] = pcRedVertexFormat::Create(uiFlags, 0x202);
    m_pVertexFormat[1] = pcRedVertexFormat::Create(uiFlags, 0x2e2);
    m_normalMap1 = 0;
    m_normalMap2 = 0;
    m_textureTransform1.m_bTransform = 0;
    m_textureTransform2.m_bTransform = 0;
    m_iShaderState = 0;
    m_blend = 0.5f;
}

// FUNCTION: SWBF2 0x0070bbb0
void pcNormalMapAdderShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcNormalMapAdderShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcNormalMapAdderShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcNormalMapAdderShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcNormalMapAdderShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcNormalMapAdderShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcNormalMapAdderShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcNormalMapAdderShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcNormalMapAdderShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x0070bc10
void pcNormalMapAdderShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x0070bcf0
void pcNormalMapAdderShader::_StaticInit() const
{
}

// SYNTHETIC: SWBF2 0x0040a170
// `dynamic initializer for 'pcNormalMapAdderShader::s_uiRenderTypeHash''

// GLOBAL: SWBF2 0x009ca254
// pcNormalMapAdderShader::s_uiRenderTypeHash
unsigned int pcNormalMapAdderShader::s_uiRenderTypeHash = PblHash("NormalMapAdder");

// SYNTHETIC: SWBF2 0x0040a190
// PblMath::`dynamic initializer for 'fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x0040a1b0
// PblMath::`dynamic initializer for 'fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x0040a130
// `dynamic initializer for 'pcNormalMapAdderShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769ed0
// `dynamic atexit destructor for 'pcNormalMapAdderShader::s_Factory''

// GLOBAL: SWBF2 0x009ca1e8
// pcNormalMapAdderShader::s_Factory
pcNormalMapAdderShader pcNormalMapAdderShader::s_Factory;

// GLOBAL: SWBF2 0x009ca1ac
unsigned int g_pcNormalMapAdderShader;
