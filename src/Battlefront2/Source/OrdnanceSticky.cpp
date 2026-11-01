#include "PebbleFL/Common/PblMath.h"
#include <d3dx9math.h>
#include "OrdnanceSticky.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "CollisionManager.h"
#include "CollisionModel.h"
#include "Damageable.h"
#include "GameObject.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "StickInfo.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedModel.h"


extern bool netIsLateTurn;
extern bool netFrameLock;
extern PblVector3 gVectorZero;

namespace AIUtil
{

    void GrenadeAlert(const PblVector3& vPos, const GameObject* pObject, float* pBeginTime,
                      float* pLastEventTime);
}

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace
{

    Damageable* AsDamageable(GameObject* pObject)
    {
        return static_cast<Damageable*>(pObject);
    }

    inline unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }
}

// SYNTHETIC: SWBF2 0x00405b80
// `dynamic initializer for 'OrdnanceSticky::rttiHashOrdnanceSticky''
// OrdnanceSticky::rttiHashOrdnanceSticky

// GLOBAL: SWBF2 0x01ef2668
// OrdnanceSticky::rttiHashOrdnanceSticky
PblHash OrdnanceSticky::rttiHashOrdnanceSticky("OrdnanceSticky");
// GLOBAL: SWBF2 0x0078aa00
// OrdnanceSticky::rttiNameOrdnanceSticky
const char OrdnanceSticky::rttiNameOrdnanceSticky[] = "OrdnanceSticky";

// SYNTHETIC: SWBF2 0x00405b90
// `dynamic initializer for 'OrdnanceStickyClass::rttiHashOrdnanceStickyClass''
// OrdnanceStickyClass::rttiHashOrdnanceStickyClass

// GLOBAL: SWBF2 0x01ef266c
// OrdnanceStickyClass::rttiHashOrdnanceStickyClass
PblHash OrdnanceStickyClass::rttiHashOrdnanceStickyClass("OrdnanceStickyClass");
// GLOBAL: SWBF2 0x0078aa20
// OrdnanceStickyClass::rttiNameOrdnanceStickyClass
const char OrdnanceStickyClass::rttiNameOrdnanceStickyClass[] = "OrdnanceStickyClass";

// FUNCTION: SWBF2 0x00606d80
OrdnanceStickyClass::OrdnanceStickyClass(unsigned int uHash)
    : OrdnanceShellClass(uHash)
{
    mStartTimerOnContact = true;
    mAlignVertical = true;
    mStickTypes = -1;
}

// FUNCTION: SWBF2 0x00606e10
OrdnanceStickyClass::OrdnanceStickyClass(OrdnanceStickyClass* pBase, unsigned int uHash)
    : OrdnanceShellClass(pBase, uHash)
{
    mSoundCollisionWater = pBase->mSoundCollisionWater;
    mSoundCollisionLeaf = pBase->mSoundCollisionLeaf;
    mSoundCollisionOther = pBase->mSoundCollisionOther;
    mStartTimerOnContact = pBase->mStartTimerOnContact;
    mAlignVertical = pBase->mAlignVertical;
    mStickTypes = pBase->mStickTypes;
}







// FUNCTION: SWBF2 0x00606f00
void OrdnanceStickyClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x2297E47D:
        mSoundCollisionWater.SetID(pszValue, false, false);
        break;
    case 0xB8C524F1:
        mSoundCollisionLeaf.SetID(pszValue, false, false);
        break;
    case 0xEDC6251A:
        mSoundCollisionOther.SetID(pszValue, false, false);
        break;
    case 0x0D0CDE2B:
        mStartTimerOnContact = atoi(pszValue) != 0;
        break;
    case 0xDB873344:
        mAlignVertical = atoi(pszValue) != 0;
        break;
    case 0xD3913CC2:
        mStickTypes = atoi(pszValue) ? -1 : 0;
        break;
    case 0x01E9CAEA:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x1;
        }
        else
        {
            mStickTypes &= ~0x1;
        }
        break;
    case 0xD1B49B73:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x2;
        }
        else
        {
            mStickTypes &= ~0x2;
        }
        break;
    case 0x3B3F45B7:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x4;
        }
        else
        {
            mStickTypes &= ~0x4;
        }
        break;
    case 0x9B4AF9ED:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x8;
        }
        else
        {
            mStickTypes &= ~0x8;
        }
        break;
    case 0x7449C0B3:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x10;
        }
        else
        {
            mStickTypes &= ~0x10;
        }
        break;
    case 0xAD9C5FA5:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x20;
        }
        else
        {
            mStickTypes &= ~0x20;
        }
        break;
    case 0xA8A2AFF4:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x40;
        }
        else
        {
            mStickTypes &= ~0x40;
        }
        break;
    case 0x95706D30:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x80000000;
        }
        else
        {
            mStickTypes &= ~0x80000000;
        }
        break;
    default:
        OrdnanceShellClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00607180
