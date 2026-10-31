#include <d3dx9math.h>
#include "EntityPropAnimated.h"
#include "PebbleFL/Common/PblVector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CollisionObject.h"
#include "SoldierAnimationBank.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "CollisionManager.h"
#include "Damageable.h"
#include "EntitySoldier.h"
#include "FirstPerson.h"
#include "GameModel.h"
#include "Team.h"
#include "RedEngineFL/Graphics/RedModel.h"


// TEMPLATE: SWBF2 0x004dc8e0
// PblHandle<GameObject>::operator()

// FUNCTION: SWBF2 0x004dc060
EntityPropAnimatedClass::EntityPropAnimatedClass(unsigned int uHash)
    : EntityPropClass(uHash)
{
    mAnimObj = 0;
    memset(mAnimations, 0, sizeof(mAnimations));
    memset(mAnimationTriggers, 0, sizeof(mAnimationTriggers));
    memset(mAttachTriggers, 0, sizeof(mAttachTriggers));
    for (int i = 0; i < 3; i++)
    {
        mAttachTriggers[i].mSoldierAnimation = -1;
    }
    mDisableForCloneWars = false;
    mTrackDeathOnAttach = false;
    mIdleDelay = 0.0f;
}



// SYNTHETIC: SWBF2 0x00402cf0
// `dynamic initializer for 'EntityPropAnimatedClass::rttiHashEntityPropAnimatedClass''
// EntityPropAnimatedClass::rttiHashEntityPropAnimatedClass

// GLOBAL: SWBF2 0x01ebc044
// EntityPropAnimatedClass::rttiHashEntityPropAnimatedClass
PblHash EntityPropAnimatedClass::rttiHashEntityPropAnimatedClass("EntityPropAnimatedClass");
// GLOBAL: SWBF2 0x007894b0
// EntityPropAnimatedClass::rttiNameEntityPropAnimatedClass
const char EntityPropAnimatedClass::rttiNameEntityPropAnimatedClass[] = "EntityPropAnimatedClass";

// SYNTHETIC: SWBF2 0x00402ce0
// `dynamic initializer for 'EntityPropAnimated::rttiHashEntityPropAnimated''
// EntityPropAnimated::rttiHashEntityPropAnimated

// GLOBAL: SWBF2 0x01ebc048
// EntityPropAnimated::rttiHashEntityPropAnimated
PblHash EntityPropAnimated::rttiHashEntityPropAnimated("EntityPropAnimated");
// GLOBAL: SWBF2 0x007894c8
// EntityPropAnimated::rttiNameEntityPropAnimated
const char EntityPropAnimated::rttiNameEntityPropAnimated[] = "EntityPropAnimated";

// FUNCTION: SWBF2 0x004db3c0
EntityPropAnimated::EntityPropAnimated(EntityPropAnimatedClass* pClass, const EntityDesc& desc)
    : EntityProp(pClass, desc), Thread(), mAttachedSoldier()
{
    mAnimObj = pClass->mAnimObj;
    mAnimatedClass = pClass;
    mBlendFactor = 1.0f;
    mAnimationId = -1;
    if (!pClass->mAnimations[0] || pClass->mDisableForCloneWars)
    {
        mAnimationTime = 0.0f;
    }
    else
    {
        mAnimationTime = mAnimatedClass->mAnimations[0]->m_u16NumFrames *
                         PblRandom::_Global.GetFloat() / 30.0f;
    }
    if (mShadowProxy)
    {
        mShadowProxy->Deactivate();
    }
    mZephyrSkeleton.Open(*mAnimatedClass->mAnimObj->GetZephyrSkeletonShared(), mZephyrPoseStatic);
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mAttachedSoldier = (GameObject*)0;
    mAttachedSoldierAnimation = -1;
    mAttachMatrix = 0;
    mIdleDelayTimer = 0.0f;
}

