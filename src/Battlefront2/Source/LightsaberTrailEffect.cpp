#include <d3dx9math.h>
#include <math.h>

#include "LightsaberTrailEffect.h"

// GLOBAL: SWBF2 0x01e573ec
LightsaberTrailEffect* LightsaberTrailEffect::s_pInstance;

// FUNCTION: SWBF2 0x00573ea0
void LightsaberTrailEffect::Create()
{
    s_pInstance = new LightsaberTrailEffect();
}

// FUNCTION: SWBF2 0x00573e30
void LightsaberTrailEffect::Init()
{
    m_fLIGHTSABER_TRAIL_PERSIST_TIME = 0.12f;
    m_fLIGHTSABER_TRAIL_FADE_TIME = 0.02f;
    m_fLIGHTSABER_TRAIL_FADE_DISTANCE = 0.35f;
    m_fLIGHTSABER_TRAIL_CORE_MAX_TIME = 0.4f;
    m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE = 0.8f;
    m_fLIGHTSABER_TRAIL_CORE_WIDTH = 0.3f;
    m_fLIGHTSABER_TRAIL_MAX_TIME_PER_STEP = 0.01f;
    m_fLIGHTSABER_TRAIL_MAX_DISTANCE_PER_STEP = 0.1f;
    _PlatformInit();
}



// FUNCTION: SWBF2 0x00573d20
void LightsaberTrailEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x00573e70
void LightsaberTrailEffect::Cleanup()
{
    _PlatformCleanup();
}

// FUNCTION: SWBF2 0x00573e80
void LightsaberTrailEffect::ReadConfig(PblConfig&)
{
}

// FUNCTION: SWBF2 0x00573e90
FLEffectClass* LightsaberTrailEffect::BuildClass(PblConfig&) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00573f40
void LightsaberTrailEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

namespace GameLoop
{

// GLOBAL: SWBF2 0x01e56058
float sClientDeltaTime;
}

// FUNCTION: SWBF2 0x00573f60
LightsaberTrail::LightsaberTrail(const LightsaberTrailClass* pClass)
    : m_pClass(const_cast<LightsaberTrailClass*>(pClass))
{
    const PblVector3 vFar(3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f);
    m_vDirPrev = vFar;
    m_vPosPrev = vFar;
    m_bTrailOn = false;
    m_bTrailWasOn = false;
    m_bEffectActive = true;
    m_fTimer = 0.0f;
    m_uiSampleNext = 0;
    m_uiSampleFirst = 0;
    m_uiNumSamples = 0;
}




// FUNCTION: SWBF2 0x00573da0
bool LightsaberTrail::IsEffectActive() const
{
    return m_bEffectActive;
}

// FUNCTION: SWBF2 0x00573db0
void LightsaberTrail::ActivateEffect()
{
    m_bEffectActive = true;
}

// FUNCTION: SWBF2 0x00573dc0
void LightsaberTrail::DeactivateEffect()
{
    m_bEffectActive = false;
}

// FUNCTION: SWBF2 0x00573dd0
void LightsaberTrail::AddToRenderList()
{
}

// FUNCTION: SWBF2 0x00573de0
void LightsaberTrail::RemoveFromRenderList()
{
}

// FUNCTION: SWBF2 0x00574040
void LightsaberTrail::UpdatedWorldMatrix()
{
    const float fDeltaTime = GameLoop::sClientDeltaTime;
    const D3DXMATRIX* pWorld = reinterpret_cast<const D3DXMATRIX*>(&m_mWorldMatrix);
    if (fDeltaTime > 0.0f)
    {
        PblVector3 vPos, vDir;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPos), reinterpret_cast<const D3DXVECTOR3*>(&m_pClass->m_vOffsetPos), pWorld);
        D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&m_pClass->m_vOffsetDir), pWorld);

        if (SquaredLength(vPos - m_vPosPrev) > 100.0f)
        {
            m_vPosPrev = vPos;
            m_vDirPrev = vDir;
        }

        _Update(fDeltaTime, vPos, vDir, m_vPosPrev, m_vDirPrev, m_bTrailOn && m_bEffectActive);
        m_vPosPrev = vPos;
        m_vDirPrev = vDir;
    }
}

