#include <math.h>
#include <string.h>

#include "pcShieldShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "PebbleFL/Common/PblMatrix.h"

void __cdecl PblTraceP(const char* Format, ...);

extern unsigned int kuiLinearRamp;

static const float kInv255 = 1.0f / 255.0f;
static const float kfSpecularSizeInv = 0.3f;

unsigned int pcShieldShader::s_uiID;
pcRedShader::GetStateFuncT pcShieldShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcShieldShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcShieldShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcShieldShader::RenderFuncArray[MAX_PIPELINES];
float pcShieldShader::s_fFadeStart;
float pcShieldShader::s_fFadeEnd;

// SYNTHETIC: SWBF2 0x00407f20
// `dynamic initializer for 'pcShieldShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769910
// `dynamic atexit destructor for 'pcShieldShader::s_shaderDef''
pcRedShaderDef pcShieldShader::s_shaderDef("shield_shader.xml");

PblVector3 pcShieldShader::s_vLightDir;
PblVector3 pcShieldShader::s_vLightX;
PblVector3 pcShieldShader::s_vLightY;

static D3DMATERIAL9 s_d3dMat;

// SYNTHETIC: SWBF2 0x00407fc0
// `dynamic initializer for 'kuiSpecularSpotMap''

// GLOBAL: SWBF2 0x00965048
// kuiSpecularSpotMap
static unsigned int kuiSpecularSpotMap = PblHash("specularspot");

// FUNCTION: SWBF2 0x006ccce0
pcShieldShader::pcShieldShader()
{
    memset(GetStateFuncArray, 0, sizeof(GetStateFuncArray));
    memset(BeginFuncArray, 0, sizeof(BeginFuncArray));
    memset(EndFuncArray, 0, sizeof(EndFuncArray));
    memset(RenderFuncArray, 0, sizeof(RenderFuncArray));

    s_uiID = _Register(s_uiRenderTypeHash, this);

    _InitPipelineFuncArray();
}

// FUNCTION: SWBF2 0x006cdda0
PblMatrix pcShieldShader::_GetNearSceneFadeTextureTransform()
{
    float fScale, fOffset;
    if (RedRenderer::pcIsNearScene())
    {
        fScale = 1.0f / (pcShieldShader::s_fFadeEnd - pcShieldShader::s_fFadeStart);
        fOffset = -(pcShieldShader::s_fFadeStart * fScale);
    }
    else
    {
        const float fInv = 1.0f / (pcShieldShader::s_fFadeEnd - pcShieldShader::s_fFadeStart);
        fOffset = pcShieldShader::s_fFadeEnd * fInv;
        fScale = -fInv;
    }
    PblMatrix m;
    m.m[3][3] = 1.0f; m.m[3][2] = 0.0f; m.m[3][1] = 0.0f; m.m[3][0] = fOffset;
    m.m[2][3] = 0.0f; m.m[2][2] = 0.0f; m.m[2][1] = 0.0f; m.m[2][0] = fScale;
    m.m[1][3] = 0.0f; m.m[1][2] = 0.0f; m.m[1][1] = 0.0f; m.m[1][0] = 0.0f;
    m.m[0][3] = 0.0f; m.m[0][2] = 0.0f; m.m[0][1] = 0.0f; m.m[0][0] = 0.0f;
    return m;
}

// FUNCTION: SWBF2 0x006cce60
void pcShieldShader::Begin_Fixed(unsigned int uiState, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                      D3DCOLORWRITEENABLE_BLUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    switch (uiState)
    {
    case 0:
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
        RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_LIGHTING, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SPECULARENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
        RedRenderer::pcSetTexture(0, RedTexture::Find(kuiLinearRamp));
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcFFSetTextureTransform(0, pcShieldShader::_GetNearSceneFadeTextureTransform());
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE | D3DTA_ALPHAREPLICATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADDSMOOTH);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;
    case 1:
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 1);
        RedRenderer::pcSetRenderState(D3DRS_LIGHTING, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SPECULARENABLE, TRUE);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetTexture(1, RedTexture::Find(kuiLinearRamp));
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcFFSetTextureTransform(1, pcShieldShader::_GetNearSceneFadeTextureTransform());
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, 0xff606060);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_SPECULAR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;
    }
}

