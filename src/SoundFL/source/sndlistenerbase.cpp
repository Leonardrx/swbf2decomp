#include <float.h>
#include <math.h>

#include "sndlistenerbase.h"
#include "sndspace.h"
#include "sndenginebase.h"
#include "win/sndlistener.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00731440
ListenerBase::ListenerBase()
{
    mFlags = flags_reverbUpdate;
    mPos = &Globals::smNullVector;
    mSpacePos = &Globals::smNullVector;
    mFront = &Globals::smFrontVector;
    mUp = &Globals::smUpVector;
    mVel = &Globals::smNullVector;
    mDopplerFactor = 1.0f;
    mRollOffFactor = 10.0f;
    mDistanceFactor = 1.0f;
    mSpace = &Space::smDefaultSpace;
    mReverb = 0;
}

// FUNCTION: SWBF2 0x00731490
ListenerBase::~ListenerBase()
{
    if (mSpace)
    {
        mSpace = &Space::smDefaultSpace;
        I3DL2ReverbPreset* pReverb = (I3DL2ReverbPreset*)Space::smDefaultSpace.mReverbPreset;
        if (pReverb != mReverb)
        {
            if (pReverb)
            {
                mFlags |= flags_reverbChanged;
            }
            mReverb = pReverb;
        }
    }
    if (mReverb)
    {
        mReverb = 0;
    }
}


// GLOBAL: SWBF2 0x009cf4a0
unsigned int ListenerBase::smActiveListeners;

// FUNCTION: SWBF2 0x007314c0
ListenerBase* ListenerBase::GetClosestListener(const ::PblVector3* pPos,
                                               ::PblVector3* pDistance,
                                               float* pfDistanceLength)
{
    ListenerBase* pClosest = 0;

    float fDistanceLength = FLT_MAX;
    ::PblVector3 vDistance;

    if (smActiveListeners > 1)
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            ListenerBase& rListener = EngineBase::smListeners[i];
            if (!(rListener.mFlags & flags_enabled))
            {
                continue;
            }

            ::PblVector3 v = *pPos - *rListener.mPos;
            float fLength2 = SquaredLength(v);

            if (fDistanceLength > fLength2)
            {
                vDistance = v;
                fDistanceLength = fLength2;
                pClosest = &rListener;
            }
        }
    }

    if (pClosest == 0)
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            ListenerBase& rListener = EngineBase::smListeners[i];
            if (rListener.mFlags & flags_enabled)
            {
                vDistance = *pPos - *rListener.mPos;
                fDistanceLength = SquaredLength(vDistance);
                pClosest = &rListener;
                break;
            }
        }
    }

    if (pClosest != 0 && pfDistanceLength != 0)
    {
        fDistanceLength = (float)sqrt(fDistanceLength);
    }

    if (pDistance != 0)
    {
        *pDistance = vDistance;
    }

    if (pfDistanceLength != 0)
    {
        *pfDistanceLength = fDistanceLength;
    }

    return pClosest;
}

// FUNCTION: SWBF2 0x00731750
void ListenerBase::SetI3DL2ReverbPreset(const I3DL2ReverbPreset* pReverb)
{
    if (pReverb != mReverb)
    {
        if (pReverb)
        {
            mFlags |= flags_reverbChanged;
        }
        mReverb = const_cast<I3DL2ReverbPreset*>(pReverb);
    }
}

// FUNCTION: SWBF2 0x00731770
void ListenerBase::SetSpace(const Space* pSpace)
{
    if (pSpace != mSpace)
    {
        if (pSpace)
        {
            mFlags |= flags_spaceChanged;
        }
        else
        {
            pSpace = &Space::smDefaultSpace;
        }
        mSpace = const_cast<Space*>(pSpace);
        if (mSpace)
        {
            SetI3DL2ReverbPreset((const I3DL2ReverbPreset*)mSpace->mReverbPreset);
        }
    }
}

// FUNCTION: SWBF2 0x007317b0
void ListenerBase::Update()
{
    mFlags &= ~(flags_spaceChanged | flags_reverbChanged);

    Space* const pSpace = Space::GetSpace(*mSpacePos);
    if (mSpace == 0 || pSpace == &Space::smDefaultSpace
        || !mSpace->IsPointInside(*mSpacePos, 0.0f)
        || pSpace->mPriority > mSpace->mPriority)
    {
        SetSpace(pSpace);
    }
}

}
