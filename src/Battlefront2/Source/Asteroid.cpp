#include <stdio.h>
#include "PebbleFL/Common/PblVector.h"

#include "Asteroid.h"
#include "CollisionBody.h"
#include "CollisionMesh.h"
#include "CollisionModel.h"
#include "CollisionObject.h"
#include "CollisionPrimitive.h"
#include "Explosion.h"
#include "GameModel.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblRegion.h"
#include "PebbleFL/Common/PblHermite.h"
#include "RedEngineFL/Application/RedPath.h"
#include "CameraManager.h"
#include <d3dx9math.h>
#include <math.h>

class PblMatrix;
extern PblMatrix gMatrixIdentity;



// GLOBAL: SWBF2 0x01e2fddc
unsigned int sSoftCollisionMask;
// GLOBAL: SWBF2 0x01e2fdd8
unsigned int sOrdnanceCollisionMask;

// SYNTHETIC: SWBF2 0x004022d0
// `dynamic initializer for 'AsteroidClass::rttiHashAsteroidClass''
// AsteroidClass::rttiHashAsteroidClass

// GLOBAL: SWBF2 0x01eb024c
// AsteroidClass::rttiHashAsteroidClass
PblHash AsteroidClass::rttiHashAsteroidClass("AsteroidClass");
// GLOBAL: SWBF2 0x00788840
// AsteroidClass::rttiNameAsteroidClass
const char AsteroidClass::rttiNameAsteroidClass[] = "AsteroidClass";

// FUNCTION: SWBF2 0x004453d0
AsteroidClass::AsteroidClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    mLODClass = 1;

    mMinSpeed = 10.0f;
    mMaxSpeed = 30.0f;
    mMinOmega = 5.0f;
    mMaxOmega = 10.0f;
    mCollidable = 1;
    mNumChildren = 0;
    mExplosionBreak = 0;
    mExplosionDestruct = 0;
}

// SYNTHETIC: SWBF2 0x00445450
// AsteroidClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00445560
AsteroidClass::~AsteroidClass()
{
}

// FUNCTION: SWBF2 0x00445480
AsteroidClass::AsteroidClass(AsteroidClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash)
{
    mMinSpeed = pBase->mMinSpeed;
    mMaxSpeed = pBase->mMaxSpeed;
    mMinOmega = pBase->mMinOmega;
    mMaxOmega = pBase->mMaxOmega;
    mCollidable = pBase->mCollidable;
    mNumChildren = pBase->mNumChildren;
    memcpy(mChildren, pBase->mChildren, sizeof(mChildren));
    mExplosionBreak = pBase->mExplosionBreak;
    mExplosionDestruct = pBase->mExplosionDestruct;
    mLODClass = 1;
}







// FUNCTION: SWBF2 0x004441e0
const GameModel* EntityGeometryClass::GetModel()
{
    return mModel;
}

// FUNCTION: SWBF2 0x00444430
int GameObjectClass::GetSoldierType() const
{
    return 5;
}

// FUNCTION: SWBF2 0x00444440
int GameObjectClass::GetVehicleType() const
{
    return 9;
}

// FUNCTION: SWBF2 0x00444450
const ControllableClass* GameObjectClass::GetControllableClass() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00444460
int GameObjectClass::GetWeaponCount() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00444470
const WeaponClass* GameObjectClass::GetWeaponClass(int iIndex) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00444480
float GameObjectClass::GetCollisionInflict() const
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x00444490
const DamageEffectClass* GameObjectClass::GetDamageEffectClass(int iIndex) const
{
    return mDamageEffectClass[iIndex];
}

// FUNCTION: SWBF2 0x004444b0
float GameObjectClass::GetTimeRequiredToEject() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004444c0
float GameObjectClass::GetReboardTime() const
{
    return 0.0f;
}