// FUNCTION: SWBF2 0x006cd550
void pcShieldShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    bool bLight = false;
    switch (pItem->uiShaderState)
    {
    case 1:
    {
        RedRenderer::pcSetTexture(0, RedTexture::Find(m_uiTextureHash));
        RedRenderer::pcFFSetTextureTransform(0, PblMatrix(PblVector3(1.0f, 0.0f, 0.0f), PblVector3(0.0f, 1.0f, 0.0f), PblVector3(m_fOffsetU, m_fOffsetV, 0.0f), PblVector3(0.0f, 0.0f, 0.0f)));
        bLight = true;
        break;
    }
    case 0:
    {
        RedColor color = pItem->tweakColor * m_colorDiffuse;
        const unsigned char a = (unsigned char)(int)((1.0f - m_fViewAngleFactor) * 255.0f);
        color.r >>= 1;
        color.g >>= 1;
        color.b >>= 1;
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(a, color.r, color.g, color.b));
        bLight = true;
        break;
    }
    }
    if (bLight)
    {
        RedRenderer::pcSetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(RedColor::BLACK.a, RedColor::BLACK.r,
                                                                   RedColor::BLACK.g, RedColor::BLACK.b));
        D3DLIGHT9 light = RedLight::m_sGlobalDirectionals[0]->m_d3dLight;
        light.Specular.r *= 2.0f;
        light.Specular.g *= 2.0f;
        light.Specular.b *= 2.0f;
        RedRenderer::pcGetDevice()->SetLight(0, &light);
        RedRenderer::pcGetDevice()->LightEnable(0, TRUE);
        D3DCOLORVALUE specular;
        specular.r = m_colorSpecular.r * kInv255;
        specular.g = m_colorSpecular.g * kInv255;
        specular.b = m_colorSpecular.b * kInv255;
        specular.a = m_colorSpecular.a * kInv255;
        s_d3dMat.Specular = specular;
        s_d3dMat.Power = m_specPower;
        RedRenderer::pcGetDevice()->SetMaterial(&s_d3dMat);
    }
    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
    if (bLight)
    {
        RedRenderer::pcGetDevice()->LightEnable(0, FALSE);
    }
}

// FUNCTION: SWBF2 0x006cd1c0
void pcShieldShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                      D3DCOLORWRITEENABLE_BLUE);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 1);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    RedRenderer::pcSetTexture(1, RedTexture::Find(kuiSpecularSpotMap));

    const PblVector3& vDir = RedLight::m_sGlobalDirectionals[0]->m_vDirection;
    if (s_vLightDir.x != vDir.x || s_vLightDir.y != vDir.y || s_vLightDir.y != vDir.z)
    {
        s_vLightDir = vDir;
        s_vLightX = PblVector3(s_vLightDir.z, 0.0f, -s_vLightDir.x);

        const float fLenSq = s_vLightX.x * s_vLightX.x + s_vLightX.y * s_vLightX.y +
                             s_vLightX.z * s_vLightX.z;
        if (fLenSq >= 1e-6f)
        {
            const float fInvLen = 1.0f / sqrtf(fLenSq);
            s_vLightX *= fInvLen;
        }
        else
        {
            s_vLightX = PblVector3(1.0f, 0.0f, 0.0f);
        }

        s_vLightY = PblVector3(s_vLightDir.y * s_vLightX.z - s_vLightDir.z * s_vLightX.y,
                               s_vLightDir.z * s_vLightX.x - s_vLightDir.x * s_vLightX.z,
                               s_vLightDir.x * s_vLightX.y - s_vLightDir.y * s_vLightX.x);
    }
}

// FUNCTION: SWBF2 0x006cd460
void pcShieldShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x006cd480
RedShadingState* pcShieldShader::CreateShadingState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x006cd490
void pcShieldShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006cd4a0
void pcShieldShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006cd4b0
unsigned int pcShieldShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x006cd4d0
RenderPhaseT pcShieldShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x006cd4e0
unsigned int pcShieldShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x006cd4f0
pcRedShaderDef* pcShieldShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x006cd500
unsigned int pcShieldShader::GetState_Fixed(const RenderItem* pItem) const
{
    return (pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF >> 9) & 1;
}