OrdnanceClass* OrdnanceStickyClass::Derive(unsigned int uHash)
{
    return new OrdnanceStickyClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x00606de0
// OrdnanceStickyClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006071f0
Ordnance* OrdnanceStickyClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceSticky* pSticky = new OrdnanceSticky(this, desc);
    pSticky->ActivateThread(-40, PER_LOCAL_TURN);
    pSticky->AddOrdnanceToCollisionManager();
    return pSticky;
}

// FUNCTION: SWBF2 0x00605fd0
OrdnanceSticky::OrdnanceSticky(OrdnanceStickyClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceShell(pClass, desc)
{
    mStickInfo = 0;
    mNumBounces = 0;
    mDecrementLifeTime = !pClass->mStartTimerOnContact;
    mLastEventTime = -1.0f;
    mBeginTime = FLT_MAX;
}

// FUNCTION: SWBF2 0x00606040
OrdnanceSticky::~OrdnanceSticky()
{
    if (mStickInfo)
    {
        delete mStickInfo;
    }
}

// SYNTHETIC: SWBF2 0x00607286
// ??_EOrdnanceSticky@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0060728e
// ??_EOrdnanceSticky@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00605f80
// OrdnanceSticky::`scalar deleting destructor'







// FUNCTION: SWBF2 0x006060b0
bool OrdnanceSticky::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }
    if (GameLoop::GetMissionTime() > mBeginTime)
    {
        SoundEvent();
    }

    float fWaterHeight;
    if (RedWater::GetWaterHeight(mPosition, fWaterHeight) && fWaterHeight - mPosition.y > 0.0f)
    {
        mVelocity = PblVector3(0.0f, -2.0f, 0.0f);
        if (!mDecrementLifeTime)
        {
            ((OrdnanceStickyClass*)mClass)->mSoundCollisionWater.Play(&mPosition, 0, false, true);
            mDecrementLifeTime = true;
        }
    }

    if (!mStickInfo)
    {
        return OrdnanceShell::Update(fDeltaTime);
    }

    if (mStickInfo->mCollHandle.mObject &&
        GameObjectHandleId(mStickInfo->mCollHandle.mObject) != mStickInfo->mCollHandle.mSavedHandleId)
    {
        delete mStickInfo;
        mStickInfo = 0;
        return OrdnanceShell::Update(fDeltaTime);
    }

    mOldPosition = mPosition;
    mPosition = mStickInfo->mLocalPosition;
    CollisionObject* pObject = mStickInfo->mCollObject;
    CollisionModel* pModel = pObject->mCollisionModel;
    if (pModel)
    {
        PblMatrix mBody = pModel->GetWorldMatrix(mStickInfo->mCollBodyID,
                                                 pObject->GetMatrix(), pObject->GetPose());
        PblVector3 vWorld;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vWorld), reinterpret_cast<const D3DXVECTOR3*>(&mPosition), reinterpret_cast<const D3DXMATRIX*>(&mBody));
        mPosition = vWorld;
    }
    SetRenderPos(mPosition);

    if (fDeltaTime != 0.0f)
    {
        mVelocity = (mPosition - mOldPosition) / fDeltaTime;
    }
    return Ordnance::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x006062c0
