#include "EntityRemoteTerminal.h"

#include "AILowLevel.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "NetGame.h"
#include "UnitController.h"
#include "Character.h"
#include "GameModel.h"
#include "PlayerController.h"
#include "UnitStationaryController.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "CollisionBody.h"
#include "CollisionMesh.h"
#include "CollisionModel.h"
#include "CollisionPrimitive.h"
#include "TargetManager.h"
#include "VehicleManager.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include <d3dx9math.h>

class PblMatrix;
extern PblMatrix gMatrixIdentity;

// FUNCTION: SWBF2 0x004dd880
EntityRemoteTerminalClass::EntityRemoteTerminalClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    m_bDynamicLighting = true;
    mHealthType = HEALTH_TYPE_VEHICLE;
    mPilotType = PILOT_VEHICLE;
    mLODClass = 1;
    mHoldForPlayer = true;
    mCollisionModel.mDenyFlyerLand = true;
}

// FUNCTION: SWBF2 0x004dd960
EntityRemoteTerminalClass::~EntityRemoteTerminalClass() {}

// SYNTHETIC: SWBF2 0x00402d30
// `dynamic initializer for 'EntityRemoteTerminalClass::rttiHashEntityRemoteTerminalClass''
// EntityRemoteTerminalClass::rttiHashEntityRemoteTerminalClass

// GLOBAL: SWBF2 0x01ebc050
// EntityRemoteTerminalClass::rttiHashEntityRemoteTerminalClass
PblHash EntityRemoteTerminalClass::rttiHashEntityRemoteTerminalClass("EntityRemoteTerminalClass");
// GLOBAL: SWBF2 0x00789510
// EntityRemoteTerminalClass::rttiNameEntityRemoteTerminalClass
const char EntityRemoteTerminalClass::rttiNameEntityRemoteTerminalClass[] = "EntityRemoteTerminalClass";

// SYNTHETIC: SWBF2 0x00402d20
// `dynamic initializer for 'EntityRemoteTerminal::rttiHashEntityRemoteTerminal''
// EntityRemoteTerminal::rttiHashEntityRemoteTerminal

// GLOBAL: SWBF2 0x01ebc04c
// EntityRemoteTerminal::rttiHashEntityRemoteTerminal
PblHash EntityRemoteTerminal::rttiHashEntityRemoteTerminal("EntityRemoteTerminal");
// GLOBAL: SWBF2 0x007894ec
// EntityRemoteTerminal::rttiNameEntityRemoteTerminal
const char EntityRemoteTerminal::rttiNameEntityRemoteTerminal[] = "EntityRemoteTerminal";

// SYNTHETIC: SWBF2 0x00402d00
// `dynamic initializer for 'EntityRemoteTerminal::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766fa0
// `dynamic atexit destructor for 'EntityRemoteTerminal::sMemoryPool''

// GLOBAL: SWBF2 0x01ebc058
// EntityRemoteTerminal::sMemoryPool
MemoryPool EntityRemoteTerminal::sMemoryPool("EntityRemoteTerminal", 0x3f0);

