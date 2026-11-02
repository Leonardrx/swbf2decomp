#include "PebbleFL/Common/PblMath.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <d3dx9math.h>

#include "OrdnanceTowCable.h"
#include "Aimer.h"
#include "CollisionManager.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityCarrier.h"
#include "EntityWalker.h"
#include "GameObject.h"
#include "WeaponTowCable.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHermite.h"
#include "PebbleFL/Common/PblRandom.h"

class AttachedEffects;
class Controllable;
class GameModel;
class RedPose;

void sRenderHermiteSegment(PblHermiteVector3& segment, unsigned char ucViewport, float fDist,
                           unsigned int uiFlags);

extern PblVector3 gVectorZero;

// SYNTHETIC: SWBF2 0x00405bc0
// `dynamic initializer for 'OrdnanceTowCable::rttiHashOrdnanceTowCable''
// OrdnanceTowCable::rttiHashOrdnanceTowCable

// GLOBAL: SWBF2 0x01ef26c0
// OrdnanceTowCable::rttiHashOrdnanceTowCable
PblHash OrdnanceTowCable::rttiHashOrdnanceTowCable("OrdnanceTowCable");
// GLOBAL: SWBF2 0x0078aa68
// OrdnanceTowCable::rttiNameOrdnanceTowCable
const char OrdnanceTowCable::rttiNameOrdnanceTowCable[] = "OrdnanceTowCable";

// SYNTHETIC: SWBF2 0x00405bd0
// `dynamic initializer for 'OrdnanceTowCableClass::rttiHashOrdnanceTowCableClass''
// OrdnanceTowCableClass::rttiHashOrdnanceTowCableClass

// GLOBAL: SWBF2 0x01ef26c4
// OrdnanceTowCableClass::rttiHashOrdnanceTowCableClass
PblHash OrdnanceTowCableClass::rttiHashOrdnanceTowCableClass("OrdnanceTowCableClass");
// GLOBAL: SWBF2 0x0078aa3c
// OrdnanceTowCableClass::rttiNameOrdnanceTowCableClass
const char OrdnanceTowCableClass::rttiNameOrdnanceTowCableClass[] = "OrdnanceTowCableClass";

// SYNTHETIC: SWBF2 0x00405ba0
// `dynamic initializer for 'OrdnanceTowCable::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769200
// `dynamic atexit destructor for 'OrdnanceTowCable::sMemoryPool''

// GLOBAL: SWBF2 0x01ef2670
// OrdnanceTowCable::sMemoryPool
MemoryPool OrdnanceTowCable::sMemoryPool("OrdnanceTowCable", sizeof(OrdnanceTowCable));

// GLOBAL: SWBF2 0x01eaed60
unsigned int OrdnanceTowCable::sTextureHash;




// SYNTHETIC: SWBF2 0x00609c29
// ??_EOrdnanceTowCable@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00609c31
// ??_EOrdnanceTowCable@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00607300
// OrdnanceTowCable::`scalar deleting destructor'







// FUNCTION: SWBF2 0x00607380
OrdnanceTowCable::OrdnanceTowCable(OrdnanceTowCableClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc)
{
    sTextureHash = 0;
    if (desc.mFireWeapon->IsRtti(WeaponTowCable::rttiHashWeaponTowCable))
    {
        mFireWeapon = (WeaponTowCable*)desc.mFireWeapon;
    }
    else
    {
        mFireWeapon = NULL;
    }
    mHitWalker = NULL;
    mLastOwnerFirePos = mFireWeapon->mAimer->mFirePos;
    mLastOwnerFireDir = mFireWeapon->mAimer->mDirection;
    mTowCableFallTime = 0.0f;
    mFakeCable = false;
    mDeleteNextUpdate = false;
}

