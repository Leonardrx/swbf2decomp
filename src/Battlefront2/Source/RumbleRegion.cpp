#include <stdio.h>
#include <string.h>

#include "RumbleRegion.h"

#include "ChaseCamera.h"
#include "FLEffect.h"
#include "PebbleFL/Common/PblRandom.h"

// SYNTHETIC: SWBF2 0x00631377
// ??_ERumbleEffectClass@@WBI@AEPAXI@Z

// SYNTHETIC: SWBF2 0x00406220
// `dynamic initializer for 'RumbleEffectClass::s_RumbleEffectClassList''
// SYNTHETIC: SWBF2 0x00769460
// `dynamic atexit destructor for 'RumbleEffectClass::s_RumbleEffectClassList''

// GLOBAL: SWBF2 0x007ec288
// RumbleEffectClass::s_RumbleEffectClassList
ListPool<RumbleEffectClass*, int> RumbleEffectClass::s_RumbleEffectClassList(16);
// SYNTHETIC: SWBF2 0x00406270
// `dynamic initializer for 'RumbleRegionManager::sRegions''
// SYNTHETIC: SWBF2 0x00769470
// `dynamic atexit destructor for 'RumbleRegionManager::sRegions''

// GLOBAL: SWBF2 0x007ec298
// RumbleRegionManager::sRegions
ListPool<RumbleRegion*, int> RumbleRegionManager::sRegions(16);
// SYNTHETIC: SWBF2 0x004062c0
// `dynamic initializer for 'RumbleEffectClass::s_RumbleEffectList''
// SYNTHETIC: SWBF2 0x00769480
// `dynamic atexit destructor for 'RumbleEffectClass::s_RumbleEffectList''

// GLOBAL: SWBF2 0x007ec278
// RumbleEffectClass::s_RumbleEffectList
ListPool<RumbleEffect*, int> RumbleEffectClass::s_RumbleEffectList(32);

// SYNTHETIC: SWBF2 0x00406310
// `dynamic initializer for 'rumbleRandom''

// GLOBAL: SWBF2 0x01f3a1ac
// rumbleRandom
PblRandom rumbleRandom;

// FUNCTION: SWBF2 0x00630e90
RumbleEffectClass::RumbleEffectClass(unsigned int uHash)
    : Thread()
    , EntityClass(uHash)
{
}

// FUNCTION: SWBF2 0x00630ab0
RumbleRegionManager::RumbleRegionManager()
    : RedRegionFactory("rumble")
{
}

// FUNCTION: SWBF2 0x00630b00
RumbleRegionManager::~RumbleRegionManager()
{
}

// FUNCTION: SWBF2 0x00630a60
bool RumbleEffectClass::Update(float fDeltaTime)
{
    return true;
}

// FUNCTION: SWBF2 0x00630f00
EntityClass* RumbleEffectClass::Derive(unsigned int uHash)
{
    RumbleEffectClass* pClass = new RumbleEffectClass(uHash);
    s_RumbleEffectClassList.Append(pClass);
    return pClass;
}

// FUNCTION: SWBF2 0x00630f90
void RumbleEffectClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x5a4042b1: sscanf(pszValue, "%f", &mRumbleRegionDesc.mLight[0]); break;
    case 0x569c3e43: sscanf(pszValue, "%f", &mRumbleRegionDesc.mLight[1]); break;
    case 0x4456e84a: sscanf(pszValue, "%f", &mRumbleRegionDesc.mHeavy[0]); break;
    case 0x25779a5c: sscanf(pszValue, "%f", &mRumbleRegionDesc.mHeavy[1]); break;
    case 0xa4fbdbc8: sscanf(pszValue, "%f", &mRumbleRegionDesc.mHeavyDecay[0]); break;
    case 0xf440c23a: sscanf(pszValue, "%f", &mRumbleRegionDesc.mHeavyDecay[1]); break;
    case 0x1c5461b1: sscanf(pszValue, "%f", &mRumbleRegionDesc.mLightDecay[0]); break;
    case 0x78221603: sscanf(pszValue, "%f", &mRumbleRegionDesc.mLightDecay[1]); break;
    case 0xe1c681ca: sscanf(pszValue, "%f", &mRumbleRegionDesc.mDelayLight[0]); break;
    case 0xe2742820: sscanf(pszValue, "%f", &mRumbleRegionDesc.mDelayLight[1]); break;
    case 0x1cb9e68d: sscanf(pszValue, "%f", &mRumbleRegionDesc.mDelayHeavy[0]); break;
    case 0x7189dcc3: sscanf(pszValue, "%f", &mRumbleRegionDesc.mDelayHeavy[1]); break;
    case 0xbf6413ae: sscanf(pszValue, "%f", &mRumbleRegionDesc.mTimeLeftHeavy[0]); break;
    case 0x220abe04: sscanf(pszValue, "%f", &mRumbleRegionDesc.mTimeLeftHeavy[1]); break;
    case 0x4104158d: sscanf(pszValue, "%f", &mRumbleRegionDesc.mTimeLeftLight[0]); break;
    case 0x931a6a3b: sscanf(pszValue, "%f", &mRumbleRegionDesc.mTimeLeftLight[1]); break;
    case 0xf8c69a26: sscanf(pszValue, "%f", &mRumbleRegionDesc.mInitialTime[0]); break;
    case 0x8b37cab4: sscanf(pszValue, "%f", &mRumbleRegionDesc.mInitialTime[1]); break;
    case 0x4fb4269f: sscanf(pszValue, "%f", &mRumbleRegionDesc.mShakeAmt[0]); break;
    case 0xc3e9c081: sscanf(pszValue, "%f", &mRumbleRegionDesc.mShakeAmt[1]); break;
    case 0xcff18812: sscanf(pszValue, "%f", &mRumbleRegionDesc.mShakeLen[0]); break;
    case 0x626eeba0: sscanf(pszValue, "%f", &mRumbleRegionDesc.mShakeLen[1]); break;
    case 0xb5234678:
    {
        char name[32];
        sscanf(pszValue, "%s", name);
        mRumbleRegionDesc.mParticleID = PblHash(name);
        break;
    }
    case 0x7a5ff18f:
    {
        char name[32];
        sscanf(pszValue, "%s", name);
        mRumbleRegionDesc.mRumbleSound.SetID(PblHash(name), false, false);
        break;
    }
    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x006311e0
