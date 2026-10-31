#include <d3dx9math.h>
#include "EntityDoor.h"
#include "PebbleFL/Common/PblVector.h"

#include <stdio.h>
#include <stdlib.h>

#include "NetGame.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "GameModel.h"
#include "GameObject.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Portals/Portal.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"

extern bool netFrameLock;


// FUNCTION: SWBF2 0x00498460
void EntityDoor::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!mAnimatedClass->m_bDynamicLighting)
    {
        uiFlags |= 0x80u;
    }
    else
    {
        uiFlags &= ~0x80u;
    }
    SetupShadow(0, mModel, ucViewport, uiFlags, 0);

    float fPoseTime = mZephyrPoseDyn.m_fCurT;
    if (NetGame::IsEnabled() && !netFrameLock && !mIsLocked)
    {
        const float fRenderDt = mRenderDt;
        float fStep = fRenderDt * NetGame::GetTurnRatio();
        float fInterpolated;
        if (mIsOpen)
        {
            fInterpolated = mLastAnimTime + fStep;
            fInterpolated = 1.0f > fInterpolated ? fInterpolated : 1.0f;
        }
        else
        {
            fInterpolated = mLastAnimTime - fStep;
            fInterpolated = fInterpolated > 0.0f ? fInterpolated : 0.0f;
        }
        mZephyrPoseDyn.SetCurTime(fInterpolated);
        mZephyrPoseDyn.Update(0.0f);
        ZephyrPoseStatic<32> kPose;
        mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
        kPose.Open(mZephyrSkeleton);
        kPose.Set(mZephyrPoseDyn, true);
        mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }

    const GameModel* pModel = GetModel();
    const RedColor kWhite(255, 255, 255, 255);
    pModel->Render(ucViewport, fDist, &mMatrix, &mPose, kWhite,
                   uiFlags | 0x01000000u, 0);
    if (!(uiFlags & 0x00200000u))
    {
        PblMatrix reflection;
        if (FLRenderer::IsReflected(GetCollisionCenter(), GetCollisionRadius(), reflection, false))
        {
            reflection = mMatrix * reflection;
            pModel->Render(ucViewport, fDist, &reflection, &mPose, RedColor::WHITE,
                           (uiFlags & 0xeffefeffu) | 0x10000100u, 0);
        }
    }

    if (NetGame::IsEnabled() && !netFrameLock && !mIsLocked)
    {
        mZephyrPoseDyn.SetCurTime(fPoseTime);
        mZephyrPoseDyn.Update(0.0f);
        ZephyrPoseStatic<32> kPose;
        mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
        kPose.Open(mZephyrSkeleton);
        kPose.Set(mZephyrPoseDyn, true);
        mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }
    mJustRendered = true;
}

// FUNCTION: SWBF2 0x004982a0
EntityDoor::~EntityDoor()
{
}

// SYNTHETIC: SWBF2 0x00497fc0
// EntityDoor::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004992b4
// ??_EEntityDoor@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004992bf
// ??_EEntityDoor@@WBIA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004992ca
// ??_EEntityDoor@@WM@AEPAXI@Z


namespace AI
{

enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};
}

class Team;

namespace ControllerManager
{

int GetUnitsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                    int iMax, const Team* pTeam, AI::AIAffil eAffil);
}

// SYNTHETIC: SWBF2 0x004029c0
// `dynamic initializer for 'EntityDoor::rttiHashEntityDoor''
// EntityDoor::rttiHashEntityDoor

// GLOBAL: SWBF2 0x01ebb998
// EntityDoor::rttiHashEntityDoor
PblHash EntityDoor::rttiHashEntityDoor("EntityDoor");
// GLOBAL: SWBF2 0x00789130
// EntityDoor::rttiNameEntityDoor
const char EntityDoor::rttiNameEntityDoor[] = "EntityDoor";

// SYNTHETIC: SWBF2 0x004029d0
// `dynamic initializer for 'EntityDoorClass::rttiHashEntityDoorClass''
// EntityDoorClass::rttiHashEntityDoorClass