// FUNCTION: SWBF2 0x00445570
void AsteroidClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x47c86b4a:
    {
        EntityGeometryClass::SetProperty(uProperty, pszValue);
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
            const PblVector3 vCenter = (box._vMax + box._vMin) * 0.5f;
            const PblVector3 vHalf = (box._vMax - box._vMin) * 0.5f;
            CollisionPrimitive primitive;
            PblMatrix matrix = gMatrixIdentity;
            primitive.SetAACylinder((vHalf.x + vHalf.z) * 0.5f, vHalf.y * 2.0f);
            matrix[3] = vCenter;
            matrix.m[3][3] = 1.0f;
            mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));
        }
        if (pMesh)
        {
            mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
        }
        mCollisionModel.mIsFlyerFlag = false;
        break;
    }

    case 0xfbe2eb62:
        sscanf(pszValue, "%f", &mMinSpeed);
        break;
    case 0x86df0364:
        sscanf(pszValue, "%f", &mMaxSpeed);
        break;
    case 0x8ec2719e:
        sscanf(pszValue, "%f", &mMinOmega);
        break;
    case 0x38970514:
        sscanf(pszValue, "%f", &mMaxOmega);
        break;
    case 0x74e9dbae:
        sscanf(pszValue, "%d", &mCollidable);
        break;

    case 0xa20a3388:
        if (mNumChildren < 10)
        {
            char acClass[256];
            char acHardPoint[256];
            sscanf(pszValue, "%s %s", acClass, acHardPoint);
            mChildren[mNumChildren].mClassHash = PblHash(acClass);
            mChildren[mNumChildren].mHardPointHash = PblTEMPHash(acHardPoint);
            ++mNumChildren;
        }
        break;

    case 0x0e15a87f:
        mExplosionBreak = ExplosionClass::Find(PblHash(pszValue));
        break;
    case 0x02bb1fe0:
    case 0x8a323f32:
    case 0x9b30e763:
        mExplosionDestruct = ExplosionClass::Find(PblHash(pszValue));
        break;

    case 0x5dfdc07f:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sSoftCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
        }
        break;
    }
    case 0xfb2bdf07:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sOrdnanceCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
        }
        break;
    }

    default:
        GameObjectClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// SYNTHETIC: SWBF2 0x00445af0
// CollisionPrimitive::CollisionPrimitive