// FUNCTION: SWBF2 0x006074a0
OrdnanceTowCable::~OrdnanceTowCable()
{
    if (mFireWeapon && mOwner)
    {
        if (NetGame::IsEnabled() && !NetGame::IsClient())
        {
            NetGame::CreateEvent_DestroyTowCable(mOwner, mFireWeapon, (unsigned __int64)-1);
        }
        mFireWeapon->SetOrdnance(NULL);
        mFireWeapon = NULL;
    }
    mTowCable.Clear();
}

// FUNCTION: SWBF2 0x006075a0
void OrdnanceTowCable::TowCableFailure()
{
    if (mTowCable.mTowState == TOW_FALL)
    {
        return;
    }

    CollisionManager::RemoveOrdnance(this);

    if (mOwner && mFireWeapon)
    {
        mFireWeapon->SetOrdnance(NULL);
        mFireWeapon = NULL;
        EntityFlyer* pFlyer = (EntityFlyer*)mOwner->GetEntity();
        pFlyer->SetTowCableWalker(NULL);
    }

    PblVector3 vSlope(mLastOwnerFireDir.x * -15.0f, mLastOwnerFireDir.y * -15.0f,
                      mLastOwnerFireDir.z * -15.0f);
    mTowCable.AddAttachPoint(0, mLastOwnerFirePos, vSlope);
    ConvertPointsToWorldSpace();

    for (int i = 0; i < 13; i++)
    {
        TowCableAttachPointData& point = mTowCable.mAttachPoints[i];
        if (!point.mIsAttached)
        {
            break;
        }

        PblVector3 vStart = point.mBodyOffset + PblVector3(0.0f, 20.0f, 0.0f);
        PblVector3 vDown(0.0f, -1.0f, 0.0f);
        CollisionObject* pHit;
        float fHit = CollisionManager::RayHit(vStart, vDown, 512.0f, pHit, NULL, NULL, 0, 0x90, true);
        if (fHit < 1.0f)
        {
            point.mFinalHeight = point.mBodyOffset.y + 0.05f + 20.0f - fHit * 512.0f;
        }
        else
        {
            point.mFinalHeight = -512.0f;
        }
    }

    mTowCable.mTowState = TOW_FALL;
}

// FUNCTION: SWBF2 0x00607760
bool OrdnanceTowCable::Collide(CollisionObject* pObject, const PblVector3& vPos, const PblVector3& vNormal,
                               int iBodyId)
{
    if (mFakeCable)
    {
        mDeleteNextUpdate = true;
        return false;
    }

    CollisionModel* pModel = pObject->mCollisionModel;
    if (pModel)
    {
        CollisionObjectType eType = COLL_TOWCABLE;
        CollisionMask mask = pModel->GetCollisionMask(eType);
        if (mask.IsSet(iBodyId))
        {
            Entity* pEntity = pObject->GetEntity();
            if (pEntity && pEntity->IsRtti(EntityWalker::rttiHashEntityWalker))
            {
                mTowCable.mTowState = TOW_WRAP;
                mHitWalker = pEntity->GetGameObject();
                mTowCable.Clear();

                CollisionObject* pWalkerCollision = mHitWalker->GetCollisionObject();
                PblMatrix mBody = pModel->GetWorldMatrix(iBodyId, mHitWalker->GetMatrix(), pWalkerCollision->GetPose());
                PblVector3 vLocal;
                D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vPos,
                                       (const D3DXMATRIX*)&mBody.GetInverseFast());
                mTowCable.AddAttachPoint(iBodyId, vLocal, gVectorZero);
                CollisionManager::RemoveOrdnance(this);

                if (mOwner)
                {
                    EntityFlyer* pFlyer = (EntityFlyer*)mOwner->GetEntity();
                    pFlyer->SetTowCableWalker(mHitWalker);
                }

                OrdnanceDesc desc;
                if (NetGame::IsEnabled() && !NetGame::IsClient())
                {
                    PblVector3 vDir;
                    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocity);
                    NetGame::CreateEvent_ConnectTowCable(mOwner, mFireWeapon, vDir, vPos, iBodyId, mHitWalker,
                                                         (unsigned __int64)-1);
                }
                return false;
            }
        }
    }

    mTowCable.AddAttachPoint(0, GetPosition(), gVectorZero);
    TowCableFailure();
    CollisionManager::RemoveOrdnance(this);
    return false;
}

