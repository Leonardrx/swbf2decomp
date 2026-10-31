#include "Entity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NetGame.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

// SYNTHETIC: SWBF2 0x00402830
// `dynamic initializer for 'Entity::rttiHashEntity''

// GLOBAL: SWBF2 0x01eb9864
// Entity::rttiHashEntity
PblHash Entity::rttiHashEntity("Entity");
// GLOBAL: SWBF2 0x00788f2c
// Entity::rttiNameEntity
const char Entity::rttiNameEntity[] = "Entity";

// SYNTHETIC: SWBF2 0x00402840
// `dynamic initializer for 'EntityEx::rttiHashEntityEx''
// EntityEx::rttiHashEntityEx

// GLOBAL: SWBF2 0x01eb9868
// EntityEx::rttiHashEntityEx
PblHash EntityEx::rttiHashEntityEx("EntityEx");
// GLOBAL: SWBF2 0x00788f44
// EntityEx::rttiNameEntityEx
const char EntityEx::rttiNameEntityEx[] = "EntityEx";

// SYNTHETIC: SWBF2 0x00402850
// `dynamic initializer for 'EntityClass::rttiHashEntityClass''
// EntityClass::rttiHashEntityClass

// GLOBAL: SWBF2 0x01eb9860
// EntityClass::rttiHashEntityClass
PblHash EntityClass::rttiHashEntityClass("EntityClass");
// GLOBAL: SWBF2 0x00788f34
// EntityClass::rttiNameEntityClass
const char EntityClass::rttiNameEntityClass[] = "EntityClass";
// SYNTHETIC: SWBF2 0x00402860
// `dynamic initializer for 'EntityEx::mIdMap''

// GLOBAL: SWBF2 0x01eb9870
PblHashTable<EntityEx, 1024> EntityEx::mIdMap;



// FUNCTION: SWBF2 0x004918d0
EntityClass::EntityClass(unsigned int uHash)
    : Factory<Entity, EntityClass, EntityDesc>(uHash)
{
    mLabel = 0;
    mIconTexture = 0;
    mHealthTexture = 0;
    mHUDModelID = 0;
    mRemoveFromMapWhenDead = false;
}

// FUNCTION: SWBF2 0x00491ab0
EntityClass::~EntityClass()
{
}

// SYNTHETIC: SWBF2 0x00491990
// EntityClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00491fb0
// Factory<Entity,EntityClass,EntityDesc>::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004919c0
EntityClass::EntityClass(EntityClass* pParent, unsigned int uHash)
    : Factory<Entity, EntityClass, EntityDesc>(pParent, uHash)
{
    mLabel = pParent->mLabel;
    mIconTexture = pParent->mIconTexture;
    mHealthTexture = pParent->mHealthTexture;
    mHUDModelID = pParent->mHUDModelID;
    mMapParams = pParent->mMapParams;
    mRemoveFromMapWhenDead = pParent->mRemoveFromMapWhenDead;
}

// FUNCTION: SWBF2 0x00491af0
void EntityClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x0e8c3fe5:
        sscanf(pszValue, "%f", &mMapParams.mScale);
        break;
    case 0x7f8a4c00:
        sscanf(pszValue, "%f", &mMapParams.mSpeedMin);
        break;
    case 0x9576e6ea:
        sscanf(pszValue, "%f", &mMapParams.mSpeedMax);
        break;
    case 0x95eee380:
        sscanf(pszValue, "%f", &mMapParams.mViewRangeMin);
        mMapParams.mRangeViewCone = mMapParams.mViewRangeMin > mMapParams.mRangeViewCone
                                        ? mMapParams.mViewRangeMin
                                        : mMapParams.mRangeViewCone;
        break;
    case 0xabdb7e6a:
        sscanf(pszValue, "%f", &mMapParams.mViewRangeMax);
        break;
    case 0x9dc38d80:
        mMapParams.mTexture = PblHash(pszValue);
        break;
    case 0x29ef7150:
        sscanf(pszValue, "%f", &mMapParams.mTargetFadeTime);
        break;
    case 0x2382f4bf:
        sscanf(pszValue, "%f", &mMapParams.mRangeOverall);
        break;
    case 0x3f6ae0b3:
        sscanf(pszValue, "%f", &mMapParams.mRangeShooting);
        break;
    case 0x3ac4d4c8:
        sscanf(pszValue, "%f", &mMapParams.mRangeViewCone);
        break;
    case 0xbdd7449e:
    {
        float fAngle;
        sscanf(pszValue, "%f", &fAngle);
        mMapParams.mViewConeCosine = PblMath::Cos(fAngle * 0.5f);
        break;
    }
    case 0x8ddbeb4b:
        mIconTexture = PblHash(pszValue);
        break;
    case 0x0db7468e:
        mHealthTexture = PblHash(pszValue);
        break;
    case 0x8f8faeb3:
        mHUDModelID = PblHash(pszValue);
        break;
    case 0x58071137:
        mRemoveFromMapWhenDead = atoi(pszValue) != 0;
        break;
    }
}

// FUNCTION: SWBF2 0x00491cb0
void EntityClass::PostReadSetup()
{
}

// FUNCTION: SWBF2 0x00491cc0
void EntityClass::Read(PblFileChunk& chunk)
{
    char acBuffer[256];
    char acType[128];
    EntityClass* pClass = 0;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x45505954:
        {
            memset(acType, 0, sizeof(acType));
            const unsigned int uType = PblHash(child.ReadString(acType, sizeof(acType)));
            if (Find(uType))
            {
                return;
            }

            pClass = pClass->Derive(uType);
            if (pClass)
            {
                _snprintf(acBuffer, 255, "entity.%s", acType);
                acBuffer[255] = 0;
                char* pcUnderscore = strchr(acBuffer, '_');
                if (pcUnderscore)
                {
                    *pcUnderscore = '.';
                }
                pClass->mLabel = (unsigned short*)RedLocalizeDB::Find(PblHash(acBuffer), true);
            }
            break;
        }
        case 0x45534142:
            pClass = Find(PblHash(child.ReadString(acBuffer, sizeof(acBuffer))));
            break;
        case 0x504F5250:
        {
            const unsigned int uProperty = child._pFile->Read32();
            pClass->SetProperty(uProperty, child.ReadString(acBuffer, sizeof(acBuffer)));
            break;
        }
        }
    }

    if (pClass)
    {
        pClass->PostReadSetup();
    }
}

void EntityEx::Store(unsigned int uId, EntityEx* pEntity)
{
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        mIdMap.Store(uId, pEntity);
    }
}

void EntityEx::Remove(unsigned int uId)
{
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        mIdMap.Remove(uId);
    }
}

// FUNCTION: SWBF2 0x00491720
EntityEx::EntityEx(unsigned int uId, EntityClass* pEntityClass)
{
    mId = uId;
    mEntityClass = pEntityClass;
    if (mId)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            mIdMap.Store(mId, this);
        }
    }
}

// FUNCTION: SWBF2 0x00491820
EntityEx::~EntityEx()
{
    if (mId)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            mIdMap.Remove(mId);
        }
    }
}

// SYNTHETIC: SWBF2 0x004917f0
// EntityEx::`scalar deleting destructor'




// [thunk]:MountedTurret::GetEntity`adjustor{12}'
// SYNTHETIC: SWBF2 0x005a93fc
// ?GetEntity@MountedTurret@@WM@AEPAVEntity@@XZ
