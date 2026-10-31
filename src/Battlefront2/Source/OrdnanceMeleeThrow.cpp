#include "OrdnanceMeleeThrow.h"
#include "EntityLight.h"
#include "WeaponMeleeInstance.h"
#include "GameObject.h"
#include "BirdEffect.h"
#include "LeafEffect.h"
#include "PebbleFL/Common/PblRandom.h"
#include "CollisionManager.h"
#include "Damageable.h"
#include "FLEffect.h"
#include "NetGame.h"
#include "WeaponMelee.h"
#include <math.h>
#include <d3dx9math.h>
#include "WeaponMeleeThrow.h"
#include <stdio.h>
#include <string.h>

// FUNCTION: SWBF2 0x00602b40
OrdnanceMeleeThrow::~OrdnanceMeleeThrow()
{
    static_cast<RedSceneObject*>(this)->Deactivate();
    delete m_pLight;
    if (m_pDamageData)
    {
        WeaponMelee::FreeDamageData((WeaponMelee::DamageData*)m_pDamageData);
        m_pDamageData = 0;
    }
}

// SYNTHETIC: SWBF2 0x006048b6
// ??_EOrdnanceMeleeThrow@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x006048be
// ??_EOrdnanceMeleeThrow@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x006021f0
// OrdnanceMeleeThrow::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00604820
Ordnance* OrdnanceMeleeThrowClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.IsFull())
    {
        return 0;
    }
    OrdnanceMeleeThrow* pOrdnance = new OrdnanceMeleeThrow(this, desc);
    pOrdnance->ActivateThread(-40, PER_LOCAL_TURN);
    pOrdnance->AddOrdnanceToCollisionManager();
    return pOrdnance;
}

// FUNCTION: SWBF2 0x006031f0
bool OrdnanceMeleeThrow::Collide(CollisionObject* pObject, const PblVector3& vPos,
                                const PblVector3& vNormal, int iBodyId)
{
    if (mLifeTime <= 0.0f)
    {
        return false;
    }

    int iReturn = -1;
    GameObject* pVictim = pObject->GetGameObject();
    if (m_pLight)
    {
        PblVector3 vMid = vPos + vNormal * 0.5f;
        static_cast<FLEffectObject*>(m_pLight)->SetPosition(vMid);
    }

    if (pObject->GetCollisionType() == COLL_SHIELD)
    {
        if (pObject->mParent)
        {
            pVictim = pObject->mParent->GetGameObject();
            iBodyId = -1;
            iReturn = 1;
        }
    }
    else if (pObject->mParent)
    {
        if (pObject->mParent->GetCollisionRadius() > pObject->GetCollisionRadius() * 1.5f)
        {
            iReturn = 2;
        }
    }

    if (pVictim)
    {
        PblVector3 vDirection;
        D3DXVec3Normalize((D3DXVECTOR3*)&vDirection, (const D3DXVECTOR3*)&mVelocity);
        if (pVictim->Deflect(this, vPos, vDirection))
        {
            Return();
            return false;
        }
    }

    if (pObject->GetCollisionType() == COLL_SOFT || pObject->GetCollisionType() == COLL_ATTACHED)
    {
        if (iReturn == 1)
        {
            Return();
        }
    }
    else if (pObject->GetCollisionType() == COLL_LEAFPATCH && mOffscreenTime < 0.5f)
    {
        PblVector3 vHorizontalNormal = vNormal;
        vHorizontalNormal.y = 0.0f;
        PblVector3 vUnit, vWind;
        D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)&vHorizontalNormal);
        vWind = PblVector3(vUnit.x * 2.0f, vUnit.y * 2.0f, vUnit.z * 2.0f);
        for (int i = 0; i < 3; ++i)
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

    Ordnance::Collide(pObject, vPos, vNormal, iBodyId);
    return false;
}

class RedPose;

namespace
{