// FUNCTION: SWBF2 0x00607a10
void OrdnanceTowCable::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (mOwner && (mTowCable.mTowState == TOW_WRAP || mTowCable.mTowState == TOW_ATTACHING))
    {
        PblVector3 vFirePos = mLastOwnerFirePos;
        PblVector3 vEndPos = GetPosition();
        if (mOwner)
        {
            PblMatrix mOwnerMatrix = mOwner->GetMatrix();
            PblMatrix mSmoothed = mOwner->GetSmoothedMatrix();
            PblVector3 v;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&v, (const D3DXVECTOR3*)&vFirePos,
                                   (const D3DXMATRIX*)&mOwnerMatrix.GetInverseFast());
            vFirePos = v;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&v, (const D3DXVECTOR3*)&vFirePos, (const D3DXMATRIX*)&mSmoothed);
            vFirePos = v;
        }

        PblVector3 vEndSlope;
        const PblVector3* pEndSlope;
        if (mHitWalker)
        {
            int i;
            for (i = 0; i < 13; i++)
            {
                if (!mTowCable.mAttachPoints[i].mIsAttached)
                {
                    break;
                }
            }
            if (i >= 13)
            {
                return;
            }

            CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
            CollisionObject* pWalkerCollision = mHitWalker->GetCollisionObject();
            TowCableAttachPointData& point = mTowCable.mAttachPoints[i - 1];
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vEndPos, (const D3DXVECTOR3*)&point.mBodyOffset,
                                   (const D3DXMATRIX*)&pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(),
                                                                              pWalkerCollision->GetPose()));
            pEndSlope = &point.mSlopeOut;
        }
        else
        {
            vEndSlope.x = 0.0f;
            vEndSlope.y = 0.0f;
            vEndSlope.z = 0.0f;
            pEndSlope = &vEndSlope;
        }

        PblVector3 vFireSlope(mLastOwnerFireDir.x * -15.0f, mLastOwnerFireDir.y * -15.0f,
                              mLastOwnerFireDir.z * -15.0f);
        PblHermiteVector3 segment;
        segment.Set(vEndPos, vFirePos, *pEndSlope, vFireSlope, 1.0f);
        sRenderHermiteSegment(segment, ucViewport, fDist, uiFlags);
    }

    if (mTowCable.mTowState != TOW_ATTACHING)
    {
        float fFade = fDist;
        if (mTowCableFallTime > 8.0f)
        {
            fFade = (10.0f - mTowCableFallTime) * 0.5f * fDist;
        }
        mTowCable.Render(mHitWalker, ucViewport, fFade, uiFlags);
    }
}

