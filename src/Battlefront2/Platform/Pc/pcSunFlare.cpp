#include <d3dx9math.h>
#include "Battlefront2/Source/SunFlare.h"
#include "pcSunFlare.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcFlareShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcSampleShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedCamera.h"

// GLOBAL: SWBF2 0x007b2750
// __xmm@00000010000000100000000000000000
// GLOBAL: SWBF2 0x007b2910
// __xmm@41700000000000000000000000000000

namespace FLRenderer
{
float GetNearSceneFadeStart();
float GetFarSceneRange();
}

RedColor ReadColor(const PblConfig::Data& data);

// GLOBAL: SWBF2 0x01e2fd4c
SunFlareEffect* SunFlareEffect::s_pInstance;


// FUNCTION: SWBF2 0x0042d140
void SunFlareEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new SunFlareEffect();
    }
}

// SYNTHETIC: SWBF2 0x00402120
// `dynamic initializer for 's_SunSkyRenderable''
// SYNTHETIC: SWBF2 0x00766d00
// `dynamic atexit destructor for 's_SunSkyRenderable''

// GLOBAL: SWBF2 0x01eaffc8
// s_SunSkyRenderable
SunSkyRenderable s_SunSkyRenderable;

// GLOBAL: SWBF2 0x01e2fd54
static RedTexture* s_sampleTex;
// GLOBAL: SWBF2 0x01e2fd50
static RedTexture* s_intensityTex;
// GLOBAL: SWBF2 0x01f9af38
static pcClearShader* s_pClearShader;
// GLOBAL: SWBF2 0x01f9af34
static pcSampleShader* s_pSampleShader;
// GLOBAL: SWBF2 0x01f9af74
static pcRedPrimitive* s_samplePoints;

// GLOBAL: SWBF2 0x01f9aeac
static pcFlareShader* s_pFlareOutShader;
// GLOBAL: SWBF2 0x01f9af78
static pcFlareShader* s_pRingShader;
// GLOBAL: SWBF2 0x01f9af3c
static pcFlareShader* s_pSunAndHaloShader;


// GLOBAL: SWBF2 0x007eb748
// s_boundingBox
static PblBox s_boundingBox(-3000.0f, -3000.0f, -3000.0f, 3000.0f, 3000.0f, 3000.0f);





struct FlareVertex
{
    PosXYZ pos;
    RedColor color;
    TexCoord uv;
};
 


struct FlareCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};

struct SampleVertex
{
    PblVector4 pos;
    TexCoord uv;
};
 

template <class VERTEX, int RVF>
bool CreateTextures(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                    SunFlare::Rect& rect, bool bSunAndHalo);
template <class VERTEX, int RVF>
void RenderFlareOuts(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                     const SunFlare::Rect& rect, float fFlareOutSize, int iNumFlareOuts,
                     unsigned char initialAlpha);
template <class VERTEX>
void EmitRing(VERTEX*& pVertices, const PblVector3& vPosition, float, float, float, float,
              const RedColor&, const RedColor&, const RedColor&, int, float, float);




static void SetSunFlareVertex(FlareVertex& v, float x, float y, float z,
                              const RedColor& color, float u, float texV)
{
    v.pos.SetPos(x, y, z);
    v.color = color;
    v.uv.SetUV(u, texV);
}

static void SetSunFlareVertex(FlareCompressedVertex& v, float x, float y, float z,
                              const RedColor& color, float u, float texV)
{
    v.pos.SetPos(x, y, z);
    v.color = color;
    v.uv.SetUV(u, texV);
}

static void SetSunFlareDiscVertex(FlareVertex& v, float x, float y, float z)
{
    v.pos.SetPos(x, y, z);
    v.color = RedColor::WHITE;
}

static void SetSunFlareDiscVertex(FlareCompressedVertex& v, float x, float y, float z)
{
    v.pos.SetPos(x, y, z);
    v.color = RedColor::WHITE;
}

// FUNCTION: SWBF2 0x0042d8b0
void SunFlareEffect::Init()
{
    s_sampleTex = new RedTexture(16, 16, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                 D3DPOOL_DEFAULT, 0);
    s_intensityTex = new RedTexture(2, 2, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                    D3DPOOL_DEFAULT, 0);
    s_pClearShader = (pcClearShader*)pcRedShader::Create(
        PblHash("Clear"), 0x203, 0, 0, 0x80000, RedColor::WHITE, RedColor::BLACK, 32.0f);

    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        s_samplePoints = pcRedPrimitive::Create(D3DPT_POINTLIST, 2, 12, 256, 0, true);
        float* v = (float*)s_samplePoints->BeginVertexData();
        if (!v)
        {
            return;
        }
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                v[0] = (float)i;
                v[1] = (float)j;
                v[2] = 0.0f;
                v += 3;
            }
        }
        s_samplePoints->EndVertexData();

        s_pFlareOutShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x282, 0, 0, 0x30000, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            kuiSampleShader, 2, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader->SetInputTexture(s_sampleTex);
        s_pSampleShader->SetScale(1.0f / 120.0f);
        s_pSampleShader->SetOutputPosition(0.0f, 0.0f);
        s_pRingShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pRingShader->SetTexture(s_intensityTex);
        s_pSunAndHaloShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x82, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        s_samplePoints = pcRedPrimitive::Create(D3DPT_POINTLIST, 0x9282, 8, 256, 0, true);
        const PblBox box(0.0f, 0.0f, 0.0f, 15.0f, 15.0f, 0.0f);
        s_samplePoints->m_bbox = box;
        CompressedPos::SetBoundingBox(box);
        CompressedPos* v = (CompressedPos*)s_samplePoints->BeginVertexData();
        if (!v)
        {
            return;
        }
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                v->SetPos((float)i, (float)j, 0.0f);
                ++v;
            }
        }
        s_samplePoints->EndVertexData();

        s_pFlareOutShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x9282, 0, 0, 0x30000, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            kuiSampleShader, 0x1002, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader->SetInputTexture(s_sampleTex);
        s_pSampleShader->SetScale(1.0f / 120.0f);
        s_pSampleShader->SetOutputPosition(0.0f, 0.0f);
        s_pRingShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x9282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pRingShader->SetTexture(s_intensityTex);
        s_pSunAndHaloShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x1082, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }

    s_SunSkyRenderable.Activate(0);
}

