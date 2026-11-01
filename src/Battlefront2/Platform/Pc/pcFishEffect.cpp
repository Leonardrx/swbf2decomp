#include <d3dx9math.h>

#include "Battlefront2/Source/FishEffect.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// GLOBAL: SWBF2 0x007b2650
// __xmm@000000003eaa7efa0000000000000000
// GLOBAL: SWBF2 0x007b2660
// __xmm@000000003f800000000000003f2a7efa
// GLOBAL: SWBF2 0x007b27d0
// __xmm@3f0000003eaa7efa3f00000000000000
// GLOBAL: SWBF2 0x007b2800
// __xmm@3f0000003f8000003f0000003f2a7efa
// GLOBAL: SWBF2 0x007b2830
// __xmm@3f8000003eaa7efa3f80000000000000
// GLOBAL: SWBF2 0x007b2880
// __xmm@3f8000003f8000003f8000003f2a7efa

// FUNCTION: SWBF2 0x00411f90
void FishParticle::Render(unsigned char, float, unsigned int)
{
    float fSize = mType.mSize * 0.5f;
    mTimeOffCamera = 0.0f;

    PblMatrix mInverse = mMatrix.GetInverseFast();
    PblVector3 vDir;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)mBaseMatrix.m[2],
                            (const D3DXMATRIX*)&mInverse);

    const PblVector3 vRight(1.0f, 0.0f, 0.0f);
    const PblVector3 vUp(0.0f, 1.0f, 0.0f);
    const PblVector3 vAt(0.0f, 0.0f, 1.0f);
    float fHalf = fSize * 0.5f;
    float fSegment = fSize * 0.666f;
    float fFin = fSize * 0.1f;

    PblVector3 vertices[12];
    PblVector3& a = vertices[0];
    a.x = vDir.x * fSize + vUp.x * fHalf;
    a.y = vDir.y * fSize + vUp.y * fHalf;
    a.z = vDir.z * fSize + vUp.z * fHalf;
    for (int i = 1; i < 3; i++)
    {
        vertices[i].x = vertices[i - 1].x - vDir.x * fSegment;
        vertices[i].y = vertices[i - 1].y - vDir.y * fSegment;
        vertices[i].z = vertices[i - 1].z - vDir.z * fSegment;
    }
    vertices[3].x = vUp.x * fHalf - vAt.x * fSize;
    vertices[3].y = vUp.y * fHalf - vAt.y * fSize;
    vertices[3].z = vUp.z * fHalf - vAt.z * fSize;

    for (int i = 0; i < 4; i++)
    {
        vertices[i + 4] = vertices[i];
        vertices[i + 4].y = 0.0f;
    }
    for (int i = 5; i < 7; i++)
    {
        vertices[i].x += vRight.x * fFin;
        vertices[i].y += vRight.y * fFin;
        vertices[i].z += vRight.z * fFin;
    }

    for (int i = 0; i < 4; i++)
    {
        vertices[i + 8].x = vertices[i].x - vUp.x * fSize;
        vertices[i + 8].y = vertices[i].y - vUp.y * fSize;
        vertices[i + 8].z = vertices[i].z - vUp.z * fSize;
    }

    TexCoord texCoords[12] = {
        { 0.0f, 0.0f }, { 0.333f, 0.0f }, { 0.666f, 0.0f }, { 1.0f, 0.0f },
        { 0.0f, 0.5f }, { 0.333f, 0.5f }, { 0.666f, 0.5f }, { 1.0f, 0.5f },
        { 0.0f, 1.0f }, { 0.333f, 1.0f }, { 0.666f, 1.0f }, { 1.0f, 1.0f },
    };

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode())
    {
        pPrimitive = CreateCompressedGeometry(vertices, texCoords);
    }
    else
    {
        pPrimitive = CreateGeometry(vertices, texCoords);
    }

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   mType.m_pShader,
                                   mMatrix,
                                   RedColor::WHITE,
                                   0x01000010, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00411eb0
