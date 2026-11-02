#include "PebbleFL/Common/PblMath.h"
#include <d3dx9math.h>
#include "OrdnanceBullet.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"

#include <stdio.h>

#include <math.h>

#include "BirdEffect.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "Damageable.h"
#include "EntityLight.h"
#include "GameObject.h"
#include "LeafEffect.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "WaterEffect.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/RedWater.h"

// GLOBAL: SWBF2 0x007b2940
// __xmm@459c4000c59c4000c59c4000c59c4000

extern bool netIsLocalTurn;
extern bool netFrameLock;
extern bool netIsLateTurn;


const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);




PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace
{

    inline unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }

    int GameObjectTeam(const GameObject* pObject)
    {
        return pObject->GetTeam();
    }

    inline EntityClass* GetEntityClass(GameObject* pObject)
    {
        return pObject->GetEntityClass();
    }

    inline Controllable* GetControllable(GameObject* pObject)
    {
        return pObject->GetControllable();
    }
}

// SYNTHETIC: SWBF2 0x00405a60
// `dynamic initializer for 'OrdnanceBulletClass::rttiHashOrdnanceBulletClass''
// OrdnanceBulletClass::rttiHashOrdnanceBulletClass

// GLOBAL: SWBF2 0x01ef2620
// OrdnanceBulletClass::rttiHashOrdnanceBulletClass
PblHash OrdnanceBulletClass::rttiHashOrdnanceBulletClass("OrdnanceBulletClass");
// GLOBAL: SWBF2 0x0078a7d4
// OrdnanceBulletClass::rttiNameOrdnanceBulletClass
const char OrdnanceBulletClass::rttiNameOrdnanceBulletClass[] = "OrdnanceBulletClass";

static const RedColor kColorZero = { 0, 0, 0, 0 };

// FUNCTION: SWBF2 0x005fbb60
OrdnanceBulletClass::OrdnanceBulletClass(unsigned int uHash)
    : OrdnanceClass(uHash)
{
    mLifeSpan = 2.0f;

    mModel = 0;
    mStraightBullet = true;
    mFadeColorLight = false;
    mVelocity = 150.0f;
    mLodData = OrdnanceLodData();
    mImpactEffectSoft = 0;
    mImpactEffectRigid = 0;
    mImpactEffectStatic = 0;
    mImpactEffectTerrain = 0x72968E25u /* "dirtspray" */;
    mImpactEffectWater = 0x4955BD1Bu /* "watersplash_ord" */;
    mImpactEffectShieldSmall = 0xB3503FC5u /* "com_sfx_shieldimpact_sm" */;
    mImpactEffectShieldLarge = 0xD4391A69u /* "com_sfx_shieldimpact_xl" */;
    mImpactDecalTerrain = 0;
    mImpactDecalStatic = 0;
    m_uiLightOdfNameHash = 0;
    m_colorLight = RedColor(0, 0, 0, 0);
    m_endColorLight = RedColor(0, 0, 0, 0);
    m_fRadiusLight = 0.0f;
}

// FUNCTION: SWBF2 0x005fbca0
OrdnanceBulletClass::OrdnanceBulletClass(OrdnanceBulletClass* pBase, unsigned int uHash)
    : OrdnanceClass(pBase, uHash)
{
    mModel = pBase->mModel;
    mStraightBullet = pBase->mStraightBullet;
    mFadeColorLight = pBase->mFadeColorLight;
    mVelocity = pBase->mVelocity;
    mLodData = OrdnanceLodData();
    mImpactEffectSoft = pBase->mImpactEffectSoft;
    mImpactEffectRigid = pBase->mImpactEffectRigid;
    mImpactEffectStatic = pBase->mImpactEffectStatic;
    mImpactEffectTerrain = pBase->mImpactEffectTerrain;
    mImpactEffectWater = pBase->mImpactEffectWater;
    mImpactEffectShieldSmall = pBase->mImpactEffectShieldSmall;
    mImpactEffectShieldLarge = pBase->mImpactEffectShieldLarge;
    mImpactDecalTerrain = pBase->mImpactDecalTerrain;
    mImpactDecalStatic = pBase->mImpactDecalStatic;
    m_uiLightOdfNameHash = pBase->m_uiLightOdfNameHash;
    m_colorLight = pBase->m_colorLight;
    m_endColorLight = pBase->m_endColorLight;
    m_fRadiusLight = pBase->m_fRadiusLight;
}