// FUNCTION: SWBF2 0x00607e70
void OrdnanceTowCable::RecalculateCenterAndRadius()
{
    if (!mOwner)
    {
        return;
    }
    if (mTowCable.mTowState == TOW_DAMAGE || mTowCable.mTowState == TOW_FALL)
    {
        return;
    }

    PblVector3 vOwnerPos = mOwner->GetCollisionObject()->GetCollisionCenter();
    float fOwnerRadius = mOwner->GetCollisionObject()->GetCollisionRadius();
    PblVector3 vEndPos = GetPosition();
    float fEndRadius = _Sphere.w;
    if (mHitWalker)
    {
        vEndPos = mHitWalker->GetCollisionObject()->GetCollisionCenter();
        fEndRadius = mHitWalker->GetCollisionObject()->GetCollisionRadius();
    }

    PblVector3 vDelta(vOwnerPos.x - vEndPos.x, vOwnerPos.y - vEndPos.y, vOwnerPos.z - vEndPos.z);
    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vDelta);
    if (vDir.y * vDir.y + vDir.x * vDir.x + vDir.z * vDir.z < 0.0001f)
    {
        SetRenderRadius(fOwnerRadius > fEndRadius ? fOwnerRadius : fEndRadius);
        SetRenderPos(vOwnerPos);
        return;
    }

    PblVector3 vOwnerEdge = vDir * fOwnerRadius + vOwnerPos;
    PblVector3 vEndEdge = vEndPos - vDir * fEndRadius;
    float fX = vOwnerEdge.x - vEndEdge.x;
    float fY = vOwnerEdge.y - vEndEdge.y;
    float fZ = vOwnerEdge.z - vEndEdge.z;
    SetRenderRadius(PblMath::Sqrt(fX * fX + fY * fY + fZ * fZ) * 0.5f);
    SetRenderPos((vEndEdge + vOwnerEdge) * 0.5f);
}

