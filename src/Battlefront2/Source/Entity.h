#pragma once

#include "PebbleFL/Common/PblHashTable.h"

class PblFileChunk;

#include <stddef.h>

#include "Factory.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblMath.h"

class Entity;
class EntityClass;
class GameObject;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

struct EntityDesc
{
    // FUNCTION: SWBF2 0x00443f10
    EntityDesc()
        : mMatrix(*reinterpret_cast<const PblMatrix*>(&gMatrixIdentity)),
mNameId(0), mPlayerId(-1), mPriority(0.001f)
{
}

    PblMatrix mMatrix;
    unsigned int mNameId;
    int mPlayerId;
    float mPriority;
};
 

// VTABLE: SWBF2 0x00797300
class Entity
{
public:
    Entity() {}

    // FUNCTION: SWBF2 0x00443ea0
    virtual bool IsRtti(unsigned int uRtti) const { return rttiHashEntity == uRtti; }
    // FUNCTION: SWBF2 0x00458a00
    virtual unsigned int GetDerivedRtti() const { return rttiHashEntity; }
    // FUNCTION: SWBF2 0x00458a10
    virtual const char* GetDerivedRttiName() const { return rttiNameEntity; }
    // FUNCTION: SWBF2 0x00458a20
    virtual ~Entity() {}
    // FUNCTION: SWBF2 0x00458a30
    virtual void SetProperty(unsigned int, const char*) {}
    // FUNCTION: SWBF2 0x00458a40
    virtual void Init() {}
    // FUNCTION: SWBF2 0x00458a50
    virtual Entity* GetEntity() { return this; }
    // FUNCTION: SWBF2 0x00458a60
    virtual const Entity* GetEntity() const { return this; }
    // FUNCTION: SWBF2 0x00458a70
    virtual GameObject* GetGameObject() { return 0; }
    // FUNCTION: SWBF2 0x00458a80
    virtual const GameObject* GetGameObject() const { return 0; }
    // FUNCTION: SWBF2 0x00458a90
    virtual EntityClass* GetEntityClass() { return 0; }
    // FUNCTION: SWBF2 0x00458aa0
    virtual const EntityClass* GetEntityClass() const { return 0; }

    static PblHash rttiHashEntity;

    static unsigned int GetRtti() { return rttiHashEntity; }
    static const char rttiNameEntity[];
};
 

// SYNTHETIC: SWBF2 0x00458ab0
// Entity::`scalar deleting destructor'

// VTABLE: SWBF2 0x0079934c
class EntityEx : public Entity
{
public:
    EntityEx(unsigned int uId, EntityClass* pEntityClass);
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityEx();
    // FUNCTION: SWBF2 0x00443ef0
    virtual EntityClass* GetEntityClass() { return mEntityClass; }
    // FUNCTION: SWBF2 0x00443f00
    virtual const EntityClass* GetEntityClass() const { return mEntityClass; }

    // FUNCTION: SWBF2 0x0047a670
    unsigned int GetId() const { return mId; }

    unsigned int mId;
    EntityClass* mEntityClass;

    static PblHash rttiHashEntityEx;

    static unsigned int GetRtti() { return rttiHashEntityEx; }
    static const char rttiNameEntityEx[];
    static PblHashTable<EntityEx, 1024> mIdMap;

    static PblHashTable<EntityEx, 1024>& GetEntityMap() { return mIdMap; }

    static void InitFind() { mIdMap.Clear(); }

    // FUNCTION: SWBF2 0x0046db30
    static EntityEx* Find(unsigned int uiHash) { return mIdMap.Find(uiHash); }

private:
    static void Store(unsigned int uId, EntityEx* pEntity);
    static void Remove(unsigned int uId);
};
 
 
 

// VTABLE: SWBF2 0x00799328
class EntityClass : public Factory<Entity, EntityClass, EntityDesc>
{
public:

    static void Read(PblFileChunk& chunk);

    class MapParams
    {
    public:
        MapParams(float scale = 1.0f, float speedMin = 10.0f, float speedMax = 100.0f,
                  float viewRangeMin = 125.0f, float viewRangeMax = 250.0f);

        float mScale;
        float mSpeedMin;
        float mSpeedMax;
        float mViewRangeMin;
        float mViewRangeMax;
        unsigned int mTexture;
        float mTargetFadeTime;
        float mRangeOverall;
        float mRangeShooting;
        float mRangeViewCone;
        float mViewConeCosine;
    };

    EntityClass(unsigned int uHash);
    EntityClass(EntityClass* pParent, unsigned int uHash);

    virtual ~EntityClass();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void PostReadSetup();

    static PblHash rttiHashEntityClass;
    static const char rttiNameEntityClass[];

    unsigned short* mLabel;
    unsigned int mIconTexture;
    unsigned int mHealthTexture;
    unsigned int mHUDModelID;
    MapParams mMapParams;
    bool mRemoveFromMapWhenDead;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// FUNCTION: SWBF2 0x00443f60
inline bool EntityClass::IsRtti(unsigned int uRtti) const
{
    return rttiHashEntityClass == uRtti;
}

// FUNCTION: SWBF2 0x0048ec00
inline unsigned int EntityClass::GetDerivedRtti() const
{
    return rttiHashEntityClass;
}

// FUNCTION: SWBF2 0x0048ec10
inline const char* EntityClass::GetDerivedRttiName() const
{
    return rttiNameEntityClass;
}

// FUNCTION: SWBF2 0x00443ec0
inline bool EntityEx::IsRtti(unsigned int uRtti) const
{
    if (rttiHashEntityEx == uRtti)
    {
        return true;
    }
    return Entity::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00491700
inline unsigned int EntityEx::GetDerivedRtti() const
{
    return rttiHashEntityEx;
}

// FUNCTION: SWBF2 0x00491710
inline const char* EntityEx::GetDerivedRttiName() const
{
    return rttiNameEntityEx;
}

inline EntityClass::MapParams::MapParams(float scale, float speedMin, float speedMax,
                                         float viewRangeMin, float viewRangeMax)
{
    mScale = scale;
    mSpeedMin = speedMin;
    mSpeedMax = speedMax;
    mViewRangeMin = viewRangeMin;
    mViewRangeMax = viewRangeMax;
    mTexture = 0;
    mTargetFadeTime = 32.0f;
    mRangeOverall = 15.0f;
    mRangeShooting = 1000.0f;
    mRangeViewCone = 60.0f;
    mViewConeCosine = PblMath::Cos(0.41887903f);
}