    void RenderWeapon(WeaponMelee* pWeapon, const PblMatrix& mat, const RedPose* pPose,
                      const RedColor& color, unsigned int uiFlags, bool bArg)
    {
        typedef void(__thiscall * RenderFn)(WeaponMelee*, const PblMatrix&, const RedPose*,
                                            const RedColor&, unsigned int, bool);
        pWeapon->Render(mat, pPose, color, uiFlags, bArg);
    }
}

// SYNTHETIC: SWBF2 0x00405b20
// `dynamic initializer for 'OrdnanceMeleeThrow::rttiHashOrdnanceMeleeThrow''
// OrdnanceMeleeThrow::rttiHashOrdnanceMeleeThrow

// GLOBAL: SWBF2 0x01ef2654
// OrdnanceMeleeThrow::rttiHashOrdnanceMeleeThrow
PblHash OrdnanceMeleeThrow::rttiHashOrdnanceMeleeThrow("OrdnanceMeleeThrow");
// GLOBAL: SWBF2 0x0078a94c
// OrdnanceMeleeThrow::rttiNameOrdnanceMeleeThrow
const char OrdnanceMeleeThrow::rttiNameOrdnanceMeleeThrow[] = "OrdnanceMeleeThrow";

// SYNTHETIC: SWBF2 0x00405b30
// `dynamic initializer for 'OrdnanceMeleeThrowClass::rttiHashOrdnanceMeleeThrowClass''
// OrdnanceMeleeThrowClass::rttiHashOrdnanceMeleeThrowClass

// GLOBAL: SWBF2 0x01ef2650
// OrdnanceMeleeThrowClass::rttiHashOrdnanceMeleeThrowClass
PblHash OrdnanceMeleeThrowClass::rttiHashOrdnanceMeleeThrowClass("OrdnanceMeleeThrowClass");
// GLOBAL: SWBF2 0x0078a96c
// OrdnanceMeleeThrowClass::rttiNameOrdnanceMeleeThrowClass
const char OrdnanceMeleeThrowClass::rttiNameOrdnanceMeleeThrowClass[] = "OrdnanceMeleeThrowClass";







// FUNCTION: SWBF2 0x006021d0
const PblVector3& OrdnanceMeleeThrow::GetOldPosition() const
{
    return mOldPosition;
}

// FUNCTION: SWBF2 0x006021e0
const PblVector3& OrdnanceMeleeThrow::GetVelocity() const
{
    return mVelocity;
}







// FUNCTION: SWBF2 0x00602270
float OrdnanceMeleeThrowClass::GetVelocity()
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00602280
float OrdnanceMeleeThrowClass::GetRange()
{
    return mVelocity * mLifeSpan;
}

// FUNCTION: SWBF2 0x006034e0
void OrdnanceMeleeThrow::Render(unsigned char, float, unsigned int uiFlags)
{
    if (m_pOwner.mObject && m_pOwner.mObject->mHandleId != m_pOwner.mSavedHandleId)
    {
        return;
    }
    if (!m_pWeapon || !m_pWeapon->m_pPrimaryWeapon)
    {
        return;
    }

    mOffscreenTime = 0.0f;
    PblMatrix mat = m_mMatrix;
    RenderWeapon(m_pWeapon->m_pPrimaryWeapon, mat, 0, RedColor::WHITE, uiFlags | 1, true);
}

// FUNCTION: SWBF2 0x00603550
void OrdnanceMeleeThrow::Return()
{
    if (mLifeTime > 0.0f)
    {
        m_fReturnTime -= mLifeTime;
        mLifeTime = 0.0f;
    }
}

extern bool netIsLateTurn;



