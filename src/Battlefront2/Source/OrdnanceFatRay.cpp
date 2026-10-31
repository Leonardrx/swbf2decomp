#include "OrdnanceFatRay.h"

#include <math.h>
#include <stdio.h>

#include "CollisionManager.h"
#include "GameObject.h"

// SYNTHETIC: SWBF2 0x00405a80
// `dynamic initializer for 'OrdnanceFatRay::rttiHashOrdnanceFatRay''
// OrdnanceFatRay::rttiHashOrdnanceFatRay

// GLOBAL: SWBF2 0x01ef262c
// OrdnanceFatRay::rttiHashOrdnanceFatRay
PblHash OrdnanceFatRay::rttiHashOrdnanceFatRay("OrdnanceFatRay");
// GLOBAL: SWBF2 0x0078a850
// OrdnanceFatRay::rttiNameOrdnanceFatRay
const char OrdnanceFatRay::rttiNameOrdnanceFatRay[] = "OrdnanceFatRay";

// FUNCTION: SWBF2 0x005fe800
OrdnanceFatRay::OrdnanceFatRay(OrdnanceFatRayClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc)
{
    mDamage = (pClass->mMaxDamage - pClass->mMinDamage) * desc.mStrength + pClass->mMinDamage;
    mPush = (pClass->mMaxPush - pClass->mMinPush) * desc.mStrength + pClass->mMinPush;
}

// FUNCTION: SWBF2 0x005fe880
OrdnanceFatRay::~OrdnanceFatRay()
{
}

// SYNTHETIC: SWBF2 0x005ff076
// ??_EOrdnanceFatRay@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005ff07e
// ??_EOrdnanceFatRay@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005fe780
// OrdnanceFatRay::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005fe8a0
bool OrdnanceFatRay::CheckCollision()
{
    OrdnanceFatRayClass* pClass = (OrdnanceFatRayClass*)mClass;
    const PblVector3 vOld = GetOldPosition();
    const PblVector3 vCur = GetPosition();
    const PblVector3 vDelta = vCur - vOld;
    const float fLength = Length(vDelta);
    if (!(fLength > 0.0f))
    {
        return true;
    }

    const PblVector3 vDir = vDelta / fLength;

    GameObject* pObjects[32];
    int iCount = CollisionManager::GetObjectsInRange(vDir * (fLength * 0.5f) + vOld, pClass->mRayRadius + fLength * 0.5f, pObjects, 32,
                                                     GetOwner(), 0x1022);
    while (iCount)
    {
        GameObject* pObject = pObjects[--iCount];
        const PblVector3 vTarget = pObject->GetTargetPoint(vOld, vDir, -1);
        const PblVector3 vFromOld = vTarget - vOld;
        const float fAlong = DotProduct(vFromOld, vDir);
        if (!(fAlong > 0.0f))
        {
            continue;
        }
        const PblVector3 vFromCur = vTarget - vCur;
        if (!(DotProduct(vFromCur, vDir) < 0.0f))
        {
            continue;
        }

        const PblVector3 vOffset = vTarget - (vDir * fAlong + vOld);
        const float fOffSq = SquaredLength(vOffset);
        if (!(pClass->mRayRadius * pClass->mRayRadius > fOffSq))
        {
            continue;
        }

        const float fOff = (float)sqrt((double)fOffSq);
        const float fScale = 1.0f - fOff / pClass->mRayRadius;

        if (mDamage > 0.0f)
        {
            DamageDesc damage(mDamageOwner, mDamage * fScale);
            if (!pObject->Deflect(this, vTarget, vDir))
            {
                pObject->ApplyDamage(damage, 0xffffffffu);
            }
            mDamage -= pClass->mDamageDeduction * fScale;
            if (pClass->mMinDamage > mDamage)
            {
                mDamage = pClass->mMinDamage;
            }
        }

        if (mPush > 0.0f)
        {
            const float fPush = mPush;
            const float fInvOff = 1.0f / fOff;
            const float fOutward = (1.0f - fScale) * 0.5f;
            const PblVector3 vSide(vOffset.x * fInvOff, 0.0f, vOffset.z * fInvOff);
            const PblVector3 vPush = vSide * (fPush * fOutward) + vDir * (1.0f - fOutward) * fPush;
            pObject->ApplyPush(vPush, 1.0f, false);
            const float fWorn = mPush - pClass->mPushDeduction;
            mPush = pClass->mMinPush > fWorn ? fWorn : pClass->mMinPush;
        }
    }
    return true;
}