Entity* RumbleEffectClass::Build(const EntityDesc& desc)
{
    RumbleEffect* pEffect = new RumbleEffect(this, desc.mMatrix);
    pEffect->mNameHash = desc.mNameId;
    pEffect->mEffectHash = mRumbleRegionDesc.mParticleID;
    s_RumbleEffectList.Append(pEffect);
    return pEffect;
}

// FUNCTION: SWBF2 0x00631240
RumbleEffectClass* RumbleEffectClass::FindRumbleEffectClass(unsigned int uHash)
{
    for (int i = 0; i < s_RumbleEffectClassList.GetLength(); ++i)
    {
        if (s_RumbleEffectClassList[i]->mId == uHash)
        {
            return s_RumbleEffectClassList[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00631270
void RumbleEffectClass::StartEffects(unsigned int uNameHash)
{
    for (int i = 0; i < s_RumbleEffectList.GetLength(); ++i)
    {
        RumbleEffect* pEffect = s_RumbleEffectList[i];
        if (pEffect->mNameHash != uNameHash)
        {
            continue;
        }
        FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(pEffect->mEffectHash);
        if (!pClass)
        {
            continue;
        }
        FLEffectObject* pObject = pClass->BuildEffect();
        if (pObject)
        {
            pObject->AttachEffectToMatrix(&pEffect->mMatrix, false);
        }
    }
}

// FUNCTION: SWBF2 0x006312d0
void RumbleEffectClass::Reset()
{
    s_RumbleEffectClassList.Clear();
    s_RumbleEffectList.Clear();
}

// FUNCTION: SWBF2 0x00631300
RumbleEffect::RumbleEffect(RumbleEffectClass* pClass, const PblMatrix& matrix)
{
    mMatrix = matrix;
}

// FUNCTION: SWBF2 0x00631370
RumbleEffect::~RumbleEffect()
{
}

// FUNCTION: SWBF2 0x00630b10
void RumbleRegionManager::Reset()
{
    RumbleEffectClass::Reset();
    sRegions.Clear();
}

// FUNCTION: SWBF2 0x00630b30
void* RumbleRegionManager::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    RumbleRegion* pRegion = new RumbleRegion(desc);

    char format[512];
    strcpy(format, GetPrefix());
    strcat(format, " %s %s");

    char rumbleName[64] = "";
    char effectName[64] = "";
    sscanf(pcName, format, rumbleName, effectName);

    pRegion->mCurRumbleID = PblHash(rumbleName);
    pRegion->mEffectHash = PblHash(effectName);
    pRegion->m_pREClass = RumbleEffectClass::FindRumbleEffectClass(pRegion->mCurRumbleID);
    if (!pRegion->m_pREClass)
    {
        return 0;
    }
    if (sRegions.IsFull())
    {
        return 0;
    }

    sRegions.Append(pRegion);
    pRegion->mTimer = -1.0f;
    return pRegion;
}

// FUNCTION: SWBF2 0x00630ca0
void RumbleRegionManager::Update(float fDeltaTime)
{
    for (int i = 0; i < sRegions.GetLength(); ++i)
    {
        RumbleRegion* pRegion = sRegions[i];
        if (pRegion->mTimer < 0.0f)
        {
            const float r = rumbleRandom.GetFloat();
            const RumbleRegionDesc& desc = pRegion->m_pREClass->mRumbleRegionDesc;
            pRegion->mTimer = (1.0f - r) * desc.mInitialTime[1] + desc.mInitialTime[0] * r;
        }
        else
        {
            pRegion->mTimer -= fDeltaTime;
        }
    }
}

// FUNCTION: SWBF2 0x00630d60
void RumbleRegionManager::CheckForRumble(unsigned int uPlayer, const PblVector3& vPos,
                                         ChaseCamera* pCamera, float fDist)
{
    for (int i = 0; i < sRegions.GetLength(); ++i)
    {
        RumbleRegion* pRegion = sRegions[i];
        if (pRegion->mTimer <= 0.0f && pRegion->IsPointInside(vPos))
        {
            const RumbleRegionDesc& desc = pRegion->m_pREClass->mRumbleRegionDesc;
            const float r = rumbleRandom.GetFloat();
            const float fAmount = (1.0f - r) * desc.mShakeAmt[1] + desc.mShakeAmt[0] * r;
            const float fLength = (1.0f - r) * desc.mShakeLen[1] + desc.mShakeLen[0] * r;
            pCamera->ApplyShake(fAmount, fLength);
            desc.mRumbleSound.Play(&vPos, 0, false, true);
            if (pRegion->mEffectHash)
            {
                RumbleEffectClass::StartEffects(pRegion->mEffectHash);
            }
            return;
        }
    }
}
