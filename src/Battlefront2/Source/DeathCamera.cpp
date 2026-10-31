#include <d3dx9math.h>
#include "DeathCamera.h"
#include "CameraManager.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "GameObject.h"
#include "Trackable.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblMath.h"

static CollisionObject* GetCollisionObject(GameObject* pObject);

// FUNCTION: SWBF2 0x0048a2b0
DeathCamera::DeathCamera(CameraManager* pManager, int iView)
    : GameCamera(pManager, iView)
{
    mKiller = 0;
}

// SYNTHETIC: SWBF2 0x0048a2e0
// DeathCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048a310
DeathCamera::~DeathCamera()
{
}

// FUNCTION: SWBF2 0x0048a810
void DeathCamera::SetInfo(Trackable* pOwner, Character* pKiller, const PblMatrix& rMatrix)
{
    mOwner = pOwner;
    mMatrix = rMatrix;
    mTimer = 0.0f;
    mKiller = 0;

    if (!mOwner)
    {
        return;
    }

    if (!mOwner->GetGameObject())
    {
        return;
    }

    mDeathPos = mOwner->GetGameObject()->GetMatrix().GetTranslation()
                + PblVector3(0.0f, 0.8f, 0.0f);

    PblVector3 vForward = rMatrix.GetZAxis();
    vForward.y = 0.0f;
    {
        PblVector3 vNormalized;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNormalized), reinterpret_cast<const D3DXVECTOR3*>(&vForward));
        vForward = vNormalized;
    }
    vForward.y = 0.5f;
    mDeathMidPos = vForward
                   + (mOwner->GetGameObject()->GetMatrix().GetTranslation()
                      + PblVector3(0.0f, 1.0f, 0.0f));

    if (pKiller && mOwner->GetGameObject()->GetControllable()->mCharacter != pKiller)
    {
        mKiller = pKiller;
    }
}

class CollisionManager
{
public:
    static float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        CollisionObject*& pObject, PblVector3* pvPoint,
                        const GameObject** const ppIgnore, int iNumIgnore, int iMask, bool bFlag);
};

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

PblQuaternion Interpolate(const PblQuaternion& q0, const PblQuaternion& q1, float fT);


static inline CollisionObject* GetCollisionObject(GameObject* pObject)
{
    return static_cast<CollisionObject*>(pObject);
}

static const PblVector3& GetEyePoint(const Controllable* pUnit)
{
    return pUnit->mEyePoint;
}

// FUNCTION: SWBF2 0x0048a320
void DeathCamera::Update(float fDeltaTime)
{
    mTimer += fDeltaTime;

    if (mOwner)
    {
        GameObject* pObject = mOwner->GetGameObject();
        if (pObject)
        {
            mDeathPos = pObject->GetMatrix().GetTranslation()
                        + PblVector3(0.0f, 0.8f, 0.0f);
        }
    }

    PblVector3 vOldPos = mMatrix.GetTranslation();
    PblVector3 vTarget = mDeathPos;
    PblVector3 vEye = mDeathMidPos;
    float fBlend = 1.0f;
    if (mTimer < 1.0f)
    {
        fBlend = mTimer;
    }
    else if (mTimer > 1.5f && mKiller)
    {
        fBlend = Min((mTimer - 1.5f) * 0.5f, 1.0f);

        Controllable* pUnit = mKiller->mVehicle;
        if (!pUnit)
        {
            pUnit = mKiller->mUnit;
        }
        if (pUnit)
        {
            vEye = mDeathPos;
            vTarget = pUnit->mEyePoint;
        }
    }

    PblVector3 vDelta = vTarget - vEye;
    PblVector3 vDir;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vDelta));

    CollisionObject* pHitObject;
    PblVector3 vUpper = vEye + PblVector3(0.0f, 0.75f, 0.0f);
    float fHitUpper = CollisionManager::RayHit(vUpper, -vDir, 6.0f, pHitObject, NULL, NULL, 0, 0x90, false);
    PblVector3 vLower(vUpper.x, vEye.y - 0.75f, vUpper.z);
    float fHitLower = CollisionManager::RayHit(vLower, -vDir, 6.0f, pHitObject, NULL, NULL, 0, 0x90, false);

    float fHit = Min(fHitUpper, fHitLower);
    float fDist = fHit * 6.0f - 1.0f;
    PblVector3 vPos = vEye - vDir * fDist;

    PblQuaternion qNew = PblQuaternion(DirectionMatrix(-vDir, gVectorUnitY, gVectorZero));
    PblQuaternion qOld = PblQuaternion(mMatrix);
    vPos = (vPos - vOldPos) * fBlend + vOldPos;
    PblQuaternion qRot = Interpolate(qOld, qNew, fBlend);

    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize(reinterpret_cast<D3DXQUATERNION*>(&qUnit), reinterpret_cast<const D3DXQUATERNION*>(&qRot));
    D3DXMatrixRotationQuaternion(reinterpret_cast<D3DXMATRIX*>(&mNew), reinterpret_cast<const D3DXQUATERNION*>(&qUnit));
    mNew.m[3][0] = vPos.x;
    mNew.m[3][1] = vPos.y;
    mNew.m[3][2] = vPos.z;
    mNew.m[3][3] = 1.0f;
    mMatrix = mNew;
}

// FUNCTION: SWBF2 0x0048a7c0
void DeathCamera::SetupCamera(RedCamera* pCamera, float fDeltaTime)
{
    pCamera->SetMatrix(mMatrix);
    pCamera->SetZoom(1.0f);
}

// FUNCTION: SWBF2 0x0048a7f0
void DeathCamera::SetupIconVisibility(int iView)
{
    mManager->m_uiVisibleIconMask[iView] = 0;
}


// FUNCTION: SWBF2 0x0043cd40
PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos)
{
    PblVector3 vFront;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vFront), reinterpret_cast<const D3DXVECTOR3*>(&vDir));

    PblVector3 vCross = CrossProduct(vUp, vDir);
    PblVector3 vRight;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vRight), reinterpret_cast<const D3DXVECTOR3*>(&vCross));

    return PblMatrix(vRight, CrossProduct(vFront, vRight), vFront, vPos);
}
