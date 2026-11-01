#include "OrbitalStrike.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblRandom.h"

#include <d3dx9math.h>
#include <stdio.h>

extern bool netIsLocalTurn;

// FUNCTION: SWBF2 0x005f6f80
OrbitalStrikeClass::OrbitalStrikeClass(unsigned int uHash)
{
    (void)uHash;

    mAimAzimuth = 0.0f;
    mAimElevation = PblMath::fHalfPi;
    mAimDistance = 100.0f;
    mSalvoCount = 1;
    mSalvoDelay = 0.0f;
    mInitialSalvoDelay = 0.0f;
    mScatterDistance = 0.0f;
    mStrikeClass = 0;
}

// FUNCTION: SWBF2 0x005f6fd0
OrbitalStrikeClass::OrbitalStrikeClass(OrbitalStrikeClass* pBase, unsigned int uHash)
{
    (void)uHash;
    mAimAzimuth = pBase->mAimAzimuth;
    mAimElevation = pBase->mAimElevation;
    mAimDistance = pBase->mAimDistance;
    mSalvoCount = pBase->mSalvoCount;
    mSalvoDelay = pBase->mSalvoDelay;
    mInitialSalvoDelay = pBase->mInitialSalvoDelay;
    mScatterDistance = pBase->mScatterDistance;
    mStrikeClass = pBase->mStrikeClass;
}

// FUNCTION: SWBF2 0x005f6aa0
OrbitalStrike::OrbitalStrike(const OrbitalStrikeClass* pClass, OrbitalStrikeDesc& desc)
    : Thread(), PblHandled(), mClass(const_cast<OrbitalStrikeClass*>(pClass)),
      mPosition(desc.mPosition), mDamageOwner(desc.mDamageOwner)
{
    mSalvoCount = mClass->mSalvoCount;
    mSalvoDelay = mClass->mInitialSalvoDelay;
    ActivateThread(-40, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x005f6b80
OrbitalStrike::~OrbitalStrike()
{
}

// SYNTHETIC: SWBF2 0x005f6b50
// OrbitalStrike::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005f7193
// ??_EOrbitalStrike@@WBI@AEPAXI@Z

// FUNCTION: SWBF2 0x005f6ba0
bool OrbitalStrike::Update(float fDeltaTime)
{
    while (fDeltaTime > 0.00012207031f)
    {
        if (mSalvoCount <= 0)
        {
            return false;
        }

        while (mSalvoCount > 0 && mSalvoDelay <= 0.0f)
        {
            PblMatrix aim;
            aim.SetRotateX(-mClass->mAimElevation);
            aim.RotateY(mClass->mAimAzimuth);
            aim.SetTranslation(mPosition);

            OrdnanceDesc desc;
            const PblVector3 offset((PblRandom::GetSim().GetFloat() - PblRandom::GetSim().GetFloat()) * mClass->mScatterDistance,
                                    (PblRandom::GetSim().GetFloat() - PblRandom::GetSim().GetFloat()) * mClass->mScatterDistance,
                                    mClass->mAimDistance);
            desc.mPosition = aim.ApplyTransform(offset);
            desc.mDirection = PblVector3(-aim.m[2][0], -aim.m[2][1], -aim.m[2][2]);
            desc.mStrength = 1.0f;
            desc.mOwner = 0;
            desc.mDamageOwner = mDamageOwner;
            desc.mTarget = 0;
            desc.mBodyID = -1;
            desc.mFireWeapon = 0;

            if (NetGame::IsEnabled() && NetGame::IsClient())
            {
                if (netIsLocalTurn)
                {
                    mClass->mStrikeClass->Build(desc);
                }
            }
            else
            {
                if (NetGame::IsEnabled())
                {
                    NetGame::CreateEvent_CreateOrdnance(mClass->mStrikeClass, desc);
                }
                mClass->mStrikeClass->Build(desc);
            }

            --mSalvoCount;
            mSalvoDelay += mClass->mSalvoDelay;
        }

        const float fStep = (mSalvoDelay > fDeltaTime) ? fDeltaTime : mSalvoDelay;
        fDeltaTime -= fStep;
        mSalvoDelay -= fStep;
    }
    return true;
}

// TEMPLATE: SWBF2 0x005f7140
// Factory<Ordnance,OrdnanceClass,OrdnanceDesc>::Find
template OrdnanceClass* Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Find(unsigned int);

// FUNCTION: SWBF2 0x005f7010
bool OrbitalStrikeClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x3186FBFE:
        if (sscanf(pszValue, "%f", &mAimAzimuth))
        {
            mAimAzimuth *= 0.017453292f;
        }
        return true;
    case 0x8339FB35:
        if (sscanf(pszValue, "%f", &mAimElevation))
        {
            mAimElevation *= 0.017453292f;
        }
        return true;
    case 0xECDB9D1D:
        sscanf(pszValue, "%f", &mAimDistance);
        return true;
    case 0x9260EA63:
        sscanf(pszValue, "%i", &mSalvoCount);
        return true;
    case 0xD76F37DB:
        sscanf(pszValue, "%f", &mSalvoDelay);
        return true;
    case 0xC8388D9D:
        sscanf(pszValue, "%f", &mInitialSalvoDelay);
        return true;
    case 0xEBD81A5E:
        sscanf(pszValue, "%f", &mScatterDistance);
        return true;
    case 0x6FD92BE8:
    case 0x0E52B6A3:
        mStrikeClass = Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Find(PblHash(pszValue));
        return true;
    }
    return false;
}