// FUNCTION: SWBF2 0x00602290
OrdnanceMeleeThrow::OrdnanceMeleeThrow(OrdnanceMeleeThrowClass* pClass, const OrdnanceDesc& desc)
    : Ordnance(pClass, desc),
      RedSceneObject(&((OrdnanceMeleeThrowClass*)mClass)->mLodData),
      m_pOwner(desc.mOwner),
      mOldPosition(desc.mPosition),
      mVelocity(desc.mDirection * Interpolate(((const OrdnanceMeleeThrowClass*)mClass)->mVelocity,
                                              ((const OrdnanceMeleeThrowClass*)mClass->mNextCharge)->mVelocity,
                                              mRatio))
{
    m_pDamageData = 0;
    m_pLight = 0;
    mOffscreenTime = 0.0f;
    m_fReturnTime = 0.0f;

    m_pWeapon = (WeaponMeleeThrow*)desc.mFireWeapon;
    m_pWeapon->m_pPrimaryWeapon->TurnOn(0.01f);
    m_pWeapon->m_pPrimaryWeapon->m_uiTrailOnMask = 0xf;
    m_pWeapon->NotifyThrow();
    m_bLastFrameIndicator = false;

    m_mMatrix = m_pWeapon->m_mThrow0Local * m_pOwner->GetMatrix();

    const WeaponMeleeClass* pWeaponClass = (const WeaponMeleeClass*)m_pWeapon->m_pPrimaryWeapon->mClass;
    PblVector3 vEdgeDir = gVectorUnitZ;
    if (pWeaponClass->m_uiNumDamageEdges)
    {
        vEdgeDir = m_mMatrix.ApplyRotation(pWeaponClass->m_aDamageEdge[0].vOffsetDir);
    }
    const PblVector3& vDir = desc.mDirection;
    m_vRotationAxis = Normalize(CrossProduct(vEdgeDir, vDir));

    mCheckStaticCollision = true;
    mLifeTime = mClass->mLifeSpan;
    m_fReturnTime = mLifeTime;

    float fRadius2 = 1.0f;
    for (unsigned int i = 0; i < pWeaponClass->m_uiNumDamageEdges; ++i)
    {
        const WeaponMeleeClass::DamageEdge& edge = pWeaponClass->m_aDamageEdge[i];
        if (edge.uiAttachToWeapon)
        {
            continue;
        }
        const float fLength =
            pClass->m_bDamageLengthFromEdge ? edge.fLength * pClass->m_fDamageLength : pClass->m_fDamageLength;
        const float fWidth =
            pClass->m_bDamageWidthFromEdge ? edge.fWidth * pClass->m_fDamageWidth : pClass->m_fDamageWidth;
        const PblVector3 vTip = edge.vOffsetPos + edge.vOffsetDir * fLength;
        const float fTip2 = SquaredLength(vTip) + fWidth * fWidth;
        if (fTip2 > fRadius2)
        {
            fRadius2 = fTip2;
        }
        const float fBase2 = SquaredLength(edge.vOffsetPos);
        if (fBase2 > fRadius2)
        {
            fRadius2 = fBase2;
        }
    }

    const float fRadius = sqrtf(fRadius2);
    SetRenderPos(mPosition);
    SetRenderRadius(fRadius);
    AddToScene();

    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->SetOrientation(mVelocity);
    }

    mDecrementLifeTime = false;
    if (!NetGame::IsEnabled() || NetGame::IsClient())
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
                matrix[1] = vDir;
                matrix.m[1][3] = 0.0f;
                PblVector3 vFront = -gVectorUnitY + vDir * vDir.y;
                float fLength2 = SquaredLength(vFront);
                if (fLength2 <= 0.0001f)
                {
                    vFront = gVectorUnitZ - vDir * vDir.z;
                    fLength2 = SquaredLength(vFront);
                }
                vFront *= 1.0f / sqrtf(fLength2);
                matrix[0] = CrossProduct(vDir, vFront);
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