// FUNCTION: SWBF2 0x00445b30
EntityClass* AsteroidClass::Derive(unsigned int uHash)
{
    return new AsteroidClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x004022a0
// `dynamic initializer for 'Asteroid::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766d80
// `dynamic atexit destructor for 'Asteroid::sMemoryPool''

// GLOBAL: SWBF2 0x01eb0500
// Asteroid::sMemoryPool
MemoryPool Asteroid::sMemoryPool("Asteroid", sizeof(Asteroid));

// SYNTHETIC: SWBF2 0x004022c0
// `dynamic initializer for 'Asteroid::rttiHashAsteroid''
// Asteroid::rttiHashAsteroid

// GLOBAL: SWBF2 0x01eb0248
// Asteroid::rttiHashAsteroid
PblHash Asteroid::rttiHashAsteroid("Asteroid");
const char Asteroid::rttiNameAsteroid[] = "Asteroid";

class CollisionManager
{
public:
    static bool AddAsteroidBody(CollisionObject* pObject);
};

// FUNCTION: SWBF2 0x00445ba0
Entity* AsteroidClass::Build(const EntityDesc& desc)
{
    if (NetGame::IsEnabled() && NetGame::IsOneWorld() && !NetGame::IsHost() && !NetGame::IsClient())
    {
        return 0;
    }
    if (Asteroid::sMemoryPool.mUsed >= Asteroid::sMemoryPool.mCount)
    {
        return 0;
    }

    Asteroid* pAsteroid = new Asteroid(this, desc);
    if (pAsteroid)
    {
        if (mCollidable)
        {
            pAsteroid->AddAsteroidToCollisionManager();
        }

        pAsteroid->ActivatePhysics();
    }
    return pAsteroid;
}

struct LengthEntryT
{
    float length;
    int segmentIndex;
};

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// FUNCTION: SWBF2 0x00445c90
void AsteroidClass::FillRegion(const char* pcRegion, int iCount, PblVector3 vMin, PblVector3 vMax)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    PblRegion* pRegion = PblRegion::Find(PblHash(pcRegion));
    if (!pRegion)
    {
        return;
    }
    for (int i = 0; i < iCount; ++i)
    {
        PblVector3 vPosition;
        switch (pRegion->mType)
        {
        case PblRegion::type_sphere:
        {
            PblVector3 vUnit = PblRandom::GetSim().GetVector3() * 2.0f - PblVector3(1.0f, 1.0f, 1.0f);
            PblVector3 vDir;
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vUnit));
            const float fMaxRadius = pRegion->mSize.w;
            float fRadius = fMaxRadius * PblRandom::GetSim().GetFloat();
            vPosition = vDir * fRadius;
            vPosition += pRegion->mMatrix.GetTranslation();
            break;
        }
        case PblRegion::type_box:
        case PblRegion::type_box_aligned:
        {
            PblVector3 vUnit = PblRandom::GetSim().GetVector3() * 2.0f - PblVector3(1.0f, 1.0f, 1.0f);
            PblVector3 vLocal = vUnit * pRegion->mSize;
            vPosition = pRegion->mMatrix.ApplyTransform(vLocal);
            break;
        }
        case PblRegion::type_cylinder:
        case PblRegion::type_cylinder_xaligned:
        case PblRegion::type_cylinder_yaligned:
        case PblRegion::type_cylinder_zaligned:
        {
            PblVector3 vUnit = PblRandom::GetSim().GetVector3() * 2.0f - PblVector3(1.0f, 1.0f, 1.0f);
            float fLength = sqrtf(vUnit.x * vUnit.x + vUnit.z * vUnit.z);
            const float fMaxRadius = pRegion->mSize.x;
            float fRadius = fMaxRadius * PblRandom::GetSim().GetFloat();
            PblVector3 vLocal(vUnit.x / fLength * fRadius, pRegion->mSize.y * vUnit.y, vUnit.z / fLength * fRadius);
            vPosition = pRegion->mMatrix.ApplyTransform(vLocal);
            break;
        }
        default:
            break;
        }

        PblVector3 vDir;
        vDir.x = (vMax.x - vMin.x) * PblRandom::GetSim().GetFloat() + vMin.x;
        vDir.y = (vMax.y - vMin.y) * PblRandom::GetSim().GetFloat() + vMin.y;
        vDir.z = (vMax.z - vMin.z) * PblRandom::GetSim().GetFloat() + vMin.z;
        const PblVector3& vUp =
            (float)fabs((double)(vDir.x * gVectorUnitY.x + vDir.y * gVectorUnitY.y + vDir.z * gVectorUnitY.z)) < 0.99f
                ? gVectorUnitZ
                : gVectorUnitY;
        EntityDesc desc;
        desc.mMatrix = gMatrixIdentity;
        desc.mPlayerId = 0;
        desc.mMatrix = DirectionMatrix(vDir, vUp, vPosition);
        Build(desc);
    }
}

// FUNCTION: SWBF2 0x00446250
void GetPoint(float s, LengthEntryT* parameterizedArcLength, int numEntries,
              PblHermiteVector3* hermiteSegment, PblVector3& pos, PblVector3& direction)
{
    int low = 0;
    int high = numEntries - 2;
    int index;
    float start, end;
    for (;;)
    {
        index = (high + low) / 2;
        start = parameterizedArcLength[index].length;
        if (s >= start)
        {
            end = parameterizedArcLength[index + 1].length;
            if (s <= end)
            {
                break;
            }
        }
        if (start <= s)
        {
            if (s > start)
            {
                low = index + 1;
            }
        }
        else
        {
            high = index - 1;
        }
    }
    const float t = (s - start) / (end - start);
    const int segmentIndex = parameterizedArcLength[index].segmentIndex;
    pos = hermiteSegment[segmentIndex].getValue(t);
    direction = hermiteSegment[segmentIndex].getSlope(t);
}

