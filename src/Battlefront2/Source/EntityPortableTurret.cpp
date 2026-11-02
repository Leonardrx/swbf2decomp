#include "PebbleFL/Common/PblMath.h"
#include <d3dx9math.h>
#include "EntityPortableTurret.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include "CollisionBody.h"
#include "CollisionManager.h"
#include "CollisionMesh.h"
#include "CollisionModel.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblVector.h"

// FUNCTION: SWBF2 0x004d89d0
EntityPortableTurretClass::EntityPortableTurretClass(unsigned int uHash)
    : EntityBuildingArmedDynamicClass(uHash)
{
    mLifeTime = FLT_MAX;
    mGravity = 1.0f;
    mRebound = 0.3f;
    mFriction = 0.5f;
}

// SYNTHETIC: SWBF2 0x00402cb0
// `dynamic initializer for 'EntityPortableTurretClass::rttiHashEntityPortableTurretClass''
// EntityPortableTurretClass::rttiHashEntityPortableTurretClass

// GLOBAL: SWBF2 0x01ebbf94
// EntityPortableTurretClass::rttiHashEntityPortableTurretClass
PblHash EntityPortableTurretClass::rttiHashEntityPortableTurretClass("EntityPortableTurretClass");
// GLOBAL: SWBF2 0x00789464
// EntityPortableTurretClass::rttiNameEntityPortableTurretClass
const char EntityPortableTurretClass::rttiNameEntityPortableTurretClass[] = "EntityPortableTurretClass";

// SYNTHETIC: SWBF2 0x00402ca0
// `dynamic initializer for 'EntityPortableTurret::rttiHashEntityPortableTurret''
// EntityPortableTurret::rttiHashEntityPortableTurret

// GLOBAL: SWBF2 0x01ebbfe8
// EntityPortableTurret::rttiHashEntityPortableTurret
PblHash EntityPortableTurret::rttiHashEntityPortableTurret("EntityPortableTurret");
// GLOBAL: SWBF2 0x00789448
// EntityPortableTurret::rttiNameEntityPortableTurret
const char EntityPortableTurret::rttiNameEntityPortableTurret[] = "EntityPortableTurret";

// SYNTHETIC: SWBF2 0x00402c80
// `dynamic initializer for 'EntityPortableTurret::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f90
// `dynamic atexit destructor for 'EntityPortableTurret::sMemoryPool''

// GLOBAL: SWBF2 0x01ebbff0
// EntityPortableTurret::sMemoryPool
MemoryPool EntityPortableTurret::sMemoryPool("EntityPortableTurret", sizeof(EntityPortableTurret));


extern PblVector3 gVectorZero;

extern bool netFrameLock;
extern bool netIsLocalTurn;
extern bool netIsLateTurn;

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);



