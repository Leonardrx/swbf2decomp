#include "EntityDefenseGridTurret.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "CollisionManager.h"
#include "NetGame.h"

#include <stdio.h>

// SYNTHETIC: SWBF2 0x004029b0
// `dynamic initializer for 'EntityDefenseGridTurretClass::rttiHashEntityDefenseGridTurretClass''
// EntityDefenseGridTurretClass::rttiHashEntityDefenseGridTurretClass

// GLOBAL: SWBF2 0x01ebb990
// EntityDefenseGridTurretClass::rttiHashEntityDefenseGridTurretClass
PblHash EntityDefenseGridTurretClass::rttiHashEntityDefenseGridTurretClass("EntityDefenseGridTurretClass");
// GLOBAL: SWBF2 0x007890cc
// EntityDefenseGridTurretClass::rttiNameEntityDefenseGridTurretClass
const char EntityDefenseGridTurretClass::rttiNameEntityDefenseGridTurretClass[] = "EntityDefenseGridTurretClass";

// SYNTHETIC: SWBF2 0x004029a0
// `dynamic initializer for 'EntityDefenseGridTurret::rttiHashEntityDefenseGridTurret''
// EntityDefenseGridTurret::rttiHashEntityDefenseGridTurret

// GLOBAL: SWBF2 0x01ebb994
// EntityDefenseGridTurret::rttiHashEntityDefenseGridTurret
PblHash EntityDefenseGridTurret::rttiHashEntityDefenseGridTurret("EntityDefenseGridTurret");
// GLOBAL: SWBF2 0x007890f8
// EntityDefenseGridTurret::rttiNameEntityDefenseGridTurret
const char EntityDefenseGridTurret::rttiNameEntityDefenseGridTurret[] = "EntityDefenseGridTurret";

// SYNTHETIC: SWBF2 0x00402980
// `dynamic initializer for 'EntityDefenseGridTurret::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f20
// `dynamic atexit destructor for 'EntityDefenseGridTurret::sMemoryPool''

// GLOBAL: SWBF2 0x01ebb940
// EntityDefenseGridTurret::sMemoryPool
MemoryPool EntityDefenseGridTurret::sMemoryPool("EntityDefenseGridTurret", sizeof(EntityDefenseGridTurret));

inline void* EntityDefenseGridTurret::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

// SYNTHETIC: SWBF2 0x00497a20
// EntityDefenseGridTurret::operator delete
inline void EntityDefenseGridTurret::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

// FUNCTION: SWBF2 0x00497ad0
EntityDefenseGridTurret::EntityDefenseGridTurret(EntityDefenseGridTurretClass* pClass,
                                                   const EntityDesc& desc)
    : EntityBuildingArmedDynamic(pClass, desc)
{
    mClass = pClass;
    mRespawnTimer = pClass->mRespawnTime;
}


// FUNCTION: SWBF2 0x00497b40
bool EntityDefenseGridTurret::Update(float fDeltaTime)
{
    const bool bClient = (netInShell ? netEnabled : netEnabledNext) &&
                         netEnabledNext && netClientActive;
    if (!bClient)
    {
        if (!IsAlive())
        {
            mRespawnTimer -= fDeltaTime;
            if (mRespawnTimer < 0.0f)
            {
                mRespawnTimer = mClass->mRespawnTime;
                Respawn();
            }
        }
        if (mMatrix.m[3][1] < -500.0f)
        {
            Kill();
            Deactivate();
        }
    }
    return true;
}

// SYNTHETIC: SWBF2 0x00497dd2
// ??_EEntityDefenseGridTurret@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497ddd
// ??_EEntityDefenseGridTurret@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497de8
// ??_EEntityDefenseGridTurret@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497df0
// ??_EEntityDefenseGridTurret@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497dfb
// ??_EEntityDefenseGridTurret@@WCGA@AEPAXI@Z







// FUNCTION: SWBF2 0x00497d00
Entity* EntityDefenseGridTurretClass::Build(const EntityDesc& desc)
{
    if (EntityDefenseGridTurret::sMemoryPool.mUsed >= EntityDefenseGridTurret::sMemoryPool.mCount)
    {
        return 0;
    }
    EntityDefenseGridTurret* pTurret = new EntityDefenseGridTurret(this, desc);
    if (pTurret)
    {
        pTurret->AddStaticToCollisionManager();
        pTurret->ActivatePhysics();
    }
    return pTurret;
}

// FUNCTION: SWBF2 0x00497bf0
EntityDefenseGridTurretClass::EntityDefenseGridTurretClass(unsigned int uHash)
    : EntityBuildingArmedDynamicClass(uHash)
{
    mRespawnTime = 30.0f;
}

// FUNCTION: SWBF2 0x00497c50
EntityDefenseGridTurretClass::EntityDefenseGridTurretClass(EntityDefenseGridTurretClass* pBase,
                                                           unsigned int uHash)
    : EntityBuildingArmedDynamicClass(pBase, uHash)
{
    mRespawnTime = 30.0f;
}

// FUNCTION: SWBF2 0x00497c80
EntityDefenseGridTurretClass::~EntityDefenseGridTurretClass()
{
}

// SYNTHETIC: SWBF2 0x00497c20
// EntityDefenseGridTurretClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00497c90
EntityClass* EntityDefenseGridTurretClass::Derive(unsigned int uHash)
{
    return new EntityDefenseGridTurretClass(this, uHash);
}







// FUNCTION: SWBF2 0x00497da0
void EntityDefenseGridTurretClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty != 0xBD94B44E /* "RespawnTime" */)
    {
        EntityBuildingArmedDynamicClass::SetProperty(uProperty, pszValue);
        return;
    }
    sscanf(pszValue, "%f", &mRespawnTime);
}
