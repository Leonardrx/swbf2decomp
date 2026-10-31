#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "FlagItem.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameModel.h"
#include "EntityItem.h"
#include "Cloth.h"
#include "CameraManager.h"
#include "Team.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblList.h"
#include "NetGame.h"
#include "Character.h"
#include "Controllable.h"
#include "TriggerRegionManager.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "EventManager_Timer.h"
#include "EventInfo.h"
#include "ControllerManager.h"
#include "CollisionBody.h"
#include "CollisionManager.h"
#include "EntityLight.h"
#include "ItemTimerDisplay.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "EntityDroideka.h"
#include "HUDGameEvents.h"
#include "NetComm.h"
#include "PlayerStats.h"
#include "Trackable.h"
#include "RedEngineFL/Portals/Portal.h"
#include <d3dx9math.h>
#include <float.h>

extern PblVector3 gVectorZero;

namespace EventManager
{
namespace FlagEvent
{
extern Event<FlagItem, Character> mPickUp;
extern Event<FlagItem, Character> mDrop;
extern Event<FlagItem, char> mReset;
}

template <>
void Event<FlagItem, Character>::Callback::Trigger(FlagItem& item, Character& param);
template <>
void Event<FlagItem, char>::Callback::Trigger(FlagItem& item, char& param);
}

extern int gSpawnInvicibility;
namespace GameLoop
{
extern bool mbHistoricalRules;
}

namespace
{
CollisionObject* FlagCollision(FlagItem* pItem)
{
    return static_cast<CollisionObject*>(pItem);
}

}

// FUNCTION: SWBF2 0x00523fd0
GameObject* FlagItem::GetCarrier()
{
    FlagItem* pItem = this;
    Character* pCarrier = reinterpret_cast<EntityItem*>(pItem)->mOwner;
    if (!pCarrier)
    {
        return 0;
    }
    if (pCarrier->mVehicle)
    {
        return static_cast<Trackable*>(pCarrier->mVehicle)->GetGameObject();
    }
    if (pCarrier->mUnit)
    {
        return static_cast<Trackable*>(pCarrier->mUnit)->GetGameObject();
    }
    ResetFlag();
    return 0;
}

// FUNCTION: SWBF2 0x005246d0
bool FlagItem::Drop()
{
    FlagItem* pItem = this;
    if (!NetGame::IsEnabled() || !netEnabledNext || !netClientActive)
    {
        EventManager::FlagEvent::mDrop.mRoot.Trigger(*pItem, *reinterpret_cast<EntityItem*>(pItem)->mOwner);
    }

    AI::EventInfo event(AI::EVT_CTF_FlagDrop, reinterpret_cast<GameObject*>(pItem));
    ControllerManager::BroadcastEvent(&event, 1000);

    Character* pCarrier = reinterpret_cast<EntityItem*>(pItem)->mOwner;
    if (pCarrier && pCarrier->mUnit)
    {
        const PblVector3& vPos = pCarrier->mUnit->GetMatrix().GetTranslation();
        PblVector3 vDropPos(vPos.x + 0.0f, vPos.y + 1.0f, vPos.z + 0.0f);
        static_cast<CollisionObject*>(pItem)->SetMatrix(PblMatrix(pItem->mStartRotation, vDropPos));
        reinterpret_cast<EntityItem*>(pItem)->mVelocity.SetAll(0.0f);
    }

    pItem->mLastCarrier = reinterpret_cast<EntityItem*>(pItem)->mOwner;
    pItem->ClearCarrierFlagItem();
    reinterpret_cast<EntityItem*>(pItem)->mOwner = 0;
    reinterpret_cast<EntityItem*>(pItem)->mStationary = false;
    if (pItem->mDroppedInLandingZoneModel && pItem->IsInLandingZone())
    {
        reinterpret_cast<EntityGeometry*>(pItem)->SetModel(pItem->mDroppedInLandingZoneModel);
    }
    else if (pItem->mDroppedModel)
    {
        reinterpret_cast<EntityGeometry*>(pItem)->SetModel(pItem->mDroppedModel);
    }
    else
    {
        static_cast<RedSceneObject*>(pItem)->Activate();
        pItem->SetupCollisionGeometry();
    }

    if (reinterpret_cast<EntityItem*>(pItem)->m_pLight)
    {
        reinterpret_cast<EntityItem*>(pItem)->m_pLight->TurnOn();
    }
    if (pItem->mHomeRegion)
    {
        EntityItemClass* pClass = reinterpret_cast<EntityItem*>(pItem)->mClass;
        reinterpret_cast<EntityItem*>(pItem)->mLifeTimer = pClass->mLifeSpan;
        if (pClass->mLifeSpan < FLT_MAX)
        {
            ItemTimerDisplay::AddItemTimer(reinterpret_cast<EntityItem*>(pItem));
        }
    }
    else
    {
        reinterpret_cast<EntityItem*>(pItem)->mLifeTimer = 60.0f;
    }
    return true;
}