// FUNCTION: SWBF2 0x004d7cf0
EntityPortableTurret::EntityPortableTurret(EntityPortableTurretClass* pClass, const EntityDesc& desc)
    : EntityBuildingArmedDynamic(pClass, desc)
{
    mClass = pClass;
    mLifeTimer = 0.0f;
    mVelocity = gVectorZero;
    mOwnerChar = 0;
    mStationary = false;
    mBecomeStationaryNow = false;

    PblVector3& vRight = mMatrix[0];
    PblVector3& vUp = mMatrix[1];
    PblVector3& vFront = mMatrix[2];
    vUp = gVectorUnitY;
    mMatrix.m[1][3] = 0.0f;
    vRight = Normalize(CrossProduct(vUp, vFront));
    mMatrix.m[0][3] = 0.0f;
    vFront = Normalize(CrossProduct(vRight, vUp));
    mMatrix.m[2][3] = 0.0f;

    PblBox box(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    bool bHaveBody = false;
    for (int i = 0; i < mCollisionModel->GetNumBodies(); ++i)
    {
        CollisionBody* pBody = mCollisionModel->GetBody(i);
        if (!pBody)
        {
            continue;
        }
        bHaveBody = true;
        PblBox bodyBox;
        if (pBody->bIsMesh)
        {
            bodyBox = static_cast<CollisionBodyMesh*>(pBody)->mMesh->GetWorldspaceAABB(
                pBody->GetWorldMatrix(mMatrix, &mPose));
        }
        else
        {
            bodyBox = static_cast<CollisionBodyPrimitive*>(pBody)->mPrimitive.GetWorldspaceAABB(
                pBody->GetWorldMatrix(mMatrix, &mPose));
        }
        box._vMin.x = bodyBox._vMin.x > box._vMin.x ? box._vMin.x : bodyBox._vMin.x;
        box._vMin.y = bodyBox._vMin.y > box._vMin.y ? box._vMin.y : bodyBox._vMin.y;
        box._vMin.z = bodyBox._vMin.z > box._vMin.z ? box._vMin.z : bodyBox._vMin.z;
        box._vMax.x = box._vMax.x > bodyBox._vMax.x ? box._vMax.x : bodyBox._vMax.x;
        box._vMax.y = box._vMax.y > bodyBox._vMax.y ? box._vMax.y : bodyBox._vMax.y;
        box._vMax.z = box._vMax.z > bodyBox._vMax.z ? box._vMax.z : bodyBox._vMax.z;
    }
    if (bHaveBody)
    {
        mAABB = box;
    }
}

// SYNTHETIC: SWBF2 0x004d7c50
// EntityPortableTurret::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004d8d10
// ??_EEntityPortableTurret@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004d8d1b
// ??_EEntityPortableTurret@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004d8d23
// ??_EEntityPortableTurret@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004d8d39
// ??_EEntityPortableTurret@@WJE@AEPAXI@Z

// SYNTHETIC: SWBF2 0x004d8d2e
// ??_EEntityPortableTurret@@WCGA@AEPAXI@Z

// FUNCTION: SWBF2 0x004d8230
bool EntityPortableTurret::Update(float fDeltaTime)
{
    if (mClass->mLifeTime > 0.0f)
    {
        mLifeTimer += fDeltaTime;
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        if (!IsAlive() && !mClass->mDestroyedModel && !mClass->mUnbuiltModel)
        {
            Kill();
            return false;
        }
        if (mLifeTimer > mClass->mLifeTime || mMatrix.m[3][1] < -500.0f)
        {
            Kill();
            return false;
        }
    }

    if (mBecomeStationaryNow)
    {
        mStationary = true;
        mBecomeStationaryNow = false;
        RemoveFromCollisionManager();
        this->AddRigidToCollisionManager();
    }

    if (!mStationary)
    {
        float fHalfDrop = mClass->mGravity * -9.8f * 0.5f * fDeltaTime;
        mVelocity.y = fHalfDrop + mVelocity.y;
        PblVector3 vStep = mVelocity * fDeltaTime;
        const PblVector3& vPos = GetPosition();
        SetPosition(PblVector3(vPos.x + vStep.x, vPos.y + vStep.y, vPos.z + vStep.z));
        mAABB._vMax = mAABB._vMax + mVelocity * fDeltaTime;
        mAABB._vMin = mAABB._vMin + mVelocity * fDeltaTime;
        mVelocity.y = fHalfDrop + mVelocity.y;
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mMatrix.m[3][1] < -500.0f)
        {
            Kill();
            return false;
        }
    }

    if (netFrameLock || !NetGame::IsEnabled())
    {
        return true;
    }
    if (!(NetGame::IsClient() && netIsLocalTurn))
    {
        if (!NetGame::IsHost() || !NetGame::IsOneWorld() || netIsLateTurn)
        {
            return true;
        }
        if (mNetUniqueId >= 0)
        {
            mNetUniqueId = NetGame::GetHostWorldUniqueId(mNetUniqueId, this);
        }
    }
    gameObjectInterpolator.SetNext(mNetUniqueId, mStationary ? 0 : &mMatrix);
    return true;
}