// FUNCTION: SWBF2 0x00574150
void LightsaberTrail::_Update(float fDeltaTime, const PblVector3& vPos, const PblVector3& vDir,
                              const PblVector3& vPosPrev, const PblVector3& vDirPrev, bool bTrailOn)
{
    if (fDeltaTime <= 0.0f)
    {
        return;
    }

    m_fTimer += fDeltaTime;

    unsigned char i;
    for (i = 0; i < m_uiNumSamples; ++i)
    {
        unsigned char uiSample = (unsigned char)((m_uiSampleFirst + i) % MAX_SAMPLES_PER_TRAIL);
        if (m_afTime[uiSample] >=
            m_fTimer - LightsaberTrailEffect::s_pInstance->m_fLIGHTSABER_TRAIL_PERSIST_TIME)
        {
            if (m_afDist[uiSample] > 0.0f)
            {
                --i;
            }
            break;
        }
        m_afDist[uiSample] = 0.0f;
    }

    if (i)
    {
        if (i < m_uiNumSamples)
        {
            m_uiNumSamples -= i;
        }
        else
        {
            m_uiNumSamples = m_afTime[m_uiNumSamples - 1] >= m_fTimer - fDeltaTime * 1.5f;
        }
        m_uiSampleFirst = (unsigned char)((m_uiSampleNext - m_uiNumSamples + MAX_SAMPLES_PER_TRAIL) %
                                          MAX_SAMPLES_PER_TRAIL);
    }

    if (bTrailOn || m_bTrailWasOn)
    {
        if (!m_uiNumSamples || m_fTimer - GetLastTime() > fDeltaTime * 1.5f)
        {
            _AddSample(m_fTimer - fDeltaTime, vPosPrev, vDirPrev, false);
        }
        _AddSample(m_fTimer, vPos, vDir, true);
    }
    m_bTrailWasOn = bTrailOn;
}

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x005742f0
void LightsaberTrail::_AddSample(float fTime, const PblVector3& vPos, const PblVector3& vDir, bool bHead)
{
    m_afTime[m_uiSampleNext] = fTime;
    m_avPos[m_uiSampleNext] = vPos;
    m_avDir[m_uiSampleNext] = vDir;

    if (m_uiNumSamples && bHead)
    {
        const int iSamplePrev = (m_uiSampleNext + MAX_SAMPLES_PER_TRAIL - 1) % MAX_SAMPLES_PER_TRAIL;
        const float fDt = fTime - m_afTime[iSamplePrev];
        const float fDtInv = 1.0f / fDt;
        const PblVector3 vDPosDt = (vPos - m_avPos[iSamplePrev]) * fDtInv;
        PblVector3 vDDirDt;
        const float fC = DotProduct(vDir, m_avDir[iSamplePrev]);
        if (fC >= 0.999f)
        {
            m_avOmg[m_uiSampleNext] = (vDir - m_avDir[iSamplePrev]) * fDtInv;
            vDDirDt = m_avOmg[m_uiSampleNext];
        }
        else
        {
            const float fLen = sqrtf(fC * 2.0f + 2.0f);
            const float fF = (2.0f - fLen) * (fDtInv * 4.0f) / ((1.0f - fC) * fLen);
            m_avOmg[m_uiSampleNext] = vDir * (fF * fC) - m_avDir[iSamplePrev] * fF;
            vDDirDt = vDir * fF - m_avDir[iSamplePrev] * (fF * fC);
        }

        if (m_uiNumSamples == 1 || m_afDist[iSamplePrev] <= 0.0f)
        {
            m_avVel[iSamplePrev] = vDPosDt;
            m_avOmg[iSamplePrev] = vDDirDt;
        }
        else
        {
            const float fF = (fDt * 0.5f) /
                (fTime - m_afTime[(m_uiSampleNext + MAX_SAMPLES_PER_TRAIL - 1) % MAX_SAMPLES_PER_TRAIL]);
            m_avVel[iSamplePrev] = m_avVel[iSamplePrev] * fF + vDPosDt * (1.0f - fF);
            m_avOmg[iSamplePrev] = m_avOmg[iSamplePrev] * fF + vDDirDt * (1.0f - fF);
        }
        m_avVel[m_uiSampleNext] = vDPosDt * 2.0f - m_avVel[iSamplePrev];

        const float fVel = Length(m_avVel[m_uiSampleNext]);
        const float fVelPrev = Length(m_avVel[iSamplePrev]);
        float fDistance = (fVel + fVelPrev) * 0.5f;
        if (fVel > 0.001f && fVelPrev > 0.001f)
        {
            fDistance *= DotProduct(m_avVel[m_uiSampleNext], m_avVel[iSamplePrev]) * 0.25f /
                         (fVel * fVelPrev) + 0.75f;
        }
        fDistance *= fDt;

        const PblVector3 vVelEnd = m_avVel[m_uiSampleNext] + m_avOmg[m_uiSampleNext];
        const PblVector3 vVelEndPrev = m_avVel[iSamplePrev] + m_avOmg[iSamplePrev];
        const float fVelEnd = Length(vVelEnd);
        const float fVelEndPrev = Length(vVelEndPrev);
        float fDistanceEnd = (fVelEnd + fVelEndPrev) * 0.5f;
        if (fVelEnd > 0.001f && fVelEndPrev > 0.001f)
        {
            fDistanceEnd *= DotProduct(vVelEnd, vVelEndPrev) * 0.25f /
                            (fVelEnd * fVelEndPrev) + 0.75f;
        }
        fDistanceEnd *= fDt;
        if (fDistance < fDistanceEnd)
        {
            fDistance = fDistanceEnd;
        }
        if (fDistance < 0.001f)
        {
            fDistance = 0.001f;
        }
        m_afDist[m_uiSampleNext] = fDistance;
    }
    else
    {
        m_avOmg[m_uiSampleNext] = gVectorZero;
        m_avVel[m_uiSampleNext] = m_avOmg[m_uiSampleNext];
        m_afDist[m_uiSampleNext] = 0.0f;
    }

    m_uiSampleNext = (unsigned char)((m_uiSampleNext + 1) % MAX_SAMPLES_PER_TRAIL);
    if (m_uiNumSamples < MAX_SAMPLES_PER_TRAIL)
    {
        ++m_uiNumSamples;
    }
    else
    {
        m_uiSampleFirst = (unsigned char)((m_uiSampleFirst + 1) % MAX_SAMPLES_PER_TRAIL);
    }
}