// FUNCTION: SWBF2 0x0042d1e0
void SunFlareEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x0042d200
void SunSkyRenderable::Render(int value)
{
    if (SunFlareEffect::s_pInstance)
    {
        SunFlareEffect::s_pInstance->RenderSky(value);
    }
}

// FUNCTION: SWBF2 0x0042d2f0
FLEffectClass* SunFlareEffect::BuildClass(PblConfig& config) const
{
    SunFlare* pFlare = new SunFlare();
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3D7E6258u:
            pFlare->SetColor(ReadColor(data));
            break;
        case 0x23A0D95Cu:
            pFlare->SetSize(data.GetFloatArg(0));
            break;
        case 0x4D609778u:
            pFlare->SetFlareOutSize(data.GetFloatArg(0));
            break;
        case 0x9EBE759Fu:
        {
            RedColor color;
            color.r = data.GetUint8Arg(1);
            color.g = data.GetUint8Arg(2);
            color.b = data.GetUint8Arg(3);
            color.a = data.GetNumArgs() > 4 ? data.GetUint8Arg(4) : 0xff;
            pFlare->SetHaloInnerRing(data.GetFloatArg(0), color);
            break;
        }
        case 0x5671AC28u:
        {
            RedColor color;
            color.r = data.GetUint8Arg(1);
            color.g = data.GetUint8Arg(2);
            color.b = data.GetUint8Arg(3);
            color.a = data.GetNumArgs() > 4 ? data.GetUint8Arg(4) : 0xff;
            pFlare->SetHaloMiddleRing(data.GetFloatArg(0), color);
            break;
        }
        case 0x59B407CEu:
        {
            RedColor color;
            color.r = data.GetUint8Arg(1);
            color.g = data.GetUint8Arg(2);
            color.b = data.GetUint8Arg(3);
            color.a = data.GetNumArgs() > 4 ? data.GetUint8Arg(4) : 0xff;
            pFlare->SetHaloOutterRing(data.GetFloatArg(0), color);
            break;
        }
        case 0xB9BAA468u:
        {
            RedColor color;
            color.r = data.GetUint8Arg(0);
            color.g = data.GetUint8Arg(1);
            color.b = data.GetUint8Arg(2);
            color.a = data.GetNumArgs() > 3 ? data.GetUint8Arg(3) : 0xff;
            pFlare->SetSpikeColor(color);
            break;
        }
        case 0xACAAECACu:
            pFlare->SetSpikeSize(data.GetFloatArg(0));
            break;
        case 0xAD544418u:
        {
            float fPitch = data.GetFloatArg(0) * 0.017453292f;
            float fYaw = data.GetNumArgs() > 1 ? data.GetFloatArg(1) * 0.017453292f : 0.0f;
            float fSinPitch, fCosPitch, fSinYaw, fCosYaw;
            PblMath::SinCos(&fSinPitch, &fCosPitch, fPitch);
            PblVector3 v(fSinPitch, -fCosPitch, 0.0f);
            PblMath::SinCos(&fSinYaw, &fCosYaw, fYaw);
            PblVector3 vDirection(fCosYaw * v.x - fSinYaw * v.z, v.y,
                                  fSinYaw * v.x + fCosYaw * v.z);
            pFlare->SetDirection(vDirection);
            RedWater::SetSpecularDirection(
                PblVector3(-vDirection.x, -vDirection.y, -vDirection.z));
            break;
        }
        case 0xE3C8D91Eu:
            pFlare->SetFlareEnable(data.GetInt32Arg(0) != 0);
            break;
        case 0xE453BE48u:
            pFlare->SetNumFlareOuts(data.GetInt32Arg(0));
            break;
        }
    }
    pFlare->ActivateEffect();
    return 0;
}

// FUNCTION: SWBF2 0x0042dd30
void SunFlareEffect::Cleanup()
{
    if (s_samplePoints)
    {
        s_samplePoints->Release();
        s_samplePoints = 0;
    }
    delete s_sampleTex;
    delete s_intensityTex;
    if (s_pClearShader)
    {
        s_pClearShader->Release();
        s_pClearShader = 0;
    }
    if (s_pFlareOutShader)
    {
        s_pFlareOutShader->Release();
        s_pFlareOutShader = 0;
    }
    if (s_pSampleShader)
    {
        s_pSampleShader->Release();
        s_pSampleShader = 0;
    }
    if (s_pRingShader)
    {
        s_pRingShader->Release();
        s_pRingShader = 0;
    }
    if (s_pSunAndHaloShader)
    {
        s_pSunAndHaloShader->Release();
        s_pSunAndHaloShader = 0;
    }
    _List.Cleanup();
    s_SunSkyRenderable.Deactivate();
}

// FUNCTION: SWBF2 0x0042def0
void SunFlareEffect::Render(int value)
{
    if (value & 0x200000)
    {
        return;
    }
    for (PblList<SunFlare>::Itor it = _List.Begin(); it != _List.End(); ++it)
    {
        it->_Render();
    }
}

// FUNCTION: SWBF2 0x0042df30
void SunFlareEffect::RenderSky(unsigned int uiPass) const
{
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        for (PblList<SunFlare>::Itor it = _List.Begin(); it != _List.End(); ++it)
        {
            it->RenderSunAndHalo<FlareVertex, 642>(uiPass);
        }
    }
    else
    {
        for (PblList<SunFlare>::Itor it = _List.Begin(); it != _List.End(); ++it)
        {
            it->RenderSunAndHalo<FlareCompressedVertex, 37506>(uiPass);
        }
    }
}

// FUNCTION: SWBF2 0x0042dfa0
void SunFlareEffect::SunFlareList::Cleanup()
{
    for (Itor it = Begin(); it != End(); ++it)
    {
        delete (SunFlare*)it;
    }
    RemoveAll();
}

// FUNCTION: SWBF2 0x0042e000
void SunFlareEffect::SunFlareList::Add(SunFlare* pFlare)
{
    PblList<SunFlare>::Add(&pFlare->_Node, pFlare);
}

// FUNCTION: SWBF2 0x0042e030
void SunFlareEffect::SunFlareList::Remove(SunFlare* pFlare)
{
    PblList<SunFlare>::Remove(&pFlare->_Node);
}

