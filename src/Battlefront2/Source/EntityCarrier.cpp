#include <d3dx9math.h>
#include <new>
#include <stdio.h>
#include <string.h>

#include "EntityCarrier.h"
#include "CommandFlyer.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "Character.h"
#include "CollisionObject.h"
#include "EntitySquadron.h"
#include "FirstPerson.h"
#include "Tracker.h"
#include "NetGame.h"
#include "WeaponCannonInstance.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "CollisionManager.h"
#include "GameLoop.h"
#include "MotionBlurEffect.h"
#include "Trackable.h"
#include <float.h>
float GetDefaultFOV();

// SYNTHETIC: SWBF2 0x00496ea0
// EntityCarrier::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00497941
// ??_EEntityCarrier@@WCEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0049794c
// ??_EEntityCarrier@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497962
// ??_EEntityCarrier@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0049796d
// ??_EEntityCarrier@@WDMM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497978
// ??_EEntityCarrier@@WCFI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00497983
// ??_EEntityCarrier@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0049798b
// ??_EEntityCarrier@@WCAA@AEPAXI@Z

// FUNCTION: SWBF2 0x00497060
EntityCarrier::~EntityCarrier()
{
}

// SYNTHETIC: SWBF2 0x00478579
// ?GetGameObject@EntityFlyer@@WBEA@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004785ad
// ?GetGameObject@EntityFlyer@@WM@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004785bd
// ?GetEntity@EntityFlyer@@WM@BEPBVEntity@@XZ
// SYNTHETIC: SWBF2 0x004785db
// ?GetControllable@EntityFlyer@@WCFI@AEPAVControllable@@XZ
// SYNTHETIC: SWBF2 0x004785e6
// ?GetControllable@EntityFlyer@@WCFI@BEPBVControllable@@XZ
// SYNTHETIC: SWBF2 0x004785fc
// ?GetMatrix@EntityFlyer@@WDMA@BEABVPblMatrix@@XZ

// FUNCTION: SWBF2 0x00496f60
EntityCarrier::EntityCarrier(EntityCarrierClass* pClass, const EntityDesc& desc)
    : EntityFlyer(pClass, desc)
{
    memset(mCargoArray, 0, sizeof(mCargoArray));
    mCargoCount = pClass->mCargoCount;
    VehicleEngine::TurnOn(GetClass(), mMatrix.GetTranslation(), mVelocity, 1.0f);
}

// SYNTHETIC: SWBF2 0x00497957
// ?Kill@EntityCarrier@@WBAA@AEXXZ

// FUNCTION: SWBF2 0x00497110
void EntityCarrier::Kill()
{
    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iPlayer = NetGame::GetJoystickIndex(mPlayerId);
        if (iPlayer >= 0)
        {
            FirstPerson::Activate(iPlayer, false);
            FirstPerson::Update(iPlayer, TYPE_COCKPIT, -2, 0, 0);
        }
    }
    for (int i = 0; i < mCargoCount; ++i)
    {
        GameObject* pObject = mCargoArray[i].mObject;
        if (pObject)
        {
            Controllable* pControllable = pObject->GetControllable();
            if (pControllable)
            {
                pControllable->Kill();
            }
            DetachCargo(i);
        }
    }
    EntityFlyer::Kill();
}

// FUNCTION: SWBF2 0x004971d0
bool EntityCarrier::Update(float fDeltaTime)
{
    if (!EntityFlyer::Update(fDeltaTime))
    {
        return false;
    }

    for (int i = 0; i < mCargoCount; ++i)
    {
        GameObject* pObject = mCargoArray[i].mObject;
        if (!pObject)
        {
            continue;
        }

        PblMatrix matrix = mMatrix;
        PblVector3 position;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&position), reinterpret_cast<const D3DXVECTOR3*>(&mCargoArray[i].mOffset), reinterpret_cast<const D3DXMATRIX*>(&matrix));
        matrix.SetTranslation(position);
        pObject->SetMatrix(matrix);
        pObject->SetVelocity(mVelocity);
    }
    return true;
}


namespace
{

    inline void EntityFlyerUpdateCollisionSphere(EntityFlyer* pFlyer)
    {
        const PblMatrix& matrix = pFlyer->mMatrix;
        const GameModel* pModel = pFlyer->mModel;
        PblVector3 center;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&center), reinterpret_cast<const D3DXVECTOR3*>(&pModel->_mModel->_BoundingSphere.GetCenter()), reinterpret_cast<const D3DXMATRIX*>(&matrix));
        CollisionObject* pCollision = static_cast<CollisionObject*>(pFlyer);
        pCollision->SetCollisionCenter(center);
        pCollision->RecalculateAABB();
    }
}