void OrdnanceSticky::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    OrdnanceStickyClass* pClass = (OrdnanceStickyClass*)mClass;
    if (!pClass->mModel)
    {
        return;
    }

    PblMatrix mat;
    StickInfo* pStick = mStickInfo;
    if (pStick)
    {
        if (!pStick->mCollObject)
        {
            return;
        }

        {
            D3DXQUATERNION qRot;
            PblMatrix mRot;
            D3DXMatrixRotationQuaternion(reinterpret_cast<D3DXMATRIX*>(&mRot), D3DXQuaternionNormalize(&qRot, reinterpret_cast<const D3DXQUATERNION*>(&pStick->mLocalRotation)));
            mRot.SetTranslation(pStick->mLocalPosition.x, pStick->mLocalPosition.y, pStick->mLocalPosition.z, 1.0f);
            mat = mRot;
        }

        CollisionObject* pObject = mStickInfo->mCollObject;
        CollisionModel* pModel = pObject->mCollisionModel;
        if (pModel)
        {
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mat), reinterpret_cast<const D3DXMATRIX*>(&mat),
                               reinterpret_cast<const D3DXMATRIX*>(&pModel->GetWorldMatrix(mStickInfo->mCollBodyID,
                                                                                            pObject->GetMatrix(), pObject->GetPose())));
        }
    }
    else if (pClass->mAlignVertical)
    {
        PblVector3 vPos = mPosition;
        if (!netFrameLock && NetGame::IsEnabled() &&
            netInterpolator.GetIndex(mInterpIndex, 0, PblHandled::mHandleId))
        {
            netInterpolator.GetVec(mInterpIndex, vPos);
        }
        mat = gMatrixIdentity;
        mat.SetTranslation(vPos);
    }
    else
    {
        OrdnanceBullet::Render(ucViewport, fDist, uiFlags);
        return;
    }
    ((OrdnanceStickyClass*)mClass)->mModel->Render(&mat, 0, RedColor::WHITE, uiFlags, 0);
}