// FUNCTION: SWBF2 0x0042e070
FLEffectObject* SunFlare::BuildEffect()
{
    SunFlare* pFlare = new SunFlare(*this);
    pFlare->ActivateEffect();
    return pFlare;
}

// FUNCTION: SWBF2 0x0042e0e0
bool SunFlare::IsEffectActive() const
{
    return _Node.IsInList();
}

// FUNCTION: SWBF2 0x0042e0f0
void SunFlare::ActivateEffect()
{
    if (!IsEffectActive())
    {
        SunFlareEffect::s_pInstance->_List.Add(this);
        FLEffectObject::ActivateEffect();
    }
}

// FUNCTION: SWBF2 0x0042e120
void SunFlare::DeactivateEffect()
{
    if (IsEffectActive())
    {
        SunFlareEffect::s_pInstance->_List.Remove(this);
        FLEffectObject::DeactivateEffect();
    }
}

// FUNCTION: SWBF2 0x0042e150
SunFlare::SunFlare()
    : _fSize(1.0f)
    , _color(RedColor::WHITE)
    , _initialFlareOutAlpha(70)
    , _spikeColor(RedColor(255, 255, 0, 128))
    , _flareEnable(true)
{
    _sunHaloColor[0] = RedColor(255, 255, 0, 255);
    _sunHaloColor[1] = RedColor(255, 255, 0, 128);
    _sunHaloColor[2] = RedColor(255, 255, 0, 0);
    _sunHaloSize[0] = 0.0f;
    _sunHaloSize[1] = 5.0f;
    _sunHaloSize[2] = 40.0f;
    m_sunTex = new RedTexture(256, 256, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                              D3DPOOL_DEFAULT, 0);
}

// SYNTHETIC: SWBF2 0x0042e320
// SunFlare::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0042e350
SunFlare::SunFlare(const SunFlare& that)
    : FLEffectClass()
    , FLEffectObject()
    , _fSize(that._fSize)
    , _color(that._color)
    , _fFlareOutSize(that._fFlareOutSize)
    , _iNumFlareOuts(that._iNumFlareOuts)
    , _flareEnable(that._flareEnable)
{
    m_sunTex = new RedTexture(256, 256, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                              D3DPOOL_DEFAULT, 0);
}

// SYNTHETIC: SWBF2 0x00433dd8
// ??_ESunFlare@@WCI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00433de0
// ??_ESunFlare@@WBA@AEPAXI@Z
// FUNCTION: SWBF2 0x0042e4d0
SunFlare::~SunFlare()
{
    delete m_sunTex;
    DeactivateEffect();
}

// FUNCTION: SWBF2 0x0042e560
const PblMatrix& SunFlare::GetMatrix() const
{
    float fDistance = GetDistance();
    PblVector3 vOffset(_vDirection.x * fDistance, _vDirection.y * fDistance,
                       _vDirection.z * fDistance);
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    const_cast<SunFlare*>(this)->SetPosition(
        PblVector3(vCamera.x + vOffset.x, vCamera.y + vOffset.y, vCamera.z + vOffset.z));
    return m_mWorldMatrix;
}

// FUNCTION: SWBF2 0x0042e5f0
const PblVector3& SunFlare::GetPosition() const
{
    float fDistance = GetDistance();
    PblVector3 vOffset(_vDirection.x * fDistance, _vDirection.y * fDistance,
                       _vDirection.z * fDistance);
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    const_cast<SunFlare*>(this)->SetPosition(
        PblVector3(vCamera.x + vOffset.x, vCamera.y + vOffset.y, vCamera.z + vOffset.z));
    return m_mWorldMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x0042e680
float SunFlare::GetSize() const
{
    float fZoom = RedRenderer::GetCamera()->GetZoom();
    return GetDistance() * _fSize * 0.00833f / fZoom;
}

// FUNCTION: SWBF2 0x0042e6c0
float SunFlare::GetDistance()
{
    float fDistance;
    if (FLRenderer::GetFarSceneRange() > FLRenderer::GetNearSceneFadeStart())
    {
        fDistance = FLRenderer::GetFarSceneRange() * 0.9f;
    }
    else
    {
        float fNear, fFar, fWidth, fHeight;
        RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
        fDistance = fFar * 0.9f;
    }
    if (fDistance > 2000.0f)
    {
        fDistance = 2000.0f;
    }
    return fDistance;
}

// FUNCTION: SWBF2 0x0042e960
void sub_42E960()
{
}

// FUNCTION: SWBF2 0x0042e740
void SunFlare::_Render() const
{
    if (!_flareEnable)
    {
        return;
    }
    const PblVector3& vAxis = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    if (_vDirection.x * vAxis.x + _vDirection.y * vAxis.y + _vDirection.z * vAxis.z > 0.0f)
    {
        return;
    }

    float fDistance = GetDistance();
    const PblVector3 vOffset = _vDirection * fDistance;
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    PblVector3 vPosition = vCamera + vOffset;
    {
        PblVector3 vView;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vView), reinterpret_cast<const D3DXVECTOR3*>(&vPosition), reinterpret_cast<const D3DXMATRIX*>(&RedRenderer::GetCamera()->GetMatrixInverse()));
        vPosition = vView;
    }

    float fSize = _fSize * fDistance * 0.00833f / RedRenderer::GetCamera()->GetZoom();
    float fFlareOutSize = _fFlareOutSize * fDistance * 0.00833f;
    Rect rect;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        if (!CreateTextures<FlareVertex, 642>(m_sunTex, vPosition, fSize, rect, false))
        {
            return;
        }
        RenderFlareOuts<FlareVertex, 642>(m_sunTex, vPosition, fSize, rect, fFlareOutSize,
                                          _iNumFlareOuts, _initialFlareOutAlpha);
        RenderSunRings<FlareVertex, 642>(vPosition, fSize, 0.0f, 0.0f);
    }
    else
    {
        if (!CreateTextures<FlareCompressedVertex, 37506>(m_sunTex, vPosition, fSize, rect, false))
        {
            return;
        }
        RenderFlareOuts<FlareCompressedVertex, 37506>(m_sunTex, vPosition, fSize, rect,
                                                      fFlareOutSize, _iNumFlareOuts,
                                                      _initialFlareOutAlpha);
        RenderSunRings<FlareCompressedVertex, 37506>(vPosition, fSize, 0.0f, 0.0f);
    }
}

