#include "Chunk.h"
#include "PebbleFL/Common/PblValues.h"
#include "PebbleFL/Common/PblVector.h"

#include <d3dx9math.h>

#include "CollisionManager.h"
#include "EntityGeometry.h"
#include "FLEffect.h"
#include "GameModel.h"
#include "GameObject.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"



static const float DYNAMIC_LIFETIME = 8.0f;
static const float STATIC_LIFETIME = 53.0f;
static const float EPSILON = 0.0001f;

static inline const PblVector3 TransformCoord(const PblVector3& v, const PblMatrix& m)
{
    PblVector3 result;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&result, (const D3DXVECTOR3*)&v, (const D3DXMATRIX*)&m);
    return result;
}

// FUNCTION: SWBF2 0x00454e30
void Chunk::SetPosition(const PblVector3& vPos)
{
    mMatrix.SetTranslation(vPos);
    mCollisionMatrix = mPrimitiveMatrix.GetInverseFast() * mMatrix;
    SetCollisionCenter(mCollisionMatrix.GetTranslation());
}

// FUNCTION: SWBF2 0x00454f10
const PblVector3& Chunk::GetPosition() const
{
    return mMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x00454f20
const PblMatrix& Chunk::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00454f30
const PblVector3& Chunk::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00454f40
const PblVector3& Chunk::GetOmega() const
{
    return mOmega;
}

// FUNCTION: SWBF2 0x00454f50
bool Chunk::IsAChunk() const
{
    return true;
}

// FUNCTION: SWBF2 0x00454f60
Chunk::Chunk()
    : CollisionObject(COLL_UNKNOWN, NULL, NULL, NULL),
      mIsValid(false),
      mIsMoving(false),
      mMatrix(gMatrixIdentity),
      mModel(NULL),
      mMaxTerrainCollisions(0),
      mNumTerrainCollisions(0)
{
    SetCollisionModel(&mSimpleCollisionModel);
    mTotalUpdateDt = 0.0f;
    mJustRendered = false;
}

// SYNTHETIC: SWBF2 0x00455080
// Chunk::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x004550c0
// Chunk::~Chunk

static const PblVector3 vGravity(0.0f, -9.8f, 0.0f);

// FUNCTION: SWBF2 0x00455150
void Chunk::ClearAll()
{
    mIsValid = false;
    mIsMoving = false;
    mMaxTerrainCollisions = 0;
    mNumTerrainCollisions = 0;
    mMatrix = gMatrixIdentity;
    SetCollisionRadius(1.0f);
    mModel = NULL;
    RemoveFromCollisionManager();
    mSimpleCollisionModel.ClearAll(false);
    mFullCollisionModel = NULL;
    SetCollisionModel(&mSimpleCollisionModel);

    mAngularMomentum.z = 0.0f;
    mAngularMomentum.y = 0.0f;
    mAngularMomentum.x = 0.0f;
    mOmega.z = 0.0f;
    mOmega.y = 0.0f;
    mOmega.x = 0.0f;
    mGravity = vGravity;
    mMaxTerrainCollisions = 0;
    mNumTerrainCollisions = 0;
    mTerrainImpactEffect = 0;
    memset(mTrailEffect, 0, sizeof(mTrailEffect));
    memset(mSmokeEffect, 0, sizeof(mSmokeEffect));
    mPhysicsType = PHYSICS_SIMPLE_RIGID;

    for (int i = 0; i < 3; i++)
    {
        if (mTrailEmitters[i])
        {
            mTrailEmitters[i]->StopAndFinish();
            mTrailEmitters[i] = NULL;
        }
        if (mSmokeEmitters[i])
        {
            mSmokeEmitters[i]->StopAndFinish();
            mSmokeEmitters[i] = NULL;
        }
    }
    mTotalUpdateDt = 0.0f;
    mJustRendered = false;
}

// FUNCTION: SWBF2 0x00455300
void Chunk::FixPosition()
{
    mLifeTime = 0.0f;
    CreateSmokeEmitters();
    mIsMoving = false;
    if (RedTerrain::_bEnable)
    {
        AddStaticToCollisionManager();
    }
    if (mFullCollisionModel)
    {
        SetCollisionModel(mFullCollisionModel);
    }
}

// FUNCTION: SWBF2 0x00455340
void Chunk::SetData(ChunkDescriptor& desc)
{
    RedPose pose;

    bCollisionOnUpdate = false;
    mIsValid = true;
    mIsMoving = true;
    mLifeTime = 0.0f;
    mMatrix = desc.mMatrix;

    float fRadius = desc.mPrimitive->GetRadius();
    SetCollisionRadius(fRadius);

    mModel = desc.mModel;
    mModel->GetRedModel(0)->SetupPose(pose);
    mVelocity = desc.mVelocity;

    mBodyPrimitive.Init(desc.mPrimitive->mPrimitive, "", desc.mPrimitive->mMatrix,
                        desc.mPrimitive->mParentHashName);
    mSimpleCollisionModel.AddBody(&mBodyPrimitive);
    mFullCollisionModel = desc.mFullCollisionModel;

    mNumTerrainCollisions = 0;
    mMaxTerrainCollisions = desc.mMaxTerrainCollisions;
    mTerrainImpactEffect = desc.mTerrainImpactEffect;
    memcpy(mTrailEffect, desc.mTrailEffect, sizeof(mTrailEffect));
    memcpy(mSmokeEffect, desc.mSmokeEffect, sizeof(mSmokeEffect));
    memcpy(mTrailAttachPoints, desc.mTrailAttachPoints, sizeof(mTrailAttachPoints));
    memcpy(mSmokeAttachPoints, desc.mSmokeAttachPoints, sizeof(mSmokeAttachPoints));
    mPhysicsType = (ChunkPhysicsType)desc.mPhysicsType;
    mCoefficientOfRestitution = desc.mCoefficientOfRestitution;
    mCoefficientOfFriction = desc.mCoefficientOfFriction;
    mSimpleFrictionConstant = desc.mSimpleFrictionConstant;
    mMinDeathCollisionSpeed = desc.mMinDeathCollisionSpeed;
    mForceZeroYAxisVelocity = desc.mForceZeroYAxisVelocity;
    mMaxStaticLifetime = desc.mMaxStaticLifetime;
    mGravity = desc.mGravity;

    CreateTrailEmitters();

    mMass = 40.0f;
    mOneOverMass = 1.0f / 40.0f;
    PblMatrix bodyInertia = desc.mPrimitive->GetMomentOfInertia();
    mInverseBodyInertiaTensor = gMatrixIdentity;
    bodyInertia.m[0][0] *= 40.0f;
    bodyInertia.m[1][1] *= 40.0f;
    bodyInertia.m[2][2] *= 40.0f;
    if (bodyInertia.m[0][0] > 0.0f)
    {
        mInverseBodyInertiaTensor.m[0][0] = 1.0f / bodyInertia.m[0][0];
    }
    if (bodyInertia.m[1][1] > 0.0f)
    {
        mInverseBodyInertiaTensor.m[1][1] = 1.0f / bodyInertia.m[1][1];
    }
    if (bodyInertia.m[2][2] > 0.0f)
    {
        mInverseBodyInertiaTensor.m[2][2] = 1.0f / bodyInertia.m[2][2];
    }

    PblMatrix localMatrix = desc.mPrimitive->GetLocalMatrix(&pose);
    mCollisionMatrix = localMatrix * mMatrix;
    SetCollisionCenter(mCollisionMatrix.GetTranslation());
    RecalculateAABB();

    if (mPhysicsType == PHYSICS_STATIC)
    {
        if (mFullCollisionModel)
        {
            SetCollisionModel(mFullCollisionModel);
        }
        AddStaticToCollisionManager();
        mMaxTerrainCollisions = 0;
        mNumTerrainCollisions = 1;
        mIsMoving = false;
    }
    else if (RedTerrain::_bEnable || desc.mKeepSoldierCollisions)
    {
        AddSoftToCollisionManager();
    }

    PblMatrix rotation = mCollisionMatrix;
    rotation.m[3][0] = 0.0f;
    rotation.m[3][1] = 0.0f;
    rotation.m[3][2] = 0.0f;
    rotation.m[3][3] = 1.0f;
    const PblMatrix rotationInverse = rotation.GetInverseFast();
    mInverseWorldInertiaTensor = rotationInverse * mInverseBodyInertiaTensor * rotation;
    mAngularMomentum = TransformCoord(desc.mOmega, bodyInertia * rotation);
    mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);
    const PblMatrix primitiveInverse = localMatrix.GetInverseFast();
    mPrimitiveMatrix = primitiveInverse;

    if (!desc.mKeepSoldierCollisions)
    {
        mSimpleCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(0));
    }
    if (mFullCollisionModel)
    {
        mFullCollisionModel->SetCollisionMask(COLL_SOFT, CollisionMask(0));
    }
    if (desc.mNoBuildingCollisions)
    {
        mSimpleCollisionModel.SetCollisionMask(COLL_STATIC, CollisionMask(0));
    }

    if (mPhysicsType == PHYSICS_COCKPIT)
    {
        const PblMatrix& m = mCollisionMatrix;
        float fX = DotProduct(m.GetXAxis(), mVelocity);
        float fY = DotProduct(m.GetYAxis(), mVelocity);
        float fZ = DotProduct(m.GetZAxis(), mVelocity);
        if (fX > fY)
        {
            mForwardAxis = (fX > fZ) ? 0 : 2;
        }
        else
        {
            mForwardAxis = (fY > fZ) ? 1 : 2;
        }
    }
    mLinearDamping = desc.mLinearDamping;
    mAngularDamping = desc.mAngularDamping;
}

