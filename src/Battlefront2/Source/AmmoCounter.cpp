#include "AmmoCounter.h"

#include <float.h>
#include <limits.h>
#include <stdio.h>

#include "RedEngineFL/Memory/RedMemoryPool.h"


 

// SYNTHETIC: SWBF2 0x00402280
// `dynamic initializer for 'AmmoCounter::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766d70
// `dynamic atexit destructor for 'AmmoCounter::sMemoryPool''

// GLOBAL: SWBF2 0x01eb04b0
// AmmoCounter::sMemoryPool
MemoryPool AmmoCounter::sMemoryPool("AmmoCounter", sizeof(AmmoCounter));

// FUNCTION: SWBF2 0x00443680
bool AmmoCounterClass::SetProperty(unsigned int uiHash, const char* pcValue)
{
    if (uiHash != 0x7bff9ec9)
    {
        return false;
    }

    if (sscanf(pcValue, "%d", &mRoundsPerClip))
    {
        if (mRoundsPerClip > 0)
        {
            mAmmoPerRound = 1.0f / ((float)mRoundsPerClip + 0.0001f);
            return true;
        }
        mRoundsPerClip = 0x7fffffff;
        mAmmoPerRound = 0.0f;
    }
    return true;
}

// FUNCTION: SWBF2 0x00443790
int AmmoCounter::GetMaxNumRounds() const
{
    if (m_pClass->mRoundsPerClip == INT_MAX || mMaxClips == FLT_MAX)
    {
        return INT_MAX;
    }
    return (int)((float)m_pClass->mRoundsPerClip * mMaxClips);
}

// FUNCTION: SWBF2 0x004437d0
int AmmoCounter::GetNumRounds() const
{
    if (m_pClass->mRoundsPerClip == INT_MAX || mNumClips == FLT_MAX)
    {
        return INT_MAX;
    }
    return (int)((mCurClip + mNumClips) * (float)m_pClass->mRoundsPerClip);
}

// FUNCTION: SWBF2 0x00443810
bool AmmoCounter::CanExpendRounds(float fRounds) const
{
    return mCurClip >= m_pClass->mAmmoPerRound * fRounds;
}

// FUNCTION: SWBF2 0x00443830
void AmmoCounter::ExpendRounds(float fRounds)
{
    mCurClip -= m_pClass->mAmmoPerRound * fRounds;
    if (m_pClass->mAmmoPerRound > mCurClip)
    {
        mCurClip = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00443860
void AmmoCounter::AddClips(float fClips)
{
    float fMaxClips = mMaxClips - (float)!IsClipEmpty();
    mNumClips += fClips;
    if (mNumClips > fMaxClips)
    {
        mNumClips = fMaxClips;
    }
}

// FUNCTION: SWBF2 0x004438a0
void AmmoCounter::UnloadClip()
{
    if (mNumClips != FLT_MAX)
    {
        mNumClips += mCurClip;
    }
    mCurClip = 0.0f;
}

// FUNCTION: SWBF2 0x00443920
void AmmoCounter::UnloadClip_Unlimitted()
{
    mCurClip = 0.0f;
}

// FUNCTION: SWBF2 0x00443930
void AmmoCounter::LoadClip_Unlimitted()
{
    mCurClip = 1.0f;
}

// FUNCTION: SWBF2 0x004436f0
AmmoCounter::AmmoCounter(const AmmoCounterClass* pClass, int iNumClips)
{
    m_pClass = pClass;
    float fClips = (iNumClips == 0x7fffffff) ? FLT_MAX : (float)iNumClips;
    mDefaultMaxClips = fClips;
    mMaxClips = fClips;
    m_uiRefCount = 1;
    mNumClips = fClips;
    mCurClip = 0.0f;
    LoadClip();
}

// FUNCTION: SWBF2 0x00443750
AmmoCounter* AmmoCounter::AddRef()
{
    if (!m_uiRefCount)
    {
        return 0;
    }
    ++m_uiRefCount;
    return this;
}

// FUNCTION: SWBF2 0x004438d0
void AmmoCounter::LoadClip()
{
    if (mNumClips == FLT_MAX)
    {
        mCurClip = 1.0f;
    }
    else
    {
        float fClip = mCurClip + mNumClips;
        if (fClip >= 1.0f)
        {
            mNumClips -= 1.0f - mCurClip;
            mCurClip = 1.0f;
        }
        else
        {
            mCurClip = fClip;
            mNumClips = 0.0f;
        }
    }
}

// FUNCTION: SWBF2 0x00443740
AmmoCounter::~AmmoCounter()
{
    m_uiRefCount = 0;
}

// FUNCTION: SWBF2 0x00443760
unsigned int AmmoCounter::Release()
{
    if (!m_uiRefCount)
    {
        return 0;
    }
    if (m_uiRefCount == 1)
    {
        this->~AmmoCounter();
        sMemoryPool.Free(this);
        return 0;
    }
    return --m_uiRefCount;
}
