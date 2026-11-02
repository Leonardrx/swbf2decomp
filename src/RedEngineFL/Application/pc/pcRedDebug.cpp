#include <d3dx9math.h>
#include <stdarg.h>
#include "PebbleFL/Common/PblVector.h"

#include "pcRedDebug.h"
#include "pcRedShape.h"

#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);

void RemoveInterfaceElement(RedScreenGroupElement* pElement);
}

void __cdecl PblTraceP(const char* Format, ...);


// SYNTHETIC: SWBF2 0x00408f10
// `dynamic initializer for 'RedDebug::_PrintfColor''

// GLOBAL: SWBF2 0x009ae64c
// RedDebug::_PrintfColor
RedColor RedDebug::_PrintfColor = RedColor::WHITE;

// SYNTHETIC: SWBF2 0x00408f40
// `dynamic initializer for 'RedDebug::_ScreenGroup''
// SYNTHETIC: SWBF2 0x00769b30
// `dynamic atexit destructor for 'RedDebug::_ScreenGroup''

// GLOBAL: SWBF2 0x009af450
// RedDebug::_ScreenGroup
RedScreenGroupElement RedDebug::_ScreenGroup("_DebugGroup");

struct RedDebug::Radius
{
    Radius();

    PblVector3 _vPos;
    RedColor _color;
    float _fRadius;
    float _fHeight;
};
 

struct RedDebug::SolidOBB
{
    SolidOBB();

    PblBox _vBox;
    PblMatrix _mBox;
    RedColor _color;
};
 
 

// FUNCTION: SWBF2 0x006f0780
RedDebug::Radius::Radius()
{
}

// FUNCTION: SWBF2 0x006f08a0
RedDebug::SolidOBB::SolidOBB()
{
}

// SYNTHETIC: SWBF2 0x00408f80
// `dynamic initializer for 'RedDebug::_aMarkers3D''
// SYNTHETIC: SWBF2 0x00769b40
// `dynamic atexit destructor for 'RedDebug::_aMarkers3D''

// GLOBAL: SWBF2 0x009c7c10
// RedDebug::_aMarkers3D
RedTetrahedronShape RedDebug::_aMarkers3D[16];

// SYNTHETIC: SWBF2 0x00408fb0
// `dynamic initializer for 'RedDebug::_aOBB''

// GLOBAL: SWBF2 0x009ae650
// RedDebug::_aOBB
RedDebug::SolidOBB RedDebug::_aOBB[32];

// SYNTHETIC: SWBF2 0x00408fd0
// `dynamic initializer for 'RedDebug::_aRadii''

// GLOBAL: SWBF2 0x009b57d0
// RedDebug::_aRadii
RedDebug::Radius RedDebug::_aRadii[32];

// SYNTHETIC: SWBF2 0x00408ff0
// `dynamic initializer for 'RedDebug::_aSpheres''
// SYNTHETIC: SWBF2 0x00769b60
// `dynamic atexit destructor for 'RedDebug::_aSpheres''

// GLOBAL: SWBF2 0x009c7410
// RedDebug::_aSpheres
RedSphereShape RedDebug::_aSpheres[16];

// SYNTHETIC: SWBF2 0x00409020
// `dynamic initializer for 'RedDebug::_aText3D''
// SYNTHETIC: SWBF2 0x00769b80
// `dynamic atexit destructor for 'RedDebug::_aText3D''

// GLOBAL: SWBF2 0x009b23d0
// RedDebug::_aText3D
RedDebug::Text3D RedDebug::_aText3D[64];

// SYNTHETIC: SWBF2 0x00409050
// `dynamic initializer for 'RedDebug::_aTextLines''
// SYNTHETIC: SWBF2 0x00769ba0
// `dynamic atexit destructor for 'RedDebug::_aTextLines''

// GLOBAL: SWBF2 0x009acfb0
// RedDebug::_aTextLines
RedTextElement RedDebug::_aTextLines[30];

// SYNTHETIC: SWBF2 0x00409080
// `dynamic initializer for 'RedDebug::_kStaticRenderable''
// SYNTHETIC: SWBF2 0x00769bc0
// `dynamic atexit destructor for 'RedDebug::_kStaticRenderable''

// GLOBAL: SWBF2 0x009ae630
// RedDebug::_kStaticRenderable
RedDebug::RedDebugRenderable RedDebug::_kStaticRenderable;