// FUNCTION: SWBF2 0x00602bf0
bool OrdnanceMeleeThrow::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }

    OrdnanceMeleeThrowClass* pClass = (OrdnanceMeleeThrowClass*)mClass;
    if (m_pOwner.mObject && m_pOwner.mObject->mHandleId != m_pOwner.mSavedHandleId)
    {
        return false;
    }
    if (m_bLastFrameIndicator)
    {
        return false;
    }

    if (mLifeTime <= 0.0f)
    {
        mLifeTime -= fDeltaTime;
        const float fReturnDuration = m_fReturnTime / pClass->mLifeSpan * pClass->m_fReturnDuration;
        if (-mLifeTime >= fReturnDuration)
        {
            m_pWeapon->NotifyReturned();
            m_pWeapon->m_pPrimaryWeapon->m_uiTrailOnMask = 0;
            m_bLastFrameIndicator = true;
            return true;
        }

        const float fPrev = (-mLifeTime - fDeltaTime) / fReturnDuration;
        const float fT = (-(mLifeTime / fReturnDuration) - fPrev) / (1.0f - fPrev);
        const PblVector3 vTarget = mOwner->GetMatrix().ApplyTransform(m_pWeapon->m_mThrow0Local.GetTranslation());
        mOldPosition = mPosition;
        mPosition = Interpolate(mOldPosition, vTarget, fT);
        PblMatrix mRotate(gMatrixIdentity);
        D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis,
                               pClass->m_fOmega * fDeltaTime * 0.5f);
        m_mMatrix = m_mMatrix * mRotate;
        m_mMatrix.SetTranslation(mPosition);
        m_pWeapon->m_pPrimaryWeapon->UpdateEffects(fDeltaTime, mPosition, mVelocity, 0);
        SetRenderPos(mPosition);
        ApplyDamage(fDeltaTime);
        return true;
    }

    if (mCheckStaticCollision)
    {
        const float fSpeed = Length(mVelocity);
        const PblVector3 vDir = mVelocity / fSpeed;
        CollisionObject* pHit;
        PblVector3 vNormal;
        const float fHit =
            CollisionManager::RayHit(mPosition, vDir, mLifeTime * fSpeed, pHit, &vNormal, 0, 0, 0x90, true);
        if (fHit < 1.0f)
        {
            mLifeTime *= fHit;
            m_fReturnTime *= fHit;
        }
        mCheckStaticCollision = false;
    }

    mLifeTime -= fDeltaTime;
    float fStep = fDeltaTime;
    if (mLifeTime <= 0.0f)
    {
        if (mLifeTime + fDeltaTime > 0.0f && m_pDamageData)
        {
            ((WeaponMelee::DamageData*)m_pDamageData)->iNumObjects = 0;
        }
        fStep = mLifeTime;
    }
    mOffscreenTime += fStep;
    ApplyDamage(fStep);

    mOldPosition = mPosition;
    mPosition += mVelocity * fStep;
    PblMatrix mRotate = gMatrixIdentity;
    D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis, pClass->m_fOmega * fStep);
    m_mMatrix = m_mMatrix * mRotate;
    m_mMatrix.SetTranslation(mPosition);

    if (!Ordnance::Update(fStep) && mLifeTime > 0.0f)
    {
        Return();
    }

    m_pWeapon->m_pPrimaryWeapon->UpdateEffects(fStep, mPosition, mVelocity,
                                               pClass->m_OnSound.GetProperties() ? &pClass->m_OnSound : 0);
    SetRenderPos(mPosition);
    if (m_pLight)
    {
        static_cast<FLEffectObject*>(m_pLight)->SetPosition((mPosition + mOldPosition) * 0.5f);
    }
    if (mLifeTime <= 0.0f)
    {
        Return();
    }
    return true;
}