// FUNCTION: SWBF2 0x005fbde0
void OrdnanceBulletClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x746CE73E:
    {
        int r, g, b, a;
        int iCount = sscanf(pszValue, "%d %d %d %d", &r, &g, &b, &a);
        RedColor color;
        if (iCount >= 4)
        {
            color.b = (unsigned char)(b * a / 255);
            color.g = (unsigned char)(g * a / 255);
            color.r = (unsigned char)(r * a / 255);
        }
        else if (iCount == 3)
        {
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
        }
        else
        {
            return;
        }
        color.a = 255;
        m_colorLight = color;
        break;
    }
    case 0x96E0CCB1:
    {
        int r, g, b, a;
        int iCount = sscanf(pszValue, "%d %d %d %d", &r, &g, &b, &a);
        RedColor color;
        if (iCount >= 4)
        {
            color.b = (unsigned char)(b * a / 255);
            color.g = (unsigned char)(g * a / 255);
            color.r = (unsigned char)(r * a / 255);
        }
        else if (iCount == 3)
        {
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
        }
        else
        {
            return;
        }
        color.a = 255;
        m_endColorLight = color;
        break;
    }
    case 0x32741C32:
        sscanf(pszValue, "%f", &mVelocity);
        break;
    case 0x2776564D:
        sscanf(pszValue, "%f", &m_fRadiusLight);
        break;
    case 0x087EA2F0:
        mImpactEffectStatic = PblHash(pszValue);
        break;
    case 0x2BBD32B1:
        mImpactDecalTerrain = PblHash(pszValue);
        break;
    case 0x3F30370C:
        break;
    case 0x47C86B4A:
        if (*pszValue)
        {
            mModel = RedModel::_HashTable.Find(PblHash(pszValue));
            mLodData = ModelLodData(0, 0, 0, mModel, 0);
        }
        else
        {
            mModel = 0;
        }
        break;
    case 0x5A554273:
        mImpactEffectTerrain = PblHash(pszValue);
        break;
    case 0x7A140FC6:
        mImpactDecalStatic = PblHash(pszValue);
        break;
    case 0x8FC79EBE:
        mImpactEffectShieldSmall = PblHash(pszValue);
        break;
    case 0xA3091269:
        mImpactEffectRigid = PblHash(pszValue);
        break;
    case 0xA58ACBD0:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mFadeColorLight = iValue != 0;
        break;
    }
    case 0xD6220FE6:
        mImpactEffectSoft = PblHash(pszValue);
        break;
    case 0xE369687E:
        m_uiLightOdfNameHash = PblHash(pszValue);
        break;
    case 0xECAAA59D:
        mImpactEffectWater = PblHash(pszValue);
        break;
    case 0xF73EC52A:
        mImpactEffectShieldSmall = PblHash(pszValue);
        break;
    default:
        OrdnanceClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x005fc1c0
OrdnanceClass* OrdnanceBulletClass::Derive(unsigned int uHash)
{
    return new OrdnanceBulletClass(this, uHash);
}

// FUNCTION: SWBF2 0x005fc230
Ordnance* OrdnanceBulletClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceBullet* pBullet = new OrdnanceBullet(this, desc);
    pBullet->ActivateThread(-40, PER_LOCAL_TURN);
    pBullet->AddOrdnanceToCollisionManager();
    return pBullet;
}