// GLOBAL: SWBF2 0x009ace98
char RedDebug::_aLineBuffer[256];

// GLOBAL: SWBF2 0x009acf98
char* RedDebug::_pCurrLinePos;

// GLOBAL: SWBF2 0x009acf9c
unsigned int RedDebug::_uiNumTextLines;

// GLOBAL: SWBF2 0x009acfa0
unsigned int RedDebug::_uiNumText3D;

// GLOBAL: SWBF2 0x009acfa4
RedGlyphCacheElement::TextStyleT RedDebug::_ePrintfStyle;

// GLOBAL: SWBF2 0x009c8410
pcRedShader* s_debugShader;

// GLOBAL: SWBF2 0x009ace84
unsigned int RedDebug::_n3DLines;
// GLOBAL: SWBF2 0x009ace88
unsigned int RedDebug::_nSpheres;
// GLOBAL: SWBF2 0x009ace8c
unsigned int RedDebug::_nMarkers3D;
// GLOBAL: SWBF2 0x009ace90
unsigned int RedDebug::_nRadii;
// GLOBAL: SWBF2 0x009ace94
unsigned int RedDebug::_nOBB;

// GLOBAL: SWBF2 0x009af4f0
RedColor RedDebug::_aColor[3000];

// SYNTHETIC: SWBF2 0x00408f60
// `dynamic initializer for 'RedDebug::_a3DLineVectors''

// GLOBAL: SWBF2 0x009b5ad0
// RedDebug::_a3DLineVectors
PblVector3 RedDebug::_a3DLineVectors[6000];

// FUNCTION: SWBF2 0x006f0790
RedDebug::RedDebugRenderable::RedDebugRenderable()
    : Renderable(7)
{
}

// FUNCTION: SWBF2 0x006f08b0
RedDebug::Text3D::Text3D()
{
}


// FUNCTION: SWBF2 0x006f07b0
RedShape::RedShape()
    : Renderable(6), _Matrix(gMatrixIdentity), _pMatrix(&_Matrix),
      _vMin(-1.0f, -1.0f, -1.0f), _vMax(1.0f, 1.0f, 1.0f)
{
    SetColor(RedColor::WHITE);
}

// FUNCTION: SWBF2 0x006f0860
RedSphereShape::RedSphereShape()
{
}

// FUNCTION: SWBF2 0x006f0880
RedTetrahedronShape::RedTetrahedronShape()
{
}

// FUNCTION: SWBF2 0x006f09b0
void RedDebug::RedDebugRenderable::Cleanup()
{
    FLRenderer::RemoveInterfaceElement(&_ScreenGroup);

    for (unsigned int i = 0; i < 30; i++)
    {
        RedTextElement& line = _aTextLines[i];
        line.SetFont("");
        line.RedTextElement::Cleanup();
        _ScreenGroup.RemoveElement(&line);
    }

    for (unsigned int i = 0; i < 64; i++)
    {
        RedTextElement& text = _aText3D[i].text;
        text.SetFont("");
        text.RedTextElement::Cleanup();
        _ScreenGroup.RemoveElement(&text);
    }

    if (s_debugShader)
    {
        s_debugShader->Release();
        s_debugShader = 0;
    }
}

// FUNCTION: SWBF2 0x006f0a70
void RedDebug::DrawAABB(const PblBox& box, RedColor color)
{
    const PblVector3& vMin = box._vMin;
    const PblVector3& vMax = box._vMax;
    DrawLine3D(PblVector3(vMin.x, vMin.y, vMin.z), PblVector3(vMin.x, vMin.y, vMax.z), color);
    DrawLine3D(PblVector3(vMin.x, vMin.y, vMin.z), PblVector3(vMin.x, vMax.y, vMin.z), color);
    DrawLine3D(PblVector3(vMin.x, vMin.y, vMin.z), PblVector3(vMax.x, vMin.y, vMin.z), color);
    DrawLine3D(PblVector3(vMin.x, vMax.y, vMax.z), PblVector3(vMin.x, vMin.y, vMax.z), color);
    DrawLine3D(PblVector3(vMin.x, vMax.y, vMax.z), PblVector3(vMin.x, vMax.y, vMin.z), color);
    DrawLine3D(PblVector3(vMin.x, vMax.y, vMax.z), PblVector3(vMax.x, vMax.y, vMax.z), color);
    DrawLine3D(PblVector3(vMax.x, vMin.y, vMax.z), PblVector3(vMin.x, vMin.y, vMax.z), color);
    DrawLine3D(PblVector3(vMax.x, vMin.y, vMax.z), PblVector3(vMax.x, vMin.y, vMin.z), color);
    DrawLine3D(PblVector3(vMax.x, vMin.y, vMax.z), PblVector3(vMax.x, vMax.y, vMax.z), color);
    DrawLine3D(PblVector3(vMax.x, vMax.y, vMin.z), PblVector3(vMin.x, vMax.y, vMin.z), color);
    DrawLine3D(PblVector3(vMax.x, vMax.y, vMin.z), PblVector3(vMax.x, vMin.y, vMin.z), color);
    DrawLine3D(PblVector3(vMax.x, vMax.y, vMin.z), PblVector3(vMax.x, vMax.y, vMax.z), color);
}

