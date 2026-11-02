#include <float.h>

#include "CommandWalker.h"
#include "CollisionManager.h"
#include "GameModel.h"
#include "NetGame.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// SYNTHETIC: SWBF2 0x00402700
// `dynamic initializer for 'CommandWalker::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766ea0
// `dynamic atexit destructor for 'CommandWalker::sMemoryPool''

// GLOBAL: SWBF2 0x01eb9700
// CommandWalker::sMemoryPool
MemoryPool CommandWalker::sMemoryPool("CommandWalker", sizeof(CommandWalker));

// SYNTHETIC: SWBF2 0x00402720
// `dynamic initializer for 'CommandWalker::rttiHashCommandWalker''
// CommandWalker::rttiHashCommandWalker

// GLOBAL: SWBF2 0x01eb96f8
// CommandWalker::rttiHashCommandWalker
PblHash CommandWalker::rttiHashCommandWalker("CommandWalker");

// SYNTHETIC: SWBF2 0x00402730
// `dynamic initializer for 'CommandWalkerClass::rttiHashCommandWalkerClass''
// CommandWalkerClass::rttiHashCommandWalkerClass

// GLOBAL: SWBF2 0x01eb96fc
// CommandWalkerClass::rttiHashCommandWalkerClass
PblHash CommandWalkerClass::rttiHashCommandWalkerClass("CommandWalkerClass");
// GLOBAL: SWBF2 0x00788cfc
// CommandWalkerClass::rttiNameCommandWalkerClass
const char CommandWalkerClass::rttiNameCommandWalkerClass[] = "CommandWalkerClass";

// FUNCTION: SWBF2 0x00480080
CommandWalkerClass::CommandWalkerClass(unsigned int uHash)
    : EntityWalkerClass(uHash), CommandPostClass(uHash)
{
    mIsNeutralWhenEmpty = false;
    mTimeRequiredToEject = FLT_MAX;
    mEjectResist = 10.0f;
}







// SYNTHETIC: SWBF2 0x00480100
// CommandWalkerClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00480130
CommandWalkerClass::CommandWalkerClass(CommandWalkerClass* pBase, unsigned int uHash)
    : EntityWalkerClass(pBase, uHash), CommandPostClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x004801b0
CommandWalkerClass::~CommandWalkerClass()
{
}

// FUNCTION: SWBF2 0x00480210
void CommandWalkerClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityWalkerClass::SetProperty(uProperty, pszValue);
    CommandPostClass::SetProperty(uProperty, pszValue, mModel ? mModel->_mLod[0] : 0, 0);
}

// FUNCTION: SWBF2 0x00480260
EntityClass* CommandWalkerClass::Derive(unsigned int uHash)
{
    return new CommandWalkerClass(this, uHash);
}

// FUNCTION: SWBF2 0x004802d0
Entity* CommandWalkerClass::Build(const EntityDesc& desc)
{
    if (CommandWalker::sMemoryPool.IsFull())
    {
        return 0;
    }

    CommandWalker* pWalker = new CommandWalker(this, desc);
    if (pWalker)
    {
        NetGame::Created(pWalker);
        pWalker->AddRigidToCollisionManager();
        pWalker->ActivatePhysics();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            pWalker->BuildEffects();
        }
    }
    return pWalker;
}





// FUNCTION: SWBF2 0x0047fdc0
EntityType CommandWalker::GetEntityType() const
{
    return ENT_POST_WALKER;
}

// SYNTHETIC: SWBF2 0x0047fdd0
// CommandWalker::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x004803bb
// ??_ECommandWalker@@WCEA@AEPAXI@Z

// FUNCTION: SWBF2 0x0047fe70
CommandWalker::CommandWalker(CommandWalkerClass* pClass, const EntityDesc& desc)
    : EntityWalker(pClass, desc)
{
    mPost = CommandPost::BuildPost(this, pClass);
}

// FUNCTION: SWBF2 0x0047ff20
CommandWalker::~CommandWalker()
{
    if (mPost)
    {
        mPost->Kill();
    }
}

// FUNCTION: SWBF2 0x0047ffc0
void CommandWalker::Init()
{
    EntityWalker::Init();
    mPost->Respawn();
}

// FUNCTION: SWBF2 0x0047ffe0
void CommandWalker::Kill()
{
    EntityWalker::Kill();
    mPost->Kill();
    mPost = 0;
}

// FUNCTION: SWBF2 0x00480050
void CommandWalker::SetTeam(int iTeam)
{
    int iOldTeam = GetTeam();
    EntityWalker::SetTeam(iTeam);
    mPost->SetTeam(iOldTeam, iTeam);
}



// FUNCTION: SWBF2 0x00480000
void CommandWalker::Respawn()
{
    mPost->Respawn();
    GameObject::Respawn();
}

// FUNCTION: SWBF2 0x00480020
void CommandWalker::SetProperty(unsigned int uProperty, const char* pszValue)
{
    GameObject::SetProperty(uProperty, pszValue);
    mPost->SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x0047fe40
unsigned int CommandWalkerClass::GetRtti()
{
    return CommandWalkerClass::rttiHashCommandWalkerClass;
}
