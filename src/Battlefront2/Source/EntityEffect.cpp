#include "EntityEffect.h"
#include "FLEffect.h"

// SYNTHETIC: SWBF2 0x00402aa0
// `dynamic initializer for 'EntityEffectClass::rttiHashEntityEffectClass''
// EntityEffectClass::rttiHashEntityEffectClass

// GLOBAL: SWBF2 0x01ebbc60
// EntityEffectClass::rttiHashEntityEffectClass
PblHash EntityEffectClass::rttiHashEntityEffectClass("EntityEffectClass");
// GLOBAL: SWBF2 0x007891c4
// EntityEffectClass::rttiNameEntityEffectClass
const char EntityEffectClass::rttiNameEntityEffectClass[] = "EntityEffectClass";

// FUNCTION: SWBF2 0x004a9a00
EntityEffectClass::EntityEffectClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mEffectClass = 0;
}







// SYNTHETIC: SWBF2 0x004a9a30
// EntityEffectClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004a9a60
EntityEffectClass::~EntityEffectClass()
{
}

// FUNCTION: SWBF2 0x004a9a70
void EntityEffectClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty != 0x6E6E8D54 /* "Effect" */)
    {
        EntityClass::SetProperty(uProperty, pszValue);
        return;
    }
    mEffectClass = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
}

// FUNCTION: SWBF2 0x004a9ac0
EntityClass* EntityEffectClass::Derive(unsigned int uHash)
{
    return new EntityEffectClass(uHash);
}

// FUNCTION: SWBF2 0x004a9b20
Entity* EntityEffectClass::Build(const EntityDesc& desc)
{
    if (mEffectClass)
    {
        FLEffectObject* pEffect = mEffectClass->BuildEffect();
        if (pEffect)
        {
            pEffect->AttachEffectToMatrix((const PblMatrix*)&desc, false);
        }
    }
    return 0;
}