namespace
{

inline Damageable* GameObjectDamageable(GameObject* pObject)
{
    return static_cast<Damageable*>(pObject);
}
inline unsigned int GameObjectHandleId(GameObject* pObject)
{
    return pObject->mHandleId;
}

inline GameObject* HandleObject(PblHandle<GameObject>& handle)
{
    if (handle.mObject && GameObjectHandleId(handle.mObject) != handle.mSavedHandleId)
    {
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return handle.mObject;
}

inline Controllable* AsControllable(EntityRemoteTerminal* pTerminal)
{
    return static_cast<Controllable*>(pTerminal);
}


inline const Controllable* SoldierControllable(const EntitySoldier* pSoldier)
{
    return static_cast<const Controllable*>(pSoldier);
}

inline Controllable* SoldierControllable(EntitySoldier* pSoldier)
{
    return static_cast<Controllable*>(pSoldier);
}

inline EntitySoldier* PilotSoldier(Controllable* pPilot)
{
    return pPilot ? static_cast<EntitySoldier*>(pPilot) : 0;
}

}

// FUNCTION: SWBF2 0x004dcc90
EntityRemoteTerminal::EntityRemoteTerminal(EntityRemoteTerminalClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_RIGID, &pClass->mModel->_mLodData)
    , Controllable(pClass, desc.mPlayerId)
{
    mControllableList.AddLast(&this->Controllable::mNode, this);
    mClass = pClass;
    static_cast<RedSceneObject*>(this)->Activate();
    TargetManager::AddTarget(this, true);
    ClearControllable();
    VehicleManager::Add(this);
    NetGame::Created(this);
    mRemoteGameObjectHash = 0;
    mNextLinkedTerminalHash = 0;
    mPrevLinkedTerminalHash = 0;
    mRemoteGameObject.mObject = 0;
    mRemoteGameObject.mSavedHandleId = 0;
    mNextLinkedTerminal = 0;
    mPrevLinkedTerminal = 0;
}

// SYNTHETIC: SWBF2 0x004ddea4
// ??_EEntityRemoteTerminal@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004ddeaf
// ??_EEntityRemoteTerminal@@WCFI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004ddeba
// ??_EEntityRemoteTerminal@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004ddec2
// ??_EEntityRemoteTerminal@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004ddecd
// ??_EEntityRemoteTerminal@@WCEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004ddee3
// ??_EEntityRemoteTerminal@@WJE@AEPAXI@Z

// FUNCTION: SWBF2 0x004dce60
EntityRemoteTerminal::~EntityRemoteTerminal()
{
    NetGame::Destroyed(this);
    mControllableList.Remove(&this->Controllable::mNode);
    TargetManager::RemoveTarget(this);
    VehicleManager::Remove(this);
}

// SYNTHETIC: SWBF2 0x004dcbf0
// EntityRemoteTerminal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004dd420
void EntityRemoteTerminal::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!mClass->m_bDynamicLighting)
    {
        uiFlags |= 0x80;
    }
    else
    {
        uiFlags &= ~0x80;
    }
    SetupShadow(this, mModel, ucViewport, uiFlags, 0);

    RedColor color;
    color.b = 0xff;
    color.g = 0xff;
    color.r = 0xff;
    color.a = (unsigned char)(int)(fDist * 255.0f);
    if (!mModel)
    {
        return;
    }
    mModel->Render(ucViewport, fDist, GetRenderMatrix(), GetRenderPose(), color, uiFlags | 0x1000000, 0);

    PblMatrix mWorld;
    if ((uiFlags & 0x200000) ||
        !FLRenderer::IsReflected(*(const PblVector3*)&_Sphere, _Sphere.w, mWorld, false))
    {
        return;
    }
    mWorld = *GetRenderMatrix() * mWorld;
    uiFlags = (uiFlags & ~0x10000) | 0x10000100;
    mModel->Render(ucViewport, fDist, &mWorld, GetRenderPose(), color, uiFlags, 0);
}




// SYNTHETIC: SWBF2 0x004dde83
// ?GetEntity@EntityRemoteTerminal@@WCEA@BEPBVEntity@@XZ
// SYNTHETIC: SWBF2 0x004ddef6
// ?GetEntity@EntityRemoteTerminal@@WM@BEPBVEntity@@XZ
// FUNCTION: SWBF2 0x004dc9c0
const Entity* EntityRemoteTerminal::GetEntity() const
{
    return reinterpret_cast<const Entity*>(this);
}

// SYNTHETIC: SWBF2 0x004dde78
// ?GetEntity@EntityRemoteTerminal@@WCEA@AEPAVEntity@@XZ
// SYNTHETIC: SWBF2 0x004ddeee
// ?GetEntity@EntityRemoteTerminal@@WM@AEPAVEntity@@XZ
// FUNCTION: SWBF2 0x004dc9b0
Entity* EntityRemoteTerminal::GetEntity()
{
    return reinterpret_cast<Entity*>(this);
}

// SYNTHETIC: SWBF2 0x004dde47
// ?GetGameObject@EntityRemoteTerminal@@WBEA@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004dde5d
// ?GetGameObject@EntityRemoteTerminal@@WCFI@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004dde70
// ?GetGameObject@EntityRemoteTerminal@@WM@BEPBVGameObject@@XZ
// FUNCTION: SWBF2 0x004dc9a0
const GameObject* EntityRemoteTerminal::GetGameObject() const
{
    return reinterpret_cast<const GameObject*>(this);
}

// SYNTHETIC: SWBF2 0x004dde3c
// ?GetGameObject@EntityRemoteTerminal@@WBEA@AEPAVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004dde52
// ?GetGameObject@EntityRemoteTerminal@@WCFI@AEPAVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004dde68
// ?GetGameObject@EntityRemoteTerminal@@WM@AEPAVGameObject@@XZ
// FUNCTION: SWBF2 0x004dc990
GameObject* EntityRemoteTerminal::GetGameObject()
{
    return reinterpret_cast<GameObject*>(this);
}

// SYNTHETIC: SWBF2 0x004dded8
// ?GetMatrix@EntityRemoteTerminal@@WCDE@BEABVPblMatrix@@XZ