// FUNCTION: SWBF2 0x00603580
void OrdnanceMeleeThrow::ApplyDamage(float fDeltaTime)
{
    OrdnanceMeleeThrowClass* pClass = (OrdnanceMeleeThrowClass*)mClass;
    const WeaponMeleeClass* pWeaponClass = (const WeaponMeleeClass*)m_pWeapon->m_pPrimaryWeapon->mClass;
    const unsigned int uiNumEdges = pWeaponClass->m_uiNumDamageEdges;
    if (!uiNumEdges)
    {
        return;
    }

    float fMaxLength = 0.0f;
    if (!m_pDamageData)
    {
        m_pDamageData = WeaponMelee::AllocDamageData();
        if (!m_pDamageData)
        {
            return;
        }
        ((WeaponMelee::DamageData*)m_pDamageData)->pAttack = 0;
        ((WeaponMelee::DamageData*)m_pDamageData)->pNext = 0;
        ((WeaponMelee::DamageData*)m_pDamageData)->fTime = 0.0f;
        ((WeaponMelee::DamageData*)m_pDamageData)->iNumObjects = 0;
    }

    float fStepTime = 0.5f / (pClass->mVelocity + pClass->m_fOmega);
    if (fStepTime > 0.0333f)
    {
        fStepTime = 0.0333f;
    }
    const int iNumSteps = (int)(fDeltaTime / fStepTime) + 1;
    const float fStep = fDeltaTime / (float)iNumSteps;

    PblMatrix mRotate = gMatrixIdentity;
    D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis, fStep * pClass->m_fOmega);
    PblMatrix mFirst = m_mMatrix * mRotate;
    mFirst.SetTranslation(m_mMatrix.GetTranslation() + mVelocity * fStep);
    PblMatrix mLast;
    if (fDeltaTime > fStep)
    {
        D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis,
                               fDeltaTime * pClass->m_fOmega);
        mLast = m_mMatrix * mRotate;
        mLast.SetTranslation(m_mMatrix.GetTranslation() + mVelocity * fDeltaTime);
    }
    else
    {
        mLast = mFirst;
    }

    PblBox box(mLast.GetTranslation());
    float fMaxWidth = 0.0f;
    for (unsigned int i = 0; i < uiNumEdges; ++i)
    {
        const WeaponMeleeClass::DamageEdge& edge = pWeaponClass->m_aDamageEdge[i];
        if (edge.uiAttachToWeapon)
        {
            continue;
        }
        if (edge.fLength > fMaxLength)
        {
            fMaxLength = edge.fLength;
        }
        if (edge.fWidth > fMaxWidth)
        {
            fMaxWidth = edge.fWidth;
        }
        box.AddPoint(mFirst.ApplyTransform(edge.vOffsetPos));
        box.AddPoint(mLast.ApplyTransform(edge.vOffsetPos));
    }
    const float fLength = pClass->m_bDamageLengthFromEdge ? fMaxLength * pClass->m_fDamageLength
                                                          : pClass->m_fDamageLength;
    const float fWidth = pClass->m_bDamageWidthFromEdge ? pClass->m_fDamageWidth * fMaxWidth
                                                        : pClass->m_fDamageWidth;
    const PblVector3 vCenter = (box._vMax + box._vMin) * 0.5f;
    const float fRadius = Length(box._vMax - box._vMin) * 0.5f + (fWidth + fLength) * 1.1f;

    GameObject* apObjects[8];
    int iNumObjects =
        CollisionManager::GetObjectsInRange(vCenter, fRadius, apObjects, 8, mOwner->GetGameObject(), 0x3a);
    GameObject* pFirstHit = 0;
    FLEffectClass* pHitEffectClass = 0;
    for (int i = 0; i < iNumObjects;)
    {
        GameObject* pObject = apObjects[i];
        if (!pObject->GetCollisionObject())
        {
            apObjects[i] = apObjects[--iNumObjects];
            continue;
        }
        const WeaponMelee::DamageData* pData = (const WeaponMelee::DamageData*)m_pDamageData;
        int k;
        for (k = 0; k < pData->iNumObjects; ++k)
        {
            if (pData->apObject[k] == pObject)
            {
                break;
            }
        }
        if (k < pData->iNumObjects)
        {
            apObjects[i] = apObjects[--iNumObjects];
            continue;
        }
        ++i;
    }

    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocity);
    for (int iStep = 0; iStep < iNumSteps && iNumObjects; ++iStep)
    {
        const float fTime = (float)iStep * fStep;
        if (fTime > m_fReturnTime)
        {
            break;
        }
        PblMatrix mStep;
        if (iStep < iNumSteps - 1)
        {
            D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis,
                                   fTime * pClass->m_fOmega);
            mStep = m_mMatrix * mRotate;
            mStep.SetTranslation(m_mMatrix.GetTranslation() + mVelocity * fTime);
        }
        else
        {
            mStep = mLast;
        }

        for (unsigned int e = 0; e < uiNumEdges && iNumObjects; ++e)
        {
            const WeaponMeleeClass::DamageEdge& edge = pWeaponClass->m_aDamageEdge[e];
            if (edge.uiAttachToWeapon)
            {
                continue;
            }
            const float fEdgeLength =
                pClass->m_bDamageLengthFromEdge ? edge.fLength * pClass->m_fDamageLength : pClass->m_fDamageLength;
            const float fEdgeWidth =
                pClass->m_bDamageWidthFromEdge ? edge.fWidth * pClass->m_fDamageWidth : pClass->m_fDamageWidth;
            PblVector3 vEdgeDir;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vEdgeDir, (const D3DXVECTOR3*)&edge.vOffsetDir,
                                    (const D3DXMATRIX*)&mStep);
            const PblVector3 vBack = vEdgeDir * 0.2f;
            const PblVector3 vStart = mStep.ApplyTransform(edge.vOffsetPos) - vBack;

            for (int i = 0; i < iNumObjects;)
            {
                GameObject* pObject = apObjects[i];
                const float t =
                    pObject->GetCollisionObject()->RayHit(vStart, vEdgeDir, fEdgeLength, 0, 0, true, fEdgeWidth);
                if (t >= 1.0f)
                {
                    ++i;
                }
                else
                {
                    apObjects[i] = apObjects[--iNumObjects];

                    WeaponMelee::DamageData* pData = (WeaponMelee::DamageData*)m_pDamageData;
                    if (pData->iNumObjects < WeaponMelee::DamageData::MAX_OBJECTS_PER_ATTACK)
                    {
                        pData->apObject[pData->iNumObjects] = pObject;
                        ++((WeaponMelee::DamageData*)m_pDamageData)->iNumObjects;
                        const PblVector3 vHit = vEdgeDir * (t * fEdgeLength) + vStart;

                        if (!pObject->Deflect(0, vDir, vHit))
                        {
                            static_cast<Damageable*>(pObject)->ApplyDamage(GetDamage(), vDir, vHit, -1, 0xffffffff);

                            if (pWeaponClass->m_uiNumHitEffects > 0 && pWeaponClass->m_uiHitEffects[0] &&
                                !pHitEffectClass)
                            {
                                pHitEffectClass = FLEffect::s_EffectClasses.Find(pWeaponClass->m_uiHitEffects[0]);
                            }
                            if (pHitEffectClass)
                            {
                                FLEffectObject* pEffect = pHitEffectClass->BuildEffect();
                                if (pEffect)
                                {
                                    pEffect->AttachEffectToVector(&vHit, false);
                                }
                            }
                            if (!pFirstHit)
                            {
                                pFirstHit = pObject;
                            }
                        }
                    }
                }
            }
        }
    }

    if (pFirstHit)
    {
        pWeaponClass->m_HitSound.Play(&pFirstHit->GetMatrix().GetTranslation(), 0, false, true);
    }
}