// FUNCTION: SWBF2 0x00455a90
void Chunk::CreateTrailEmitters()
{
    for (int i = 0; i < 3; i++)
    {
        if (!mTrailEffect[i])
        {
            continue;
        }

        FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mTrailEffect[i]);
        if (!pClass)
        {
            continue;
        }
        FLEffectObject* pEffect = pClass->BuildEffect();
        if (!pEffect)
        {
            continue;
        }

        if (mTrailAttachPoints[i])
        {
            RedPose pose;
            mModel->GetRedModel(0)->SetupPose(pose);
            const PblMatrix* pAttach = pose.Find(mTrailAttachPoints[i]);
            if (pAttach)
            {
                pEffect->AttachEffectToMatrix(&mMatrix, *pAttach, true);
            }
            else
            {
                pEffect->AttachEffectToMatrix(&mCollisionMatrix, true);
            }
        }
        else
        {
            pEffect->AttachEffectToMatrix(&mCollisionMatrix, true);
        }
        mTrailEmitters[i] = pEffect;
    }
}

// FUNCTION: SWBF2 0x00455b90
void Chunk::CreateSmokeEmitters()
{
    for (int i = 0; i < 3; i++)
    {
        if (!mSmokeEffect[i])
        {
            continue;
        }

        FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mSmokeEffect[i]);
        if (!pClass)
        {
            continue;
        }
        FLEffectObject* pEffect = pClass->BuildEffect();
        if (!pEffect)
        {
            continue;
        }

        if (mSmokeAttachPoints[i])
        {
            RedPose pose;
            mModel->GetRedModel(0)->SetupPose(pose);
            const PblMatrix* pAttach = pose.Find(mSmokeAttachPoints[i]);
            if (pAttach)
            {
                pEffect->AttachEffectToMatrix(&mMatrix, *pAttach, true);
            }
            else
            {
                pEffect->AttachEffectToMatrix(&mMatrix, true);
            }
        }
        else
        {
            pEffect->AttachEffectToMatrix(&mMatrix, true);
        }
        mSmokeEmitters[i] = pEffect;
    }
}