// FUNCTION: SWBF2 0x006081c0
bool OrdnanceTowCable::Update(float fDeltaTime)
{
    if (mDeleteNextUpdate)
    {
        return false;
    }

    if (mOwner)
    {
        if (mFireWeapon)
        {
            mLastOwnerFirePos = mFireWeapon->mAimer->mFirePos;
            mLastOwnerFireDir = mFireWeapon->mAimer->mDirection;
        }
    }
    else if (mTowCable.mTowState != TOW_DAMAGE)
    {
        TowCableFailure();
    }

    if (mTowCable.mTowState != TOW_WRAP && mTowCable.mTowState != TOW_DAMAGE &&
        mTowCable.mTowState != TOW_FALL)
    {
        bool bResult = OrdnanceBullet::Update(fDeltaTime);
        RecalculateCenterAndRadius();
        const PblVector3& vPos = GetPosition();
        float fX = mLastOwnerFirePos.x - vPos.x;
        float fY = mLastOwnerFirePos.y - vPos.y;
        float fZ = mLastOwnerFirePos.z - vPos.z;
        float fDist = PblMath::Sqrt(fY * fY + fX * fX + fZ * fZ);
        if (fDist > ((OrdnanceTowCableClass*)mClass)->mInitialCableLimit)
        {
            return false;
        }
        return bResult;
    }

    RecalculateCenterAndRadius();
    if (mHitWalker)
    {
        if (!mHitWalker->IsAlive())
        {
            CalculateWorldSpacePoints();
        }
    }
    else
    {
        TowCableFailure();
    }

    int iNumPoints;
    for (iNumPoints = 0; iNumPoints < 13; iNumPoints++)
    {
        if (!mTowCable.mAttachPoints[iNumPoints].mIsAttached)
        {
            break;
        }
    }

    if (mHitWalker)
    {
        mTowCable.UpdateAttachPoints(fDeltaTime, mHitWalker);
        if (mTowCable.mTowState == TOW_DAMAGE)
        {
            int iPoint = (int)(PblRandom::GetGlobalFloat() * (float)(iNumPoints - 1));
            TowCableAttachPointData& point = mTowCable.mAttachPoints[iPoint];
            CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
            const RedPose* pPose = mHitWalker->GetCollisionObject()->GetPose();

            PblVector3 vPos =
                pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(), pPose).ApplyTransform(point.mBodyOffset);
            PblVector3 vDir = Normalize(
                pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(), pPose).ApplyRotation(point.mBodyOffset));

            DamageDesc damage = GetDamage();
            damage.mBaseValue = mHitWalker->mMaxHealth;
            float fSameTeamDamageScale = Damageable::sSameTeamDamageScale;
            Damageable::SetTeamDamageScale(1.0f);
            if (mOwner)
            {
                for (PblList<Controllable>::Itor itor(mOwner->GetControllableList()); (Controllable*)itor; ++itor)
                {
                    if (!itor->GetPilot())
                    {
                        continue;
                    }
                    damage.mDamageOwner.mCharacter = itor->GetPilot()->mCharacter;
                    mHitWalker->ApplyDamage(damage, vDir, vPos, point.mBodyID, (unsigned int)-1);
                }
            }
            Damageable::SetTeamDamageScale(fSameTeamDamageScale);
        }
    }
    else
    {
        mTowCable.UpdateAttachPoints(fDeltaTime, NULL);
    }

    if (mTowCable.mTowState == TOW_FALL)
    {
        mTowCableFallTime += fDeltaTime;
    }
    if (mTowCableFallTime > 10.0f)
    {
        return false;
    }

    if (!mOwner || !mHitWalker || mTowCable.mTowState != TOW_WRAP)
    {
        return true;
    }

    if (iNumPoints < 13)
    {
        TowCableAttachPointData& last = mTowCable.mAttachPoints[iNumPoints - 1];
        CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
        CollisionObject* pWalkerCollision = mHitWalker->GetCollisionObject();
        PblVector3 vStart = pModel->GetWorldMatrix(last.mBodyID, mHitWalker->GetMatrix(), pWalkerCollision->GetPose())
                     .ApplyTransform(last.mBodyOffset);
        PblVector3 vFirePos = mLastOwnerFirePos;
        int iLastBodyId = last.mBodyID;
        int iPrevBodyId = -1;
        if (iNumPoints > 2)
        {
            iPrevBodyId = mTowCable.mAttachPoints[iNumPoints - 2].mBodyID;
        }

        PblVector3 vFireSlope(mLastOwnerFireDir.x * -15.0f, mLastOwnerFireDir.y * -15.0f,
                              mLastOwnerFireDir.z * -15.0f);
        PblHermiteVector3 segment(vStart, vFirePos, last.mSlopeOut, vFireSlope, 1.0f);
        OrdnanceTowCableClass* pClass = (OrdnanceTowCableClass*)mClass;
        float fLimit = iNumPoints == 1 ? pClass->mInitialCableLimit : pClass->mCableLimit;
        if (segment.GetLengthApproximation(10) > fLimit)
        {
            TowCableFailure();
            return true;
        }

        for (float fT = 0.0f; fT < 1.0f; fT += 0.1f)
        {
            PblVector3 vA = segment.getValue(fT);
            PblVector3 vB = segment.getValue(fT + 0.1f);
            float fX = vB.x - vA.x;
            float fY = vB.y - vA.y;
            float fZ = vB.z - vA.z;
            float fLength = PblMath::Sqrt(fY * fY + fX * fX + fZ * fZ);
            if (fLength > 0.0f)
            {
                float fInvLength = 1.0f / fLength;
                PblVector3 vDir(fX * fInvLength, fInvLength * fY, fInvLength * fZ);
                PblVector3 vNormal;
                int iBodyId;
                float fHit =
                    mHitWalker->GetCollisionObject()->RayHit(vA, vDir, fLength, &vNormal, &iBodyId, true, 0.0f);
                if (fHit < 1.0f)
                {
                    CollisionObjectType eType = COLL_TOWCABLE;
                    CollisionMask mask = pModel->GetCollisionMask(eType);
                    if (!mask.IsSet(iBodyId))
                    {
                        TowCableFailure();
                        return true;
                    }
                    if (iBodyId != last.mBodyID)
                    {
                        EntityWalker* pWalker = (EntityWalker*)mHitWalker->GetEntity();
                        int iValid = pWalker->IsTowCableCollisionValid(iBodyId, iLastBodyId, iPrevBodyId);
                        if (iValid > 0)
                        {
                            PblMatrix mHitBody = pModel->GetWorldMatrix(iBodyId, mHitWalker->GetMatrix(),
                                                                        mHitWalker->GetCollisionObject()->GetPose());
                            PblVector3 vHit(vDir.x * fHit + vA.x, vDir.y * fHit + vA.y, vDir.z * fHit + vA.z);
                            PblVector3 vLocal;
                            D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vHit,
                                                   (const D3DXMATRIX*)&mHitBody.GetInverseFast());
                            PblVector3 vSlope = segment.getSlope(fHit * 0.1f + fT);
                            vSlope.x *= 0.5f;
                            vSlope.y *= 0.5f;
                            vSlope.z *= 0.5f;
                            mTowCable.AddAttachPoint(iBodyId, vLocal, vSlope);
                            return true;
                        }
                        if (iValid < 0)
                        {
                            TowCableFailure();
                            return true;
                        }
                    }
                }
            }
        }
        return true;
    }

    mTowCable.mTowState = TOW_DAMAGE;
    if (mOwner)
    {
        EntityFlyer* pFlyer = (EntityFlyer*)mOwner->GetEntity();
        pFlyer->SetTowCableWalker(NULL);
        if (mFireWeapon)
        {
            mFireWeapon->SetOrdnance(NULL);
            mFireWeapon = NULL;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00608e90
void OrdnanceTowCable::ConvertPointsToWorldSpace()
{
    if (mHitWalker)
    {
        CalculateWorldSpacePoints();
    }

    for (int i = 0; i < 13; i++)
    {
        TowCableAttachPointData& point = mTowCable.mAttachPoints[i];
        if (!point.mIsAttached)
        {
            break;
        }
        if (point.mBodyID >= 0)
        {
            point.mBodyID = -1;
            point.mBodyOffset = point.mWorldBodyOffset;
            point.mInitialBodyOffset = point.mWorldInitialBodyOffset;
        }
    }
}

// FUNCTION: SWBF2 0x00608f10
void OrdnanceTowCable::CalculateWorldSpacePoints()
{
    if (!mHitWalker)
    {
        return;
    }

    for (int i = 0; i < 13; i++)
    {
        TowCableAttachPointData& point = mTowCable.mAttachPoints[i];
        if (!point.mIsAttached)
        {
            return;
        }

        if (point.mBodyID >= 0)
        {
            CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
            const RedPose* pPose = mHitWalker->GetCollisionObject()->GetPose();
            PblVector3 vWorld;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorld, (const D3DXVECTOR3*)&point.mBodyOffset,
                                   (const D3DXMATRIX*)&pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(),
                                                                              pPose));
            PblVector3 vWorldInitial;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorldInitial, (const D3DXVECTOR3*)&point.mInitialBodyOffset,
                                   (const D3DXMATRIX*)&pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(),
                                                                              pPose));
            point.mWorldBodyOffset = vWorld;
            point.mWorldInitialBodyOffset = vWorldInitial;
        }
        else
        {
            point.mWorldBodyOffset = point.mBodyOffset;
            point.mWorldInitialBodyOffset = point.mInitialBodyOffset;
        }
    }
}