// FUNCTION: SWBF2 0x00446400
void AsteroidClass::FillPath(const char* pcPath, float fDensity, int iCount,
                               PblVector3 vMin, PblVector3 vMax)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    const RedPath* pPath = RedPath::Find(PblHash(pcPath));
    if (!pPath)
    {
        return;
    }

    int iNumPoints = pPath->GetNumPoints();
    int iNumKeys = 10 * iNumPoints - 9;
    LengthEntryT* pKeys = new LengthEntryT[iNumKeys];
    PblHermiteVector3* pSegments = new PblHermiteVector3[iNumPoints - 1];
    pKeys[0].length = 0.0f;
    pKeys[0].segmentIndex = 0;
    float fLength = 0.0f;
    for (int i = 1; i < iNumPoints; ++i)
    {
        PblMatrix mStart(pPath->GetPointRotation(i - 1), pPath->GetPointPosition(i - 1));
        PblMatrix mEnd(pPath->GetPointRotation(i), pPath->GetPointPosition(i));
        pSegments[i - 1].Set(mStart.GetTranslation(), mEnd.GetTranslation(), mStart.GetZAxis(), mEnd.GetZAxis(), 1.0f);
        for (int j = 0; j < 10; ++j)
        {
            PblVector3 vFrom = pSegments[i - 1].getValue((float)j / 10.0f);
            PblVector3 vTo = pSegments[i - 1].getValue((float)(j + 1) / 10.0f);
            PblVector3 vStep = vTo - vFrom;
            fLength += PblMath::Sqrt(vStep.y * vStep.y + vStep.x * vStep.x + vStep.z * vStep.z);
            pKeys[(i - 1) * 10 + j + 1].length = fLength;
            pKeys[(i - 1) * 10 + j + 1].segmentIndex = i - 1;
        }
    }

    for (int i = 0; i < iCount; ++i)
    {
        PblVector3 vPosition, vDir;
        GetPoint(PblRandom::GetSim().GetFloat() * fLength, pKeys, iNumKeys,
                 pSegments, vPosition, vDir);
        const PblVector3& vPathUp =
            (float)fabs((double)(vDir.x * gVectorUnitY.x + vDir.y * gVectorUnitY.y + vDir.z * gVectorUnitY.z)) < 0.99f
                ? gVectorUnitZ
                : gVectorUnitY;
        PblMatrix mPath = DirectionMatrix(vDir, vPathUp, vPosition);

        PblVector3 vUnit = PblRandom::GetSim().GetVector3() * 2.0f - PblVector3(1.0f, 1.0f, 1.0f);
        float fOffsetLength = sqrtf(vUnit.x * vUnit.x + vUnit.z * vUnit.z);
        float fRadius = PblRandom::GetSim().GetFloat() * fDensity;
        PblVector3 vOffset(vUnit.x / fOffsetLength * fRadius, vUnit.y / fOffsetLength * fRadius, 0.0f);
        vPosition = mPath.ApplyTransform(vOffset);

        vDir.x = (vMax.x - vMin.x) * PblRandom::GetSim().GetFloat() + vMin.x;
        vDir.y = (vMax.y - vMin.y) * PblRandom::GetSim().GetFloat() + vMin.y;
        vDir.z = (vMax.z - vMin.z) * PblRandom::GetSim().GetFloat() + vMin.z;
        const PblVector3& vUp =
            (float)fabs((double)(vDir.x * gVectorUnitY.x + vDir.y * gVectorUnitY.y + vDir.z * gVectorUnitY.z)) < 0.99f
                ? gVectorUnitZ
                : gVectorUnitY;
        EntityDesc desc;
        desc.mMatrix = gMatrixIdentity;
        desc.mPlayerId = 0;
        desc.mMatrix = DirectionMatrix(vDir, vUp, vPosition);
        Build(desc);
    }
    delete[] pKeys;
    delete[] pSegments;
}

// FUNCTION: SWBF2 0x00444390
void GameObject::ReserveForPlayer(float fTime)
{
}

static inline void AsteroidCollisionFollows(Asteroid* pAsteroid)
{
    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                           (const D3DXVECTOR3*)&pAsteroid->mModel->_mModel->_BoundingSphere.GetCenter(),
                           (const D3DXMATRIX*)&pAsteroid->mMatrix);
    pAsteroid->SetCollisionCenter(vCenter);
    pAsteroid->RecalculateAABB();
}