// FUNCTION: SWBF2 0x004dc9d0
const PblMatrix& EntityRemoteTerminal::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x004dc9e0
const PblMatrix* EntityRemoteTerminal::GetRenderMatrix() const
{
    return &mMatrix;
}

// FUNCTION: SWBF2 0x004dc9f0
const RedPose* EntityRemoteTerminal::GetRenderPose() const
{
    return GetPose();
}

// FUNCTION: SWBF2 0x004dca00
void EntityRemoteTerminal::SetMatrix(const PblMatrix& matrix)
{
    mMatrix = matrix;
    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter, (const D3DXVECTOR3*)&mModel->_mModel->_BoundingSphere,
                           (const D3DXMATRIX*)&mMatrix);
    SetCollisionCenter(vCenter);
    RecalculateAABB();
}

// FUNCTION: SWBF2 0x004dcab0
Trackable* EntityRemoteTerminal::GetTrackable()
{
    return static_cast<Trackable*>(this);
}

// SYNTHETIC: SWBF2 0x004dde99
// ?GetControllable@EntityRemoteTerminal@@WCFI@BEPBVControllable@@XZ
// FUNCTION: SWBF2 0x004dcae0
Controllable* EntityRemoteTerminal::GetControllable()
{
    return this ? AsControllable(this) : 0;
}

// SYNTHETIC: SWBF2 0x004dde8e
// ?GetControllable@EntityRemoteTerminal@@WCFI@AEPAVControllable@@XZ
// FUNCTION: SWBF2 0x004dcad0
const Controllable* EntityRemoteTerminal::GetControllable() const
{
    return this ? AsControllable(const_cast<EntityRemoteTerminal*>(this)) : 0;
}

// FUNCTION: SWBF2 0x004dcaf0
GameObjectType EntityRemoteTerminal::GetGameObjectType()
{
    return GO_VEHICLE;
}

// FUNCTION: SWBF2 0x004dcb00
EntityType EntityRemoteTerminal::GetEntityType() const
{
    return ENT_ARMED;
}

// FUNCTION: SWBF2 0x004dcb10
float EntityRemoteTerminal::GetMaxSpeed() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004dcb20
float EntityRemoteTerminal::GetMaxStrafe() const
{
    return 0.0f;
}





// FUNCTION: SWBF2 0x0046db50
GameObject* EntityRemoteTerminal::GetRemoteGameObject()
{
    if (!mRemoteGameObject)
    {
        EntityEx* pEntity = EntityEx::mIdMap.Find(mRemoteGameObjectHash);
        if (pEntity)
        {
            mRemoteGameObject = pEntity->GetGameObject();
        }
    }
    return mRemoteGameObject;
}

// FUNCTION: SWBF2 0x004dce00
void EntityRemoteTerminal::Init()
{
    reinterpret_cast<GameObject*>(this)->GameObject::Init();
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        for (PblList<Controllable>::Itor it(GetControllableList()); (Controllable*)it; ++it)
        {
            it->SetupController();
        }
    }
}

// FUNCTION: SWBF2 0x004dcf50
const ControllableClass* EntityRemoteTerminal::GetControllableClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x004dcf70
ControllableClass* EntityRemoteTerminal::GetControllableClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x004dcf90
bool EntityRemoteTerminal::UpdateIndirect(float fDeltaTime)
{
    UnitControllerPtr(mCtrl)->mLowLevel
        .UpdateIndirect(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x004dcfb0
bool EntityRemoteTerminal::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(reinterpret_cast<GameObject*>(this), fDeltaTime))
    {
        return true;
    }

    UpdateHealthRemote(0.0f);

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mPilot &&
        mPilot->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        GameObject* pRemote = GetRemoteGameObject();
        if (pRemote)
        {
            const PblList<Controllable>& list = pRemote->GetControllableList();
            if (list.GetCount() > 0)
            {
                Controllable* pRemoteControl = list.GetFirst()->_pObject;
                if (pRemoteControl)
                {
                    EntitySoldier* pSoldier = PilotSoldier(mPilot);
                    if (pRemoteControl->mControlUse.IsPressed())
                    {
                        pSoldier->ExitVehicle(true, true);
                    }
                    else if (pRemoteControl->mControlSwitch[0] != 0.0f)
                    {
                        EntityRemoteTerminal* pTerminal = pRemoteControl->mControlSwitch[0] > 0.0f
                                                              ? GetNextAvailableTerminal(pSoldier)
                                                              : GetPrevAvailableTerminal(pSoldier);
                        if (pTerminal)
                        {
                            pSoldier->ExitVehicle(true, true);
                            pSoldier->EnterVehicle(reinterpret_cast<GameObject*>(pTerminal));
                        }
                    }
                }
            }
        }
    }

    PblVector3 vOffset(0.0f, 0.0f, 0.0f);
    mClass->UpdatePilotAnimation(mPilot, GetRenderPose(), mMatrix, vOffset, fDeltaTime, -1.0f, -1.0f, -1.0f);
    return true;
}