static const RedColor kColorZero = { 0, 0, 0, 0 };

// FUNCTION: SWBF2 0x006042d0
OrdnanceMeleeThrowClass::OrdnanceMeleeThrowClass(unsigned int uHash)
    : OrdnanceClass(uHash)
{
    mLifeSpan = 3.0f;

    mLodData = OrdnanceLodData();
    m_bDamageLengthFromEdge = 1;
    m_bDamageWidthFromEdge = 1;
    m_colorLight = RedColor(0, 0, 0, 0);
    mVelocity = 20.0f;
    m_fReturnDuration = 1.0f;
    m_fDamageLength = 1.0f;
    m_fOmega = PblMath::fTwoPi * 2.0f;
    m_fDamageWidth = 1.0f;
    m_uiLightOdfNameHash = 0;
    m_fRadiusLight = 0.0f;
}

// FUNCTION: SWBF2 0x006043e0
OrdnanceMeleeThrowClass::OrdnanceMeleeThrowClass(OrdnanceMeleeThrowClass* pBase, unsigned int uHash)
    : OrdnanceClass(pBase, uHash)
{
    mLodData = OrdnanceLodData();
    mVelocity = pBase->mVelocity;
    m_fReturnDuration = pBase->m_fReturnDuration;
    m_fOmega = pBase->m_fOmega;
    m_fDamageLength = pBase->m_fDamageLength;
    m_fDamageWidth = pBase->m_fDamageWidth;
    m_bDamageLengthFromEdge = pBase->m_bDamageLengthFromEdge;
    m_bDamageWidthFromEdge = pBase->m_bDamageWidthFromEdge;
    m_uiLightOdfNameHash = pBase->m_uiLightOdfNameHash;
    m_colorLight = pBase->m_colorLight;
    m_fRadiusLight = pBase->m_fRadiusLight;
    m_OnSound = pBase->m_OnSound;
}