void FishParticleManager::PlatformInit()
{
    unsigned int uiFormat =
        RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED ? 0x9282 : 0x282;
    for (int i = 0; i < sNumFishTypes; i++)
    {
        sFishTypes[i].m_pShader = pcRedShader::Create(kuiNormalShader, uiFormat, &sFishTypes[i].mTexture, 1,
                                                 0x50002, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
}

// FUNCTION: SWBF2 0x00411f20
void FishParticleManager::PlatformCleanup()
{
    for (int i = 0; i < sNumFishTypes; i++)
    {
        if (sFishTypes[i].m_pShader)
        {
            sFishTypes[i].m_pShader->Release();
            sFishTypes[i].m_pShader = 0;
        }
    }
    sNumFishTypes = 0;
}

// GLOBAL: SWBF2 0x0078c5f4
const RedColor kFishColor = { 0xcc, 0xcc, 0xcc, 0xff };

struct FishVertex
{
    PblVector3 pos;
    RedColor color;
    TexCoord uv;
};

struct FishCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};

// FUNCTION: SWBF2 0x00412440
pcRedPrimitive* FishParticle::CreateCompressedGeometry(PblVector3* pVertices, TexCoord* pTexCoords)
{
    float fishBodySize = mType.mSize * 0.5f;
    float fExtent = mType.mSize * 2.0f * 0.5f;

    PblBox boundingBox(-fExtent, -fExtent, -fExtent, fExtent, fExtent, fExtent);
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282,
                                                        sizeof(FishCompressedVertex), 40, 0, false);
    pPrimitive->m_bbox = boundingBox;
    CompressedPos::SetBoundingBox(boundingBox);

    FishCompressedVertex* pVerts = (FishCompressedVertex*)pPrimitive->BeginVertexData();
    if (pVerts)
    {
        float fBack = fishBodySize * 0.2f;
        pVerts[0].pos.SetPos(pVertices[0]);
        pVerts[0].uv.SetUV(pTexCoords[0].u, pTexCoords[0].v);
        pVerts[0].color = kFishColor;
        pVerts[1].pos.SetPos(pVertices[4]);
        pVerts[1].uv.SetUV(pTexCoords[4].u, pTexCoords[4].v);
        pVerts[1].color = kFishColor;
        pVerts[2].pos.SetPos(pVertices[1]);
        pVerts[2].uv.SetUV(pTexCoords[1].u, pTexCoords[1].v);
        pVerts[2].color = kFishColor;
        pVerts[3].pos.SetPos(pVertices[5]);
        pVerts[3].uv.SetUV(pTexCoords[5].u, pTexCoords[5].v);
        pVerts[3].color = kFishColor;
        pVerts[4].pos.SetPos(pVertices[2]);
        pVerts[4].uv.SetUV(pTexCoords[2].u, pTexCoords[2].v);
        pVerts[4].color = kFishColor;
        pVerts[5].pos.SetPos(pVertices[6]);
        pVerts[5].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[5].color = kFishColor;
        pVerts[6].pos.SetPos(pVertices[6]);
        pVerts[6].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[6].color = kFishColor;
        pVerts[7].pos.SetPos(pVertices[10]);
        pVerts[7].uv.SetUV(pTexCoords[10].u, pTexCoords[10].v);
        pVerts[7].color = kFishColor;
        pVerts[8].pos.SetPos(pVertices[5]);
        pVerts[8].uv.SetUV(pTexCoords[5].u, pTexCoords[5].v);
        pVerts[8].color = kFishColor;
        pVerts[9].pos.SetPos(pVertices[9]);
        pVerts[9].uv.SetUV(pTexCoords[9].u, pTexCoords[9].v);
        pVerts[9].color = kFishColor;
        pVerts[10].pos.SetPos(pVertices[4]);
        pVerts[10].uv.SetUV(pTexCoords[4].u, pTexCoords[4].v);
        pVerts[10].color = kFishColor;
        pVerts[11].pos.SetPos(pVertices[8]);
        pVerts[11].uv.SetUV(pTexCoords[8].u, pTexCoords[8].v);
        pVerts[11].color = kFishColor;
        pVerts[12].pos.SetPos(pVertices[4]);
        pVerts[12].uv.SetUV(pTexCoords[4].u, pTexCoords[4].v);
        pVerts[12].color = kFishColor;
        pVerts[13].pos.SetPos(pVertices[9]);
        pVerts[13].uv.SetUV(pTexCoords[9].u, pTexCoords[9].v);
        pVerts[13].color = kFishColor;
        pVerts[14].pos.SetPos(PblVector3(pVertices[5].x - fBack, pVertices[5].y, pVertices[5].z));
        pVerts[14].uv.SetUV(pTexCoords[5].u, pTexCoords[5].v);
        pVerts[14].color = kFishColor;
        pVerts[15].pos.SetPos(pVertices[10]);
        pVerts[15].uv.SetUV(pTexCoords[10].u, pTexCoords[10].v);
        pVerts[15].color = kFishColor;
        pVerts[16].pos.SetPos(PblVector3(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z));
        pVerts[16].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[16].color = kFishColor;
        pVerts[17].pos.SetPos(PblVector3(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z));
        pVerts[17].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[17].color = kFishColor;
        pVerts[18].pos.SetPos(pVertices[2]);
        pVerts[18].uv.SetUV(pTexCoords[2].u, pTexCoords[2].v);
        pVerts[18].color = kFishColor;
        pVerts[19].pos.SetPos(PblVector3(pVertices[5].x - fBack, pVertices[5].y, pVertices[5].z));
        pVerts[19].uv.SetUV(pTexCoords[5].u, pTexCoords[5].v);
        pVerts[19].color = kFishColor;
        pVerts[20].pos.SetPos(pVertices[1]);
        pVerts[20].uv.SetUV(pTexCoords[1].u, pTexCoords[1].v);
        pVerts[20].color = kFishColor;
        pVerts[21].pos.SetPos(pVertices[4]);
        pVerts[21].uv.SetUV(pTexCoords[4].u, pTexCoords[4].v);
        pVerts[21].color = kFishColor;
        pVerts[22].pos.SetPos(pVertices[0]);
        pVerts[22].uv.SetUV(pTexCoords[0].u, pTexCoords[0].v);
        pVerts[22].color = kFishColor;
        pVerts[23] = pVerts[22];
        pVerts[24].pos.SetPos(pVertices[11]);
        pVerts[24].uv.SetUV(pTexCoords[11].u, pTexCoords[11].v);
        pVerts[24].color = kFishColor;
        pVerts[25] = pVerts[24];
        pVerts[26].pos.SetPos(pVertices[7]);
        pVerts[26].uv.SetUV(pTexCoords[7].u, pTexCoords[7].v);
        pVerts[26].color = kFishColor;
        pVerts[27].pos.SetPos(pVertices[10]);
        pVerts[27].uv.SetUV(pTexCoords[10].u, pTexCoords[10].v);
        pVerts[27].color = kFishColor;
        pVerts[28].pos.SetPos(pVertices[6]);
        pVerts[28].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[28].color = kFishColor;
        pVerts[29].pos.SetPos(pVertices[6]);
        pVerts[29].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[29].color = kFishColor;
        pVerts[30].pos.SetPos(pVertices[2]);
        pVerts[30].uv.SetUV(pTexCoords[2].u, pTexCoords[2].v);
        pVerts[30].color = kFishColor;
        pVerts[31].pos.SetPos(pVertices[7]);
        pVerts[31].uv.SetUV(pTexCoords[7].u, pTexCoords[7].v);
        pVerts[31].color = kFishColor;
        pVerts[32].pos.SetPos(pVertices[3]);
        pVerts[32].uv.SetUV(pTexCoords[3].u, pTexCoords[3].v);
        pVerts[32].color = kFishColor;
        pVerts[33].pos.SetPos(pVertices[7]);
        pVerts[33].uv.SetUV(pTexCoords[7].u, pTexCoords[7].v);
        pVerts[33].color = kFishColor;
        pVerts[34].pos.SetPos(pVertices[2]);
        pVerts[34].uv.SetUV(pTexCoords[2].u, pTexCoords[2].v);
        pVerts[34].color = kFishColor;
        pVerts[35].pos.SetPos(PblVector3(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z));
        pVerts[35].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[35].color = kFishColor;
        pVerts[36].pos.SetPos(PblVector3(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z));
        pVerts[36].uv.SetUV(pTexCoords[6].u, pTexCoords[6].v);
        pVerts[36].color = kFishColor;
        pVerts[37].pos.SetPos(pVertices[10]);
        pVerts[37].uv.SetUV(pTexCoords[10].u, pTexCoords[10].v);
        pVerts[37].color = kFishColor;
        pVerts[38].pos.SetPos(pVertices[7]);
        pVerts[38].uv.SetUV(pTexCoords[7].u, pTexCoords[7].v);
        pVerts[38].color = kFishColor;
        pVerts[39].pos.SetPos(pVertices[11]);
        pVerts[39].uv.SetUV(pTexCoords[11].u, pTexCoords[11].v);
        pVerts[39].color = kFishColor;
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x00413d00
pcRedPrimitive* FishParticle::CreateGeometry(PblVector3* pVertices, TexCoord* pTexCoords)
{
    float fishBodySize = mType.mSize * 0.5f;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282,
                                                        sizeof(FishVertex), 40, 0, false);
    struct FishVertex
    {
        PosXYZ pos;
        RedColor color;
        TexCoord uv;
    };
    FishVertex* pVerts = (FishVertex*)pPrimitive->BeginVertexData();
    if (pVerts)
    {
        pVerts[0].pos.SetPos(pVertices[0].x, pVertices[0].y, pVertices[0].z);
        pVerts[0].uv = pTexCoords[0];
        pVerts[0].color = kFishColor;
        pVerts[1].pos.SetPos(pVertices[4].x, pVertices[4].y, pVertices[4].z);
        pVerts[1].uv = pTexCoords[4];
        pVerts[1].color = kFishColor;
        pVerts[2].pos.SetPos(pVertices[1].x, pVertices[1].y, pVertices[1].z);
        pVerts[2].uv = pTexCoords[1];
        pVerts[2].color = kFishColor;
        pVerts[3].pos.SetPos(pVertices[5].x, pVertices[5].y, pVertices[5].z);
        pVerts[3].uv = pTexCoords[5];
        pVerts[3].color = kFishColor;
        pVerts[4].pos.SetPos(pVertices[2].x, pVertices[2].y, pVertices[2].z);
        pVerts[4].uv = pTexCoords[2];
        pVerts[4].color = kFishColor;
        pVerts[5].pos.SetPos(pVertices[6].x, pVertices[6].y, pVertices[6].z);
        pVerts[5].uv = pTexCoords[6];
        pVerts[5].color = kFishColor;
        pVerts[6].pos.SetPos(pVertices[6].x, pVertices[6].y, pVertices[6].z);
        pVerts[6].uv = pTexCoords[6];
        pVerts[6].color = kFishColor;
        pVerts[7].pos.SetPos(pVertices[10].x, pVertices[10].y, pVertices[10].z);
        pVerts[7].uv = pTexCoords[10];
        pVerts[7].color = kFishColor;
        pVerts[8].pos.SetPos(pVertices[5].x, pVertices[5].y, pVertices[5].z);
        pVerts[8].uv = pTexCoords[5];
        pVerts[8].color = kFishColor;
        pVerts[9].pos.SetPos(pVertices[9].x, pVertices[9].y, pVertices[9].z);
        pVerts[9].uv = pTexCoords[9];
        pVerts[9].color = kFishColor;
        pVerts[10].pos.SetPos(pVertices[4].x, pVertices[4].y, pVertices[4].z);
        pVerts[10].uv = pTexCoords[4];
        pVerts[10].color = kFishColor;
        pVerts[11].pos.SetPos(pVertices[8].x, pVertices[8].y, pVertices[8].z);
        pVerts[11].uv = pTexCoords[8];
        pVerts[11].color = kFishColor;
        pVerts[12].pos.SetPos(pVertices[4].x, pVertices[4].y, pVertices[4].z);
        pVerts[12].uv = pTexCoords[4];
        pVerts[12].color = kFishColor;
        pVerts[13].pos.SetPos(pVertices[9].x, pVertices[9].y, pVertices[9].z);
        pVerts[13].uv = pTexCoords[9];
        pVerts[13].color = kFishColor;
        float fBack = fishBodySize * 0.2f;
        pVerts[14].pos.SetPos(pVertices[5].x - fBack, pVertices[5].y, pVertices[5].z);
        pVerts[14].uv = pTexCoords[5];
        pVerts[14].color = kFishColor;
        pVerts[15].pos.SetPos(pVertices[10].x, pVertices[10].y, pVertices[10].z);
        pVerts[15].uv = pTexCoords[10];
        pVerts[15].color = kFishColor;
        pVerts[16].pos.SetPos(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z);
        pVerts[16].uv = pTexCoords[6];
        pVerts[16].color = kFishColor;
        pVerts[17].pos.SetPos(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z);
        pVerts[17].uv = pTexCoords[6];
        pVerts[17].color = kFishColor;
        pVerts[18].pos.SetPos(pVertices[2].x, pVertices[2].y, pVertices[2].z);
        pVerts[18].uv = pTexCoords[2];
        pVerts[18].color = kFishColor;
        pVerts[19].pos.SetPos(pVertices[5].x - fBack, pVertices[5].y, pVertices[5].z);
        pVerts[19].uv = pTexCoords[5];
        pVerts[19].color = kFishColor;
        pVerts[20].pos.SetPos(pVertices[1].x, pVertices[1].y, pVertices[1].z);
        pVerts[20].uv = pTexCoords[1];
        pVerts[20].color = kFishColor;
        pVerts[21].pos.SetPos(pVertices[4].x, pVertices[4].y, pVertices[4].z);
        pVerts[21].uv = pTexCoords[4];
        pVerts[21].color = kFishColor;
        pVerts[22].pos.SetPos(pVertices[0].x, pVertices[0].y, pVertices[0].z);
        pVerts[22].uv = pTexCoords[0];
        pVerts[22].color = kFishColor;
        pVerts[23] = pVerts[22];
        pVerts[24].pos.SetPos(pVertices[11].x, pVertices[11].y, pVertices[11].z);
        pVerts[24].uv = pTexCoords[11];
        pVerts[24].color = kFishColor;
        pVerts[25] = pVerts[24];
        pVerts[26].pos.SetPos(pVertices[7].x, pVertices[7].y, pVertices[7].z);
        pVerts[26].uv = pTexCoords[7];
        pVerts[26].color = kFishColor;
        pVerts[27].pos.SetPos(pVertices[10].x, pVertices[10].y, pVertices[10].z);
        pVerts[27].uv = pTexCoords[10];
        pVerts[27].color = kFishColor;
        pVerts[28].pos.SetPos(pVertices[6].x, pVertices[6].y, pVertices[6].z);
        pVerts[28].uv = pTexCoords[6];
        pVerts[28].color = kFishColor;
        pVerts[29].pos.SetPos(pVertices[6].x, pVertices[6].y, pVertices[6].z);
        pVerts[29].uv = pTexCoords[6];
        pVerts[29].color = kFishColor;
        pVerts[30].pos.SetPos(pVertices[2].x, pVertices[2].y, pVertices[2].z);
        pVerts[30].uv = pTexCoords[2];
        pVerts[30].color = kFishColor;
        pVerts[31].pos.SetPos(pVertices[7].x, pVertices[7].y, pVertices[7].z);
        pVerts[31].uv = pTexCoords[7];
        pVerts[31].color = kFishColor;
        pVerts[32].pos.SetPos(pVertices[3].x, pVertices[3].y, pVertices[3].z);
        pVerts[32].uv = pTexCoords[3];
        pVerts[32].color = kFishColor;
        pVerts[33].pos.SetPos(pVertices[7].x, pVertices[7].y, pVertices[7].z);
        pVerts[33].uv = pTexCoords[7];
        pVerts[33].color = kFishColor;
        pVerts[34].pos.SetPos(pVertices[2].x, pVertices[2].y, pVertices[2].z);
        pVerts[34].uv = pTexCoords[2];
        pVerts[34].color = kFishColor;
        pVerts[35].pos.SetPos(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z);
        pVerts[35].uv = pTexCoords[6];
        pVerts[35].color = kFishColor;
        pVerts[36].pos.SetPos(pVertices[6].x - fBack, pVertices[6].y, pVertices[6].z);
        pVerts[36].uv = pTexCoords[6];
        pVerts[36].color = kFishColor;
        pVerts[37].pos.SetPos(pVertices[10].x, pVertices[10].y, pVertices[10].z);
        pVerts[37].uv = pTexCoords[10];
        pVerts[37].color = kFishColor;
        pVerts[38].pos.SetPos(pVertices[7].x, pVertices[7].y, pVertices[7].z);
        pVerts[38].uv = pTexCoords[7];
        pVerts[38].color = kFishColor;
        pVerts[39].pos.SetPos(pVertices[11].x, pVertices[11].y, pVertices[11].z);
        pVerts[39].uv = pTexCoords[11];
        pVerts[39].color = kFishColor;
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}