// FUNCTION: SWBF2 0x004db5d0
EntityPropAnimated::~EntityPropAnimated()
{
}

// FUNCTION: SWBF2 0x004db670
void EntityPropAnimated::Render(unsigned char ucLod, float fAlpha, unsigned int uFlags)
{
    if (!static_cast<EntityPropAnimatedClass*>(mEntityClass)->m_bDynamicLighting)
    {
        uFlags |= 0x80u;
    }
    else
    {
        uFlags &= ~0x80u;
    }

    // GLOBAL: SWBF2 0x01faa92c
    // sPlayerShadow
    static PblHash sPlayerShadow("player_shadow");
    SetupShadow(0, mModel, ucLod, uFlags, sPlayerShadow);
    const GameModel* pModel = GetModel();
    const RedColor kWhite(255, 255, 255, 255);
    pModel->Render(ucLod, fAlpha, &mMatrix, &mPose, kWhite,
                   uFlags | 0x01000000, 0);
}

// SYNTHETIC: SWBF2 0x004db340
// EntityPropAnimated::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004992da
// ?GetGameObject@EntityGeometry@@WM@BEPBVGameObject@@XZ
// SYNTHETIC: SWBF2 0x004dc901
// ??_EEntityPropAnimated@@WBIA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004dc90c
// ??_EEntityPropAnimated@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004dc914
// ??_EEntityPropAnimated@@WJE@AEPAXI@Z









// SYNTHETIC: SWBF2 0x004dc150
// EntityPropAnimatedClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004dc180
EntityPropAnimatedClass::EntityPropAnimatedClass(EntityPropAnimatedClass* pBase, unsigned int uHash)
    : EntityPropClass(pBase, uHash)
{
    mAnimObj = pBase->mAnimObj;
    memcpy(mAnimations, pBase->mAnimations, sizeof(mAnimations));
    memcpy(mAnimationTriggers, pBase->mAnimationTriggers, sizeof(mAnimationTriggers));
    memcpy(mAttachTriggers, pBase->mAttachTriggers, sizeof(mAttachTriggers));
    for (int i = 0; i < 3; i++)
    {
        mAttachTriggers[i].mSoldierAnimation = -1;
    }
    mDisableForCloneWars = pBase->mDisableForCloneWars;
    mTrackDeathOnAttach = pBase->mTrackDeathOnAttach;
    mIdleDelay = pBase->mIdleDelay;
}

// FUNCTION: SWBF2 0x004dc2e0
EntityPropAnimatedClass::~EntityPropAnimatedClass()
{
}

// FUNCTION: SWBF2 0x004dc340
bool EntityPropAnimatedClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    return mAnimObj ? true : false;
}

// FUNCTION: SWBF2 0x004dc380
bool EntityPropAnimatedClass::SetupAnimation(ZephyrPoseDyn<32>& kPose, int iAnim, float fTime, bool bLoop)
{
    if (iAnim < 0)
    {
        return false;
    }
    const ZephyrAnim* pAnim = mAnimations[iAnim];
    if (!pAnim)
    {
        return false;
    }

    kPose.SetLoop(bLoop);
    float fLength = pAnim->m_u16NumFrames / 30.0f;
    if (fTime > fLength)
    {
        fTime -= (int)(fTime / fLength) * fLength;
    }
    kPose.SetAnimation(pAnim, 30.0f);
    kPose.SetCurTime(fTime);
    kPose.Update(0.0f);
    return true;
}