// SYNTHETIC: SWBF2 0x00405a50
// `dynamic initializer for 'OrdnanceBullet::rttiHashOrdnanceBullet''
// OrdnanceBullet::rttiHashOrdnanceBullet

// GLOBAL: SWBF2 0x01ef261c
// OrdnanceBullet::rttiHashOrdnanceBullet
PblHash OrdnanceBullet::rttiHashOrdnanceBullet("OrdnanceBullet");
// GLOBAL: SWBF2 0x0078a7f8
// OrdnanceBullet::rttiNameOrdnanceBullet
const char OrdnanceBullet::rttiNameOrdnanceBullet[] = "OrdnanceBullet";

// SYNTHETIC: SWBF2 0x005fc2c6
// ??_EOrdnanceBullet@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005fc2d1
// ??_EOrdnanceBullet@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005fa840
// OrdnanceBullet::`scalar deleting destructor'







// FUNCTION: SWBF2 0x005fb900
void OrdnanceBullet::CreateRicochet(const PblVector3& vPos, const PblVector3& vNormal,
                                    GameObject* pObject) const
{
    OrdnanceDesc desc;
    desc.mStrength = 1.0f;
    desc.mPosition = vPos;
    desc.mOwner = pObject;
    desc.mDamageOwner.mGameObject.mObject = pObject;
    desc.mDirection = vNormal;
    desc.mDamageOwner.mGameObject.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
    if (pObject)
    {
        Controllable* pControllable = GetControllable(pObject);
        desc.mDamageOwner.mCharacter = pControllable->mCharacter;
        desc.mDamageOwner.mTeam = pObject->GetTeam();
        desc.mDamageOwner.mClass = GetEntityClass(pObject);
        desc.mDamageOwner.mGameObject.mObject = pObject;
        desc.mDamageOwner.mGameObject.mSavedHandleId = GameObjectHandleId(pObject);
        desc.mDamageOwner.mDamageMultiplier = pControllable->GetOffenseMultiplier();
    }
    else
    {
        desc.mDamageOwner.mCharacter = 0;
        desc.mDamageOwner.mTeam = 0;
        desc.mDamageOwner.mClass = 0;
        desc.mDamageOwner.mGameObject.mObject = 0;
        desc.mDamageOwner.mGameObject.mSavedHandleId = 0;
        desc.mDamageOwner.mDamageMultiplier = 1.0f;
    }
    desc.mDamageOwner.mWeapon = 0;
    desc.mTarget = 0;
    desc.mBodyID = -1;

    if (NetGame::IsEnabled())
    {
        if (!NetGame::IsClient())
        {
            NetGame::CreateEvent_CreateOrdnance(mClass, desc);
        }
        else if (!netIsLocalTurn)
        {
            return;
        }
    }
    mClass->Build(desc);
}

// FUNCTION: SWBF2 0x005fbaa0
void OrdnanceBullet::Render(unsigned char, float, unsigned int uiFlags)
{
    if (!((OrdnanceBulletClass*)mClass)->mModel)
    {
        return;
    }

    PblVector3 vPos = mPosition;
    if (!netFrameLock && NetGame::IsEnabled() &&
        netInterpolator.GetIndex(mInterpIndex, 0, PblHandled::mHandleId))
    {
        netInterpolator.GetVec(mInterpIndex, vPos);
    }
    mOffscreenTime = 0.0f;
    const PblMatrix mat = DirectionMatrix(mVelocity, gVectorUnitY, vPos);
    ((OrdnanceBulletClass*)mClass)->mModel->Render(&mat, 0, mRenderColor, uiFlags, 0);
}

