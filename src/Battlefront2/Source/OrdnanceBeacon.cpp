#include "OrdnanceBeacon.h"

#include <stdio.h>

#include "Ordnance.h"
#include "GameObject.h"
#include "NetGame.h"
#include "StickInfo.h"
#include "PebbleFL/Common/PblRandom.h"

#include <d3dx9math.h>

extern bool netIsLocalTurn;

namespace
{

    inline GameObject* ResolveHandle(PblHandle<GameObject>& handle)
    {
        GameObject* pObject = handle.mObject;
        if (pObject &&
            pObject->mHandleId !=
                handle.mSavedHandleId)
        {
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
            pObject = 0;
        }
        return pObject;
    }
}

// SYNTHETIC: SWBF2 0x00405a10
// `dynamic initializer for 'OrdnanceBeacon::rttiHashOrdnanceBeacon''
// OrdnanceBeacon::rttiHashOrdnanceBeacon

// GLOBAL: SWBF2 0x01ef2610
// OrdnanceBeacon::rttiHashOrdnanceBeacon
PblHash OrdnanceBeacon::rttiHashOrdnanceBeacon("OrdnanceBeacon");
// GLOBAL: SWBF2 0x0078a78c
// OrdnanceBeacon::rttiNameOrdnanceBeacon
const char OrdnanceBeacon::rttiNameOrdnanceBeacon[] = "OrdnanceBeacon";

// SYNTHETIC: SWBF2 0x00405a20
// `dynamic initializer for 'OrdnanceBeaconClass::rttiHashOrdnanceBeaconClass''
// OrdnanceBeaconClass::rttiHashOrdnanceBeaconClass

// GLOBAL: SWBF2 0x01ef257c
// OrdnanceBeaconClass::rttiHashOrdnanceBeaconClass
PblHash OrdnanceBeaconClass::rttiHashOrdnanceBeaconClass("OrdnanceBeaconClass");
// GLOBAL: SWBF2 0x0078a768
// OrdnanceBeaconClass::rttiNameOrdnanceBeaconClass
const char OrdnanceBeaconClass::rttiNameOrdnanceBeaconClass[] = "OrdnanceBeaconClass";

// FUNCTION: SWBF2 0x005f8b10
const PblVector3& OrdnanceBullet::GetOldPosition() const
{
    return mOldPosition;
}

// FUNCTION: SWBF2 0x005f8b20
const PblVector3& OrdnanceBullet::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x005f8b60
float OrdnanceBulletClass::GetVelocity()
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x005f8b70
float OrdnanceBulletClass::GetRange()
{
    return mVelocity * mLifeSpan;
}

// FUNCTION: SWBF2 0x005f8be0
float OrdnanceShellClass::GetGravity()
{
    return mGravity;
}