// TEMPLATE: SWBF2 0x0042e970
// SunFlare::RenderSunAndHalo<FlareVertex,642>
// TEMPLATE: SWBF2 0x0042ed30
// SunFlare::RenderSunAndHalo<FlareCompressedVertex,37506>
template <class VERTEX, int RVF>
void SunFlare::RenderSunAndHalo(unsigned int uiPass) const
{
    const PblVector3& vAxis = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    if (_vDirection.x * vAxis.x + _vDirection.y * vAxis.y + _vDirection.z * vAxis.z > 0.0f)
    {
        return;
    }

    float fDistance = GetDistance();
    PblVector3 vOffset(_vDirection.x * fDistance, _vDirection.y * fDistance,
                       _vDirection.z * fDistance);
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    PblVector3 vPosition(vCamera.x + vOffset.x, vCamera.y + vOffset.y, vCamera.z + vOffset.z);
    PblVector3 vView;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vView), reinterpret_cast<const D3DXVECTOR3*>(&vPosition), reinterpret_cast<const D3DXMATRIX*>(&RedRenderer::GetCamera()->GetMatrixInverse()));
    vPosition = vView;

    const float fScale = fDistance * 0.00833f;
    const float fSize = _fSize * fScale / RedRenderer::GetCamera()->GetZoom();
    if (!(uiPass & 0x200000))
    {
        Rect rect;
        CreateTextures<VERTEX, RVF>(m_sunTex, vPosition, fSize, rect, true);
    }

    pcRedVertexBuffer* pBuffer = pcRedVertexBuffer::Create(300, sizeof(VERTEX), false, true);
    CompressedPos::SetBoundingBox(s_boundingBox);
    unsigned int uiStart;
    VERTEX* pBase = (VERTEX*)pBuffer->Lock(300, uiStart);
    VERTEX* pVertices = pBase;
    EmitRing(pVertices, vPosition, 0.0f, _sunHaloSize[0] * fScale, _sunHaloSize[1] * fScale,
             _sunHaloSize[2] * fScale, _sunHaloColor[0], _sunHaloColor[1], _sunHaloColor[2], 30,
             0.0f, 0.0f);
    if (_flareEnable)
    {
        EmitRing(pVertices, vPosition, 0.0f, 0.0f, 0.0f, fSize, _color, _color, _color, 30, 0.0f,
                 0.0f);
    }
    const int iNumVertices = pVertices - pBase;
    pBuffer->Unlock();

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, iNumVertices - 2, RVF,
                                                        iNumVertices, uiStart, pBuffer, 0, 0);
    pPrimitive->m_bbox = s_boundingBox;
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pSunAndHaloShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);
    pBuffer->Release();
    pPrimitive->Release();
}