// FUNCTION: SWBF2 0x00524900
void FlagItem::ResetFlag()
{
    FlagItem* pItem = this;
    const PblMatrix mStart(pItem->mStartRotation, pItem->mStartPosition);
    SetMatrix(mStart);
    pItem->mLastPosition = pItem->mStartPosition;

    if (!NetGame::IsEnabled() || !netEnabledNext || !netClientActive)
    {
        EventManager::FlagEvent::mReset.mRoot.Trigger(*pItem, EventManager::noContext);
    }
    AI::EventInfo event(AI::EVT_CTF_FlagKill, reinterpret_cast<GameObject*>(pItem));
    ControllerManager::BroadcastEvent(&event, 1000);

    pItem->mLastCarrier = 0;
    pItem->ClearCarrierFlagItem();
    reinterpret_cast<EntityItem*>(pItem)->mOwner = 0;
    reinterpret_cast<EntityItem*>(pItem)->mStationary = false;
    if (pItem->mDroppedInLandingZoneModel && pItem->IsInLandingZone())
    {
        reinterpret_cast<EntityGeometry*>(pItem)->SetModel(pItem->mDroppedInLandingZoneModel);
    }
    else if (pItem->mDroppedModel)
    {
        reinterpret_cast<EntityGeometry*>(pItem)->SetModel(pItem->mDroppedModel);
    }
    else
    {
        static_cast<RedSceneObject*>(pItem)->Activate();
        pItem->SetupCollisionGeometry();
    }
    reinterpret_cast<EntityGeometry*>(pItem)->SetVelocity(gVectorZero);
    if (reinterpret_cast<EntityItem*>(pItem)->m_pLight)
    {
        reinterpret_cast<EntityItem*>(pItem)->m_pLight->TurnOn();
    }
    pItem->mIsAlive = true;
    pItem->mCurHealth = pItem->mMaxHealth;
    reinterpret_cast<EntityItem*>(pItem)->mLifeTimer = FLT_MAX;
    pItem->mMoved = false;
    if (reinterpret_cast<EntityItem*>(pItem)->mClass->mLifeSpan < FLT_MAX)
    {
        ItemTimerDisplay::RemoveItemTimer(reinterpret_cast<EntityItem*>(pItem));
    }
}

// FUNCTION: SWBF2 0x00524b80
void FlagItem::SetupCollisionGeometry()
{
    CollisionPrimitive primitive;
    primitive.SetSphere(mModel->_mModel->_BoundingSphere.GetRadius());

    PblMatrix matrix;
    matrix = gMatrixIdentity;
    memcpy(&matrix.m[3][0], &mModel->_mModel->_BoundingSphere.GetCenter(), sizeof(PblVector3));
    matrix.m[3][3] = 1.0f;

    CollisionModel* pModel = &mCollisionModel;
    pModel->ClearAll(true);
    pModel->AddBody(new CollisionBodyPrimitive(primitive, 0, matrix, 0));

    const float fRadius = mModel->_mModel->_BoundingSphere.GetRadius();
    SetCollisionRadius(fRadius);
    Set2DCollisionRadius(fRadius);
    SetCollisionCenter(mMatrix.GetTranslation());
    SetCollisionModel(pModel);
    RecalculateAABB();
    RemoveFromCollisionManager();
    AddItemToCollisionManager();
}

// FUNCTION: SWBF2 0x00524ab0
void FlagItem::ClearCarrierFlagItem()
{
    Character* pCarrier = reinterpret_cast<EntityItem*>(this)->mOwner;
    if (pCarrier)
    {
        pCarrier->mCarriedFlagPtr = 0;
    }
}

