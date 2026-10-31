#include <d3dx9math.h>
#include "PebbleFL/Common/PblHashTable.h"

#include <new>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "WeaponMelee.h"
#include "WeaponMeleeInstance.h"
#include "WeaponMeleeThrow.h"
#include "LightsaberTrailEffect.h"
#include "ParticleSystem.h"
#include "Aimer.h"
#include "Combo.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "SoldierAnimationBank.h"
#include "SoldierAnimationData.h"
#include "SoldierAnimator.h"
#include "SoldierAnimatorClass.h"
#include "SoldierAnimType.h"
#include "Trigger.h"
#include "Explosion.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "Battlefront2/Platform/Pc/pcLightBeam.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblMath.h"
#include "EnergyBar.h"
#include "FLEffect.h"
#include "Ordnance.h"
#include "OrdnanceBeam.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "Battlefront2/Platform/Pc/pcSunFlare.h"

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);

extern bool netInShell;
extern bool netEnabled;
extern bool netEnabledNext;
extern bool netIsLocalTurn;

extern bool gAltMeleeHitEffects;

// FUNCTION: SWBF2 0x0068f260
void _RenderLightSabre(const PblVector3& vPosition, const PblVector3& vDirection,
                       unsigned int uiTexture, unsigned int uiGlowTexture,
                       float fLength, float fWidth, unsigned int uiFlags)
{
    if (!uiTexture || fLength == 0.0f || fWidth == 0.0f)
    {
        return;
    }

    RedRenderer::GetCamera();
    const float fExtent = (fLength + fWidth) * 2.0f;
    RedParticleRenderer::SubmitBounds(
        PblVector3(vPosition.x - fExtent, vPosition.y - fExtent, vPosition.z - fExtent),
        PblVector3(vPosition.x + fExtent, vPosition.y + fExtent, vPosition.z + fExtent));
    RedParticleRenderer::SubmitTexture(uiTexture, BLEND_ADDITIVE, uiFlags);

    RedParticleRenderer::TexCoords texCoords = {0, 0, 255, 255};
    RedParticleRenderer::SubmitParticle(PIT_LASER_START, vPosition + vDirection * fLength, RedColor::WHITE,
                                        fWidth * 0.8f, 0.0f, texCoords, gVectorUnitX, gVectorUnitY);
    RedParticleRenderer::SubmitParticle(PIT_LASER_END, vPosition - vDirection * (fLength * 0.04f), RedColor::WHITE,
                                        fWidth, 0.0f, texCoords, gVectorUnitX, gVectorUnitY);

    if (uiGlowTexture)
    {
        fWidth = (PblRandom::GetGlobalFloat() + 1.5f) * fWidth;
        RedParticleRenderer::SubmitTexture(uiGlowTexture, BLEND_ADDITIVE, uiFlags);
        RedColor glow = {255, 255, 255, 0x33};
        RedParticleRenderer::SubmitParticle(PIT_LASER_START, vPosition + vDirection * fLength, glow,
                                            fWidth * 0.8f, 0.0f, texCoords, gVectorUnitX, gVectorUnitY);
        RedParticleRenderer::SubmitParticle(PIT_LASER_END, vPosition, glow,
                                            fWidth, 0.0f, texCoords, gVectorUnitX, gVectorUnitY);
    }
}

// FUNCTION: SWBF2 0x0068a050
void WeaponMelee::Render(const PblMatrix& matrix, const RedPose* pPose,
                         const RedColor& color, unsigned int uiFlags, bool bRenderModel) const
{
    uiFlags |= 1;
    bool bThrown = false;
    if (m_pMeleeThrow)
    {
        if (pPose && m_pMeleeThrow->IsCurrentlyThrown())
        {
            bThrown = true;
        }
        const bool bReturned = m_pMeleeThrow->m_bReturnedThisFrame != 0;
        if (bReturned)
        {
            bThrown = true;
        }
    }

    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mRenderClass);
    RedModel* pModel = bRenderModel && pClass->mHighResModel ?
                       pClass->mHighResModel : pClass->mModel;
    const PblMatrix* apMatrix[4] = {};
    PblMatrix aMatrix[4];
    if (pModel && !bThrown)
    {
        if (pPose)
        {
            const PblMatrix* pBone = pPose->Find(pClass->m_auiHardpoint[0]);
            if (pBone)
            {
                aMatrix[0] = *pBone * matrix;
                apMatrix[0] = &aMatrix[0];
                if (bRenderModel)
                {
                    pModel->Render(apMatrix[0], 0, color, uiFlags, 0);
                }
            }
        }
        else
        {
            apMatrix[0] = &matrix;
            if (bRenderModel)
            {
                pModel->Render(apMatrix[0], 0, color, uiFlags, 0);
            }
        }
    }

    for (unsigned int i = 1; i < pClass->m_uiNumWeapons; ++i)
    {
        RedModel* pWeaponModel = pClass->m_apWeaponModel[i - 1];
        if (!pWeaponModel || !pPose)
        {
            continue;
        }
        const PblMatrix* pBone = pPose->Find(pClass->m_auiHardpoint[i]);
        if (!pBone)
        {
            continue;
        }
        aMatrix[i] = *pBone * matrix;
        apMatrix[i] = &aMatrix[i];
        if (bRenderModel)
        {
            pWeaponModel->Render(apMatrix[i], 0, color, uiFlags, 0);
        }
    }

    const bool bNoTrail = (uiFlags & 0x100) != 0;
    bool bRenderedBlade = false;
    for (unsigned int i = bThrown ? 1 : 0; i < pClass->m_uiNumDamageEdges; ++i)
    {
        const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[i];
        if (!edge.uiTextureHash)
        {
            continue;
        }
        const unsigned int uiWeapon = edge.uiAttachToWeapon;
        const PblMatrix* pEdgeMatrix = apMatrix[uiWeapon];
        if (!pEdgeMatrix)
        {
            continue;
        }
        if (!bThrown && edge.colorTrail.a && !bNoTrail && bRenderModel)
        {
            PblMatrix mTrail = *pEdgeMatrix;
            const_cast<WeaponMelee*>(this)->RenderLightsaberTrail(i, &mTrail,
                                                                   ((1u << i) & m_uiTrailOnMask) && IsOn(uiWeapon), uiFlags);
        }
        PblVector3 vPosition, vDirection;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPosition), reinterpret_cast<const D3DXVECTOR3*>(&edge.vOffsetPos), reinterpret_cast<const D3DXMATRIX*>(pEdgeMatrix));
        D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vDirection), reinterpret_cast<const D3DXVECTOR3*>(&edge.vOffsetDir), reinterpret_cast<const D3DXMATRIX*>(pEdgeMatrix));
        _RenderLightSabre(vPosition, vDirection, edge.uiTextureHash,
                           bRenderModel ? edge.uiGlowTextureHash : 0,
                           GetLightSaberLengthFactor(uiWeapon) * edge.fLength, edge.fWidth, uiFlags);
        bRenderedBlade = true;
    }
    if (bRenderedBlade && (uiFlags & 0x4000000))
    {
        ParticleSystem::FlushParticleCache();
        RedParticleRenderer::RenderAll();
    }
}

// FUNCTION: SWBF2 0x0068e8f0
void WeaponMeleeClass::Render(float fTurnOn, const PblMatrix& matrix, const RedPose* pPose,
                              const RedColor& color, unsigned int  , bool  ) const
{
    const unsigned int uiRenderFlags = 0x04000000;
    const PblMatrix* apMatrix[4] = {};
    PblMatrix aMatrix[4];
    RedModel* pModel = mModel;
    if (pModel)
    {
        if (pPose)
        {
            const PblMatrix* pBone = pPose->Find(m_auiHardpoint[0]);
            if (pBone)
            {
                aMatrix[0] = *pBone * matrix;
                apMatrix[0] = &aMatrix[0];
                pModel->Render(apMatrix[0], 0, color, uiRenderFlags, 0);
            }
        }
        else
        {
            apMatrix[0] = &matrix;
            pModel->Render(apMatrix[0], 0, color, uiRenderFlags, 0);
        }
    }

    for (unsigned int i = 1; i < m_uiNumWeapons; ++i)
    {
        RedModel* pWeaponModel = m_apWeaponModel[i - 1];
        if (!pWeaponModel || !pPose)
        {
            continue;
        }
        const PblMatrix* pBone = pPose->Find(m_auiHardpoint[i]);
        if (!pBone)
        {
            continue;
        }
        aMatrix[i] = *pBone * matrix;
        apMatrix[i] = &aMatrix[i];
        pWeaponModel->Render(apMatrix[i], 0, color, uiRenderFlags, 0);
    }

    bool bRenderedBlade = false;
    for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
    {
        const DamageEdge& edge = m_aDamageEdge[i];
        if (!edge.uiTextureHash)
        {
            continue;
        }
        const PblMatrix* pEdgeMatrix = apMatrix[edge.uiAttachToWeapon];
        if (!pEdgeMatrix)
        {
            continue;
        }
        PblVector3 vPosition, vDirection;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPosition), reinterpret_cast<const D3DXVECTOR3*>(&edge.vOffsetPos), reinterpret_cast<const D3DXMATRIX*>(pEdgeMatrix));
        D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vDirection), reinterpret_cast<const D3DXVECTOR3*>(&edge.vOffsetDir), reinterpret_cast<const D3DXMATRIX*>(pEdgeMatrix));
        _RenderLightSabre(vPosition, vDirection, edge.uiTextureHash, edge.uiGlowTextureHash,
                           edge.fLength * fTurnOn, edge.fWidth, uiRenderFlags);
        bRenderedBlade = true;
    }
    if (bRenderedBlade)
    {
        ParticleSystem::FlushParticleCache();
        RedParticleRenderer::RenderAll();
    }
}

namespace GameLoop
{
    extern float sClientDeltaTime;
}

// GLOBAL: SWBF2 0x01fac398
static unsigned int s_uiODFComboId;

namespace DefaultCombo
{
    enum ComboState
    {
        IDLE = 0,
        ATTACK1 = 1,
        RECOVER1 = 2,
        ATTACK2 = 3,
        RECOVER2 = 4,
        ATTACK3 = 5,
        RECOVER3 = 6,
        NUM_STATES = 7
    };

    struct DeflectAnimation
    {
        char* szName;
        unsigned char uiDirections;
    };

    static const unsigned int NUM_DEFLECT_ANIMATIONS = 8;

    // GLOBAL: SWBF2 0x0078c0fc
    char* aszAnimationName[2] = { "stand_attack1a", "stand_attack1b" };
    // GLOBAL: SWBF2 0x0078c104
    int saiAnimForState[NUM_STATES] = { -1, 0, 0, 1, 1, 0, 0 };
    // GLOBAL: SWBF2 0x0078c164
    char* aszStateName[NUM_STATES] = { "IDLE", "ATTACK1", "RECOVER1", "ATTACK2",
                                       "RECOVER2", "ATTACK3", "RECOVER3" };
}

static const float LIGHTSABER_DEFAULT_LENGTH = 1.0f;
static const float LIGHTSABER_DEFAULT_WIDTH = 0.08f;
static const float MELEE_DEFAULT_STATE_DURATION = 1.0f;
static const float MELEE_DEFAULT_DAMAGE_TIME = 0.2f;
static const float MELEE_DEFAULT_DAMAGE = 1000.0f;
static const float MELEE_DEFAULT_DAMAGE_DISTANCE = 1.2f;
static const float MELEE_DEFAULT_TURNONDURATION = 0.2f;

// GLOBAL: SWBF2 0x01f9a8f8
RedColor LIGHTSABER_DEFAULT_TRAIL_COLOR;


// FUNCTION: SWBF2 0x00689d10
bool WeaponMelee::WillBecomeBusy() const
{
    return IsBusy();
}

// FUNCTION: SWBF2 0x00689d80
void WeaponMelee::TurnOn(float fDuration)
{
    if (fDuration < 0.0f)
    {
        fDuration = static_cast<const WeaponMeleeClass*>(mClass)->m_fTurnOnDuration;
    }

    if (!m_bLightsaberOn)
    {
        if (m_fTurnOnTimer < m_fTurnOnDuration)
        {
            m_fTurnOnTimer = (1.0f - m_fTurnOnTimer / m_fTurnOnDuration) * fDuration;
        }
        else
        {
            m_fTurnOnTimer = 0.0f;
        }
        m_bLightsaberOn = 1;
        m_fTurnOnDuration = fDuration;
    }
    else if (m_fTurnOnTimer < m_fTurnOnDuration && fDuration < m_fTurnOnDuration)
    {
        m_fTurnOnTimer = fDuration / m_fTurnOnDuration * m_fTurnOnTimer;
        m_fTurnOnDuration = fDuration;
    }
}

// FUNCTION: SWBF2 0x00689e30
void WeaponMelee::TurnOff(float fDuration)
{
    if (m_pMeleeThrow && m_pMeleeThrow->IsCurrentlyThrown())
    {
        return;
    }

    if (fDuration < 0.0f)
    {
        fDuration = static_cast<const WeaponMeleeClass*>(mClass)->m_fTurnOnDuration;
    }

    if (m_bLightsaberOn)
    {
        if (m_fTurnOnTimer < m_fTurnOnDuration)
        {
            m_fTurnOnTimer = (1.0f - m_fTurnOnTimer / m_fTurnOnDuration) * fDuration;
        }
        else
        {
            m_fTurnOnTimer = 0.0f;
        }
        m_bLightsaberOn = 0;
        m_fTurnOnDuration = fDuration;
    }
    else if (m_fTurnOnTimer < m_fTurnOnDuration && fDuration < m_fTurnOnDuration)
    {
        m_fTurnOnTimer = fDuration / m_fTurnOnDuration * m_fTurnOnTimer;
        m_fTurnOnDuration = fDuration;
    }
}