// FUNCTION: SWBF2 0x00574b70
float LightsaberTrail::GetLastTime() const
{
    if (!m_uiNumSamples)
    {
        return -3.402823466e+38f;
    }
    return m_afTime[(m_uiSampleNext + MAX_SAMPLES_PER_TRAIL - 1) % MAX_SAMPLES_PER_TRAIL];
}

// FUNCTION: SWBF2 0x00574bb0
unsigned int LightsaberTrail::GetTotalNumSteps(float fMaxTimeStep, float fMaxDistanceStep,
                                      unsigned char* pSteps, float* pTotalDistance) const
{
    const unsigned char uiCount = m_uiNumSamples;
    if (!uiCount)
    {
        return 0;
    }
    const unsigned char uiFirst = m_uiSampleFirst;
    unsigned int uiStepCount = 1;
    float fPreviousTime = m_afTime[uiFirst];
    if (pSteps)
    {
        pSteps[0] = 1;
    }
    if (pTotalDistance)
    {
        *pTotalDistance = 0.0f;
    }

    for (unsigned char i = 1; i < uiCount; ++i)
    {
        const unsigned char uiIndex = (uiFirst + i) % MAX_SAMPLES_PER_TRAIL;
        const float fDistance = m_afDist[uiIndex];
        unsigned int uiSteps = 1;
        if (fDistance > 0.0f)
        {
            const float fElapsed = m_afTime[uiIndex] - fPreviousTime;
            if (fElapsed > fMaxTimeStep)
            {
                uiSteps = static_cast<unsigned int>(fElapsed / fMaxTimeStep) + 1;
            }
            if (fDistance > fMaxDistanceStep)
            {
                const unsigned int uiDistanceSteps =
                    static_cast<unsigned int>(fDistance / fMaxDistanceStep) + 1;
                if (uiSteps < uiDistanceSteps)
                {
                    uiSteps = uiDistanceSteps;
                }
            }
            if (pTotalDistance)
            {
                *pTotalDistance += fDistance;
            }
        }
        if (pSteps)
        {
            pSteps[i] = static_cast<unsigned char>(uiSteps);
        }
        fPreviousTime = m_afTime[uiIndex];
        uiStepCount += uiSteps;
    }
    return uiStepCount;
}

// FUNCTION: SWBF2 0x00574d00
PblBox LightsaberTrail::GetBoundingBox(float fLength) const
{
    PblBox bbox(m_avPos[m_uiSampleFirst]);
    unsigned char uiSamplePrev = m_uiSampleFirst;
    for (unsigned char i = 1; i < m_uiNumSamples; ++i)
    {
        const unsigned char uiSample = static_cast<unsigned char>((m_uiSampleFirst + i) % MAX_SAMPLES_PER_TRAIL);
        const PblVector3 vPosEnd = m_avPos[uiSample] + m_avDir[uiSample] * fLength;
        bbox.AddPoint(m_avPos[uiSample]);
        bbox.AddPoint(vPosEnd);
        if (m_afDist[uiSample] > 0.0f)
        {
            const float fExtrapolateTime = (m_afTime[uiSample] - m_afTime[uiSamplePrev]) * 0.5f;
            bbox.AddPoint(m_avPos[uiSamplePrev] + m_avVel[uiSamplePrev] * fExtrapolateTime);
            bbox.AddPoint(m_avPos[uiSample] - m_avVel[uiSample] * fExtrapolateTime);
        }
        uiSamplePrev = uiSample;
    }
    bbox.SetMax(PblVector3(fLength * 1.5f, fLength * 1.5f, fLength * 1.5f) + bbox.GetMax());
    bbox.SetMin(bbox.GetMin() - PblVector3(fLength * 1.5f, fLength * 1.5f, fLength * 1.5f));
    return bbox;
}

