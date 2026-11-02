#include "EntitySoundStream.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameLoop.h"
#include "GameState.h"

// SYNTHETIC: SWBF2 0x00402e90
// `dynamic initializer for 'EntitySoundStreamClass::rttiHashEntitySoundStreamClass''
// EntitySoundStreamClass::rttiHashEntitySoundStreamClass

// GLOBAL: SWBF2 0x01ebc1fc
// EntitySoundStreamClass::rttiHashEntitySoundStreamClass
PblHash EntitySoundStreamClass::rttiHashEntitySoundStreamClass("EntitySoundStreamClass");
// GLOBAL: SWBF2 0x00789638
// EntitySoundStreamClass::rttiNameEntitySoundStreamClass
const char EntitySoundStreamClass::rttiNameEntitySoundStreamClass[] = "EntitySoundStreamClass";

// FUNCTION: SWBF2 0x004fbc80
EntitySoundStreamClass::EntitySoundStreamClass(unsigned int uHash)
    : EntitySoundClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fbce0
EntitySoundStreamClass::EntitySoundStreamClass(EntitySoundStreamClass* pBase,
                                               unsigned int uHash)
    : EntitySoundClass(pBase, uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fbd20
EntitySoundStreamClass::~EntitySoundStreamClass()
{
}

// SYNTHETIC: SWBF2 0x004fbcb0
// EntitySoundStreamClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fbd30
EntityClass* EntitySoundStreamClass::Derive(unsigned int uHash)
{
    return new EntitySoundStreamClass(this, uHash);
}



// FUNCTION: SWBF2 0x004fbda0
Entity* EntitySoundStreamClass::Build(const EntityDesc& desc)
{
    return new EntitySoundStream(this, desc);
}





// SYNTHETIC: SWBF2 0x00402e80
// `dynamic initializer for 'EntitySoundStream::rttiHashEntitySoundStream''
// EntitySoundStream::rttiHashEntitySoundStream

// GLOBAL: SWBF2 0x01ebc250
// EntitySoundStream::rttiHashEntitySoundStream
PblHash EntitySoundStream::rttiHashEntitySoundStream("EntitySoundStream");
// GLOBAL: SWBF2 0x00789650
// EntitySoundStream::rttiNameEntitySoundStream
const char EntitySoundStream::rttiNameEntitySoundStream[] = "EntitySoundStream";

// SYNTHETIC: SWBF2 0x00402e60
// `dynamic initializer for 'EntitySoundStream::sMemoryPool''
// SYNTHETIC: SWBF2 0x00767000
// `dynamic atexit destructor for 'EntitySoundStream::sMemoryPool''

// GLOBAL: SWBF2 0x01ebc200
// EntitySoundStream::sMemoryPool
MemoryPool EntitySoundStream::sMemoryPool("EntitySoundStream", sizeof(EntitySoundStream));







// FUNCTION: SWBF2 0x004fba90
EntitySoundStream::EntitySoundStream(EntitySoundStreamClass* pClass, const EntityDesc& desc)
    : EntitySound(pClass, desc)
{
    ActivateThread(0, PER_FRAME);
}

// FUNCTION: SWBF2 0x004fbb10
EntitySoundStream::~EntitySoundStream()
{
    DeactivateThread();
}

// SYNTHETIC: SWBF2 0x004fbe08
// ??_EEntitySoundStream@@WCI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004fba60
// EntitySoundStream::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fbb80
void EntitySoundStream::Play()
{
    if (GameState::GetState() != 0x11E1FC01 && GameLoop::IsGameOver())
    {
        return;
    }
    if (!mSoundId)
    {
        return;
    }
    const Snd::SoundStream::Properties* pProps =
        Snd::SoundStream::Properties::FindByHashID(mSoundId);
    if (!pProps)
    {
        return;
    }

    mSoundStream.SetProperties(pProps);
    mSoundStream.Set3DMode(true);
    mSoundStream.SetPosition(&mPosition);
    if (mFlags & 1)
    {
        mSoundStream.SetMinDistance(mMinDistance);
    }
    if (mFlags & 4)
    {
        mSoundStream.SetMuteDistance(mMuteDistance);
    }
    if (mFlags & 2)
    {
        mSoundStream.SetMaxDistance(mMaxDistance);
    }
    mSoundStream.Play(true);
}

// FUNCTION: SWBF2 0x004fbc50
bool EntitySoundStream::Update(float fDeltaTime)
{
    if (mSoundStream.GetPlayState() == Snd::SourceTransport::playState_stopped &&
        mSoundStream.GetProperties())
    {
        mSoundStream.Play(true);
    }
    return true;
}

// SYNTHETIC: SWBF2 0x004fb840
// Snd::VoiceVirtualBus::~VoiceVirtualBus