// FUNCTION: SWBF2 0x00689d20
float WeaponMelee::GetLightSaberLengthFactor() const
{
    if (m_bLightsaberOn)
    {
        if (m_fTurnOnTimer < m_fTurnOnDuration)
        {
            return m_fTurnOnTimer / m_fTurnOnDuration;
        }
        return 1.0f;
    }
    if (m_fTurnOnTimer < m_fTurnOnDuration)
    {
        return 1.0f - m_fTurnOnTimer / m_fTurnOnDuration;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x00689f00
bool WeaponMelee::IsOn(unsigned int uiBlade) const
{
    if (uiBlade >= static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumWeapons)
    {
        return false;
    }
    if (!IsOn())
    {
        return false;
    }
    unsigned int uiMask = 1u << uiBlade;
    if (m_uiLightsaberOnMask & uiMask)
    {
        return true;
    }
    if ((m_uiLightsaberWasOnMask & uiMask) && m_fComboStateTimer < 0.1f)
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00689f80
float WeaponMelee::GetLightSaberLengthFactor(unsigned int uiBlade) const
{
    if (uiBlade >= static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumWeapons)
    {
        return 0.0f;
    }
    float fOn = GetLightSaberLengthFactor();
    if (fOn <= 0.0f)
    {
        return 0.0f;
    }
    unsigned int uiMask = 1u << uiBlade;
    bool bOn = (m_uiLightsaberOnMask & uiMask) != 0;
    bool bWasOn = (m_uiLightsaberWasOnMask & uiMask) != 0;
    if (bOn == bWasOn)
    {
        return bOn * fOn;
    }
    if (bOn)
    {
        return m_fComboStateTimer < 0.1f ?
               m_fComboStateTimer / 0.1f * fOn : fOn;
    }
    return m_fComboStateTimer < 0.1f ?
           fOn - m_fComboStateTimer / 0.1f * fOn : 0.0f;
}

// FUNCTION: SWBF2 0x0068a410
void WeaponMelee::RenderLightsaberTrail(unsigned int uiEdge, const PblMatrix* pMatrix, bool bTrailOn,
                                        unsigned int uiFlags)
{
    WeaponMelee::TrailData** ppData = &m_pTrailData;
    WeaponMelee::TrailData* pSelected = 0;
    WeaponMelee::TrailData* pData = *ppData;
    while (pData)
    {
        if (pData->m_pTrail->m_uiNumSamples <= 0)
        {
            *ppData = pData->pNext;
            WeaponMelee::_FreeTrailData(pData);
        }
        else
        {
            if (pData->m_uiEdge == uiEdge)
            {
                pSelected = pData;
            }
            ppData = &pData->pNext;
        }
        pData = *ppData;
    }

    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mClass);
    const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[uiEdge];
    if (pSelected)
    {
        if (pSelected->m_uiFrameLastUpdated != (unsigned char)RedRenderer::GetFrameNumber())
        {
            pSelected->m_pTrail->m_bTrailOn = bTrailOn;
            pSelected->m_pTrail->AttachEffectToMatrix(pMatrix, false);
        }
    }
    else
    {
        if (!bTrailOn)
        {
            return;
        }
        pSelected = WeaponMelee::_AllocTrailData();
        if (!pSelected)
        {
            return;
        }
        const RedColor colorHead = Interpolate(RedColor::WHITE, edge.colorTrail, 0.5f);
        pSelected->Init(uiEdge, edge.vOffsetPos, edge.vOffsetDir,
                        edge.fLength, edge.fWidth, edge.colorTrail, colorHead);
        pSelected->pNext = m_pTrailData;
        m_pTrailData = pSelected;
        pSelected->m_pTrail->m_bTrailOn = bTrailOn;
        pSelected->m_pTrail->AttachEffectToMatrix(pMatrix, false);
    }
    if (pSelected->m_pTrail->m_uiNumSamples)
    {
        LightsaberTrailEffect* pEffect = LightsaberTrailEffect::s_pInstance;
        LightsaberTrail::s_uiSamplePrev = 0;
        LightsaberTrail::s_uiSample = 0;
        pEffect->RenderLightsaberTrail(pSelected->m_pTrail, uiFlags);
    }
}

// FUNCTION: SWBF2 0x0068a550
bool WeaponMelee::Deflect(const Ordnance* pOrdnance, const PblVector3& pos, const PblVector3& dir)
{
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    const Combo::Deflect* pDeflect = state.m_pDeflect;
    if (!pDeflect)
    {
        return false;
    }

    PblVector3 vVelocity = mOwner->GetGameObject()->GetVelocity();
    vVelocity.y = 0.0f;
    if (vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z > 0.09f)
    {
        PblVector3 vNormal;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNormal), reinterpret_cast<const D3DXVECTOR3*>(&vVelocity));
        vVelocity = vNormal;
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        if (DotProduct(vVelocity, mOwnerMatrix.GetZAxis()) <= 0.0f)
        {
            vVelocity = PblVector3(-vVelocity.x, -vVelocity.y, -vVelocity.z);
        }
    }
    else
    {
        vVelocity = mOwner->GetMatrix().GetZAxis();
    }

    unsigned char uiAnim;

    if (!pDeflect->TestDeflect(mOwner->GetMatrix(), vVelocity, dir, pos, m_uiAnimIndex, uiAnim))
    {
        m_uiAnimIndex = state.m_uiAnimIndex;
        return false;
    }

    const float fEnergyCost = pDeflect->m_uiEnergyCost * 0.1f;
    EnergyBar* pEnergyBar = mOwner->GetEnergyBar();
    if (pEnergyBar && !pEnergyBar->SpendEnergy(fEnergyCost))
    {
        m_uiAnimIndex = state.m_uiAnimIndex;
        return false;
    }
    m_uiAnimIndex = uiAnim;

    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mClass);
    PblVector3 vHitPos = pos;
    if (pClass->m_amDeflectMatrix)
    {
        PblVector3 vFront = mOwner->GetMatrix().GetZAxis();
        AlignVectorToThrust(vFront, false);
        const PblVector3 vRight = Normalize(CrossProduct(mOwner->GetMatrix().GetYAxis(), vFront));
        PblMatrix mDeflect(vRight, CrossProduct(vFront, vRight), vFront, mOwner->GetMatrix().GetTranslation());

        for (unsigned int i = 0; i < pDeflect->m_uiNumDeflectAnimations; ++i)
        {
            if (pDeflect->m_aDeflectAnim[i].uiAnimIndex != uiAnim)
            {
                continue;
            }

            PblMatrix mBlock = pClass->m_amDeflectMatrix[i] * mDeflect;
            const float dx = pos.x - mBlock.m[3][0];
            const float dy = pos.y - mBlock.m[3][1];
            const float dz = pos.z - mBlock.m[3][2];
            const unsigned char uiWeapon = pDeflect->m_aDeflectAnim[i].uiWeapon;
            PblVector3 vLocal(mBlock.m[0][1] * dy + dx * mBlock.m[0][0] + mBlock.m[0][2] * dz,
                              mBlock.m[1][0] * dx + mBlock.m[1][1] * dy + mBlock.m[1][2] * dz,
                              mBlock.m[2][0] * dx + mBlock.m[2][1] * dy + mBlock.m[2][2] * dz);
            int iEdge = -1;
            float fBestDistSq = FLT_MAX;
            for (unsigned int e = 0; e < pClass->m_uiNumDamageEdges; ++e)
            {
                const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[e];
                if (edge.uiAttachToWeapon != uiWeapon)
                {
                    continue;
                }
                const float fHalf = edge.fLength * 0.5f;
                const float cx = edge.vOffsetPos.x + edge.vOffsetDir.x * fHalf - vLocal.x;
                const float cy = edge.vOffsetPos.y + edge.vOffsetDir.y * fHalf - vLocal.y;
                const float cz = edge.vOffsetPos.z + edge.vOffsetDir.z * fHalf - vLocal.z;
                const float fDistSq = cy * cy + cx * cx + cz * cz;
                if (fDistSq < fBestDistSq)
                {
                    iEdge = e;
                    fBestDistSq = fDistSq;
                }
            }
            if (iEdge == -1)
            {
                vHitPos = mBlock.GetTranslation();
            }
            else
            {
                const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[iEdge];
                float t = ((edge.vOffsetDir.y * (vLocal.y - edge.vOffsetPos.y) +
                            edge.vOffsetDir.x * (vLocal.x - edge.vOffsetPos.x) +
                            edge.vOffsetDir.z * (vLocal.z - edge.vOffsetPos.z)) /
                               edge.fLength +
                           1.0f) *
                          0.5f;
                if (t > 1.0f)
                {
                    t = 1.0f;
                }
                else if (t < 0.0f)
                {
                    t = 0.0f;
                }
                const float fAlong = t * edge.fLength;
                PblVector3 vEdgePoint(edge.vOffsetDir.x * fAlong + edge.vOffsetPos.x,
                                      edge.vOffsetPos.y + edge.vOffsetDir.y * fAlong,
                                      edge.vOffsetPos.z + edge.vOffsetDir.z * fAlong);
                vHitPos = mBlock.ApplyTransform(vEdgePoint);
            }
            break;
        }
    }

    if (pOrdnance)
    {
        float fSpread = 0.05f;
        if (m_fDeflectTimer < 1.0f)
        {
            fSpread = (1.0f - m_fDeflectTimer) * 0.25f + m_fDeflectTimer * 0.05f;
        }
        else if (m_fComboStateTimer < 0.4f)
        {
            fSpread = 0.02f;
        }

        PblVector3 vDeflectDir;
        if (pOrdnance->IsRtti(OrdnanceBeam::rttiHashOrdnanceBeam))
        {

            PblRandom rand(((unsigned int)pOrdnance ^ (unsigned int)pOrdnance->GetOwner() ^
                          pOrdnance->PblHandled::mHandleId) ^ 0xC71C2830u);
            const float fPhase = rand.GetFloat();
            const float fScatter = rand.GetFloat();
            float fSin = 0.0f;
            float fCos = 1.0f;
            PblMath::SinCos(&fSin, &fCos,
                            ((fScatter * 2.0f - 1.0f) * m_fComboStateTimer + fPhase) * 6.2831855f);
            fCos *= fSpread;
            fSin *= fSpread;
            const PblVector3 vCross = CrossProduct(mAimer->mDirection, mOwner->GetMatrix().GetXAxis()) * fSin;
            const PblVector3 vScattered = mAimer->mDirection + mOwner->GetMatrix().GetXAxis() * fCos + vCross;
            vDeflectDir = Normalize(vScattered);

            m_fDeflectTimer -= GameLoop::sClientDeltaTime;
            if (m_fDeflectTimer >= 1.0f)
            {
                m_fDeflectTimer = 1.0f;
            }
            m_fDeflectTimer -= GameLoop::sClientDeltaTime * 2.0f;
            if (m_fDeflectTimer < 0.0f)
            {
                m_fDeflectTimer = 0.0f;
            }
        }
        else
        {
            const float fX = (PblRandom::_Global.GetFloat() * 2.0f - 1.0f) * fSpread;
            const float fY = (PblRandom::_Global.GetFloat() * 2.0f - 1.0f) * fSpread;
            const PblVector3 vCross = CrossProduct(mAimer->mDirection, mOwner->GetMatrix().GetXAxis()) * fY;
            const PblVector3 vScattered = mAimer->mDirection + mOwner->GetMatrix().GetXAxis() * fX + vCross;
            vDeflectDir = Normalize(vScattered);
            m_fDeflectTimer = 0.0f;
        }
        pOrdnance->CreateRicochet(vHitPos, vDeflectDir, mOwner->GetGameObject());
    }
    else
    {
        m_fDeflectTimer = 0.0f;
    }

    TurnOn(0.1f);
    GameObject* pObject = mOwner->GetGameObject();
    static_cast<const WeaponMeleeClass*>(mClass)->m_DeflectSound.Play(&mOwner->GetGameObject()->GetPosition(),
                                                                      &pObject->GetVelocity(), false, true);
    FLEffectClass* pEffectClass =
        FLEffect::s_EffectClasses.Find(static_cast<const WeaponMeleeClass*>(mClass)->m_uiDeflectEffect);
    if (pEffectClass)
    {
        FLEffectObject* pEffect = pEffectClass->BuildEffect();
        if (pEffect)
        {
            pEffect->AttachEffectToVector(&vHitPos, false);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0068b850
int WeaponMelee::_ClearInputQueueToTime(WeaponMelee::InputQueue& queue, float fTime)
{
    if (queue.GetCount() < 1 || queue[0].fTime > fTime)
    {
        return 0;
    }
    int iConsumed = 1;
    while (iConsumed < queue.GetCount() &&
           !(queue[iConsumed].fTime > fTime))
    {
        ++iConsumed;
    }
    queue.Pop(iConsumed);
    return iConsumed;
}

// FUNCTION: SWBF2 0x0068b8d0
bool WeaponMelee::TestInputQueueForButtons(unsigned int iPressedMask, unsigned int iReleasedMask) const
{
    if (!iPressedMask && !iReleasedMask)
    {
        return true;
    }
    for (int i = 0; i < m_InputQueue.GetCount(); ++i)
    {
        int iButton = 1 << m_InputQueue[i].uiButton;
        if ((iButton & iPressedMask) && !(m_InputQueue[i].uiButtonAction & 3))
        {
            return true;
        }
        if ((iButton & iReleasedMask) && (m_InputQueue[i].uiButtonAction & 1))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068b400
void WeaponMelee::InputQueue::Update(unsigned char uiButtons, float fTime)
{
    for (unsigned char uiButton = 1; uiButton < 8; ++uiButton)
    {
        unsigned char uiMask = 1 << uiButton;
        unsigned char uiAction = 0xff;
        if (uiButtons & uiMask)
        {
            if (!(m_uiButtonDown & uiMask))
            {
                // pressed: a quick press after a tap is a double tap
                uiAction = 0;
                for (unsigned char i = 0; i < m_uiCount; ++i)
                {
                    unsigned char uiAge = m_uiCount - i - 1;
                    const Event& event = m_aEventQueue[(m_uiStart + uiAge) & 7];
                    if (event.uiButton == uiButton)
                    {
                        if ((event.uiButtonAction & 3) == 1 && event.fTime + 0.25f >= fTime)
                        {
                            uiAction = 4;
                        }
                        break;
                    }
                }
                m_uiButtonDown |= uiMask;
            }
            else
            {
                // still held: mark the last press as held once it has lasted long enough
                uiAction = 2;
                for (unsigned char i = 0; i < m_uiCount; ++i)
                {
                    unsigned char uiAge = m_uiCount - i - 1;
                    const Event& event = m_aEventQueue[(m_uiStart + uiAge) & 7];
                    if (event.uiButton == uiButton)
                    {
                        unsigned char uiOld = event.uiButtonAction;
                        if (!(uiOld & 1))
                        {
                            if (!(uiOld & 2) && fTime > event.fTime + 0.3f)
                            {
                                uiAction = uiOld | 2;
                            }
                            else
                            {
                                uiAction = 0xff;
                            }
                        }
                        break;
                    }
                }
            }
        }
        else if (m_uiButtonDown & uiMask)
        {
            // released
            for (unsigned char i = 0; i < m_uiCount; ++i)
            {
                unsigned char uiAge = m_uiCount - i - 1;
                const Event& event = m_aEventQueue[(m_uiStart + uiAge) & 7];
                if (event.uiButton == uiButton)
                {
                    unsigned char uiOld = event.uiButtonAction;
                    if (!(uiOld & 1))
                    {
                        if ((uiOld & 2) || fTime > event.fTime + 0.3f)
                        {
                            uiAction = uiOld | 3;
                        }
                        else
                        {
                            uiAction = uiOld | 1;
                        }
                    }
                    break;
                }
            }
            m_uiButtonDown &= ~uiMask;
        }
        if (uiAction != 0xff)
        {
            unsigned char uiIndex = (m_uiStart + m_uiCount) & 7;
            Event& event = m_aEventQueue[uiIndex];
            event.fTime = fTime;
            event.uiButton = uiButton;
            event.uiButtonAction = uiAction;
            if (m_uiCount < 8)
            {
                ++m_uiCount;
            }
            else
            {
                ++m_uiStart;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0068b5f0
bool WeaponMelee::_TestInputQueueForButtonEvent(const WeaponMelee::InputQueue& queue, float fTime,
                unsigned int iButton, unsigned int iAction, float fBegin, float fEnd, float& fMatch)
{
    if (iAction == 10)
    {
        int iPressed = -1;
        bool bReleased = false;
        for (int i = 0; i < queue.GetCount(); ++i)
        {
            const InputQueue::Event& event = queue[i];
            if (event.uiButton == iButton)
            {
                const float fEventTime = event.fTime;
                if (fEventTime > fEnd)
                {
                    break;
                }
                if (fBegin > fEventTime)
                {
                    iPressed = (event.uiButtonAction & 1) ? -1 : i;
                }
                else
                {
                    if (event.uiButtonAction & 1)
                    {
                        bReleased = true;
                        break;
                    }
                    fMatch = fEventTime;
                    return true;
                }
            }
        }
        if (iPressed >= 0 || bReleased)
        {
            fMatch = fBegin;
            return true;
        }
        if (fBegin <= fTime && fTime <= fEnd && queue.IsButtonDown(static_cast<unsigned char>(iButton)))
        {
            fMatch = fBegin;
            return true;
        }
    }
    else if (iAction == 8)
    {
        int iReleased = -1;
        bool bPressed = false;
        for (int i = 0; i < queue.GetCount(); ++i)
        {
            const InputQueue::Event& event = queue[i];
            if (event.uiButton == iButton)
            {
                const float fEventTime = event.fTime;
                if (fEventTime > fEnd)
                {
                    break;
                }
                if (fBegin > fEventTime)
                {
                    iReleased = (event.uiButtonAction & 1) ? i : -1;
                }
                else
                {
                    if (event.uiButtonAction & 1)
                    {
                        fMatch = fEventTime;
                        return true;
                    }
                    if (!(event.uiButtonAction & 2))
                    {
                        bPressed = true;
                        break;
                    }
                }
            }
        }
        if (iReleased >= 0 || bPressed)
        {
            fMatch = fBegin;
            return true;
        }
        if (fBegin <= fTime && fTime <= fEnd && !queue.IsButtonDown(static_cast<unsigned char>(iButton)))
        {
            fMatch = fBegin;
            return true;
        }
    }
    else
    {
        unsigned int uiRequired = 0;
        unsigned int uiMask = 0;
        if (iAction == 9)
        {
            uiRequired = uiMask = 1;
        }
        else
        {
            if (iAction & 4)
            {
                uiRequired = uiMask = 4;
            }
            uiMask |= 2;
            if (iAction & 2)
            {
                uiRequired |= 2;
            }
            if (iAction & 1)
            {
                uiMask |= 1;
                uiRequired |= 1;
            }
        }
        for (int i = 0; i < queue.GetCount(); ++i)
        {
            const InputQueue::Event& event = queue[i];
            if (event.uiButton == iButton && (event.uiButtonAction & uiMask) == uiRequired)
            {
                const float fEventTime = event.fTime;
                if (fEventTime > fEnd)
                {
                    return false;
                }
                if (fBegin <= fEventTime)
                {
                    fMatch = fEventTime;
                    return true;
                }
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068b980
bool WeaponMelee::TestConditionExceptInputQueue(float fPreviousTime, float fCurrentTime,
                const Combo::Condition* pCondition)
{
    float fStart = pCondition->m_iTimeStart * Combo::Condition::TIME_SCALE;
    if (pCondition->m_bTimeStartFromEnd)
    {
        fStart += fPreviousTime;
    }
    if (!(fStart > fCurrentTime))
    {
        float fEnd = pCondition->m_iTimeEnd * Combo::Condition::TIME_SCALE;
        if (pCondition->m_bTimeEndFromEnd)
        {
            fEnd += fPreviousTime;
        }
        if (!(fCurrentTime > fEnd))
        {
            if (pCondition->TestSoldierState(m_iSoldierState, m_fSoldierStateTimer))
            {
                EnergyBar* pEnergy = mOwner->GetEnergyBar();
                const float fEnergy = pEnergy ? pEnergy->GetEnergy() : 0.0f;
                if (pCondition->TestEnergy(fEnergy))
                {
                    if (pCondition->TestInputThrust(m_fInputThrustZ, -m_fInputThrustX))
                    {
                        if (pCondition->TestVelocity(m_vVelocityLocal.x,
                                                     m_vVelocityLocal.y,
                                                     m_vVelocityLocal.z))
                        {
                            if (!pCondition->m_uiInputButtonCnd ||
                                TestInputQueueForButtons(pCondition->m_uiInputButtonsPressed,
                                                         pCondition->m_uiInputButtonsReleased))
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068baa0
bool WeaponMelee::TestCondition(float fPreviousTime, float fCurrentTime,
                const Combo::Condition* pCondition, float* pfMatchTime)
{
    if (!TestConditionExceptInputQueue(fPreviousTime, fCurrentTime, pCondition))
    {
        return false;
    }

    const float fOffset = m_fInputTimer - fCurrentTime;
    if (pCondition->m_uiButton)
    {
        float fEnd = pCondition->m_iTimeEnd * Combo::Condition::TIME_SCALE;
        if (pCondition->m_bTimeEndFromEnd)
        {
            fEnd += fPreviousTime;
        }
        const float fStart = pCondition->m_bTimeStartFromEnd
                                 ? pCondition->m_iTimeStart * Combo::Condition::TIME_SCALE + fPreviousTime
                                 : pCondition->m_iTimeStart * Combo::Condition::TIME_SCALE;
        if (!WeaponMelee::_TestInputQueueForButtonEvent(m_InputQueue, m_fInputTimer,
                         pCondition->m_uiButton, pCondition->m_uiButtonAction,
                         fStart + fOffset, fEnd + fOffset, *pfMatchTime))
        {
            return false;
        }
        *pfMatchTime -= fOffset;
    }
    else
    {
        const float fStart = pCondition->m_bTimeStartFromEnd
                                 ? pCondition->m_iTimeStart * Combo::Condition::TIME_SCALE + fPreviousTime
                                 : pCondition->m_iTimeStart * Combo::Condition::TIME_SCALE;
        *pfMatchTime = fStart;
    }
    return true;
}

// FUNCTION: SWBF2 0x00689cb0
bool WeaponMelee::IsBusy() const
{
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    if (state.m_uiInputLock & 4)
    {
        float fLockTime = state.m_uiInputLockDuration * 0.01f;
        if (fLockTime <= 0.0f || fLockTime > m_fComboStateTimer)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068b140
bool WeaponMelee::AlignVectorToThrust(PblVector3& v, bool bInitial) const
{
    float fX = m_fAlignX;
    float fZ = m_fAlignZ;
    float fLengthSquared = fX * fX + fZ * fZ;
    if (fLengthSquared < 0.09f)
    {
        return false;
    }
    const PblVector3 vOld = v;
    if (!bInitial)
    {
        fX = -fX;
    }
    if (m_fInputThrustZ < 0.0f)
    {
        fX = -fX;
        fZ = -fZ;
    }

    float fCos, fSin;
    if (fX >= fZ)
    {
        fCos = 0.70709997f;
        fSin = 0.70709997f;
    }
    else if (-fX >= fZ)
    {
        fCos = 0.70709997f;
        fSin = -0.70709997f;
    }
    else
    {
        float fInvLength = 1.0f / static_cast<float>(sqrt(static_cast<double>(fLengthSquared)));
        fCos = fInvLength * fZ;
        fSin = fInvLength * fX;
    }
    v.x = vOld.x * fCos + vOld.z * fSin;
    v.z = vOld.z * fCos - vOld.x * fSin;
    return true;
}

// FUNCTION: SWBF2 0x0068b2c0
Trigger* WeaponMelee::GetButton(unsigned char uiButton)
{
    switch (uiButton)
    {
    case 1: return &mOwner->mControlFire[0];
    case 2: return &mOwner->mControlFire[1];
    case 3: return &mOwner->mControlJump;
    case 4: return &mOwner->mControlSprint;
    case 5: return &mOwner->mControlCrouch;
    case 6: return &mOwner->mControlReload;
    default: return 0;
    }
}

// FUNCTION: SWBF2 0x0068b330
void WeaponMelee::ClearInput()
{
    m_fInputThrustX = 0.0f;
    m_fInputThrustZ = 0.0f;
    m_fInputTimer = 0.0f;
    m_uiButtonDown = 0;
    m_InputQueue.ClearAll();
}

// FUNCTION: SWBF2 0x0068b370
void WeaponMelee::ReadButtons()
{
    if (m_bSoldierInputLock)
    {
        m_uiButtonDown = 0;
        return;
    }
    for (unsigned char uiButton = 1; uiButton < 8; ++uiButton)
    {
        unsigned char uiMask = 1 << uiButton;
        bool bDown;
        if (m_bAI_Deflect && uiButton == m_pCombo->m_uiDeflectButton)
        {
            bDown = true;
        }
        else if (uiButton == 7)
        {
            bDown = (m_uiButtonDown & 6) == 6;
        }
        else
        {
            bDown = (GetButton(uiButton)->m_uiButtonState & 1) != 0;
        }
        if (bDown)
        {
            m_uiButtonDown |= uiMask;
        }
        else
        {
            m_uiButtonDown &= ~uiMask;
        }
    }
}

// FUNCTION: SWBF2 0x0068bc00
void WeaponMelee::ReleaseDamageData()
{
    while (m_pDamageData)
    {
        DamageData* pData = m_pDamageData;
        m_pDamageData = pData->pNext;
        FreeDamageData(pData);
    }
}

// FUNCTION: SWBF2 0x0068b100
void WeaponMelee::SetNetworkState(ComboState eState, bool bAnimatedMove)
{
    if (eState != m_eComboState)
    {
        ExitState();
        EnterState(eState);
    }
    m_bAnimatedMove = bAnimatedMove;
}

// FUNCTION: SWBF2 0x0068bc30
void WeaponMelee::EnterState(ComboState eState)
{
    m_eComboState = eState;
    m_fComboStateTimer = 0.0f;
    const Combo::State& state = m_pCombo->m_aState[eState];
    m_uiAnimIndex = state.m_uiAnimIndex;
    m_bAnimatedMove = state.m_bAnimatedMove;
    m_bGravityDone = 0;
    m_uiLightsaberWasOnMask = m_uiLightsaberOnMask;
    m_bUnreachableState_WeaponUpdate = 0;
    m_bUnreachableState_SoldierUpdate = 0;
    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mClass);
    const unsigned int uiBladeMask = (1u << pClass->m_uiNumWeapons) - 1;
    m_uiLightsaberOnMask = (state.m_uiLightsaberOffMask ^ uiBladeMask) & uiBladeMask;
    m_bRestartAnimation = state.m_bRestartAnimation;

    if (state.m_bPlayExplosion != 0 && pClass->mImpactExplosion &&
        (!(netInShell ? netEnabled : netEnabledNext) || netIsLocalTurn))
    {
        DamageOwner owner(mOwner->GetCharacter(), mOwner->GetGameObject(),
                          mOwner->GetGameObject()->GetTeam(), 0, mClass, 1.0f);
        PblVector3 vPos = mOwner->GetMatrix().GetTranslation() + PblVector3(0.0f, 0.2f, 0.0f);
        pClass->mImpactExplosion->CreateExplosion(vPos, owner, false, COLL_UNKNOWN);
    }

    if (state.m_sound.GetProperties() && state.m_uiSoundTime * 0.01f == 0.0f)
    {
        state.m_sound.Play(&mOwner->GetGameObject()->GetPosition(),
                           &mOwner->GetGameObject()->GetVelocity(), false, true);
    }

    if (m_eComboState == COMBO_IDLE)
    {
        m_InputQueue.Clear();
        mState = (WeaponState)MELEE_IDLE;
        EnterIdle();
    }
    else if (state.m_uiNumAttacks)
    {
        mState = (WeaponState)MELEE_ATTACK;
        if (state.m_pDeflect)
        {
            EnterDeflect();
        }
        EnterFire();
    }
    else if (state.m_pDeflect)
    {
        mState = (WeaponState)MELEE_DEFLECT;
        EnterDeflect();
    }
    else
    {
        mState = (WeaponState)MELEE_RECOVER;
        EnterRecover();
    }
}

// FUNCTION: SWBF2 0x0068bfa0
bool WeaponMelee::UpdateState(float fDeltaTime)
{
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    if (state.m_sound.GetProperties())
    {
        float fSoundTime = state.m_uiSoundTime * 0.01f;
        if (fSoundTime > m_fComboStateTimer - fDeltaTime &&
            m_fComboStateTimer >= fSoundTime)
        {
            state.m_sound.Play(&mOwner->GetGameObject()->GetPosition(),
                               &mOwner->GetGameObject()->GetVelocity(), false, true);
        }
    }

    bool bResult;
    switch (GetMeleeState())
    {
    case MELEE_IDLE:
        bResult = UpdateIdle(fDeltaTime);
        break;
    case MELEE_ATTACK:
        if (state.m_pDeflect)
        {
            UpdateDeflect(fDeltaTime);
            mStateTimer -= fDeltaTime;
        }
        bResult = UpdateFire(fDeltaTime);
        break;
    case MELEE_RECOVER:
        bResult = UpdateRecover(fDeltaTime);
        break;
    case MELEE_DEFLECT:
        bResult = UpdateDeflect(fDeltaTime);
        break;
    default:
        bResult = UpdateIdle(fDeltaTime);
        break;
    }

    DamageData** ppData = &m_pDamageData;
    for (DamageData* pData = *ppData; pData; pData = *ppData)
    {
        if (m_fComboStateTimer >= pData->fTime + 0.3f)
        {
            *ppData = pData->pNext;
            FreeDamageData(pData);
        }
        else
        {
            ppData = &pData->pNext;
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x0068bef0
void WeaponMelee::ExitState()
{
    switch (GetMeleeState())
    {
    case MELEE_IDLE:
        ExitIdle();
        break;
    case MELEE_ATTACK:
        ExitFire();
        if (m_pCombo->m_aState[m_eComboState].m_pDeflect)
        {
            ExitDeflect();
        }
        break;
    case MELEE_RECOVER:
        ExitRecover();
        break;
    case MELEE_DEFLECT:
        ExitDeflect();
        break;
    default:
        ExitIdle();
        break;
    }
    for (DamageData* pData = m_pDamageData; pData; pData = pData->pNext)
    {
        pData->fTime -= m_fComboStateTimer;
    }
}

// FUNCTION: SWBF2 0x0068c140
void WeaponMelee::EnterIdle()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068c150
void WeaponMelee::ExitIdle()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068c160
bool WeaponMelee::UpdateIdle(float fDeltaTime)
{
    bool bDeactivate = mDeactivateScheduled != 0;
    mStateTimer += fDeltaTime;
    if (bDeactivate)
    {
        Thread::DeactivateThread();
    }
    return true;
}

// FUNCTION: SWBF2 0x0068c210
void WeaponMelee::ExitFire()
{
    m_uiTrailOnMask = 0;
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068c230
bool WeaponMelee::UpdateFire(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    const Combo::State& state = *m_pCombo->GetState(m_eComboState);
    if (!state.GetNumAttacks())
    {
        return true;
    }
    const unsigned int uiNumEdges = static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumDamageEdges;
    if (!uiNumEdges)
    {
        return true;
    }

    m_uiTrailOnMask = 0;
    for (unsigned int uiEdge = 0; uiEdge < uiNumEdges; ++uiEdge)
    {
        float fStart = FLT_MAX;
        float fEnd = -FLT_MAX;
        for (unsigned int a = 0; a < state.GetNumAttacks(); ++a)
        {
            const Combo::Attack& attack = *state.GetAttack(a);
            if (attack.GetDamageEdge() != uiEdge)
            {
                continue;
            }

            const float fAttackStart = attack.GetDamageTimeStart() - 0.0f;
            const float fAttackEnd = attack.IsDamageTimeEndFromAnim()
                                         ? m_fComboStateTimer
                                         : attack.GetDamageTimeEnd() + 0.05f;
            if (fAttackStart < fStart)
            {
                fStart = fAttackStart;
            }
            if (fAttackEnd > fEnd)
            {
                fEnd = fAttackEnd;
            }
            if (m_fComboStateTimer >= fStart && fEnd >= m_fComboStateTimer)
            {
                m_uiTrailOnMask |= 1 << uiEdge;
                break;
            }
        }
    }

    const float fPrevTime = m_fComboStateTimer - fDeltaTime;
    bool bMatrixReady = false;
    bool bHit = false;
    PblMatrix mAttack;
    for (unsigned int a = 0; a < state.GetNumAttacks(); ++a)
    {
        const Combo::Attack& attack = *state.GetAttack(a);

        const float fDamage = attack.GetDamage();
        const float fPush = attack.IsPush() ? attack.GetPushStrength() : 0.0f;
        const float fStart = attack.GetDamageTimeStart();
        const float fEnd = attack.IsDamageTimeEndFromAnim() ? FLT_MAX : attack.GetDamageTimeEnd();
        if (fEnd < fPrevTime || m_fComboStateTimer <= fStart)
        {
            continue;
        }
        const unsigned int uiEdge = attack.GetDamageEdge();
        if (uiEdge >= uiNumEdges)
        {
            continue;
        }

        DamageData* pData;
        for (pData = m_pDamageData; pData; pData = pData->pNext)
        {
            const Combo::Attack* pOther = static_cast<const Combo::Attack*>(pData->pAttack);
            if (pOther == &attack)
            {
                break;
            }
            if (attack.GetAttackId() && attack.GetAttackId() == pOther->GetAttackId())
            {
                break;
            }
        }
        if (!pData)
        {
            pData = AllocDamageData();
            if (!pData)
            {
                continue;
            }
            pData->fTime = fEnd;
            pData->pAttack = &attack;
            pData->iNumObjects = 0;
            pData->pNext = m_pDamageData;
            m_pDamageData = pData;
        }

        const WeaponMeleeClass::DamageEdge& edge = static_cast<const WeaponMeleeClass*>(mClass)->m_aDamageEdge[uiEdge];
        float fLength = attack.GetDamageLength();
        float fWidth = attack.GetDamageWidth();
        if (attack.IsDamageLengthFromEdge())
        {
            fLength = edge.fLength * fLength;
        }
        if (attack.IsDamageWidthFromEdge())
        {
            fWidth = edge.fWidth * fWidth;
        }
        const float fRayLength = fLength + 0.2f;

        const unsigned int uiNumSamples = attack.GetNumDamageSamples();
        int iFirst = -1;
        PblBox box;
        unsigned int s;
        for (s = 0; s < uiNumSamples; ++s)
        {
            const Combo::DamageSample& sample = *attack.GetDamageSample(s);
            if (fPrevTime >= sample.fTime && s < uiNumSamples - 1)
            {
                continue;
            }
            if (sample.fTime > m_fComboStateTimer)
            {
                break;
            }
            PblVector3 vStart = sample.vPos - sample.vDir * 0.2f;
            if (iFirst == -1)
            {
                iFirst = s;
                box.Set(vStart, vStart);
            }
            else
            {
                box.AddPoint(vStart);
            }
            PblVector3 vEnd = sample.vDir * fRayLength + vStart;
            box.AddPoint(vEnd);
        }
        const unsigned int uiEnd = s;
        if (iFirst == -1)
        {
            continue;
        }

        if (!bMatrixReady)
        {
            if (state.IsAlignedToThrust())
            {
                const PblVector3 vAimX =
                    Normalize(CrossProduct(mOwner->GetMatrix().GetYAxis(), mAimer->GetDirection()));
                mAttack = PblMatrix(vAimX, CrossProduct(mAimer->GetDirection(), vAimX), mAimer->GetDirection(),
                                    mOwner->GetMatrix().GetTranslation());
            }
            else
            {
                mAttack = mOwner->GetMatrix();
            }
            bMatrixReady = true;
        }

        PblVector3 vCenter = mAttack.ApplyTransform((box.GetMax() + box.GetMin()) * 0.5f);
        const float fRadius = Length(box.GetMax() - box.GetMin()) * 0.5f;
        GameObject* apObjects[8];
        const int iNumObjects =
            CollisionManager::GetObjectsInRange(vCenter, fRadius, apObjects, 8, mOwner->GetGameObject(), 0x3a);

        FLEffectClass* pHitEffectClass = 0;
        GameObject* pOwnerObject = mOwner->GetGameObject();
        DamageDesc damage(DamageOwner(mOwner->GetCharacter(), mOwner->GetGameObject(),
                                      mOwner->GetGameObject()->GetTeam(), pOwnerObject->GetEntityClass(),
                                      mClass, 1.0f),
                          fDamage);
        for (int o = 0; o < iNumObjects; ++o)
        {
            GameObject* pObject = apObjects[o];
            CollisionObject* pCollision = pObject->GetCollisionObject();
            if (!pCollision)
            {
                continue;
            }
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
                continue;
            }

            PblVector3 vStart, vDir;
            float t = 1.0f;
            unsigned int i;
            for (i = iFirst; i < uiEnd; ++i)
            {
                const Combo::DamageSample& sample = *attack.GetDamageSample(i);
                vDir = mAttack.ApplyRotation(sample.vDir);
                vStart = mAttack.ApplyTransform(sample.vPos) - vDir * 0.2f;
                t = pCollision->RayHit(vStart, vDir, fRayLength, 0, 0, true, fWidth);
                if (t < 1.0f)
                {
                    break;
                }
            }
            if (i >= uiEnd || pData->iNumObjects >= 8)
            {
                continue;
            }

            pData->apObject[pData->iNumObjects++] = pObject;
            const float fAlong = t * fRayLength;
            PblVector3 vHit = vStart + vDir * fAlong;
            PblVector3 vSwing = mAttack.GetZAxis();

            if (pObject->Deflect(0, vSwing, vHit))
            {
                continue;
            }
            static_cast<Damageable*>(pObject)->ApplyDamage(damage, vSwing, vHit, -1, 0xffffffff);
            if (fPush > 0.0f)
            {
                pObject->ApplyPush(vSwing, fPush, false);
            }

            if (static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumHitEffects && !pHitEffectClass)
            {
                unsigned int uiEffect;
                if (gAltMeleeHitEffects && !(netInShell ? netEnabled : netEnabledNext) &&
                    static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumHitEffects != 1)
                {
                    uiEffect = static_cast<const WeaponMeleeClass*>(mClass)->m_uiHitEffects[
                        1 + PblRandom::GetGlobalInt() % (static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumHitEffects - 1)];
                }
                else
                {
                    uiEffect = static_cast<const WeaponMeleeClass*>(mClass)->m_uiHitEffects[0];
                }
                if (uiEffect)
                {
                    pHitEffectClass = FLEffect::s_EffectClasses.Find(uiEffect);
                }
            }
            if (pHitEffectClass)
            {
                FLEffectObject* pEffect = pHitEffectClass->BuildEffect();
                if (pEffect)
                {
                    pEffect->AttachEffectToVector(&vHit, false);
                }
            }
            bHit = true;
        }
    }

    if (bHit)
    {
        GameObject* pObject = mOwner->GetGameObject();
        static_cast<const WeaponMeleeClass*>(mClass)->m_HitSound.Play(&mOwner->GetGameObject()->GetPosition(),
                                                                      &pObject->GetVelocity(), false, true);
    }
    return true;
}

// FUNCTION: SWBF2 0x0068c190
void WeaponMelee::EnterFire()
{
    m_uiTrailOnMask = 0;
    mStateTimer = 0.0f;
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    SignalFire();
    float fDuration = static_cast<const WeaponMeleeClass*>(mClass)->m_fTurnOnDuration;
    if (state.m_uiNumAttacks)
    {
        float fStart = state.m_aAttack[0].m_uiDamageTimeStart * 0.01f;
        if (fStart < fDuration)
        {
            fDuration = fStart;
        }
    }
    TurnOn(fDuration);
}

// FUNCTION: SWBF2 0x0068d180
void WeaponMelee::EnterRecover()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068d190
void WeaponMelee::ExitRecover()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068d1a0
bool WeaponMelee::UpdateRecover(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    return true;
}

// FUNCTION: SWBF2 0x0068d1c0
void WeaponMelee::EnterDeflect()
{
    mStateTimer = 0.0f;
    m_fDeflectTimer = 1.0f;
    TurnOn(-1.0f);
}

// FUNCTION: SWBF2 0x0068d1f0
void WeaponMelee::ExitDeflect()
{
    mStateTimer = 0.0f;
    m_fDeflectTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068d210
bool WeaponMelee::UpdateDeflect(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    m_fDeflectTimer += fDeltaTime;
    unsigned char uiAnim = state.m_uiAnimIndex;
    if (m_uiAnimIndex != uiAnim && m_fDeflectTimer > 1.0f)
    {
        m_uiAnimIndex = uiAnim;
    }
    return true;
}

// FUNCTION: SWBF2 0x0068ebb0
void WeaponMeleeClass::GetDamageEdgeRay(unsigned int uiEdge, unsigned int& uiAttach,
                                        PblVector3& vBegin, PblVector3& vEnd) const
{
    uiAttach = m_auiHardpoint[m_aDamageEdge[uiEdge].uiAttachToWeapon];
    vBegin = m_aDamageEdge[uiEdge].vOffsetPos;
    vEnd = m_aDamageEdge[uiEdge].vOffsetDir;
}

// FUNCTION: SWBF2 0x0068ecb0
Combo* WeaponMeleeClass::_CreateODFCombo(unsigned int  )
{
    const unsigned int uiComboId = s_uiODFComboId;
    if (!m_pCombo)
    {
        m_pCombo = const_cast<Combo*>(Combo::Find(uiComboId));
        if (!m_pCombo)
        {
            if (Combo::sMemoryPool.IsFull())
            {
                return 0;
            }
            m_pComboODF = new Combo;
            m_pComboODF->InitDefault(uiComboId);
            m_pCombo = m_pComboODF;
            return m_pComboODF;
        }
        m_pComboODF = 0;
    }
    return m_pComboODF;
}

// FUNCTION: SWBF2 0x0068e830
Weapon* WeaponMeleeClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    WeaponMelee* pWeapon = new WeaponMelee(this, desc);
    if (!m_amDeflectMatrix)
    {
        m_eDeflectMatrixMap = pWeapon->mSoldierAnimationMap;
        _InitDeflectMatrices(m_eDeflectMatrixMap);
    }
    return pWeapon;
}

// FUNCTION: SWBF2 0x0068f150
void WeaponMeleeClass::_InitDeflectMatrices(int eMap)
{
    if (!m_pCombo)
    {
        return;
    }
    const Combo::Deflect* pDeflect = 0;
    for (unsigned int i = 0; i < m_pCombo->m_uiNumStates && !pDeflect; ++i)
    {
        pDeflect = m_pCombo->m_aState[i].m_pDeflect;
    }
    if (!pDeflect || !pDeflect->m_uiNumDeflectAnimations)
    {
        return;
    }

    unsigned int uiCount = pDeflect->m_uiNumDeflectAnimations;
    unsigned int uiSize = uiCount > 0x03ffffffu ? 0xffffffffu : uiCount * sizeof(PblMatrix);
    char* pRaw = uiSize > 0xffffffefu ? 0 : new char[uiSize + 16];
    if (pRaw)
    {
        unsigned char uiOffset = 16 - (reinterpret_cast<unsigned int>(pRaw) & 15);
        pRaw += uiOffset;
        pRaw[-1] = uiOffset;
    }
    m_amDeflectMatrix = reinterpret_cast<PblMatrix*>(pRaw);
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        const Combo::Deflect::DEFLECT_ANIM& anim = pDeflect->m_aDeflectAnim[i];
        unsigned int uiHardpoint = m_auiHardpoint[0];
        unsigned char uiWeapon = anim.uiWeapon;
        if (uiWeapon && uiWeapon < m_uiNumWeapons)
        {
            uiHardpoint = m_auiHardpoint[uiWeapon];
        }
        SoldierAnimatorClass::sInstance->GetAnimationBoneMatrix(
            static_cast<SoldierAnimationBank::MAP>(eMap), anim.uiAnimIndex,
            uiHardpoint, 0.1f, m_amDeflectMatrix[i]);
    }
}

// GLOBAL: SWBF2 0x0078c120
DefaultCombo::DeflectAnimation aDeflectAnimation[DefaultCombo::NUM_DEFLECT_ANIMATIONS] = {
    { "stand_block_front1", 1 },
    { "stand_block_front2", 1 },
    { "stand_block_right1", 2 },
    { "stand_block_right2", 2 },
    { "stand_block_left1", 8 },
    { "stand_block_left2", 8 },
    { "stand_block_rear1", 32 },
    { "stand_block_rear2", 32 },
};

// SYNTHETIC: SWBF2 0x00407200
// `dynamic initializer for 'WeaponMelee::rttiHashWeaponMelee''
// WeaponMelee::rttiHashWeaponMelee

// GLOBAL: SWBF2 0x01f9a8ec
// WeaponMelee::rttiHashWeaponMelee
PblHash WeaponMelee::rttiHashWeaponMelee("WeaponMelee");
// GLOBAL: SWBF2 0x0078c198
// WeaponMelee::rttiNameWeaponMelee
const char WeaponMelee::rttiNameWeaponMelee[] = "WeaponMelee";

// SYNTHETIC: SWBF2 0x00407210
// `dynamic initializer for 'WeaponMeleeClass::rttiHashWeaponMeleeClass''
// WeaponMeleeClass::rttiHashWeaponMeleeClass

// GLOBAL: SWBF2 0x01f9a8f4
// WeaponMeleeClass::rttiHashWeaponMeleeClass
PblHash WeaponMeleeClass::rttiHashWeaponMeleeClass("WeaponMeleeClass");
// GLOBAL: SWBF2 0x0078c0e0
// WeaponMeleeClass::rttiNameWeaponMeleeClass
const char WeaponMeleeClass::rttiNameWeaponMeleeClass[] = "WeaponMeleeClass";

// GLOBAL: SWBF2 0x01eafd70
WeaponMelee::TrailData* WeaponMelee::s_aTrailDataPool;
// GLOBAL: SWBF2 0x01eafd68
WeaponMelee::TrailData* WeaponMelee::s_pTrailDataFreeList;
// GLOBAL: SWBF2 0x01eafd6c
WeaponMelee::DamageData* WeaponMelee::s_aDamageDataPool;
// GLOBAL: SWBF2 0x01eafd64
WeaponMelee::DamageData* WeaponMelee::s_pDamageDataFreeList;







// FUNCTION: SWBF2 0x00687c40
bool WeaponMelee::IsMelee() const
{
    return true;
}

// FUNCTION: SWBF2 0x00687c50
bool WeaponMelee::SoldierCanOperate() const
{
    return true;
}

// SYNTHETIC: SWBF2 0x00687c60
// WeaponMelee::`scalar deleting destructor'










// FUNCTION: SWBF2 0x00687cf0
FLEffectObject* LightsaberTrailClass::BuildEffect()
{
    return new LightsaberTrail(this);
}


// FUNCTION: SWBF2 0x00687d80
void Combo::InitDefault(unsigned int uiNameHash)
{
    m_uiNameHash = uiNameHash;
    m_eMap = -1;
    m_eCombo = SoldierAnimationBank::AddCombo(uiNameHash);
    m_pSkeleton = 0;
    if (m_eCombo == SoldierAnimationBank::INVALID_COMBO)
    {
        return;
    }

    s_ComboTable.Store(uiNameHash, this);

    m_aState = 0;
    m_uiNumStates = 0;
    State* pState = AddState(PblHash(DefaultCombo::aszStateName[DefaultCombo::IDLE]));
    if (pState)
    {
        pState->SetInputLock(0x38, 0.15f);
        pState->SetEnergyRestoreRate(0.0f, true);
    }
}

// FUNCTION: SWBF2 0x00687e10
void Combo::AddDefaultAttackState()
{
    unsigned char uiNumStates = m_uiNumStates;
    if (uiNumStates >= DefaultCombo::NUM_STATES || !(m_uiNumStates & 1))
    {
        return;
    }

    unsigned char uiAnimIndex = 0xff;
    if (uiNumStates == DefaultCombo::ATTACK1)
    {
        Transition* pTransition = m_aState[DefaultCombo::IDLE].AddTransition(DefaultCombo::ATTACK1);
        if (!pTransition)
        {
            return;
        }
        Condition* pCondition = pTransition->AddCondition();
        if (!pCondition)
        {
            return;
        }
        pCondition->m_uiSoldierStates = 0x0b;
        pCondition->m_uiButtonAction = 1;
        pCondition->m_uiButton = 1;
    }
    else
    {
        unsigned int uiPrev = uiNumStates - 2;
        if (uiPrev > uiNumStates)
        {
            return;
        }
        State* pPrev = &m_aState[uiPrev];
        Transition* pTransition = pPrev->m_aTransition;
        if (!pTransition)
        {
            return;
        }
        pTransition->m_uiTargetState = uiNumStates;
        Condition* pCondition = pTransition->AddCondition();
        if (!pCondition)
        {
            return;
        }
        pCondition->m_uiSoldierStates = 0x01;
        pCondition->m_uiButtonAction = 0;
        pCondition->m_uiButton = 1;
        pPrev->AddTransition(m_uiNumStates - 1);
    }

    unsigned int uiState = m_uiNumStates;
    State* pState = AddState(PblHash(DefaultCombo::aszStateName[uiState]));
    if (!pState)
    {
        return;
    }
    pState->SetInputLock(0x7f, -1.0f);
    if (m_eCombo != SoldierAnimationBank::INVALID_COMBO)
    {
        SoldierAnimationBank::COMBO_ANIMATION eAnim = SoldierAnimationBank::AddComboAnimation(
            (SoldierAnimationBank::COMBO)m_eCombo,
            DefaultCombo::aszAnimationName[DefaultCombo::saiAnimForState[uiState]]);
        if (eAnim == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
        {
            uiAnimIndex = 0xff;
        }
        else
        {
            uiAnimIndex = SoldierAnimationData::Find(eAnim, SoldierAnimType::UPPER)->GetAnimIndex();
        }
    }
    pState->m_uiAnimIndex = uiAnimIndex;
    pState->m_uiSoldierStates = 1;
    pState->AddAttack();
    if (!pState->AddTransition(uiState + 1))
    {
        return;
    }

    uiState = m_uiNumStates;
    pState = AddState(PblHash(DefaultCombo::aszStateName[uiState]));
    if (!pState)
    {
        return;
    }
    pState->SetInputLock(0x7f, -1.0f);
    if (m_eCombo != SoldierAnimationBank::INVALID_COMBO)
    {
        SoldierAnimationBank::COMBO_ANIMATION eAnim = SoldierAnimationBank::AddComboAnimation(
            (SoldierAnimationBank::COMBO)m_eCombo,
            DefaultCombo::aszAnimationName[DefaultCombo::saiAnimForState[uiState]]);
        if (eAnim == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
        {
            uiAnimIndex = 0xff;
        }
        else
        {
            uiAnimIndex = SoldierAnimationData::Find(eAnim, SoldierAnimType::UPPER)->GetAnimIndex();
        }
    }
    pState->m_uiAnimIndex = uiAnimIndex;
    pState->m_uiSoldierStates = 1;
}

// FUNCTION: SWBF2 0x00687fd0
void Combo::AddDefaultDeflect()
{
    if (m_aState[DefaultCombo::IDLE].m_pDeflect)
    {
        return;
    }
    Deflect* pDeflect = m_aState[DefaultCombo::IDLE].AddDeflect();
    if (!pDeflect)
    {
        return;
    }
    for (unsigned int i = 0; i < DefaultCombo::NUM_DEFLECT_ANIMATIONS; ++i)
    {
        SoldierAnimationBank::COMBO_ANIMATION eAnim = SoldierAnimationBank::AddComboAnimation(
            (SoldierAnimationBank::COMBO)m_eCombo, aDeflectAnimation[i].szName);
        if (eAnim == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
        {
            continue;
        }
        if (pDeflect->AddDeflectAnimation(
                SoldierAnimationData::Find(eAnim, SoldierAnimType::UPPER)->GetAnimIndex(),
                aDeflectAnimation[i].uiDirections, 0) >= 15)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00688040
WeaponMelee::TrailData::TrailData()
    : m_pTrail(0)
{
}

// FUNCTION: SWBF2 0x00688060
WeaponMelee::TrailData::~TrailData()
{
    delete m_pTrail;
}

// FUNCTION: SWBF2 0x006880c0
void WeaponMelee::TrailData::Init(unsigned int uiEdge, const PblVector3& vOffsetPos,
                                  const PblVector3& vOffsetDir, float fLength, float fWidth,
                                  const RedColor& colorTrail, const RedColor& colorHead)
{
    m_uiEdge = (unsigned char)uiEdge;
    m_TrailClass.m_vOffsetPos = vOffsetPos;
    m_TrailClass.m_vOffsetDir = vOffsetDir;
    m_TrailClass.m_fLength = fLength;
    m_TrailClass.m_fWidth = fWidth;
    m_TrailClass.m_colorTrail = colorTrail;
    m_TrailClass.m_colorHead = colorHead;
    delete m_pTrail;
    m_pTrail = (LightsaberTrail*)m_TrailClass.BuildEffect();
    m_uiFrameLastUpdated = (unsigned char)(RedRenderer::GetFrameNumber() - 1);
}

// FUNCTION: SWBF2 0x00688140
void WeaponMelee::_InitDamageDataPool()
{
    s_aDamageDataPool = new DamageData[16];
    for (int i = 0; i < 16; ++i)
    {
        s_aDamageDataPool[i].pNext = (i < 15) ? &s_aDamageDataPool[i + 1] : 0;
    }
    s_pDamageDataFreeList = s_aDamageDataPool;
}

// FUNCTION: SWBF2 0x00688180
WeaponMelee::DamageData* WeaponMelee::AllocDamageData()
{
    DamageData* pData = s_pDamageDataFreeList;
    if (!pData)
    {
        return 0;
    }
    s_pDamageDataFreeList = pData->pNext;
    pData->pNext = 0;
    return pData;
}

// FUNCTION: SWBF2 0x006881a0
void WeaponMelee::FreeDamageData(DamageData* pData)
{
    pData->pNext = s_pDamageDataFreeList;
    s_pDamageDataFreeList = pData;
}

// FUNCTION: SWBF2 0x006881b0
void WeaponMelee::_InitTrailDataPool()
{
    s_aTrailDataPool = new TrailData[16];
    for (int i = 0; i < 16; ++i)
    {
        s_aTrailDataPool[i].pNext = (i < 15) ? &s_aTrailDataPool[i + 1] : 0;
    }
    s_pTrailDataFreeList = s_aTrailDataPool;
}

// FUNCTION: SWBF2 0x00688240
WeaponMelee::TrailData* WeaponMelee::_AllocTrailData()
{
    TrailData* pData = s_pTrailDataFreeList;
    if (!pData)
    {
        return 0;
    }
    s_pTrailDataFreeList = pData->pNext;
    pData->pNext = 0;
    return pData;
}

// FUNCTION: SWBF2 0x00688260
void WeaponMelee::_FreeTrailData(TrailData* pData)
{
    pData->pNext = s_pTrailDataFreeList;
    s_pTrailDataFreeList = pData;
}

// FUNCTION: SWBF2 0x00688270
WeaponMelee::WeaponMelee(const WeaponMeleeClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    if (pClass->m_pCombo)
    {
        if (mSoldierAnimationMap != -1)
        {
            const ZephyrSkeletonShared* pSkeleton = SoldierAnimatorClass::Instance()->GetDefaultSkeleton();
            if (mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
            {
                pSkeleton = ((EntitySoldier*)mOwner->GetGameObject())->GetClass()->mSpecialSkeleton;
            }
            m_pCombo = pClass->m_pCombo->ResolveForWeapon(this, pSkeleton);
        }
        else
        {
            m_pCombo = pClass->m_pCombo;
        }
    }
    else
    {
        m_pCombo = 0;
    }
    m_pMeleeThrow = 0;
    mState = IDLE;
    m_eComboState = COMBO_IDLE;
    m_eComboStatePrev = COMBO_IDLE;
    m_fComboStateTimer = 0.0f;
    m_fComboStateDurationPrev = 0.0f;
    m_fDeflectTimer = 0.0f;
    EnterState(COMBO_IDLE);
    m_bInHeroFlee = 0;
    m_fVelocityOverrideFactor = 0.0f;
    m_fTurnOnTimer = 0.0f;
    m_fTurnOnDuration = 0.0f;
    m_uiLightsaberWasOnMask = m_uiLightsaberOnMask;
    m_uiLightsaberSoundOnMask = m_uiLightsaberWasOnMask;
    m_uiTrailOnMask = 0;
    m_bAnimatedMove = 0;
    m_bLightsaberOn = 1;
    m_bLightsaberSoundOn = 0;
    m_bAI_Deflect = 0;
    m_pTrailData = 0;
    m_pDamageData = 0;
    ClearInput();
}

// FUNCTION: SWBF2 0x00688430
WeaponMelee::~WeaponMelee()
{
    ReleaseDamageData();
    while (m_pTrailData)
    {
        TrailData* pData = m_pTrailData;
        m_pTrailData = pData->pNext;
        _FreeTrailData(pData);
    }
}

// FUNCTION: SWBF2 0x006884c0
void WeaponMelee::Select(unsigned int uFlags, bool bOffhand)
{
    Weapon::Select(uFlags, bOffhand);
    ClearInput();
}

// FUNCTION: SWBF2 0x006884e0
void WeaponMelee::Deselect(unsigned int  , bool bOffhand)
{
    mSelectedFlag = 0;
    ClearInput();
    m_iSoldierState = SoldierState::STAND;
    if (!bOffhand)
    {
        TurnOff(-1.0f);
    }
    if (m_eComboState == COMBO_IDLE)
    {
        return;
    }

    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    unsigned int i;
    for (i = 0; i < pState->m_uiNumTransitions; ++i)
    {
        if (pState->m_aTransition[i].m_uiTargetState == COMBO_IDLE)
        {
            break;
        }
    }
    if (i < pState->m_uiNumTransitions)
    {
        ExitState();
        EnterState(COMBO_IDLE);
        m_eComboStatePrev = m_eComboState;
        m_fComboStateDurationPrev = 0.0f;
        ReleaseDamageData();
    }
}

// FUNCTION: SWBF2 0x00688580
void WeaponMelee::NotifySoldierState(int iState, float fTime, bool b0, bool b1, const PblVector3& vec)
{
    Weapon::NotifySoldierState(iState, fTime, b0, b1, vec);
    m_fSoldierStateTimer = fTime;
    m_vVelocityLocal = vec;
    m_bSoldierInputLock = b0;
    if (m_iSoldierState == SoldierState::HERO_FLEE)
    {
        TurnOff(-1.0f);
        ReleaseDamageData();
        ClearInput();
        if (m_pCombo && m_pCombo->m_pTakeKneeState)
        {
            for (unsigned int i = 0; i < m_pCombo->m_uiNumStates; ++i)
            {
                if (&m_pCombo->m_aState[i] == m_pCombo->m_pTakeKneeState)
                {
                    if ((unsigned int)m_eComboState != i)
                    {
                        ExitState();
                        EnterState((ComboState)i);
                    }
                    break;
                }
            }
        }
        m_bInHeroFlee = 1;
        mState = (WeaponState)MELEE_ATTACK;
        return;
    }

    if (m_iSoldierState > SoldierState::JET_JUMP)
    {
        ClearInput();
    }
    if (mState == (WeaponState)MELEE_IDLE)
    {
        int iSoldierState = m_iSoldierState;
        if (iSoldierState == SoldierState::STAND || iSoldierState == SoldierState::JUMP ||
            iSoldierState == SoldierState::JET_JUMP || iSoldierState == SoldierState::SPRINT)
        {
            TurnOn(-1.0f);
        }
        else
        {
            TurnOff(-1.0f);
        }
    }
    else if (m_iSoldierState > SoldierState::JET_JUMP)
    {
        ExitState();
        EnterState(COMBO_IDLE);
        TurnOff(-1.0f);
        ReleaseDamageData();
    }
}

// FUNCTION: SWBF2 0x00688700
bool WeaponMelee::OverrideSoldierControls()
{
    m_fInputThrustZ = mOwner->mControlMove;
    m_fInputThrustX = mOwner->mControlStrafe;
    ReadButtons();

    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    const bool bAlignToInitialThrust = pState->m_bAlignToInitialThrust;
    if (!bAlignToInitialThrust)
    {
        m_fAlignZ = m_fInputThrustZ;
        m_fAlignX = m_fInputThrustX;
    }

    if (pState->m_uiInputLock)
    {
        float fDuration = (float)pState->m_uiInputLockDuration * 0.01f;
        if (fDuration <= 0.0f || fDuration > m_fComboStateTimer)
        {
            if (pState->m_uiInputLock & 1)
            {
                mOwner->mControlMove = 0.0f;
                mOwner->mControlStrafe = 0.0f;
            }
            for (unsigned char uiButton = 1; uiButton < 7; ++uiButton)
            {
                if (pState->m_uiInputLock & (1 << uiButton))
                {
                    GetButton(uiButton)->Clear();
                }
            }
        }
    }

    mOwner->mControlMove = (float)pState->m_uiThrustFactor * 0.05f * mOwner->mControlMove;
    mOwner->mControlStrafe = (float)pState->m_uiStrafeFactor * 0.05f * mOwner->mControlStrafe;
    mOwner->mControlTurn = (float)pState->m_uiTurnFactor * 0.05f * mOwner->mControlTurn;
    mOwner->mControlPitch = (float)pState->m_uiPitchFactor * 0.05f * mOwner->mControlPitch;

    unsigned int uiSoldierStates = pState->m_uiSoldierStates;
    m_bUnreachableState_SoldierUpdate = 0;
    if (uiSoldierStates)
    {
        m_bUnreachableState_SoldierUpdate = 1;
        if (m_iSoldierState <= SoldierState::JUMP || m_iSoldierState == SoldierState::JET_JUMP)
        {
            m_bUnreachableState_SoldierUpdate = 0;
            while (!(uiSoldierStates & (1 << m_iSoldierState)) && !m_bUnreachableState_SoldierUpdate)
            {
                int iPrev = m_iSoldierState;
                switch (m_iSoldierState)
                {
                case SoldierState::STAND:
                    if ((uiSoldierStates & 8) && mOwner->Sprint())
                    {
                        m_iSoldierState = SoldierState::SPRINT;
                    }
                    else if ((uiSoldierStates & 2) && mOwner->Crouch())
                    {
                        m_iSoldierState = SoldierState::CROUCH;
                    }
                    else if ((uiSoldierStates & 16) && mOwner->Jump())
                    {
                        m_iSoldierState = SoldierState::JUMP;
                    }
                    else if ((uiSoldierStates & 32) && mOwner->Roll())
                    {
                        m_iSoldierState = SoldierState::ROLL;
                    }
                    else if ((uiSoldierStates & 64) && mOwner->Jump())
                    {
                        m_iSoldierState = SoldierState::JUMP;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::CROUCH:
                    if ((uiSoldierStates & 1) && mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else if (mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::PRONE:
                    if ((uiSoldierStates & 1) && mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else if ((uiSoldierStates & 2) && mOwner->Crouch())
                    {
                        m_iSoldierState = SoldierState::CROUCH;
                    }
                    else if (mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::SPRINT:
                    if (mOwner->EndSprint())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::JUMP:
                    if ((uiSoldierStates & 64) && mOwner->JetJump())
                    {
                        m_iSoldierState = SoldierState::JET_JUMP;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::JET_JUMP:
                    if ((uiSoldierStates & 16) && mOwner->EndJetJump())
                    {
                        m_iSoldierState = SoldierState::JUMP;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                default:
                    break;
                }
                if (m_iSoldierState == iPrev)
                {
                    m_bUnreachableState_SoldierUpdate = 1;
                }
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00688b50
bool WeaponMelee::ShouldShowReticule() const
{
    return m_pCombo->m_aState[m_eComboState].m_pDeflect != 0;
}

// FUNCTION: SWBF2 0x00688b70
unsigned char WeaponMelee::GetWeaponMeleeAnimation() const
{
    return m_uiAnimIndex;
}

// FUNCTION: SWBF2 0x00688b80
unsigned char WeaponMelee::GetNextWeaponMeleeAnimation() const
{
    const Combo::State* aState = m_pCombo->m_aState;
    const Combo::State* pState = &aState[m_eComboState];
    if (GetMeleeState() == MELEE_DEFLECT)
    {
        return (unsigned char)pState->m_uiAnimIndex;
    }
    unsigned int uiNumTransitions = pState->m_uiNumTransitions;
    if (uiNumTransitions == 0)
    {
        return 0xa4;
    }
    return (unsigned char)aState[pState->m_aTransition[uiNumTransitions - 1].m_uiTargetState].m_uiAnimIndex;
}

// FUNCTION: SWBF2 0x00688bc0
unsigned char WeaponMelee::GetOffhandWeaponMeleeAnimation(int eState) const
{
    return m_pCombo->GetOffhandAnimIndex(eState);
}

// FUNCTION: SWBF2 0x00688bd0
unsigned char WeaponMelee::GetWeaponMeleeThrowAnimation() const
{
    return (unsigned char)m_pCombo->m_uiThrowAnimIndex;
}

// FUNCTION: SWBF2 0x00688be0
bool WeaponMelee::IsAnimationVelocityAligned() const
{
    const Combo::State* pState = m_pCombo->GetState(m_eComboState);
    if (GetMeleeState() == MELEE_DEFLECT && m_uiAnimIndex != pState->GetAnimIndex())
    {
        return true;
    }
    return pState->IsAlignedToThrust();
}

// FUNCTION: SWBF2 0x00688c20
bool WeaponMelee::IsFullBodyAnimation() const
{
    return m_bAnimatedMove;
}

// FUNCTION: SWBF2 0x00688c30
bool WeaponMelee::OverrideSoldierVelocity(float& fVelocityZ, float& fVelocityX, float& fFactor,
                                          float& fVelocityY) const
{
    bool bOverride = false;
    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    if (pState->m_iGravity == Combo::State::GRAVITY_IMPULSE)
    {
        if (!m_bGravityDone)
        {
            fVelocityY = (float)pState->m_iGravityVelocityYTarget * 0.1f;
            const_cast<WeaponMelee*>(this)->m_bGravityDone = 1;
            bOverride = true;
        }
    }
    else if (pState->m_iGravity != 10)
    {
        float fGravity = (float)pState->m_iGravity * 0.1f;
        int iTarget = pState->m_iGravityVelocityYTarget;
        if (iTarget != -2048)
        {
            if (m_bGravityDone || (fGravity > 0.0f) == ((float)iTarget * 0.1f >= fVelocityY))
            {
                const_cast<WeaponMelee*>(this)->m_bGravityDone = 1;
                fGravity = 1.0f - (float)(int)pState->m_bGravityZeroAtTarget;
            }
        }
        if (fGravity != 1.0f)
        {
            fVelocityY -= (fGravity - 1.0f) * 18.0f * GameLoop::sClientDeltaTime;
            bOverride = true;
        }
    }

    float fOverride = m_fVelocityOverrideFactor;
    if (fOverride <= 0.0f)
    {
        return bOverride;
    }

    if (!pState->IsAnimatedMove() && m_eComboStatePrev != m_eComboState)
    {
        const Combo::State* pPrev = &m_pCombo->m_aState[m_eComboStatePrev];
        if (pPrev->IsAnimatedMove())
        {
            pState = pPrev;
        }
    }
    if (!pState->IsAnimatedMove())
    {
        return bOverride;
    }

    float fStateX = pState->GetVelocityX();
    float fStateZ = pState->GetVelocityZ();
    float fX;
    float fZ;
    if (pState->IsAlignedToThrust())
    {
        PblVector3 vThrust(-m_fInputThrustX, 0.0f, m_fInputThrustZ);
        AlignVectorToThrust(vThrust, true);
        PblVector3 vVelocity(pState->GetVelocityFromStrafe() * vThrust.x + fStateX, 0.0f,
                             pState->GetVelocityFromThrust() * vThrust.z + fStateZ);
        AlignVectorToThrust(vVelocity, false);
        fZ = vVelocity.z;
        fX = vVelocity.x;
    }
    else
    {
        fX = -(pState->GetVelocityFromStrafe() * m_fInputThrustX) + fStateX;
        fZ = pState->GetVelocityFromThrust() * m_fInputThrustZ + fStateZ;
    }
    fFactor = fOverride;
    fVelocityZ = fZ;
    fVelocityX = fX;
    return true;
}

// FUNCTION: SWBF2 0x00688f10
bool WeaponMelee::OverrideSoldierEnergyRestore(float& fRestore) const
{
    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    const bool bOverride = pState->m_bEnergyRestoreRateOverride;
    if (!bOverride && pState->m_iEnergyRestoreRate == 0)
    {
        return false;
    }
    if (bOverride)
    {
        fRestore = (float)pState->m_iEnergyRestoreRate * 0.2f;
    }
    else
    {
        fRestore = (float)pState->m_iEnergyRestoreRate * 0.2f + fRestore;
    }
    return true;
}

// FUNCTION: SWBF2 0x00688f90
bool WeaponMelee::OverrideAimer()
{
    if (mOwner->mPlayerId < 0)
    {
        return false;
    }
    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    const bool bAlignToThrust = pState->m_bAlignToThrust;
    if (pState->m_pDeflect || !bAlignToThrust)
    {
        return false;
    }
    Aimer* pAimer = mAimer;
    PblVector3 vDirection;
    vDirection = pAimer->mDirection;
    if (!AlignVectorToThrust(vDirection, false))
    {
        return false;
    }
    pAimer->SetSoldierInfo(pAimer->mFirePos, vDirection);
    return true;
}

// FUNCTION: SWBF2 0x00689010
void WeaponMelee::UpdateEffects(float fDeltaTime, const PblVector3& vPos, const PblVector3& vVel,
                                const GameSound* pSound)
{
    bool bOn = m_bLightsaberOn && m_uiLightsaberOnMask;
    if (m_bLightsaberSoundOn != bOn)
    {
        if (bOn)
        {
            if (!(mSoundControl.mFlags & GameSoundControllable::flags_playing))
            {
                if (!pSound)
                {
                    pSound = &static_cast<const WeaponMeleeClass*>(mClass)->m_OnSound;
                }
                mSoundProps = *pSound;
                mSoundControl.Play(&mSoundProps, &vPos, &vVel, true);
            }
        }
        else if (mSoundControl.mFlags & GameSoundControllable::flags_playing)
        {
            mSoundControl.Stop(true);
        }
        m_bLightsaberSoundOn = bOn;
    }
    else if (m_bLightsaberSoundOn && (mSoundControl.mFlags & GameSoundControllable::flags_playing))
    {
        if (!pSound)
        {
            pSound = &static_cast<const WeaponMeleeClass*>(mClass)->m_OnSound;
        }
        if (mSoundProps.GetProperties() != pSound->GetProperties())
        {
            mSoundControl.Stop(true);
            mSoundProps = *pSound;
            mSoundControl.Play(&mSoundProps, &vPos, &vVel, true);
        }
    }

    unsigned int uiMask = bOn ? m_uiLightsaberOnMask : 0;
    if (m_uiLightsaberSoundOnMask != uiMask)
    {
        if (uiMask & ~(unsigned int)m_uiLightsaberSoundOnMask)
        {
            m_OnOffSound.Stop(true);
            m_OnOffSoundProps = static_cast<const WeaponMeleeClass*>(mClass)->m_TurnOnSound;
            m_OnOffSound.Play(&m_OnOffSoundProps, &vPos, &vVel, true);
        }
        if (~uiMask & m_uiLightsaberSoundOnMask)
        {
            m_OnOffSound.Stop(true);
            m_OnOffSoundProps = static_cast<const WeaponMeleeClass*>(mClass)->m_TurnOffSound;
            m_OnOffSound.Play(&m_OnOffSoundProps, &vPos, &vVel, true);
        }
        m_uiLightsaberSoundOnMask = uiMask;
    }

    if (m_OnOffSound.mFlags & GameSoundControllable::flags_playing)
    {
        m_OnOffSound.Update(&m_OnOffSoundProps, &vPos, &vVel);
    }
    if (mSoundControl.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControl.Update(&mSoundProps, &vPos, &vVel);
    }
    if (mSoundControlFire.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControlFire.Update(&mSoundPropsFire, &vPos, &vVel);
    }
    if (mFoleyFXControl.mFlags & GameSoundControllable::flags_playing)
    {
        mFoleyFXControl.Update(&mFoleyFXProps, &vPos, &vVel);
    }
    m_fTurnOnTimer += fDeltaTime;
}

// FUNCTION: SWBF2 0x00689290
bool WeaponMelee::Update(float fDeltaTime)
{
    if (mOwner->GetPlayerId() < 0)
    {
        if (mSkip)
        {
            mSkipTime = fDeltaTime;
            return true;
        }
        fDeltaTime += mSkipTime;
        mSkipTime = 0.0f;
    }
    float fDt = fDeltaTime;

    GameObject* pObject = mOwner->GetGameObject();
    UpdateEffects(fDt, mOwner->GetGameObject()->GetPosition(), pObject->GetVelocity(), 0);
    m_bRestartAnimation = 0;

    if (m_pMeleeThrow)
    {
        const bool bReturned = m_pMeleeThrow->m_bReturnedThisFrame != 0;
        if (bReturned)
        {
            m_pMeleeThrow->m_bReturnedThisFrame = 0;
            if ((m_iSoldierState == SoldierState::STAND || m_iSoldierState == SoldierState::CROUCH) &&
                m_pCombo && m_pCombo->m_pCatchState)
            {
                for (unsigned int i = 0; i < m_pCombo->GetNumStates(); ++i)
                {
                    if (&m_pCombo->m_aState[i] == m_pCombo->m_pCatchState)
                    {
                        ExitState();
                        EnterState((ComboState)i);
                        break;
                    }
                }
            }
        }
        else if (m_pMeleeThrow->IsCurrentlyThrown())
        {
            if (m_eComboState != COMBO_IDLE)
            {
                ExitState();
                EnterState(COMBO_IDLE);
            }
            m_fInputTimer += fDt;
            return true;
        }
    }

    if (m_bAnimatedMove)
    {
        if (m_fVelocityOverrideFactor < 1.0f)
        {
            m_fVelocityOverrideFactor += fDt * (1.0f / 0.15f);
            if (m_fVelocityOverrideFactor > 1.0f)
            {
                m_fVelocityOverrideFactor = 1.0f;
            }
        }
    }
    else if (m_fVelocityOverrideFactor > 0.0f)
    {
        m_fVelocityOverrideFactor -= fDt * (1.0f / 0.15f);
        if (m_fVelocityOverrideFactor < 0.0f)
        {
            m_fVelocityOverrideFactor = 0.0f;
        }
    }

    if (m_bSoldierInputLock)
    {
        m_uiButtonDown = 0;
        m_InputQueue.Clear();
    }
    else
    {
        m_InputQueue.Update(m_uiButtonDown, m_fInputTimer);
    }

    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    const Combo::State* pStatePrev =
        m_eComboStatePrev != m_eComboState ? &m_pCombo->m_aState[m_eComboStatePrev] : 0;
    m_fComboStateTimer += fDt;
    bool bDone = false;
    while (!bDone)
    {
        ComboState eNextState = m_eComboState;
        const Combo::Transition* pTransition = 0;
        const Combo::Condition* pCondition = 0;
        float fMatchTime = FLT_MAX;
        bool bOvertime = false;
        float fDuration = pState->GetDuration();
        const bool bNoDuration = fDuration <= 0.0f;
        if (bNoDuration)
        {
            fDuration = m_fComboStateTimer;
        }

        if (!bNoDuration && m_fComboStateTimer >= fDuration)
        {

            if (pState->m_uiNumTransitions)
            {
                bool bLater = false;
                for (unsigned int i = 0; i < pState->m_uiNumTransitions; ++i)
                {
                    const Combo::Transition& transition = pState->m_aTransition[i];
                    const float fCost = transition.m_uiEnergyCost * 0.1f;
                    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
                    if (!pEnergy || !pEnergy->m_bOverheat || fCost <= 0.0f)
                    {
                        if (!transition.m_uiNumConditions && fMatchTime > fDuration)
                        {
                            pTransition = &transition;
                            pCondition = 0;
                            fMatchTime = fDuration;
                        }
                        for (unsigned int j = 0; j < transition.m_uiNumConditions; ++j)
                        {
                            const Combo::Condition* pTest = &transition.m_aCondition[j];
                            float fTime;
                            if (TestCondition(fDuration, m_fComboStateTimer, pTest, &fTime) &&
                                fMatchTime > fTime)
                            {
                                pTransition = &transition;
                                pCondition = pTest;
                                fMatchTime = fTime;
                            }
                            float fEnd = pTest->m_iTimeEnd * Combo::Condition::TIME_SCALE;
                            if (pTest->m_bTimeEndFromEnd)
                            {
                                fEnd += fDuration;
                            }
                            if (fEnd > fDuration)
                            {
                                bLater = true;
                            }
                        }
                    }
                }
                eNextState = (ComboState)pState->m_aTransition[pState->m_uiNumTransitions - 1].m_uiTargetState;
                bOvertime = !pCondition && bLater;
            }
            else
            {
                eNextState = COMBO_IDLE;
                fMatchTime = fDuration;
            }
            const float fOver = m_fComboStateTimer - fDuration;
            if (fDt > fOver)
            {
                UpdateState(fDt - fOver);
            }
        }
        else
        {

            if (pStatePrev)
            {
                const float fPrevTimer = m_fComboStateDurationPrev + m_fComboStateTimer;
                for (unsigned int i = 0; i < pStatePrev->m_uiNumTransitions; ++i)
                {
                    const Combo::Transition& transition = pStatePrev->m_aTransition[i];
                    const float fCost = transition.m_uiEnergyCost * 0.1f;
                    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
                    if (!pEnergy || !pEnergy->m_bOverheat || fCost <= 0.0f)
                    {
                        for (unsigned int j = 0; j < transition.m_uiNumConditions; ++j)
                        {
                            const Combo::Condition* pTest = &transition.m_aCondition[j];
                            float fTime;
                            if (TestCondition(m_fComboStateDurationPrev, fPrevTimer, pTest, &fTime))
                            {
                                fTime -= m_fComboStateDurationPrev;
                                if (fMatchTime > fTime)
                                {
                                    pTransition = &transition;
                                    pCondition = pTest;
                                    fMatchTime = fTime;
                                }
                            }
                        }
                    }
                }
                if (pCondition && pCondition->m_bSyncIfOvertime)
                {
                    fDt = m_fComboStateTimer;
                }
            }

            if (!pTransition)
            {
                for (unsigned int i = 0; i < pState->m_uiNumTransitions; ++i)
                {
                    const Combo::Transition& transition = pState->m_aTransition[i];
                    const float fCost = transition.m_uiEnergyCost * 0.1f;
                    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
                    if (!pEnergy || !pEnergy->m_bOverheat || fCost <= 0.0f)
                    {
                        for (unsigned int j = 0; j < transition.m_uiNumConditions; ++j)
                        {
                            const Combo::Condition* pTest = &transition.m_aCondition[j];
                            float fTime;
                            if ((pTest->IsBreak() || bNoDuration) &&
                                pTest->m_uiBreakTime * Combo::Condition::TIME_SCALE <= m_fComboStateTimer &&
                                TestCondition(fDuration, m_fComboStateTimer, pTest, &fTime) &&
                                fMatchTime > fTime)
                            {
                                pTransition = &transition;
                                pCondition = pTest;
                                fMatchTime = fTime;
                            }
                        }
                    }
                }
            }

            if (!pTransition && m_bAnimatedMove)
            {
                for (unsigned int i = 0; i < pState->m_uiNumAnimatedMoveConditions; ++i)
                {
                    const Combo::Condition* pTest = &pState->m_aAnimatedMoveCondition[i];
                    float fTime;
                    if (pTest->m_uiBreakTime * Combo::Condition::TIME_SCALE <= m_fComboStateTimer &&
                        TestCondition(fDuration, m_fComboStateTimer, pTest, &fTime))
                    {
                        m_bAnimatedMove = 0;
                    }
                }
            }
        }

        if (pTransition)
        {
            eNextState = (ComboState)pTransition->m_uiTargetState;
            EnergyBar* pEnergy = mOwner->GetEnergyBar();
            if (pEnergy)
            {
                pEnergy->SpendEnergy(pTransition->m_uiEnergyCost * 0.1f);
            }
        }

        if (pTransition || eNextState != m_eComboState)
        {
            const float fTimer = m_fComboStateTimer;
            const unsigned char uiAnimIndex = pState->GetAnimIndex();
            if (bOvertime)
            {
                m_eComboStatePrev = m_eComboState;
                m_fComboStateDurationPrev = fDuration;
                pStatePrev = pState;
            }
            else
            {
                m_eComboStatePrev = eNextState;
                m_fComboStateDurationPrev = 0.0f;
                pStatePrev = 0;
            }
            WeaponMelee::_ClearInputQueueToTime(m_InputQueue, m_fInputTimer + fMatchTime - fTimer);
            ExitState();
            EnterState(eNextState);
            m_fComboStateTimer += fDt;
            pState = &m_pCombo->m_aState[m_eComboState];
            if (uiAnimIndex == pState->GetAnimIndex())
            {
                m_fComboStateTimer = fTimer;
            }
            if (pState->m_bMustShowOneFrame)
            {
                bDone = UpdateState(fDt);
            }
            continue;
        }

        m_bUnreachableState_WeaponUpdate = 0;
        const unsigned char uiSoldierStates = (unsigned char)pState->m_uiSoldierStates;
        if (uiSoldierStates)
        {
            const int iSoldierState = m_iSoldierState;
            if (!((1 << iSoldierState) & uiSoldierStates))
            {
                m_bUnreachableState_WeaponUpdate = 1;
                if (iSoldierState > SoldierState::JET_JUMP)
                {
                    m_bUnreachableState_SoldierUpdate = 1;
                }
                else if (iSoldierState == SoldierState::JUMP || iSoldierState == SoldierState::JET_JUMP)
                {
                    m_bUnreachableState_SoldierUpdate =
                        ((unsigned char)pState->m_uiSoldierStates &
                         ((1 << SoldierState::JUMP) | (1 << SoldierState::JET_JUMP))) == 0;
                }
                else if (iSoldierState > SoldierState::SPRINT)
                {
                    m_bUnreachableState_SoldierUpdate = 1;
                }
            }
        }
        if (m_bUnreachableState_SoldierUpdate && m_bUnreachableState_WeaponUpdate && !m_bInHeroFlee)
        {
            ExitState();
            EnterState(COMBO_IDLE);
            m_fComboStateTimer += fDt;
            pState = &m_pCombo->m_aState[m_eComboState];
            m_eComboStatePrev = m_eComboState;
            pStatePrev = 0;
            m_fComboStateDurationPrev = 0.0f;
            ReleaseDamageData();
        }
        else
        {
            bDone = UpdateState(fDt);
        }
    }
    m_fInputTimer += fDt;
    return true;
}

// FUNCTION: SWBF2 0x0068d270
WeaponMeleeClass::WeaponMeleeClass(unsigned int uHash)
    : WeaponClass(uHash),
      mDamage(DamageOwner(0, 0, 0, 0, 0, 1.0f), 0.0f)
{
    m_uiNumHitEffects = 0;
    m_pComboODF = 0;
    m_pCombo = 0;
    for (int i = 0; i < 5; ++i)
    {
        m_uiHitEffects[i] = 0;
    }
    m_uiDeflectEffect = 0;
    m_uiNumDamageEdges = 0;
    m_aDamageEdge = 0;
    m_fTurnOnDuration = 0.2f;
    m_uiNumWeapons = 1;
    m_auiHardpoint[0] = 0;
    m_amDeflectMatrix = 0;
    mImpactExplosion = 0;
    m_eDeflectMatrixMap = -1;
}

// FUNCTION: SWBF2 0x0068d420
WeaponMeleeClass::WeaponMeleeClass(WeaponMeleeClass* aParent, unsigned int aId)
    : WeaponClass(aParent, aId),
      mDamage(DamageOwner(0, 0, 0, 0, 0, 1.0f), 0.0f)
{
    m_pCombo = aParent->m_pCombo;
    m_pComboODF = 0;
    m_uiNumHitEffects = aParent->m_uiNumHitEffects;
    for (int i = 0; i < 5; ++i)
    {
        m_uiHitEffects[i] = aParent->m_uiHitEffects[i];
    }
    m_HitSound = aParent->m_HitSound;
    m_uiDeflectEffect = aParent->m_uiDeflectEffect;
    m_DeflectSound = aParent->m_DeflectSound;
    m_OnSound = aParent->m_OnSound;
    m_TurnOnSound = aParent->m_TurnOnSound;
    m_TurnOffSound = aParent->m_TurnOffSound;
    m_fTurnOnDuration = aParent->m_fTurnOnDuration;
    m_uiNumDamageEdges = aParent->m_uiNumDamageEdges;
    if (m_uiNumDamageEdges)
    {
        m_aDamageEdge = new DamageEdge[m_uiNumDamageEdges];
        memcpy(m_aDamageEdge, aParent->m_aDamageEdge, m_uiNumDamageEdges * sizeof(DamageEdge));
    }
    else
    {
        m_aDamageEdge = 0;
    }
    memcpy(m_apWeaponModel, aParent->m_apWeaponModel, sizeof(m_apWeaponModel));
    memcpy(m_auiHardpoint, aParent->m_auiHardpoint, sizeof(m_auiHardpoint));
    m_uiNumWeapons = aParent->m_uiNumWeapons;
    m_eDeflectMatrixMap = -1;
    m_amDeflectMatrix = 0;
    mImpactExplosion = aParent->mImpactExplosion;
}

static inline int ClampColorChannel(int i)
{
    return i < 0 ? 0 : (i > 255 ? 255 : i);
}

// SYNTHETIC: SWBF2 0x0068d3f0
// WeaponMeleeClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068d6f0
WeaponMeleeClass::~WeaponMeleeClass()
{
    delete[] m_aDamageEdge;
    m_aDamageEdge = 0;
    if (m_amDeflectMatrix)
    {
        unsigned char* pAligned = reinterpret_cast<unsigned char*>(m_amDeflectMatrix);
        delete[] (pAligned - pAligned[-1]);
    }
    m_amDeflectMatrix = 0;
}

// FUNCTION: SWBF2 0x0068d780
RedColor aToColor(const char* pcArg)
{
    int r, g, b, a = 255;
    int iCount = sscanf(pcArg, "%d %d %d %d", &r, &g, &b, &a);
    if (iCount < 3)
    {
        iCount = sscanf(pcArg, "%2x%2x%2x%2x", &r, &g, &b, &a);
    }
    if (iCount < 3)
    {
        RedColor color;
        *(unsigned int*)&color = 0;
        return color;
    }
    return RedColor((unsigned char)ClampColorChannel(r),
                    (unsigned char)ClampColorChannel(g),
                    (unsigned char)ClampColorChannel(b),
                    (unsigned char)(iCount == 4 ? ClampColorChannel(a) : 255));
}

// FUNCTION: SWBF2 0x0068d880
void WeaponMeleeClass::SetProperty(unsigned int aId, const char* aValue)
{
    // GLOBAL: SWBF2 0x01faa75c
    static WeaponMeleeClass* s_pThis;
    // GLOBAL: SWBF2 0x01fac3a0
    static RedModel* s_pAttachToModel;
    // GLOBAL: SWBF2 0x01fac39c
    static unsigned int s_uiAttachToHP;
    // GLOBAL: SWBF2 0x01fac3a4
    static unsigned int s_uiEdgeHP;

    if (s_pThis != this)
    {
        s_pThis = this;
        s_uiODFComboId = 0;
        s_pAttachToModel = 0;
        s_uiAttachToHP = 0;
        s_uiEdgeHP = 0;
    }

    switch (aId)
    {
    case 0x02BB1FE0:
    case 0x9B30E763:
        mImpactExplosion = ExplosionClass::Find(PblHash(aValue));
        break;

    case 0x03B58AB8:
    {
        int iNum = atoi(aValue);
        if (iNum < 1)
        {
            iNum = 1;
        }
        else if (iNum > 4)
        {
            iNum = 4;
        }
        _CreateDamageEdges(iNum);
        break;
    }

    case 0x042DABAE:
        m_TurnOffSound.SetID(aValue, false, false);
        break;
    case 0xC5BDD741:
        m_HitSound.SetID(aValue, false, false);
        break;
    case 0xC20DFE6F:
        m_DeflectSound.SetID(aValue, false, false);
        break;
    case 0xDE71B19F:
        m_OnSound.SetID(aValue, false, false);
        break;
    case 0xEA603E52:
        m_TurnOnSound.SetID(aValue, false, false);
        break;

    case 0x15A6D28C:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            m_aDamageEdge[iEdge].uiGlowTextureHash = PblHash(aValue);
        }
        break;
    }
    case 0x3C6468F4:
    case 0xFF773107:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            m_aDamageEdge[iEdge].uiTextureHash = PblHash(aValue);
        }
        break;
    }
    case 0x643005E0:
    case 0xE2AF0205:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            float fLength = LIGHTSABER_DEFAULT_LENGTH;
            sscanf(aValue, "%f", &fLength);
            m_aDamageEdge[iEdge].fLength = fLength;
        }
        break;
    }
    case 0x1F56AD0F:
    case 0xC9CBCCD8:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            float fWidth = LIGHTSABER_DEFAULT_WIDTH;
            sscanf(aValue, "%f", &fWidth);
            m_aDamageEdge[iEdge].fWidth = fWidth;
        }
        break;
    }
    case 0xFB0C9787:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            m_aDamageEdge[iEdge].colorTrail = aToColor(aValue);
        }
        break;
    }

    case 0x18BDF340:
        if (m_uiNumHitEffects < 5)
        {
            m_uiHitEffects[m_uiNumHitEffects++] = PblHash(aValue);
        }
        break;
    case 0xC0929477:
        m_uiHitEffects[0] = PblHash(aValue);
        m_uiNumHitEffects = 1;
        break;
    case 0xF1820751:
        m_uiDeflectEffect = PblHash(aValue);
        break;

    case 0xA7DDE527:
        if (sscanf(aValue, "%f", &m_fTurnOnDuration) != 1)
        {
            m_fTurnOnDuration = MELEE_DEFAULT_TURNONDURATION;
        }
        else if (m_fTurnOnDuration < 0.0f)
        {
            m_fTurnOnDuration = 0.0f;
        }
        break;

    case 0x2D32CE43:
    case 0xA7C25382:
    {

        char acBuf[257];
        strlcpy(acBuf, aValue, sizeof(acBuf));
        char* pcParent = 0;
        char* pcCombo = 0;
        char* pcName = strtok(acBuf, " ");
        while (pcName && !*pcName)
        {
            pcName = strtok(0, " ");
        }
        if (pcName)
        {
            do
                pcParent = strtok(0, " ");
            while (pcParent && !*pcParent);
            if (pcParent)
            {
                do
                    pcCombo = strtok(0, " ");
                while (pcCombo && !*pcCombo);
            }
        }
        if (aId == 0x2D32CE43)
        {
            m_pComboODF = 0;
            m_pCombo = pcCombo ? (Combo*)Combo::Find(PblHash(pcCombo)) : 0;
            if (!m_pCombo)
            {
                mSoldierAnimationWeapon = SoldierAnimationBank::INVALID_WEAPON;
            }
            else
            {
                mSoldierAnimationWeapon = SoldierAnimationBank::AddWeaponCombo(pcName, pcParent, pcCombo);
            }
        }
        else
        {

            mSoldierAnimationWeapon = SoldierAnimationBank::AddWeaponCombo(pcName, pcParent, mFilename);
            if (mSoldierAnimationWeapon != SoldierAnimationBank::INVALID_WEAPON)
            {
                s_uiODFComboId = SoldierAnimationBank::GetComboNameHash(SoldierAnimationBank::GetWeaponCombo(
                    (SoldierAnimationBank::WEAPON)mSoldierAnimationWeapon));
                _CreateODFCombo(s_uiODFComboId);
            }
        }
        break;
    }

    case 0xC1A67869:
        if (_CreateODFCombo(s_uiODFComboId) && m_pComboODF->m_uiNumStates < 7)
        {
            m_pComboODF->AddDefaultAttackState();
        }
        break;

    case 0x7C9C8AE0:
        if (_CreateODFCombo(s_uiODFComboId) && m_pComboODF->m_uiNumStates < 7)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates;
            m_pComboODF->AddDefaultAttackState();
            char soundID[256] = "";
            float fDuration = MELEE_DEFAULT_STATE_DURATION;
            float fDamageTime = -MELEE_DEFAULT_DAMAGE_TIME;
            sscanf(aValue, "%f %f %s", &fDuration, &fDamageTime, soundID);
            if (fDuration >= 0.0f && (fDamageTime < 0.0f || fDamageTime > fDuration))
            {
                fDamageTime = fDuration * 0.2f;
            }
            m_pComboODF->m_aState[uiState].SetDuration(fDuration);
            m_pComboODF->m_aState[uiState].m_aAttack->SetDamageTimeStart(fDamageTime, false);
            if (fDuration > 0.0f)
            {
                m_pComboODF->m_aState[uiState].m_aAttack->SetDamageTimeEnd(fDuration * 0.2f + fDamageTime, false);
            }
            else
            {
                m_pComboODF->m_aState[uiState].m_aAttack->SetDamageTimeEnd(fDamageTime + 0.16666667f, false);
            }
            m_pComboODF->m_aState[uiState].m_sound.SetID(soundID, false, false);
        }
        break;

    case 0xB0D14CE0:
        if (atoi(aValue) && m_pComboODF && m_pComboODF->m_uiNumStates > 0)
        {
            m_pComboODF->AddDefaultDeflect();
        }
        break;

    case 0x45C02D38:
        if (m_pComboODF && m_pComboODF->m_uiNumStates > 2)
        {
            float fComboContinueTime = 0.0f;
            float fComboEndTime = 0.0f;
            sscanf(aValue, "%f %f", &fComboContinueTime, &fComboEndTime);
            for (unsigned int i = 1; i < m_pComboODF->m_uiNumStates; i += 2)
            {
                Combo::Condition* pCondition = m_pComboODF->m_aState[i].m_aTransition->m_aCondition;
                pCondition->SetTimeStart(fComboContinueTime, false);
                pCondition->SetTimeEnd(fComboEndTime, true, false);
            }
        }
        break;

    case 0xD0537A7C:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            m_pComboODF->m_aState[m_pComboODF->m_uiNumStates - 2].m_sound.SetID(aValue, false, false);
        }
        break;

    case 0x49021F55:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates - 2;
            float fDuration = MELEE_DEFAULT_STATE_DURATION;
            if (sscanf(aValue, "%f", &fDuration) != 1)
            {
                fDuration = -1.0f;
            }
            m_pComboODF->m_aState[uiState].SetDuration(fDuration);
        }
        break;

    case 0x1E111A47:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates - 1;
            float fDuration = MELEE_DEFAULT_STATE_DURATION;
            if (sscanf(aValue, "%f", &fDuration) != 1)
            {
                fDuration = -1.0f;
            }
            m_pComboODF->m_aState[uiState].SetDuration(fDuration);
        }
        break;

    case 0x91F410CB:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates - 1;
            float fBreakOutTime = -1.0f;
            if (sscanf(aValue, "%f", &fBreakOutTime) == 1 && fBreakOutTime >= 0.0f)
            {
                Combo::State& state = m_pComboODF->m_aState[uiState];
                Combo::Transition* pTransition;
                if (state.m_uiNumTransitions == 0)
                {
                    pTransition = state.AddTransition(0);
                    if (!pTransition)
                    {
                        break;
                    }
                }
                else
                {
                    pTransition = state.m_aTransition;
                }
                Combo::Condition* pCondition = pTransition->AddCondition();
                if (pCondition)
                {
                    pCondition->SetBreak(true);
                    pCondition->SetTimeStart(fBreakOutTime, false);
                    pCondition->SetTimeEnd(0.0f, true, false);
                    pCondition->SetInputMag(0.5477f, false);
                }
            }
        }
        break;

    case 0x308AD121:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates - 2;
            float fDamageTime = -1.0f;
            if (sscanf(aValue, "%f", &fDamageTime) != 1)
            {
                fDamageTime = -1.0f;
            }
            if (fDamageTime < 0.0f)
            {
                m_pComboODF->m_aState[uiState].m_aAttack->SetDamageTimeStart(-fDamageTime, true);
                m_pComboODF->m_aState[uiState].m_aAttack->SetDamageTimeEnd(-fDamageTime - 0.16666667f, true);
            }
            else
            {
                m_pComboODF->m_aState[uiState].m_aAttack->SetDamageTimeStart(fDamageTime, false);
                m_pComboODF->m_aState[uiState].m_aAttack->SetDamageTimeEnd(fDamageTime + 0.16666667f, false);
            }
        }
        break;

    case 0x49F8B243:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates - 2;
            float fDamageDistance = MELEE_DEFAULT_DAMAGE_DISTANCE;
            if (sscanf(aValue, "%f", &fDamageDistance) != 1)
            {
                fDamageDistance = MELEE_DEFAULT_DAMAGE_DISTANCE;
            }
            else if (fDamageDistance < 0.0f)
            {
                fDamageDistance = 0.0f;
            }
            m_pComboODF->m_aState[uiState].m_aAttack->SetDamageLength(fDamageDistance, false);
        }
        break;

    case 0xF34ADD68:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates - 2;
            float fDamage = MELEE_DEFAULT_DAMAGE;
            if (sscanf(aValue, "%f", &fDamage) != 1)
            {
                fDamage = MELEE_DEFAULT_DAMAGE;
            }
            else if (fDamage < 0.0f)
            {
                fDamage = 0.0f;
            }
            m_pComboODF->m_aState[uiState].m_aAttack->SetDamage(fDamage);
        }
        break;

    case 0x47C86B4A:
        WeaponClass::SetProperty(aId, aValue);
        s_pAttachToModel = mModel;
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = 0x2B960099;
            s_uiEdgeHP = 0xB3EA0EE8;
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;

    case 0x4274FC96:
        WeaponClass::SetProperty(aId, aValue);
        s_pAttachToModel = mModel;
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = 0x2B960099;
            s_uiEdgeHP = PblTEMPHash(aValue);
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;

    case 0xCA67EAC4:
        if (m_uiNumWeapons >= 4)
        {
            break;
        }
        s_pAttachToModel = (RedModel*)RedModel::Find(PblHash(aValue));
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = 0xB7EC1D31;
            s_uiEdgeHP = 0xB3EA0EE8;
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;

    case 0x9E10B228:
    {
        char acEdgeHP[64];
        char acAttachToHP[64];
        int iCount = sscanf(aValue, "%s %s", acEdgeHP, acAttachToHP);
        if (iCount < 1)
        {
            break;
        }
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = iCount >= 2 ? PblTEMPHash(acAttachToHP) : 0xB7EC1D31;
            s_uiEdgeHP = PblTEMPHash(acEdgeHP);
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;
    }

    case 0x94F22065:
    {
        char acAttachToHP[64];
        float fX = 0.0f, fY = 0.0f, fZ = 0.0f;
        float fDirX = 0.0f, fDirY = 0.0f, fDirZ = 1.0f;
        int iCount = sscanf(aValue, "%s %f %f %f %f %f %f", acAttachToHP, &fDirX, &fDirY, &fDirZ, &fX, &fY, &fZ);
        if (iCount < 1)
        {
            break;
        }
        s_pAttachToModel = 0;
        s_uiAttachToHP = PblTEMPHash(acAttachToHP);
        s_uiEdgeHP = 0;
        int iEdge = _AttachDamageEdge(0, s_uiAttachToHP, 0);
        if (iEdge < 0)
        {
            s_uiAttachToHP = 0;
            break;
        }
        s_uiEdgeHP = m_aDamageEdge[iEdge].uiEdgeHP;
        if (iCount >= 4)
        {
            PblVector3 vDir(fDirX, fDirY, fDirZ);
            PblVector3 vNormal;
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNormal), reinterpret_cast<const D3DXVECTOR3*>(&vDir));
            m_aDamageEdge[iEdge].vOffsetDir = vNormal;
            if (iCount >= 7)
            {
                m_aDamageEdge[iEdge].vOffsetPos = PblVector3(fX, fY, fZ);
            }
        }
        break;
    }

    default:
        WeaponClass::SetProperty(aId, aValue);
        break;
    }
}

// FUNCTION: SWBF2 0x0068e7c0
WeaponClass* WeaponMeleeClass::Derive(unsigned int uHash)
{
    return new WeaponMeleeClass(this, uHash);
}

// FUNCTION: SWBF2 0x0068ec10
void WeaponMeleeClass::Init()
{
    Combo::Init();
}

// FUNCTION: SWBF2 0x0068ec20
void WeaponMeleeClass::PostLoad()
{
    Combo::PostLoad();
    WeaponMelee::_InitDamageDataPool();
    WeaponMelee::_InitTrailDataPool();
}

// FUNCTION: SWBF2 0x0068ec40
void WeaponMeleeClass::ReadCombo(PblFileChunk& Chunk)
{
    PblConfig config(Chunk);
    if (!Combo::s_ComboTable.Find(config.GetNameHash()) && Combo::sMemoryPool.mUsed < Combo::sMemoryPool.GetCount())
    {
        Combo* pCombo = new Combo;
        pCombo->ReadConfig(config);
    }
}

// FUNCTION: SWBF2 0x0068ed40
int WeaponMeleeClass::_AttachDamageEdge(const RedModel* pAttachToModel, unsigned int uiAttachModelToHP,
                                        unsigned int uiAttachToModelHP)
{
    static const unsigned int DEFAULT_NUM_DAMAGE_EDGES = 1;

    if (!uiAttachModelToHP)
    {
        return -1;
    }

    unsigned int uiWeapon = 0;
    PblVector3 vOffsetPos;
    PblVector3 vOffsetDir;
    if (pAttachToModel)
    {
        if (!uiAttachToModelHP)
        {
            return -1;
        }
        if (pAttachToModel == mModel && !m_auiHardpoint[0])
        {
            m_auiHardpoint[0] = uiAttachModelToHP;
        }
        else if (pAttachToModel != mModel || uiAttachModelToHP != m_auiHardpoint[0])
        {
            for (uiWeapon = 1; uiWeapon < m_uiNumWeapons; ++uiWeapon)
            {
                if (m_apWeaponModel[uiWeapon - 1] == pAttachToModel && m_auiHardpoint[uiWeapon] == uiAttachModelToHP)
                {
                    break;
                }
            }
            if (uiWeapon >= m_uiNumWeapons)
            {
                if (m_uiNumWeapons >= 4)
                {
                    return -1;
                }
                m_apWeaponModel[uiWeapon - 1] = (RedModel*)pAttachToModel;
                m_auiHardpoint[uiWeapon] = uiAttachModelToHP;
                ++m_uiNumWeapons;
            }
        }
        for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
        {
            if (m_aDamageEdge[i].uiAttachToWeapon == uiWeapon && m_aDamageEdge[i].uiEdgeHP == uiAttachToModelHP)
            {
                return i;
            }
        }

        PblMatrix mHP;
        if (pAttachToModel->GetParentBoneAndOffsetMatrix(uiAttachToModelHP, mHP))
        {
            vOffsetPos = mHP.GetTranslation();
            vOffsetDir = mHP.GetZAxis();
        }
        else
        {
            vOffsetPos = PblVector3(0.0f, 0.0f, 0.0f);
            vOffsetDir = gVectorUnitZ;
        }
    }
    else
    {
        if (m_uiNumDamageEdges && uiAttachToModelHP > m_uiNumDamageEdges)
        {
            return -1;
        }
        for (uiWeapon = 1; uiWeapon < m_uiNumWeapons; ++uiWeapon)
        {
            if (!m_apWeaponModel[uiWeapon - 1] && m_auiHardpoint[uiWeapon] == uiAttachModelToHP)
            {
                break;
            }
        }
        if (uiWeapon >= m_uiNumWeapons)
        {
            if (m_uiNumWeapons >= 4)
            {
                return -1;
            }
            m_apWeaponModel[uiWeapon - 1] = 0;
            m_auiHardpoint[uiWeapon] = uiAttachModelToHP;
            ++m_uiNumWeapons;
        }
        if (!uiAttachToModelHP)
        {
            uiAttachToModelHP = 1;
            for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
            {
                if (m_aDamageEdge[i].uiAttachToWeapon == uiWeapon)
                {
                    unsigned int uiEdge = m_aDamageEdge[i].uiEdgeHP;
                    if (uiEdge <= m_uiNumDamageEdges && uiAttachToModelHP <= uiEdge)
                    {
                        uiAttachToModelHP = uiEdge + 1;
                    }
                }
            }
        }
        else
        {
            for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
            {
                if (m_aDamageEdge[i].uiAttachToWeapon == uiWeapon && m_aDamageEdge[i].uiEdgeHP == uiAttachToModelHP)
                {
                    return i;
                }
            }
        }
        vOffsetPos = PblVector3(0.0f, 0.0f, 0.0f);
        vOffsetDir = gVectorUnitZ;
    }

    int iEdge = 0;
    if (!m_aDamageEdge)
    {
        _CreateDamageEdges(m_uiNumDamageEdges ? m_uiNumDamageEdges : DEFAULT_NUM_DAMAGE_EDGES);
    }
    else
    {
        for (iEdge = 0; iEdge < (int)m_uiNumDamageEdges; ++iEdge)
        {
            if (!m_aDamageEdge[iEdge].uiEdgeHP)
            {
                break;
            }
        }
        if (iEdge >= (int)m_uiNumDamageEdges)
        {
            return -1;
        }
    }

    m_aDamageEdge[iEdge].uiAttachToWeapon = uiWeapon;
    m_aDamageEdge[iEdge].uiEdgeHP = uiAttachToModelHP;
    m_aDamageEdge[iEdge].vOffsetPos = vOffsetPos;
    m_aDamageEdge[iEdge].vOffsetDir = vOffsetDir;
    m_aDamageEdge[iEdge].fLength = LIGHTSABER_DEFAULT_LENGTH;
    m_aDamageEdge[iEdge].fWidth = LIGHTSABER_DEFAULT_WIDTH;
    m_aDamageEdge[iEdge].uiTextureHash = 0;
    m_aDamageEdge[iEdge].uiGlowTextureHash = 0x79B932AB;
    m_aDamageEdge[iEdge].colorTrail = LIGHTSABER_DEFAULT_TRAIL_COLOR;
    return iEdge;
}

// FUNCTION: SWBF2 0x0068f0d0
WeaponMeleeClass::DamageEdge* WeaponMeleeClass::_CreateDamageEdges(unsigned int uiNumEdges)
{
    if (m_aDamageEdge && uiNumEdges != m_uiNumDamageEdges)
    {
        delete m_aDamageEdge;
        m_aDamageEdge = 0;
    }
    if (!m_aDamageEdge)
    {
        m_uiNumDamageEdges = uiNumEdges;
        m_aDamageEdge = new DamageEdge[m_uiNumDamageEdges];
        memset(m_aDamageEdge, 0, m_uiNumDamageEdges * sizeof(DamageEdge));
    }
    return m_aDamageEdge;
}

// FUNCTION: SWBF2 0x0068e8c0
bool WeaponMeleeClass::IsLightSaber() const
{
    for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
    {
        if (m_aDamageEdge[i].uiTextureHash)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068eb70
void WeaponMeleeClass::PlayTurnOnSound() const
{
    m_TurnOnSound.Play(0, 0, true, true);
}

// FUNCTION: SWBF2 0x0068eb90
void WeaponMeleeClass::PlayTurnOffSound() const
{
    m_TurnOffSound.Play(0, 0, true, true);
}

// SYNTHETIC: SWBF2 0x00407220
// `dynamic initializer for 'gDefaultHash''

// GLOBAL: SWBF2 0x01f9a8f0
// gDefaultHash
unsigned int gDefaultHash = PblHash("default");