// FUNCTION: SWBF2 0x006f0ee0
void RedDebug::DrawCone(const PblMatrix& matrix, float fHeight, float fRadius0, float fRadius1,
                        RedColor color)
{
    const float fStep = PblMath::fTwoPi * 0.125f;
    const PblVector3 vBase = matrix.GetTranslation();

    PblVector3 aCircle[8];
    for (int i = 0; i < 8; i++)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * fStep);
        aCircle[i] = matrix.GetXAxis() * fSin + matrix.GetZAxis() * fCos;
    }

    const PblVector3 vTop = vBase + matrix.GetYAxis() * fHeight;

    for (int i = 0; i < 8; i++)
    {
        const PblVector3& c0 = aCircle[i];
        DrawLine3D(vBase + c0 * fRadius0, vTop + c0 * fRadius1, color);
        const int iNext = (i + 1) % 8;
        DrawLine3D(vBase + c0 * fRadius0, vBase + aCircle[iNext] * fRadius0, color);
        DrawLine3D(vTop + c0 * fRadius1, vTop + aCircle[iNext] * fRadius1, color);
    }
}

// FUNCTION: SWBF2 0x006f12a0
void RedDebug::DrawLine3D(PblVector3 v0, PblVector3 v1, RedColor color)
{
    if (_n3DLines < 6000 - 2)
    {
        _aColor[_n3DLines / 2] = color;
        _a3DLineVectors[_n3DLines++] = v0;
        _a3DLineVectors[_n3DLines++] = v1;
    }
}

// FUNCTION: SWBF2 0x006f1300
void RedDebug::DrawOBB(const PblBox& box, const PblMatrix& matrix, RedColor color)
{
    PblVector3 aCorners[8];
    for (int i = 0; i < 8; i++)
    {
        PblVector3 vCorner((i & 4) ? box._vMax.x : box._vMin.x,
                           (i & 2) ? box._vMax.y : box._vMin.y,
                           (i & 1) ? box._vMax.z : box._vMin.z);
        PblVector3 vOut;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vOut), reinterpret_cast<const D3DXVECTOR3*>(&vCorner), reinterpret_cast<const D3DXMATRIX*>(&matrix));
        aCorners[i] = vOut;
    }

    DrawLine3D(aCorners[0], aCorners[1], color);
    DrawLine3D(aCorners[0], aCorners[2], color);
    DrawLine3D(aCorners[0], aCorners[4], color);
    DrawLine3D(aCorners[3], aCorners[1], color);
    DrawLine3D(aCorners[3], aCorners[2], color);
    DrawLine3D(aCorners[3], aCorners[7], color);
    DrawLine3D(aCorners[5], aCorners[1], color);
    DrawLine3D(aCorners[5], aCorners[4], color);
    DrawLine3D(aCorners[5], aCorners[7], color);
    DrawLine3D(aCorners[6], aCorners[2], color);
    DrawLine3D(aCorners[6], aCorners[4], color);
    DrawLine3D(aCorners[6], aCorners[7], color);
}