// TEMPLATE: SWBF2 0x0042f0e0
// CreateTextures<FlareVertex,642>
template <class VERTEX, int RVF>
bool CreateTextures(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                    SunFlare::Rect& rect, bool bSunAndHalo)
{
    if (bSunAndHalo && FLRenderer::GetNearSceneFadeStart() >= FLRenderer::GetFarSceneRange())
    {
        return false;
    }

    const PblVector3 vMin = RedRenderer::TransformProjectionPointToScreenSpace(
        RedRenderer::TransformCameraPointToProjectionSpace(
            PblVector3(vPosition.x - fSize, vPosition.y - fSize, vPosition.z)));
    const PblVector3 vMax = RedRenderer::TransformProjectionPointToScreenSpace(
        RedRenderer::TransformCameraPointToProjectionSpace(
            PblVector3(vPosition.x + fSize, vPosition.y + fSize, vPosition.z)));
    float viewMinX, viewMinY, viewMaxX, viewMaxY;
    RedRenderer::pcGetViewportExtents(viewMinX, viewMinY, viewMaxX, viewMaxY);
    if (viewMinX >= vMax.x || vMin.x >= viewMaxX || viewMinY >= vMin.y ||
        vMax.y >= viewMaxY)
    {
        return false;
    }

    const float sourceWidth = vMax.x - vMin.x;
    const float sourceHeight = vMin.y - vMax.y;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    if (bSunAndHalo)
    {
        IDirect3DSurface9* pBack;
        IDirect3DSurface9* pSky;
        RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBack);
        ((IDirect3DTexture9*)RedRenderer::pcGetBuffer(PC_BUFFER_SKY)->m_pTexture)->GetSurfaceLevel(0, &pSky);
        D3DSURFACE_DESC backDesc, skyDesc;
        pBack->GetDesc(&backDesc);
        pSky->GetDesc(&skyDesc);
        pBack->Release();
        pSky->Release();
        if (backDesc.Width != skyDesc.Width)
        {
            scaleX = (float)backDesc.Width / (float)skyDesc.Width;
        }
        if (backDesc.Height != skyDesc.Height)
        {
            scaleY = (float)backDesc.Height / (float)skyDesc.Height;
        }
    }
    if (sourceWidth > (float)pTexture->GetWidth() || sourceHeight > (float)pTexture->GetHeight())
    {
        if (sourceWidth > sourceHeight)
        {
            scaleX *= (float)pTexture->GetWidth() / sourceWidth;
        }
        else
        {
            scaleY *= (float)pTexture->GetHeight() / sourceHeight;
        }
    }
    float destWidth = sourceWidth * scaleX;
    float destHeight = sourceHeight * scaleY;
    float destMinX = 0.0f, destMinY = 0.0f;
    float minX = vMin.x, maxX = vMax.x, minY = vMax.y, maxY = vMin.y;
    if (viewMinX > minX) { destMinX = (viewMinX - minX) * scaleX; minX = viewMinX; }
    if (maxX > viewMaxX) { destWidth -= (maxX - viewMaxX) * scaleX; maxX = viewMaxX; }
    if (viewMinY > minY) { destMinY = (viewMinY - minY) * scaleY; minY = viewMinY; }
    if (maxY > viewMaxY) { destHeight -= (maxY - viewMaxY) * scaleY; maxY = viewMaxY; }
    RECT source = { (int)minX, (int)minY, (int)maxX, (int)maxY };
    RECT dest = { (int)destMinX, (int)destMinY, (int)destWidth, (int)destHeight };
    if (source.left >= source.right || source.top >= source.bottom ||
        dest.left >= dest.right || dest.top >= dest.bottom)
    {
        return false;
    }

    rect.minX = 0.0f; rect.minY = 0.0f;
    rect.maxX = sourceWidth * scaleX / (float)pTexture->GetWidth();
    rect.maxY = sourceHeight * scaleY / (float)pTexture->GetHeight();

    pcRedPrimitive* pClear = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 515, sizeof(SampleVertex), 4, 0, false);
    SampleVertex* pClearV = (SampleVertex*)pClear->BeginVertexData();
    if (!pClearV)
    {
        return false;
    }
    const float x0 = minX - 2.0f, x1 = maxX + 2.0f;
    const float y0 = minY - 2.0f, y1 = maxY + 2.0f;
    pClearV[0].pos.Set(x0, y0, 0.0f, 1.0f);
    pClearV[1].pos.Set(x0, y1, 0.0f, 1.0f);
    pClearV[2].pos.Set(x1, y0, 0.0f, 1.0f);
    pClearV[3].pos.Set(x1, y1, 0.0f, 1.0f);
    pClear->EndVertexData();
    RedColor transparent = { 0, 0, 0, 0 };
    RedRenderer::pcRenderPrimitive(pClear, s_pClearShader,
                                   gMatrixIdentity, transparent, 0x200, 0, 0, 0);
    pClear->Release();

    pcRedPrimitive* pDisc = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, RVF,
                                                   sizeof(VERTEX), 42, 0, false);
    pDisc->m_bbox = s_boundingBox;
    CompressedPos::SetBoundingBox(s_boundingBox);
    VERTEX* pDiscV = (VERTEX*)pDisc->BeginVertexData();
    if (!pDiscV)
    {
        return false;
    }
    for (int i = 0; i <= 20; ++i)
    {
        float s, c;
        PblMath::SinCos(&s, &c, (float)i * 6.2831855f / 20.0f);
        SetSunFlareDiscVertex(pDiscV[2 * i], vPosition.x, vPosition.y, vPosition.z);
        SetSunFlareDiscVertex(pDiscV[2 * i + 1], vPosition.x + c * fSize,
                             vPosition.y + s * fSize, vPosition.z);
    }
    pDisc->EndVertexData();
    RedRenderer::pcRenderPrimitive(pDisc, s_pSunAndHaloShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::BLACK, 0x200, 0, 0, 0);
    pDisc->Release();

    if (!bSunAndHalo && FLRenderer::GetFarSceneRange() > FLRenderer::GetNearSceneFadeStart())
    {
        pcRedPrimitive* pFade = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 515, sizeof(SampleVertex), 4, 0, false);
        SampleVertex* pFadeV = (SampleVertex*)pFade->BeginVertexData();
        if (!pFadeV)
        {
            return false;
        }
        pFadeV[0].pos.Set(vMin.x, vMax.y, 0.0f, 1.0f);
        pFadeV[0].uv.SetUV(rect.minX, rect.minY);
        pFadeV[1].pos.Set(vMin.x, vMin.y, 0.0f, 1.0f);
        pFadeV[1].uv.SetUV(rect.minX, rect.maxY);
        pFadeV[2].pos.Set(vMax.x, vMax.y, 0.0f, 1.0f);
        pFadeV[2].uv.SetUV(rect.maxX, rect.minY);
        pFadeV[3].pos.Set(vMax.x, vMin.y, 0.0f, 1.0f);
        pFadeV[3].uv.SetUV(rect.maxX, rect.maxY);
        pFade->EndVertexData();
        s_pClearShader->SetBlendMode(pcClearShader::BLENDMODE_MODULATE);
        s_pClearShader->m_pTex = pTexture;
        RedRenderer::pcRenderPrimitive(pFade, s_pClearShader,
                                       gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
        pFade->Release();
        s_pClearShader->SetBlendMode(pcClearShader::BLENDMODE_NONE);
        s_pClearShader->m_pTex = 0;
    }

    if (bSunAndHalo || FLRenderer::GetNearSceneFadeStart() >= FLRenderer::GetFarSceneRange())
    {
        RedTexture* pOld = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepth = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(pTexture, pDepth);
        RedColor clearColor(RedColor::BLACK, 0);
        RedRenderer::pcClear(1, clearColor, 1.0f, 0);
        RedRenderer::pcSetRenderTarget(pOld, pDepth);
        RedRenderer::pcSetViewport(viewport);
    }

    IDirect3DSurface9* pSource;
    IDirect3DSurface9* pSun;
    IDirect3DSurface9* pSample;
    RedRenderer::pcGetDevice()->GetRenderTarget(0, &pSource);
    ((IDirect3DTexture9*)pTexture->m_pTexture)->GetSurfaceLevel(0, &pSun);
    ((IDirect3DTexture9*)s_sampleTex->m_pTexture)->GetSurfaceLevel(0, &pSample);
    RedRenderer::pcGetDevice()->StretchRect(pSource, &source, pSun, &dest, D3DTEXF_NONE);
    if (!bSunAndHalo)
    {
        RECT sampleDest = { 0, 0, 16, 16 };
        RedRenderer::pcGetDevice()->StretchRect(pSource, &source, pSample, &sampleDest,
                                                D3DTEXF_NONE);
    }
    pSource->Release(); pSun->Release(); pSample->Release();
    if (!bSunAndHalo)
    {
        RedTexture* pOld = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepth = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(s_intensityTex, 0);
        RedColor clearColor(RedColor::BLACK, 0);
        RedRenderer::pcClear(1, clearColor, 1.0f, 0);
        RedRenderer::pcRenderPrimitive(s_samplePoints, s_pSampleShader,
                                       gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
        RedRenderer::pcSetRenderTarget(pOld, pDepth);
        RedRenderer::pcSetViewport(viewport);
    }
    return true;
}

// TEMPLATE: SWBF2 0x00430100
// CreateTextures<FlareCompressedVertex,37506>
template bool CreateTextures<FlareVertex, 642>(RedTexture*, const PblVector3&, float,
                                                SunFlare::Rect&, bool);
