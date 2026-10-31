#include <string.h>
#include "EntityBuildingAnimatedArmed.h"
#include "Controllable.h"
#include "NetGame.h"
#include "MountedTurret.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "GameModel.h"

// GLOBAL: SWBF2 0x007e6330
static int sCurrentSection = -1;

// FUNCTION: SWBF2 0x004955e0
EntityBuildingAnimatedArmedClass::EntityBuildingAnimatedArmedClass(unsigned int uHash)
    : EntityBuildingAnimatedClass(uHash)
{
    memset(mTurretClass, 0, sizeof(mTurretClass));
    sCurrentSection = -1;
    mRespawnTime = 120.0f;
}

// FUNCTION: SWBF2 0x004956b0
EntityBuildingAnimatedArmedClass::~EntityBuildingAnimatedArmedClass()
{
}

// FUNCTION: SWBF2 0x004956c0
void EntityBuildingAnimatedArmedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xD89B1A5Cu)
    {
        int iSection;
        switch ((unsigned int)PblHash(pszValue))
        {
        case 0xEA20FEF2u:
        case 0xFB2AA4B7u: iSection = 0; break;
        case 0xE920FD5Fu: iSection = 1; break;
        case 0xE820FBCCu: iSection = 2; break;
        case 0xE720FA39u: iSection = 3; break;
        case 0xE620F8A6u: iSection = 4; break;
        case 0xE520F713u: iSection = 5; break;
        case 0xE420F580u: iSection = 6; break;
        case 0xF3210D1Du: iSection = 7; break;
        case 0xDBAA7975u:
            sCurrentSection = -1;
            return;
        default:
            sCurrentSection = -1;
            return;
        }
        sCurrentSection = iSection;
        if (!mTurretClass[sCurrentSection])
        {
            mTurretClass[sCurrentSection] = new MountedTurretClass(mId);
        }
        return;
    }
    if (sCurrentSection < 0 ||
        !mTurretClass[sCurrentSection]->SetProperty(
            uProperty, pszValue, mModel ? mModel->_mLod[0] : 0))
    {
        EntityBuildingAnimatedClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x004958d0
Entity* EntityBuildingAnimatedArmedClass::Build(const EntityDesc& desc)
{
    EntityBuildingAnimatedArmed* pBuilding = new EntityBuildingAnimatedArmed(this, desc);
    if (pBuilding)
    {
        static_cast<CollisionObject*>(pBuilding)->AddStaticToCollisionManager();
        pBuilding->ActivatePhysics();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            static_cast<EntityGeometry*>(pBuilding)->BuildEffects();
        }
    }
    return pBuilding;
}

// SYNTHETIC: SWBF2 0x004028d0
// `dynamic initializer for 'EntityBuildingAnimatedArmedClass::rttiHashEntityBuildingAnimatedArmedClass''
// EntityBuildingAnimatedArmedClass::rttiHashEntityBuildingAnimatedArmedClass

// GLOBAL: SWBF2 0x01ebb884
// EntityBuildingAnimatedArmedClass::rttiHashEntityBuildingAnimatedArmedClass
PblHash EntityBuildingAnimatedArmedClass::rttiHashEntityBuildingAnimatedArmedClass("EntityBuildingAnimatedArmedClass");
// GLOBAL: SWBF2 0x00788fe4
// EntityBuildingAnimatedArmedClass::rttiNameEntityBuildingAnimatedArmedClass
const char EntityBuildingAnimatedArmedClass::rttiNameEntityBuildingAnimatedArmedClass[] = "EntityBuildingAnimatedArmedClass";







// SYNTHETIC: SWBF2 0x00495630
// EntityBuildingAnimatedArmedClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00495660
EntityBuildingAnimatedArmedClass::EntityBuildingAnimatedArmedClass(EntityBuildingAnimatedArmedClass* pBase,
                                                                   unsigned int uHash)
    : EntityBuildingAnimatedClass(pBase, uHash)
{
    sCurrentSection = -1;
    memcpy(mTurretClass, pBase->mTurretClass, sizeof(mTurretClass));
}

// FUNCTION: SWBF2 0x00495860
EntityClass* EntityBuildingAnimatedArmedClass::Derive(unsigned int uHash)
{
    return new EntityBuildingAnimatedArmedClass(this, uHash);
}

// FUNCTION: SWBF2 0x00495060
EntityBuildingAnimatedArmed::EntityBuildingAnimatedArmed(
    EntityBuildingAnimatedArmedClass* pClass, const EntityDesc& desc)
    : EntityBuildingAnimated(pClass, desc), mNumTurrets(0)
{
    for (int i = 0; i < 8; ++i)
    {
        if (pClass->mTurretClass[i])
        {
            mTurret[mNumTurrets] = new MountedTurret(this, pClass->mTurretClass[i], &mPose);
            mControllableList.AddLast(&mTurret[mNumTurrets]->mNode, static_cast<Controllable*>(mTurret[mNumTurrets]));
            ++mNumTurrets;
        }
    }
    NetGame::Created(this);
}

// SYNTHETIC: SWBF2 0x00495010
// EntityBuildingAnimatedArmed::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x0049597a
// ??_EEntityBuildingAnimatedArmed@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00495985
// ??_EEntityBuildingAnimatedArmed@@WCGA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00495990
// ??_EEntityBuildingAnimatedArmed@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0049599b
// ??_EEntityBuildingAnimatedArmed@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004959a6
// ??_EEntityBuildingAnimatedArmed@@WM@AEPAXI@Z