static inline PblVector3 VecAdd(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline PblVector3 VecSub(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// FUNCTION: SWBF2 0x006f16f0
void RedDebug::DrawSphere(PblVector3 vCenter, float fRadius, RedColor color)
{
    const float fDiag = fRadius * 0.7071f;
    const float fNegDiag = -fDiag;

    const PblVector3 vX(fRadius, 0.0f, 0.0f);
    const PblVector3 vY(0.0f, fRadius, 0.0f);
    const PblVector3 vZ(0.0f, 0.0f, fRadius);
    const PblVector3 vXY(fDiag, fDiag, 0.0f);
    const PblVector3 vXnY(fDiag, fNegDiag, 0.0f);
    const PblVector3 vYZ(0.0f, fDiag, fDiag);
    const PblVector3 vYnZ(0.0f, fDiag, fNegDiag);
    const PblVector3 vXZ(fDiag, 0.0f, fDiag);
    const PblVector3 vnXZ(fNegDiag, 0.0f, fDiag);

    DrawLine3D(vCenter - vX, vCenter + vX, color);
    DrawLine3D(vCenter - vY, vCenter + vY, color);
    DrawLine3D(vCenter - vZ, vCenter + vZ, color);

    DrawLine3D(vCenter + vY, vCenter + vXY, color);
    DrawLine3D(vCenter + vXY, vCenter + vX, color);
    DrawLine3D(vCenter + vX, vCenter + vXnY, color);
    DrawLine3D(vCenter + vXnY, vCenter - vY, color);
    DrawLine3D(vCenter - vY, vCenter - vXY, color);
    DrawLine3D(vCenter - vXY, vCenter - vX, color);
    DrawLine3D(vCenter - vX, vCenter - vXnY, color);
    DrawLine3D(vCenter - vXnY, vCenter + vY, color);

    DrawLine3D(vCenter + vZ, vCenter + vYZ, color);
    DrawLine3D(vCenter + vYZ, vCenter + vY, color);
    DrawLine3D(vCenter + vY, vCenter + vYnZ, color);
    DrawLine3D(vCenter + vYnZ, vCenter - vZ, color);
    DrawLine3D(vCenter - vZ, vCenter - vYZ, color);
    DrawLine3D(vCenter - vYZ, vCenter - vY, color);
    DrawLine3D(vCenter - vY, vCenter - vYnZ, color);
    DrawLine3D(vCenter - vYnZ, vCenter + vZ, color);

    DrawLine3D(vCenter + vX, vCenter + vXZ, color);
    DrawLine3D(vCenter + vXZ, vCenter + vZ, color);
    DrawLine3D(vCenter + vZ, vCenter + vnXZ, color);
    DrawLine3D(vCenter + vnXZ, vCenter - vX, color);
    DrawLine3D(vCenter - vX, vCenter - vXZ, color);
    DrawLine3D(vCenter - vXZ, vCenter - vZ, color);
    DrawLine3D(vCenter - vZ, vCenter - vnXZ, color);
    DrawLine3D(vCenter - vnXZ, vCenter + vX, color);
}

// FUNCTION: SWBF2 0x006f2510
void RedDebug::RedDebugRenderable::Init()
{
    _n3DLines = 0;
    _nSpheres = 0;
    _nMarkers3D = 0;
    _nRadii = 0;

    _pCurrLinePos = _aLineBuffer;
    *_pCurrLinePos = 0;
    _uiNumText3D = 0;
    _uiNumTextLines = 0;

    FLRenderer::AddInterfaceElement(&_ScreenGroup, RedInterfaceScreen::LEFT,
                                    RedInterfaceScreen::TOP, false);

    for (unsigned int i = 0; i < 30; i++)
    {
        RedTextElement& line = _aTextLines[i];
        line.SetFont("systemfont");
        line.SetIntercharacterSpacing(0);
        line.RedTextElement::Enable(false);
        line.BringToFront();
        _ScreenGroup.AddElement(&line);
    }

    for (unsigned int i = 0; i < 64; i++)
    {
        RedTextElement& text = _aText3D[i].text;
        text.SetFont("systemfont");
        text.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
        text.RedTextElement::Enable(false);
        text.SendToBack();
        _ScreenGroup.AddElement(&text);
    }

    SetPrintfData(RedColor::WHITE, RedGlyphCacheElement::TEXT_STYLE_NORMAL);

    s_debugShader = pcRedShader::Create(PblHash("lightbeam"), 0x1082, 0, 0, 0x200,
                                        RedColor::WHITE, RedColor::BLACK, 32.0f);
}

struct DebugVertex
{
    CompressedPos position;
    RedColor color;
};
 
 

// FUNCTION: SWBF2 0x006f2dd0
void RedDebug::_DrawRadius(const Radius& radius)
{
    const PblVector3 vMin(-512.0f, -512.0f, -512.0f);
    const PblVector3 vMax(512.0f, 512.0f, 512.0f);
    const PblVector3 vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    PblMatrix mWorld;
    mWorld.Reset();
    mWorld.SetTranslation(vCamera);
    const float fX = radius._vPos.x - vCamera.x;
    const float fY = radius._vPos.y - vCamera.y;
    const float fZ = radius._vPos.z - vCamera.z;
    const float fHalfHeight = radius._fHeight * 0.5f;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x1082, 12, 66, 0, false);
    const PblBox bounds(vMin, vMax);
    pPrimitive->m_bbox = bounds;
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = PblVector3{ 0.015625238f, 0.015625238f, 0.015625238f };
    CompressedPos::s_vScaleInv = PblVector3{ 63.999023f, 63.999023f, 63.999023f };

    DebugVertex* pVertex = (DebugVertex*)pPrimitive->BeginVertexData();
    if (!pVertex)
    {
        return;
    }
    const float fBottom = -fHalfHeight + fY;
    const float fTop = fY + fHalfHeight;
    for (unsigned int i = 0; i < 33; ++i)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * 0.19634955f);
        const float x = fX + radius._fRadius * fSin;
        const float z = fZ + radius._fRadius * fCos;
        pVertex->position.SetPos(x, fBottom, z);
        pVertex->color = radius._color;
        ++pVertex;
        pVertex->position.SetPos(x, fTop, z);
        pVertex->color = radius._color;
        ++pVertex;
    }
    pPrimitive->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   s_debugShader,
                                   mWorld,
                                   RedColor::WHITE,
                                   0, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x006f3170