// FUNCTION: SWBF2 0x005fa890
OrdnanceBullet::OrdnanceBullet(OrdnanceBulletClass* pClass, const OrdnanceDesc& desc)
    : Ordnance(pClass, desc),
      RedSceneObject(&((OrdnanceBulletClass*)mClass)->mLodData),
      mOldPosition(desc.mPosition),
      mVelocity(Interpolate(
          ((const OrdnanceBulletClass*)mClass)->mVelocity,
          ((const OrdnanceBulletClass*)mClass->mNextCharge)->mVelocity, mRatio) * desc.mDirection),
      mFinalCollision(0)
{
    mFinalCollisionOnNextUpdate = false;
    m_pLight = 0;
    mOffscreenTime = 0.0f;
    mCheckStaticCollision = true;

    SetRenderPos(mPosition);
    SetRenderRadius(5.0f);
    AddToScene();

    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->SetOrientation(mVelocity);
    }

    if (!NetGame::IsEnabled() || NetGame::IsClient() || NetGame::IsOneWorld())
    {
        if (pClass->m_uiLightOdfNameHash)
        {
            m_pLight = EntityLightClass::Build(pClass->m_uiLightOdfNameHash);
            if (pClass->m_fRadiusLight > 0.0f)
            {
                m_pLight->SetOmniLightRadius(pClass->m_fRadiusLight);
            }
            if (pClass->m_colorLight.a > 0)
            {
                m_pLight->SetColor(pClass->m_colorLight);
            }
            PblMatrix matrix = gMatrixIdentity;
            matrix.SetTranslation(mPosition);
            if (m_pLight->_fBeamLength > 0.0f)
            {
                matrix[1] = desc.mDirection;
                matrix.m[1][3] = 0.0f;
                PblVector3 vFront = (-gVectorUnitY) + desc.mDirection * desc.mDirection.y;
                float fLength2 = vFront.x * vFront.x + vFront.y * vFront.y + vFront.z * vFront.z;
                if (fLength2 <= 0.0001f)
                {
                    vFront = gVectorUnitZ - desc.mDirection * desc.mDirection.z;
                    fLength2 = vFront.x * vFront.x + vFront.y * vFront.y + vFront.z * vFront.z;
                }
                const float fInv = 1.0f / (float)sqrt((double)fLength2);
                vFront = vFront * fInv;
                matrix[0] = CrossProduct(desc.mDirection, vFront);
                matrix.m[0][3] = 0.0f;
                matrix[2] = vFront;
                matrix.m[2][3] = 0.0f;
            }
            m_pLight->SetMatrix(matrix);
            m_pLight->TurnOn();
        }
        else if (pClass->m_fRadiusLight > 0.0f && pClass->m_colorLight.a > 0)
        {
            m_pLight = new EntityLight();
            m_pLight->SetColor(pClass->m_colorLight);
            m_pLight->SetOmniLightRadius(pClass->m_fRadiusLight);
            m_pLight->SetHaloRadius(0.0f);
            m_pLight->SetFlareIntensity(0.0f);
            PblMatrix matrix = gMatrixIdentity;
            matrix.SetTranslation(mPosition);
            m_pLight->SetMatrix(matrix);
            m_pLight->TurnOn();
        }
    }
}

// FUNCTION: SWBF2 0x005fae50
OrdnanceBullet::~OrdnanceBullet()
{
    RemoveFromScene();
    delete m_pLight;
}