// FUNCTION: SWBF2 0x00444820
Asteroid::Asteroid(AsteroidClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_ASTEROID, &pClass->mModel->_mLodData)
{
    mClass = pClass;
    GameObject* pObject = this;
    pObject->Activate();

    const float fOmega =
        (mClass->mMaxOmega - mClass->mMinOmega) * PblRandom::_Global.GetFloat() + mClass->mMinOmega;
    const PblVector3 vAxis = PblRandom::GetGlobalVector3();
    mOmega = vAxis * fOmega;

    const float fSpeed =
        (mClass->mMaxSpeed - mClass->mMinSpeed) * PblRandom::_Global.GetFloat() + mClass->mMinSpeed;
    mVelocity = mMatrix.GetZAxis() * fSpeed;
    mMass = 10.0f;
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x00444a30
Asteroid::~Asteroid()
{
    NetGame::Destroyed(this);
}

// SYNTHETIC: SWBF2 0x00444760
// Asteroid::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00446dca
// ??_EAsteroid@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00446dd5
// ??_EAsteroid@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00446ddd
// ??_EAsteroid@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00446de8
// ??_EAsteroid@@WCEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00446df3
// ??_EAsteroid@@WJE@AEPAXI@Z







// FUNCTION: SWBF2 0x00444540
const PblMatrix& Asteroid::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00444550
const PblVector3& Asteroid::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00444560
void Asteroid::SetPosition(const PblVector3& vPos)
{
    mMatrix.SetTranslation(vPos);
    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                           (const D3DXVECTOR3*)&mModel->_mModel->_BoundingSphere.GetCenter(),
                           (const D3DXMATRIX*)&mMatrix);
    SetCollisionCenter(vCenter);
    RecalculateAABB();
}

// FUNCTION: SWBF2 0x00444600
void Asteroid::SetMatrix(const PblMatrix& matrix)
{
    mMatrix = matrix;
    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                           (const D3DXVECTOR3*)&mModel->_mModel->_BoundingSphere.GetCenter(),
                           (const D3DXMATRIX*)&mMatrix);
    SetCollisionCenter(vCenter);
    RecalculateAABB();
}

// FUNCTION: SWBF2 0x004446b0
float Asteroid::GetMaxSpeed() const
{
    return mVelocity.y * mMatrix.m[2][1] + mVelocity.x * mMatrix.m[2][0] +
           mVelocity.z * mMatrix.m[2][2];
}

// FUNCTION: SWBF2 0x004446e0
float Asteroid::GetMaxStrafe() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004446f0
bool Asteroid::AddToTeamManager() const
{
    return false;
}

// FUNCTION: SWBF2 0x00444700
void Asteroid::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x00444720
GameObjectType Asteroid::GetGameObjectType()
{
    return GO_UNKNOWN;
}

// FUNCTION: SWBF2 0x00444730
EntityType Asteroid::GetEntityType() const
{
    return ENT_ASTEROID;
}

// FUNCTION: SWBF2 0x00444740
void Asteroid::AddSends(void (*)(GameObject*, int), int)
{
}

// FUNCTION: SWBF2 0x00444750
const PblMatrix& Asteroid::GetSmoothedMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00444ad0
void Asteroid::Init()
{
    SetRenderPos(mMatrix.GetTranslation());
    AsteroidCollisionFollows(this);
}

// FUNCTION: SWBF2 0x00444b70
void Asteroid::ClientKill()
{
    Kill();
}

