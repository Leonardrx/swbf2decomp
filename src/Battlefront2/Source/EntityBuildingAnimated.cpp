#include <string.h>
#include "EntityBuildingAnimated.h"
#include "PebbleFL/Common/PblVector.h"

#include <stdio.h>
#include <float.h>
#include <d3dx9math.h>
#include "CameraManager.h"
#include "FLEffect.h"
#include "GameLoop.h"
#include "NetGame.h"

#include "CollisionObject.h"
#include "Damageable.h"
#include "Explosion.h"
#include "GameModel.h"
#include "Thread.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

// FUNCTION: SWBF2 0x00494b30
EntityBuildingAnimatedClass::EntityBuildingAnimatedClass(unsigned int uHash)
    : EntityBuildingClass(uHash)
{
    mAnimObj = 0;
    memset(mAnimations, 0, sizeof(mAnimations));
    memset(mDeathEffectHash, 0, sizeof(mDeathEffectHash));
    memset(mDeathEffectNodeHash, 0, sizeof(mDeathEffectNodeHash));
    mDeathExplosions = false;
}

// FUNCTION: SWBF2 0x00494c50
EntityBuildingAnimatedClass::~EntityBuildingAnimatedClass()
{
}



// SYNTHETIC: SWBF2 0x004028b0
// `dynamic initializer for 'EntityBuildingAnimatedClass::rttiHashEntityBuildingAnimatedClass''
// EntityBuildingAnimatedClass::rttiHashEntityBuildingAnimatedClass

// GLOBAL: SWBF2 0x01ebb87c
// EntityBuildingAnimatedClass::rttiHashEntityBuildingAnimatedClass
PblHash EntityBuildingAnimatedClass::rttiHashEntityBuildingAnimatedClass("EntityBuildingAnimatedClass");
// GLOBAL: SWBF2 0x00788f84
// EntityBuildingAnimatedClass::rttiNameEntityBuildingAnimatedClass
const char EntityBuildingAnimatedClass::rttiNameEntityBuildingAnimatedClass[] = "EntityBuildingAnimatedClass";

// SYNTHETIC: SWBF2 0x004028a0
// `dynamic initializer for 'EntityBuildingAnimated::rttiHashEntityBuildingAnimated''
// EntityBuildingAnimated::rttiHashEntityBuildingAnimated

// GLOBAL: SWBF2 0x01ebb878
// EntityBuildingAnimated::rttiHashEntityBuildingAnimated
PblHash EntityBuildingAnimated::rttiHashEntityBuildingAnimated("EntityBuildingAnimated");
// GLOBAL: SWBF2 0x00788fa0
// EntityBuildingAnimated::rttiNameEntityBuildingAnimated
const char EntityBuildingAnimated::rttiNameEntityBuildingAnimated[] = "EntityBuildingAnimated";







// SYNTHETIC: SWBF2 0x00494b90
// EntityBuildingAnimatedClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00494bc0
EntityBuildingAnimatedClass::EntityBuildingAnimatedClass(EntityBuildingAnimatedClass* pBase, unsigned int uHash)
    : EntityBuildingClass(pBase, uHash)
{
    mAnimObj = pBase->mAnimObj;
    mAnimations[0] = pBase->mAnimations[0];
    mAnimations[1] = pBase->mAnimations[1];
    memcpy(mDeathEffectHash, pBase->mDeathEffectHash, sizeof(mDeathEffectHash));
    memcpy(mDeathEffectNodeHash, pBase->mDeathEffectNodeHash, sizeof(mDeathEffectNodeHash));
    mDeathExplosions = pBase->mDeathExplosions;
}

// FUNCTION: SWBF2 0x00494c60
bool EntityBuildingAnimatedClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    if (!mAnimObj)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x00494ca0
void EntityBuildingAnimatedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x98557a5e: /* "AnimationName" */
        InitAnimations(pszValue);
        break;

    case 0x297bc6eb: /* "IdleAnimation" */
        if (mAnimObj)
        {
            mAnimations[0] = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
        }
        break;

    case 0x5fac58b5: /* "DeathAnimation" */
    {
        RedAnimation* pAnimObj = mAnimObj;
        if (pAnimObj)
        {
            mAnimations[1] = (ZephyrAnim*)(pAnimObj->_pZephyrAnimBank ? pAnimObj->_pZephyrAnimBank->Find(PblTEMPHash(pszValue)) : 0);
        }
        break;
    }

    case 0x3e4e9ba0: /* "EnableDeathExplosions" */
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mDeathExplosions = iValue != 0;
        break;
    }

    case 0xf138015c: /* "DeathEffect" */
        for (int i = 0; i < 5; i++)
        {
            if (!mDeathEffectHash[i])
            {
                char acEffect[32];
                char acNode[32];
                sscanf(pszValue, "%s %s", acEffect, acNode);
                mDeathEffectHash[i] = PblHash(acEffect);
                mDeathEffectNodeHash[i] = PblTEMPHash(acNode);
                break;
            }
        }
        break;

    default:
        EntityBuildingClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00494e00