template bool CreateTextures<FlareCompressedVertex, 37506>(RedTexture*, const PblVector3&,
                                                             float, SunFlare::Rect&, bool);

// TEMPLATE: SWBF2 0x0042fb20
// RenderFlareOuts<FlareVertex,642>
template <class VERTEX, int RVF>
void RenderFlareOuts(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                     const SunFlare::Rect& rect, float fFlareOutSize, int iNumFlareOuts,
                     unsigned char initialAlpha)
{
    pcRedVertexBuffer* pBuffer = pcRedVertexBuffer::Create(6 * iNumFlareOuts,
                                                            sizeof(VERTEX), false, true);
    CompressedPos::SetBoundingBox(s_boundingBox);
    unsigned int startVertex;
    VERTEX* pBase = (VERTEX*)pBuffer->Lock(6 * iNumFlareOuts, startVertex);
    VERTEX* pVertex = pBase;
    float size = fSize;
    float step = (fFlareOutSize - fSize) / (float)iNumFlareOuts;
    for (int i = 0; i < iNumFlareOuts; ++i)
    {
        size += step;
        float fade = 1.0f - (float)i / (float)iNumFlareOuts;
        RedColor color(RedColor::WHITE, (unsigned char)(int)((float)initialAlpha * fade * fade * fade));
        if (color.a < 3)
        {
            break;
        }
        const float x0 = vPosition.x - size, x1 = vPosition.x + size;
        const float y0 = vPosition.y - size, y1 = vPosition.y + size;
        SetSunFlareVertex(pVertex[0], x0, y0, vPosition.z, color, rect.minX, rect.maxY);
        SetSunFlareVertex(pVertex[1], x1, y0, vPosition.z, color, rect.maxX, rect.maxY);
        SetSunFlareVertex(pVertex[2], x1, y1, vPosition.z, color, rect.maxX, rect.minY);
        SetSunFlareVertex(pVertex[3], x0, y0, vPosition.z, color, rect.minX, rect.maxY);
        SetSunFlareVertex(pVertex[4], x1, y1, vPosition.z, color, rect.maxX, rect.minY);
        SetSunFlareVertex(pVertex[5], x0, y1, vPosition.z, color, rect.minX, rect.minY);
        pVertex += 6;
    }
    const int vertexCount = pVertex - pBase;
    pBuffer->Unlock();
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST,
                                                         vertexCount / 3, RVF, vertexCount,
                                                         startVertex, pBuffer, 0, 0);
    pPrimitive->m_bbox = s_boundingBox;
    s_pFlareOutShader->SetTexture(pTexture);
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pFlareOutShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);
    pBuffer->Release();
    pPrimitive->Release();
}

// TEMPLATE: SWBF2 0x00430b70
// RenderFlareOuts<FlareCompressedVertex,37506>
template void RenderFlareOuts<FlareVertex, 642>(RedTexture*, const PblVector3&, float,
                                                const SunFlare::Rect&, float, int, unsigned char);
template void RenderFlareOuts<FlareCompressedVertex, 37506>(RedTexture*, const PblVector3&,
                                                             float, const SunFlare::Rect&, float,
                                                             int, unsigned char);

// TEMPLATE: SWBF2 0x0042ff10
// SunFlare::RenderSunRings<FlareVertex,642>
template <class VERTEX, int RVF>
void SunFlare::RenderSunRings(const PblVector3& vPosition, float fSize, float fTexU,
                             float fTexV) const
{
    pcRedVertexBuffer* pBuffer = pcRedVertexBuffer::Create(700, sizeof(VERTEX), false, true);
    CompressedPos::SetBoundingBox(s_boundingBox);
    unsigned int uiStart;
    VERTEX* pBase = (VERTEX*)pBuffer->Lock(700, uiStart);
    VERTEX* pVertices = pBase;
    EmitSunRings(pVertices, vPosition, fSize, fTexU, fTexV);
    const int iNumVertices = pVertices - pBase;
    pBuffer->Unlock();
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP,
        iNumVertices - 2, RVF, iNumVertices, uiStart, pBuffer, 0, 0);
    pPrimitive->m_bbox = s_boundingBox;
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pRingShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);
    pBuffer->Release();
    pPrimitive->Release();
}

// TEMPLATE: SWBF2 0x00431140
// SunFlare::RenderSunRings<FlareCompressedVertex,37506>
template void SunFlare::RenderSunRings<FlareVertex, 642>(const PblVector3&, float, float, float) const;
template void SunFlare::RenderSunRings<FlareCompressedVertex, 37506>(const PblVector3&, float, float, float) const;

template <class VERTEX>
void EmitRing(VERTEX*& pVertices, const PblVector3& vPosition, float fAngle,
              float fInnerRadius, float fMiddleRadius, float fOuterRadius,
              const RedColor& innerColor, const RedColor& middleColor,
              const RedColor& outerColor, int iSegments, float fU, float fV)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    PblVector3 tempPos = vPosition + PblVector3(fCos * fInnerRadius, fSin * fInnerRadius, 0.0f);
    pVertices->pos.SetPos(tempPos);
    ++pVertices;
    for (int i = 0; i <= iSegments; ++i)
    {
        PblMath::SinCos(&fSin, &fCos, (float)i * 6.2831855f / (float)iSegments + fAngle);
        PblVector3 vSegPos[3] = {
            vPosition + PblVector3(fCos * fInnerRadius, fSin * fInnerRadius, 0.0f),
            vPosition + PblVector3(fCos * fMiddleRadius, fSin * fMiddleRadius, 0.0f),
            vPosition + PblVector3(fCos * fOuterRadius, fSin * fOuterRadius, 0.0f)
        };
        pVertices[0].pos.SetPos(vSegPos[0]);
        pVertices[0].color = innerColor;
        pVertices[0].uv.SetUV(fU, fV);
        pVertices[1].pos.SetPos(vSegPos[1]);
        pVertices[1].color = middleColor;
        pVertices[1].uv.SetUV(fU, fV);
        if (i == 0)
        {
            pVertices[2 * iSegments + 2].pos.SetPos(vSegPos[1]);
            pVertices[2 * iSegments + 2].color = middleColor;
            pVertices[2 * iSegments + 2].uv.SetUV(fU, fV);
        }
        pVertices[2 * iSegments + 3].pos.SetPos(vSegPos[1]);
        pVertices[2 * iSegments + 3].color = middleColor;
        pVertices[2 * iSegments + 3].uv.SetUV(fU, fV);
        pVertices[2 * iSegments + 4].pos.SetPos(vSegPos[2]);
        pVertices[2 * iSegments + 4].color = outerColor;
        pVertices[2 * iSegments + 4].uv.SetUV(fU, fV);
        pVertices += 2;
    }
    pVertices += 2 * iSegments + 3;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    tempPos = vPosition + PblVector3(fCos * fOuterRadius, fSin * fOuterRadius, 0.0f);
    pVertices->pos.SetPos(tempPos);
    ++pVertices;
}