static inline void SetAxis(PblMatrix& m, int iRow, const PblVector3& v)
{
    m.m[iRow][0] = v.x;
    m.m[iRow][1] = v.y;
    m.m[iRow][2] = v.z;
    m.m[iRow][3] = 0.0f;
}



// FUNCTION: SWBF2 0x00455c90
void Chunk::Update(float fDeltaTime)
{
    if (!mIsValid)
    {
        return;
    }

    if (!mIsMoving || !RedTerrain::_bEnable || mMaxTerrainCollisions == 255)
    {
        float fMaxLifeTime =
            (mPhysicsType == PHYSICS_STATIC || mPhysicsType == PHYSICS_COCKPIT) ? STATIC_LIFETIME : DYNAMIC_LIFETIME;
        if (mMaxStaticLifetime >= 0.0f)
        {
            fMaxLifeTime = mMaxStaticLifetime;
        }
        if (mMaxTerrainCollisions == 255)
        {
            fMaxLifeTime = 30.0f;
        }
        if (mLifeTime < fMaxLifeTime)
        {
            mLifeTime += fDeltaTime;
            if (mLifeTime >= fMaxLifeTime)
            {
                ClearAll();
            }
        }
        if (!mIsMoving)
        {
            return;
        }
    }

    if (mJustRendered)
    {
        mTotalUpdateDt = fDeltaTime;
        mJustRendered = false;
    }
    else
    {
        mTotalUpdateDt += fDeltaTime;
    }

    bCollisionOnUpdate = false;
    PblVector3 vPos = mCollisionMatrix.GetTranslation();
    PblVector3 vForce(0.0f, 0.0f, 0.0f);
    vForce += mGravity * mMass;
    vForce -= mVelocity * mLinearDamping;

    if (mPhysicsType == PHYSICS_COCKPIT)
    {
        switch (mForwardAxis)
        {
        case 0:
        {
            PblVector3 vForward = Normalize(mVelocity);
            PblVector3 vZ = Normalize(CrossProduct(vForward, mCollisionMatrix.GetYAxis()));
            PblVector3 vY = CrossProduct(vZ, vForward);
            mCollisionMatrix[0] = vForward;
            mCollisionMatrix.m[0][3] = 0.0f;
            mCollisionMatrix[1] = vY;
            mCollisionMatrix.m[1][3] = 0.0f;
            mCollisionMatrix[2] = vZ;
            mCollisionMatrix.m[2][3] = 0.0f;
            break;
        }
        case 1:
        {
            PblVector3 vForward = Normalize(mVelocity);
            PblVector3 vX = Normalize(CrossProduct(vForward, mCollisionMatrix.GetZAxis()));
            PblVector3 vZ = CrossProduct(vX, vForward);
            mCollisionMatrix[0] = vX;
            mCollisionMatrix.m[0][3] = 0.0f;
            mCollisionMatrix[1] = vForward;
            mCollisionMatrix.m[1][3] = 0.0f;
            mCollisionMatrix[2] = vZ;
            mCollisionMatrix.m[2][3] = 0.0f;
            break;
        }
        case 2:
        {
            PblVector3 vForward = Normalize(mVelocity);
            PblVector3 vY = Normalize(CrossProduct(vForward, mCollisionMatrix.GetXAxis()));
            PblVector3 vX = CrossProduct(vY, vForward);
            mCollisionMatrix[0] = vX;
            mCollisionMatrix.m[0][3] = 0.0f;
            mCollisionMatrix[1] = vY;
            mCollisionMatrix.m[1][3] = 0.0f;
            mCollisionMatrix[2] = vForward;
            mCollisionMatrix.m[2][3] = 0.0f;
            break;
        }
        }
    }
    else if (mPhysicsType != PHYSICS_SIMPLE_RIGID)
    {
        float fDamping = -mAngularDamping;
        mAngularMomentum += mAngularMomentum * fDamping * fDeltaTime;
        mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);
    }

    PblQuaternion q = PblQuaternion(mCollisionMatrix);
    PblVector3 vSpin;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vSpin, (const D3DXVECTOR3*)&mOmega,
                            (const D3DXMATRIX*)&mCollisionMatrix.GetInverseFast());
    vSpin.x *= fDeltaTime;
    vSpin.y *= fDeltaTime;
    vSpin.z *= fDeltaTime;
    float fAngle = PblMath::Sqrt(vSpin.x * vSpin.x + vSpin.y * vSpin.y + vSpin.z * vSpin.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion(vSpin * fInv, fAngle);
    }

    mVelocity += mOneOverMass * vForce * fDeltaTime;
    if (mForceZeroYAxisVelocity)
    {
        const PblVector3& vUp = mMatrix.GetYAxis();
        float fDot = mVelocity.y * vUp.y + vUp.x * mVelocity.x + mVelocity.z * vUp.z;
        mVelocity -= fDot * vUp;
    }
    vPos += mVelocity * fDeltaTime;

    mCollisionMatrix = PblMatrix(q, vPos);
    mMatrix = mPrimitiveMatrix * mCollisionMatrix;
    SetCollisionCenter(mCollisionMatrix.GetTranslation());
    RecalculateAABB();

    PblMatrix rotation = mCollisionMatrix;
    rotation.m[3][0] = 0.0f;
    rotation.m[3][1] = 0.0f;
    rotation.m[3][2] = 0.0f;
    rotation.m[3][3] = 1.0f;
    const PblMatrix rotationInverse = rotation.GetInverseFast();
    mInverseWorldInertiaTensor = rotationInverse * mInverseBodyInertiaTensor * rotation;

    CollisionObject* pHit = NULL;
    PblVector3 vNormal;
    if (mNumTerrainCollisions > mMaxTerrainCollisions)
    {
        float fDist = CollisionManager::RayHit(vPos, PblVector3(0.0f, -1.0f, 0.0f), GetCollisionRadius() * 1.001f,
                                               pHit, &vNormal, NULL, 0, 0x90, false);
        if ((pHit && fDist < (mPhysicsType == PHYSICS_COCKPIT ? 0.3f : 0.75f)) || mPhysicsType == PHYSICS_STATIC ||
            mNumTerrainCollisions > mMaxTerrainCollisions + 5)
        {
            RemoveFromCollisionManager();
            EntityGeometryClass* pClass = mParentCluster->mParentClass;
            if (pClass && NetGame::IsEnabled() && !NetGame::IsClient())
            {
                NetGame::CreateEvent_CorrectChunkCluster(mParentCluster, pClass, mID, (unsigned __int64)-1);
            }
            FixPosition();
            if (!mParentCluster->mParentClass)
            {
                if (mMaxStaticLifetime >= 0.0f)
                {
                    mLifeTime = mMaxStaticLifetime - 5.0f;
                }
                mLifeTime = 3.0f;
            }
        }
    }

    if (RedTerrain::_bEnable)
    {
        float fHeight = RedTerrain::GetHoleHeight(vPos.x, vPos.z);
        if (fHeight > vPos.y + GetCollisionRadius())
        {
            pHit = NULL;
            CollisionManager::RayHit(vPos, PblVector3(0.0f, -1.0f, 0.0f), 256.0f, pHit, &vNormal, NULL, 0, 0x10,
                                     false);
            if (!pHit)
            {
                ClearAll();
            }
        }
    }
}