// FUNCTION: SWBF2 0x00609110
OrdnanceTowCableClass::OrdnanceTowCableClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mInitialCableLimit = 50.0f;
    mCableLimit = 45.0f;
}

// FUNCTION: SWBF2 0x006091b0
OrdnanceTowCableClass::~OrdnanceTowCableClass()
{
}

// SYNTHETIC: SWBF2 0x00609140
// OrdnanceTowCableClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00609170
OrdnanceTowCableClass::OrdnanceTowCableClass(OrdnanceTowCableClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mInitialCableLimit = pBase->mInitialCableLimit;
    mCableLimit = pBase->mCableLimit;
}

// FUNCTION: SWBF2 0x006091c0
OrdnanceClass* OrdnanceTowCableClass::Derive(unsigned int uHash)
{
    return new OrdnanceTowCableClass(this, uHash);
}

// FUNCTION: SWBF2 0x00609230
Ordnance* OrdnanceTowCableClass::Build(const OrdnanceDesc& desc)
{
    if (OrdnanceTowCable::sMemoryPool.mUsed >= OrdnanceTowCable::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceTowCable* pCable = new OrdnanceTowCable(this, desc);
    pCable->ActivateThread(-40, PER_LOCAL_TURN);
    pCable->AddOrdnanceToCollisionManager();
    return pCable;
}

// FUNCTION: SWBF2 0x006092d0
void OrdnanceTowCableClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xE4C659B4:
        sscanf(pszValue, "%f", &mInitialCableLimit);
        break;
    case 0xD9B746B2:
        sscanf(pszValue, "%f", &mCableLimit);
        break;
    default:
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00609320
TowCableRenderObject::TowCableRenderObject()
{
    mTowState = TOW_ATTACHING;
    memset(mAttachPoints, 0, sizeof(mAttachPoints));
}

// FUNCTION: SWBF2 0x00609340
void TowCableRenderObject::Clear()
{
    memset(mAttachPoints, 0, sizeof(mAttachPoints));
}

// FUNCTION: SWBF2 0x00609360
void TowCableRenderObject::AddAttachPoint(int iBodyID, const PblVector3& vOffset, const PblVector3& vSlope)
{
    int i;
    for (i = 0; i < 13; i++)
    {
        if (!mAttachPoints[i].mIsAttached)
        {
            break;
        }
    }
    if (i >= 13)
    {
        return;
    }

    TowCableAttachPointData& point = mAttachPoints[i];
    point.mBodyID = iBodyID;
    point.mIsAttached = true;
    point.mInitialBodyOffset = vOffset;
    point.mBodyOffset = vOffset;
    point.mSlopeIn = vSlope;
    point.mSlopeOut = vSlope;
    point.mFinalSlope = vSlope;
    point.mFinalSlope.y = 0.0f;
    float fLength = Length(point.mFinalSlope);
    if (fLength > 0.0f)
    {
        float fScale = 10.0f / fLength;
        point.mFinalSlope *= fScale;
    }
    point.mFallSpeed = 0.0f;
    point.mFinalHeight = 0.0f;
}

// FUNCTION: SWBF2 0x006094a0
void TowCableRenderObject::UpdateAttachPoints(float fDeltaTime, GameObject* pObject)
{
    if ((mTowState == TOW_WRAP || mTowState == TOW_DAMAGE) && pObject)
    {
        for (int i = 1; i < 13; i++)
        {
            TowCableAttachPointData& point = mAttachPoints[i];
            if (!point.mIsAttached)
            {
                break;
            }

            PblVector3& vIn = point.mSlopeIn;
            float fLength = Length(vIn);
            float fStep = fDeltaTime * 30.0f;
            if (fLength <= fStep)
            {
                vIn = gVectorZero;
            }
            else
            {
                vIn -= fStep * vIn / fLength;
            }

            PblVector3& vOut = mAttachPoints[i - 1].mSlopeOut;
            fLength = Length(vOut);
            if (fLength <= fStep)
            {
                vOut = gVectorZero;
            }
            else
            {
                vOut -= fStep * vOut / fLength;
            }

            EntityWalker* pWalker = (EntityWalker*)pObject->GetEntity();
            PblVector3 vTarget = pWalker->GetTowCableCollisionOffset(point.mBodyID);
            vTarget.y = point.mBodyOffset.y;
            PblVector3 vDiff = vTarget - point.mBodyOffset;
            fLength = Length(vDiff);
            if (fLength <= fStep)
            {
                point.mBodyOffset = vTarget;
            }
            else
            {
                point.mBodyOffset += vDiff * fStep / fLength;
            }
        }
    }
    else if (mTowState == TOW_FALL)
    {
        float fPrevFallSpeed = -5.0f;
        for (int i = 12; i >= 0; i--)
        {
            TowCableAttachPointData& point = mAttachPoints[i];
            if (!point.mIsAttached || !(fPrevFallSpeed < -4.0f))
            {
                continue;
            }

            point.mFallSpeed -= fDeltaTime * 9.8f;
            fPrevFallSpeed = point.mFallSpeed;
            point.mInitialBodyOffset.y += point.mFallSpeed * fDeltaTime;
            if (point.mBodyOffset.y > point.mFinalHeight)
            {
                point.mBodyOffset.y = point.mInitialBodyOffset.y;
                if (point.mFinalHeight > point.mInitialBodyOffset.y)
                {
                    point.mBodyOffset.y = point.mFinalHeight;
                }
            }
            else
            {
                point.mInitialBodyOffset.y = point.mBodyOffset.y;
            }

            const PblVector3& vFinal = point.mFinalSlope;
            float fLength = Length(vFinal);
            PblVector3& vIn = point.mSlopeIn;
            if (Length(vIn) + fDeltaTime > fLength)
            {
                vIn = vFinal;
            }
            else
            {
                vIn += fDeltaTime * vFinal / fLength;
            }

            PblVector3& vOut = point.mSlopeOut;
            fLength = Length(vOut);
            if (fDeltaTime + fLength > fLength)
            {
                vOut = vFinal;
            }
            else
            {
                vOut += vFinal * fDeltaTime / fLength;
            }

            PblVector3 vDiff = point.mInitialBodyOffset - point.mBodyOffset;
            fLength = Length(vDiff);
            if (fLength <= fDeltaTime)
            {
                point.mBodyOffset = point.mInitialBodyOffset;
            }
            else
            {
                point.mBodyOffset += vDiff * fDeltaTime / fLength;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00609a80
void TowCableRenderObject::Render(GameObject* pObject, unsigned char ucViewport, float fDist,
                                  unsigned int uiFlags)
{
    PblVector3 vPrev;
    int i = 1;
    if (mAttachPoints[0].mIsAttached)
    {
        if (mAttachPoints[0].mBodyID >= 0)
        {
            if (!pObject)
            {
                return;
            }
            CollisionObject* pCollision = pObject->GetCollisionObject();
            CollisionModel* pModel = pObject->GetCollisionObject()->mCollisionModel;
            PblVector3 vWorld;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorld, (const D3DXVECTOR3*)&mAttachPoints[0].mBodyOffset,
                                   (const D3DXMATRIX*)&pModel->GetWorldMatrix(mAttachPoints[0].mBodyID,
                                                                              pObject->GetMatrix(),
                                                                              pCollision->GetPose()));
            vPrev = vWorld;
        }
        else
        {
            vPrev = mAttachPoints[0].mBodyOffset;
        }
    }

    for (; i < 13; i++)
    {
        TowCableAttachPointData& point = mAttachPoints[i];
        if (!point.mIsAttached)
        {
            return;
        }

        PblVector3 vStart = vPrev;
        PblVector3 vEnd;
        if (point.mBodyID >= 0)
        {
            if (!pObject)
            {
                return;
            }
            CollisionObject* pCollision = pObject->GetCollisionObject();
            CollisionModel* pModel = pObject->GetCollisionObject()->mCollisionModel;
            PblVector3 vWorld;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorld, (const D3DXVECTOR3*)&point.mBodyOffset,
                                   (const D3DXMATRIX*)&pModel->GetWorldMatrix(point.mBodyID, pObject->GetMatrix(),
                                                                              pCollision->GetPose()));
            vEnd = vWorld;
        }
        else
        {
            vEnd = point.mBodyOffset;
        }
        vPrev = vEnd;

        PblHermiteVector3 segment(vStart, vPrev, mAttachPoints[i - 1].mSlopeOut, point.mSlopeIn, 1.0f);
        sRenderHermiteSegment(segment, ucViewport, fDist, uiFlags);
    }
}