// TEMPLATE: SWBF2 0x00431330
// EmitRing<FlareVertex>
template void EmitRing<FlareVertex>(FlareVertex*&, const PblVector3&, float,
                        float, float, float, const RedColor&, const RedColor&,
                        const RedColor&, int, float, float);

// TEMPLATE: SWBF2 0x004315f0
// EmitRing<FlareCompressedVertex>
template void EmitRing<FlareCompressedVertex>(FlareCompressedVertex*&, const PblVector3&, float,
                        float, float, float, const RedColor&, const RedColor&,
                        const RedColor&, int, float, float);

template <class VERTEX>
void SunFlare::EmitSunRings(VERTEX*& pVertices, const PblVector3& vPosition,
                             float radius, float fTexU, float fTexV) const
{
    const PblMatrix& projection = RedRenderer::GetProjectionMatrix();
    radius = GetDistance();
    const float fScale = radius * 0.0037f;
    const float fDepthInv = 1.0f / vPosition.z;
    const float fProjX = projection.m[0][0] * vPosition.x * fDepthInv;
    const float fProjY = projection.m[1][1] * vPosition.y * fDepthInv;
    float fForeshorten = 1.0f - (fProjX * fProjX + fProjY * fProjY) * 0.5f;
    if (fForeshorten < 0.0f)
    {
        fForeshorten = 0.0f;
    }

    unsigned char maxColor = _color.b;
    if (_color.g > maxColor)
    {
        maxColor = _color.g;
    }
    if (_color.r > maxColor)
    {
        maxColor = _color.r;
    }
    const unsigned char alpha = (unsigned char)PblMath::Floor((float)maxColor * fForeshorten + 0.5f);
    const float fSpikeScale = _spikeSize * fScale;
    const float fAngle = (vPosition.y * 6.2831855f + vPosition.x * 3.1415927f)
                         / vPosition.z + fSpikeScale * 3.1415927f * 0.33333001f;
    float fSin[2], fCos[2];
    PblMath::SinCos(&fSin[0], &fCos[0], fAngle);
    PblMath::SinCos(&fSin[1], &fCos[1], 1.0471871f - fAngle);
    const float r = ((fForeshorten + 1.0f) * (float)maxColor * 0.5f * 0.05f * 0.1f)
                    * fSpikeScale;
    const float x = vPosition.x, y = vPosition.y, z = vPosition.z;
    PblVector3 vVert[6];
    vVert[0] = vPosition + PblVector3(fCos[0] * r, fSin[0] * (r * 2.0f), 0.0f);
    vVert[1] = vPosition + PblVector3((fCos[0] * -0.5f - fSin[0] * 0.866f) * r,
                                     (fCos[0] * 0.866f - fSin[0] * 0.5f) * (r * 2.0f), 0.0f);
    vVert[2] = vPosition + PblVector3((fCos[0] * -0.5f - fSin[0] * -0.866f) * r,
                                     (fSin[0] * -0.5f - fCos[0] * 0.866f) * (r * 2.0f), 0.0f);
    vVert[3] = vPosition + PblVector3(fCos[1] * (r * 2.5f), fSin[1] * r, 0.0f);
    vVert[4] = vPosition + PblVector3((fCos[1] * -0.5f - fSin[1] * 0.866f) * (r * 2.5f),
                                     (fCos[1] * 0.866f - fSin[1] * 0.5f) * r, 0.0f);
    vVert[5] = vPosition + PblVector3((fCos[1] * -0.5f - fSin[1] * -0.866f) * (r * 2.5f),
                                     (fSin[1] * -0.5f - fCos[1] * 0.866f) * r, 0.0f);

    const RedColor clear(0, 0, 0, 0);
    const RedColor& spike = _spikeColor;
    pVertices[0].pos.SetPos(vVert[0].x, vVert[0].y, vVert[0].z);
    pVertices[0].uv.SetUV(fTexU, fTexV);
    pVertices[0].color = clear;
    pVertices[1].pos.SetPos(vVert[1].x, vVert[1].y, vVert[1].z);
    pVertices[1].uv.SetUV(fTexU, fTexV);
    pVertices[1].color = clear;
    pVertices[2].pos.SetPos(vPosition.x, vPosition.y, vPosition.z);
    pVertices[2].uv.SetUV(fTexU, fTexV);
    pVertices[2].color = spike;
    pVertices[3].pos.SetPos(vVert[2].x, vVert[2].y, vVert[2].z);
    pVertices[3].uv.SetUV(fTexU, fTexV);
    pVertices[3].color = clear;
    pVertices[4].pos.SetPos(vVert[0].x, vVert[0].y, vVert[0].z);
    pVertices[4].uv.SetUV(fTexU, fTexV);
    pVertices[4].color = clear;
    pVertices[5] = pVertices[4];
    pVertices[6].pos.SetPos(vVert[3].x, vVert[3].y, vVert[3].z);
    pVertices[6].uv.SetUV(fTexU, fTexV);
    pVertices[6].color = clear;
    pVertices[7] = pVertices[6];
    pVertices[8].pos.SetPos(vVert[4].x, vVert[4].y, vVert[4].z);
    pVertices[8].uv.SetUV(fTexU, fTexV);
    pVertices[8].color = clear;
    pVertices[9].pos.SetPos(vPosition.x, vPosition.y, vPosition.z);
    pVertices[9].uv.SetUV(fTexU, fTexV);
    pVertices[9].color = spike;
    pVertices[10].pos.SetPos(vVert[5].x, vVert[5].y, vVert[5].z);
    pVertices[10].uv.SetUV(fTexU, fTexV);
    pVertices[10].color = clear;
    pVertices[11].pos.SetPos(vVert[3].x, vVert[3].y, vVert[3].z);
    pVertices[11].uv.SetUV(fTexU, fTexV);
    pVertices[11].color = clear;
    pVertices[12] = pVertices[11];
    pVertices += 13;

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const float fThreshold = PblMath::Cos(pCamera->GetFOV() * 0.5f);
    const PblVector3& cameraAxis = pCamera->GetMatrix().GetZAxis();
    float facing = -(cameraAxis.x * _vDirection.x +
                     cameraAxis.y * _vDirection.y +
                     cameraAxis.z * _vDirection.z);
    if (facing < 0.0f)
    {
        facing = fThreshold;
    }
    float fade = (facing - fThreshold) / (1.0f - fThreshold);
    if (fade < 0.0f)
    {
        fade = 0.0f;
    }

    static float factor = fScale * 35.0f;
    static unsigned char a1;
    const RedColor whiteInner = RedColor(255, 255, 255,
        (unsigned char)(int)((float)a1 * fade));
    const RedColor whiteMiddle = RedColor(255, 255, 255,
        (unsigned char)(int)(fade * 72.0f));
    const RedColor whiteOuter = RedColor(255, 255, 255, 0);
    EmitRing(pVertices, vPosition, 0.0f, 0.0f, fade * factor,
             fade * 3.0f * factor, whiteInner, whiteMiddle, whiteOuter,
             30, fTexU, fTexV);

    vVert[0].x = x * -0.30000001f;
    vVert[0].y = y * -0.30000001f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.015f, z * 0.02f,
             RedColor(27, 54, 13, alpha),
             RedColor(13, 27, 13, alpha),
             RedColor(13, 27, 27, 0),
             6, fTexU, fTexV);

    vVert[0].x = x * -0.34f;
    vVert[0].y = y * -0.34f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.0049999999f, z * 0.0099999998f,
             RedColor(108, 54, 54, alpha),
             RedColor(54, 54, 54, alpha),
             RedColor(54, 54, 108, 0),
             5, fTexU, fTexV);

    vVert[0].x = x * -0.5f;
    vVert[0].y = y * -0.5f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * fForeshorten * 0.059999999f,
             z * (fForeshorten + 1.0f) * 0.082500003f, z * (fForeshorten + 1.0f) * 0.1f,
             RedColor(27, 13, 54, 0),
             RedColor(20, 27, 10, alpha),
             RedColor(54, 13, 0, 0),
             20, fTexU, fTexV);

    vVert[0].x = x * -0.77499998f;
    vVert[0].y = y * -0.77499998f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.0099999998f, z * 0.02f,
             RedColor(13, 20, 13, alpha),
             RedColor(13, 20, 13, alpha),
             RedColor(13, 20, 13, 0),
             6, fTexU, fTexV);

    vVert[0].x = x * -0.82499999f;
    vVert[0].y = y * -0.82499999f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.02f, z * 0.039999999f,
             RedColor(20, 34, 20, alpha),
             RedColor(20, 34, 20, alpha),
             RedColor(20, 34, 20, 0),
             8, fTexU, fTexV);

    vVert[0].x = x * -0.89499998f;
    vVert[0].y = y * -0.89499998f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.01825f, z * 0.037500001f,
             RedColor(20, 34, 20, alpha),
             RedColor(20, 34, 20, alpha),
             RedColor(27, 40, 27, 0),
             8, fTexU, fTexV);

    vVert[0].x = x * -0.94999999f;
    vVert[0].y = y * -0.94999999f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.015f, z * 0.029999999f,
             RedColor(13, 20, 13, alpha),
             RedColor(13, 20, 13, alpha),
             RedColor(13, 20, 13, 0),
             7, fTexU, fTexV);

    vVert[0].x = x * 0.30000001f;
    vVert[0].y = y * 0.30000001f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.0040000002f, z * 0.0080000004f,
             RedColor(136, 68, 27, alpha),
             RedColor(108, 68, 13, alpha),
             RedColor(0, 0, 0, alpha),
             5, fTexU, fTexV);

    vVert[0].x = x * 0.2f;
    vVert[0].y = y * 0.2f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.025f, z * 0.039999999f,
             RedColor(68, 0, 0, alpha),
             RedColor(54, 13, 0, alpha),
             RedColor(0, 0, 0, alpha),
             6, fTexU, fTexV);

    vVert[0].x = x * 0.125f;
    vVert[0].y = y * 0.125f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.015f, z * 0.025f,
             RedColor(61, 54, 0, alpha),
             RedColor(27, 54, 0, alpha),
             RedColor(0, 0, 0, alpha),
             6, fTexU, fTexV);

    vVert[0].x = x * 1.7f;
    vVert[0].y = y * 1.7f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.015f, z * 0.02f,
             RedColor(0, 13, 81, alpha),
             RedColor(0, 0, 68, alpha),
             RedColor(0, 0, 68, 0),
             6, fTexU, fTexV);

    vVert[0].x = x * 1.53f;
    vVert[0].y = y * 1.53f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.0145f, z * 0.0175f,
             RedColor(34, 0, 81, alpha),
             RedColor(0, 0, 68, alpha),
             RedColor(0, 0, 68, 0),
             5, fTexU, fTexV);

    vVert[0].x = x * 1.62f;
    vVert[0].y = y * 1.62f;
    vVert[0].z = z;
    EmitRing(pVertices, vVert[0], 0.0f, z * 0,
             z * 0.02f, z * 0.022500001f,
             RedColor(20, 27, 68, alpha),
             RedColor(13, 20, 54, alpha),
             RedColor(0, 0, 0, 0),
             7, fTexU, fTexV);
}

// TEMPLATE: SWBF2 0x00431ad0
// SunFlare::EmitSunRings<FlareVertex>
template void SunFlare::EmitSunRings<FlareVertex>(FlareVertex*&, const PblVector3&,
                                                     float, float, float) const;

// TEMPLATE: SWBF2 0x00432b00
// SunFlare::EmitSunRings<FlareCompressedVertex>
template void SunFlare::EmitSunRings<FlareCompressedVertex>(FlareCompressedVertex*&,
                                                               const PblVector3&, float,
                                                               float, float) const;
