#include <d3dx9math.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "EntityMine.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "Explosion.h"
#include "NetGame.h"
#include "StickInfo.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "GameSound.h"
#include "FLEffect.h"
#include "ItemTimerDisplay.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "SoundFL/source/sndvoicevirtual.h"


extern PblVector3 gVectorZero;

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

// FUNCTION: SWBF2 0x004d08b0
EntityMineClass::EntityMineClass(unsigned int uHash)
    : EntityItemClass(uHash)
{
    mIsNeutralWhenEmpty = false;
    mExplosionTrigger = 0;
    mExplosionExpire = 0;
    mExplosionDeath = 0;
    mTriggerContact = false;
    mTriggerRadius = 0.0f;
    mSuppressRadius = 0.0f;
    mTickPitch = 1.0f;
}

// SYNTHETIC: SWBF2 0x00402c70
// `dynamic initializer for 'EntityMineClass::rttiHashEntityMineClass''
// EntityMineClass::rttiHashEntityMineClass

// GLOBAL: SWBF2 0x01ebbf90
// EntityMineClass::rttiHashEntityMineClass
PblHash EntityMineClass::rttiHashEntityMineClass("EntityMineClass");
// GLOBAL: SWBF2 0x0078935c
// EntityMineClass::rttiNameEntityMineClass
const char EntityMineClass::rttiNameEntityMineClass[] = "EntityMineClass";







// FUNCTION: SWBF2 0x004d0aa0
Entity* EntityMineClass::Build(const EntityDesc& desc)
{
    if (EntityMine::sMemoryPool.mUsed >= EntityMine::sMemoryPool.mCount)
    {
        return 0;
    }

    EntityMine* pMine = new EntityMine(this, desc);
    if (pMine)
    {
        (static_cast<CollisionObject*>(pMine))->AddItemToCollisionManager();

        pMine->ActivatePhysics();

        pMine->BuildEffects();
    }
    return pMine;
}

// SYNTHETIC: SWBF2 0x004d0930
// EntityMineClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004d0960
EntityMineClass::EntityMineClass(EntityMineClass* pBase, unsigned int uHash)
    : EntityItemClass(pBase, uHash)
{
    mExplosionTrigger = pBase->mExplosionTrigger;
    mExplosionExpire = pBase->mExplosionExpire;
    mExplosionDeath = pBase->mExplosionDeath;
    mTriggerContact = pBase->mTriggerContact;
    mTriggerRadius = pBase->mTriggerRadius;
    mSuppressRadius = pBase->mSuppressRadius;
    mTick = pBase->mTick;
    mTickPitch = pBase->mTickPitch;
}

// FUNCTION: SWBF2 0x004d0a20
EntityMineClass::~EntityMineClass()
{
}

// FUNCTION: SWBF2 0x004d0a30
EntityClass* EntityMineClass::Derive(unsigned int uHash)
{
    return new EntityMineClass(this, uHash);
}

// FUNCTION: SWBF2 0x004d0b50
void EntityMineClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x02BB1FE0:
    case 0x6A5E371C:
    case 0x9B30E763:
    {
        mExplosionTrigger = ExplosionClass::Find(PblHash(pszValue));
            break;
    }
    case 0x2E51081F:
    {
        mExplosionExpire = ExplosionClass::Find(PblHash(pszValue));
            break;
    }
    case 0xA2723442:
    {
        mExplosionDeath = ExplosionClass::Find(PblHash(pszValue));
            break;
    }
    case 0xACFE0D9B:
    {
        mTick.SetID(pszValue, false, false);
            break;
    }
    case 0x18953967:
    {
        sscanf("%f", pszValue, &mTickPitch);
            break;
    }
    case 0xBFB22E35:
    {
        mTriggerContact = atoi(pszValue) != 0;
            break;
    }
    case 0xC461D811:
    {
        sscanf(pszValue, "%f", &mTriggerRadius);
            break;
    }
    case 0xCE754894:
    {
        sscanf(pszValue, "%f", &mSuppressRadius);
            break;
    }
    default:
    {
        EntityItemClass::SetProperty(uProperty, pszValue);
            break;
    }
    }
}