// FUNCTION: SWBF2 0x00524b10
bool FlagItem::IsInLandingZone() const
{
    TriggerRegionManager* pManager = TriggerRegionManager::sInstance;
    if (pManager->mLandingRegionsExist)
    {
        for (int i = 0; i < 32; ++i)
        {
            TriggerRegionManager::TriggerRegionData* pData = pManager->GetTriggerRegionData(i);
            if (pData && pData->mType == TriggerRegionManager::TRIGGER_LANDING &&
                pData->mRegion->IsPointInside(GetMatrix().GetTranslation()))
            {
                return true;
            }
        }
    }
    return false;
}

namespace EventManager
{

// FUNCTION: SWBF2 0x00525c70
template <>
void Event<FlagItem, Character>::Callback::Trigger(FlagItem& item, Character& param)
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

// FUNCTION: SWBF2 0x00525d90
template <>
void Event<FlagItem, char>::Callback::Trigger(FlagItem& item, char& param)
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

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x00525730
FlagItemClass::FlagItemClass(unsigned int uHash)
    : EntityItemClass(uHash)
{
    mCarriedModel = 0;
    mDroppedInLandingZoneModel = 0;
    mCarriedOffset = gVectorZero;
    memset(mTeamColorize, 0, sizeof(mTeamColorize));
    mCarriedSoldierBoneHash = 0xE5B51072u;
    mCarriedDroidekaBoneHash = 0x698AED8Au;
}

// SYNTHETIC: SWBF2 0x00403850
// `dynamic initializer for 'FlagItem::rttiHashFlagItem''
// FlagItem::rttiHashFlagItem

// GLOBAL: SWBF2 0x01ebc2b8
// FlagItem::rttiHashFlagItem
PblHash FlagItem::rttiHashFlagItem("FlagItem");
// GLOBAL: SWBF2 0x00789858
// FlagItem::rttiNameFlagItem
const char FlagItem::rttiNameFlagItem[] = "FlagItem";

// SYNTHETIC: SWBF2 0x00403860
// `dynamic initializer for 'FlagItemClass::rttiHashFlagItemClass''
// FlagItemClass::rttiHashFlagItemClass

// GLOBAL: SWBF2 0x01ebc2b4
// FlagItemClass::rttiHashFlagItemClass
PblHash FlagItemClass::rttiHashFlagItemClass("FlagItemClass");
// GLOBAL: SWBF2 0x00789848
// FlagItemClass::rttiNameFlagItemClass
const char FlagItemClass::rttiNameFlagItemClass[] = "FlagItemClass";

// SYNTHETIC: SWBF2 0x00403830
// `dynamic initializer for 'FlagItem::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768720
// `dynamic atexit destructor for 'FlagItem::sMemoryPool''

// GLOBAL: SWBF2 0x01ebcfa0
// FlagItem::sMemoryPool
MemoryPool FlagItem::sMemoryPool("FlagItem", 0x450);

// GLOBAL: SWBF2 0x007eb888
PblList<FlagItem> FlagItem::sHostList;
// GLOBAL: SWBF2 0x007eb89c
PblList<FlagItem> FlagItem::sClientList;
// GLOBAL: SWBF2 0x01e55f44
PblList<FlagItem>* FlagItem::sList;

// GLOBAL: SWBF2 0x01e55f1c
FlagItem::FlagGameplayType FlagItem::sFlagGameplayType;

// FUNCTION: SWBF2 0x005250f0
void FlagItem::RenderHelper(unsigned char ucLod, float fAlpha, unsigned int uiFlags)
{
    const FlagItemClass* pClass = static_cast<const FlagItemClass*>(mEntityClass);
    const bool bCarried = mOwner != 0;
    EntityCloth* const* pCloth = mClothPiece;

    if (pClass->mTeamColorize[bCarried])
    {
        int iLocal = NetGame::GetLocalIndex(CameraManager::sInstance->mUpdatedCameraId);
        int iSpectator = NetGame::GetSpectator(iLocal);
        if (iSpectator >= 0)
        {
            iLocal = iSpectator;
        }
        Character* pCharacter = Character::GetPtr(iLocal);
        Team* pTeam = pCharacter && pCharacter->mPlayerId == iLocal
                    ? pCharacter->mTeamPtr : Team::sTeams[1];
        const int iFlagTeam = mTeam;
        RedColor color = pTeam->GetColor(iFlagTeam);
        const float fLife = mLifeTimer;
        const float fFade = fLife + 1.0f > 1.0f ? 1.0f : fLife + 1.0f;
        color.a = static_cast<unsigned char>(static_cast<int>(color.a * fAlpha * fFade));
        const PblMatrix mWorld = mMatrix;
        if (mModel->mClothCount)
        {
            RedColor modelColor = RedColor::WHITE;
            modelColor.a = color.a;
            mModel->Render(ucLod, fAlpha, &mWorld, 0, modelColor, uiFlags, 0);
            for (unsigned int i = 0; i < mModel->mClothCount; ++i, ++pCloth)
            {
                if (*pCloth)
                {
                    (*pCloth)->Render(mWorld, 0, ucLod, color, uiFlags, 1);
                }
            }
        }
        else
        {
            mModel->Render(ucLod, fAlpha, &mWorld, 0, color, uiFlags, 0);
        }
    }
    else
    {
        reinterpret_cast<EntityItem*>(this)->EntityItem::Render(ucLod, fAlpha, uiFlags);
        if (mModel->mClothCount)
        {
            const PblMatrix mWorld = mMatrix;
            const float fLife = mLifeTimer;
            const float fFade = fLife + 1.0f > 1.0f ? 1.0f : fLife + 1.0f;
            const float fClothAlpha = fFade * fAlpha;
            for (unsigned int i = 0; i < mModel->mClothCount; ++i, ++pCloth)
            {
                if (*pCloth)
                {
                    RedColor color = RedColor::WHITE;
                    color.a = static_cast<unsigned char>(static_cast<int>(fClothAlpha * 255.0f));
                    (*pCloth)->Render(mWorld, 0, ucLod, color, uiFlags, 1);
                }
            }
        }
    }
}













// FUNCTION: SWBF2 0x00523fb0
EntityType FlagItem::GetEntityType() const
{
    return ENT_FLAG;
}

// FUNCTION: SWBF2 0x00523fc0
bool FlagItem::AddToTeamManager() const
{
    return false;
}

// FUNCTION: SWBF2 0x00524090
void FlagItem::Reset()
{
    sHostList.RemoveAll();
    sClientList.RemoveAll();
    sFlagGameplayType = (FlagGameplayType)0;
}

// TEMPLATE: SWBF2 0x00525bf0
// PblList<FlagItem>::RemoveAll
template void PblList<FlagItem>::RemoveAll();

// FUNCTION: SWBF2 0x00524460
void FlagItem::Create()
{
    sList = &sHostList;
}

// FUNCTION: SWBF2 0x005244c0
void FlagItem::Destroy()
{
}

void FlagItem::DestroyClient()
{
}

// FUNCTION: SWBF2 0x00524470
void FlagItem::CreateClient()
{
    if (!NetGame::IsHost() && NetGame::IsOneWorld())
    {
        while (sList->GetCount())
        {
            delete sList->GetFirst()->_pObject;
        }
    }
}

// FUNCTION: SWBF2 0x005244d0
void FlagItem::EnterClient()
{
    sList = &sClientList;
}

// FUNCTION: SWBF2 0x005244e0
void FlagItem::LeaveClient()
{
    sList = &sHostList;
}

// SYNTHETIC: SWBF2 0x005257c0
// FlagItemClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005257f0
FlagItemClass::FlagItemClass(FlagItemClass* pBase, unsigned int uHash)
    : EntityItemClass(pBase, uHash)
{
    mCarriedModel = pBase->mCarriedModel;
    mDroppedInLandingZoneModel = pBase->mDroppedInLandingZoneModel;
    mCarriedOffset = pBase->mCarriedOffset;
    memcpy(mTeamColorize, pBase->mTeamColorize, sizeof(mTeamColorize));
    mCarriedSoldierBoneHash = pBase->mCarriedSoldierBoneHash;
    mCarriedDroidekaBoneHash = pBase->mCarriedDroidekaBoneHash;
    mPickupSound = pBase->mPickupSound;
    mDropSound = pBase->mDropSound;
    mAmbientSound = pBase->mAmbientSound;
}

// FUNCTION: SWBF2 0x00525900
FlagItemClass::~FlagItemClass()
{
}

// FUNCTION: SWBF2 0x00525910
void FlagItemClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x05CEA8DE: /* "CarriedGeometryName" */
        mCarriedModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        break;
    case 0x0E0C4B60: /* "DroppedInLandingZoneGeometryName" */
        mDroppedInLandingZoneModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        break;
    case 0x58A48A9E: /* "CarriedOffset" */
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mCarriedOffset.x = x;
        mCarriedOffset.y = y;
        mCarriedOffset.z = z;
        break;
    }
    case 0xDA070BBA: /* "DroppedColorize" */
        mTeamColorize[0] = atoi(pszValue) != 0;
        break;
    case 0xCC229F56: /* "CarriedColorize" */
        mTeamColorize[1] = atoi(pszValue) != 0;
        break;
    case 0x759C8F54: /* "PickupSound" */
        mPickupSound.SetID(pszValue, false, false);
        break;
    case 0x5B2F29F3: /* "DropSound" */
        mDropSound.SetID(pszValue, false, false);
        break;
    case 0x65A920BA: /* "AmbientSound" */
        mAmbientSound.SetID(pszValue, false, false);
        break;
    case 0x826BFFDA: /* "SoldierAttachBone" */
        mCarriedSoldierBoneHash = PblTEMPHash(pszValue);
        break;
    case 0x4B80027B: /* "DroidekaAttachBone" */
        mCarriedDroidekaBoneHash = PblTEMPHash(pszValue);
        break;
    default:
        EntityItemClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00525ad0