// FUNCTION: SWBF2 0x004dd160
void EntityRemoteTerminal::UpdateHealthRemote(float fAddHealth)
{
    GameObject* pRemote = GetRemoteGameObject();
    if (pRemote)
    {
        float fHealth = pRemote->mCurHealth;
        float fMaxHealth = pRemote->mMaxHealth;
        if (fAddHealth != 0.0f)
        {
            float fNewHealth = fHealth + fAddHealth;
            if (fNewHealth < 0.0f)
            {
                fNewHealth = 0.0f;
            }
            else if (fNewHealth > fMaxHealth)
            {
                fNewHealth = fMaxHealth;
            }
            pRemote->SetCurHealth(fNewHealth, true, 0);
            fHealth = pRemote->mCurHealth;
        }
        if (fHealth == fMaxHealth)
        {
            mIsAlive = 1;
        }

        float fOldHealth = mCurHealth;
        mMaxHealth = fMaxHealth;
        SetCurHealth(fHealth, false, 0);
        mHealthType = pRemote->mHealthType;
        if (fOldHealth != fHealth)
        {
            Damageable::HealthChangeCallback(false, fOldHealth - fHealth);
        }
    }
    else
    {
        mCurHealth = 0.0f;
    }

    if (mCurHealth <= 0.0f)
    {
        Controllable* pPilot = mPilot;
        if (pPilot)
        {
            pPilot->Kill();
        }
    }
}

// FUNCTION: SWBF2 0x004dd290
void EntityRemoteTerminal::HealthChangeCallback(bool bUnused, float fDelta)
{
    if (!GetRemoteGameObject())
    {
        mCurHealth = 0.0f;
        return;
    }
    mCurHealth -= fDelta;
    if (mCurHealth < 0.0f)
    {
        mCurHealth = 0.0f;
    }
    if (fDelta < 0.0f)
    {
        UpdateHealthRemote(-fDelta);
    }
}