// FUNCTION: SWBF2 0x00477720
void EntityFlyer::SetPosition(const PblVector3& vPos)
{
    mMatrix.SetTranslation(vPos);
    PblVector3 center;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&center), reinterpret_cast<const D3DXVECTOR3*>(&mModel->_mModel->_BoundingSphere.GetCenter()), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
    SetCollisionCenter(center);
    RecalculateAABB();
}

// FUNCTION: SWBF2 0x004777c0
void EntityFlyer::SetMatrix(const PblMatrix& mMatrix)
{
    this->mMatrix = mMatrix;
    PblVector3 center;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&center), reinterpret_cast<const D3DXVECTOR3*>(&mModel->_mModel->_BoundingSphere.GetCenter()), reinterpret_cast<const D3DXMATRIX*>(&this->mMatrix));
    SetCollisionCenter(center);
    RecalculateAABB();
}



// FUNCTION: SWBF2 0x004778d0
Trackable* EntityFlyer::GetTrackable()
{
    return this;
}

// FUNCTION: SWBF2 0x004779a0
unsigned int EntityFlyer::GetAllegiance()
{
    const Character* pCharacter = mCharacter;
    return pCharacter ? pCharacter->mTeamNumber : 0;
}

// FUNCTION: SWBF2 0x004779c0
bool EntityFlyer::CanJoinSquadron() const
{
    return !EntitySquadron::InSquadron() && GetFollowingPath();
}

// FUNCTION: SWBF2 0x00477b00
int EntityFlyerClass::GetVehicleType() const
{
    return mVehicleType;
}

// FUNCTION: SWBF2 0x00477b10
int EntityFlyerClass::GetWeaponCount() const
{
    return mWeaponCount;
}

// FUNCTION: SWBF2 0x00477b20
const WeaponClass* EntityFlyerClass::GetWeaponClass(int iIndex) const
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x00477b40
float EntityFlyerClass::GetTimeRequiredToEject() const
{
    return mTimeRequiredToEject;
}

// FUNCTION: SWBF2 0x00477b50
float EntityFlyerClass::GetReboardTime() const
{
    return mTimeTilReboard;
}

// FUNCTION: SWBF2 0x00477b60
const ControllableClass* EntityFlyerClass::GetControllableClass() const
{
    return this ? static_cast<const ControllableClass*>(this) : 0;
}

// SYNTHETIC: SWBF2 0x00402960
// `dynamic initializer for 'EntityCarrier::rttiHashEntityCarrier''
// EntityCarrier::rttiHashEntityCarrier

// GLOBAL: SWBF2 0x01ebb8ec
// EntityCarrier::rttiHashEntityCarrier
PblHash EntityCarrier::rttiHashEntityCarrier("EntityCarrier");
// GLOBAL: SWBF2 0x007890b4
// EntityCarrier::rttiNameEntityCarrier
const char EntityCarrier::rttiNameEntityCarrier[] = "EntityCarrier";

// SYNTHETIC: SWBF2 0x00402940
// `dynamic initializer for 'EntityCarrier::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f10
// `dynamic atexit destructor for 'EntityCarrier::sMemoryPool''

// GLOBAL: SWBF2 0x01ebb8f0
// EntityCarrier::sMemoryPool
MemoryPool EntityCarrier::sMemoryPool("EntityCarrier", sizeof(EntityCarrier));







// SYNTHETIC: SWBF2 0x00402970
// `dynamic initializer for 'EntityCarrierClass::rttiHashEntityCarrierClass''
// EntityCarrierClass::rttiHashEntityCarrierClass

// GLOBAL: SWBF2 0x01ebb8e8
// EntityCarrierClass::rttiHashEntityCarrierClass
PblHash EntityCarrierClass::rttiHashEntityCarrierClass("EntityCarrierClass");
// GLOBAL: SWBF2 0x007890a0
// EntityCarrierClass::rttiNameEntityCarrierClass
const char EntityCarrierClass::rttiNameEntityCarrierClass[] = "EntityCarrierClass";







// FUNCTION: SWBF2 0x00497300
bool EntityCarrier::AttachCargo(int, GameObject* pGameObject)
{
    if (mCargoArray[0].mObject)
    {
        return false;
    }

    EntityCarrierClass* pClass = GetClass();
    mCargoArray[0].mOffset = pClass->mCargoInfo[0].mOffset - pGameObject->GetAttachOffset();
    mCargoArray[0].mObject = pGameObject;
    mCargoArray[0].mObject->mParent = static_cast<CollisionObject*>(this);

    UpdateLandedHeight();

    GetClass()->mSoundCargoPickup.Play(&mMatrix.GetTranslation(), &GetVelocity(), true, true);
    return true;
}

// FUNCTION: SWBF2 0x00497410
bool EntityCarrier::DetachCargo(int iSlot)
{
    if (!mCargoArray[iSlot].mObject)
    {
        return false;
    }

    mCargoArray[iSlot].mObject->mParent = 0;
    mCargoArray[iSlot].mObject = 0;

    UpdateLandedHeight();

    const PblVector3* pVelocity = &GetVelocity();
    GetClass()->mSoundCargoDropoff.Play(&mMatrix.GetTranslation(), pVelocity, true, true);
    return true;
}