// FUNCTION: SWBF2 0x004951a0
EntityBuildingAnimatedArmed::~EntityBuildingAnimatedArmed()
{
    NetGame::Destroyed(this);
    while (mNumTurrets)
    {
        --mNumTurrets;
        mControllableList.Remove(&mTurret[mNumTurrets]->mNode);
        delete mTurret[mNumTurrets];
    }
}



// FUNCTION: SWBF2 0x00495290
void EntityBuildingAnimatedArmed::Init()
{
    EntityBuildingAnimated::Init();
    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (mTurret[i])
        {
            mTurret[i]->Init();
        }
    }
    if (mpShadow)
    {
        mpShadow->Deactivate();
    }
}

// FUNCTION: SWBF2 0x004952e0
void EntityBuildingAnimatedArmed::ActivatePhysics()
{
    ActivateThread(0, PER_TURN);
    if (!mState)
    {
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->ActivatePhysics();
            }
        }
    }
}

// FUNCTION: SWBF2 0x00495330
void EntityBuildingAnimatedArmed::DeactivatePhysics()
{
    if (!mState)
    {
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->DeactivatePhysics();
            }
        }
    }
    static_cast<Thread*>(this)->DeactivateThread();
}

// FUNCTION: SWBF2 0x00495380
bool EntityBuildingAnimatedArmed::Update(float fDeltaTime)
{
    Damageable::UpdateStatus(fDeltaTime);
    return EntityBuildingAnimated::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x004954d0
void EntityBuildingAnimatedArmed::Kill()
{
    if (mState == STATE_DEAD)
    {
        return;
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Kill();
        mTurret[i]->DeactivatePhysics();
    }
    EntityBuildingAnimated::Kill();
}

// FUNCTION: SWBF2 0x00495520
void EntityBuildingAnimatedArmed::Respawn()
{
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->ActivatePhysics();
    }
    EntityBuilding::Respawn();
}

// FUNCTION: SWBF2 0x004953b0
void EntityBuildingAnimatedArmed::Render(unsigned char uFlags, float fDist, unsigned int uMask)
{
    if (!mClass->m_bDynamicLighting)
    {
        uMask |= 0x80;
    }
    else
    {
        uMask &= ~0x80;
    }

    PblMatrix mSaved[8];
    if (mNumTurrets > 0)
    {
        PblMatrix mInverse = mMatrix.GetInverseFast();
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->Render(mMatrix, mInverse, mPose, uMask, mSaved[i]);
            }
        }
    }

    EntityBuildingAnimated::Render(uFlags, fDist, uMask);
    if (mNumTurrets > 0)
    {
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->PostRender(mSaved[i]);
            }
        }
    }
}



static inline Controllable* TurretControllable(MountedTurret* pTurret)
{
    return static_cast<Controllable*>(pTurret);
}

// SYNTHETIC: SWBF2 0x004028c0
// `dynamic initializer for 'EntityBuildingAnimatedArmed::rttiHashEntityBuildingAnimatedArmed''
// EntityBuildingAnimatedArmed::rttiHashEntityBuildingAnimatedArmed

// GLOBAL: SWBF2 0x01ebb880
// EntityBuildingAnimatedArmed::rttiHashEntityBuildingAnimatedArmed
PblHash EntityBuildingAnimatedArmed::rttiHashEntityBuildingAnimatedArmed("EntityBuildingAnimatedArmed");
// GLOBAL: SWBF2 0x00788fb8
// EntityBuildingAnimatedArmed::rttiNameEntityBuildingAnimatedArmed
const char EntityBuildingAnimatedArmed::rttiNameEntityBuildingAnimatedArmed[] = "EntityBuildingAnimatedArmed";





// FUNCTION: SWBF2 0x00476770
GameObjectType EntityBuildingAnimatedArmed::GetGameObjectType()
{
    return (GameObjectType)(GO_BUILDING | GO_ARMEDBUILDING);
}

// FUNCTION: SWBF2 0x00495000
EntityType EntityBuildingAnimatedArmed::GetEntityType() const
{
    return ENT_ARMED;
}

// FUNCTION: SWBF2 0x00495560
Trackable* EntityBuildingAnimatedArmed::GetTrackable()
{
    for (int i = 0; i < mNumTurrets; i++)
    {
        Controllable* pPilot = mTurret[i]->GetPilot();
        if (pPilot && pPilot->mPlayerId >= 0 && NetGame::GetJoystickIndex(pPilot->mPlayerId) >= 0)
        {
            return mTurret[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00476740
int EntityBuildingAnimatedArmed::GetTurretCount() const
{
    return mNumTurrets;
}

// FUNCTION: SWBF2 0x00476750
const Controllable* EntityBuildingAnimatedArmed::GetTurretPtr(int iIndex) const
{
    return TurretControllable(mTurret[iIndex]);
}

// FUNCTION: SWBF2 0x00476730
unsigned int EntityBuildingAnimatedArmed::GetRtti()
{
    return EntityBuildingAnimatedArmed::rttiHashEntityBuildingAnimatedArmed;
}