// SYNTHETIC: SWBF2 0x00402c60
// `dynamic initializer for 'EntityMine::rttiHashEntityMine''
// EntityMine::rttiHashEntityMine

// GLOBAL: SWBF2 0x01ebbf8c
// EntityMine::rttiHashEntityMine
PblHash EntityMine::rttiHashEntityMine("EntityMine");
// GLOBAL: SWBF2 0x00789340
// EntityMine::rttiNameEntityMine
const char EntityMine::rttiNameEntityMine[] = "EntityMine";

// SYNTHETIC: SWBF2 0x00402c40
// `dynamic initializer for 'EntityMine::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f80
// `dynamic atexit destructor for 'EntityMine::sMemoryPool''

// GLOBAL: SWBF2 0x01ebbf98
// EntityMine::sMemoryPool
MemoryPool EntityMine::sMemoryPool("EntityMine", sizeof(EntityMine));

// FUNCTION: SWBF2 0x004cf5e0
EntityMine::EntityMine(EntityMineClass* pClass, const EntityDesc& desc)
    : EntityItem(pClass, desc)
{
    mNetDead = false;
    mPrevTimerVal = 0;
    mStickInfo = 0;
    mHitSurface = 0;
    mTrigger = false;
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x004cf690
EntityMine::~EntityMine()
{
    ItemTimerDisplay::RemoveItemTimer(this);
    NetGame::Destroyed(this);
    if (mStickInfo)
    {
        StickInfo::sMemoryPool.Free(mStickInfo);
    }
}

// SYNTHETIC: SWBF2 0x004cf550
// EntityMine::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004d0cc9
// ??_EEntityMine@@WJE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004d0cd4
// ??_EEntityMine@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004d0cdf
// ??_EEntityMine@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004d0ce7
// ??_EEntityMine@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004d0cf2
// ??_EEntityMine@@WCEA@AEPAXI@Z







// FUNCTION: SWBF2 0x004cf540
EntityType EntityMine::GetEntityType() const
{
    return static_cast<EntityType>(8);
}

// FUNCTION: SWBF2 0x004cf730
void EntityMine::Kill()
{
    mTrigger = true;
}

// FUNCTION: SWBF2 0x004cf740
void EntityMine::ClientKill()
{
    if (mLifeTimer <= 0.1f)
    {
        if (static_cast<EntityMineClass*>(mClass)->mExplosionExpire)
        {
            DamageOwner owner(mOwner, reinterpret_cast<GameObject*>(this), mTeam, mClass, 0, 1.0f);
            static_cast<EntityMineClass*>(mClass)->mExplosionExpire->CreateExplosion(
                GetPosition(), owner, false, COLL_UNKNOWN);
        }
    }
    else if (!IsAlive())
    {
        if (static_cast<EntityMineClass*>(mClass)->mExplosionDeath)
        {
            DamageOwner owner(mOwner, reinterpret_cast<GameObject*>(this), mTeam, mClass, 0, 1.0f);
            static_cast<EntityMineClass*>(mClass)->mExplosionDeath->CreateExplosion(
                GetPosition(), owner, false, COLL_UNKNOWN);
        }
    }
    else
    {
        if (static_cast<EntityMineClass*>(mClass)->mExplosionTrigger)
        {
            DamageOwner owner(mOwner, reinterpret_cast<GameObject*>(this), mTeam, mClass, 0, 1.0f);
            static_cast<EntityMineClass*>(mClass)->mExplosionTrigger->CreateExplosion(
                GetPosition(), owner, false, COLL_UNKNOWN);
        }
    }
}

// FUNCTION: SWBF2 0x004cf890
bool EntityMine::Update(float fDeltaTime)
{
    const bool bClient = NetGame::IsEnabled() && NetGame::IsClient();

    StickInfo* pStick = mStickInfo;
    if (pStick)
    {
        GameObject* pStuckTo = pStick->mCollHandle.mObject;
        if (pStuckTo && pStuckTo->mHandleId != pStick->mCollHandle.mSavedHandleId)
        {
            mParent = 0;
            delete pStick;
            mStickInfo = 0;
            mStationary = false;
        }
        else
        {
            D3DXQUATERNION qRot;
            PblMatrix mStick;
            D3DXMatrixRotationQuaternion(reinterpret_cast<D3DXMATRIX*>(&mStick), D3DXQuaternionNormalize(&qRot, reinterpret_cast<const D3DXQUATERNION*>(&pStick->mLocalRotation)));
            mStick.m[3][0] = pStick->mLocalPosition.x;
            mStick.m[3][1] = pStick->mLocalPosition.y;
            mStick.m[3][2] = pStick->mLocalPosition.z;
            mStick.m[3][3] = 1.0f;
            CollisionObject* pObject = mStickInfo->mCollObject;
            CollisionModel* pModel = pObject->mCollisionModel;
            if (pModel)
            {
                PblMatrix mBody = pModel->GetWorldMatrix(mStickInfo->mCollBodyID,
                                                         pObject->GetMatrix(), pObject->GetPose());
                mStick *= mBody;
            }
            SetMatrix(mStick);
            if (fDeltaTime > 0.0f)
            {
                mVelocity = (mMatrix.GetTranslation() - mOldPosition) / fDeltaTime;
                mOldPosition = mMatrix.GetTranslation();
            }
            mStationary = true;
        }
    }

    if (mNetDead)
    {
        return true;
    }

    const int iTimer = (int)mLifeTimer;
    if (iTimer != mPrevTimerVal)
    {
        const GameSound* pTick = &static_cast<EntityMineClass*>(mClass)->mTick;
        const float fTickPitch = static_cast<EntityMineClass*>(mClass)->mTickPitch;
        GameSoundControllable tick;
        tick.Play(pTick, (const PblVector3*)mMatrix.m[3], 0, true);
        Snd::VoiceVirtual* pVoice = tick.GetVoiceVirtual();
        if (pVoice && static_cast<EntityMineClass*>(mClass)->mLifeSpan > 0.0f)
        {
            const float fLife = mLifeTimer / static_cast<EntityMineClass*>(mClass)->mLifeSpan;
            pVoice->SetPitch(pTick->GetPitch() * ((1.0f - fLife) * fTickPitch + fLife));
        }
        mPrevTimerVal = iTimer;
    }

    if (mLifeTimer <= 1.0f && mTrailEmitterObj)
    {
        mTrailEmitterObj->DeactivateEffect();
    }

    if (mLifeTimer <= fDeltaTime)
    {
        if (!bClient && static_cast<EntityMineClass*>(mClass)->mExplosionExpire)
        {
            static_cast<EntityMineClass*>(mClass)->mExplosionExpire->CreateExplosion(
                GetPosition(), DamageOwner(mOwner, this, mTeam, mClass, 0, 1.0f), false, COLL_UNKNOWN);
        }
        mNetDead = true;
        return bClient;
    }

    if (!IsAlive())
    {
        if (!bClient && static_cast<EntityMineClass*>(mClass)->mExplosionDeath)
        {
            static_cast<EntityMineClass*>(mClass)->mExplosionDeath->CreateExplosion(
                GetPosition(), DamageOwner(mOwner, this, mTeam, mClass, 0, 1.0f), false, COLL_UNKNOWN);
        }
        mNetDead = true;
        return bClient;
    }

    if (mStationary && static_cast<EntityMineClass*>(mClass)->mExplosionTrigger &&
        static_cast<EntityMineClass*>(mClass)->mTriggerRadius > 0.0f)
    {
        PblVector3 vPos = mMatrix.GetTranslation();
        const float fRange = static_cast<EntityMineClass*>(mClass)->mTriggerRadius >
                                     static_cast<EntityMineClass*>(mClass)->mSuppressRadius
                                 ? static_cast<EntityMineClass*>(mClass)->mTriggerRadius
                                 : static_cast<EntityMineClass*>(mClass)->mSuppressRadius;
        GameObject* apObjects[64];
        const int iCount = CollisionManager::GetObjectsInRange(vPos, fRange, apObjects, 64, this, 42);
        for (int i = 0; i < iCount; ++i)
        {
            if (IsMyEnemy(apObjects[i]) && apObjects[i]->IsAlive())
            {
                if (apObjects[i]->GetEntityClass()->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass) &&
                    static_cast<EntitySoldierClass*>(
                        static_cast<GameObjectClass*>(apObjects[i]->GetEntityClass()))->mImmuneToMines)
                {
                    continue;
                }

                const PblVector3 vCenter = apObjects[i]->GetCollisionObject()->GetCollisionCenter();
                const float fRadius = apObjects[i]->GetCollisionObject()->GetCollisionRadius();
                const float fDX = vCenter.x - mMatrix.m[3][0];
                const float fDY = vCenter.y - mMatrix.m[3][1];
                const float fDZ = vCenter.z - mMatrix.m[3][2];
                const float fReach = static_cast<EntityMineClass*>(mClass)->mTriggerRadius + fRadius;
                if (fReach * fReach > fDY * fDY + fDX * fDX + fDZ * fDZ)
                {
                    const PblVector3& vVelocity = apObjects[i]->GetVelocity();
                    if (vVelocity.y * fDY + fDX * vVelocity.x + vVelocity.z * fDZ >= 0.0f)
                    {
                        mTrigger = true;
                    }
                }
            }
            else if (IsMyFriend(apObjects[i]) && apObjects[i]->IsAlive() &&
                     static_cast<EntityMineClass*>(mClass)->mSuppressRadius > 0.0f)
            {
                const PblVector3 vCenter = apObjects[i]->GetCollisionObject()->GetCollisionCenter();
                const float fRadius = apObjects[i]->GetCollisionObject()->GetCollisionRadius();
                const float fDX = vCenter.x - vPos.x;
                const float fDY = vCenter.y - vPos.y;
                const float fDZ = vCenter.z - vPos.z;
                const float fReach = static_cast<EntityMineClass*>(mClass)->mSuppressRadius + fRadius;
                if (fReach * fReach > fDY * fDY + fDX * fDX + fDZ * fDZ)
                {
                    mTrigger = false;
                    break;
                }
            }
        }
    }

    if (mTrigger)
    {
        if (mTrailEmitterObj)
        {
            mTrailEmitterObj->DeactivateEffect();
        }
        if (!bClient && static_cast<EntityMineClass*>(mClass)->mExplosionTrigger)
        {
            DamageOwner owner(mOwner, this, mTeam, mClass, 0, 1.0f);
            static_cast<EntityMineClass*>(mClass)->mExplosionTrigger->CreateExplosion(
                GetPosition(), owner, false, COLL_UNKNOWN);
        }
        mNetDead = true;
        return bClient;
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        const PblVector3 vPos = mMatrix.GetTranslation();
        if (vPos.y < -500.0f)
        {
            Kill();
            return false;
        }
    }
    return EntityItem::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x004d00e0
bool EntityMine::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor)
{
    if (mStationary)
    {
        if (mStickInfo && mStickInfo->mCollObject == pOther)
        {
            return true;
        }

        EntityMineClass* pClass = static_cast<EntityMineClass*>(mClass);
        if (pClass->mTriggerContact && pClass->mExplosionTrigger)
        {
            GameObject* pObject = pOther->GetGameObject();
            if (pObject)
            {
                FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
                PlayImpactEffects((const PblVector3*)mMatrix.m[3], &mVelocity);
                if (IsMyEnemy(pObject) &&
                    !(pObject->GetEntityClass()->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass) &&
                      static_cast<EntitySoldierClass*>(
                          static_cast<GameObjectClass*>(pObject->GetEntityClass()))->mImmuneToMines))
                {
                    DamageOwner owner(mOwner, this, mTeam, mClass, 0, 1.0f);
                    static_cast<EntityMineClass*>(mClass)->mExplosionTrigger->CreateExplosion(
                        GetPosition(), owner, false, COLL_UNKNOWN);
                    InstantDeath(owner, true);
                    return true;
                }
            }
        }
        return false;
    }

    const bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    const CollisionObjectType eType = pOther->GetCollisionType();
    if (eType == COLL_WATER)
    {
        if (!mHitSurface)
        {
            mHitSurface = 1;
            static_cast<EntityMineClass*>(mClass)->mCollisionWaterSound.Play(
                (const PblVector3*)mMatrix.m[3], 0, false, true);
        }
        mVelocity = PblVector3(0.0f, -0.2f, 0.0f);
        return false;
    }
    if (eType == COLL_LEAFPATCH)
    {
        if (!mHitSurface)
        {
            mHitSurface = 1;
            static_cast<EntityMineClass*>(mClass)->mCollisionFoliageSound.Play(
                (const PblVector3*)mMatrix.m[3], 0, false, true);
        }
        return false;
    }

    if (mLifeTimer > static_cast<EntityMineClass*>(mClass)->mLifeSpan - 0.5f && mOwner && pOther->GetGameObject() &&
        pOther->GetGameObject()->GetControllable() &&
        pOther->GetGameObject()->GetControllable()->mCharacter == mOwner)
    {
        return bResult;
    }

    if ((!pOther->GetGameObject() || (HealthType)pOther->GetGameObject()->mHealthType >= HEALTH_TYPE_VEHICLE) &&
        StickInfo::sMemoryPool.mUsed < StickInfo::sMemoryPool.mCount)
    {
        CollisionModel* pModel = pOther->mCollisionModel;
        PblVector3 vNormal = pResult->mSeparationNormal;
        PblVector3 vDown = -vNormal;
        PblVector3 vStart = mMatrix.GetTranslation() + vNormal * 0.5f;
        pOther->RayHit(vStart, vDown, 1.0f, &vNormal, 0, true, 0.0f);
        PblMatrix mStick = NormalMatrix(mMatrix.GetZAxis(), vNormal,
                                      mMatrix.GetTranslation());

        mStickInfo = new StickInfo;
        mStickInfo->mCollHandle = pOther->GetGameObject();
        mStickInfo->mCollObject = pOther;
        if (pModel)
        {
            PblMatrix mBody = pModel->GetWorldMatrix(pResult->mReceiverId, pOther->GetMatrix(), pOther->GetPose());
            mStick *= mBody.GetInverseFast();
            mStickInfo->mCollBodyID = pResult->mReceiverId;
        }
        else
        {
            mStickInfo->mCollBodyID = -1;
        }
        mStickInfo->mLocalPosition = mStick.GetTranslation();
        mStickInfo->mLocalRotation = PblQuaternion(mStick);

        mParent = pOther;
        mVelocity = gVectorZero;
        mOldPosition = mMatrix.GetTranslation();
        if (!mHitSurface)
        {
            mHitSurface = 1;
            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
            PlayImpactEffects((const PblVector3*)mMatrix.m[3], &mVelocity);
            static_cast<EntityMineClass*>(mClass)->mCollisionSound.Play(
                (const PblVector3*)mMatrix.m[3], 0, false, true);
        }
        mStationary = true;
        ItemTimerDisplay::RemoveItemTimer(this);
        ItemTimerDisplay::AddItemTimer(this);
        return bResult;
    }

    const PblVector3& vNormal = pResult->mSeparationNormal;
    const float fDot = mVelocity.x * vNormal.x + vNormal.y * mVelocity.y + vNormal.z * mVelocity.z;
    if (fDot < 0.0f)
    {
        const float fRebound = static_cast<EntityMineClass*>(mClass)->mRebound;
        const float fFriction = static_cast<EntityMineClass*>(mClass)->mFriction * fDot * (fRebound + 1.0f);
        mVelocity -= vNormal * fDot;
        float fScale = -(fFriction / Length(mVelocity));
        if (!(1.0f > fScale))
        {
            fScale = 1.0f;
        }
        mVelocity -= mVelocity * fScale;
        const float fBounce = fRebound * fDot;
        mVelocity -= vNormal * fBounce;
        if (!mHitSurface)
        {
            mHitSurface = 1;
            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
            PlayImpactEffects((const PblVector3*)mMatrix.m[3], &mVelocity);
            static_cast<EntityMineClass*>(mClass)->mCollisionSound.Play(
                (const PblVector3*)mMatrix.m[3], 0, false, true);
        }
    }
    return bResult;
}
