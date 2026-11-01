#include <d3dx9math.h>
#include "Battlefront2/Source/LensFlare.h"
#include <math.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcFlareShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcSampleShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

// GLOBAL: SWBF2 0x007b2920
// __xmm@41880000000000000000000000000000

// GLOBAL: SWBF2 0x01f9a924
static RedTexture* s_sampleTex;
// GLOBAL: SWBF2 0x01f9ae40
static RedTexture* s_intensityTex;
// GLOBAL: SWBF2 0x01f9ae48
static pcRedShader* s_pClearShader;
// GLOBAL: SWBF2 0x01f9ae3c
static pcSampleShader* s_pSampleShader;
// GLOBAL: SWBF2 0x01f9ae50
static pcRedPrimitive* s_samplePoints;

// GLOBAL: SWBF2 0x01f9ae44
static pcFlareShader* s_pFlareShader;
// GLOBAL: SWBF2 0x01f9ae4c
static pcFlareShader* s_pFillShader;

struct CompressedPosXY
{
    short x;
    short y;
    static void SetBoundingBox(const PblBox& box)
    {
        s_vMin = box.GetMin();
        s_vScale = (box.GetMax() - box.GetMin()) / 65535.0f;
        s_vScaleInv = PblVector3(1.0f / s_vScale.x, 1.0f / s_vScale.y, 1.0f / s_vScale.z);
    }

    static PblVector3 s_vMin;
    static PblVector3 s_vScale;
    static PblVector3 s_vScaleInv;
};
// GLOBAL: SWBF2 0x007f587c
PblVector3 CompressedPosXY::s_vMin;
// GLOBAL: SWBF2 0x007de288
PblVector3 CompressedPosXY::s_vScale = { 1.0f / 65535.0f, 1.0f / 65535.0f, 1.0f / 65535.0f };
// GLOBAL: SWBF2 0x007de294
PblVector3 CompressedPosXY::s_vScaleInv = { 65535.0f, 65535.0f, 65535.0f };

// GLOBAL: SWBF2 0x007eb728
// s_boundingBox
static PblBox s_boundingBox(-2200.0f, -2200.0f, -2200.0f, 2200.0f, 2200.0f, 2200.0f);

static const float kfFadeDrawDistance = 0.9f;
// GLOBAL: SWBF2 0x0078c920
static const float kfFadeDrawDistanceSquared = kfFadeDrawDistance * kfFadeDrawDistance;
// GLOBAL: SWBF2 0x0078c91c
static const float kfFadeDrawDistanceFactor = 1.0f / (1.0f - kfFadeDrawDistanceSquared);

struct FlareCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
 





struct FlareVertex
{
    PosXYZ pos;
    RedColor color;
    TexCoord uv;
};
 