// FUNCTION: SWBF2 0x00456960
void Chunk::DoTerrainImpactEffect(const PblVector3& vPos)
{
    FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mTerrainImpactEffect);
    if (!pClass)
    {
        return;
    }
    FLEffectObject* pEffect = pClass->BuildEffect();
    if (pEffect)
    {
        pEffect->AttachEffectToVector(&vPos, false);
    }
}



// FUNCTION: SWBF2 0x004569a0
bool Chunk::DoSimpleRigidCollision(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor)
{
    const CollisionObjectType eType = pOther->GetCollisionType();
    if (mIsMoving)
    {
        if (mNumTerrainCollisions >= mMaxTerrainCollisions && eType == COLL_TERRAIN)
        {
            if (mNumTerrainCollisions == mMaxTerrainCollisions)
            {
                PblVector3 vImpact = pResult->mCollisionPosition;
                vImpact.y = RedTerrain::GetHoleHeight(vImpact.x, vImpact.z);
                DoTerrainImpactEffect(vImpact);
                ++mNumTerrainCollisions;
            }
            return true;
        }

        PblVector3& vVelocity = mVelocity;
        const PblVector3& vNormal = pResult->mSeparationNormal;
        const float fImpactSpeed = -(vVelocity.x * vNormal.x + vVelocity.y * vNormal.y +
                                     vVelocity.z * vNormal.z);
        if (fImpactSpeed > 0.0f)
        {
            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(),
                                               static_cast<FoleyFXCollidee*>(pOther), &GetPosition());
            PlayImpactEffects(&mMatrix.GetTranslation(), &vVelocity);

            if ((pOther->GetCollisionType() & (COLL_SOFT | COLL_RIGID)) && mMinDeathCollisionSpeed > 0.0f &&
                vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z >
                    mMinDeathCollisionSpeed * mMinDeathCollisionSpeed)
            {
                GameObject* pObject = pOther->GetGameObject();
                if (pObject)
                {
                    pObject->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
                    return true;
                }
            }

            const float fImpulse = (mCoefficientOfRestitution + 1.0f) * fImpactSpeed;
            vVelocity += vNormal * fImpulse;

            if (mSimpleFrictionConstant > 0.0f)
            {
                const PblVector3 vTangent = Normalize(CrossProduct(CrossProduct(vNormal, vVelocity), vNormal));
                const float fSlip = DotProduct(vVelocity, vTangent) * mSimpleFrictionConstant * mOneOverMass;
                vVelocity -= vTangent * fSlip;

                const PblVector3 vAxis = mCollisionMatrix.GetYAxis();
                PblVector3 vRadius = pResult->mCollisionPosition - mCollisionMatrix.GetTranslation();
                vRadius -= vAxis * DotProduct(vRadius, vAxis);
                const float fRadius = PblMath::Sqrt(vRadius.x * vRadius.x + vRadius.y * vRadius.y + vRadius.z * vRadius.z);
                if (fRadius > 0.0f)
                {
                    const float fOmega = PblMath::Sqrt(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y +
                                                       vVelocity.z * vVelocity.z) / fRadius;
                    mOmega = vAxis * fOmega;
                    if (DotProduct(vAxis, CrossProduct(vVelocity, PblVector3(0.0f, 1.0f, 0.0f))) > 0.0f)
                    {
                        mOmega = -mOmega;
                    }
                }
            }
        }

        if (eType == COLL_TERRAIN || vNormal.y > 0.8f)
        {
            ++mNumTerrainCollisions;
        }
    }
    return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x00456ef0
