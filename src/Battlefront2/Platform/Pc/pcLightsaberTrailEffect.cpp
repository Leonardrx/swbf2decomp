#include <d3dx9math.h>
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "Battlefront2/Platform/Pc/pcLightBeam.h"
const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);
#include "Battlefront2/Source/LightsaberTrailEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

// FUNCTION: SWBF2 0x004213e0
void LightsaberTrailEffect::_PlatformInit()
{
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        m_pShader = pcRedShader::Create(
            0xE8651328u /* "LightBeam" */, 0x1082,
            0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        m_pShader = pcRedShader::Create(
            0xE8651328u /* "LightBeam" */, 0x82,
            0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
}

// FUNCTION: SWBF2 0x00421440
void LightsaberTrailEffect::_PlatformCleanup()
{
    m_pShader->Release();
    m_pShader = 0;
}

namespace
{

struct VertexU
{
    PblVector3 pos;
    RedColor color;

    void SetPos(const PblVector3& v) { pos = v; }
    void SetColor(const RedColor& c) { color = c; }
};
 

const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });
}

// FUNCTION: SWBF2 0x00421480
void LightsaberTrailEffect::RenderLightsaberTrail(const LightsaberTrail* pTrail, unsigned int uiFlags) const
{
    unsigned char auiNumSteps[LightsaberTrail::MAX_SAMPLES_PER_TRAIL];
    float fTotalDist = 0.0f;
    unsigned int uiNumSteps = pTrail->GetTotalNumSteps(m_fLIGHTSABER_TRAIL_MAX_TIME_PER_STEP,
                                         m_fLIGHTSABER_TRAIL_MAX_DISTANCE_PER_STEP,
                                         auiNumSteps, &fTotalDist);
    if (!uiNumSteps)
    {
        return;
    }
    if (pTrail->IsTrailOn())
    {
        uiNumSteps += 2;
    }

    const LightsaberTrailClass* pClass = pTrail->m_pClass;
    RedColor colorTrail;
    colorTrail.a = 0xff;
    const RedColor colorHead = pClass->m_colorHead;
    const float fWidth = pClass->m_fWidth;
    const float fLength = pClass->m_fLength;
    colorTrail.b = (unsigned char)(int)((pClass->m_colorTrail.b * pClass->m_colorTrail.a + 0.5f) * (1.0f / 255.0f));
    colorTrail.g = (unsigned char)(int)((pClass->m_colorTrail.g * pClass->m_colorTrail.a + 0.5f) * (1.0f / 255.0f));
    colorTrail.r = (unsigned char)(int)((pClass->m_colorTrail.r * pClass->m_colorTrail.a + 0.5f) * (1.0f / 255.0f));
    PblBox bbox = pTrail->GetBoundingBox(fLength);
    const PblVector3& vCamZ = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    const float fPersistTime = m_fLIGHTSABER_TRAIL_PERSIST_TIME;
    const float fFadeTime = m_fLIGHTSABER_TRAIL_FADE_TIME;
    const float fFadeTimeInv = 1.0f / (fPersistTime - fFadeTime);
    const float fFadeDist = m_fLIGHTSABER_TRAIL_FADE_DISTANCE;
    const float fFadeDistInv = 1.0f / fFadeDist;
    const unsigned int uiNumVertices = 6 * uiNumSteps - 2;

    const bool bCompressed = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED;
    pcRedPrimitive* pPrim;
    if (bCompressed)
    {
        pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1082, sizeof(VertexC), uiNumVertices, 0, false);
        CompressedPos::SetBoundingBox(bbox);
    }
    else
    {
        pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x82, sizeof(VertexU), uiNumVertices, 0, false);
    }
    pPrim->m_bbox = bbox;
    RedColor colorEdge = colorTrail;
    colorEdge.a = 0;
    void* pData = pPrim->BeginVertexData();
    float fTime;
    if (bCompressed)
    {
        if (!pData)
        {
            return;
        }
        VertexC* pFront = (VertexC*)pData;
        VertexC* pBack = pFront + 4 * uiNumSteps - 2;
        VertexC* pThird = pBack + 1;

        PblVector3 vPos = pTrail->GetPos(0);
        PblVector3 vPosEnd = vPos + pTrail->GetDir(0) * fLength;
        PblVector3 vCap = pTrail->GetDir(0) - vCamZ * DotProduct(pTrail->GetDir(0), vCamZ);
        if (SquaredLength(vCap) <= 0.01f)
        {
            vCap *= 10.0f;
        }
        else
        {
            vCap = Normalize(vCap);
        }
        pFront[0].SetPos(vPos - vCap * fWidth);
        pFront[0].SetColor(colorEdge);
        pFront[1].SetPos(vPos);
        pFront[1].SetColor(colorEdge);
        pFront += 2;
        pBack[0].SetPos(vPosEnd);
        pBack[0].SetColor(colorEdge);
        pBack[-1].SetPos(vPos);
        pBack[-1].SetColor(colorEdge);
        pBack -= 2;
        pThird[0].SetPos(vPosEnd + vCap * fWidth);
        pThird[0].SetColor(colorEdge);
        pThird += 1;

        bool bCoreDone = !pTrail->IsTrailOn();
        float fTimePrev = pTrail->GetTime(0);
        float fAgePrev = pTrail->GetTimer() - fTimePrev;
        float fDist = 0.0f;
        for (unsigned int i = 1; i < pTrail->GetNumSamples(); ++i)
        {
            const float fSampleDist = pTrail->GetDist(i);
            const bool bMoved = fSampleDist > 0.0f;
            bool bNextVisible;
            if (i == pTrail->GetNumSamples() - 1)
            {
                bNextVisible = pTrail->IsTrailOn();
            }
            else
            {
                bNextVisible = auiNumSteps[i + 1] != 0;
            }
            const float fSampleTime = pTrail->GetTime(i);
            const int iNumSteps = auiNumSteps[i];
            const float fStepInv = 1.0f / iNumSteps;
            const float fTimeStep = (fSampleTime - fTimePrev) * fStepInv;
            const float fDistStep = fStepInv * fSampleDist;
            for (int j = 0; j < iNumSteps; ++j)
            {
                fTime = fTimePrev + fTimeStep;
                fDist += fDistStep;
                RedColor color = colorTrail;
                const float fAge = pTrail->GetTimer() - fTime;
                const PblVector3 vDir = pTrail->GetInterpolatedDir(fTime);
                vCap = vDir - vCamZ * DotProduct(vDir, vCamZ);
                if (SquaredLength(vCap) <= 0.01f)
                {
                    vCap *= 10.0f;
                }
                else
                {
                    vCap = Normalize(vCap);
                }

                if (bCoreDone || m_fLIGHTSABER_TRAIL_CORE_MAX_TIME < fAge ||
                    fDist <= fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)
                {
                    vPos = pTrail->GetInterpolatedPos(fTime);
                    vPosEnd = vPos + vDir * fLength;
                }
                else
                {

                    const PblVector3 vPosPrev = vPos;
                    const PblVector3 vPosEndPrev = vPosEnd;
                    vPos = pTrail->GetInterpolatedPos(fTime);
                    vPosEnd = vPosPrev + vDir * fLength;
                    const float fPosMove = Length(vPos - vPosPrev);
                    const float fTipMove = Length(vPosEnd - vPosEndPrev);
                    float fCore;
                    if (fAgePrev <= m_fLIGHTSABER_TRAIL_CORE_MAX_TIME)
                    {
                        fCore = (fDist - (fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)) /
                                (fPosMove > fTipMove ? fPosMove : fTipMove);
                        if (fCore > 1.0f)
                        {
                            fCore = 1.0f;
                        }
                    }
                    else
                    {
                        fCore = (m_fLIGHTSABER_TRAIL_CORE_MAX_TIME - fAge) / (fAgePrev - fAge);
                    }
                    float fPosWidth = 0.0f;
                    if (fPosMove > 0.01f)
                    {
                        fPosWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fPosMove;
                    }
                    float fTipWidth = 0.0f;
                    if (fTipMove > 0.01f)
                    {
                        fTipWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fTipMove;
                    }

                    float fPos = fPosWidth + fCore;
                    float fTip = fTipWidth + fCore;
                    PblVector3 vPosNext = Interpolate(vPos, vPosPrev, fPos);
                    PblVector3 vPosEndNext = Interpolate(vPosEnd, vPosEndPrev, fTip);
                    pFront[0].SetPos(vPosNext - vCap * fWidth);
                    pFront[0].SetColor(colorEdge);
                    pFront[1].SetPos(vPosNext);
                    pFront[1].SetColor(colorTrail);
                    pBack[0].SetPos(vPosEndNext);
                    pBack[0].SetColor(colorTrail);
                    pBack[-1].SetPos(vPosNext);
                    pBack[-1].SetColor(colorTrail);
                    pBack -= 2;
                    pThird[0].SetPos(vPosEndNext);
                    pThird[0].SetColor(colorTrail);
                    pThird[1].SetPos(vPosEndNext + vCap * fWidth);
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    if (i == pTrail->GetNumSamples() - 1 && j == iNumSteps - 1)
                    {
                        if (fPosWidth > fCore)
                        {
                            fPosWidth = fCore;
                        }
                        if (fTipWidth > fCore)
                        {
                            fTipWidth = fCore;
                        }
                    }
                    fPos = fCore - fPosWidth;
                    fTip = fCore - fTipWidth;
                    vPosNext = Interpolate(vPos, vPosPrev, fPos);
                    vPosEndNext = Interpolate(vPosEnd, vPosEndPrev, fTip);
                    pFront[2].SetPos(vPosNext - vCap * fWidth);
                    pFront[2].SetColor(colorEdge);
                    pFront[3].SetPos(vPosNext);
                    pFront[3].SetColor(colorHead);
                    pFront += 4;
                    pBack[0].SetPos(vPosEndNext);
                    pBack[0].SetColor(colorHead);
                    pBack[-1].SetPos(vPosNext);
                    pBack[-1].SetColor(colorHead);
                    pBack -= 2;
                    pThird[0].SetPos(vPosEndNext);
                    pThird[0].SetColor(colorHead);
                    pThird[1].SetPos(vPosEndNext + vCap * fWidth);
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    vPos = fPos > 0.0f ? vPos : vPosNext;
                    vPosEnd = fTip > 0.0f ? vPosEnd : vPosEndNext;
                    bCoreDone = true;
                }

                if (bCoreDone && pTrail->IsTrailOn())
                {
                    color = colorHead;
                }
                else if (fAge < fPersistTime && bMoved && (bNextVisible || j != iNumSteps - 1))
                {
                    float fFade = 1.0f;
                    if (fAge > fFadeTime)
                    {
                        fFade = (fPersistTime - fAge) * fFadeTimeInv * ((fPersistTime - fAge) * fFadeTimeInv);
                    }
                    if (fFadeDist > fDist)
                    {
                        fFade = fDist * fFadeDistInv * fFade;
                    }
                    if (fFade < 1.0f)
                    {
                        color.a = (unsigned char)(int)(color.a * fFade);
                    }
                }
                else
                {
                    color.a = 0;
                }

                pFront[0].SetPos(vPos - vCap * fWidth);
                pFront[0].SetColor(colorEdge);
                pFront[1].SetPos(vPos);
                pFront[1].SetColor(color);
                pFront += 2;
                pBack[0].SetPos(vPosEnd);
                pBack[0].SetColor(color);
                pBack[-1].SetPos(vPos);
                pBack[-1].SetColor(color);
                pBack -= 2;
                pThird[0].SetPos(vPosEnd);
                pThird[0].SetColor(color);
                pThird[1].SetPos(vPosEnd + vCap * fWidth);
                pThird[1].SetColor(colorEdge);
                pThird += 2;

                fAgePrev = fAge;
                fTimePrev = fTime;
            }
            fTimePrev = fSampleTime;
        }
    }
    else
    {
        if (!pData)
        {
            return;
        }
        VertexU* pFront = (VertexU*)pData;
        VertexU* pBack = pFront + 4 * uiNumSteps - 2;
        VertexU* pThird = pBack + 1;

        PblVector3 vPos = pTrail->GetPos(0);
        PblVector3 vPosEnd = vPos + pTrail->GetDir(0) * fLength;
        PblVector3 vCap = pTrail->GetDir(0) - vCamZ * DotProduct(pTrail->GetDir(0), vCamZ);
        if (SquaredLength(vCap) <= 0.01f)
        {
            vCap *= 10.0f;
        }
        else
        {
            vCap = Normalize(vCap);
        }
        pFront[0].SetPos(vPos - vCap * fWidth);
        pFront[0].SetColor(colorEdge);
        pFront[1].SetPos(vPos);
        pFront[1].SetColor(colorEdge);
        pFront += 2;
        pBack[0].SetPos(vPosEnd);
        pBack[0].SetColor(colorEdge);
        pBack[-1].SetPos(vPos);
        pBack[-1].SetColor(colorEdge);
        pBack -= 2;
        pThird[0].SetPos(vPosEnd + vCap * fWidth);
        pThird[0].SetColor(colorEdge);
        pThird += 1;

        bool bCoreDone = !pTrail->IsTrailOn();
        float fTimePrev = pTrail->GetTime(0);
        float fAgePrev = pTrail->GetTimer() - fTimePrev;
        float fDist = 0.0f;
        for (unsigned int i = 1; i < pTrail->GetNumSamples(); ++i)
        {
            const float fSampleDist = pTrail->GetDist(i);
            const bool bMoved = fSampleDist > 0.0f;
            bool bNextVisible;
            if (i == pTrail->GetNumSamples() - 1)
            {
                bNextVisible = pTrail->IsTrailOn();
            }
            else
            {
                bNextVisible = auiNumSteps[i + 1] != 0;
            }
            const float fSampleTime = pTrail->GetTime(i);
            const int iNumSteps = auiNumSteps[i];
            const float fStepInv = 1.0f / iNumSteps;
            const float fTimeStep = (fSampleTime - fTimePrev) * fStepInv;
            const float fDistStep = fStepInv * fSampleDist;
            for (int j = 0; j < iNumSteps; ++j)
            {
                fTime = fTimePrev + fTimeStep;
                fDist += fDistStep;
                RedColor color = colorTrail;
                const float fAge = pTrail->GetTimer() - fTime;
                const PblVector3 vDir = pTrail->GetInterpolatedDir(fTime);
                vCap = vDir - vCamZ * DotProduct(vDir, vCamZ);
                if (SquaredLength(vCap) <= 0.01f)
                {
                    vCap *= 10.0f;
                }
                else
                {
                    vCap = Normalize(vCap);
                }

                if (bCoreDone || m_fLIGHTSABER_TRAIL_CORE_MAX_TIME < fAge ||
                    fDist <= fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)
                {
                    vPos = pTrail->GetInterpolatedPos(fTime);
                    vPosEnd = vPos + vDir * fLength;
                }
                else
                {

                    const PblVector3 vPosPrev = vPos;
                    const PblVector3 vPosEndPrev = vPosEnd;
                    vPos = pTrail->GetInterpolatedPos(fTime);
                    vPosEnd = vPosPrev + vDir * fLength;
                    const float fPosMove = Length(vPos - vPosPrev);
                    const float fTipMove = Length(vPosEnd - vPosEndPrev);
                    float fCore;
                    if (fAgePrev <= m_fLIGHTSABER_TRAIL_CORE_MAX_TIME)
                    {
                        fCore = (fDist - (fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)) /
                                (fPosMove > fTipMove ? fPosMove : fTipMove);
                        if (fCore > 1.0f)
                        {
                            fCore = 1.0f;
                        }
                    }
                    else
                    {
                        fCore = (m_fLIGHTSABER_TRAIL_CORE_MAX_TIME - fAge) / (fAgePrev - fAge);
                    }
                    float fPosWidth = 0.0f;
                    if (fPosMove > 0.01f)
                    {
                        fPosWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fPosMove;
                    }
                    float fTipWidth = 0.0f;
                    if (fTipMove > 0.01f)
                    {
                        fTipWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fTipMove;
                    }

                    float fPos = fPosWidth + fCore;
                    float fTip = fTipWidth + fCore;
                    PblVector3 vPosNext = Interpolate(vPos, vPosPrev, fPos);
                    PblVector3 vPosEndNext = Interpolate(vPosEnd, vPosEndPrev, fTip);
                    pFront[0].SetPos(vPosNext - vCap * fWidth);
                    pFront[0].SetColor(colorEdge);
                    pFront[1].SetPos(vPosNext);
                    pFront[1].SetColor(colorTrail);
                    pBack[0].SetPos(vPosEndNext);
                    pBack[0].SetColor(colorTrail);
                    pBack[-1].SetPos(vPosNext);
                    pBack[-1].SetColor(colorTrail);
                    pBack -= 2;
                    pThird[0].SetPos(vPosEndNext);
                    pThird[0].SetColor(colorTrail);
                    pThird[1].SetPos(vPosEndNext + vCap * fWidth);
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    if (i == pTrail->GetNumSamples() - 1 && j == iNumSteps - 1)
                    {
                        if (fPosWidth > fCore)
                        {
                            fPosWidth = fCore;
                        }
                        if (fTipWidth > fCore)
                        {
                            fTipWidth = fCore;
                        }
                    }
                    fPos = fCore - fPosWidth;
                    fTip = fCore - fTipWidth;
                    vPosNext = Interpolate(vPos, vPosPrev, fPos);
                    vPosEndNext = Interpolate(vPosEnd, vPosEndPrev, fTip);
                    pFront[2].SetPos(vPosNext - vCap * fWidth);
                    pFront[2].SetColor(colorEdge);
                    pFront[3].SetPos(vPosNext);
                    pFront[3].SetColor(colorHead);
                    pFront += 4;
                    pBack[0].SetPos(vPosEndNext);
                    pBack[0].SetColor(colorHead);
                    pBack[-1].SetPos(vPosNext);
                    pBack[-1].SetColor(colorHead);
                    pBack -= 2;
                    pThird[0].SetPos(vPosEndNext);
                    pThird[0].SetColor(colorHead);
                    pThird[1].SetPos(vPosEndNext + vCap * fWidth);
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    vPos = fPos > 0.0f ? vPos : vPosNext;
                    vPosEnd = fTip > 0.0f ? vPosEnd : vPosEndNext;
                    bCoreDone = true;
                }

                if (bCoreDone && pTrail->IsTrailOn())
                {
                    color = colorHead;
                }
                else if (fAge < fPersistTime && bMoved && (bNextVisible || j != iNumSteps - 1))
                {
                    float fFade = 1.0f;
                    if (fAge > fFadeTime)
                    {
                        fFade = (fPersistTime - fAge) * fFadeTimeInv * ((fPersistTime - fAge) * fFadeTimeInv);
                    }
                    if (fFadeDist > fDist)
                    {
                        fFade = fDist * fFadeDistInv * fFade;
                    }
                    if (fFade < 1.0f)
                    {
                        color.a = (unsigned char)(int)(color.a * fFade);
                    }
                }
                else
                {
                    color.a = 0;
                }

                pFront[0].SetPos(vPos - vCap * fWidth);
                pFront[0].SetColor(colorEdge);
                pFront[1].SetPos(vPos);
                pFront[1].SetColor(color);
                pFront += 2;
                pBack[0].SetPos(vPosEnd);
                pBack[0].SetColor(color);
                pBack[-1].SetPos(vPos);
                pBack[-1].SetColor(color);
                pBack -= 2;
                pThird[0].SetPos(vPosEnd);
                pThird[0].SetColor(color);
                pThird[1].SetPos(vPosEnd + vCap * fWidth);
                pThird[1].SetColor(colorEdge);
                pThird += 2;

                fAgePrev = fAge;
                fTimePrev = fTime;
            }
            fTimePrev = fSampleTime;
        }
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, m_pShader,
                                   _IdentityMatrix, RedColor::WHITE,
                                   uiFlags, 0, 0, 0);
    pPrim->Release();
}
