#include <math.h>
#include <d3dx9math.h>

#include "sndsource3d.h"
#include "sndsource.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblMath.h"
#include "sndenginebase.h"

namespace Snd
{

// GLOBAL: SWBF2 0x007e37a0
float Source3D::sPitchMultiplier = 1.0f;

// FUNCTION: SWBF2 0x0073afd0
Source3D::Properties::Properties()
{
    mRollInFactor = 0.0f;
    mRollInDistance = 0.0f;
    mMinDistance = 0.0f;
    mMuteDistance = 1.0f;
    mMaxDistance = 1.0f;
    mRollOffFactor = 0.0f;

    mOffCentreGain = 0.6f;
    mSpaceDistance = 0.0f;
    mFlags = (unsigned char)flags_dopplerMode;
}

// FUNCTION: SWBF2 0x0073b010
Source3D::Source3D(Properties* pProps)
{
    mProps = pProps;
    mClosestListener = 0;
    mDistanceLength = 0.0f;
    mPos3D = &Globals::smNullVector;
    mVel3D = &Globals::smNullVector;
    mPrevAngle = 0.0f;
    mFlags = (unsigned char)flags_updateListener;
}

// FUNCTION: SWBF2 0x0073b050
void Source3D::CalculateGain(float& rfGain) const
{
    float fDistance = mDistanceLength;
    float rollInDistance = mProps->mRollInDistance;
    float minDistance = mProps->mMinDistance;
    float maxDistance = mProps->mMaxDistance;

    if (rollInDistance > fDistance)
    {
        float fOffset = rollInDistance - fDistance;
        rfGain = (rollInDistance - fOffset)
               / (mProps->mRollOffFactor * mClosestListener->mRollOffFactor
                    * fOffset
                  + rollInDistance);
    }

    else if (minDistance >= fDistance)
    {
        rfGain = 1.0f;
    }

    else if (fDistance >= mProps->mMuteDistance)
    {
        rfGain = 0.0f;
    }
    else
    {
        float fOffset = fDistance - minDistance;
        float fRange = maxDistance - minDistance;
        rfGain = (fRange - fOffset)
               / (mProps->mRollOffFactor * mClosestListener->mRollOffFactor
                    * fOffset
                  + fRange);
    }

    if (!mProps->GetCentrePeakMode())
    {
        return;
    }

    ::PblVector3 vDirection;
    CalculateNormalizedDirection(vDirection);

    const Listener* pListener = mClosestListener;
    const ::PblVector3 right = pListener->mRight;
    const ::PblVector3 front = *pListener->mFront;

    float fRight = right.y * vDirection.y
                 + vDirection.x * right.x
                 + vDirection.z * right.z;
    float fCone = 1.0f - (float)fabs((double)fRight) * 2.0f;

    float fFront = -(front.x * vDirection.x
                     + front.y * vDirection.y
                     + front.z * vDirection.z);
    fCone *= fFront;

    if (mProps->mOffCentreGain > fCone)
    {
        fCone = mProps->mOffCentreGain;
    }

    rfGain *= fCone;
}

// FUNCTION: SWBF2 0x0073b1d0
void Source3D::CalculateGain(float fGain, float fReverbGain, GainMatrix& rMatrix) const
{
    const float fHalfPi = 1.57079637f;

    ::PblVector3 vDirection = mDistance;
    float fDistanceGain;
    CalculateGain(fDistanceGain);

    if (fDistanceGain == 0.0f)
    {
        rMatrix.frontLeft = 0.0f;
        rMatrix.frontRight = 0.0f;
        rMatrix.rearLeft = 0.0f;
        rMatrix.rearRight = 0.0f;
        rMatrix.reverb = fReverbGain;
        return;
    }

    float fRight;
    float fBack;
    if (EngineBase::smSpeakerConfig != EngineBase::speakerConfig_Mono)
    {
        CalculateNormalizedDirection(vDirection);

        const ::PblVector3 vRight = mClosestListener->mRight;
        const float fRawDot = vDirection.x * vRight.x + vRight.y * vDirection.y + vRight.z * vDirection.z;
        const float fDot = (fRawDot < -1.0f) ? -1.0f : ((fRawDot > 1.0f) ? 1.0f : fRawDot);

        float fLeft = 0.5f - fDot * 0.5f;
        if (fLeft < 0.0f)
        {
            fLeft = 0.0f;
        }
        else if (fLeft > 1.0f)
        {
            fLeft = 1.0f;
        }
        fRight = 1.0f - fLeft;

        const ::PblVector3 vFront = *mClosestListener->mFront;
        fBack = 0.5f - (vFront.x * vDirection.x * 0.5f + vFront.y * vDirection.y * 0.5f
                        + vFront.z * vDirection.z * 0.5f);
        if (fBack < 0.0f)
        {
            fBack = 0.0f;
        }
        else if (fBack > 1.0f)
        {
            fBack = 1.0f;
        }
    }
    else
    {
        fRight = 0.5f;
        fBack = 0.5f;
    }

    const float fTotal = fDistanceGain * fGain;
    rMatrix.reverb = fReverbGain;

    switch (EngineBase::smSpeakerConfig)
    {
    case EngineBase::speakerConfig_Mono:
        rMatrix.frontLeft = fTotal;
        rMatrix.frontRight = fTotal;
        return;

    case EngineBase::speakerConfig_Headphones:
        fRight = fRight * 0.5f + 0.25f;

    case EngineBase::speakerConfig_Stereo:
    case EngineBase::speakerConfig_Quadraphonic:
    case EngineBase::speakerConfig_5point1:
    case EngineBase::speakerConfig_7point1:
    {
        float fLeft = 1.0f - fRight;
        fRight *= fTotal;
        fLeft *= fTotal;
        rMatrix.frontRight = fRight;
        rMatrix.rearRight = fRight;
        rMatrix.frontLeft = fLeft;
        rMatrix.rearLeft = fLeft;
        return;
    }

    case EngineBase::speakerConfig_AnalogSurround:
    {
        const float fX = (float)fabs((double)(fBack - 0.5f));
        const float fY = (float)fabs((double)(fRight - 0.5f));
        float fAngle = (float)asin((double)(fY / (float)sqrt((double)(fY * fY + fX * fX))));

        if (fRight < 0.5f)
        {
            if (fBack < 0.5f)
            {
                fAngle += PblMath::fPi;
            }
            else
            {
                fAngle = (fHalfPi - fAngle) + (PblMath::fPi + fHalfPi);
            }
        }
        else if (fBack < 0.5f)
        {
            fAngle = (fHalfPi - fAngle) + fHalfPi;
        }

        if ((float)fabs((double)(fAngle - mPrevAngle)) > PblMath::fPi)
        {
            if (mFlags & 8)
            {
                mFlags &= ~8;
            }
            else
            {
                mFlags |= 8;
            }
        }
        mPrevAngle = fAngle;

        const float fPan = fAngle * 0.5f + fHalfPi * 0.5f;
        rMatrix.frontLeft = PblMath::Sin(fPan + fHalfPi) * fTotal;
        rMatrix.frontRight = PblMath::Sin(fPan) * fTotal;
        if (mFlags & 8)
        {
            rMatrix.frontLeft = -rMatrix.frontLeft;
            rMatrix.frontRight = -rMatrix.frontRight;
        }
        rMatrix.rearLeft = 0.0f;
        rMatrix.rearRight = 0.0f;
        break;
    }

    default:
        return;
    }
}

// FUNCTION: SWBF2 0x0073b590
void Source3D::CalculateNormalizedDirection(::PblVector3& rvDirection) const
{
    ::PblVector3 vNormal;

    if (0.0f == mDistance.x && 0.0f == mDistance.y && 0.0f == mDistance.z)
    {
        vNormal = *mClosestListener->mFront;
    }
    else if (EngineBase::smSpeakerConfig == EngineBase::speakerConfig_AnalogSurround)
    {
        D3DXVec3Normalize((D3DXVECTOR3*)&vNormal,
                           (const D3DXVECTOR3*)&mDistance);
    }
    else
    {
        vNormal = NormalizeEst(mDistance);
    }

    rvDirection = vNormal;
}

// FUNCTION: SWBF2 0x0073b6f0
float Source3D::CalculatePitch() const
{
    if (mProps->GetDopplerMode())
    {
        const Listener* listener = mClosestListener;
        float lastDistance = LengthEst((*mPos3D - *mVel3D)
                                       - (*listener->mPos - *listener->mVel));
        float distance = LengthEst(*mPos3D - *listener->mPos);
        float diff = lastDistance - distance;

        if (diff < 172.0f)
        {
            float speed = listener->mDistanceFactor * 344.0f;
            float closing = (distance - lastDistance) * listener->mDopplerFactor;
            return (speed - closing) / speed * sPitchMultiplier;
        }
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x0073b8a0
void Source3D::Properties::CopyParameters(const Properties& rOther)
{
    const bool bCentrePeak = (mFlags & flags_centrePeakMode) != 0;
    mRollInFactor = rOther.mRollInFactor;
    mRollInDistance = rOther.mRollInDistance;
    mMinDistance = rOther.mMinDistance;
    mMuteDistance = rOther.mMuteDistance;
    mMaxDistance = rOther.mMaxDistance;
    mRollOffFactor = rOther.mRollOffFactor;
    mSpaceDistance = rOther.mSpaceDistance;
    mFlags = rOther.mFlags;
    if (bCentrePeak)
    {
        mFlags |= flags_centrePeakMode;
    }
    else
    {
        mFlags &= (unsigned char)~(unsigned int)flags_centrePeakMode;
    }
}

// FUNCTION: SWBF2 0x0073b900
void Source3D::CopyParameters(const Source3D& rOther)
{
    mClosestListener = rOther.mClosestListener;
    mDistance = rOther.mDistance;
    mDistanceLength = rOther.mDistanceLength;
    mPos3D = rOther.mPos3D;
    mVel3D = rOther.mVel3D;
    mFlags |= (unsigned char)((rOther.mFlags & ~8u) | flags_updateListener);
    if (mProps && rOther.mProps)
    {
        mProps->CopyParameters(*rOther.mProps);
    }
}

// FUNCTION: SWBF2 0x0073b960
void Source3D::DetermineClosestListener()
{
    if ((mFlags & flags_updateListener) || mClosestListener == 0)
    {
        mClosestListener = static_cast<Listener*>(
            ListenerBase::GetClosestListener(mPos3D, &mDistance,
                                             &mDistanceLength));
    }
}

// FUNCTION: SWBF2 0x0073b990
bool Source3D::Properties::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    switch (uiPropertyID)
    {
    case 0x1CE8C2E1u:
        chunk._pFile->Read8();
        SetRollIn(chunk._pFile->ReadFloat());
        return true;
    case 0x0FBA43F2u:
        chunk._pFile->Read8();
        SetDopplerMode(chunk._pFile->ReadFloat() > 0.0f);
        return true;
    case 0x47C87922u:
        chunk._pFile->Read8();
        mSpaceDistance = chunk._pFile->ReadFloat();
        return true;
    case 0x80A163FEu:
        chunk._pFile->Read8();
        SetRollInDistance(chunk._pFile->ReadFloat());
        return true;
    case 0xAB59D4BBu:
        chunk._pFile->Read8();
        SetRollOff(chunk._pFile->ReadFloat());
        return true;
    case 0xB835CF50u:
        chunk._pFile->Read8();
        SetMinDistance(chunk._pFile->ReadFloat());
        return true;
    case 0xE4B12EDFu:
        chunk._pFile->Read8();
        SetMuteDistance(chunk._pFile->ReadFloat());
        return true;
    case 0xE6915CF6u:
        chunk._pFile->Read8();
        SetMaxDistance(chunk._pFile->ReadFloat());
        return true;
    }
    return false;
}

}