EntityClass* FlagItemClass::Derive(unsigned int uHash)
{
    return new FlagItemClass(this, uHash);
}



// FUNCTION: SWBF2 0x005240b0
FlagItem::FlagItem(FlagItemClass* pClass, const EntityDesc& desc)
    : EntityItem(pClass, desc), mStartRotation(desc.mMatrix),
      mStartPosition(desc.mMatrix.GetTranslation())
{
    mLifeTimer = FLT_MAX;
    NetGame::Created(this);
    mDroppedModel = mModel;
    mCarriedModel = pClass->mCarriedModel;
    mDroppedInLandingZoneModel = pClass->mDroppedInLandingZoneModel;
    mLastCarrier = 0;
    SetupCollisionGeometry();
    mIsAlive = true;
    mCarriedTimer = FLT_MAX;
    mHomeRegion = 0;
    mAllowTeamPickup = true;
    mAllowAIPickup = true;
    mMoved = false;
    sList->Add(&mNode, this);

    SetPortalActive(false);
    pvs::IManager::GetInstance().UnregisterDynamic(*this);

    unsigned int i = 0;
    memset(mClothPiece, 0, sizeof(mClothPiece));
    for (; i < mModel->mClothCount; ++i)
    {
        EntityClass* pClothClass = EntityClass::Find(mModel->mClothList[i]);
        if (pClothClass)
        {
            mClothPiece[i] = static_cast<EntityCloth*>(pClothClass->Build(desc));
        }
    }

    mCarriedSoldierBoneHash = pClass->mCarriedSoldierBoneHash;
    mCarriedDroidekaBoneHash = pClass->mCarriedDroidekaBoneHash;
}

