#include "CommandHover.h"
#include "CollisionManager.h"
#include "NetGame.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameModel.h"

// SYNTHETIC: SWBF2 0x004026c0
// `dynamic initializer for 'CommandHover::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e90
// `dynamic atexit destructor for 'CommandHover::sMemoryPool''

// GLOBAL: SWBF2 0x01eb96a8
// CommandHover::sMemoryPool
MemoryPool CommandHover::sMemoryPool("CommandHover", sizeof(CommandHover));

// SYNTHETIC: SWBF2 0x004026e0
// `dynamic initializer for 'CommandHover::rttiHashCommandHover''
// CommandHover::rttiHashCommandHover

// GLOBAL: SWBF2 0x01eb96a4
// CommandHover::rttiHashCommandHover
PblHash CommandHover::rttiHashCommandHover("CommandHover");
// GLOBAL: SWBF2 0x00788ca4
// CommandHover::rttiNameCommandHover
const char CommandHover::rttiNameCommandHover[] = "CommandHover";







// FUNCTION: SWBF2 0x00478ef0
EntityType CommandHover::GetEntityType() const
{
    return ENT_POST_HOVER;
}

// SYNTHETIC: SWBF2 0x00478f00
// CommandHover::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00478fa0
CommandHover::CommandHover(CommandHoverClass* pClass, const EntityDesc& desc)
    : EntityHover(pClass, desc)
{
    mClass = pClass;
    mPost = CommandPost::BuildPost(this, pClass);
}

// FUNCTION: SWBF2 0x00479050
CommandHover::~CommandHover()
{
    if (mPost)
    {
        mPost->Kill();
    }
}

// FUNCTION: SWBF2 0x004790f0
void CommandHover::Init()
{
    EntityHover::Init();
    mPost->Respawn();
}

// FUNCTION: SWBF2 0x00479110
void CommandHover::Kill()
{
    EntityHover::Kill();
    mPost->Kill();
}

// FUNCTION: SWBF2 0x00479130
void CommandHover::Respawn()
{
    mPost->Respawn();
    GameObject::Respawn();
}

// FUNCTION: SWBF2 0x00479150
void CommandHover::SetProperty(unsigned int uProperty, const char* pszValue)
{
    GameObject::SetProperty(uProperty, pszValue);
    mPost->SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x00479180
void CommandHover::SetTeam(int iTeam)
{
    int iOldTeam = mTeam;
    GameObject::SetTeam(iTeam);
    mPost->SetTeam(iOldTeam, iTeam);
}

// FUNCTION: SWBF2 0x004791b0
CommandHoverClass::CommandHoverClass(unsigned int uHash)
    : EntityHoverClass(uHash), CommandPostClass(uHash)
{
    mIsNeutralWhenEmpty = false;
}

// SYNTHETIC: SWBF2 0x004026f0
// `dynamic initializer for 'CommandHoverClass::rttiHashCommandHoverClass''
// CommandHoverClass::rttiHashCommandHoverClass

// GLOBAL: SWBF2 0x01eb96a0
// CommandHoverClass::rttiHashCommandHoverClass
PblHash CommandHoverClass::rttiHashCommandHoverClass("CommandHoverClass");
// GLOBAL: SWBF2 0x00788cb8
// CommandHoverClass::rttiNameCommandHoverClass
const char CommandHoverClass::rttiNameCommandHoverClass[] = "CommandHoverClass";







// SYNTHETIC: SWBF2 0x00479220
// CommandHoverClass::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004794f6
// ??_ECommandHoverClass@@WOHA@AEPAXI@Z

// FUNCTION: SWBF2 0x00479250
CommandHoverClass::CommandHoverClass(CommandHoverClass* pBase, unsigned int uHash)
    : EntityHoverClass(pBase, uHash), CommandPostClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x004792d0
CommandHoverClass::~CommandHoverClass()
{
}

// FUNCTION: SWBF2 0x00479330
void CommandHoverClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityHoverClass::SetProperty(uProperty, pszValue);
    CommandPostClass::SetProperty(uProperty, pszValue, mModel ? mModel->_mLod[0] : 0, 0);
}

// FUNCTION: SWBF2 0x00479380
EntityClass* CommandHoverClass::Derive(unsigned int uHash)
{
    return new CommandHoverClass(this, uHash);
}

// FUNCTION: SWBF2 0x00478f70
unsigned int CommandHoverClass::GetRtti()
{
    return CommandHoverClass::rttiHashCommandHoverClass;
}

// FUNCTION: SWBF2 0x004793f0
Entity* CommandHoverClass::Build(const EntityDesc& desc)
{
    if (CommandHover::sMemoryPool.IsFull())
    {
        return 0;
    }

    CommandHover* pHover = new CommandHover(this, desc);
    if (pHover)
    {
        NetGame::Created(pHover);
        pHover->AddRigidToCollisionManager();
        pHover->ActivatePhysics();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            pHover->BuildEffects();
        }
    }
    return pHover;
}