// FUNCTION: SWBF2 0x004974b0
void EntityCarrier::UpdateLandedHeight()
{
    mLandedHeight = GetClass()->mLandedHeight;

    for (int i = 0; i < mCargoCount; ++i)
    {
        CargoSlot* pSlot = &mCargoArray[i];
        if (!pSlot->mObject.mObject)
        {
            continue;
        }

        if (pSlot->mObject.mObject->mHandleId == pSlot->mObject.mSavedHandleId)
        {
            const GameModel* pModel = pSlot->mObject.mObject->GetModel();
            if (pModel)
            {
                float fCandidate = -pSlot->mOffset.y - pModel->_mModel->_TotalBoundingBox._vMin.y;
                mLandedHeight = mLandedHeight > fCandidate ? mLandedHeight : fCandidate;
            }
        }
        else
        {
            pSlot->mObject.mObject = 0;
            pSlot->mObject.mSavedHandleId = 0;
        }
    }
}

// FUNCTION: SWBF2 0x00497560
void EntityCarrier::ActivatePhysics()
{
    ActivateThread(-1, PER_TURN);
}

// FUNCTION: SWBF2 0x00497580
EntityCarrierClass::EntityCarrierClass(unsigned int uHash)
    : EntityFlyerClass(uHash)
{
    mPilotType = PILOT_NONE;

    memset(mCargoInfo, 0, sizeof(mCargoInfo));

    mCargoCount = 0;
}

// SYNTHETIC: SWBF2 0x004975e0
// EntityCarrierClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004976a0
EntityCarrierClass::~EntityCarrierClass()
{
}

// SYNTHETIC: SWBF2 0x004780b0
// CommandFlyerClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00478160
CommandFlyerClass::~CommandFlyerClass()
{
}

// SYNTHETIC: SWBF2 0x004785f1
// ??_ECommandFlyerClass@@WBAKA@AEPAXI@Z

// FUNCTION: SWBF2 0x00497610
EntityCarrierClass::EntityCarrierClass(EntityCarrierClass* pSrc, unsigned int uHash)
    : EntityFlyerClass(pSrc, uHash)
{
    memcpy(mCargoInfo, pSrc->mCargoInfo, sizeof(mCargoInfo));
    mCargoCount = pSrc->mCargoCount;
}

// FUNCTION: SWBF2 0x004976b0
void EntityCarrierClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x759C8F54u:
        mSoundCargoPickup.SetID(pszValue, true, false);
        break;

    case 0x3E2C4DA4u:
        mCargoInfo[mCargoCount].mHash = PblTEMPHash(pszValue);

        if (mModel)
        {
            const RedSkeleton* pSkeleton = mModel->GetRedModel(0)->GetSkeleton();
            if (pSkeleton)
            {
                for (int i = 0; i < 128; ++i)
                {
                    if (pSkeleton->GetBoneNameHash(i) == mCargoInfo[mCargoCount].mHash)
                    {

                        const PblMatrix& mBone = pSkeleton->GetDefaultPoseMatrix(i);
                        mCargoInfo[mCargoCount].mOffset = mBone.GetTranslation();
                        ++mCargoCount;
                        break;
                    }
                }
            }
        }
        break;

    case 0x8897DB28u:
        mSoundCargoDropoff.SetID(pszValue, true, false);
        break;

    default:
        EntityFlyerClass::SetProperty(uProperty, pszValue);
        break;

    case 0x910A89FCu:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mCargoInfo[mCargoCount].mHash = 0;
        PblVector3& vOffset = mCargoInfo[mCargoCount].mOffset;
        vOffset.x = x;
        vOffset.y = y;
        vOffset.z = z;
        ++mCargoCount;
        break;
    }
    }
}

// FUNCTION: SWBF2 0x00497830
EntityClass* EntityCarrierClass::Derive(unsigned int uHash)
{
    return (EntityClass*)new EntityCarrierClass(this, uHash);
}




// FUNCTION: SWBF2 0x004978a0
Entity* EntityCarrierClass::Build(const EntityDesc& desc)
{
    if (EntityCarrier::sMemoryPool.IsFull())
    {
        return 0;
    }

    EntityCarrier* pCarrier = new EntityCarrier(this, desc);
    if (pCarrier)
    {
        (static_cast<CollisionObject*>(pCarrier))->AddRigidToCollisionManager();
        pCarrier->ActivatePhysics();
        static_cast<EntityGeometry*>(pCarrier)->BuildEffects();
    }
    return (Entity*)pCarrier;
}

inline void* EntityCarrier::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}


PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace FLRenderer
{
unsigned int GetViewportCount();
}