// GLOBAL: SWBF2 0x01ebb99c
// EntityDoorClass::rttiHashEntityDoorClass
PblHash EntityDoorClass::rttiHashEntityDoorClass("EntityDoorClass");
// GLOBAL: SWBF2 0x00789118
// EntityDoorClass::rttiNameEntityDoorClass
const char EntityDoorClass::rttiNameEntityDoorClass[] = "EntityDoorClass";
// SYNTHETIC: SWBF2 0x004029e0
// `dynamic initializer for 'EntityDoor::msHostDoors''

// GLOBAL: SWBF2 0x01ebb9a0
PblHashTable<EntityDoor, 32> EntityDoor::msHostDoors;
// SYNTHETIC: SWBF2 0x00402a00
// `dynamic initializer for 'EntityDoor::msClientDoors''

// GLOBAL: SWBF2 0x01ebbaa8
PblHashTable<EntityDoor, 32> EntityDoor::msClientDoors;

// GLOBAL: SWBF2 0x007e6338
PblHashTable<EntityDoor, 32>* EntityDoor::mspDoors = &EntityDoor::msHostDoors;

// GLOBAL: SWBF2 0x01ebbbac
PblHashTable<EntityDoor, 32>::Itor EntityDoor::msDoorIter;

// FUNCTION: SWBF2 0x00498dd0
void EntityDoor::ResetDoorList()
{
    mspDoors = &msHostDoors;
    msHostDoors.Clear();
    msClientDoors.Clear();
}

// FUNCTION: SWBF2 0x00498f00
EntityDoorClass::EntityDoorClass(unsigned int uHash)
    : EntityPropClass(uHash)
{
    mAnimObj = 0;
    mAnimation = 0;
    memset(mAnimationTriggers, 0, sizeof(mAnimationTriggers));
}

// FUNCTION: SWBF2 0x00497f60
unsigned int EntityDoor::GetRtti()
{
    return rttiHashEntityDoor;
}