template <class VERTEX>
void LensFlare::EmitRing(VERTEX*& pVertices, const PblVector3& vPosition, float fAngle,
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

// TEMPLATE: SWBF2 0x0041f1d0
// LensFlare::EmitRing<FlareVertex>
template void LensFlare::EmitRing<FlareVertex>(FlareVertex*&, const PblVector3&, float,
                        float, float, float, const RedColor&, const RedColor&,
                        const RedColor&, int, float, float);

// TEMPLATE: SWBF2 0x0041f480
// LensFlare::EmitRing<FlareCompressedVertex>
template void LensFlare::EmitRing<FlareCompressedVertex>(FlareCompressedVertex*&, const PblVector3&, float,
                        float, float, float, const RedColor&, const RedColor&,
                        const RedColor&, int, float, float);





static short CompressedPos_Encode(float f, float fMin, float fScaleInv)
{
    return (short)((f - fMin) * fScaleInv - 32767.5f);
}

static void CompressedPos_SetPos(CompressedPos& pos, float fX, float fY, float fZ)
{
    pos.x = CompressedPos_Encode(fX, CompressedPos::s_vMin.x, CompressedPos::s_vScaleInv.x);
    pos.y = CompressedPos_Encode(fY, CompressedPos::s_vMin.y, CompressedPos::s_vScaleInv.y);
    pos.z = CompressedPos_Encode(fZ, CompressedPos::s_vMin.z, CompressedPos::s_vScaleInv.z);
}

template <>
void LensFlareEffect::LensFlareList::Render<FlareCompressedVertex, 37506>() const;
template <>
void LensFlareEffect::LensFlareList::Render<FlareVertex, 642>() const;

// FUNCTION: SWBF2 0x0041bc00
void LensFlareEffect::PlatformInit()
{
    s_sampleTex = new RedTexture(64, 64, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                 D3DPOOL_DEFAULT, 0);
    s_intensityTex = new RedTexture(16, 16, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                    D3DPOOL_DEFAULT, 0);
    s_pClearShader = pcRedShader::Create(0x5c6e1222 /* "Clear" */, 3, 0, 0, 0x80000,
                                         RedColor::WHITE, RedColor::BLACK, 32.0f);

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

        s_pFlareShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pFlareShader->SetTexture(s_intensityTex);
        s_pFillShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x82, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            kuiSampleShader, 2, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        s_samplePoints = pcRedPrimitive::Create(D3DPT_POINTLIST, 0x1002, 8, 256, 0, true);
        s_samplePoints->m_bbox = PblBox(0.0f, 0.0f, 0.0f, 17.0f, 17.0f, 0.0f);
        CompressedPos::SetBoundingBox(PblBox(0.0f, 0.0f, 0.0f, 17.0f, 17.0f, 0.0f));
        short* v = (short*)s_samplePoints->BeginVertexData();
        if (!v)
        {
            return;
        }
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                v[0] = (short)(((float)i - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
                v[1] = (short)(((float)j - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
                v[2] = (short)((0.0f - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
                v += 4;
            }
        }
        s_samplePoints->EndVertexData();

        s_pFlareShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x9282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pFlareShader->SetTexture(s_intensityTex);
        s_pFillShader = (pcFlareShader*)pcRedShader::Create(
            kuiFlareShader, 0x1082, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            kuiSampleShader, 0x1002, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }

    s_pSampleShader->SetOutputPosition(0.0f, 0.0f);
    s_pSampleShader->SetScale(1.0f / 255.0f);
    s_pSampleShader->SetInputTexture(s_sampleTex);
}

// FUNCTION: SWBF2 0x0041c000
void LensFlareEffect::PlatformCleanup()
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
    if (s_pFlareShader)
    {
        s_pFlareShader->Release();
        s_pFlareShader = 0;
    }
    if (s_pSampleShader)
    {
        s_pSampleShader->Release();
        s_pSampleShader = 0;
    }
    if (s_pFillShader)
    {
        s_pFillShader->Release();
        s_pFillShader = 0;
    }
    _List.Cleanup();
}

// FUNCTION: SWBF2 0x0041c170
void LensFlareEffect::Render(int value)
{
    if (value & 0x200000)
    {
        return;
    }
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        _List.Render<FlareVertex, 642>();
    }
    else
    {
        _List.Render<FlareCompressedVertex, 37506>();
    }
}


inline const PblVector3 RedRenderer::TransformCameraPointToScreenSpace(const PblVector3& vPoint)
{
    return TransformProjectionPointToScreenSpace(TransformCameraPointToProjectionSpace(vPoint));
}

static bool IsFlareVisible(const LensFlare* pFlare, unsigned int uiView, const PblMatrix& mView,
                           float fNear, float fFar, float fWidth, float fHeight,
                           PblVector3& vPosition, float& fSize, float& fSquaredDistance)
{
    if (pFlare->GetColor().a == 0)
    {
        return false;
    }
    if (pFlare->IsHiddenInView(uiView))
    {
        return false;
    }

    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPosition), reinterpret_cast<const D3DXVECTOR3*>(&pFlare->m_mWorldMatrix.GetTranslation()), reinterpret_cast<const D3DXMATRIX*>(&mView));
    fSize = pFlare->GetSize();

    if (fNear > -vPosition.z || -vPosition.z > fFar)
    {
        return false;
    }

    if (pFlare->GetSquaredDrawDistance() > 0.0f)
    {
        fSquaredDistance = vPosition.x * vPosition.x + vPosition.y * vPosition.y +
                           vPosition.z * vPosition.z;
        if (!(fSquaredDistance < pFlare->GetSquaredDrawDistance()))
        {
            return false;
        }
    }
    else
    {
        fSquaredDistance = 0.0f;
    }

    if (((float)fabs(vPosition.x) - fSize) * fNear * 2.0f > -(vPosition.z * fWidth))
    {
        return false;
    }
    if (((float)fabs(vPosition.y) - fSize) * fNear * 2.0f > -(vPosition.z * fHeight))
    {
        return false;
    }
    return true;
}

static void SetFillVertex(FlareCompressedVertex& vertex, float fX, float fY, float fZ)
{
    CompressedPos_SetPos(vertex.pos, fX, fY, fZ);
    vertex.color = RedColor::WHITE;
}

// FUNCTION: SWBF2 0x0041e980
template <>
void LensFlare::Emit<FlareCompressedVertex>(FlareCompressedVertex*& pVertices,
                                           const PblVector3& pos, float fU,
                                           float fV, float fFade)
{
    const float angle = ((pos.y * 2.0f * 3.1415901f + pos.x * 3.1415901f) / pos.z)
                      + _fSize * 1.0472f + _fRandomRotOffset;
    float fSin[2], fCos[2];
    PblMath::SinCos(&fSin[0], &fCos[0], angle);
    PblMath::SinCos(&fSin[1], &fCos[1], 1.0472f - angle);
    const float size = _fSize * 2.0f;
    PblVector3 vVert[6];
    vVert[0] = pos + PblVector3(fCos[0] * size, fSin[0] * (size * 2.0f), 0.0f);
    vVert[1] = pos + PblVector3((fCos[0] * -0.5f - fSin[0] * 0.866f) * size,
                               (fCos[0] * 0.866f - fSin[0] * 0.5f) * (size * 2.0f), 0.0f);
    vVert[2] = pos + PblVector3((fCos[0] * -0.5f - fSin[0] * -0.866f) * size,
                               (fSin[0] * -0.5f - fCos[0] * 0.866f) * (size * 2.0f), 0.0f);
    vVert[3] = pos + PblVector3(fCos[1] * (size * 3.5f), fSin[1] * size, 0.0f);
    vVert[4] = pos + PblVector3((fCos[1] * -0.5f - fSin[1] * 0.866f) * (size * 3.5f),
                               (fCos[1] * 0.866f - fSin[1] * 0.5f) * size, 0.0f);
    vVert[5] = pos + PblVector3((fCos[1] * -0.5f - fSin[1] * -0.866f) * (size * 3.5f),
                               (fSin[1] * -0.5f - fCos[1] * 0.866f) * size, 0.0f);

    RedColor faded(_color, (unsigned char)(int)((float)_color.a * fFade));
    RedColor black(RedColor::BLACK, 0);
    pVertices[0].pos.SetPos(vVert[0].x, vVert[0].y, vVert[0].z);
    pVertices[0].uv.SetUV(fU, fV);
    pVertices[0].color = black;
    pVertices[1] = pVertices[0];
    pVertices[2].pos.SetPos(vVert[1].x, vVert[1].y, vVert[1].z);
    pVertices[2].uv.SetUV(fU, fV);
    pVertices[2].color = black;
    pVertices[3].pos.SetPos(pos.x, pos.y, pos.z);
    pVertices[3].uv.SetUV(fU, fV);
    pVertices[3].color = faded;
    pVertices[4].pos.SetPos(vVert[2].x, vVert[2].y, vVert[2].z);
    pVertices[4].uv.SetUV(fU, fV);
    pVertices[4].color = black;
    pVertices[5].pos.SetPos(vVert[0].x, vVert[0].y, vVert[0].z);
    pVertices[5].uv.SetUV(fU, fV);
    pVertices[5].color = black;
    pVertices[6] = pVertices[5];
    pVertices[7].pos.SetPos(vVert[3].x, vVert[3].y, vVert[3].z);
    pVertices[7].uv.SetUV(fU, fV);
    pVertices[7].color = black;
    pVertices[8] = pVertices[7];
    pVertices[9].pos.SetPos(vVert[4].x, vVert[4].y, vVert[4].z);
    pVertices[9].uv.SetUV(fU, fV);
    pVertices[9].color = black;
    pVertices[10].pos.SetPos(pos.x, pos.y, pos.z);
    pVertices[10].uv.SetUV(fU, fV);
    pVertices[10].color = faded;
    pVertices[11].pos.SetPos(vVert[5].x, vVert[5].y, vVert[5].z);
    pVertices[11].uv.SetUV(fU, fV);
    pVertices[11].color = black;
    pVertices[12].pos.SetPos(vVert[3].x, vVert[3].y, vVert[3].z);
    pVertices[12].uv.SetUV(fU, fV);
    pVertices[12].color = black;
    pVertices[13] = pVertices[12];
    pVertices += 14;

    RedColor clear(_color, 0);
    RedColor half(_color, _color.a >> 1);
    EmitRing(pVertices, pos, angle, 0.0f, _fSize, _fSize * 4.0f,
             _color, half, clear, 12, fU, fV);
}

static void SetFillVertex(FlareVertex& vertex, float fX, float fY, float fZ)
{
    vertex.pos.x = fX;
    vertex.pos.y = fY;
    vertex.pos.z = fZ;
    vertex.color = RedColor::WHITE;
}

// FUNCTION: SWBF2 0x0041e3b0
template <>
void LensFlare::Emit<FlareVertex>(FlareVertex*& pVertices, const PblVector3& pos, float fU,
                                  float fV, float fFade)
{
    const float angle = ((pos.y * 2.0f * 3.1415901f + pos.x * 3.1415901f) / pos.z)
                      + _fSize * 1.0472f + _fRandomRotOffset;
    float fSin[2], fCos[2];
    PblMath::SinCos(&fSin[0], &fCos[0], angle);
    PblMath::SinCos(&fSin[1], &fCos[1], 1.0472f - angle);
    const float size = _fSize * 2.0f;
    PblVector3 vVert[6];
    vVert[0] = pos + PblVector3(fCos[0] * size, fSin[0] * (size * 2.0f), 0.0f);
    vVert[1] = pos + PblVector3((fCos[0] * -0.5f - fSin[0] * 0.866f) * size,
                               (fCos[0] * 0.866f - fSin[0] * 0.5f) * (size * 2.0f), 0.0f);
    vVert[2] = pos + PblVector3((fCos[0] * -0.5f - fSin[0] * -0.866f) * size,
                               (fSin[0] * -0.5f - fCos[0] * 0.866f) * (size * 2.0f), 0.0f);
    vVert[3] = pos + PblVector3(fCos[1] * (size * 3.5f), fSin[1] * size, 0.0f);
    vVert[4] = pos + PblVector3((fCos[1] * -0.5f - fSin[1] * 0.866f) * (size * 3.5f),
                               (fCos[1] * 0.866f - fSin[1] * 0.5f) * size, 0.0f);
    vVert[5] = pos + PblVector3((fCos[1] * -0.5f - fSin[1] * -0.866f) * (size * 3.5f),
                               (fSin[1] * -0.5f - fCos[1] * 0.866f) * size, 0.0f);

    RedColor faded(_color, (unsigned char)(int)((float)_color.a * fFade));
    RedColor black(RedColor::BLACK, 0);
    pVertices[0].pos.SetPos(vVert[0].x, vVert[0].y, vVert[0].z);
    pVertices[0].uv.SetUV(fU, fV);
    pVertices[0].color = black;
    pVertices[1] = pVertices[0];
    pVertices[2].pos.SetPos(vVert[1].x, vVert[1].y, vVert[1].z);
    pVertices[2].uv.SetUV(fU, fV);
    pVertices[2].color = black;
    pVertices[3].pos.SetPos(pos.x, pos.y, pos.z);
    pVertices[3].uv.SetUV(fU, fV);
    pVertices[3].color = faded;
    pVertices[4].pos.SetPos(vVert[2].x, vVert[2].y, vVert[2].z);
    pVertices[4].uv.SetUV(fU, fV);
    pVertices[4].color = black;
    pVertices[5].pos.SetPos(vVert[0].x, vVert[0].y, vVert[0].z);
    pVertices[5].uv.SetUV(fU, fV);
    pVertices[5].color = black;
    pVertices[6] = pVertices[5];
    pVertices[7].pos.SetPos(vVert[3].x, vVert[3].y, vVert[3].z);
    pVertices[7].uv.SetUV(fU, fV);
    pVertices[7].color = black;
    pVertices[8] = pVertices[7];
    pVertices[9].pos.SetPos(vVert[4].x, vVert[4].y, vVert[4].z);
    pVertices[9].uv.SetUV(fU, fV);
    pVertices[9].color = black;
    pVertices[10].pos.SetPos(pos.x, pos.y, pos.z);
    pVertices[10].uv.SetUV(fU, fV);
    pVertices[10].color = faded;
    pVertices[11].pos.SetPos(vVert[5].x, vVert[5].y, vVert[5].z);
    pVertices[11].uv.SetUV(fU, fV);
    pVertices[11].color = black;
    pVertices[12].pos.SetPos(vVert[3].x, vVert[3].y, vVert[3].z);
    pVertices[12].uv.SetUV(fU, fV);
    pVertices[12].color = black;
    pVertices[13] = pVertices[12];
    pVertices += 14;

    RedColor clear(_color, 0);
    RedColor half(_color, _color.a >> 1);
    EmitRing(pVertices, pos, angle, 0.0f, _fSize, _fSize * 4.0f,
             _color, half, clear, 12, fU, fV);
}

// FUNCTION: SWBF2 0x0041c1b0
template <>
void LensFlareEffect::LensFlareList::Render<FlareVertex, 642>() const
{
    const PblMatrix& mView = RedRenderer::GetCamera()->GetMatrixInverse();
    RedRenderer::GetCamera();
    float fNear;
    float fFar;
    float fWidth;
    float fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const unsigned int uiView = FLRenderer::GetCurrentViewport();
    float fViewMinX;
    float fViewMinY;
    float fViewMaxX;
    float fViewMaxY;
    RedRenderer::pcGetViewportExtents(fViewMinX, fViewMinY, fViewMaxX, fViewMaxY);

    PblVector3 vPosition;
    float fSize = 0.0f;
    float fSquaredDistance = 0.0f;
    Itor it(*this);
    for (; it; ++it)
    {
        if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition, fSize,
                           fSquaredDistance))
        {
            break;
        }
    }
    if (!it)
    {
        return;
    }

    do
    {
        int iNumFlares = 0;
        pcRedVertexBuffer* pFlareBuffer = pcRedVertexBuffer::Create(1920, sizeof(FlareVertex), false, true);
        pcRedVertexBuffer* pFillBuffer = pcRedVertexBuffer::Create(96, sizeof(FlareVertex), false, true);
        pcRedVertexBuffer* pClearBuffer = pcRedVertexBuffer::Create(96, 4 * sizeof(float), false, true);

        CompressedPos::SetBoundingBox(s_boundingBox);
        CompressedPosXY::SetBoundingBox(PblBox(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

        unsigned int uiFlareStart;
        unsigned int uiFillStart;
        unsigned int uiClearStart;
        FlareVertex* pFlareVertices = (FlareVertex*)pFlareBuffer->Lock(1920, uiFlareStart);
        FlareVertex* const pFlareBase = pFlareVertices;
        FlareVertex* pFillVertices = (FlareVertex*)pFillBuffer->Lock(96, uiFillStart);
        FlareVertex* const pFillBase = pFillVertices;
        PblVector4* pClearVertices = (PblVector4*)pClearBuffer->Lock(96, uiClearStart);
        PblVector4* const pClearBase = pClearVertices;

        RECT destRects[16];
        RECT sourceRects[16];
        float fSampleY = 0.0f;
        float fSampleX = 0.0f;
        float fU = 0.03125f;
        float fV = 0.03125f;
        do
        {
            float fFade = 1.0f;
            if (fSquaredDistance > it->GetSquaredDrawDistance() * kfFadeDrawDistanceSquared)
            {
                fFade = (1.0f - fSquaredDistance / it->GetSquaredDrawDistance()) * kfFadeDrawDistanceFactor;
            }

            const float fMinX = vPosition.x - fSize;
            const float fMinY = vPosition.y - fSize;
            const PblVector3 vScreenMin = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMinX, fMinY, vPosition.z));
            const float fMaxX = vPosition.x + fSize;
            const float fMaxY = vPosition.y + fSize;
            const PblVector3 vScreenMax = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMaxX, fMaxY, vPosition.z));

            pClearVertices[0].Set(vScreenMin.x - 2.0f, vScreenMax.y - 2.0f, 0.0f, 1.0f);
            pClearVertices[1].Set(vScreenMin.x - 2.0f, vScreenMin.y + 2.0f, 0.0f, 1.0f);
            pClearVertices[2].Set(vScreenMax.x + 2.0f, vScreenMax.y - 2.0f, 0.0f, 1.0f);
            pClearVertices[3].Set(vScreenMin.x - 2.0f, vScreenMin.y + 2.0f, 0.0f, 1.0f);
            pClearVertices[4].Set(vScreenMax.x + 2.0f, vScreenMax.y - 2.0f, 0.0f, 1.0f);
            pClearVertices[5].Set(vScreenMax.x + 2.0f, vScreenMin.y + 2.0f, 0.0f, 1.0f);
            pClearVertices += 6;

            const float fZ = vPosition.z + fSize;
            SetFillVertex(pFillVertices[0], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[1], fMaxX, fMaxY, fZ);
            SetFillVertex(pFillVertices[2], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[3], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[4], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[5], fMinX, fMinY, fZ);
            pFillVertices += 6;

            RECT source;
            source.left = (int)vScreenMin.x;
            source.top = (int)vScreenMax.y;
            source.right = (int)vScreenMax.x + 1;
            source.bottom = (int)vScreenMin.y + 1;
            RECT dest;
            dest.left = (int)fSampleX;
            dest.top = (int)fSampleY;
            fSampleX += 16.0f;
            dest.right = (int)fSampleX;
            dest.bottom = (int)(fSampleY + 16.0f);
            if (fViewMinX > (float)source.left)
            {
                source.left = (int)fViewMinX;
            }
            if ((float)source.right > fViewMaxX)
            {
                source.right = (int)fViewMaxX;
            }
            if (fViewMinY > (float)source.top)
            {
                source.top = (int)fViewMinY;
            }
            if ((float)source.bottom > fViewMaxY)
            {
                source.bottom = (int)fViewMaxY;
            }
            sourceRects[iNumFlares] = source;
            destRects[iNumFlares] = dest;

            it->Emit(pFlareVertices, vPosition, fU, fV, fFade);

            fU += 0.0625f;
            if (fU >= 1.0f)
            {
                fV += 0.0625f;
                fU = 0.03125f;
            }
            if (fSampleX >= 64.0f)
            {
                fSampleY += 16.0f;
                fSampleX = 0.0f;
            }

            ++iNumFlares;
            for (++it; it; ++it)
            {
                if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition,
                                   fSize, fSquaredDistance))
                {
                    break;
                }
            }
        } while (iNumFlares < 16 && it);

        const int iNumFillVertices = pFillVertices - pFillBase;
        const int iNumFlareVertices = pFlareVertices - pFlareBase;
        const int iNumClearVertices = pClearVertices - pClearBase;
        pFillBuffer->Unlock();
        pFlareBuffer->Unlock();
        pClearBuffer->Unlock();

        pcRedPrimitive* pFlarePrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLESTRIP, iNumFlareVertices - 2, 642, iNumFlareVertices, uiFlareStart,
            pFlareBuffer, 0, 0);
        pFlarePrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pFillPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumFillVertices / 3, 642, iNumFillVertices, uiFillStart,
            pFillBuffer, 0, 0);
        pFillPrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pClearPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumClearVertices / 3, 3, iNumClearVertices, uiClearStart,
            pClearBuffer, 0, 0);

        RedColor clearColor = { 0, 0, 0, 0 };
        RedRenderer::pcRenderPrimitive(pClearPrimitive, s_pClearShader, gMatrixIdentity,
                                       clearColor, 0x200, 0, 0, 0);
        RedColor fillColor(RedColor::BLACK, 255);
        RedRenderer::pcRenderPrimitive(pFillPrimitive, s_pFillShader,
                                       RedRenderer::GetCamera()->GetMatrix(), fillColor, 0x200, 0, 0, 0);

        IDirect3DSurface9* pBackBuffer;
        RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        IDirect3DSurface9* pSampleSurface;
        ((IDirect3DTexture9*)s_sampleTex->m_pTexture)->GetSurfaceLevel(0, &pSampleSurface);
        RedRenderer::pcGetDevice()->ColorFill(pSampleSurface, 0, 0);
        for (int i = 0; i < iNumFlares; i++)
        {
            RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &sourceRects[i], pSampleSurface,
                                                    &destRects[i], D3DTEXF_NONE);
        }
        pBackBuffer->Release();
        pSampleSurface->Release();

        RedTexture* pRenderTarget = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepthTarget = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(s_intensityTex, 0);
        RedColor intensityClear(RedColor::BLACK, 0);
        RedRenderer::pcClear(1, intensityClear, 1.0f, 0);

        float fOutputX = 0.0f;
        float fOutputY = 0.0f;
        float fInputX = 0.0f;
        float fInputY = 0.0f;
        for (int i = 0; i < iNumFlares; i++)
        {
            s_pSampleShader->SetOutputPosition(fOutputX, fOutputY);
            s_pSampleShader->SetInputOffset(fInputX, fInputY);
            RedRenderer::pcRenderPrimitive(s_samplePoints, s_pSampleShader,
                                           gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
            fOutputX += 1.0f;
            if (fOutputX >= 16.0f)
            {
                fOutputY += 1.0f;
                fOutputX = 0.0f;
            }
            fInputX += 16.0f;
            if (fInputX >= 64.0f)
            {
                fInputY += 16.0f;
                fInputX = 0.0f;
            }
        }

        RedRenderer::pcSetRenderTarget(pRenderTarget, pDepthTarget);
        RedRenderer::pcSetViewport(viewport);
        RedRenderer::pcRenderPrimitive(pFlarePrimitive, s_pFlareShader,
                                       RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);

        pFlarePrimitive->Release();
        pFillPrimitive->Release();
        pClearPrimitive->Release();
        pFlareBuffer->Release();
        pFillBuffer->Release();
        pClearBuffer->Release();
    } while (it);
}