// GLOBAL: SWBF2 0x01e56fa1
unsigned char LightsaberTrail::s_uiSample = 0;
// GLOBAL: SWBF2 0x01e56fa2
unsigned char LightsaberTrail::s_uiSamplePrev = 0;
// GLOBAL: SWBF2 0x01e573e8
float LightsaberTrail::s_fDeltaTimeInv = 0.0f;

// FUNCTION: SWBF2 0x00575140
void LightsaberTrail::_SetSample(float fTime) const
{
    unsigned char uiEnd = LightsaberTrail::s_uiSample;
    if (LightsaberTrail::s_uiSample == LightsaberTrail::s_uiSamplePrev || m_afTime[LightsaberTrail::s_uiSamplePrev] > fTime)
    {
        LightsaberTrail::s_uiSamplePrev = m_uiSampleFirst;
        uiEnd = (LightsaberTrail::s_uiSample + 1) % MAX_SAMPLES_PER_TRAIL;
        LightsaberTrail::s_uiSample = uiEnd;
        LightsaberTrail::s_fDeltaTimeInv = 1.0f / (m_afTime[uiEnd] - m_afTime[LightsaberTrail::s_uiSamplePrev]);
    }
    if (fTime > m_afTime[uiEnd])
    {
        const unsigned char uiSampleNext = m_uiSampleNext;
        float fNextTime;
        do
        {
            const unsigned char uiNext = (uiEnd + 1) % MAX_SAMPLES_PER_TRAIL;
            if (uiNext == uiSampleNext)
            {
                break;
            }
            const unsigned char uiPrevious = uiEnd;
            uiEnd = uiNext;
            LightsaberTrail::s_uiSamplePrev = uiPrevious;
            LightsaberTrail::s_uiSample = uiEnd;
            fNextTime = m_afTime[uiEnd];
            LightsaberTrail::s_fDeltaTimeInv = 1.0f / (fNextTime - m_afTime[uiPrevious]);
        }
        while (fTime > fNextTime);
    }
}

// FUNCTION: SWBF2 0x00575230
PblVector3 LightsaberTrail::GetInterpolatedPos(float fTime) const
{
    PblVector3 result;
    _SetSample(fTime);
    const unsigned int uiBegin = LightsaberTrail::s_uiSamplePrev;
    const unsigned int uiEnd = LightsaberTrail::s_uiSample;
    const float fElapsed = fTime - m_afTime[uiBegin];
    const float fFraction = LightsaberTrail::s_fDeltaTimeInv * fElapsed;
    const float fRemaining = 1.0f - fFraction;
    const float fWeightedElapsed = fRemaining * fElapsed;
    const float fEndVelocityWeight = fWeightedElapsed * fFraction;
    const float fBeginVelocityWeight = fRemaining * fWeightedElapsed;
    const float fEndWeight = (3.0f - 2.0f * fFraction) * fFraction * fFraction;
    const float fBeginWeight = 1.0f - fEndWeight;
    return m_avPos[uiBegin] * fBeginWeight + m_avPos[uiEnd] * fEndWeight
         + m_avVel[uiBegin] * fBeginVelocityWeight
         - m_avVel[uiEnd] * fEndVelocityWeight;
}

// FUNCTION: SWBF2 0x005753b0
PblVector3 LightsaberTrail::GetInterpolatedDir(float fTime) const
{
    PblVector3 result;
    _SetSample(fTime);
    const unsigned int uiBegin = LightsaberTrail::s_uiSamplePrev;
    const unsigned int uiEnd = LightsaberTrail::s_uiSample;
    const float fElapsed = fTime - m_afTime[uiBegin];
    const float fFraction = LightsaberTrail::s_fDeltaTimeInv * fElapsed;
    const float fRemaining = 1.0f - fFraction;
    const float fWeightedElapsed = fRemaining * fElapsed;
    const float fEndVelocityWeight = fWeightedElapsed * fFraction;
    const float fBeginVelocityWeight = fRemaining * fWeightedElapsed;
    const float fEndWeight = (3.0f - 2.0f * fFraction) * fFraction * fFraction;
    const float fBeginWeight = 1.0f - fEndWeight;
    return m_avDir[uiBegin] * fBeginWeight + m_avDir[uiEnd] * fEndWeight
         + m_avOmg[uiBegin] * fBeginVelocityWeight
         - m_avOmg[uiEnd] * fEndVelocityWeight;
}