// FUNCTION: SWBF2 0x00497f90
EntityClass* EntityDoor::GetEntityClass()
{
    return (EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x00497fa0
const EntityClass* EntityDoor::GetEntityClass() const
{
    return (const EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x00497fb0
const RedPose* EntityDoor::GetPose() const
{
    return &mPose;
}



// FUNCTION: SWBF2 0x00498040
EntityDoor::EntityDoor(EntityDoorClass* pClass, const EntityDesc& desc)
    : EntityProp(pClass, desc)
{
    hasPortal_ = true;
    portal_ = 0;
    mAnimObj = pClass->mAnimObj;
    mOpenTrigger = false;
    mIsOpen = false;
    mIsLocked = false;
    mJustRendered = false;
    mAnimatedClass = pClass;
    mTimeSinceLastTriggered = 500.0f;
    mLastAnimTime = 0.0f;
    mRenderDt = 0.0f;
    if (mShadowProxy)
    {
        mShadowProxy->RemoveFromScene();
    }

    const ZephyrSkeletonShared* pShared = mAnimatedClass->mAnimObj->GetZephyrSkeletonShared();
    ZephyrPoseStatic<32> kPose;
    mZephyrSkeleton.Open(*pShared, kPose);
    kPose.Open(mZephyrSkeleton);
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mZephyrPoseDyn.SetLoop(false);
    mZephyrPoseDyn.SetAnimation(mAnimatedClass->mAnimation, 30.0f);
    mZephyrPoseDyn.SetCurTime(0.0f);
    mZephyrPoseDyn.Update(0.0f);
    kPose.Set(mZephyrPoseDyn, true);
    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);

    if (mId && !mspDoors->Find(mId))
    {
        mspDoors->Store(mId, this);
    }
}

// FUNCTION: SWBF2 0x00498330
void EntityDoor::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x295eb940:
    {
        float fRatio;
        sscanf(pszValue, "%f", &fRatio);
        mZephyrPoseDyn.SetCurTime(fRatio);
        mZephyrPoseDyn.Update(0.0f);
        break;
    }

    case 0x9dd5da63:
        mIsLocked = atoi(pszValue) != 0;
        if (NetGame::IsEnabled())
        {
            NetGame::CreateEvent_UpdateDoorState(reinterpret_cast<EntityEx*>(this)->mId,
                                                 0xffffffffffffffffULL);
        }
        break;

    case 0xd55de4cf:
        mOpenTrigger = atoi(pszValue) != 0;
        break;

    default:
        reinterpret_cast<EntityProp*>(this)->EntityProp::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00498850
bool EntityDoor::Update(float fDeltaTime)
{
    if (hasPortal_ && !portal_)
    {
        const PblSphere sphere = GetCollisionSphere();
        const geo::Sphere kSphere(geo::Vector3<float>(sphere.x, sphere.y, sphere.z), sphere.w);
        portal_ = pvs::IManager::GetInstance().FindNearestPortal(kSphere, 10.5f);
        hasPortal_ = portal_ != 0;
    }

    float fTime = mTimeSinceLastTriggered + fDeltaTime;
    mTimeSinceLastTriggered = fTime < 500.0f ? fTime : 500.0f;

    bool bTriggered = mOpenTrigger;
    if (!NetGame::IsEnabled() && ++mTriggerSkipCnt != 0)
    {
        bTriggered = mTriggerSkipLastResult;
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            if (!mAnimatedClass->mAnimationTriggers[i].mTriggerPoint)
            {
                continue;
            }
            PblMatrix mBone;
            if (!GetModel()->GetRedModel(0)->GetBoneMatrix(mAnimatedClass->mAnimationTriggers[i].mTriggerPoint, mBone))
            {
                continue;
            }
            const PblMatrix mWorld = mBone * mMatrix;

            GameObject* apUnits[20];
            int iCount;
            if (NetGame::IsEnabled() && netEnabledNext && netClientActive)
            {
                iCount = NetGame::GetObjectsInRange(mWorld.GetTranslation(),
                                    mAnimatedClass->mAnimationTriggers[i].mTriggerRadius, apUnits, 20, 0,
                                    COLL_SOFT);
            }
            else
            {
                iCount = ControllerManager::GetUnitsInRange(mWorld.GetTranslation(),
                                                            mAnimatedClass->mAnimationTriggers[i].mTriggerRadius,
                                                            apUnits, 20, 0, AI::AFFIL_ALL);
            }
            for (int j = 0; j < iCount; ++j)
            {
                if (apUnits[j]->GetObjectData(0) == 2)
                {
                    bTriggered = true;
                    break;
                }
            }
        }
        mTriggerSkipLastResult = bTriggered;
    }

    float fLastTime = mZephyrPoseDyn.m_fCurT;
    float fStep = (fDeltaTime * 30.0f) / (float)(mAnimatedClass->mAnimation->m_u16NumFrames - 1);
    if (bTriggered)
    {
        if (mIsLocked)
        {
            if (mTimeSinceLastTriggered > 3.0f)
            {
                mAnimatedClass->mLockedSound.Play(&mMatrix.GetTranslation(), 0, false, true);
            }
        }
        else
        {
            if (mJustRendered || !mIsOpen)
            {
                mLastAnimTime = fLastTime;
                mRenderDt = fStep;
                mJustRendered = false;
            }
            else
            {
                mRenderDt += fStep;
            }
            float fTimeNew = fLastTime + fStep;
            mZephyrPoseDyn.SetCurTime(1.0f > fTimeNew ? fTimeNew : 1.0f);
            if (!mIsOpen)
            {
                mAnimatedClass->mOpenSound.Play(&mMatrix.GetTranslation(), 0, false, true);
                mIsOpen = true;
            }
        }
    }
    else
    {
        if (mIsOpen || mJustRendered)
        {
            mLastAnimTime = fLastTime;
            mRenderDt = fStep;
            mJustRendered = false;
        }
        else
        {
            mRenderDt += fStep;
        }
        float fTimeNew = fLastTime - fStep;
        mZephyrPoseDyn.SetCurTime(fTimeNew > 0.0f ? fTimeNew : 0.0f);
        if (mIsOpen)
        {
            mAnimatedClass->mCloseSound.Play(&mMatrix.GetTranslation(), 0, false, true);
            mIsOpen = false;
        }
    }

    if (portal_)
    {
        static_cast<pvs::IPortal*>(portal_)->SetActive(mZephyrPoseDyn.m_fCurT > 0.0f);
    }

    if (mIsOpen || fLastTime - mZephyrPoseDyn.m_fCurT > 0.0f)
    {
        mZephyrPoseDyn.Update(0.0f);
        ZephyrPoseStatic<32> kPose;
        mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
        kPose.Open(mZephyrSkeleton);
        kPose.Set(mZephyrPoseDyn, true);
        mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }

    if (bTriggered)
    {
        mTimeSinceLastTriggered = 0.0f;
    }
    return true;
}







// FUNCTION: SWBF2 0x00498e10
void EntityDoor::EnterClient()
{
    if (!NetGame::IsOneWorld())
    {
        mspDoors = &msClientDoors;
    }
}

// FUNCTION: SWBF2 0x00498e30
void EntityDoor::LeaveClient()
{
    if (!NetGame::IsOneWorld())
    {
        mspDoors = &msHostDoors;
    }
}

// FUNCTION: SWBF2 0x00498e50
EntityDoor* EntityDoor::Find(unsigned int uHash)
{
    return mspDoors->Find(uHash);
}

// FUNCTION: SWBF2 0x00498e70
void EntityDoor::BeginIter()
{
    msDoorIter = mspDoors->Begin();
}

// FUNCTION: SWBF2 0x00498ea0
EntityDoor* EntityDoor::GetCurrentIter()
{
    EntityDoor* pDoor = msDoorIter;
    if (pDoor != mspDoors->End())
    {
        return pDoor;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00498ed0
void EntityDoor::IncrementIter()
{
    ++msDoorIter;
}

// SYNTHETIC: SWBF2 0x00498f70
// EntityDoorClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00498fa0
EntityDoorClass::EntityDoorClass(EntityDoorClass* pBase, unsigned int uHash)
    : EntityPropClass(pBase, uHash)
{
    mAnimObj = pBase->mAnimObj;
    mAnimation = pBase->mAnimation;
    memcpy(mAnimationTriggers, pBase->mAnimationTriggers, sizeof(mAnimationTriggers));
}

// FUNCTION: SWBF2 0x00499020
EntityDoorClass::~EntityDoorClass()
{
}

// FUNCTION: SWBF2 0x00499030
bool EntityDoorClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    return mAnimObj ? true : false;
}

// FUNCTION: SWBF2 0x00499070
void EntityDoorClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x323f7450: /* "OpenSound" */
        mOpenSound.SetID(pszValue, false, false);
        break;

    case 0x4fcf4562: /* "CloseSound" */
        mCloseSound.SetID(pszValue, false, false);
        break;

    case 0x027b5cb2: /* "LockedSound" */
        mLockedSound.SetID(pszValue, false, false);
        break;

    case 0x1d56b73b: /* "AnimationTrigger" */
    {
        char acName[256];
        float fRadius;
        sscanf(pszValue, "%s %f", acName, &fRadius);
        for (int i = 0; i < 3; i++)
        {
            if (!mAnimationTriggers[i].mTriggerPoint)
            {
                mAnimationTriggers[i].mTriggerPoint = PblTEMPHash(acName);
                mAnimationTriggers[i].mTriggerRadius = fRadius;
                break;
            }
        }
        break;
    }

    case 0xe145ee5d: /* "Animation" */
        if (mAnimObj)
        {
            mAnimation = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
        }
        break;

    case 0x98557a5e: /* "AnimationName" */
        InitAnimations(pszValue);
        break;

    default:
        EntityPropClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004991c0
EntityClass* EntityDoorClass::Derive(unsigned int uHash)
{
    return new EntityDoorClass(this, uHash);
}

// FUNCTION: SWBF2 0x00499230
Entity* EntityDoorClass::Build(const EntityDesc& desc)
{
    EntityDoor* pDoor = new EntityDoor(this, desc);
    pDoor->AddStaticToCollisionManager();
    pDoor->ActivateThread(0, PER_LOCAL_TURN);
    pDoor->BuildEffects();
    return pDoor;
}