// FUNCTION: SWBF2 0x0041d1e0
template <>
void LensFlareEffect::LensFlareList::Render<FlareCompressedVertex, 37506>() const
{
    const PblMatrix& mView = RedRenderer::GetCamera()->GetMatrixInverse();
    RedRenderer::GetCamera();
    float fNear;
    float fFar;
    float fWidth;
    float fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const unsigned int uiView = FLRenderer::GetCurrentViewport();
    float fViewMinX;
    float fViewMinY;
    float fViewMaxX;
    float fViewMaxY;
    RedRenderer::pcGetViewportExtents(fViewMinX, fViewMinY, fViewMaxX, fViewMaxY);

    PblVector3 vPosition;
    float fSize = 0.0f;
    float fSquaredDistance = 0.0f;
    Itor it(*this);
    for (; it; ++it)
    {
        if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition, fSize,
                           fSquaredDistance))
        {
            break;
        }
    }
    if (!it)
    {
        return;
    }

    do
    {
        int iNumFlares = 0;
        pcRedVertexBuffer* pFlareBuffer = pcRedVertexBuffer::Create(1920, sizeof(FlareCompressedVertex), false, true);
        pcRedVertexBuffer* pFillBuffer = pcRedVertexBuffer::Create(96, sizeof(FlareCompressedVertex), false, true);
        pcRedVertexBuffer* pClearBuffer = pcRedVertexBuffer::Create(96, 4 * sizeof(float), false, true);

        CompressedPos::SetBoundingBox(s_boundingBox);
        CompressedPosXY::SetBoundingBox(PblBox(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

        unsigned int uiFlareStart;
        unsigned int uiFillStart;
        unsigned int uiClearStart;
        FlareCompressedVertex* pFlareVertices = (FlareCompressedVertex*)pFlareBuffer->Lock(1920, uiFlareStart);
        FlareCompressedVertex* const pFlareBase = pFlareVertices;
        FlareCompressedVertex* pFillVertices = (FlareCompressedVertex*)pFillBuffer->Lock(96, uiFillStart);
        FlareCompressedVertex* const pFillBase = pFillVertices;
        PblVector4* pClearVertices = (PblVector4*)pClearBuffer->Lock(96, uiClearStart);
        PblVector4* const pClearBase = pClearVertices;

        RECT destRects[16];
        RECT sourceRects[16];
        float fSampleY = 0.0f;
        float fSampleX = 0.0f;
        float fU = 0.03125f;
        float fV = 0.03125f;
        do
        {
            float fFade = 1.0f;
            if (fSquaredDistance > it->GetSquaredDrawDistance() * kfFadeDrawDistanceSquared)
            {
                fFade = (1.0f - fSquaredDistance / it->GetSquaredDrawDistance()) * kfFadeDrawDistanceFactor;
            }

            const float fMinX = vPosition.x - fSize;
            const float fMinY = vPosition.y - fSize;
            const PblVector3 vScreenMin = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMinX, fMinY, vPosition.z));
            const float fMaxX = vPosition.x + fSize;
            const float fMaxY = vPosition.y + fSize;
            const PblVector3 vScreenMax = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMaxX, fMaxY, vPosition.z));

            pClearVertices[0].Set(vScreenMin.x - 2.0f, vScreenMax.y - 2.0f, 0.0f, 1.0f);
            pClearVertices[1].Set(vScreenMin.x - 2.0f, vScreenMin.y + 2.0f, 0.0f, 1.0f);
            pClearVertices[2].Set(vScreenMax.x + 2.0f, vScreenMax.y - 2.0f, 0.0f, 1.0f);
            pClearVertices[3].Set(vScreenMin.x - 2.0f, vScreenMin.y + 2.0f, 0.0f, 1.0f);
            pClearVertices[4].Set(vScreenMax.x + 2.0f, vScreenMax.y - 2.0f, 0.0f, 1.0f);
            pClearVertices[5].Set(vScreenMax.x + 2.0f, vScreenMin.y + 2.0f, 0.0f, 1.0f);
            pClearVertices += 6;

            const float fZ = vPosition.z + fSize;
            SetFillVertex(pFillVertices[0], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[1], fMaxX, fMaxY, fZ);
            SetFillVertex(pFillVertices[2], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[3], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[4], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[5], fMinX, fMinY, fZ);
            pFillVertices += 6;

            RECT source;
            source.left = (int)vScreenMin.x;
            source.top = (int)vScreenMax.y;
            source.right = (int)vScreenMax.x + 1;
            source.bottom = (int)vScreenMin.y + 1;
            RECT dest;
            dest.left = (int)fSampleX;
            dest.top = (int)fSampleY;
            fSampleX += 16.0f;
            dest.right = (int)fSampleX;
            dest.bottom = (int)(fSampleY + 16.0f);
            if (fViewMinX > (float)source.left)
            {
                source.left = (int)fViewMinX;
            }
            if ((float)source.right > fViewMaxX)
            {
                source.right = (int)fViewMaxX;
            }
            if (fViewMinY > (float)source.top)
            {
                source.top = (int)fViewMinY;
            }
            if ((float)source.bottom > fViewMaxY)
            {
                source.bottom = (int)fViewMaxY;
            }
            sourceRects[iNumFlares] = source;
            destRects[iNumFlares] = dest;

            it->Emit(pFlareVertices, vPosition, fU, fV, fFade);

            fU += 0.0625f;
            if (fU >= 1.0f)
            {
                fV += 0.0625f;
                fU = 0.03125f;
            }
            if (fSampleX >= 64.0f)
            {
                fSampleY += 16.0f;
                fSampleX = 0.0f;
            }

            ++iNumFlares;
            for (++it; it; ++it)
            {
                if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition,
                                   fSize, fSquaredDistance))
                {
                    break;
                }
            }
        } while (iNumFlares < 16 && it);

        const int iNumFillVertices = pFillVertices - pFillBase;
        const int iNumFlareVertices = pFlareVertices - pFlareBase;
        const int iNumClearVertices = pClearVertices - pClearBase;
        pFillBuffer->Unlock();
        pFlareBuffer->Unlock();
        pClearBuffer->Unlock();

        pcRedPrimitive* pFlarePrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLESTRIP, iNumFlareVertices - 2, 37506, iNumFlareVertices, uiFlareStart,
            pFlareBuffer, 0, 0);
        pFlarePrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pFillPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumFillVertices / 3, 37506, iNumFillVertices, uiFillStart,
            pFillBuffer, 0, 0);
        pFillPrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pClearPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumClearVertices / 3, 3, iNumClearVertices, uiClearStart,
            pClearBuffer, 0, 0);

        RedColor clearColor = { 0, 0, 0, 0 };
        RedRenderer::pcRenderPrimitive(pClearPrimitive, s_pClearShader, gMatrixIdentity,
                                       clearColor, 0x200, 0, 0, 0);
        RedColor fillColor(RedColor::BLACK, 255);
        RedRenderer::pcRenderPrimitive(pFillPrimitive, s_pFillShader,
                                       RedRenderer::GetCamera()->GetMatrix(), fillColor, 0x200, 0, 0, 0);

        IDirect3DSurface9* pBackBuffer;
        RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        IDirect3DSurface9* pSampleSurface;
        ((IDirect3DTexture9*)s_sampleTex->m_pTexture)->GetSurfaceLevel(0, &pSampleSurface);
        RedRenderer::pcGetDevice()->ColorFill(pSampleSurface, 0, 0);
        for (int i = 0; i < iNumFlares; i++)
        {
            RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &sourceRects[i], pSampleSurface,
                                                    &destRects[i], D3DTEXF_NONE);
        }
        pBackBuffer->Release();
        pSampleSurface->Release();

        RedTexture* pRenderTarget = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepthTarget = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(s_intensityTex, 0);
        RedColor intensityClear(RedColor::BLACK, 0);
        RedRenderer::pcClear(1, intensityClear, 1.0f, 0);

        float fOutputX = 0.0f;
        float fOutputY = 0.0f;
        float fInputX = 0.0f;
        float fInputY = 0.0f;
        for (int i = 0; i < iNumFlares; i++)
        {
            s_pSampleShader->SetOutputPosition(fOutputX, fOutputY);
            s_pSampleShader->SetInputOffset(fInputX, fInputY);
            RedRenderer::pcRenderPrimitive(s_samplePoints, s_pSampleShader,
                                           gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
            fOutputX += 1.0f;
            if (fOutputX >= 16.0f)
            {
                fOutputY += 1.0f;
                fOutputX = 0.0f;
            }
            fInputX += 16.0f;
            if (fInputX >= 64.0f)
            {
                fInputY += 16.0f;
                fInputX = 0.0f;
            }
        }

        RedRenderer::pcSetRenderTarget(pRenderTarget, pDepthTarget);
        RedRenderer::pcSetViewport(viewport);
        RedRenderer::pcRenderPrimitive(pFlarePrimitive, s_pFlareShader,
                                       RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);

        pFlarePrimitive->Release();
        pFillPrimitive->Release();
        pClearPrimitive->Release();
        pFlareBuffer->Release();
        pFillBuffer->Release();
        pClearBuffer->Release();
    } while (it);
}