// FUNCTION: SWBF2 0x004d85a0
bool EntityPortableTurret::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                             Restrictor* pRestrictor)
{
    if (mStationary)
    {
        return false;
    }

    PblVector3 vOldPos = GetPosition();

    if (pOther->GetCollisionType() == COLL_LEAFPATCH)
    {
        mClass->mCollisionFoliageSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
    }

    if (pOther->GetCollisionType() != COLL_SOFT && pOther->GetCollisionType() != COLL_RIGID)
    {
        const PblVector3& vNormal = pResult->mSeparationNormal;
        float fDot = DotProduct(mVelocity, vNormal);
        if (fDot < 0.0f)
        {
            if (pOther->GetCollisionType() == COLL_WATER)
            {
                mClass->mCollisionWaterSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
            }
            else
            {
                mClass->mCollisionSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
            }
            PblVector3 vUnit;
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vUnit), reinterpret_cast<const D3DXVECTOR3*>(&vNormal));
            if (vUnit.y > 0.7f)
            {
                mMatrix = NormalMatrix(mMatrix.GetZAxis(), vNormal, mMatrix.GetTranslation());
                mVelocity = gVectorZero;
                mBecomeStationaryNow = true;
            }
            else
            {
                float fRebound = mClass->mRebound;
                float fFriction = mClass->mFriction * fDot;
                mVelocity -= vNormal * fDot;
                float fScale = -(fFriction * (fRebound + 1.0f) / PblMath::Sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                                           mVelocity.z * mVelocity.z));
                if (!(1.0f > fScale))
                {
                    fScale = 1.0f;
                }
                mVelocity -= mVelocity * fScale;
                float fBounce = fRebound * fDot;
                mVelocity -= vNormal * fBounce;
            }
        }
    }

    bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    PblVector3 vDelta = GetPosition() - vOldPos;
    mAABB._vMax = mAABB._vMax + vDelta;
    mAABB._vMin = mAABB._vMin + vDelta;
    return bResult;
}





// FUNCTION: SWBF2 0x004d7c20
const PblVector3& EntityPortableTurret::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x004d7c30
void EntityPortableTurret::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x004d8220
void EntityPortableTurret::RecalculateAABB()
{
}







// FUNCTION: SWBF2 0x004d8a70
EntityPortableTurretClass::EntityPortableTurretClass(EntityPortableTurretClass* pBase, unsigned int uHash)
    : EntityBuildingArmedDynamicClass(pBase, uHash)
{
    mLifeTime = pBase->mLifeTime;
    mGravity = pBase->mGravity;
    mRebound = pBase->mRebound;
    mFriction = pBase->mFriction;
    mCollisionSound = pBase->mCollisionSound;
    mCollisionFoliageSound = pBase->mCollisionFoliageSound;
    mCollisionWaterSound = pBase->mCollisionWaterSound;
}

// FUNCTION: SWBF2 0x004d8b50
EntityPortableTurretClass::~EntityPortableTurretClass()
{
}

// SYNTHETIC: SWBF2 0x004d8a40
// EntityPortableTurretClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004d8b60
EntityClass* EntityPortableTurretClass::Derive(unsigned int uHash)
{
    return new EntityPortableTurretClass(this, uHash);
}

// FUNCTION: SWBF2 0x004d8bd0
Entity* EntityPortableTurretClass::Build(const EntityDesc& desc)
{
    if (EntityPortableTurret::sMemoryPool.mUsed >= EntityPortableTurret::sMemoryPool.mCount)
    {
        return 0;
    }

    EntityPortableTurret* pTurret = new EntityPortableTurret(this, desc);
    if (pTurret)
    {
        pTurret->AddSoftToCollisionManager();
        pTurret->ActivatePhysics();
    }
    return pTurret;
}

// FUNCTION: SWBF2 0x004d8c70
void EntityPortableTurretClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x5212ABEEu:
        sscanf(pszValue, "%f", &mLifeTime);
        break;
    case 0xD209FF45u:
        sscanf(pszValue, "%f", &mGravity);
        break;
    case 0x678DE338u:
        sscanf(pszValue, "%f", &mRebound);
        break;
    case 0xA51BE2BBu:
        sscanf(pszValue, "%f", &mFriction);
        break;
    case 0xC28F0C96u:
    case 0xEDC6251Au:
        mCollisionSound.SetID(pszValue, false, false);
        break;
    case 0x2297E47Du:
        mCollisionWaterSound.SetID(pszValue, false, false);
        break;
    default:
        EntityBuildingArmedClass::SetProperty(uProperty, pszValue);
        break;
    }
}