// FUNCTION: SWBF2 0x006064d0
bool OrdnanceSticky::CheckCollision()
{
    PblVector3 vStart = GetOldPosition();
    const PblVector3& vEnd = GetPosition();
    PblVector3 vDir(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    const float fDist = PblMath::Sqrt(vDir.y * vDir.y + vDir.x * vDir.x + vDir.z * vDir.z);
    if (fDist > 0.0f)
    {
        const float fInv = 1.0f / fDist;
        vDir *= fInv;

        CollisionObject* pHitObject = 0;
        const GameObject* pIgnore = GetOwner();
        const float fLength = fDist + 0.2f;
        vStart -= vDir * 0.1f;
        PblVector3 vNormal;
        const float fHit = CollisionManager::RayHit(vStart, vDir, fLength, pHitObject, &vNormal, &pIgnore, 1,
                                                    mCheckStaticCollision ? 0x13BA : 0x132A, true);
        if (fHit < 1.0f)
        {
            const float fAlong = fHit * fLength;
            return !Collide(pHitObject, vDir * fAlong + vStart, vNormal, CollisionManager::GetLastRayHitBodyId());
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006066e0
bool OrdnanceSticky::Collide(CollisionObject* pObject, const PblVector3& vPos,
                             const PblVector3& vNormal, int iBodyId)
{
    if (mBeginTime > GameLoop::GetMissionTime())
    {
        mBeginTime = GameLoop::GetMissionTime();
    }

    if (pObject->GetCollisionType() == COLL_WATER)
    {
        mVelocity = PblVector3(0.0f, -2.0f, 0.0f);
        ((OrdnanceStickyClass*)mClass)->mSoundCollisionWater.Play(&vPos, 0, false, true);
        mDecrementLifeTime = true;
        return false;
    }
    if (pObject->GetCollisionType() == COLL_LEAFPATCH)
    {
        ((OrdnanceStickyClass*)mClass)->mSoundCollisionLeaf.Play(&vPos, 0, false, true);
        return false;
    }
    ((OrdnanceStickyClass*)mClass)->mSoundCollisionOther.Play(&vPos, 0, false, true);

    bool bStick = false;
    if (pObject->GetCollisionType() == COLL_TERRAIN)
    {
        bStick = (((OrdnanceStickyClass*)mClass)->mStickTypes & 0x80000000) != 0;
    }
    else
    {
        GameObject* pVictim = pObject->GetGameObject();
        if (pVictim)
        {
            bStick = (((OrdnanceStickyClass*)mClass)->mStickTypes &
                      (1 << pVictim->mHealthType)) != 0;
            PblVector3 vDir;
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&mVelocity));
            static_cast<Damageable*>(pVictim)->ApplyDamage(GetDamage(), vDir, vPos, iBodyId, 0xFFFFFFFF);
        }
    }

    if (bStick && StickInfo::sMemoryPool.mUsed < StickInfo::sMemoryPool.mCount)
    {
        CollisionModel* pModel = pObject->mCollisionModel;

        const PblVector3& vY = gVectorUnitY;
        const float fCrossX = vNormal.y * vY.z - vNormal.z * vY.y;
        const float fCrossY = vNormal.z * vY.x - vNormal.x * vY.z;
        const float fCrossZ = vNormal.x * vY.y - vNormal.y * vY.x;
        PblMatrix mStick(fCrossY * fCrossY + fCrossX * fCrossX + fCrossZ * fCrossZ < 0.0001f
                             ? DirectionMatrix(vNormal, gVectorUnitZ, vPos)
                             : DirectionMatrix(vNormal, gVectorUnitY, vPos));

        mStickInfo = new StickInfo;
        GameObject* pStuckTo = pObject->GetGameObject();
        mStickInfo->mCollHandle.mObject = pStuckTo;
        mStickInfo->mCollHandle.mSavedHandleId = pStuckTo ? GameObjectHandleId(pStuckTo) : 0;
        mStickInfo->mCollObject = pObject;
        if (pModel)
        {
            PblMatrix mBody = pModel->GetWorldMatrix(iBodyId, pObject->GetMatrix(), pObject->GetPose());
            mStick *= mBody.GetInverseFast();
            mStickInfo->mCollBodyID = iBodyId;
        }
        else
        {
            mStickInfo->mCollBodyID = -1;
        }
        mStickInfo->mLocalPosition = mStick.GetTranslation();
        mStickInfo->mLocalRotation = PblQuaternion(mStick);

        mPosition = vPos + vNormal * 0.01f;
        mVelocity = gVectorZero;
        CollisionManager::RemoveOrdnance(this);
        mDecrementLifeTime = true;
        return false;
    }

    if (mNumBounces < 4)
    {
        const float fDot = DotProduct(mVelocity, vNormal);
        if (fDot < 0.0f)
        {
            const float fRebound = 0.37f;
            const float fFriction = 1.15f;
            mPosition = vPos + vNormal * 0.1f;

            mVelocity -= fDot * vNormal;
            float fSlow = -fFriction * fDot * (fRebound + 1.0f) /
                          PblMath::Sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                        mVelocity.z * mVelocity.z);
            fSlow = fSlow < 1.0f ? fSlow : 1.0f;
            mVelocity -= mVelocity * fSlow;

            const float fBounce = fDot * fRebound;
            mVelocity -= fBounce * vNormal;
            if (mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z < 1.0f)
            {
                ++mNumBounces;
            }
        }
        else
        {
            mNumBounces = 4;
        }
    }

    if (mNumBounces >= 4)
    {
        CollisionManager::RemoveOrdnance(this);
        mGravity = 0.0f;
        mVelocity.z = 0.0f;
        mVelocity.y = 0.0f;
        mVelocity.x = 0.0f;
        mDecrementLifeTime = true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00606d40
void OrdnanceSticky::SoundEvent()
{
    AIUtil::GrenadeAlert(mPosition, GetOwner(), &mBeginTime, &mLastEventTime);
}

// FUNCTION: SWBF2 0x00606ef0
OrdnanceStickyClass::~OrdnanceStickyClass()
{
}