// FUNCTION: SWBF2 0x004dc430
void EntityPropAnimatedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x98557a5e:
        InitAnimations(pszValue);
        break;

    case 0xe145ee5d:
        if (mAnimObj)
        {
            for (int i = 0; i < 5; i++)
            {
                if (!mAnimations[i])
                {
                    mAnimations[i] = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
                    return;
                }
            }
        }
        break;

    case 0x762ecc6a:
    {
        char acAnimation[256];
        char acSound[256];
        sscanf("%255s %255s\n", acAnimation, acSound);
        unsigned int uAnimation = PblTEMPHash(acAnimation);
        for (int i = 0; i < 5; i++)
        {
            if (mAnimations[i] && uAnimation == mAnimations[i]->GetNameCRC())
            {
                mAnimationSound[i].SetID(acSound, false, false);
                return;
            }
        }
        break;
    }

    case 0x72306628:
    {
        char acAttachPoint[256];
        char acAnimation[256];
        char acSoldierAnimation[256];
        char acUnused[256];
        char acEvent[256];
        float fRadius;
        float fTime;
        sscanf(pszValue, "%s %f %s %s %f %s %s", acAttachPoint, &fRadius, acAnimation,
               acSoldierAnimation, &fTime, acUnused, acEvent);
        for (int i = 0; i < 3; i++)
        {
            if (!mAttachTriggers[i].mAttachPoint)
            {
                for (int j = 0; j < 5; j++)
                {
                    if (mAnimations[j] && mAnimations[j]->GetNameCRC() == PblTEMPHash(acAnimation))
                    {
                        mAttachTriggers[i].mAnimationID = j;
                        break;
                    }
                }
                mAttachTriggers[i].mSoldierAnimation = SoldierAnimationBank::AddAnimation(acSoldierAnimation);
                mAttachTriggers[i].mAttachPoint = PblTEMPHash(acAttachPoint);
                mAttachTriggers[i].mAttachRadius = fRadius;
                mAttachTriggers[i].mAnimationTime = fTime;
                PblTEMPHash(acEvent);
                mAttachTriggers[i].mPostAttachEvent = 0;
                return;
            }
        }
    }

    case 0x1d56b73b:
    {
        char acAnimation[256];
        char acTriggerPoint[256];
        float fRadius;
        sscanf(pszValue, "%s %s %f", acAnimation, acTriggerPoint, &fRadius);
        for (int i = 0; i < 3; i++)
        {
            if (!mAnimationTriggers[i].mTriggerPoint)
            {
                for (int j = 0; j < 5; j++)
                {
                    if (mAnimations[j] && mAnimations[j]->GetNameCRC() == PblTEMPHash(acAnimation))
                    {
                        mAnimationTriggers[i].mAnimationID = j;
                        break;
                    }
                }
                mAnimationTriggers[i].mTriggerPoint = PblTEMPHash(acTriggerPoint);
                mAnimationTriggers[i].mTriggerRadius = fRadius;
                return;
            }
        }
        break;
    }

    case 0x6b014234:
        mDisableForCloneWars = atoi(pszValue) != 0;
        break;

    case 0x0550a3b2:
        mTrackDeathOnAttach = atoi(pszValue) != 0;
        break;

    case 0xf949d898:
        mKillSoldierSound.SetID(pszValue, false, false);
        break;

    case 0xff701082:
        sscanf(pszValue, "%f", &mIdleDelay);
        break;

    default:
        EntityPropClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004dc7e0
EntityClass* EntityPropAnimatedClass::Derive(unsigned int uHash)
{
    return new EntityPropAnimatedClass(this, uHash);
}

// FUNCTION: SWBF2 0x004dc850
Entity* EntityPropAnimatedClass::Build(const EntityDesc& desc)
{
    EntityPropAnimated* pProp = new EntityPropAnimated(this, desc);
    pProp->AddStaticToCollisionManager();
    pProp->ActivateThread(0, PER_LOCAL_TURN);
    pProp->BuildEffects();
    return pProp;
}

// FUNCTION: SWBF2 0x004db2e0
unsigned int EntityPropAnimated::GetRtti()
{
    return rttiHashEntityPropAnimated;
}