// FUNCTION: SWBF2 0x006cd520
unsigned int pcShieldShader::GetState_Prog1(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x006cd530
void pcShieldShader::InitFixedFunctionShaderDef() const
{
    // GLOBAL: SWBF2 0x00772f30
    static const pcRedShaderDef::FixedFunctionStateT aFFStates[2] =
    {
        { 1, { 0x141, 0x00, 0x00, 0x00 } },
        { 1, { 0x241, 0x00, 0x00, 0x00 } },
    };

    GetShaderDef()->InitFixedFunction(0x28, 2, aFFStates);
}

// FUNCTION: SWBF2 0x006cd8e0
void pcShieldShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    const RedColor color = pItem->tweakColor * m_colorDiffuse;

    RedRenderer::pcSetTexture(0, m_uiTextureHash ? RedTexture::Find(m_uiTextureHash) : 0);

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 6);
    const RedColorValue diffuse(color);
    pRegs[39].SetFloats(diffuse.r, diffuse.g, diffuse.b, diffuse.a);

    if (m_specPower < 4.0f)
    {
        const float fScale = m_specPower * 0.25f;
        const unsigned char sb = (unsigned char)(int)(m_colorSpecular.b * fScale);
        const unsigned char sg = (unsigned char)(int)(m_colorSpecular.g * fScale);
        const unsigned char sr = (unsigned char)(int)(m_colorSpecular.r * fScale);
        pRegs[40].component[3] = 1.0f;
        pRegs[40].SetFloats(sr * kInv255, sg * kInv255, sb * kInv255);
    }
    else
    {
        const RedColorValue specular(m_colorSpecular);
        pRegs[40].SetFloats(specular.r, specular.g, specular.b, specular.a);
    }

    const PblVector3 vSpecX = s_vLightX * m_specSizeInv;
    pRegs[41].SetFloats(vSpecX.x, vSpecX.y, vSpecX.z);
    const PblVector3 vSpecY = m_specSizeInv * s_vLightY;
    pRegs[42].SetFloats(vSpecY.x, vSpecY.y, vSpecY.z);

    pRegs[43].SetFloats(m_fOffsetU, m_fOffsetV, m_fViewAngleFactor, m_fVertexMinDistSqr);

    const float fFadeStart = s_fFadeStart;
    const float fFadeEnd = s_fFadeEnd;
    if (fFadeStart > 0.0f && fFadeEnd > fFadeStart)
    {
        const float fInvRange = 1.0f / (fFadeEnd - fFadeStart);
        pRegs[44].component[2] = 0.0f;
        pRegs[44].component[3] = 0.0f;
        if (pItem->flags & 0x20000)
        {
            pRegs[44].component[0] = fInvRange;
            pRegs[44].component[1] = -(fFadeStart * fInvRange);
        }
        else
        {
            pRegs[44].component[0] = -fInvRange;
            pRegs[44].component[1] = fFadeEnd * fInvRange;
        }
    }
    else
    {
        pRegs[44].component[0] = 0.0f;
        pRegs[44].component[1] = 1.0f;
        pRegs[44].component[2] = 0.0f;
        pRegs[44].component[3] = 0.0f;
    }

    RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x006cdcd0
void pcShieldShader::SetSpecularPower(float fPower)
{
    m_specPower = fPower;
    m_specSizeInv = sqrtf(fPower) * kfSpecularSizeInv;
}

// FUNCTION: SWBF2 0x006ccda0
pcShieldShader::pcShieldShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x006cde70
void pcShieldShader::_Init(unsigned int uiFlags,
                           const unsigned int* pParams,
                           unsigned int uiNumParams,
                           unsigned int,
                           const RedColor& color0,
                           const RedColor& color1,
                           float)
{
    unsigned int uiInputRVF = 0x22;
    if (uiFlags & 0x80)
    {
        uiInputRVF = 0xa2;
    }
    else if (uiFlags & 0x200)
    {
        uiInputRVF = 0x222;
    }

    if (uiNumParams > 0 && pParams[0])
    {
        m_uiTextureHash = pParams[0];
    }
    else
    {
        m_uiTextureHash = 0;
    }

    m_uiMaterialFlags = 0x10044;
    m_colorDiffuse = color0;
    m_colorSpecular = color1;
    SetSpecularPower(32.0f);
    m_fVertexMinDistSqr = 0.0f;
    m_fViewAngleFactor = 0.0f;
    m_fOffsetV = 0.0f;
    m_fOffsetU = 0.0f;
    m_pVertexInputFormat = pcRedVertexFormat::Create(uiFlags, uiInputRVF);

    s_vLightDir = PblVector3(0.0f, 0.0f, 0.0f);
    s_fFadeEnd = 0.0f;
    s_fFadeStart = 0.0f;
    memset(&s_d3dMat, 0, sizeof(s_d3dMat));
}

// FUNCTION: SWBF2 0x006cdd10
pcRedShader* pcShieldShader::_Create(unsigned int uiFlags,
                                     const unsigned int* pParams,
                                     unsigned int uiNumParams,
                                     unsigned int uiMaterialFlags,
                                     const RedColor& color0,
                                     const RedColor& color1,
                                     float fArg7) const
{
    return new pcShieldShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                              color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x006cdf60
void pcShieldShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcShieldShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcShieldShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcShieldShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcShieldShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcShieldShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcShieldShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcShieldShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcShieldShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x006cdfc0
void pcShieldShader::_Read(PblFileChunk& chunk) const
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

// SYNTHETIC: SWBF2 0x00407f40
// `dynamic initializer for 'pcShieldShader::s_uiRenderTypeHash''
unsigned int pcShieldShader::s_uiRenderTypeHash = PblHash("Shield");

// SYNTHETIC: SWBF2 0x00407f00
// `dynamic initializer for 'pcShieldShader::s_Factory''
// SYNTHETIC: SWBF2 0x007698f0
// `dynamic atexit destructor for 'pcShieldShader::s_Factory''

// GLOBAL: SWBF2 0x00964fc8
// pcShieldShader::s_Factory
pcShieldShader pcShieldShader::s_Factory;

// GLOBAL: SWBF2 0x00964fc0
unsigned int g_pcShieldShader;

// SYNTHETIC: SWBF2 0x00407fa0
// `dynamic initializer for 'kuiLinearRamp''

// GLOBAL: SWBF2 0x00964fc4
// kuiLinearRamp
unsigned int kuiLinearRamp = PblHash("linear_ramp");