// FUNCTION: SWBF2 0x005faee0
bool OrdnanceBullet::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }

    if (mFinalCollisionOnNextUpdate)
    {
        if (mFinalCollision &&
            !(mFinalCollisionHandle.mObject &&
              mFinalCollisionHandle.mObject->mHandleId != mFinalCollisionHandle.mSavedHandleId))
        {
            Collide(mFinalCollision, mPosition, mFinalCollisionNormal, 0);
        }
        return false;
    }

    if (((OrdnanceBulletClass*)mClass)->mStraightBullet && mCheckStaticCollision)
    {
        const float fSpeed = Length(mVelocity);
        const float fInv = 1.0f / fSpeed;
        const PblVector3 vDir(mVelocity.x * fInv, mVelocity.y * fInv, mVelocity.z * fInv);
        const GameObject* pOwner = static_cast<GameObject*>(mOwner);
        CollisionObject* pHitObject;
        const float fHit = CollisionManager::RayHit(mPosition, vDir, mLifeTime * fSpeed, pHitObject,
                                                    &mFinalCollisionNormal, pOwner ? &pOwner : 0,
                                                    pOwner != 0, 0x90, true);
        if (pHitObject)
        {
            mLifeTime *= fHit;
            mFinalCollision = pHitObject;
            mFinalCollisionHandle = pHitObject->GetGameObject();
        }
        mCheckStaticCollision = false;
    }

    mOldPosition = mPosition;
    const float fStep = mLifeTime > fDeltaTime ? fDeltaTime : mLifeTime;
    mPosition += mVelocity * fStep;
    mOffscreenTime += fDeltaTime;
    if (!Ordnance::Update(fDeltaTime))
    {
        mFinalCollisionOnNextUpdate = true;
    }
    SetRenderPos(mPosition);

    if (m_pLight)
    {
        m_pLight->SetPosition((mPosition + mOldPosition) * 0.5f);
        OrdnanceBulletClass* pClass = (OrdnanceBulletClass*)mClass;
        if (pClass->mFadeColorLight)
        {
            const float fAge = mLifeElapsed - pClass->mDamageTransitionDelay;
            if (fAge > 0.0f)
            {
                float fT;
                if (pClass->mDamageTransitionPeriod > 0.0f)
                {
                    fT = Clamp(fAge / pClass->mDamageTransitionPeriod, 0.0f, 1.0f);
                }
                else
                {
                    fT = 1.0f;
                }
                RedColor endColor = pClass->m_endColorLight;
                RedColor startColor = pClass->m_colorLight;
                m_pLight->SetColor(Interpolate(startColor, endColor, fT));
            }
        }
    }

    // GLOBAL: SWBF2 0x01faab98
    // sWorldBox
    static const PblBox sWorldBox(-5000.0f, -5000.0f, -5000.0f, 5000.0f, 5000.0f, 5000.0f);
    const PblVector3& vRenderPos = GetRenderPos();
    if (!(vRenderPos.x >= sWorldBox._vMin.x && vRenderPos.x <= sWorldBox._vMax.x &&
          vRenderPos.y >= sWorldBox._vMin.y && vRenderPos.y <= sWorldBox._vMax.y &&
          vRenderPos.z >= sWorldBox._vMin.z && vRenderPos.z <= sWorldBox._vMax.z))
    {
        return false;
    }

    if (!netFrameLock && NetGame::IsEnabled() && NetGame::IsClient() &&
        netInterpolator.GetIndex(mInterpIndex, 0, PblHandled::mHandleId))
    {
        netInterpolator.SetNext(mInterpIndex, &mPosition);
    }
    return true;
}