// FUNCTION: SWBF2 0x004dd2f0
void EntityRemoteTerminal::SetupController()
{
    Controller* pController = 0;
    if (mPlayerId >= 0)
    {
        pController = new PlayerController(this);
        Track(mPlayerId);
    }
    else if (mPilot && !(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        pController = new UnitStationaryController(this);
    }
    if (pController)
    {
        AddController(pController, mPilot->mPlayerId < 0);
    }
    ClearControllable();
}

// FUNCTION: SWBF2 0x004dd5d0
void EntityRemoteTerminal::SetPilot(Controllable* pPilot)
{
    if (pPilot)
    {
        Controllable::SetPilot(pPilot);
        GameObject* pRemote = GetRemoteGameObject();
        if (pRemote)
        {
            for (PblList<Controllable>::Itor it(pRemote->GetControllableList()); (Controllable*)it; ++it)
            {
                if (it->mPilotType != PILOT_REMOTE)
                {
                    it->mPilotType = PILOT_REMOTE;
                }
            }
            pPilot->EnterRemote(pRemote);
        }
    }
    else
    {
        if (mPilot && mCharacter && mCharacter->mRemote)
        {
            mPilot->ExitRemote();
        }
        Controllable::SetPilot(0);
    }
}

// FUNCTION: SWBF2 0x004dd670
bool EntityRemoteTerminal::IsTerminalAvailable(const EntitySoldier* pSoldier)
{
    GameObject* pRemote = GetRemoteGameObject();
    return pRemote && pSoldier->CanEnterRemote(pRemote);
}

// FUNCTION: SWBF2 0x004dd6a0
EntityRemoteTerminal* EntityRemoteTerminal::GetNextAvailableTerminal(const EntitySoldier* pSoldier)
{
    int iCount = 0;
    EntityRemoteTerminal* pTerminal = GetNextLinkedTerminal();
    while (pTerminal && pTerminal != this && iCount < 20)
    {
        if (pTerminal->IsTerminalAvailable(pSoldier))
        {
            return pTerminal;
        }
        ++iCount;
        pTerminal = pTerminal->GetNextLinkedTerminal();
    }
    return 0;
}

// FUNCTION: SWBF2 0x004dd740
EntityRemoteTerminal* EntityRemoteTerminal::GetPrevAvailableTerminal(const EntitySoldier* pSoldier)
{
    for (EntityRemoteTerminal* pTerminal = GetPrevLinkedTerminal();
         pTerminal && pTerminal != this;
         pTerminal = pTerminal->GetPrevLinkedTerminal())
    {
        if (pTerminal->IsTerminalAvailable(pSoldier))
        {
            return pTerminal;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x004dd7c0
void EntityRemoteTerminal::ActivatePhysics()
{
    ActivateThread(0, PER_TURN);
}

// FUNCTION: SWBF2 0x004dd7e0
void EntityRemoteTerminal::DeactivatePhysics()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x004dd7f0
void EntityRemoteTerminal::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x5B36F215u:
        mPrevLinkedTerminalHash = PblHash(pszValue);
        GetPrevLinkedTerminal();
        break;
    case 0xB7BC02ADu:
        mNextLinkedTerminalHash = PblHash(pszValue);
        GetNextLinkedTerminal();
        break;
    case 0xD50DB89Eu:
        mRemoteGameObjectHash = PblHash(pszValue);
        GetRemoteGameObject();
        break;
    default:
        reinterpret_cast<GameObject*>(this)->GameObject::SetProperty(uProperty, pszValue);
        break;
    }
}

// GLOBAL: SWBF2 0x01e51e00
static unsigned int sSoftCollisionMask;
// GLOBAL: SWBF2 0x01e51dfc
static unsigned int sOrdnanceCollisionMask;

// FUNCTION: SWBF2 0x004dd970
void EntityRemoteTerminalClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x47c86b4a:
    {
        GameObjectClass::SetProperty(0x47c86b4a, pszValue);
        sSoftCollisionMask = 0;
        sOrdnanceCollisionMask = 0;
        int iNumBodies = mCollisionModel.GetNumBodies();
        const CollisionMesh* pMesh = CollisionMesh::Find(PblHash(pszValue));
        if (pMesh)
        {
            mCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "CollisionMesh", gMatrixIdentity, 0));
            int iMaskId = mCollisionModel.GetMaskId(0x29173102);
            if (iMaskId >= 0)
            {
                sSoftCollisionMask |= 1 << iMaskId;
                sOrdnanceCollisionMask |= 1 << iMaskId;
            }
        }
        if (iNumBodies == 0 && mModel)
        {
            const PblBox& box = mModel->_mModel->_TotalBoundingBox;
            PblVector3 vCenter = (box._vMin + box._vMax) * 0.5f;
            PblVector3 vHalf = (box._vMax - box._vMin) * 0.5f;
            CollisionPrimitive primitive;
            PblMatrix matrix = gMatrixIdentity;
            primitive.SetAACylinder((vHalf.x + vHalf.z) * 0.5f, vHalf.y * 2.0f);
            matrix.SetTranslation(vCenter);
            mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));
        }
        if (pMesh)
        {
            mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
        }
        mCollisionModel.mIsFlyerFlag = true;
        break;
    }
    case 0x5233b817:
        mVehicleType = GameObjectClass::FindVehicleType(pszValue);
        break;
    default:
        if (!ControllableClass::SetProperty(uProperty, pszValue, mModel ? mModel->GetRedModel(0) : 0))
        {
            GameObjectClass::SetProperty(uProperty, pszValue);
        }
        break;
    }
}







// FUNCTION: SWBF2 0x004dcc80
int EntityRemoteTerminalClass::GetVehicleType() const
{
    return mVehicleType;
}

// FUNCTION: SWBF2 0x004dd910
EntityRemoteTerminalClass::EntityRemoteTerminalClass(EntityRemoteTerminalClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash), ControllableClass(pBase)
{
    mVehicleType = 9;
    mLODClass = 1;
}

// SYNTHETIC: SWBF2 0x004dd8e0
// EntityRemoteTerminalClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004ddd00
EntityClass* EntityRemoteTerminalClass::Derive(unsigned int uHash)
{
    return new EntityRemoteTerminalClass(this, uHash);
}

// FUNCTION: SWBF2 0x004ddd70
Entity* EntityRemoteTerminalClass::Build(const EntityDesc& desc)
{
    if (EntityRemoteTerminal::sMemoryPool.mUsed >= EntityRemoteTerminal::sMemoryPool.mCount)
    {
        return 0;
    }

    EntityRemoteTerminal* pTerminal = new EntityRemoteTerminal(this, desc);
    if (pTerminal)
    {
        pTerminal->AddStaticToCollisionManager();
        pTerminal->ActivatePhysics();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            pTerminal->BuildEffects();
        }
    }
    return pTerminal;
}
