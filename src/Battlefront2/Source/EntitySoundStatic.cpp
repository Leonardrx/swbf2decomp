#include "EntitySoundStatic.h"
#include "SoundFL/source/sndvoicevirtual.h"

// SYNTHETIC: SWBF2 0x00402e50
// `dynamic initializer for 'EntitySoundStaticClass::rttiHashEntitySoundStaticClass''
// EntitySoundStaticClass::rttiHashEntitySoundStaticClass

// GLOBAL: SWBF2 0x01ebc1a4
// EntitySoundStaticClass::rttiHashEntitySoundStaticClass
PblHash EntitySoundStaticClass::rttiHashEntitySoundStaticClass("EntitySoundStaticClass");
// GLOBAL: SWBF2 0x00789604
// EntitySoundStaticClass::rttiNameEntitySoundStaticClass
const char EntitySoundStaticClass::rttiNameEntitySoundStaticClass[] = "EntitySoundStaticClass";

// FUNCTION: SWBF2 0x004fb6b0
EntitySoundStaticClass::EntitySoundStaticClass(unsigned int uHash)
    : EntitySoundClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fb710
EntitySoundStaticClass::EntitySoundStaticClass(EntitySoundStaticClass* pBase,
                                               unsigned int uHash)
    : EntitySoundClass(pBase, uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fb750
EntitySoundStaticClass::~EntitySoundStaticClass()
{
}

// SYNTHETIC: SWBF2 0x004fb6e0
// EntitySoundStaticClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fb760
EntityClass* EntitySoundStaticClass::Derive(unsigned int uHash)
{
    return new EntitySoundStaticClass(this, uHash);
}



// FUNCTION: SWBF2 0x004fb7d0
Entity* EntitySoundStaticClass::Build(const EntityDesc& desc)
{
    return new EntitySoundStatic(this, desc);
}



// SYNTHETIC: SWBF2 0x00402e20
// `dynamic initializer for 'EntitySoundStatic::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766ff0
// `dynamic atexit destructor for 'EntitySoundStatic::sMemoryPool''

// GLOBAL: SWBF2 0x01ebc1a8
// EntitySoundStatic::sMemoryPool
MemoryPool EntitySoundStatic::sMemoryPool("EntitySoundStatic", sizeof(EntitySoundStatic));

// SYNTHETIC: SWBF2 0x00402e40
// `dynamic initializer for 'EntitySoundStatic::rttiHashEntitySoundStatic''
// EntitySoundStatic::rttiHashEntitySoundStatic

// GLOBAL: SWBF2 0x01ebc1f8
// EntitySoundStatic::rttiHashEntitySoundStatic
PblHash EntitySoundStatic::rttiHashEntitySoundStatic("EntitySoundStatic");
// GLOBAL: SWBF2 0x00789620
// EntitySoundStatic::rttiNameEntitySoundStatic
const char EntitySoundStatic::rttiNameEntitySoundStatic[] = "EntitySoundStatic";







inline void* EntitySoundStatic::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

// FUNCTION: SWBF2 0x004fb500
EntitySoundStatic::EntitySoundStatic(EntitySoundStaticClass* pClass, const EntityDesc& desc)
    : EntitySound(pClass, desc)
{
    ActivateThread(0, PER_FRAME);
}

// SYNTHETIC: SWBF2 0x004fb4d0
// EntitySoundStatic::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x004fb835
// ??_EEntitySoundStatic@@WCI@AEPAXI@Z

// FUNCTION: SWBF2 0x004fb580
EntitySoundStatic::~EntitySoundStatic()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x004fb5f0
void EntitySoundStatic::Play()
{
    if (mSoundId)
    {
        mSoundProperties.SetID(mSoundId, false, false);
        mSound.Play(&mSoundProperties, &mPosition, 0, true);
    }
}

// FUNCTION: SWBF2 0x004fb620
bool EntitySoundStatic::Update(float fDeltaTime)
{
    mSound.Update(&mSoundProperties, &mPosition, 0);
    Snd::VoiceVirtual* pVoice = mSound.GetVoiceVirtual();
    if (pVoice)
    {
        if (mFlags & 1)
        {
            pVoice->SetMinDistance(mMinDistance);
        }
        if (mFlags & 4)
        {
            pVoice->SetMuteDistance(mMuteDistance);
        }
        if (mFlags & 2)
        {
            pVoice->SetMaxDistance(mMaxDistance);
        }
    }
    if (!(mSound.mFlags & GameSoundControllable::flags_playing))
    {
        mSound.Play(&mSoundProperties, &mPosition, 0, true);
    }
    return mSoundProperties.GetProperties() != 0;
}