bool Chunk::DoFullRigidCollision(const CollisionResult* pResult, CollisionObject* pOther, Restrictor* pRestrictor)
{
    CollisionObjectType eType = pOther->GetCollisionType();
    if (eType == COLL_SOFT)
    {
        return true;
    }
    if ((eType == COLL_RIGID || eType == COLL_STATIC) && pOther->IsAChunk())
    {
        return true;
    }

    if (mIsMoving && !bCollisionOnUpdate)
    {
        bCollisionOnUpdate = true;
        PblVector3 vImpact = pResult->mCollisionPosition;
        vImpact.y = RedTerrain::GetHoleHeight(vImpact.x, vImpact.z);

        const PblVector3& vCenter = mCollisionMatrix.GetTranslation();
        PblVector3 vR(pResult->mCollisionPosition.x - vCenter.x, pResult->mCollisionPosition.y - vCenter.y,
                      pResult->mCollisionPosition.z - vCenter.z);
        PblVector3 vNormal = pResult->mSeparationNormal;

        const PblVector3 vVel = mVelocity + CrossProduct(mOmega, vR);
        float fDot = vNormal.y * vVel.y + vVel.x * vNormal.x + vNormal.z * vVel.z;
        const PblVector3 vFriction = -Normalize(PblVector3(vVel.x - vNormal.x * fDot, vVel.y - vNormal.y * fDot,
                                                           vVel.z - vNormal.z * fDot));

        if (fDot < 0.0f && (eType == COLL_TERRAIN || eType == COLL_STATIC || eType == COLL_RIGID))
        {
            if (mNumTerrainCollisions >= mMaxTerrainCollisions && eType != COLL_STATIC && eType != COLL_RIGID)
            {
                if (mNumTerrainCollisions == mMaxTerrainCollisions)
                {
                    if (eType == COLL_TERRAIN)
                    {
                        DoTerrainImpactEffect(vImpact);
                    }
                    mNumTerrainCollisions++;
                }
                return true;
            }

            float fNum = -((mCoefficientOfRestitution + 1.0f) * fDot);
            PblVector3 vA = TransformCoord(CrossProduct(vR, vNormal), mInverseWorldInertiaTensor);
            PblVector3 vAxR = CrossProduct(vA, vR);
            float fJ = fNum / (vAxR.x * vNormal.x + vNormal.y * vAxR.y + vNormal.z * vAxR.z + mOneOverMass);
            PblVector3 vJ(vNormal.x * fJ, vNormal.y * fJ, vNormal.z * fJ);
            mVelocity.z += vJ.z * mOneOverMass;
            mVelocity.x += vJ.x * mOneOverMass;
            mVelocity.y += vJ.y * mOneOverMass;
            PblVector3 vL = CrossProduct(vR, vJ);
            mAngularMomentum.y += vL.y;
            mAngularMomentum.x += vL.x;
            mAngularMomentum.z += vL.z;
            mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);

            float fSlip = DotProduct(mVelocity + CrossProduct(mOmega, vR), vFriction);
            if (fSlip < 0.0f && (eType == COLL_TERRAIN || eType == COLL_STATIC || eType == COLL_RIGID))
            {
                if (mNumTerrainCollisions >= mMaxTerrainCollisions && eType != COLL_STATIC && eType != COLL_RIGID)
                {
                    if (mNumTerrainCollisions == mMaxTerrainCollisions)
                    {
                        if (eType == COLL_TERRAIN)
                        {
                            DoTerrainImpactEffect(vImpact);
                        }
                        mNumTerrainCollisions++;
                    }
                    return true;
                }

                float fFNum = -(mCoefficientOfFriction * fSlip);
                PblVector3 vB = TransformCoord(CrossProduct(vR, vFriction), mInverseWorldInertiaTensor);
                PblVector3 vBxR = CrossProduct(vB, vR);
                float fK = fFNum / (vBxR.x * vFriction.x + vFriction.y * vBxR.y + vFriction.z * vBxR.z + mOneOverMass);
                PblVector3 vF(vFriction.x * fK, vFriction.y * fK, vFriction.z * fK);
                PblVector3 vLF = CrossProduct(vR, vF);
                mAngularMomentum.x += vLF.x;
                mAngularMomentum.y += vLF.y;
                mAngularMomentum.z += vLF.z;
                mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);
            }
        }
    }
    mNumTerrainCollisions++;
    return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x00457570