// FUNCTION: SWBF2 0x004db310
EntityClass* EntityPropAnimated::GetEntityClass()
{
    return (EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x004db320
const EntityClass* EntityPropAnimated::GetEntityClass() const
{
    return (const EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x004db330
const RedPose* EntityPropAnimated::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x004db760
void EntityPropAnimated::SetAnimation(unsigned int uAnimationId, bool bLoop)
{
    if (uAnimationId == mAnimationId)
    {
        return;
    }

    mLooping = bLoop;
    mAnimatedClass->SetupAnimation(mZephyrPoseDyn, uAnimationId, mAnimationTime, bLoop);
    if (mBlendFactor >= 1.0f)
    {
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
    }
    else
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDyn, mBlendFactor);
    }
    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    mPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    mAnimationId = uAnimationId;
}

// FUNCTION: SWBF2 0x004db800
bool EntityPropAnimated::Update(float fDeltaTime)
{
    if (mAnimationId == (unsigned int)-1)
    {
        SetAnimation(0, true);
        mZephyrPoseDyn.Update(0.0f);
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
        mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
        mPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    }

    if (mAnimatedClass->mDisableForCloneWars)
    {
        Team* pTeam = Team::GetPtr(1);
        if (pTeam && (pTeam->GetSide() == Team::side_CIS || pTeam->GetSide() == Team::side_Republic))
        {
            return true;
        }
    }

    if (!mLooping)
    {
        const ZephyrAnim* pAnim = mAnimatedClass->mAnimations[mAnimationId];
        if (pAnim && mAnimationTime > pAnim->m_u16NumFrames / 30.0f)
        {
            mAnimationTime = 0.0f;
            mBlendFactor = 0.0f;
            if (mAttachedSoldier)
            {
                GameObject* pSoldier = &mAttachedSoldier();
                pSoldier->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
                const PblVector3* pPos = (const PblVector3*)GetMatrix().m[3];
                mAnimatedClass->mKillSoldierSound.Play(pPos, 0, false, true);
            }
            mAttachedSoldier = (GameObject*)0;
            mAttachedSoldierAnimation = -1;
            mAttachMatrix = 0;
            SetAnimation(0, true);
        }
    }
    else if (mAttachedSoldier)
    {
        mAttachedSoldier->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
        const PblVector3* pPos = (const PblVector3*)GetMatrix().m[3];
        mAnimatedClass->mKillSoldierSound.Play(pPos, 0, false, true);
    }

    for (int i = 0; i < 3; ++i)
    {
        if (!mLooping || mAttachedSoldier)
        {
            continue;
        }
        if (!mAnimatedClass->mAnimationTriggers[i].mTriggerPoint ||
            mAnimationId == mAnimatedClass->mAnimationTriggers[i].mAnimationID || !(mBlendFactor >= 0.5f))
        {
            continue;
        }
        PblMatrix mBone;
        if (!GetModel()->GetRedModel(0)->GetBoneMatrix(mAnimatedClass->mAnimationTriggers[i].mTriggerPoint, mBone))
        {
            continue;
        }
        PblMatrix mWorld;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mWorld), reinterpret_cast<const D3DXMATRIX*>(&mBone), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
        GameObject* pFound;
        if (CollisionManager::GetObjectsInRange(mWorld.GetTranslation(), mAnimatedClass->mAnimationTriggers[i].mTriggerRadius,
                                                &pFound, 1, 0, 2) > 0)
        {
            mAnimationTime = 0.0f;
            mBlendFactor = 0.0f;
            SetAnimation(mAnimatedClass->mAnimationTriggers[i].mAnimationID, false);
            mAnimatedClass->mAnimationSound[i].Play((const PblVector3*)mWorld.m[3], 0, false, true);
        }
    }

    if (!mAttachedSoldier)
    {
        for (int i = 0; i < 3; ++i)
        {
            const unsigned int uAttachPoint = mAnimatedClass->mAttachTriggers[i].mAttachPoint;
            if (!uAttachPoint || mAnimationId != mAnimatedClass->mAttachTriggers[i].mAnimationID ||
                !(mAnimatedClass->mAttachTriggers[i].mAnimationTime >= mAnimationTime))
            {
                continue;
            }
            mAttachMatrix = const_cast<PblMatrix*>(GetPose()->Find(uAttachPoint));
            if (!mAttachMatrix)
            {
                continue;
            }
            PblMatrix mWorld;
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mWorld), reinterpret_cast<const D3DXMATRIX*>(mAttachMatrix), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
            GameObject* pFound;
            if (CollisionManager::GetObjectsInRange(mWorld.GetTranslation(), mAnimatedClass->mAttachTriggers[i].mAttachRadius,
                                                    &pFound, 1, 0, 2) <= 0)
            {
                continue;
            }
            if (!pFound->IsRtti(EntitySoldier::rttiHashEntitySoldier) || !pFound->IsAlive())
            {
                continue;
            }

            EntitySoldier* pSoldier = static_cast<EntitySoldier*>(pFound);
            mAttachedSoldier = pFound;
            mAttachEvent = mAnimatedClass->mAttachTriggers[i].mPostAttachEvent;
            mAttachedSoldierAnimation = mAnimatedClass->mAttachTriggers[i].mSoldierAnimation;
            pSoldier->RemoveFromCollisionManager();
            pSoldier->AddAttachedToCollisionManager();
            pSoldier->mAttachedToProp = true;
            const int iJoystick = pSoldier->GetJoystickIndex();
            if (pSoldier->mPlayerId >= 0 && iJoystick > -1)
            {
                FirstPerson::Activate(iJoystick, false);
            }
            if (mAnimatedClass->mTrackDeathOnAttach)
            {
                if (pSoldier->mInitialPlayerId >= 0)
                {
                    pSoldier->TrackDeath(pSoldier->mInitialPlayerId, 0);
                }
                pSoldier->mMarkedForDeath = true;
            }
            break;
        }
    }

    float fStep = fDeltaTime;
    if (mLooping && mAnimationTime + fDeltaTime >= mAnimatedClass->mAnimations[0]->m_u16NumFrames * (1.0f / 30.0f))
    {
        mIdleDelayTimer += fDeltaTime;
        if (mIdleDelayTimer > mAnimatedClass->mIdleDelay)
        {
            mIdleDelayTimer = 0.0f;
        }
        else
        {
            fStep = 0.0f;
        }
    }
    mAnimationTime += fStep;
    if (mLooping)
    {
        const float fLength = mAnimatedClass->mAnimations[0]->m_u16NumFrames * (1.0f / 30.0f);
        if (mAnimationTime > fLength)
        {
            mAnimationTime -= fLength;
        }
    }

    mZephyrPoseDyn.Update(fStep);
    if (mBlendFactor >= 1.0f)
    {
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
    }
    else
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDyn, mBlendFactor);
    }
    mBlendFactor += mAnimationId == 0 ? fStep * 0.2f : fStep;
    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    mPose.ConvertFromZephyrPose(mZephyrPoseStatic);

    if (mAttachedSoldier && mAttachMatrix)
    {
        PblMatrix mWorld;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mWorld), reinterpret_cast<const D3DXMATRIX*>(mAttachMatrix), reinterpret_cast<const D3DXMATRIX*>(&mMatrix));
        RedPose kPose;
        mAttachedSoldier->SetupPose(kPose);
        const PblMatrix* pRoot = kPose.Find(0x16E27226u);
        if (pRoot)
        {
            mWorld = pRoot->GetInverseFast() * mWorld;
        }
        static_cast<EntitySoldier*>((GameObject*)mAttachedSoldier)
            ->SetPilotInfo(mWorld, PblVector3(0.0f, 0.0f, 0.0f),
                           (SoldierAnimationBank::ANIMATION)mAttachedSoldierAnimation, true, -1.0f, -1.0f, -1.0f,
                           fDeltaTime);
    }

    UpdateSound();
    return true;
}
