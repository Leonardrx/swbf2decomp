#include "ActiveRegion.h"
#include "Character.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "GameObject.h"
#include "LuaHelper.h"
#include "EventManager_Region.h"
#include "NetGame.h"
#include "StringDB.h"
#include <new>

// GLOBAL: SWBF2 0x007b2990
// __xmm@ffffffffffffffffffffffffffffffff

// TEMPLATE: SWBF2 0x00437f20
// PblBitVector<600>::InvertAll
template void PblBitVector<600>::InvertAll();

// TEMPLATE: SWBF2 0x00437f80
// PblBitVector<600>::And
template void PblBitVector<600>::And(const PblBitVector<600>&);

namespace EventManager
{
// FUNCTION: SWBF2 0x00437e00
template <>
void Event<RedRegion, Character>::Callback::Trigger(RedRegion& item, Character& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild)
            {
                const bool bAlive = pChild->mAliveFlag;
                if (!bAlive)
                {
                    pChild->Release();
                    break;
                }
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
            }
        }
    }
    Release();
}
}

static bool GameObject_IsAlive(const GameObject* pObject)
{
    return pObject->mIsAlive;
}


namespace ActiveRegions
{

// FUNCTION: SWBF2 0x004375d0
ActiveRegion::ActiveRegion(RedRegion& region) : mRegion(region), mPlayers(GetPlayersInRegion())
{
    sMap.Store(mRegion.mId, this);
    ActivateThread(-100, PER_FRAME);
}

// FUNCTION: SWBF2 0x00437b50
ActiveRegion* __fastcall Activate(RedRegion* pRegion)
{
    ActiveRegion* pActive = sMap.Find(pRegion->mId);
    if (pActive)
    {
        return pActive;
    }
    return ::new (ActiveRegion::sMemoryPool.Allocate(sizeof(ActiveRegion))) ActiveRegion(*pRegion);
}

// FUNCTION: SWBF2 0x00437bc0
void __fastcall Deactivate(RedRegion* pRegion)
{
    ActiveRegion* pActive = sMap.Find(pRegion->mId);
    if (pActive)
    {
        delete pActive;
    }
}

// FUNCTION: SWBF2 0x00437bf0
RedRegion* __fastcall Find(lua_State*, int iIndex)
{
    if (lua_type(LuaHelper::mState, iIndex) == LUA_TLIGHTUSERDATA)
    {
        return (RedRegion*)lua_topointer(LuaHelper::mState, iIndex);
    }
    if (lua_isstring(LuaHelper::mState, iIndex))
    {
        return RedRegion::Find(PblHash(luaL_checklstring(LuaHelper::mState, iIndex, 0)));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00437c60
int Get(lua_State*)
{
    RedRegion* pRegion = ActiveRegions::Find(LuaHelper::mState, 1);
    if (pRegion)
    {
        lua_pushlightuserdata(LuaHelper::mState, pRegion);
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x00437ca0
int Activate(lua_State*)
{
    RedRegion* pRegion = ActiveRegions::Find(LuaHelper::mState, 1);
    if (pRegion)
    {
        ActiveRegions::Activate(pRegion);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00437cc0
int Deactivate(lua_State*)
{
    RedRegion* pRegion = ActiveRegions::Find(LuaHelper::mState, 1);
    if (pRegion)
    {
        ActiveRegions::Deactivate(pRegion);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00437ce0
int Name(lua_State*)
{
    RedRegion* pRegion = ActiveRegions::Find(LuaHelper::mState, 1);
    if (pRegion)
    {
        lua_pushstring(LuaHelper::mState, StringDB::Find(pRegion->mId));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x00437d20
int IsInside(lua_State*)
{
    const int iCharacter = (int)luaL_checknumber(LuaHelper::mState, 1);
    RedRegion* pRegion = ActiveRegions::Find(LuaHelper::mState, 2);
    ActiveRegion* pActive = pRegion ? sMap.Find(pRegion->mId) : 0;
    if (pActive)
    {
        lua_pushboolean(LuaHelper::mState, pActive->mPlayers.IsBitSet(iCharacter));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}
// SYNTHETIC: SWBF2 0x00402150
// ActiveRegions::`dynamic initializer for 'sMap''

// GLOBAL: SWBF2 0x01eafff0
PblHashTable<ActiveRegion, 64> sMap;

// SYNTHETIC: SWBF2 0x00402170
// `dynamic initializer for 'ActiveRegions::ActiveRegion::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766d20
// `dynamic atexit destructor for 'ActiveRegions::ActiveRegion::sMemoryPool''

// GLOBAL: SWBF2 0x01eb01f8
// ActiveRegion::sMemoryPool
// STRING: SWBF2 0x00796adc
MemoryPool ActiveRegion::sMemoryPool("ActiveRegion", sizeof(ActiveRegion));

ActiveRegion::~ActiveRegion()
{
    DeactivateThread();
    sMap.Remove(mRegion.mId);
}

// FUNCTION: SWBF2 0x00437660
bool ActiveRegion::Update(float fDeltaTime)
{
    PblBitVector<600> oldPlayers(mPlayers);
    mPlayers = GetPlayersInRegion();

    PblBitVector<600> notOld(~oldPlayers);
    PblBitVector<600> entered(mPlayers);
    entered.And(notOld);
    PblBitVector<600> left;
    left = ~mPlayers;
    left.And(oldPlayers);

    for (int i = 0; i < Character::sCharacterCount; ++i)
    {
        if (entered.IsBitSet(i))
        {
            Character* pChr = Character::GetPtr(i);
            EventManager::RegionEvent::mEnterRegion.Trigger(mRegion, *pChr);
        }
        else if (left.IsBitSet(i))
        {
            Character* pChr = Character::GetPtr(i);
            EventManager::RegionEvent::mLeaveRegion.Trigger(mRegion, *pChr);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00437960
PblBitVector<600> ActiveRegion::GetPlayersInRegion()
{
    PblBitVector<600> players;
    players.ClearAll();

    GameObject* pObjects[64];
    int iCount = CollisionManager::GetObjectsInRegion(mRegion, pObjects, 64, 0, 10);
    for (int i = 0; i < iCount; ++i)
    {
        const bool bAlive = pObjects[i]->mIsAlive;
        if (!bAlive)
        {
            continue;
        }
        for (PblList<Controllable>::Itor itor(pObjects[i]->GetControllableList()); itor; ++itor)
        {
            Character* pChr = itor->mCharacter;
            if (pChr)
            {
                players.SetBit(pChr - Character::sCharacters);
            }
        }
    }
    return players;
}

// SYNTHETIC: SWBF2 0x00437a20
// ActiveRegions::ActiveRegion::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00437b30
void Cleanup()
{
    sMap.Clear();
}

// FUNCTION: SWBF2 0x00437ab0
void Init()
{
    LuaHelper::Register("GetRegion", Get, 0);
    LuaHelper::Register("ActivateRegion", Activate, 0);
    LuaHelper::Register("DeactivateRegion", Deactivate, 0);
    LuaHelper::Register("GetRegionName", Name, 0);
    LuaHelper::Register("IsCharacterInRegion", IsInside, 0);
    ActiveRegion::sMemoryPool.Setup(64, ActiveRegion::sMemoryPool.GetSize(), ActiveRegion::sMemoryPool.GetGrow());
}

}

// FUNCTION: SWBF2 0x00437db0
void* __fastcall InitRawListIterator(void** pCurrent, void*, const void* pSource)
{
    *pCurrent = ((void* const*)pSource)[1];
    return pCurrent;
}



