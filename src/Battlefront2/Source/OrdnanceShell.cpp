#include "OrdnanceShell.h"

#include <stdio.h>

#include "RedEngineFL/Application/RedWater.h"

// SYNTHETIC: SWBF2 0x00405b60
// `dynamic initializer for 'OrdnanceShell::rttiHashOrdnanceShell''
// OrdnanceShell::rttiHashOrdnanceShell

// GLOBAL: SWBF2 0x01ef2660
// OrdnanceShell::rttiHashOrdnanceShell
PblHash OrdnanceShell::rttiHashOrdnanceShell("OrdnanceShell");
// GLOBAL: SWBF2 0x0078a9c4
// OrdnanceShell::rttiNameOrdnanceShell
const char OrdnanceShell::rttiNameOrdnanceShell[] = "OrdnanceShell";

// SYNTHETIC: SWBF2 0x00405b70
// `dynamic initializer for 'OrdnanceShellClass::rttiHashOrdnanceShellClass''
// OrdnanceShellClass::rttiHashOrdnanceShellClass

// GLOBAL: SWBF2 0x01ef2664
// OrdnanceShellClass::rttiHashOrdnanceShellClass
PblHash OrdnanceShellClass::rttiHashOrdnanceShellClass("OrdnanceShellClass");
// GLOBAL: SWBF2 0x0078a9e8
// OrdnanceShellClass::rttiNameOrdnanceShellClass
const char OrdnanceShellClass::rttiNameOrdnanceShellClass[] = "OrdnanceShellClass";

// FUNCTION: SWBF2 0x00605d40
OrdnanceShellClass::OrdnanceShellClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mLifeSpan = 3.0f;
    mStraightBullet = false;
    mVelocity = 35.0f;

    mGravity = -9.8f;
}

// FUNCTION: SWBF2 0x00605db0
OrdnanceShellClass::OrdnanceShellClass(OrdnanceShellClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mGravity = pBase->mGravity;
}

// FUNCTION: SWBF2 0x00605e40
OrdnanceClass* OrdnanceShellClass::Derive(unsigned int uHash)
{
    return new OrdnanceShellClass(this, uHash);
}

// FUNCTION: SWBF2 0x00605eb0
Ordnance* OrdnanceShellClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceShell* pShell = new OrdnanceShell(this, desc);
    pShell->ActivateThread(-40, PER_LOCAL_TURN);
    pShell->AddOrdnanceToCollisionManager();
    return pShell;
}







// FUNCTION: SWBF2 0x00605df0
void OrdnanceShellClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty != 0xD209FF45 /* "Gravity" */)
    {
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
    }
    else
    {
        sscanf(pszValue, "%f", &mGravity);
        mGravity *= -9.8f;
    }
}

// SYNTHETIC: SWBF2 0x00605d80
// OrdnanceShellClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00605bb0
OrdnanceShell::OrdnanceShell(OrdnanceShellClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc)
{
    OrdnanceShellClass* pShellClass = (OrdnanceShellClass*)mClass;
    mGravity = (((OrdnanceShellClass*)pShellClass->mNextCharge)->mGravity - pShellClass->mGravity) * mRatio
             + pShellClass->mGravity;
}

// FUNCTION: SWBF2 0x00605c10
OrdnanceShell::~OrdnanceShell()
{
}

// SYNTHETIC: SWBF2 0x00605f46
// ??_EOrdnanceShell@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00605f4e
// ??_EOrdnanceShell@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00605b60
// OrdnanceShell::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00605c30
bool OrdnanceShell::Update(float fDeltaTime)
{
    float fWaterHeight = 0.0f;
    if (RedWater::GetWaterHeight(mPosition, fWaterHeight) && fWaterHeight > mPosition.y)
    {
        float fDrag = fDeltaTime * 0.7f;
        fDrag = fDrag / (fDrag + 1.0f);
        mVelocity -= PblVector3(mVelocity.x, mVelocity.y - -0.5f, mVelocity.z) * fDrag;
        if (!OrdnanceBullet::Update(fDeltaTime))
        {
            return false;
        }
    }
    else
    {
        mVelocity.y += mGravity * fDeltaTime;
        if (!OrdnanceBullet::Update(fDeltaTime))
        {
            return false;
        }
    }

    return true;
}







// FUNCTION: SWBF2 0x00605de0
OrdnanceShellClass::~OrdnanceShellClass()
{
}
