#include <string.h>
#include "pcOceanShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedTextureTransform.h"
#include "PebbleFL/Common/PblMatrix.h"

void __cdecl PblTraceP(const char* Format, ...);

static const float kInv255 = 1.0f / 255.0f;

unsigned int pcOceanShader::s_uiID;
pcRedShader::GetStateFuncT pcOceanShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcOceanShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcOceanShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcOceanShader::RenderFuncArray[MAX_PIPELINES];

// SYNTHETIC: SWBF2 0x0040a210
// `dynamic initializer for 'pcOceanShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769f20
// `dynamic atexit destructor for 'pcOceanShader::s_shaderDef''

// GLOBAL: SWBF2 0x009ca330
// pcOceanShader::s_shaderDef
pcRedShaderDef pcOceanShader::s_shaderDef("ocean_shader.xml");

// SYNTHETIC: SWBF2 0x0040a290
// `dynamic initializer for 'kuiCubeNormalizationMap''

// GLOBAL: SWBF2 0x009ca32c
// kuiCubeNormalizationMap
static unsigned int kuiCubeNormalizationMap = PblHash("cube_normalizationmap");

// SYNTHETIC: SWBF2 0x0040a2b0
// `dynamic initializer for 'kuiLinearRamp''

// GLOBAL: SWBF2 0x009ca294
// kuiLinearRamp
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

// FUNCTION: SWBF2 0x0070bd00
pcOceanShader::pcOceanShader()
{

    memset(GetStateFuncArray, 0, sizeof(GetStateFuncArray));
    memset(BeginFuncArray, 0, sizeof(BeginFuncArray));
    memset(EndFuncArray, 0, sizeof(EndFuncArray));
    memset(RenderFuncArray, 0, sizeof(RenderFuncArray));

    s_uiID = _Register(s_uiRenderTypeHash, this);

    _InitPipelineFuncArray();
}

// FUNCTION: SWBF2 0x0070bdd0
pcOceanShader::pcOceanShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x0070bea0
void pcOceanShader::Begin_Fixed(unsigned int uiState, unsigned int uiPass) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    switch (uiState)
    {
    case 0:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                      D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                          D3DCOLORWRITEENABLE_BLUE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATECOLOR_ADDALPHA);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 1:
        if (uiPass == 0)
        {
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATECOLOR_ADDALPHA);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
            RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
            RedRenderer::pcSetTexture(0, RedTexture::Find(kuiLinearRamp));
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX,
                                                D3DTSS_TCI_CAMERASPACEPOSITION);
            RedRenderer::pcFFSetTextureTransform(
                0, RedRenderer::pcGetNearSceneFadeTextureTransform());
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0070c180
void pcOceanShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

// FUNCTION: SWBF2 0x0070c250
void pcOceanShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x0070c270
void pcOceanShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070c280
void pcOceanShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070c290
unsigned int pcOceanShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x0070c2b0
RenderPhaseT pcOceanShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x0070c2c0
unsigned int pcOceanShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x0070c2d0
pcRedShaderDef* pcOceanShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x0070c2e0
unsigned int pcOceanShader::GetState_Fixed(const RenderItem*) const
{
    return RedRenderer::pcIsNearScene() ? 1 : 0;
}

// FUNCTION: SWBF2 0x0070c300
unsigned int pcOceanShader::GetState_Prog1(const RenderItem*) const
{
    return RedRenderer::pcIsNearScene() ? 1 : 0;
}

// FUNCTION: SWBF2 0x0070c320
unsigned int pcOceanShader::GetZPrepassState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x0070c330
void pcOceanShader::InitFixedFunctionShaderDef() const
{
    // GLOBAL: SWBF2 0x007851e4
    static const pcRedShaderDef::FixedFunctionStateT aFFStates[2] =
    {
        { 1, { 0x42, 0x00, 0x00, 0x00 } },
        { 2, { 0x42, 0x21, 0x00, 0x00 } },
    };

    GetShaderDef()->InitFixedFunction(0x14, 2, aFFStates);
}