// FUNCTION: SWBF2 0x00444b80
void Asteroid::Kill()
{
    mIsAlive = false;
    Deactivate();
    RemoveFromCollisionManager();

    AsteroidClass* pClass = mClass;
    if (pClass->mNumChildren > 0)
    {
        EntityDesc desc;
        desc.mPlayerId = 0;
        for (int i = 0; i < pClass->mNumChildren; ++i)
        {
            EntityClass* pChildClass = EntityClass::Find(pClass->mChildren[i].mClassHash);
            if (pChildClass)
            {
                PblMatrix mBone;
                if (mModel->_mLod[0]->GetBoneMatrix(pClass->mChildren[i].mHardPointHash, mBone))
                {
                    PblMatrix mChild;
                    mChild = mBone * mMatrix;
                    desc.mMatrix = mChild;
                    Entity* pChild = pChildClass->Build(desc);
                    if (pChild)
                    {
                        pChild->Init();
                        const PblVector3 vDir = desc.mMatrix.GetTranslation() - mMatrix.GetTranslation();
                        static_cast<EntityGeometry*>(pChild)->SetVelocity(Normalize(vDir) * 50.0f);
                    }
                }
            }
        }
        if (pClass->mExplosionBreak)
        {
            DamageOwner owner(0, 0, mTeam, mClass, 0, 1.0f);
            pClass->mExplosionBreak->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
        }
        return;
    }

    if (pClass->mExplosionDestruct)
    {
        DamageOwner owner(0, 0, mTeam, pClass, 0, 1.0f);
        pClass->mExplosionDestruct->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
    }
    CreateChunkCluster(1.0f);
}

// FUNCTION: SWBF2 0x00444e60
bool Asteroid::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    if (mCurHealth <= 0.0f)
    {
        mCurHealth = 0.0f;
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            Kill();
            return false;
        }
    }

    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        const PblMatrix& mView = CameraManager::Instance()->GetCamera(i)->GetMatrixInverse();
        CollisionObject& rObject = *this;
        const float fZ = rObject.GetPosition().x * mView.m[0][2] + rObject.GetPosition().y * mView.m[1][2] +
                         rObject.GetPosition().z * mView.m[2][2] + mView.m[3][2];
        PblVector3 vView;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vView, (const D3DXVECTOR3*)&rObject.GetPosition(),
                               (const D3DXMATRIX*)&mView);
        if (fZ < -2000.0f || fZ > 0.0f)
        {
            return true;
        }
    }

    PblQuaternion qRotation(mMatrix);
    PblVector3 vPos = mMatrix.GetTranslation();
    const float fOmega = sqrtf(mOmega.x * mOmega.x + mOmega.y * mOmega.y + mOmega.z * mOmega.z);
    if (fOmega > 0.0001f)
    {
        const float fInv = 1.0f / fOmega;
        const PblVector3 vAxis = mOmega * fInv;
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, fOmega * fDeltaTime * 0.5f);
        const PblQuaternion qStep(fCos, vAxis.x * fSin, vAxis.y * fSin, vAxis.z * fSin);
        qRotation = qRotation * qStep;
    }
    vPos += mVelocity * fDeltaTime;

    D3DXQUATERNION qNormalized;
    D3DXQuaternionNormalize(&qNormalized, (const D3DXQUATERNION*)&qRotation);
    PblMatrix mNew;
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, &qNormalized);
    mNew[3] = vPos;
    mNew.m[3][3] = 1.0f;
    mMatrix = mNew;
    SetRenderPos(vPos);
    AsteroidCollisionFollows(this);
    return true;
}

// FUNCTION: SWBF2 0x004452c0
void Asteroid::Render(unsigned char ucLod, float fDist, unsigned int uMask)
{
    const PblMatrix& mRender = GetSmoothedMatrix();
    const PblMatrix mInverse = mRender.GetInverseFast();
    SetupShadow(this, mModel, ucLod, uMask, 0);
    uMask &= ~0x10u;
    mModel->Render(ucLod, fDist, &mRender, 0, RedColor::WHITE, uMask, 0);
}

// FUNCTION: SWBF2 0x00445350
bool Asteroid::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                 Restrictor* pRestrictor)
{
    if (pOther->GetGameObject() && !pOther->GetGameObject()->IsRtti(rttiHashAsteroid))
    {
        mCurHealth = 0.0f;
    }
    return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x004453a0
void Asteroid::ActivatePhysics()
{
    ActivateThread(0, (ThreadType)2);
}

// FUNCTION: SWBF2 0x004453c0
void Asteroid::DeactivatePhysics()
{
    DeactivateThread();
}

// TEMPLATE: SWBF2 0x00446d40
// Factory<Ordnance,ExplosionClass,OrdnanceDesc>::Find
template ExplosionClass* Factory<Ordnance, ExplosionClass, OrdnanceDesc>::Find(unsigned int);
