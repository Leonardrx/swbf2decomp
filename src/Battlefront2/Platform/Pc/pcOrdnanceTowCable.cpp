#include <float.h>
#include <math.h>

#include "Battlefront2/Source/OrdnanceTowCable.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblHermite.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"
#include "PebbleFL/Common/PblMath.h"

// GLOBAL: SWBF2 0x01e2fd34
pcRedShader* OrdnanceTowCable::sShader;

// FUNCTION: SWBF2 0x00424ce0
void OrdnanceTowCable::PlatformInit()
{
    sTextureHash = 0xde020766; // "white"
    unsigned int uiFormat =
        RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED ? 0x9282 : 0x282;
    sShader = pcRedShader::Create(0x8a8743bf /* "Particle" */, uiFormat,
                                  &sTextureHash, 1, 0x10004, RedColor::WHITE, RedColor::BLACK,
                                  32.0f);
}

// GLOBAL: SWBF2 0x007eb744
static RedColor kCableColor = { 0, 0, 0, 255 };



struct CableCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
struct CableVertex
{
    PblVector3 pos;
    RedColor color;
    TexCoord uv;
};

// FUNCTION: SWBF2 0x00424d70
pcRedPrimitive* sCreateCableCompressedGeometry(const PblBox& box, const PblVector3* pPoints,
                                               const PblVector3* pSlopes, float fThickness,
                                               const TexCoord* pUV, float fAlpha, int iNumPoints)
{
    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, sizeof(CableCompressedVertex), iNumPoints * 2, 0, false);
    pPrimitive->m_bbox = box;

    CompressedPos::SetBoundingBox(box);

    CableCompressedVertex* pVert = (CableCompressedVertex*)pPrimitive->BeginVertexData();
    if (pVert)
    {
        const RedCamera* pCamera = RedRenderer::GetCamera();
        const PblVector3 camDir = -pCamera->GetMatrix().GetZAxis();
        for (int i = 0; i < iNumPoints; i++)
        {
            PblVector3 dir = CrossProduct(camDir, pSlopes[i]);
            const float dirLength = Length(dir);
            if (dirLength >= 0.0001f)
            {
                dir /= dirLength;
            }
            else
            {
                dir = pCamera->GetMatrix().GetYAxis();
            }
            dir *= fThickness;
            const PblVector3& vPoint = pPoints[i];
            const TexCoord* pRow = (i & 1) ? pUV + 2 : pUV;

            RedColor color(kCableColor, (unsigned char)(int)(fAlpha * 255.0f));

            pVert[0].pos.SetPos( vPoint.x + dir.x, vPoint.y + dir.y, vPoint.z + dir.z);
            pVert[0].uv.SetUV(pRow[0].u, pRow[0].v);
            pVert[0].color = color;

            pVert[1].pos.SetPos( vPoint.x - dir.x, vPoint.y - dir.y, vPoint.z - dir.z);
            pVert[1].uv.SetUV(pRow[1].u, pRow[1].v);
            pVert[1].color = color;
            pVert += 2;
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x004251b0
pcRedPrimitive* sCreateCableGeometry(const PblBox& box, const PblVector3* pPoints, const PblVector3* pSlopes,
                                     float fThickness, const TexCoord* pUV, float fAlpha, int iNumPoints)
{
    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, sizeof(CableVertex), iNumPoints * 2, 0, false);
    pPrimitive->m_bbox = box;

    CableVertex* pVert = (CableVertex*)pPrimitive->BeginVertexData();
    if (pVert)
    {
        const RedCamera* pCamera = RedRenderer::GetCamera();
        const PblVector3 camDir = -pCamera->GetMatrix().GetZAxis();
        for (int i = 0; i < iNumPoints; i++)
        {
            PblVector3 dir = CrossProduct(camDir, pSlopes[i]);
            const float dirLength = Length(dir);
            if (dirLength >= 0.0001f)
            {
                dir /= dirLength;
            }
            else
            {
                dir = pCamera->GetMatrix().GetYAxis();
            }
            dir *= fThickness;
            const PblVector3& vPoint = pPoints[i];
            const TexCoord* pRow = (i & 1) ? pUV + 2 : pUV;

            RedColor color(kCableColor, (unsigned char)(int)(fAlpha * 255.0f));

            pVert[0].pos = vPoint + dir;
            pVert[0].uv.SetUV(pRow[0].u, pRow[0].v);
            pVert[0].color = color;

            pVert[1].pos = vPoint - dir;
            pVert[1].uv.SetUV(pRow[1].u, pRow[1].v);
            pVert[1].color = color;
            pVert += 2;
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x004254a0
void sRenderHermiteSegment(PblHermiteVector3& segment, unsigned char ucViewport, float fDist,
                           unsigned int uiFlags)
{
    int iNumPoints = (int)(segment.GetLengthApproximation(10) * 0.5f);
    iNumPoints = Clamp(iNumPoints, 2, 25);

    PblVector3 vSlopes[25];
    PblVector3 vPoints[25];
    float fStep = 1.0f / ((float)iNumPoints - 1.0f);
    float fEnd = fStep * 0.5f + 1.0f;
    PblVector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
    PblVector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    int i = 0;
    for (float fT = 0.0f; fEnd > fT; fT += fStep)
    {
        vPoints[i] = segment.getValue(fT);
        vMin.x = Min(vMin.x, vPoints[i].x);
        vMin.y = Min(vMin.y, vPoints[i].y);
        vMin.z = Min(vMin.z, vPoints[i].z);
        vMax.x = Max(vMax.x, vPoints[i].x);
        vMax.y = Max(vMax.y, vPoints[i].y);
        vMax.z = Max(vMax.z, vPoints[i].z);
        vSlopes[i] = segment.getSlope(fT);
        i++;
    }

    const float fThickness = 0.05f;
    PblBox box;
    box._vMin = PblVector3(vMin.x - fThickness, vMin.y - fThickness, vMin.z - fThickness);
    box._vMax = PblVector3(vMax.x + fThickness, vMax.y + fThickness, vMax.z + fThickness);
    TexCoord uv[4] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } };

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        pPrimitive = sCreateCableCompressedGeometry(box, vPoints, vSlopes, fThickness, uv, fDist, iNumPoints);
    }
    else
    {
        pPrimitive = sCreateCableGeometry(box, vPoints, vSlopes, fThickness, uv, fDist, iNumPoints);
    }

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   OrdnanceTowCable::sShader,
                                   gMatrixIdentity, RedColor::WHITE,
                                   0x1000010, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00424d30
void OrdnanceTowCable::PlatformCleanup()
{
    if (sShader)
    {
        sShader->Release();
        sShader = 0;
    }
}