// SYNTHETIC: SWBF2 0x005f95d6
// ??_EOrdnanceBeacon@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005f95e1
// ??_EOrdnanceBeacon@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005f8cd0
// OrdnanceBeacon::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f8d60
OrdnanceBeacon::OrdnanceBeacon(OrdnanceBeaconClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceSticky(pClass, desc)
{
    mSalvoCount = pClass->mOrdnanceClass ? pClass->mSalvoCount : 0;
}

// FUNCTION: SWBF2 0x005f8dc0
OrdnanceBeacon::~OrdnanceBeacon()
{
}

// FUNCTION: SWBF2 0x005f8de0
bool OrdnanceBeacon::Update(float fDeltaTime)
{
    while (mSalvoCount > 0 && fDeltaTime > mLifeTime)
    {
        PblMatrix aim;
        aim.SetRotateX(-static_cast<OrdnanceBeaconClass*>(mClass)->mAimElevation);
        aim.RotateY(static_cast<OrdnanceBeaconClass*>(mClass)->mAimAzimuth);

        OrdnanceDesc desc;
        desc.mPosition = mPosition;
        const float fDist = static_cast<OrdnanceBeaconClass*>(mClass)->mAimDistance;
        desc.mPosition.x += aim.m[2][0] * fDist;
        desc.mPosition.y += aim.m[2][1] * fDist;
        desc.mPosition.z += aim.m[2][2] * fDist;

        const float fRight = (PblRandom::GetSim().GetFloat() - PblRandom::GetSim().GetFloat()) *
                             static_cast<OrdnanceBeaconClass*>(mClass)->mScatterDistance;
        desc.mPosition.x += aim.m[0][0] * fRight;
        desc.mPosition.y += aim.m[0][1] * fRight;
        desc.mPosition.z += aim.m[0][2] * fRight;
        const float fUp = (PblRandom::GetSim().GetFloat() - PblRandom::GetSim().GetFloat()) *
                          static_cast<OrdnanceBeaconClass*>(mClass)->mScatterDistance;
        desc.mPosition.x += aim.m[1][0] * fUp;
        desc.mPosition.y += aim.m[1][1] * fUp;
        desc.mPosition.z += aim.m[1][2] * fUp;

        desc.mDirection = PblVector3(-aim.m[2][0], -aim.m[2][1], -aim.m[2][2]);
        desc.mStrength = 1.0f;
        desc.mOwner = ResolveHandle(mOwner);
        desc.mDamageOwner = mDamageOwner;
        desc.mTarget = mStickInfo ? ResolveHandle(mStickInfo->mCollHandle) : 0;
        desc.mBodyID = mStickInfo ? mStickInfo->mCollBodyID : -1;

        if (NetGame::IsEnabled() && !NetGame::IsClient())
        {
            NetGame::CreateEvent_CreateOrdnance(static_cast<OrdnanceBeaconClass*>(mClass)->mOrdnanceClass, desc);
        }
        if (!NetGame::IsEnabled() || !NetGame::IsClient() || netIsLocalTurn)
        {
            static_cast<OrdnanceBeaconClass*>(mClass)->mOrdnanceClass->Build(desc);
        }

        --mSalvoCount;
        mLifeTime += static_cast<OrdnanceBeaconClass*>(mClass)->mSalvoDelay;
    }
    return OrdnanceSticky::Update(fDeltaTime);
}







// FUNCTION: SWBF2 0x005f9270
OrdnanceBeaconClass::OrdnanceBeaconClass(unsigned int uHash)
    : OrdnanceStickyClass(uHash)
{
    mAimAzimuth = 0.0f;
    mAimElevation = PblMath::fHalfPi;
    mAimDistance = 100.0f;
    mSalvoCount = 1;
    mSalvoDelay = 0.0f;
    mScatterDistance = 0.0f;
    mOrdnanceClass = 0;
}

// SYNTHETIC: SWBF2 0x005f92e0
// OrdnanceBeaconClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f9310
OrdnanceBeaconClass::OrdnanceBeaconClass(OrdnanceBeaconClass* pBase, unsigned int uHash)
    : OrdnanceStickyClass(pBase, uHash)
{
    mAimAzimuth = pBase->mAimAzimuth;
    mAimElevation = pBase->mAimElevation;
    mAimDistance = pBase->mAimDistance;
    mSalvoCount = pBase->mSalvoCount;
    mSalvoDelay = pBase->mSalvoDelay;
    mScatterDistance = pBase->mScatterDistance;
    mOrdnanceClass = pBase->mOrdnanceClass;
}

// FUNCTION: SWBF2 0x005f9390
OrdnanceBeaconClass::~OrdnanceBeaconClass()
{
}

// FUNCTION: SWBF2 0x005f93a0
void OrdnanceBeaconClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x3186FBFE:
        if (sscanf(pszValue, "%f", &mAimAzimuth))
        {
            mAimAzimuth *= 0.017453292f;
        }
        break;
    case 0x8339FB35:
        if (sscanf(pszValue, "%f", &mAimElevation))
        {
            mAimElevation *= 0.017453292f;
        }
        break;
    case 0xECDB9D1D:
        sscanf(pszValue, "%f", &mAimDistance);
        break;
    case 0x9260EA63:
        sscanf(pszValue, "%i", &mSalvoCount);
        break;
    case 0xD76F37DB:
        sscanf(pszValue, "%f", &mSalvoDelay);
        break;
    case 0xEBD81A5E:
        sscanf(pszValue, "%f", &mScatterDistance);
        break;
    case 0x399BF05D:
    case 0xF8899C5E:
        mOrdnanceClass = Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Find(PblHash(pszValue));
        break;
    default:
        OrdnanceStickyClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x005f94d0
OrdnanceClass* OrdnanceBeaconClass::Derive(unsigned int uHash)
{
    return new OrdnanceBeaconClass(this, uHash);
}

// FUNCTION: SWBF2 0x005f9540
Ordnance* OrdnanceBeaconClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceBeacon* pBeacon = new OrdnanceBeacon(this, desc);
    pBeacon->ActivateThread(-40, PER_LOCAL_TURN);
    pBeacon->AddOrdnanceToCollisionManager();
    return pBeacon;
}
