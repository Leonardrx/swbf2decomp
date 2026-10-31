#include "OrdnanceHaywire.h"
#include "GameObject.h"
#include "StickInfo.h"

#include <stdio.h>







// FUNCTION: SWBF2 0x00600e40
OrdnanceHaywire::OrdnanceHaywire(OrdnanceHaywireClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceSticky(pClass, desc)
{
}

// FUNCTION: SWBF2 0x00600e70
OrdnanceHaywire::~OrdnanceHaywire()
{
}

// SYNTHETIC: SWBF2 0x006010e6
// ??_EOrdnanceHaywire@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x006010ee
// ??_EOrdnanceHaywire@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00600db0
// OrdnanceHaywire::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00600e90
bool OrdnanceHaywire::Update(float fDeltaTime)
{
    if (!OrdnanceSticky::Update(fDeltaTime))
    {
        auto* pStick = mStickInfo;
        if (pStick && pStick->mCollHandle.mObject)
        {
            GameObject* pObject = pStick->mCollHandle.mObject;
            if (pObject->mHandleId == pStick->mCollHandle.mSavedHandleId)
            {
                pObject->mDisableTime = ((OrdnanceHaywireClass*)mClass)->mDisableTime;
            }
            else
            {
                pStick->mCollHandle.mObject = 0;
                pStick->mCollHandle.mSavedHandleId = 0;
            }
        }
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x00601050
Ordnance* OrdnanceHaywireClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.IsFull())
    {
        return 0;
    }
    OrdnanceHaywire* pOrdnance = new OrdnanceHaywire(this, desc);
    pOrdnance->ActivateThread(-40, PER_LOCAL_TURN);
    pOrdnance->AddOrdnanceToCollisionManager();
    return pOrdnance;
}

// SYNTHETIC: SWBF2 0x00405ae0
// `dynamic initializer for 'OrdnanceHaywire::rttiHashOrdnanceHaywire''
// OrdnanceHaywire::rttiHashOrdnanceHaywire

// GLOBAL: SWBF2 0x01ef2644
// OrdnanceHaywire::rttiHashOrdnanceHaywire
PblHash OrdnanceHaywire::rttiHashOrdnanceHaywire("OrdnanceHaywire");
// GLOBAL: SWBF2 0x0078a8fc
// OrdnanceHaywire::rttiNameOrdnanceHaywire
const char OrdnanceHaywire::rttiNameOrdnanceHaywire[] = "OrdnanceHaywire";

// SYNTHETIC: SWBF2 0x00405af0
// `dynamic initializer for 'OrdnanceHaywireClass::rttiHashOrdnanceHaywireClass''
// OrdnanceHaywireClass::rttiHashOrdnanceHaywireClass

// GLOBAL: SWBF2 0x01ef2640
// OrdnanceHaywireClass::rttiHashOrdnanceHaywireClass
PblHash OrdnanceHaywireClass::rttiHashOrdnanceHaywireClass("OrdnanceHaywireClass");
// GLOBAL: SWBF2 0x0078a8e4
// OrdnanceHaywireClass::rttiNameOrdnanceHaywireClass
const char OrdnanceHaywireClass::rttiNameOrdnanceHaywireClass[] = "OrdnanceHaywireClass";

// FUNCTION: SWBF2 0x00600f00
OrdnanceHaywireClass::OrdnanceHaywireClass(unsigned int uHash)
    : OrdnanceStickyClass(uHash)
{
    mDisableTime = 10.0f;
}

// FUNCTION: SWBF2 0x00600f60
OrdnanceHaywireClass::OrdnanceHaywireClass(OrdnanceHaywireClass* pBase, unsigned int uHash)
    : OrdnanceStickyClass(pBase, uHash)
{
    mDisableTime = pBase->mDisableTime;
}







// FUNCTION: SWBF2 0x00600fa0
void OrdnanceHaywireClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty != 0x1C1694FA /* "DisableTime" */)
    {
        OrdnanceStickyClass::SetProperty(uProperty, pszValue);
        return;
    }
    sscanf(pszValue, "%f", &mDisableTime);
}

// FUNCTION: SWBF2 0x00600fe0
OrdnanceClass* OrdnanceHaywireClass::Derive(unsigned int uHash)
{
    return new OrdnanceHaywireClass(this, uHash);
}

// FUNCTION: SWBF2 0x00600f90
OrdnanceHaywireClass::~OrdnanceHaywireClass()
{
}

// SYNTHETIC: SWBF2 0x00600f30
// OrdnanceHaywireClass::`scalar deleting destructor'