bool Chunk::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther, Restrictor* pRestrictor)
{
    switch (mPhysicsType)
    {
    case PHYSICS_FULL_RIGID:
        return DoFullRigidCollision(pResult, pOther, pRestrictor);
    case PHYSICS_SIMPLE_RIGID:
    case PHYSICS_COCKPIT:
        return DoSimpleRigidCollision(pResult, pOther, pRestrictor);
    case PHYSICS_STATIC:
        return false;
    default:
        return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    }
}

extern bool netFrameLock;

// FUNCTION: SWBF2 0x004575c0
void Chunk::Render(unsigned char ucAlpha, float fDist, unsigned int uiFlags)
{
    float fMaxLifeTime =
        (mPhysicsType == PHYSICS_STATIC || mPhysicsType == PHYSICS_COCKPIT) ? STATIC_LIFETIME : DYNAMIC_LIFETIME;
    if (mMaxStaticLifetime >= 0.0f)
    {
        fMaxLifeTime = mMaxStaticLifetime;
    }
    if (mMaxTerrainCollisions == 255)
    {
        fMaxLifeTime = 30.0f;
    }
    if (mLifeTime > fMaxLifeTime - 2.0f)
    {
        fDist *= (fMaxLifeTime - mLifeTime) * 0.5f;
    }

    RedColor color;
    color.b = 0xff;
    color.g = 0xff;
    color.r = 0xff;
    color.a = (unsigned char)(fDist * 255.0f);

    RedPose pose;
    mModel->GetRedModel(0)->SetupPose(pose);
    PblMatrix mRender = mMatrix;

    if (mIsMoving && NetGame::IsEnabled() && !netFrameLock)
    {
        mJustRendered = true;
        float fDt = (1.0f - NetGame::GetTurnRatio()) * mTotalUpdateDt;

        PblQuaternion q = PblQuaternion(mCollisionMatrix);
        PblVector3 vBack(-mOmega.x, -mOmega.y, -mOmega.z);
        PblVector3 vSpin;
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vSpin, (const D3DXVECTOR3*)&vBack,
                                (const D3DXMATRIX*)&mCollisionMatrix.GetInverseFast());
        const PblVector3 vRotation = vSpin * fDt;
        PblVector3 vPos = mCollisionMatrix.GetTranslation();
        float fAngle = PblMath::Sqrt(vRotation.x * vRotation.x + vRotation.y * vRotation.y + vRotation.z * vRotation.z);
        if (fAngle > EPSILON)
        {
            float fInv = 1.0f / fAngle;
            q = q * PblQuaternion(vRotation * fInv, fAngle);
        }
        vPos -= mVelocity * fDt;
        mRender = mPrimitiveMatrix * PblMatrix(q, vPos);
    }

    mModel->GetRedModel(0)->Render(&mRender, &pose, color, uiFlags | 8, NULL);
}

extern NoLodData noloddata;