// FUNCTION: SWBF2 0x005fb330
bool OrdnanceBullet::Collide(CollisionObject* pObject, const PblVector3& vPos, const PblVector3& vNormal,
                             int iBodyId)
{
    unsigned int uDamageFlags = 0xFFFFFFFF;
    GameObject* pVictim = pObject->GetGameObject();
    if (m_pLight)
    {
        m_pLight->SetPosition(vPos + vNormal * 0.5f);
    }

    int iBody;
    if (pObject->GetCollisionType() == COLL_SHIELD)
    {
        if (pObject->mParent)
        {
            pVictim = pObject->mParent->GetGameObject();
            iBody = -1;
            uDamageFlags = 1;
        }
        else
        {
            iBody = iBodyId;
        }
    }
    else
    {
        if (pObject->mParent)
        {
            uDamageFlags = pObject->mParent->GetCollisionRadius() > pObject->GetCollisionRadius() * 1.5f
                               ? 2
                               : 0xFFFFFFFF;
        }
        iBody = iBodyId;
    }

    if (pVictim)
    {
        PblVector3 vDir;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&mVelocity));
        if (pVictim->Deflect(this, vPos, vDir))
        {
            return true;
        }
        Damageable* pDamageable = static_cast<Damageable*>(pVictim);
        pDamageable->ApplyDamage(GetDamage(), vDir, vPos, iBody, uDamageFlags);
    }

    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsOneWorld() && NetGame::IsHost() &&
         CameraManager::sInstance->IsInsideAnyCamera(vPos, 2.0f, 70.0f)))
    {
        OrdnanceBulletClass* pClass = (OrdnanceBulletClass*)mClass;
        bool bShield = false;
        unsigned int uEffect = 0;
        CollisionObjectType eType;
        if (pVictim && pVictim->IsShielded() && pVictim->mCurShield > 0.0f)
        {
            eType = COLL_SHIELD;
        }
        else
        {
            eType = pObject->GetCollisionType();
        }

        switch (eType)
        {
        case COLL_SOFT:
        case COLL_ATTACHED:
            uEffect = pClass->mImpactEffectSoft;
            break;
        case COLL_ASTEROID:
        case COLL_RIGID:
            uEffect = pClass->mImpactEffectRigid;
            break;
        case COLL_STATIC:
            uEffect = pClass->mImpactEffectStatic;
            break;
        case COLL_TERRAIN:
            uEffect = pClass->mImpactEffectTerrain;
            break;
        case COLL_WATER:
            RedWater::AddWaterRing(vPos, 1.5f, 0.01f);
            WaterEffect::AddWaterEffect(pClass->mImpactEffectWater, vPos);
            break;
        case COLL_LEAFPATCH:
            if (mOffscreenTime < 0.5f)
            {
                PblVector3 vWind = vNormal;
                vWind.y = 0.0f;
                PblVector3 vUnit;
                D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vUnit), reinterpret_cast<const D3DXVECTOR3*>(&vWind));
                vWind = PblVector3(vUnit.x * 2.0f, vUnit.y * 2.0f, vUnit.z * 2.0f);
                for (int i = 3; i; --i)
                {
                    FallingLeafParticle* pLeaf = LeafParticleManager::GetFreeLeaf();
                    if (pLeaf)
                    {
                        pLeaf->RespawnLeaf(vPos, 0, LEAF_NONE);
                        pLeaf->AddWindVelocity(vWind);
                    }
                }
                if (PblRandom::GetGlobalFloat() < 0.08f)
                {
                    BirdParticleManager::ScatterRandomBirds(2, vPos);
                }
            }
            break;
        case COLL_SHIELD:
            bShield = true;
            uEffect = pObject->GetCollisionRadius() < 16.0f ? pClass->mImpactEffectShieldSmall
                                                            : pClass->mImpactEffectShieldLarge;
            break;
        default:
            break;
        }

        if (uEffect)
        {
            FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(uEffect);
            if (pEffectClass)
            {
                FLEffectObject* pEffect = pEffectClass->BuildEffect();
                if (pEffect)
                {
                    if (pVictim)
                    {
                        PblMatrix mWorld = gMatrixIdentity;
                        if (bShield)
                        {
                            mWorld = DirectionMatrix(vNormal, gVectorUnitY, vPos);
                        }
                        else
                        {
                            mWorld[3] = vPos;
                            mWorld.m[3][3] = 1.0f;
                        }
                        PblMatrix mOffset = pVictim->GetMatrix().GetInverseFast();
                        mOffset = mWorld * mOffset;
                        pEffect->AttachEffectToObject(pVictim, mOffset, true);
                    }
                    else if (bShield)
                    {
                        pEffect->AttachEffectToVector(&vPos, vNormal, false);
                    }
                    else
                    {
                        pEffect->AttachEffectToVector(&vPos, false);
                    }
                }
            }
        }
    }
    return Ordnance::Collide(pObject, vPos, vNormal, iBody);
}

// SYNTHETIC: SWBF2 0x005fbc70
// OrdnanceBulletClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005fbdd0
OrdnanceBulletClass::~OrdnanceBulletClass()
{
}