// FUNCTION: SWBF2 0x00604530
void OrdnanceMeleeThrowClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x32741C32:
    {
        sscanf(pszValue, "%f", &mVelocity);
            break;
    }
    case 0x2776564D:
    {
        sscanf(pszValue, "%f", &m_fRadiusLight);
            break;
    }
    case 0x0DDA9B5B:
    {
        sscanf(pszValue, "%f", &m_fReturnDuration);
            break;
    }
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
            color.a = 255;
            m_colorLight = color;
        }
        else if (iCount == 3)
        {
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
            color.a = 255;
            m_colorLight = color;
        }
            break;
    }
    case 0xDE71B19F:
    {
        m_OnSound.SetID(pszValue, false, false);
            break;
    }
    case 0xC3B311DC:
    {
        char acEdge[256];
        m_bDamageLengthFromEdge = 0;
        if (sscanf(pszValue, "%f %s", &m_fDamageLength, acEdge) == 2 && !_stricmp(acEdge, "fromedge"))
        {
            m_bDamageLengthFromEdge = 1;
        }
            break;
    }
    case 0xE369687E:
    {
        m_uiLightOdfNameHash = PblHash(pszValue);
            break;
    }
    case 0xE6989765:
    {
        float fRate;
        sscanf(pszValue, "%f", &fRate);
        m_fOmega = PblMath::fTwoPi * fRate;
            break;
    }
    case 0xE4B667AC:
    {
        char acEdge[256];
        m_bDamageWidthFromEdge = 0;
        if (sscanf(pszValue, "%f %s", &m_fDamageWidth, acEdge) == 2 && !_stricmp(acEdge, "fromedge"))
        {
            m_bDamageWidthFromEdge = 1;
        }
            break;
    }
    default:
    {
        OrdnanceClass::SetProperty(uProperty, pszValue);
            break;
    }
    }
}

// FUNCTION: SWBF2 0x006047b0
OrdnanceClass* OrdnanceMeleeThrowClass::Derive(unsigned int uHash)
{
    return new OrdnanceMeleeThrowClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x006043b0
// OrdnanceMeleeThrowClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00604520
OrdnanceMeleeThrowClass::~OrdnanceMeleeThrowClass()
{
}
