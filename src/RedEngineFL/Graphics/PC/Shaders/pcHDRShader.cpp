#include <string.h>
#include "pcHDRShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

void __cdecl PblTraceP(const char* Format, ...);

// GLOBAL: SWBF2 0x009caabc
RedTexture* s_pTextureWhite;

unsigned int pcHDRShader::s_uiID;
pcRedShader::GetStateFuncT pcHDRShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcHDRShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcHDRShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcHDRShader::RenderFuncArray[MAX_PIPELINES];
// SYNTHETIC: SWBF2 0x0040a820
// `dynamic initializer for 'pcHDRShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x0076a070
// `dynamic atexit destructor for 'pcHDRShader::s_shaderDef''
pcRedShaderDef pcHDRShader::s_shaderDef("hdr_shader.xml");

// FUNCTION: SWBF2 0x007138b0
pcHDRShader::pcHDRShader()
{
    memset(GetStateFuncArray, 0, sizeof(GetStateFuncArray));
    memset(BeginFuncArray, 0, sizeof(BeginFuncArray));
    memset(EndFuncArray, 0, sizeof(EndFuncArray));
    memset(RenderFuncArray, 0, sizeof(RenderFuncArray));

    s_uiID = _Register(s_uiRenderTypeHash, this);

    _InitPipelineFuncArray();
}

// FUNCTION: SWBF2 0x00713970
pcHDRShader::pcHDRShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00713a30
void pcHDRShader::Begin_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00713a40
void pcHDRShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAXMIPLEVEL, 0);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MAXMIPLEVEL, 0);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MAXMIPLEVEL, 0);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
}

// FUNCTION: SWBF2 0x00713b80
void pcHDRShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00713ba0
void pcHDRShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00713bb0
void pcHDRShader::End_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetTexture(0, 0);
    RedRenderer::pcSetTexture(1, 0);
    RedRenderer::pcSetTexture(2, 0);
    RedRenderer::pcSetTexture(3, 0);
}

// FUNCTION: SWBF2 0x00713be0
unsigned int pcHDRShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00713c00
RenderPhaseT pcHDRShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x00713c10
unsigned int pcHDRShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00713c20
pcRedShaderDef* pcHDRShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x00713c30
unsigned int pcHDRShader::GetState_Fixed(const RenderItem*) const
{
    return m_state;
}

// FUNCTION: SWBF2 0x00713c40
unsigned int pcHDRShader::GetState_Prog1(const RenderItem*) const
{
    return m_state;
}

// FUNCTION: SWBF2 0x00713c50
void pcHDRShader::InitFixedFunctionShaderDef() const
{
    // GLOBAL: SWBF2 0x00785790
    static const pcRedShaderDef::FixedFunctionStateT aFFStates[1] =
    {
        { 1, { 0x11, 0x00, 0x00, 0x00 } },
    };

    GetShaderDef()->InitFixedFunction(0, 1, aFFStates);
}

// FUNCTION: SWBF2 0x00713c70
void pcHDRShader::Render_Fixed(const RenderItem*, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00713c80
void pcHDRShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    RedRenderer::pcLoadFormat(m_pVertexFormat);

    float fDestTexelX;
    float fDestTexelY;
    if (m_destTex)
    {
        fDestTexelX = 1.0f / (float)m_destTex->m_uiWidth;
        fDestTexelY = 1.0f / (float)m_destTex->m_uiHeight;
    }
    else
    {
        const D3DPRESENT_PARAMETERS& pp = RedRenderer::pcGetPresentParameters();
        fDestTexelX = 1.0f / (float)pp.BackBufferWidth;
        fDestTexelY = 1.0f / (float)pp.BackBufferHeight;
    }

    pcRedVertexShaderConstantRegister* pRegs =
        RedRenderer::pcLockVertexShaderConstants(40, 3);
    pRegs[40].SetFloats(1.0f / (float)m_sourceTex->m_uiWidth, 1.0f / (float)m_sourceTex->m_uiHeight, 0.0f, 0.0f);
    pRegs[41].SetFloats(fDestTexelX, fDestTexelY, fDestTexelX * 0.5f, fDestTexelY * 0.5f);
    pRegs[42].SetFloats(1.0f, 1.0f, 1.0f / (float)m_sourceTex->m_uiWidth * m_bloomOffset,
                        1.0f / (float)m_sourceTex->m_uiHeight * m_bloomOffset);
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    const float afPSConstants[12] =
    {
        0.3f, 0.59f, 0.11f, m_glowThreshold,
        m_glowFactor, m_glowFactor, m_glowFactor, m_bloomWeight,
        1.0f, 1.0f, 1.0f, m_glowFactor
    };
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, afPSConstants, 3);

    if (m_state == HDR_MODE_SCALEANDBLOOM)
    {
        RedRenderer::pcSetTexture(0, RedTexture::Find(0xde020766));
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
        pItem->pPrim->Draw();

        RedRenderer::pcSetTexture(0, m_sourceTex);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        pItem->pPrim->Draw();
    }
    else
    {
        RedTexture* pOldTarget = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pOldDepth = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);

        RedRenderer::pcSetRenderTarget(m_destTex, 0);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetTexture(0, m_sourceTex);
        RedRenderer::pcSetTexture(1, m_sourceTex);
        RedRenderer::pcSetTexture(2, m_sourceTex);
        RedRenderer::pcSetTexture(3, m_sourceTex);
        pItem->pPrim->Draw();

        RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
        RedRenderer::pcSetViewport(viewport);
    }
}

// FUNCTION: SWBF2 0x00713fd0
pcRedShader* pcHDRShader::_Create(unsigned int uiFlags,
                                  const unsigned int* pParams,
                                  unsigned int uiNumParams,
                                  unsigned int uiMaterialFlags,
                                  const RedColor& color0,
                                  const RedColor& color1,
                                  float fArg7) const
{
    return new pcHDRShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                           color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00714060
void pcHDRShader::_Init(unsigned int uiFlags,
                        const unsigned int*,
                        unsigned int,
                        unsigned int,
                        const RedColor&,
                        const RedColor&,
                        float)
{
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, uiFlags);
    m_sourceTex = 0;
    m_state = HDR_MODE_CREATEGLOWMASK;
    m_bloomWeight = 0.25f;
    m_bloomOffset = 0.5f;
    m_glowFactor = 1.0f;
    s_pTextureWhite = RedTexture::s_textureHashTable.Find(PblHash("white"));
}

// FUNCTION: SWBF2 0x007140d0
void pcHDRShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcHDRShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcHDRShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcHDRShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcHDRShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcHDRShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcHDRShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcHDRShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcHDRShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00714130
void pcHDRShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00714210
void pcHDRShader::_StaticInit() const
{
}

// SYNTHETIC: SWBF2 0x0040a840
// `dynamic initializer for 'pcHDRShader::s_uiRenderTypeHash''
unsigned int pcHDRShader::s_uiRenderTypeHash = PblHash("hdr");

// SYNTHETIC: SWBF2 0x0040a800
// `dynamic initializer for 'pcHDRShader::s_Factory''
// SYNTHETIC: SWBF2 0x0076a050
// `dynamic atexit destructor for 'pcHDRShader::s_Factory''

// GLOBAL: SWBF2 0x009caa80
// pcHDRShader::s_Factory
pcHDRShader pcHDRShader::s_Factory;

// GLOBAL: SWBF2 0x009caa44
unsigned int g_pcHDRShader;