// FUNCTION: SWBF2 0x00457a30
ChunkCluster::ChunkCluster()
    : RedSceneObject(&noloddata)
{
    SetRenderRadius(1.0f);
    SetRenderPos(PblVector3(0.0f, 0.0f, 0.0f));
}

// SYNTHETIC: SWBF2 0x00457ad0
// ChunkCluster::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00457b00
ChunkCluster::~ChunkCluster()
{
    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            mChunks[i].ClearAll();
        }
    }
}

// FUNCTION: SWBF2 0x00457b90
void ChunkCluster::RecalculateCenterAndRadius()
{
    PblVector3 vCenter(0.0f, 0.0f, 0.0f);
    float fRadius = 0.0f;
    int iCount = 0;
    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            const PblVector3& vPos = mChunks[i].GetCollisionCenter();
            vCenter.x += vPos.x;
            vCenter.y += vPos.y;
            vCenter.z += vPos.z;
            iCount++;
        }
    }
    if (iCount <= 0)
    {
        return;
    }

    float fInv = 1.0f / iCount;
    vCenter = fInv * vCenter;

    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            const PblVector3& vPos = mChunks[i].GetCollisionCenter();
            float fX = vPos.x - vCenter.x;
            float fY = vPos.y - vCenter.y;
            float fZ = vPos.z - vCenter.z;
            float fReach = mChunks[i].GetCollisionRadius() + sqrtf(fY * fY + fX * fX + fZ * fZ) + 1.0f;
            if (fReach > fRadius)
            {
                fRadius = fReach;
            }
        }
    }
    SetRenderRadius(fRadius);
    SetRenderPos(vCenter);
}

// FUNCTION: SWBF2 0x00457ff0
void ChunkCluster::AddChunk(ChunkDescriptor& desc)
{
    for (int i = 0; i < 10; i++)
    {
        if (!mChunks[i].IsValid())
        {
            mChunks[i].SetData(desc);
            RecalculateCenterAndRadius();
            return;
        }
    }
}

// FUNCTION: SWBF2 0x00458040
bool ChunkCluster::Update(float fDeltaTime)
{
    bool bMoved = false;
    for (int i = 0; i < 10; i++)
    {
        const bool bChunkMoving = mChunks[i].IsMoving();
        mChunks[i].Update(fDeltaTime);
        if (bChunkMoving)
        {
            bMoved = true;
        }
    }
    if (bMoved)
    {
        RecalculateCenterAndRadius();
    }
    if (!IsValid())
    {
        mID = -1;
    }
    return true;
}

// FUNCTION: SWBF2 0x004580c0
void ChunkCluster::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const RedCamera* pCamera = RedRenderer::GetCamera();
    const PblVector3& vCamDir = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    PblVector3 vNear(vCamDir.x * fNear, vCamDir.y * fNear, vCamDir.z * fNear);
    const PblVector3& vCamPos = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    float fX = _Sphere.x - vCamPos.x + vNear.x;
    float fY = _Sphere.y - vCamPos.y + vNear.y;
    float fZ = _Sphere.z - vCamPos.z + vNear.z;

    if (fY * fY + fX * fX + fZ * fZ < 800.0f && RedStencilShadow::GetShadowEnable())
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
        float fLength;
        if (pLight)
        {
            float fMinLength = _Sphere.w * -2.0f / pLight->m_vDirection.y;
            uiFlags |= 0x10000;
            PblVector3 vStart = _Sphere;
            vStart.y = _Sphere.w + vStart.y;
            const GameObject* pIgnore = NULL;
            CollisionObject* pHit;
            fLength = CollisionManager::RayHit(vStart, pLight->m_vDirection, 50.0f, pHit, NULL, &pIgnore, 1, 0x90, false) * 50.0f;
            if (fMinLength > fLength)
            {
                fLength = fMinLength;
            }
        }
        else
        {
            fLength = _Sphere.w * 2.0f;
        }
        RedRenderer::SetStencilShadowLength(fLength);

        const PblMatrix& mView = pCamera->GetMatrix();
        const PblVector3& vCenter = _Sphere;
        float fDist2 = SquaredLength(mView.GetTranslation() - mView.GetZAxis() * fNear - vCenter);
        float fReach = _Sphere.w + fLength + fWidth * 1.33f;
        RedRenderer::SetStencilShadowZFail(fReach * fReach > fDist2);
    }

    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            mChunks[i].Render(ucViewport, fDist, uiFlags);
        }
    }
}

// FUNCTION: SWBF2 0x00458370
bool ChunkCluster::IsValid()
{
    bool bValid = false;
    for (int i = 0; i < 10; i++)
    {
        bValid |= mChunks[i].IsValid();
    }
    return bValid;
}

// FUNCTION: SWBF2 0x004583b0
bool ChunkCluster::HasStaticChunks()
{
    bool bStatic = false;
    for (int i = 0; i < 10; ++i)
    {
        if (mChunks[i].mIsValid)
        {
            bStatic |= mChunks[i].mPhysicsType == PHYSICS_STATIC;
        }
    }
    return bStatic;
}