// FUNCTION: SWBF2 0x005fef70
OrdnanceClass* OrdnanceFatRayClass::Derive(unsigned int uHash)
{
    return new OrdnanceFatRayClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x00405a90
// `dynamic initializer for 'OrdnanceFatRayClass::rttiHashOrdnanceFatRayClass''
// OrdnanceFatRayClass::rttiHashOrdnanceFatRayClass

// GLOBAL: SWBF2 0x01ef2628
// OrdnanceFatRayClass::rttiHashOrdnanceFatRayClass
PblHash OrdnanceFatRayClass::rttiHashOrdnanceFatRayClass("OrdnanceFatRayClass");
// GLOBAL: SWBF2 0x0078a830
// OrdnanceFatRayClass::rttiNameOrdnanceFatRayClass
const char OrdnanceFatRayClass::rttiNameOrdnanceFatRayClass[] = "OrdnanceFatRayClass";

// FUNCTION: SWBF2 0x005feda0
OrdnanceFatRayClass::OrdnanceFatRayClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mLifeSpan = 3.0f;
    mStraightBullet = true;
    mVelocity = 35.0f;

    mMaxDamage = 20.0f;
    mMinDamage = 10.0f;
    mDamageDeduction = 5.0f;
    mMaxPush = 15.0f;
    mMinPush = 5.0f;
    mPushDeduction = 3.0f;
    mRayRadius = 3.0f;
}

// FUNCTION: SWBF2 0x005fee50
OrdnanceFatRayClass::OrdnanceFatRayClass(OrdnanceFatRayClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mMaxDamage = pBase->mMaxDamage;
    mMinDamage = pBase->mMinDamage;
    mDamageDeduction = pBase->mDamageDeduction;
    mMaxPush = pBase->mMaxPush;
    mMinPush = pBase->mMinPush;
    mPushDeduction = pBase->mPushDeduction;
    mRayRadius = pBase->mRayRadius;
}







// FUNCTION: SWBF2 0x005feee0
void OrdnanceFatRayClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x47D0E794:
    {
        sscanf(pszValue, "%f", &mMaxDamage);
            break;
    }
    case 0xFE544F96:
    {
        sscanf(pszValue, "%f", &mMinDamage);
            break;
    }
    case 0xAF922A6B:
    {
        sscanf(pszValue, "%f", &mDamageDeduction);
            break;
    }
    case 0xAEE05D99:
    {
        sscanf(pszValue, "%f", &mMaxPush);
            break;
    }
    case 0x3ED11AC3:
    {
        sscanf(pszValue, "%f", &mMinPush);
            break;
    }
    case 0x58A7DA24:
    {
        sscanf(pszValue, "%f", &mPushDeduction);
            break;
    }
    case 0xA4C53EFD:
    {
        sscanf(pszValue, "%f", &mRayRadius);
            break;
    }
    default:
    {
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
            break;
    }
    }
}

// FUNCTION: SWBF2 0x005fefe0
Ordnance* OrdnanceFatRayClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceFatRay* pRay = new OrdnanceFatRay(this, desc);
    pRay->ActivateThread(-40, PER_LOCAL_TURN);
    pRay->AddOrdnanceToCollisionManager();
    return pRay;
}

// SYNTHETIC: SWBF2 0x005fee20
// OrdnanceFatRayClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005feed0
OrdnanceFatRayClass::~OrdnanceFatRayClass()
{
}