// FUNCTION: SWBF2 0x00524320
FlagItem::~FlagItem()
{
    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        delete mClothPiece[i];
    }
    ClearCarrierFlagItem();
    NetGame::Destroyed(this);
    mAmbientSound.Stop(true);
    sList->Remove(&mNode);
    if (mClass->mLifeSpan < FLT_MAX)
    {
        ItemTimerDisplay::RemoveItemTimer(this);
    }
}

// SYNTHETIC: SWBF2 0x005242f0
// FlagItem::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00525ea7
// ??_EFlagItem@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00525eb2
// ??_EFlagItem@@WCEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00525ebd
// ??_EFlagItem@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00525ec8
// ??_EFlagItem@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00525ed3
// ??_EFlagItem@@WM@AEPAXI@Z

// FUNCTION: SWBF2 0x005244f0
bool FlagItem::PickUp(GameObject* pObject)
{
    if (!NetGame::IsPreGame())
    {
        bool bDroidekaBusy = false;
        if (pObject->IsRtti(EntityDroideka::rttiHashEntityDroideka))
        {
            const int iState = reinterpret_cast<EntityDroideka*>(pObject)->mState;
            bDroidekaBusy = (iState == 12 || iState == 11 || iState == 13);
        }
        if (!bDroidekaBusy)
        {
            Controllable* pControllable = pObject->GetControllable();
            if (pControllable && (mAllowAIPickup || pControllable->mPlayerId >= 0))
            {
                Character* pCharacter = pControllable->mCharacter;
                if (pCharacter)
                {
                    for (PblList<FlagItem>::Itor i = sList->Begin(); i; ++i)
                    {
                        if (i->mOwner == pCharacter)
                        {
                            return false;
                        }
                    }
                    if (!pCharacter->mRemote && (IsASpaceMap() || !pCharacter->mVehicle) &&
                        !(pCharacter->mPlayerId >= 0 && NetGame::IsEnabled() &&
                          NetComm::GetTime() - pCharacter->mSpawnTime < (float)gSpawnInvicibility))
                    {
                        mOwner = pCharacter;
                        mStationary = true;
                        pCharacter->mCarriedFlagPtr = this;
                        Character::GetPlayerStats(mOwner).PointsProcess(0x1a, 0);
                        if (mCarriedModel)
                        {
                            SetModel(mCarriedModel);
                        }
                        else
                        {
                            static_cast<RedSceneObject*>(this)->Deactivate();
                        }
                        if (m_pLight)
                        {
                            m_pLight->TurnOff();
                        }
                        mLifeTimer = FLT_MAX;
                        mMoved = true;
                        if (mClass->mLifeSpan < FLT_MAX)
                        {
                            ItemTimerDisplay::RemoveItemTimer(this);
                        }

                        AI::EventInfo event(AI::EVT_CTF_FlagPickup, this);
                        ControllerManager::BroadcastEvent(&event, 1000);
                        if (sFlagGameplayType == FLAG_GAMEPLAY_1FLAG)
                        {
                            mCarriedTimer = 180.0f;
                        }
                        RemoveFromCollisionManager();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005248d0
void FlagItem::Kill()
{
    if (mHomeRegion)
    {
        ResetFlag();
        return;
    }
    mIsAlive = false;
    ClearCarrierFlagItem();
    GameObject::Kill();
}

// FUNCTION: SWBF2 0x00524ad0
void FlagItem::CheckFloatingInSpace()
{
    if (!mStationary && IsASpaceMap() && !IsInLandingZone())
    {
        mStationary = true;
        CollisionManager::DeactivateItem(this);
    }
}

// FUNCTION: SWBF2 0x00524b00
bool FlagItem::IsASpaceMap()
{
    return TriggerRegionManager::sInstance->mLandingRegionsExist;
}

// FUNCTION: SWBF2 0x00524d40
void FlagItem::BroadcastMessage(unsigned int uHash)
{
    if (NetGame::IsEnabled())
    {
        const bool bLocal = NetGame::IsClient() ||
                            (NetGame::IsOneWorld() && NetGame::IsHost() && !NetGame::IsDedicated());
        if (!NetGame::IsClient())
        {
            NetGame::CreateEvent_ScriptMessage(uHash, 0xffffffff);
        }
        if (!bLocal)
        {
            return;
        }
    }
    HUD::GameEvents::DisplayCenterMessage(uHash, 0xffffffff, &RedColor::YELLOW);
}

// FUNCTION: SWBF2 0x00524de0
bool FlagItem::Update(float fDeltaTime)
{
    if (!mIsAlive)
    {
        return false;
    }

    FlagItem* pItem = this;
    if (mOwner)
    {
        Controllable* pControllable = mOwner->mVehicle ? mOwner->mVehicle : mOwner->mUnit;
        if (pControllable)
        {
            GameObject* pCarrier = static_cast<Trackable*>(pControllable)->GetGameObject();
            if (pCarrier)
            {
                if (!pCarrier->IsAlive())
                {
                    pItem->Drop();
                }
                if (!NetGame::IsEnabled() || !netEnabledNext || !netClientActive)
                {
                    if (sFlagGameplayType == FLAG_GAMEPLAY_1FLAG)
                    {
                        mCarriedTimer -= fDeltaTime;
                        if (mCarriedTimer < 0.0f)
                        {
                            mCarriedTimer = FLT_MAX;
                            pItem->BroadcastMessage(0x8dbebf7f /* "game.flag.carriedtoolong" */);
                            pItem->ResetFlag();
                        }
                    }
                }
                if (!GameLoop::mbHistoricalRules && IsASpaceMap() && pItem->IsInLandingZone())
                {
                    pItem->ResetFlag();
                    if (sFlagGameplayType == FLAG_GAMEPLAY_1FLAG ||
                        sFlagGameplayType == FLAG_GAMEPLAY_2FLAG)
                    {
                        pItem->BroadcastMessage(0x7e6537bc /* "game.flag.reset" */);
                    }
                }
            }
        }
        else
        {
            pItem->ResetFlag();
        }
    }

    const PblVector3& vPos = mMatrix.GetTranslation();
    if (mAmbientSound.GetPlaying())
    {
        mAmbientSound.Update(&static_cast<FlagItemClass*>(mClass)->mAmbientSound, &vPos, &mVelocity);
    }
    else
    {
        mAmbientSound.Play(&static_cast<FlagItemClass*>(mClass)->mAmbientSound, &vPos, &mVelocity, true);
    }

    if (!EntityItem::Update(fDeltaTime) && !mOwner)
    {
        pItem->ResetFlag();
        if (sFlagGameplayType == FLAG_GAMEPLAY_1FLAG || sFlagGameplayType == FLAG_GAMEPLAY_2FLAG)
        {
            pItem->BroadcastMessage(0x7e6537bc /* "game.flag.reset" */);
        }
    }
    pItem->CheckFloatingInSpace();

    SetCollisionCenter(vPos);
    RecalculateAABB();
    return true;
}

// FUNCTION: SWBF2 0x00524fd0
bool FlagItem::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                 Restrictor* pRestrictor)
{
    if (IsAlive() && !mOwner &&
        (mStationary || mClass->mLifeSpan - mLifeTimer > 0.5f))
    {
        GameObject* pObject = pOther->GetGameObject();
        if (pObject && pObject->IsAlive() &&
            (((pObject->mTeam ^ mTeam) & 0xf) || (mAllowTeamPickup && mMoved)) &&
            PickUp(pObject))
        {
            EventManager::FlagEvent::mPickUp.Trigger(*this, *mOwner);
            return true;
        }
    }
    return EntityItem::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x005250b0
void FlagItem::InstantDeath(const DamageOwner&, bool)
{
    ResetFlag();
}

// FUNCTION: SWBF2 0x005250c0
void FlagItem::Render(unsigned char ucLod, float fAlpha, unsigned int uiFlags)
{
    if (!mOwner)
    {
        RenderHelper(ucLod, fAlpha, uiFlags);
    }
}

// FUNCTION: SWBF2 0x00525400
void FlagItem::SetModel(const GameModel* pModel)
{
    if (mModel == pModel)
    {
        EntityGeometry::SetModel(pModel);
    }
    else
    {
        for (unsigned int i = 0; i < mModel->mClothCount; ++i)
        {
            delete mClothPiece[i];
            mClothPiece[i] = 0;
        }
        EntityGeometry::SetModel(pModel);
        if (mModel->mClothCount)
        {
            EntityDesc desc;
            desc.mMatrix = static_cast<CollisionObject*>(this)->GetMatrix();
            for (unsigned int i = 0; i < mModel->mClothCount; ++i)
            {
                EntityClass* pClothClass = EntityClass::Find(mModel->mClothList[i]);
                if (pClothClass)
                {
                    mClothPiece[i] = static_cast<EntityCloth*>(pClothClass->Build(desc));
                }
            }
        }
    }
    SetupCollisionGeometry();
}

// FUNCTION: SWBF2 0x005255b0
void FlagItem::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x0D83E25A:
        mHomeRegionId = PblHash(pszValue);
        mHomeRegion = RedRegion::Find(mHomeRegionId);
        break;
    case 0x05CEA8DE:
        mCarriedModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        SetupCollisionGeometry();
        break;
    case 0x0E0C4B60:
        mDroppedInLandingZoneModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        SetupCollisionGeometry();
        break;
    case 0x47C86B4A:
        GameObject::SetProperty(uProperty, pszValue);
        mDroppedModel = mModel;
        SetupCollisionGeometry();
        break;
    case 0x4B80027B:
        mCarriedDroidekaBoneHash = PblTEMPHash(pszValue);
        break;
    case 0x826BFFDA:
        mCarriedSoldierBoneHash = PblTEMPHash(pszValue);
        break;
    case 0xDFDB1D8B:
        mAllowTeamPickup = atoi(pszValue) != 0;
        break;
    case 0xF412DC9C:
        mAllowAIPickup = atoi(pszValue) != 0;
        break;
    default:
        GameObject::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00525b40
Entity* FlagItemClass::Build(const EntityDesc& desc)
{
    if (FlagItem::sMemoryPool.IsFull())
    {
        return 0;
    }

    FlagItem* pFlag = new FlagItem(this, desc);
    if (pFlag)
    {
        pFlag->AddItemToCollisionManager();
        pFlag->ActivateThread(0, (ThreadType)2);
        pFlag->BuildEffects();
    }
    return pFlag;
}