// FUNCTION: SWBF2 0x00458340
void ChunkCluster::ClearChunks()
{
    RemoveFromScene();
    for (int i = 0; i < 10; ++i)
    {
        mChunks[i].ClearAll();
    }
}

// GLOBAL: SWBF2 0x01e30348
ChunkClusterManager* ChunkClusterManager::sInstance;
// GLOBAL: SWBF2 0x01e3034c
ChunkClusterManager* ChunkClusterManager::sClientInstance;
// GLOBAL: SWBF2 0x01e30350
ChunkClusterManager* ChunkClusterManager::sHostInstance;

// FUNCTION: SWBF2 0x004586a0
bool ChunkClusterManager::CreateClient(bool bDeactivate)
{
    if (!NetGame::IsOneWorld())
    {
        sClientInstance = new ChunkClusterManager;
        if (bDeactivate)
        {
            sClientInstance->DeactivateThread();
        }
    }
    else
    {
        sClientInstance = 0;
        sHostInstance->DeactivateThread();
        sHostInstance->ActivateThread(-10, PER_LOCAL_TURN);
    }
    return true;
}

// FUNCTION: SWBF2 0x00458750
bool ChunkClusterManager::DestroyClient()
{
    if (sClientInstance)
    {
        delete sClientInstance;
        sClientInstance = 0;
    }
    return true;
}

// FUNCTION: SWBF2 0x00458770
void ChunkClusterManager::EnterClient()
{
    if (!NetGame::IsOneWorld())
    {
        sInstance = sClientInstance;
    }
}

// FUNCTION: SWBF2 0x00458790
void ChunkClusterManager::LeaveClient()
{
    if (!NetGame::IsOneWorld())
    {
        sInstance = sHostInstance;
    }
}

// FUNCTION: SWBF2 0x00458490
ChunkClusterManager::ChunkClusterManager()
    : mOldestChunkCluster(0)
{
    ActivateThread(-10, PER_LOCAL_TURN);
    for (int i = 0; i < 8; i++)
    {
        mChunkClusters[i].mID = -1;
        for (unsigned char j = 0; j < 10; j++)
        {
            mChunkClusters[i].mChunks[j].mParentCluster = &mChunkClusters[i];
            mChunkClusters[i].mChunks[j].mID = j;
        }
    }
}

// SYNTHETIC: SWBF2 0x00458540
// ChunkClusterManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00458570
ChunkClusterManager::~ChunkClusterManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x004585e0
bool ChunkClusterManager::Create()
{
    if (sInstance)
    {
        return false;
    }
    sInstance = sHostInstance = new ChunkClusterManager;
    return true;
}

// FUNCTION: SWBF2 0x00458660
bool ChunkClusterManager::Destroy()
{
    if (sInstance)
    {
        delete sHostInstance;
        sHostInstance = 0;
        sInstance = 0;
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00458800
ChunkCluster* ChunkClusterManager::GetFreeCluster(bool bArg)
{
    for (int i = 0; i < 8; i++)
    {
        if (!sInstance->mChunkClusters[i].IsValid())
        {
            ChunkCluster* pCluster = &sInstance->mChunkClusters[i];
            if (bArg)
            {
                pCluster->mID = i;
            }
            else
            {
                pCluster->mID = -1;
            }
            pCluster->Activate();
            return &sInstance->mChunkClusters[i];
        }
    }

    int iOldest = sInstance->mOldestChunkCluster;
    int i;
    for (i = iOldest; i < 8; i++)
    {
        if (!sInstance->mChunkClusters[i].HasStaticChunks())
        {
            break;
        }
    }
    if (i == 8)
    {
        for (i = 0; i < iOldest; i++)
        {
            if (!sInstance->mChunkClusters[i].HasStaticChunks())
            {
                break;
            }
        }
    }
    sInstance->mChunkClusters[i].ClearChunks();
    ChunkCluster* pCluster = &sInstance->mChunkClusters[i];
    if (++sInstance->mOldestChunkCluster >= 8)
    {
        sInstance->mOldestChunkCluster = 0;
    }
    if (bArg)
    {
        pCluster->mID = i;
    }
    else
    {
        pCluster->mID = -1;
    }
    pCluster->Activate();
    return pCluster;
}

// FUNCTION: SWBF2 0x004587b0
ChunkCluster* ChunkClusterManager::GetClusterByID(unsigned char ucID, bool bArg)
{
    if (ucID < 8)
    {
        for (int i = 0; i < 8; ++i)
        {
            if (sInstance->mChunkClusters[i].mID == ucID)
            {
                return &sInstance->mChunkClusters[i];
            }
        }
        if (bArg)
        {
            ChunkCluster* pCluster = GetFreeCluster(true);
            if (pCluster)
            {
                pCluster->mID = ucID;
                return pCluster;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00458920
bool ChunkClusterManager::Update(float fDeltaTime)
{
    for (int i = 0; i < 8; i++)
    {
        PblValues::Record((char)i, PVT_MOVES_HIGH);
        mChunkClusters[i].Update(fDeltaTime);
    }
    return true;
}