void RedDebug::_DrawSolidOBB(const SolidOBB& obb)
{
    const PblVector3 vMin(-512.0f, -512.0f, -512.0f);
    const PblVector3 vMax(512.0f, 512.0f, 512.0f);
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x1082, 12, 10, 0, false);
    const PblBox bounds(vMin, vMax);
    pPrimitive->m_bbox = bounds;
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = PblVector3{ 0.015625238f, 0.015625238f, 0.015625238f };
    CompressedPos::s_vScaleInv = PblVector3{ 63.999023f, 63.999023f, 63.999023f };
    DebugVertex* pVertex = (DebugVertex*)pPrimitive->BeginVertexData();
    if (pVertex)
    {
        const int order[10] = { 0, 2, 1, 3, 5, 7, 4, 6, 0, 2 };
        for (int i = 0; i < 10; ++i)
        {
            const int corner = order[i];
            const float z = (corner & 1) ? obb._vBox._vMax.z : obb._vBox._vMin.z;
            const float y = (corner & 2) ? obb._vBox._vMax.y : obb._vBox._vMin.y;
            const float x = (corner & 4) ? obb._vBox._vMax.x : obb._vBox._vMin.x;
            short* p = (short*)&pVertex->position;
            p[0] = (short)((x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
            p[1] = (short)((y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
            p[2] = (short)((z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
            pVertex->color = obb._color;
            ++pVertex;
        }
        pPrimitive->EndVertexData();
        RedRenderer::pcRenderPrimitive(pPrimitive,
                                       s_debugShader,
                                       obb._mBox,
                                       RedColor::WHITE,
                                       0, 0, 0, 0);
        pPrimitive->Release();
    }
}

// FUNCTION: SWBF2 0x006f2710
void RedDebug::RedDebugRenderable::Render(int)
{
    if (_n3DLines >= 2)
    {
        PblBox box;
        box._vMin = _a3DLineVectors[0];
        box._vMax = _a3DLineVectors[0];
        for (unsigned int i = 1; i < _n3DLines; i++)
        {
            box.AddPoint(_a3DLineVectors[i]);
        }

        unsigned int uiNumLines = _n3DLines / 2;
        const PblVector3 vOne(1.0f, 1.0f, 1.0f);
        const PblBox bounds(box._vMin - vOne, box._vMax + vOne);

        pcRedPrimitive* pPrimitive =
            pcRedPrimitive::Create(D3DPT_LINELIST, 0x1082, 12, uiNumLines * 2, 0, false);
        pPrimitive->m_bbox = bounds;
        CompressedPos::SetBoundingBox(bounds);

        DebugVertex* pVertex = (DebugVertex*)pPrimitive->BeginVertexData();
        if (!pVertex)
        {
            return;
        }

        for (unsigned int i = 0; i < uiNumLines; i++)
        {
            pVertex->position.SetPos(_a3DLineVectors[i * 2]);
            pVertex->color = _aColor[i];
            ++pVertex;
            pVertex->position.SetPos(_a3DLineVectors[i * 2 + 1]);
            pVertex->color = _aColor[i];
            ++pVertex;
        }

        pPrimitive->EndVertexData();
        RedRenderer::pcRenderPrimitive(pPrimitive, s_debugShader, gMatrixIdentity,
                                       RedColor::WHITE, 0x200, 0, 0, 0);
        pPrimitive->Release();
    }

    _n3DLines = 0;

    for (unsigned int i = _nSpheres; i < 16; i++)
    {
        FLRenderer::RemoveRenderable(&_aSpheres[i]);
    }
    for (unsigned int i = _nMarkers3D; i < 16; i++)
    {
        FLRenderer::RemoveRenderable(&_aMarkers3D[i]);
    }

    for (unsigned int i = 0; i < _nRadii; i++)
    {
        _DrawRadius(_aRadii[i]);
    }
    for (unsigned int i = 0; i < _nOBB; i++)
    {
        _DrawSolidOBB(_aOBB[i]);
    }

    _DrawPrintf();
    _DrawPrintf3D();

    _n3DLines = 0;
    _nSpheres = 0;
    _nMarkers3D = 0;
    _nRadii = 0;
    _nOBB = 0;
    _pCurrLinePos = _aLineBuffer;
    _uiNumText3D = 0;
    _uiNumTextLines = 0;
}

// FUNCTION: SWBF2 0x006f2bb0
void RedSphereShape::Render(int)
{
}

// FUNCTION: SWBF2 0x006f2bc0
void RedTetrahedronShape::Render(int)
{
}

// FUNCTION: SWBF2 0x006f2670
void RedDebug::Printf3D(const PblVector3& vPos, const char* pcFormat, ...)
{
    if (_uiNumText3D == 64)
    {
        PblTraceP("RedDebug::Printf3D !WARNING! Array of 3D positions is full\n");
        return;
    }

    _aText3D[_uiNumText3D].text.SetColor(_PrintfColor);
    _aText3D[_uiNumText3D].text.m_eStyle = _ePrintfStyle;

    va_list args;
    va_start(args, pcFormat);
    _aText3D[_uiNumText3D].text.SetTextFormattedV(pcFormat, args);
    va_end(args);

    _aText3D[_uiNumText3D].pos = vPos;
    _uiNumText3D++;
}

// FUNCTION: SWBF2 0x006f2bd0
void RedDebug::SetPrintfData(const RedColor& color, RedGlyphCacheElement::TextStyleT eStyle)
{
    _PrintfColor = color;
    _ePrintfStyle = eStyle;
}

// FUNCTION: SWBF2 0x006f2bf0
void RedDebug::_DrawPrintf3D()
{
    unsigned int i;
    for (i = 0; i < _uiNumText3D; i++)
    {
        const PblVector3 vScreen = RedRenderer::TransformProjectionPointToScreenSpace(
            RedRenderer::TransformWorldPointToProjectionSpace(_aText3D[i].pos));
        _aText3D[i].text.SetPosition(vScreen.x, vScreen.y - 25.0f);
        _aText3D[i].text.Enable(true);
    }
    for (; i < 64; i++)
    {
        _aText3D[i].text.Enable(false);
    }
}

// FUNCTION: SWBF2 0x006f2cb0
void RedDebug::_DrawPrintf()
{
    if (_pCurrLinePos > _aLineBuffer)
    {
        _aTextLines[_uiNumTextLines % 30].SetText(_aLineBuffer, true);
        _uiNumTextLines++;
        _pCurrLinePos = _aLineBuffer;
        *_aLineBuffer = 0;
    }

    unsigned int uiFirst = 0;
    if (_uiNumTextLines > 30)
    {
        uiFirst = _uiNumTextLines - 29;
    }

    unsigned int i;
    for (i = 0; i < _uiNumTextLines - uiFirst; i++)
    {
        _aTextLines[(i + uiFirst) % 30].SetPosition(10.0f, (float)i * 15.0f + 10.0f);
        _aTextLines[(i + uiFirst) % 30].Enable(true);
    }
    for (; i < 30; i++)
    {
        _aTextLines[(i + uiFirst) % 30].Enable(false);
    }
}

// SYNTHETIC: SWBF2 0x004090a0
// PblMath::`dynamic initializer for 'fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x004090c0
// PblMath::`dynamic initializer for 'fOneOverTwoPi''