// FUNCTION: SWBF2 0x0070c350
void pcOceanShader::RenderZPrepass(const RenderItem* pItem) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcLoadFormat(m_pZPrepassVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070c580
void pcOceanShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    // GLOBAL: SWBF2 0x009ca35c
    static RedColor s_colorUnused = RedColor::BLACK;

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 5);
    const RedColorValue tweak(pItem->tweakColor);
    pRegs[39].SetFloats(tweak.r, tweak.g, tweak.b, tweak.a);

    pRegs[40].SetFloats(m_textureTransform(0, 0), m_textureTransform(1, 0), 0.0f,
                        m_textureTransform(2, 0));
    pRegs[41].SetFloats(m_textureTransform(0, 1), m_textureTransform(1, 1), 0.0f,
                        m_textureTransform(2, 1));

    pRegs[42].SetFloats(m_foamMapTransform(0, 0), m_foamMapTransform(1, 0), 0.0f,
                        m_foamMapTransform(2, 0));
    pRegs[43].SetFloats(m_foamMapTransform(0, 1), m_foamMapTransform(1, 1), 0.0f,
                        m_foamMapTransform(2, 1));

    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    const RedDirectionalLight* pSun = RedLight::m_sGlobalDirectionals[0];
    const RedColorValue lightDir(-pSun->m_vDirection.x, -pSun->m_vDirection.y,
                                 -pSun->m_vDirection.z, 0.0f);
    const RedColorValue lightColor(pSun->m_color);
    const RedColorValue ambient(RedLight::m_sGlobalAmbient->m_color);

    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &lightDir.r, 1);
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &lightColor.r, 1);
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(2, &ambient.r, 1);

    RedRenderer::pcSetTexture(0, m_baseMap);
    RedRenderer::pcSetTexture(1, m_foamMap);
    RedRenderer::pcSetTexture(2, RedTexture::Find(kuiCubeNormalizationMap));
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070c8b0
pcRedShader* pcOceanShader::_Create(unsigned int uiFlags,
                                    const unsigned int* pParams,
                                    unsigned int uiNumParams,
                                    unsigned int uiMaterialFlags,
                                    const RedColor& color0,
                                    const RedColor& color1,
                                    float fArg7) const
{
    return new pcOceanShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                             color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x0070c940
void pcOceanShader::_Init(unsigned int uiFlags,
                          const unsigned int*,
                          unsigned int,
                          unsigned int,
                          const RedColor&,
                          const RedColor&,
                          float)
{
    m_transmissiveColor = RedColorValue(0.0f, 0.0f, 0.0f, 0.0f);
    m_foamMap = 0;
    m_reflectionMap = 0;
    m_textureTransform.m_bTransform = 0;
    m_foamMapTransform.m_bTransform = 0;
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, 0x2a2);
    m_pZPrepassVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 2);
}

// FUNCTION: SWBF2 0x0070c990
void pcOceanShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcOceanShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcOceanShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcOceanShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcOceanShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcOceanShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcOceanShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcOceanShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcOceanShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x0070c9f0
void pcOceanShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x0070cad0
void pcOceanShader::_StaticInit() const
{
}

// SYNTHETIC: SWBF2 0x0040a230
// `dynamic initializer for 'pcOceanShader::s_uiRenderTypeHash''

// GLOBAL: SWBF2 0x009ca328
// pcOceanShader::s_uiRenderTypeHash
unsigned int pcOceanShader::s_uiRenderTypeHash = PblHash("Ocean");

// SYNTHETIC: SWBF2 0x0040a250
// PblMath::`dynamic initializer for 'fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x0040a270
// PblMath::`dynamic initializer for 'fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x0040a1f0
// `dynamic initializer for 'pcOceanShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769f00
// `dynamic atexit destructor for 'pcOceanShader::s_Factory''

// GLOBAL: SWBF2 0x009ca298
// pcOceanShader::s_Factory
pcOceanShader pcOceanShader::s_Factory;

static PblMatrix OceanTextureMatrix(const RedTextureTransform& t)
{
    PblMatrix m;
    m.m[0][0] = t(0, 0); m.m[0][1] = t(0, 1); m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
    m.m[1][0] = t(1, 0); m.m[1][1] = t(1, 1); m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
    m.m[2][0] = t(2, 0); m.m[2][1] = t(2, 1); m.m[2][2] = 0.0f; m.m[2][3] = 0.0f;
    m.m[3][0] = 0.0f;    m.m[3][1] = 0.0f;    m.m[3][2] = 0.0f; m.m[3][3] = 1.0f;
    return m;
}

// FUNCTION: SWBF2 0x0070c380
void pcOceanShader::Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const
{
    if (uiPass == 0)
    {
        const D3DXMATRIXA16 mBase(m_textureTransform(0, 0), m_textureTransform(0, 1), 0.0f, 0.0f,
                                  m_textureTransform(1, 0), m_textureTransform(1, 1), 0.0f, 0.0f,
                                  m_textureTransform(2, 0), m_textureTransform(2, 1), 0.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f);
        const D3DXMATRIXA16 mFoam(m_foamMapTransform(0, 0), m_foamMapTransform(0, 1), 0.0f, 0.0f,
                                  m_foamMapTransform(1, 0), m_foamMapTransform(1, 1), 0.0f, 0.0f,
                                  m_foamMapTransform(2, 0), m_foamMapTransform(2, 1), 0.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f);
        RedRenderer::pcFFSetTextureTransform(0, reinterpret_cast<const PblMatrix&>(mFoam));
        RedRenderer::pcFFSetTextureTransform(1, reinterpret_cast<const PblMatrix&>(mBase));
        RedRenderer::pcSetTexture(0, m_foamMap);
        RedRenderer::pcSetTexture(1, m_baseMap);
    }
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// GLOBAL: SWBF2 0x009ca290
unsigned int g_pcOceanShader;
