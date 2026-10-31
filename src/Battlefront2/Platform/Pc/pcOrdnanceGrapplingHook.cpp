#include <d3dx9math.h>
#include <math.h>

#include "Battlefront2/Source/OrdnanceGrapplingHook.h"
#include "Battlefront2/Source/GameObject.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// GLOBAL: SWBF2 0x007b2810
// __xmm@3f800000000000000000000000000000
// GLOBAL: SWBF2 0x007b2860
// __xmm@3f8000003f800000000000003f800000

// GLOBAL: SWBF2 0x01e2fd30
pcRedShader* OrdnanceGrapplingHook::sShader;


// GLOBAL: SWBF2 0x007eb740
static RedColor kCableColor = { 0xcc, 0xcc, 0xcc, 0xff };



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

// FUNCTION: SWBF2 0x004243a0
pcRedPrimitive* sCreateGrapplingHookGeometry(const PblBox& box, const PblVector3& vStart,
                            const PblVector3& vEnd, int iSegments, const TexCoord* pTexCoords,
                            float fAlpha, const PblVector3& vDirection,
                            const PblVector3& vWaveDirection, const PblVector3& vOffset,
                            float fWaveHeight, float fWaveLength, bool bWavy, float fLength)
{
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, sizeof(CableVertex),
                                                         2 * iSegments + 2, 0, false);
    pPrimitive->m_bbox = box;
    CableVertex* pVertices = (CableVertex*)pPrimitive->BeginVertexData();
    if (pVertices)
    {
        const float fSegmentLength = fLength / (float)iSegments;
        const unsigned char ucAlpha = (unsigned char)(int)(fAlpha * 255.0f);
        for (int i = 0; i <= iSegments; ++i)
        {
            float fRatio = (float)i / (float)iSegments;
            float fDistance = (float)i * fSegmentLength;
            PblVector3 vCenter(vStart.x + vDirection.x * fDistance,
                               vStart.y + vDirection.y * fDistance,
                               vStart.z + vDirection.z * fDistance);
            if (i == iSegments)
            {
                vCenter = vEnd;
            }
            if (bWavy)
            {
                float fWave = PblMath::Sin((float)i * 6.2831855f * fSegmentLength)
                            * (fWaveHeight / fWaveLength)
                            * PblMath::Sin(fRatio * 3.1415927f);
                vCenter.x += vWaveDirection.x * fWave;
                vCenter.y += vWaveDirection.y * fWave;
                vCenter.z += vWaveDirection.z * fWave;
            }

            CableVertex& front = pVertices[i * 2];
            CableVertex& back = pVertices[i * 2 + 1];
            front.pos.x = vCenter.x + vOffset.x * 0.02f;
            front.pos.y = vCenter.y + vOffset.y * 0.02f;
            front.pos.z = vCenter.z + vOffset.z * 0.02f;
            back.pos.x = front.pos.x - vOffset.x * 0.04f;
            back.pos.y = front.pos.y - vOffset.y * 0.04f;
            back.pos.z = front.pos.z - vOffset.z * 0.04f;
            front.color = RedColor(kCableColor, ucAlpha);
            back.color = RedColor(kCableColor, ucAlpha);
            front.uv = pTexCoords[(i & 1) ? 2 : 0];
            back.uv = pTexCoords[(i & 1) ? 3 : 1];
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x00423f30
pcRedPrimitive* sCreateGrapplingHookCompressedGeometry(const PblBox& box, const PblVector3& vStart,
                           const PblVector3& vEnd, int iSegments, const TexCoord* pTexCoords,
                           float fAlpha, const PblVector3& vDirection,
                           const PblVector3& vWaveDirection, const PblVector3& vOffset,
                           float fWaveHeight, float fWaveLength, bool bWavy, float fLength)
{
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, sizeof(CableCompressedVertex),
                                                         2 * iSegments + 2, 0, false);
    pPrimitive->m_bbox = box;

    CompressedPos::SetBoundingBox(box);

    CableCompressedVertex* pVertices = (CableCompressedVertex*)pPrimitive->BeginVertexData();
    if (pVertices)
    {
        const float fSegmentLength = fLength / (float)iSegments;
        for (int i = 0; i <= iSegments; ++i)
        {
            float fRatio = (float)i / (float)iSegments;
            float fDistance = (float)i * fSegmentLength;
            PblVector3 vCenter(vStart.x + vDirection.x * fDistance,
                               vStart.y + vDirection.y * fDistance,
                               vStart.z + vDirection.z * fDistance);
            if (i == iSegments)
            {
                vCenter = vEnd;
            }
            if (bWavy)
            {
                float fWave = PblMath::Sin((float)i * 6.2831855f * fSegmentLength)
                            * (fWaveHeight / fWaveLength)
                            * PblMath::Sin(fRatio * 3.1415927f);
                vCenter.x += vWaveDirection.x * fWave;
                vCenter.y += vWaveDirection.y * fWave;
                vCenter.z += vWaveDirection.z * fWave;
            }

            const float fx = vCenter.x + vOffset.x * 0.02f;
            const float fy = vCenter.y + vOffset.y * 0.02f;
            const float fz = vCenter.z + vOffset.z * 0.02f;
            const TexCoord* pRow = (i & 1) ? pTexCoords + 2 : pTexCoords;

            CableCompressedVertex& front = pVertices[i * 2];
            CableCompressedVertex& back = pVertices[i * 2 + 1];
            front.pos.SetPos( fx, fy, fz);
            front.uv.SetUV(pRow[0].u, pRow[0].v);
            front.color = RedColor(kCableColor, (unsigned char)(int)(fAlpha * 255.0f));

            back.pos.SetPos( fx - vOffset.x * 0.04f, fy - vOffset.y * 0.04f,
                                  fz - vOffset.z * 0.04f);
            back.uv.SetUV(pRow[1].u, pRow[1].v);
            back.color = RedColor(kCableColor, (unsigned char)(int)(fAlpha * 255.0f));
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x00424690
void OrdnanceGrapplingHook::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!mOwner)
    {
        return;
    }

    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    PblVector3 vCameraRight(mCamera.m[0][0], mCamera.m[0][1], mCamera.m[0][2]);
    PblVector3 vStart = GetPosition();
    PblVector3 vEnd;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vEnd), reinterpret_cast<const D3DXVECTOR3*>(&mWeaponOffset), reinterpret_cast<const D3DXMATRIX*>(&mOwner->GetMatrix()));
    if (mState == HOOK_LIFTING)
    {
        vEnd.x += mVelocity.x * mDeltaTime;
        vEnd.y += mVelocity.y * mDeltaTime;
        vEnd.z += mVelocity.z * mDeltaTime;
    }

    PblVector3 vDirection(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    const PblVector3 vUp(0.0f, 1.0f, 0.0f);
    PblVector3 vSide(vDirection.y * vUp.z - vDirection.z * vUp.y,
                     vDirection.z * vUp.x - vDirection.x * vUp.z,
                     vDirection.x * vUp.y - vDirection.y * vUp.x);
    PblVector3 vWave(vSide.y * vDirection.z - vSide.z * vDirection.y,
                     vSide.z * vDirection.x - vSide.x * vDirection.z,
                     vSide.x * vDirection.y - vSide.y * vDirection.x);
    PblVector3 vWaveDirection;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vWaveDirection), reinterpret_cast<const D3DXVECTOR3*>(&vWave));

    float fLength = sqrtf(vDirection.x * vDirection.x + vDirection.y * vDirection.y
                          + vDirection.z * vDirection.z);
    if (fLength < 0.0001f)
    {
        return;
    }
    float fInvLength = 1.0f / fLength;
    vDirection.x *= fInvLength;
    vDirection.y *= fInvLength;
    vDirection.z *= fInvLength;

    int iSegments = (int)(fLength * 8.0f);
    if (iSegments < 2)
    {
        iSegments = 2;
    }
    else if (iSegments > 80)
    {
        iSegments = 80;
    }
    if (mMaxLifetime < 0.0001f)
    {
        return;
    }

    PblBox box;
    box._vMin = PblVector3((vEnd.x > vStart.x ? vStart.x : vEnd.x) - 1.0f,
                           (vEnd.y > vStart.y ? vStart.y : vEnd.y) - 1.0f,
                           (vEnd.z > vStart.z ? vStart.z : vEnd.z) - 1.0f);
    box._vMax = PblVector3((vEnd.x > vStart.x ? vEnd.x : vStart.x) + 1.0f,
                           (vEnd.y > vStart.y ? vEnd.y : vStart.y) + 1.0f,
                           (vEnd.z > vStart.z ? vEnd.z : vStart.z) + 1.0f);
    TexCoord uv[4] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } };

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        pPrimitive = sCreateGrapplingHookCompressedGeometry(box, vStart, vEnd, iSegments, uv, fDist, vDirection,
                                vWaveDirection, vCameraRight, mLifeTime, mMaxLifetime,
                                mState == HOOK_ATTACHING, fLength);
    }
    else
    {
        pPrimitive = sCreateGrapplingHookGeometry(box, vStart, vEnd, iSegments, uv, fDist, vDirection,
                                vWaveDirection, vCameraRight, mLifeTime, mMaxLifetime,
                                mState == HOOK_ATTACHING, fLength);
    }

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   sShader,
                                   gMatrixIdentity, RedColor::WHITE,
                                   0x1000010, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00423ea0
void OrdnanceGrapplingHook::PlatformInit()
{
    unsigned int uiFormat =
        RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED ? 0x9282 : 0x282;
    sShader = pcRedShader::Create(0x8a8743bf /* "Particle" */, uiFormat,
                                  &sTextureHash, 1, 0x10004, RedColor::WHITE, RedColor::BLACK,
                                  32.0f);
}

// FUNCTION: SWBF2 0x00423ef0
void OrdnanceGrapplingHook::PlatformCleanup()
{
    if (sShader)
    {
        sShader->Release();
        sShader = 0;
    }
}