bool EntityBuildingAnimatedClass::SetupAnimation(ZephyrPoseDyn<32>& kPose, int iAnim, float fTime, bool bLoop)
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

// FUNCTION: SWBF2 0x00494eb0
EntityClass* EntityBuildingAnimatedClass::Derive(unsigned int uHash)
{
    return new EntityBuildingAnimatedClass(this, uHash);
}

// FUNCTION: SWBF2 0x00494f20
Entity* EntityBuildingAnimatedClass::Build(const EntityDesc& desc)
{
    typedef void (__thiscall* ActivatePhysicsFn)(void*);

    EntityBuildingAnimated* pBuilding = new EntityBuildingAnimated(this, desc);
    if (pBuilding)
    {
        static_cast<CollisionObject*>(pBuilding)->AddStaticToCollisionManager();
        pBuilding->ActivatePhysics();
    }
    return (Entity*)pBuilding;
}



// FUNCTION: SWBF2 0x00475fa0
const RedPose* EntityBuildingAnimated::GetPose() const
{
    return &mPose;
}





// FUNCTION: SWBF2 0x00475f80
EntityClass* EntityBuildingAnimated::GetEntityClass()
{
    return (EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x00475f90
const EntityClass* EntityBuildingAnimated::GetEntityClass() const
{
    return (const EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x00494160
EntityBuildingAnimated::EntityBuildingAnimated(EntityBuildingAnimatedClass* pClass, const EntityDesc& desc)
    : EntityBuilding(pClass, desc)
{
    mAnimObj = pClass->mAnimObj;
    mAnimatedClass = pClass;
    mAnimationId = (unsigned int)-1;
    if (pClass->mAnimations[0])
    {
        mAnimationTime = mAnimatedClass->mAnimations[0]->m_u16NumFrames * PblRandom::_Global.GetFloat() * 30.0f;
    }
    else
    {
        mAnimationTime = 0.0f;
    }
    if (mAnimObj)
    {
        mZephyrSkeleton.Open(*mAnimatedClass->mAnimObj->GetZephyrSkeletonShared());
        mZephyrPoseDyn.Open(mZephyrSkeleton);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }
    mDeathExplosionTime = 0.0f;
}

// SYNTHETIC: SWBF2 0x00494110
// EntityBuildingAnimated::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00494f98
// ??_EEntityBuildingAnimated@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00494fa3
// ??_EEntityBuildingAnimated@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00494fab
// ??_EEntityBuildingAnimated@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00494fb6
// ??_EEntityBuildingAnimated@@WCGA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00494fc1
// ??_EEntityBuildingAnimated@@WJE@AEPAXI@Z

// FUNCTION: SWBF2 0x00494320
EntityBuildingAnimated::~EntityBuildingAnimated()
{
}

// FUNCTION: SWBF2 0x004943d0
void EntityBuildingAnimated::Init()
{
    mModel->GetRedModel(0)->SetupPose(mPose);
    if (mModel->GetRedModel(0)->_pStencilShadow)
    {
        RedDirectionalLight* pLight = NULL;
        if (RedLight::GetGlobalDirectionalLight(0)->IsShadowCaster())
        {
            pLight = RedLight::GetGlobalDirectionalLight(0);
        }
        else if (RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster())
        {
            pLight = RedLight::GetGlobalDirectionalLight(1);
        }
        if (pLight)
        {
            const PblVector3& vDir = pLight->m_vDirection;
            float fGrow;
            if (vDir.y < -0.5f)
            {
                fGrow = _Sphere.w / (vDir.y * -2.0f);
            }
            else
            {
                fGrow = _Sphere.w;
            }
            SetRenderPos(PblVector3(vDir.x * fGrow + _Sphere.x, vDir.y * fGrow + _Sphere.y,
                                    vDir.z * fGrow + _Sphere.z));
            SetRenderRadius(_Sphere.w + fGrow);
            SetTestBBox(false);
        }
    }
    if (mpShadow)
    {
        mpShadow->RemoveFromScene();
    }
}

// FUNCTION: SWBF2 0x00494b00
void EntityBuildingAnimated::ActivatePhysics()
{
    ActivateThread(0, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x00494b20
void EntityBuildingAnimated::DeactivatePhysics()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x00494a10
void EntityBuildingAnimated::SetAnimation(unsigned int uAnimationId, bool bLoop)
{
    if (!mAnimObj || uAnimationId == mAnimationId)
    {
        return;
    }

    mAnimatedClass->SetupAnimation(mZephyrPoseDyn, uAnimationId, mAnimationTime, bLoop);

    ZephyrPoseStatic<32> kPose;
    mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
    kPose.Open(mZephyrSkeleton);
    kPose.Set(mZephyrPoseDyn, true);
    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    mAnimationId = uAnimationId;
}

// FUNCTION: SWBF2 0x00494940
void EntityBuildingAnimated::Kill()
{
    if (mAnimationId != 1 && mAnimatedClass->mAnimations[1])
    {
        mState = STATE_DYING;
        mAnimationTime = 0.0f;
        SetAnimation(1, false);
        if (mAnimatedClass->mExplosion)
        {
            DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam, mClass, 0,
                              1.0f);
            mAnimatedClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
        }
        return;
    }
    EntityBuilding::Kill();
}

// FUNCTION: SWBF2 0x004944f0
bool EntityBuildingAnimated::Update(float fDeltaTime)
{
    EntityBuilding::Update(fDeltaTime);

    if (mAnimObj)
    {
        if (mAnimationId == (unsigned int)-1 && mAnimatedClass->mAnimations[0])
        {
            SetAnimation(0, true);
        }

        if (mAnimationId == 1 && mAnimationTime > mAnimatedClass->mAnimations[1]->m_u16NumFrames / 30.0f)
        {
            mAnimationId = (unsigned int)-1;
            mAnimationTime = 0.0f;
            for (int i = 0; i < 5; ++i)
            {
                if (!mAnimatedClass->mDeathEffectHash[i])
                {
                    continue;
                }
                PblMatrix mEffect;
                GetModel()->GetRedModel(0)->GetBoneMatrix(mAnimatedClass->mDeathEffectNodeHash[i], mEffect);
                PblMatrix mWorld;
                mWorld = mMatrix * mEffect;
                mEffect = mWorld;
                PblVector3 vEffectPos = mEffect.GetTranslation();
                if (CameraManager::sInstance->IsInsideAnyCamera(vEffectPos, 60.0f, 70.0f))
                {
                    if (FLEffectClass* pEffectClass = FLEffect::Get(mAnimatedClass->mDeathEffectHash[i]))
                    {
                        if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                        {
                            pEffect->AttachEffectToMatrix(&mEffect, false);
                        }
                    }
                }
            }
            EntityBuilding::Kill();
        }

        ZephyrPoseStatic<32> kPose;
        mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
        kPose.Open(mZephyrSkeleton);
        if (mAnimationId != (unsigned int)-1)
        {
            mAnimationTime = fDeltaTime + mAnimationTime;
            mZephyrPoseDyn.Update(fDeltaTime);
            kPose.Set(mZephyrPoseDyn, true);
        }
        mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mState == STATE_DEAD && !IsInScene() &&
        !IsInCollisionList() && mAddHealth < FLT_EPSILON)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x004947d0
void EntityBuildingAnimated::Render(unsigned char uFlags, float fDist, unsigned int uMask)
{
    if (mAnimationId == 1 && mAnimatedClass->mDeathExplosions)
    {
        mDeathExplosionTime += GameLoop::GetDeltaTime();
        if (mDeathExplosionTime > 0.4f)
        {
            mDeathExplosionTime = 0.0f;
            FLEffectClass* pEffectClass = 0;
            switch (PblRandom::GetGlobalInt() % 3)
            {
            case 0:
                pEffectClass = FLEffect::Get(0x02bb1fe0);
                break;
            case 1:
                pEffectClass = FLEffect::Get(0xe07f39b5);
                break;
            case 2:
                pEffectClass = FLEffect::Get(0x9db69ee4);
                break;
            }
            if (pEffectClass)
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    CollisionModel* pModel = mCollisionModel;
                    int iBody = PblRandom::GetGlobalInt() % pModel->GetNumBodies();
                    PblMatrix mBody = pModel->GetWorldMatrix(iBody, mMatrix, GetPose());
                    pEffect->AttachEffectToMatrix(&mBody, false);
                }
            }
        }
    }
    SetupShadow(this, mModel, uFlags, uMask, 0);
    EntityBuilding::Render(uFlags, fDist, uMask);
}
