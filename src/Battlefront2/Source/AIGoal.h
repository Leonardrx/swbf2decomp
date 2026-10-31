#pragma once

#include "PebbleFL/Common/PblHashTable.h"

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "Character.h"
#include "Controllable.h"
#include "GameObject.h"

class Character;
class CommandPost;
class GameObject;

namespace AI
{

enum AIGoalType
{
    AIGOAL_NONE = 0,
    AIGOAL_CONQUEST = 1,
    AIGOAL_DESTROY = 2,
    AIGOAL_DEFEND = 3,
    AIGOAL_DEATHMATCH = 4,
    AIGOAL_CTFOFFENSE = 5,
    AIGOAL_CTFDEFENSE = 6,
    AIGOAL_FOLLOW = 7,
    AIGOAL_COUNT = 8
};

enum GoalTargetType
{
    GTT_NONE = 0,
    GTT_POS = 1,
    GTT_CHARACTER = 2,
    GTT_GAMEOBJECT = 3,
    GTT_FLAG = 4
};

struct GoalTarget
{
    // SYNTHETIC: SWBF2 0x00488290
    // AI::GoalTarget::operator=
    GoalTarget& operator=(const GoalTarget& other) = default;
    GoalTarget()
    {
    }
    explicit GoalTarget(GoalTargetType eType) : mType(eType)
    {
    }

    GoalTarget(const PblVector3& vPos);
    GoalTarget(const GameObject* pObject);

    GoalTargetType mType;
    PblVector3 mPos;
    Character* mChr;
    PblHandle<const GameObject> mGameObj;

    bool GetLocation(PblVector3& vPos);

    // FUNCTION: SWBF2 0x00439c90
    const GameObject* GetGameObject()
    {
        if (mType == GTT_GAMEOBJECT)
        {
            return mGameObj;
        }
        if (mType == GTT_CHARACTER)
        {
            if (mChr->GetControllable())
            {
                return mChr->GetControllable()->GetGameObject();
            }
        }
        return 0;
    }
};

// VTABLE: SWBF2 0x00796b6c
class AIGoal
{
public:

    AIGoal(AIGoalType eType, unsigned char uTeam, float fWeight);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~AIGoal();
    virtual void AddUnit(Character* pChr);
    virtual void Update();
    virtual CommandPost* GetSpawnLocation(const Character* pChr) const;

protected:
    virtual void Assign(Character* pChr);

    void SetEnabled(bool bEnabled);

public:

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    void UnAssignAll();

    unsigned char GetTeam() { return mTeam; }

    static PblHash rttiHashAIGoal;
    static const char rttiNameAIGoal[];
    static MemoryPool sMemoryPool;

    PblList<Character> mUnitList;
    float mWeight;
    bool mEnabled;
    unsigned char mTeam;
};

}

 
 
 
 
 

// FUNCTION: SWBF2 0x00439ab0
inline bool AI::AIGoal::IsRtti(unsigned int uRtti) const
{
    return rttiHashAIGoal == uRtti;
}

// FUNCTION: SWBF2 0x00439ad0
inline unsigned int AI::AIGoal::GetDerivedRtti() const
{
    return rttiHashAIGoal;
}

// FUNCTION: SWBF2 0x00439ae0
inline const char* AI::AIGoal::GetDerivedRttiName() const
{
    return rttiNameAIGoal;
}
