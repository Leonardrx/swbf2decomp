#include <d3dx9math.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "EntityHologram.h"
#include "Battlefront2Lod.h"
#include "CameraManager.h"
#include "Character.h"
#include "LightBeam.h"
#include "NetGame.h"
#include "Team.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"


extern NoLodData noloddata;

namespace FLRenderer
{
bool IsReflected(const PblVector3& vPos, float fRadius, PblMatrix& mReflection, bool b);

unsigned int GetCurrentViewport();
}


static const float kfHoloBeamWidthCone = 0.8f;
static const float kfHoloBeamFadeFactor = 0.25f;
static const float kfHoloBeamFadeLength = 1.0f;
static const float kfHoloBeamFadeLengthFactor = 0.2f;
static const float kfHoloPopAspectProbability = 0.8f;
static const float kfHoloPopRotateProbability = 0.7f;
static const float kfHoloMaxTilt = 30.0f;

static const float kfFadeDrawDistance = 0.9f;
// GLOBAL: SWBF2 0x0078f430
static const float kfFadeDrawDistanceSquared = kfFadeDrawDistance * kfFadeDrawDistance;
// GLOBAL: SWBF2 0x0078f42c
static const float kfFadeDrawDistanceFactor = 1.0f / (1.0f - kfFadeDrawDistanceSquared);



// FUNCTION: SWBF2 0x004bd820
inline const RedColor Interpolate(const RedColor& a, const RedColor& b, float t)
{
    RedColor result;
    result.b = (unsigned char)(int)(a.b * (1.0f - t) + b.b * t + 0.5f);
    result.g = (unsigned char)(int)(a.g * (1.0f - t) + b.g * t + 0.5f);
    result.r = (unsigned char)(int)(a.r * (1.0f - t) + b.r * t + 0.5f);
    result.a = (unsigned char)(int)(a.a * (1.0f - t) + b.a * t + 0.5f);
    return result;
}

// FUNCTION: SWBF2 0x004bd990
LensFlare::LensFlare()
    : _fSize(1.0f)
    , _color(RedColor::WHITE)
    , _fSquaredDrawDistance(0.0f)
    , _uiHideInView(0)
{
    _fRandomRotOffset = PblRandom::_Global.GetFloat() * PblMath::fTwoPi;
}

// FUNCTION: SWBF2 0x004bdc00
LightBeam::LightBeam()
    : _vHeadPosition(0.0f, 0.0f, 0.0f)
    , _vDirection(0.0f, 1.0f, 0.0f)
    , _color(RedColor::WHITE)
    , _fLength(10.0f)
    , _fHeadSizeX(1.0f)
    , _fHeadSizeZ(1.0f)
    , _fTailSizeX(1.0f)
    , _fTailSizeZ(1.0f)
    , _fFadeLength(1.0f)
    , _fFadeAmount(0.2f)
    , _fSquaredDrawDistance(0.0f)
    , _fFlareIntensity(1.0f)
    , _fBeamIntensity(1.0f)
    , _fFlareCosAngle(0.5f)
{
}

namespace
{

inline RedColorValue ToColorValue(const RedColor& c)
{
    RedColorValue v;
    v.r = c.r * (1.0f / 255.0f);
    v.g = c.g * (1.0f / 255.0f);
    v.b = c.b * (1.0f / 255.0f);
    v.a = c.a * (1.0f / 255.0f);
    return v;
}
}

// SYNTHETIC: SWBF2 0x00402b40
// `dynamic initializer for 'EntityHologram::rttiHashEntityHologram''
// EntityHologram::rttiHashEntityHologram

// GLOBAL: SWBF2 0x01ebbcc4
// EntityHologram::rttiHashEntityHologram
PblHash EntityHologram::rttiHashEntityHologram("EntityHologram");
// GLOBAL: SWBF2 0x00789280
// EntityHologram::rttiNameEntityHologram
const char EntityHologram::rttiNameEntityHologram[] = "EntityHologram";

// SYNTHETIC: SWBF2 0x00402b50
// `dynamic initializer for 'EntityHologramClass::rttiHashEntityHologramClass''
// EntityHologramClass::rttiHashEntityHologramClass

// GLOBAL: SWBF2 0x01ebbdcc
// EntityHologramClass::rttiHashEntityHologramClass
PblHash EntityHologramClass::rttiHashEntityHologramClass("EntityHologramClass");
// GLOBAL: SWBF2 0x00789264
// EntityHologramClass::rttiNameEntityHologramClass
const char EntityHologramClass::rttiNameEntityHologramClass[] = "EntityHologramClass";

// SYNTHETIC: SWBF2 0x0042cf30
// PblHandled::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0042cf60
void FLEffectObject::StopAndFinish()
{
    m_uiAttachFlags |= KILL_ON_NEXT_UPDATE;
    if (!IsThreadActive())
    {
        ActivateThread(0, PER_FRAME);
    }
}

// FUNCTION: SWBF2 0x0042cf80
void FLEffectObject::UpdatedWorldMatrix()
{
}

// SYNTHETIC: SWBF2 0x0042cf90
// FLEffectObject::`scalar deleting destructor'




















// FUNCTION: SWBF2 0x004be160
void LightBeam::AddToRenderList()
{
}

// FUNCTION: SWBF2 0x004be170
void LightBeam::RemoveFromRenderList()
{
}


// SYNTHETIC: SWBF2 0x004c08ea
// ??_ELightBeam@@WCI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004c08fa
// ??_ELightBeam@@WBA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004be180
// LightBeam::`scalar deleting destructor'







// FUNCTION: SWBF2 0x004be200
bool EntityHologram::MyThread::Update(float fDeltaTime)
{
    _pOwner->Update(fDeltaTime);
    return true;
}




// SYNTHETIC: SWBF2 0x004c08f2
// ??_EEntityHologram@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x004be270
// EntityHologram::`scalar deleting destructor'



// SYNTHETIC: SWBF2 0x004be2b0
// EntityHologram::MyThread::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004be2e0
EntityHologram::EntityHologram(EntityHologramClass* pClass, const EntityDesc& desc)
    : EntityEx(desc.mNameId, pClass)
    , RedSceneObject(&noloddata)
{
    m_WorldMatrix = desc.mMatrix;
    m_iTeam = 0;
    m_pLightBeam = 0;
    m_apOmniLight[0] = 0;

    if (pClass->m_fBeamIntensity > 0.0f || pClass->m_fFlareIntensity > 0.0f)
    {
        m_pLightBeam = new LightBeam();
        m_pLightBeam->SetLength(pClass->m_fHeight);
        float fFadeLength = pClass->m_fHeight * kfHoloBeamFadeLengthFactor;
        if (fFadeLength < kfHoloBeamFadeLength)
        {
            m_pLightBeam->SetFadeLength(fFadeLength);
        }
        else
        {
            m_pLightBeam->SetFadeLength(kfHoloBeamFadeLength);
        }
        m_pLightBeam->SetDrawDistance(pClass->m_fVisibleDistance);
        m_pLightBeam->SetInitialWidth(pClass->m_fBeamInitialWidth);
        m_pLightBeam->SetFinalWidth(pClass->m_fSize * kfHoloBeamWidthCone);
        m_pLightBeam->SetFadeFactor(kfHoloBeamFadeFactor);
        m_pLightBeam->SetFlareIntensity(pClass->m_fFlareIntensity);
        m_pLightBeam->AttachEffectToMatrix(&m_WorldMatrix, false);
    }

    if (pClass->m_fLightIntensity > 0.0f && pClass->m_fLightRadius > 0.0f)
    {
        m_apOmniLight[0] = new RedOmniLight(m_WorldMatrix.GetTranslation(), pClass->m_fLightRadius,
                                            ToColorValue(pClass->m_color));
    }

    if (pClass->m_eType == TYPE_ICON_ALIGNED)
    {
        if (pClass->m_fLightIntensity > 0.0f && pClass->m_fLightRadius > 0.0f)
        {
            m_apOmniLight[0]->SetColor(ToColorValue(RedColor::BLACK));
            for (unsigned int i = 1; i < CameraManager::Instance()->GetNumCameras(); ++i)
            {
                m_apOmniLight[0]->m_uiHideInView |= 1 << i;
                m_apOmniLight[i] = new RedOmniLight(m_WorldMatrix.GetTranslation(),
                                                    pClass->m_fLightRadius,
                                                    ToColorValue(RedColor::BLACK));
                for (unsigned int j = 0; j < CameraManager::Instance()->GetNumCameras(); ++j)
                {
                    if (j != i)
                    {
                        m_apOmniLight[i]->m_uiHideInView |= 1 << j;
                    }
                }
            }
        }
        SetTeam(0);
    }
    else
    {
        SetColor(pClass->m_color);
    }

    m_fAspect = 1.0f;
    m_fFlickerTimer = 0.0f;
    m_fRotateTimer = 0.0f;
    m_fPopTimer = 0.0f;
    m_fFadeInTimer = 0.0f;
    m_fTilt = 0.0f;
    m_fCurrentIntensity = 0.0f;
    m_fCurrentFadeInFactor = 0.0f;

    float fRadius = pClass->m_fSize * 0.5f + pClass->m_fHeight * 0.5f;
    PblVector3 vCenter;
    {
        PblVector3 vOffset(0.0f, fRadius, 0.0f);
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vCenter), reinterpret_cast<const D3DXVECTOR3*>(&vOffset), reinterpret_cast<const D3DXMATRIX*>(&m_WorldMatrix));
    }

    PblMatrix mReflection;
    if (FLRenderer::IsReflected(vCenter, fRadius, mReflection, true))
    {
        PblVector3 vReflected;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vReflected), reinterpret_cast<const D3DXVECTOR3*>(&vCenter), reinterpret_cast<const D3DXMATRIX*>(&mReflection));
        float fDX = vCenter.x - vReflected.x;
        float fDY = vCenter.y - vReflected.y;
        float fDZ = vCenter.z - vReflected.z;
        float fDist = PblMath::Sqrt(fDX * fDX + fDY * fDY + fDZ * fDZ);
        if (fDist > fRadius * 2.0f)
        {
            vCenter = Interpolate(vCenter, vReflected, fRadius / fDist);
            fRadius += fRadius;
        }
        else
        {
            vCenter = PblVector3((vReflected.x + vCenter.x) * 0.5f, (vReflected.y + vCenter.y) * 0.5f,
                                 (vReflected.z + vCenter.z) * 0.5f);
            fRadius = fDist * 0.5f + fRadius;
        }
        SetRenderPos(vCenter);
    }
    else
    {
        SetRenderPos(vCenter);
        fRadius *= 1.1f;
    }
    SetRenderRadius(fRadius);
    AddToScene();

    _Thread.SetOwner(this);
    SetModel(pClass->m_pHoloModel);
    m_bTurnOn = true;
    m_bActive = false;
    m_uiHideInView = 0;
    SetIntensity(1.0f);
}

// FUNCTION: SWBF2 0x004bea60
EntityHologram::~EntityHologram()
{
    static_cast<RedSceneObject*>(this)->Deactivate();
    delete m_apOmniLight[0];
    m_apOmniLight[0] = 0;
    delete m_pLightBeam;
}

// FUNCTION: SWBF2 0x004beb00
bool EntityHologram::Update(float fDeltaTime)
{
    const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);

    m_fRotateTimer += fDeltaTime;
    m_fPopTimer += fDeltaTime;
    m_fFlickerTimer += fDeltaTime;
    m_fFadeInTimer += fDeltaTime;

    float fReturn = fDeltaTime * 2.0f;
    if (fReturn > 0.8f)
    {
        fReturn = 0.8f;
    }
    m_fAspect = (1.0f - m_fAspect) * fReturn + m_fAspect;

    if (PblRandom::_Global.GetFloat() < pClass->m_fPopRate * m_fPopTimer * 0.5f)
    {
        float fPop = PblRandom::_Global.GetFloat();
        if (fPop < kfHoloPopAspectProbability)
        {
            float fAspect = PblRandom::_Global.GetFloat();
            if (fAspect < 0.5f)
            {
                fAspect *= 2.0f;
                m_fAspect = (1.0f - fAspect) * 0.5f + fAspect;
            }
            else
            {
                fAspect *= 2.0f;
                m_fAspect = (fAspect - 1.0f) + 1.0f;
            }
        }
        if (fPop > kfHoloPopRotateProbability)
        {
            float fTilt = PblRandom::_Global.GetFloat();
            m_fTilt = fTilt * fTilt * 0.017453292f * kfHoloMaxTilt;
            m_fRotateTimer = PblRandom::_Global.GetFloat() * 6.2831855f / pClass->m_fRotateRate;
        }
        m_fPopTimer = 0.0f;
    }

    if (PblRandom::_Global.GetFloat() < pClass->m_fFlickerRate * m_fFlickerTimer * 0.5f)
    {
        m_fFlickerTimer = 0.0f;
    }

    m_fCurrentFadeInFactor = 1.0f;
    if (m_fFadeInTimer < pClass->m_fFadeInTime)
    {
        float fFade = m_fFadeInTimer / pClass->m_fFadeInTime;
        if (m_bTurnOn)
        {
            m_fCurrentFadeInFactor = fFade;
        }
        else
        {
            m_fCurrentFadeInFactor = 1.0f - fFade;
        }
    }
    else if (!m_bTurnOn)
    {
        m_fCurrentFadeInFactor = 0.0f;
        if (m_bActive)
        {
            if (m_pLightBeam)
            {
                m_pLightBeam->DeactivateEffect();
            }
            if (m_apOmniLight[0])
            {
                m_apOmniLight[0]->Deactivate();
            }
            _Thread.DeactivateThread();
            Deactivate();
            m_bActive = false;
        }
    }

    m_fCurrentIntensity = m_fIntensity * m_fCurrentFadeInFactor;
    if (m_fFlickerTimer <= 0.0f)
    {
        m_fCurrentIntensity =
            ((pClass->m_fFlickerAlphaMax - pClass->m_fFlickerAlphaMin) * PblRandom::_Global.GetFloat() +
             pClass->m_fFlickerAlphaMin) * m_fCurrentIntensity;
    }

    const int iType = pClass->m_eType;
    if (iType == TYPE_ICON_BUILD)
    {
        CameraManager* pCameraManager = CameraManager::Instance();
        unsigned int uView = pCameraManager->m_hView[0];
        if (uView < 1)
        {
            unsigned char ucBit = (unsigned char)(1 << uView);
            if (!(pCameraManager->m_uiVisibleIconMask[0] & 1))
            {
                m_uiHideInView |= ucBit;
            }
            else
            {
                m_uiHideInView &= ~ucBit;
            }
            m_pLightBeam->HideInView(uView, true);
            if (m_apOmniLight[0])
            {
                m_apOmniLight[0]->m_uiHideInView |= ucBit;
            }
        }

        if (m_apOmniLight[0])
        {
            m_apOmniLight[0]->SetColor(ToColorValue(
                Interpolate(RedColor::BLACK, m_color, pClass->m_fLightIntensity * m_fCurrentIntensity)));
        }
        if (m_pLightBeam)
        {
            RedColor beamColor = m_color;
            beamColor.a = (unsigned char)(int)(pClass->m_fBeamIntensity * m_fCurrentIntensity * m_color.a);
            m_pLightBeam->SetColor(beamColor);
            m_pLightBeam->SetFinalWidth(pClass->m_fSize * kfHoloBeamWidthCone * m_fCurrentFadeInFactor *
                                        m_fAspect);
        }
    }
    else
    {
        m_uiHideInView = 0;
        if (iType == TYPE_ICON_ALIGNED)
        {
            int iPlayer = NetGame::GetLocalIndex(0);
            int iSpectator = NetGame::GetSpectator(iPlayer);
            if (iSpectator >= 0)
            {
                iPlayer = iSpectator;
            }
            Character* pCharacter = Character::FindPlayer(iPlayer);
            Team* pTeam = pCharacter ? pCharacter->mTeamPtr : Team::sTeams[1];

            RedColor teamColor = pTeam->mColor[m_iTeam];
            if (m_apOmniLight[0])
            {
                m_apOmniLight[0]->SetColor(ToColorValue(
                    Interpolate(RedColor::BLACK, teamColor, pClass->m_fLightIntensity * m_fCurrentIntensity)));
            }
            if (m_pLightBeam)
            {
                RedColor beamColor = m_color;
                beamColor.a = (unsigned char)(int)(pClass->m_fBeamIntensity * m_fCurrentIntensity * m_color.a);
                m_pLightBeam->SetColor(beamColor);
                m_pLightBeam->SetFinalWidth(pClass->m_fSize * kfHoloBeamWidthCone * m_fCurrentFadeInFactor *
                                            m_fAspect);
            }
        }
        else
        {
            if (m_apOmniLight[0])
            {
                m_apOmniLight[0]->SetColor(ToColorValue(
                    Interpolate(RedColor::BLACK, m_color, pClass->m_fLightIntensity * m_fCurrentIntensity)));
            }
            if (m_pLightBeam)
            {
                RedColor beamColor = m_color;
                beamColor.a = (unsigned char)(int)(pClass->m_fBeamIntensity * m_fCurrentIntensity * m_color.a);
                m_pLightBeam->SetColor(beamColor);
                m_pLightBeam->SetFinalWidth(pClass->m_fSize * kfHoloBeamWidthCone * m_fCurrentFadeInFactor *
                                            m_fAspect);
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x004bf620
void EntityHologram::SetTeam(int iTeam)
{
    m_iTeam = iTeam;
}

// FUNCTION: SWBF2 0x004bf630
void EntityHologram::SetColor(const RedColor& color)
{
    m_color = color;
    const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);
    if (m_pLightBeam)
    {
        RedColor beamColor = m_color;
        beamColor.a = (unsigned char)(int)(m_fCurrentIntensity * pClass->m_fBeamIntensity * 255.0f);
        m_pLightBeam->SetColor(beamColor);
    }
    if (pClass->m_eType != TYPE_ICON_ALIGNED && m_apOmniLight[0])
    {
        RedColor lightColor = m_color;
        lightColor.a = (unsigned char)(int)(m_fCurrentIntensity * pClass->m_fLightIntensity * 255.0f);
        m_apOmniLight[0]->SetColor(ToColorValue(lightColor));
    }
}

// FUNCTION: SWBF2 0x004bf780
void EntityHologram::SetModel(const RedModel* pModel)
{
    m_pHoloModel = const_cast<RedModel*>(pModel);
}

// FUNCTION: SWBF2 0x004bf790
void EntityHologram::SetIntensity(float fIntensity)
{
    if (fIntensity <= 0.0f)
    {
        m_fIntensity = 0.0f;
        m_fCurrentIntensity = 0.0f;
        if (m_bActive)
        {
            if (m_pLightBeam)
            {
                m_pLightBeam->DeactivateEffect();
            }
            if (m_apOmniLight[0])
            {
                m_apOmniLight[0]->Deactivate();
            }
            _Thread.DeactivateThread();
            Deactivate();
            m_bActive = false;
        }
    }
    else
    {
        m_fIntensity = fIntensity;
        const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);
        if ((m_bTurnOn || pClass->m_fFadeInTime > m_fFadeInTimer) && !m_bActive)
        {
            if (m_pLightBeam)
            {
                m_pLightBeam->ActivateEffect();
            }
            if (m_apOmniLight[0])
            {
                m_apOmniLight[0]->Activate();
            }
            _Thread.ActivateThread(0, PER_FRAME);
            Activate();
            m_bActive = true;
        }
    }
}

// FUNCTION: SWBF2 0x004bf880
void EntityHologram::TurnOn()
{
    if (m_bTurnOn)
    {
        return;
    }

    const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);
    if (pClass->m_fFadeInTime > m_fFadeInTimer)
    {
        m_fFadeInTimer = pClass->m_fFadeInTime - m_fFadeInTimer;
    }
    else
    {
        m_fFadeInTimer = 0.0f;
    }

    if (m_fIntensity > 0.0f && !m_bActive)
    {
        if (m_pLightBeam)
        {
            m_pLightBeam->ActivateEffect();
        }
        if (m_apOmniLight[0])
        {
            m_apOmniLight[0]->Activate();
        }
        _Thread.ActivateThread(0, PER_FRAME);
        Activate();
        m_bActive = true;
    }
    m_bTurnOn = true;
}

// FUNCTION: SWBF2 0x004bf930
void EntityHologram::TurnOff()
{
    if (!m_bTurnOn)
    {
        return;
    }

    const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);
    if (pClass->m_fFadeInTime > m_fFadeInTimer)
    {
        m_fFadeInTimer = pClass->m_fFadeInTime - m_fFadeInTimer;
    }
    else
    {
        m_fFadeInTimer = 0.0f;
    }
    m_bTurnOn = false;
}

// FUNCTION: SWBF2 0x004bf980
void EntityHologram::TurnOnNow()
{
    TurnOff();
    const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);
    m_fFadeInTimer = pClass->m_fFadeInTime;
}

// FUNCTION: SWBF2 0x004bf9a0
void EntityHologram::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);
    unsigned int uiViewport = FLRenderer::GetCurrentViewport();
    if (m_uiHideInView & (1 << uiViewport))
    {
        return;
    }

    PblMatrix mReflection;

    bool bReflected = !(uiFlags & 0x200000) &&
                      FLRenderer::IsReflected(*reinterpret_cast<const PblVector3*>(&_Sphere), _Sphere.w,
                                              mReflection, false);

    int iCamera = CameraManager::Instance()->GetCameraIdForView(uiViewport);
    RedColor color;
    if (pClass->m_eType == TYPE_ICON_ALIGNED)
    {
        int iPlayer = NetGame::GetLocalIndex(iCamera);
        int iSpectator = NetGame::GetSpectator(iPlayer);
        if (iSpectator >= 0)
        {
            iPlayer = iSpectator;
        }
        Character* pCharacter = Character::FindPlayer(iPlayer);
        Team* pTeam = pCharacter ? pCharacter->mTeamPtr : Team::sTeams[1];
        color = pTeam->GetColor(m_iTeam);
    }
    else
    {
        color = m_color;
    }

    if (m_pLightBeam)
    {
        RedColor beamColor = color;
        beamColor.a = (unsigned char)(int)(m_fCurrentIntensity * pClass->m_fBeamIntensity * color.a);
        m_pLightBeam->SetColor(beamColor);
        m_pLightBeam->Render(uiFlags);
        if (bReflected)
        {
            PblMatrix mSaved(m_WorldMatrix);
            m_WorldMatrix *= mReflection;
            m_pLightBeam->AttachEffectToMatrix(&m_WorldMatrix, false);
            m_pLightBeam->Render(uiFlags | 0x10000100);
            m_WorldMatrix = mSaved;
            m_pLightBeam->AttachEffectToMatrix(&m_WorldMatrix, false);
        }
    }

    if (m_pHoloModel && m_fCurrentIntensity * pClass->m_fImageIntensity > 0.0f)
    {
        const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
        const PblVector3& vPos = m_WorldMatrix.GetTranslation();
        float fDX = vCamera.x - vPos.x;
        float fDY = vCamera.y - vPos.y;
        float fDZ = vCamera.z - vPos.z;
        float fDistSq = fDY * fDY + fDX * fDX + fDZ * fDZ;
        float fVisibleSq = pClass->m_fVisibleDistance * pClass->m_fVisibleDistance;
        if (fDistSq < fVisibleSq)
        {
            float fFade = 1.0f;
            if (fDistSq > kfFadeDrawDistanceSquared * fVisibleSq)
            {
                fFade = (1.0f - fDistSq / fVisibleSq) * kfFadeDrawDistanceFactor;
            }
            float fInvAspect = 1.0f / m_fAspect;
            float fSize = m_fCurrentFadeInFactor * pClass->m_fSize;

            PblMatrix mModel(gMatrixIdentity);
            PblMatrix mTemp;
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mModel), reinterpret_cast<const D3DXMATRIX*>(&mModel),
                               D3DXMatrixRotationX(reinterpret_cast<D3DXMATRIX*>(&mTemp), m_fTilt));
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mModel), reinterpret_cast<const D3DXMATRIX*>(&mModel),
                               D3DXMatrixRotationY(reinterpret_cast<D3DXMATRIX*>(&mTemp), pClass->m_fRotateRate * m_fRotateTimer));

            mModel.m[3][0] += 0.0f;
            mModel.m[3][1] += pClass->m_fHeight;
            mModel.m[3][2] += 0.0f;
            float fScaleXZ = m_fAspect * fSize;
            float fScaleY = fSize * fInvAspect;
            mModel[0] = mModel[0] * fScaleXZ;
            mModel.m[0][3] = 0.0f;
            mModel[1] = mModel[1] * fScaleY;
            mModel.m[1][3] = 0.0f;
            mModel[2] = mModel[2] * fScaleXZ;
            mModel.m[2][3] = 0.0f;
            mModel *= m_WorldMatrix;

            float fNear, fUnused;
            RedRenderer::GetCamera()->GetFrustum(fNear, fUnused, fUnused, fUnused);
            const PblVector3& vZ = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
            const PblVector3& vEye = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
            const PblVector3 vToModel = mModel.GetTranslation() - vEye;
            float fDepth = -fNear - (vZ.y * vToModel.y + vZ.x * vToModel.x + vZ.z * vToModel.z);
            if (fDepth > 0.0f)
            {
                if (fSize > fDepth)
                {
                    fFade *= fDepth / fSize;
                }
                color.a = (unsigned char)(int)(color.a * m_fCurrentIntensity * pClass->m_fImageIntensity * fFade);
                m_pHoloModel->Render(&mModel, 0, color, uiFlags | 0x10, 0);
                if (bReflected)
                {
                    D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mTemp), reinterpret_cast<const D3DXMATRIX*>(&mModel), reinterpret_cast<const D3DXMATRIX*>(&mReflection));
                    mReflection = mTemp;
                    m_pHoloModel->Render(&mReflection, 0, color, (uiFlags & ~0x10000u) | 0x10000110, 0);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x004bffc0
void EntityHologram::Init()
{
    m_color = RedColor::WHITE;
    m_pHoloModel = 0;
    m_fFadeInTimer = 0.0f;
    m_fFlickerTimer = 0.0f;
    m_fPopTimer = 0.0f;
    m_fRotateTimer = 0.0f;
    m_fAspect = 1.0f;
    m_fCurrentIntensity = 0.0f;
    m_fIntensity = 0.0f;
    m_fTilt = 0.0f;
    SetIntensity(0.0f);
}

// FUNCTION: SWBF2 0x004c0030
void EntityHologram::Deinit()
{
    SetIntensity(0.0f);
}

// FUNCTION: SWBF2 0x004bffb0
void EntityHologram::Eliminate()
{
    Deinit();
}

// FUNCTION: SWBF2 0x004c0040
void EntityHologram::SetStoredMatrix(const PblMatrix& mMatrix)
{
    m_WorldMatrix = mMatrix;
    if (m_pLightBeam)
    {
        m_pLightBeam->AttachEffectToMatrix(&m_WorldMatrix, false);
    }
    if (m_apOmniLight[0])
    {
        m_apOmniLight[0]->SetPosition(m_WorldMatrix.GetTranslation());
    }

    const EntityHologramClass* pClass = static_cast<const EntityHologramClass*>(mEntityClass);
    float fRadius = pClass->m_fSize * 0.5f + pClass->m_fHeight * 0.5f;
    PblVector3 vCenter;
    {
        PblVector3 vOffset(0.0f, fRadius, 0.0f);
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vCenter), reinterpret_cast<const D3DXVECTOR3*>(&vOffset), reinterpret_cast<const D3DXMATRIX*>(&m_WorldMatrix));
    }

    PblMatrix mReflection;
    if (FLRenderer::IsReflected(vCenter, fRadius, mReflection, true))
    {
        PblVector3 vReflected;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vReflected), reinterpret_cast<const D3DXVECTOR3*>(&vCenter), reinterpret_cast<const D3DXMATRIX*>(&mReflection));
        float fDX = vCenter.x - vReflected.x;
        float fDY = vCenter.y - vReflected.y;
        float fDZ = vCenter.z - vReflected.z;
        float fDist = PblMath::Sqrt(fDX * fDX + fDY * fDY + fDZ * fDZ);
        if (fDist > fRadius * 2.0f)
        {
            vCenter = Interpolate(vCenter, vReflected, fRadius / fDist);
            fRadius += fRadius;
        }
        else
        {
            vCenter = PblVector3((vReflected.x + vCenter.x) * 0.5f, (vReflected.y + vCenter.y) * 0.5f,
                                 (vReflected.z + vCenter.z) * 0.5f);
            fRadius = fDist * 0.5f + fRadius;
        }
        SetRenderPos(vCenter);
    }
    else
    {
        SetRenderPos(vCenter);
        fRadius *= 1.1f;
    }
    SetRenderRadius(fRadius);
}

// SYNTHETIC: SWBF2 0x004c0390
// EntityHologramClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004c02a0
EntityHologramClass::EntityHologramClass(unsigned int uHash)
    : EntityClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    m_pHoloModel = 0;
    m_color = RedColor::WHITE;
    m_fVisibleDistance = 200.0f;
    m_fHeight = 10.0f;
    m_fSize = 2.0f;
    m_fBeamInitialWidth = 1.0f;
    m_fBeamIntensity = 1.0f;
    m_fLightIntensity = 1.0f;
    m_fImageIntensity = 0.5f;
    m_fFlareIntensity = 0.0f;
    m_fLightRadius = 10.0f;
    m_fRotateRate = PblMath::fTwoPi * 0.5f;
    m_fPopRate = 0.03f;
    m_fFlickerRate = 0.2f;
    m_fFlickerAlphaMin = 0.4f;
    m_fFlickerAlphaMax = 0.8f;
    m_fFadeInTime = 2.0f;
    m_fTurnOnDistance = 200.0f;
    m_eType = 0;
}

// FUNCTION: SWBF2 0x004c03c0
EntityHologramClass::EntityHologramClass(EntityHologramClass* pParent, unsigned int uHash)
    : EntityClass(pParent, uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    m_pHoloModel = pParent->m_pHoloModel;
    m_color = pParent->m_color;
    m_fVisibleDistance = pParent->m_fVisibleDistance;
    m_fHeight = pParent->m_fHeight;
    m_fSize = pParent->m_fSize;
    m_fBeamInitialWidth = pParent->m_fBeamInitialWidth;
    m_fBeamIntensity = pParent->m_fBeamIntensity;
    m_fLightIntensity = pParent->m_fLightIntensity;
    m_fImageIntensity = pParent->m_fImageIntensity;
    m_fFlareIntensity = pParent->m_fFlareIntensity;
    m_fLightRadius = pParent->m_fLightRadius;
    m_fRotateRate = pParent->m_fRotateRate;
    m_fPopRate = pParent->m_fPopRate;
    m_fFlickerRate = pParent->m_fFlickerRate;
    m_fFlickerAlphaMin = pParent->m_fFlickerAlphaMin;
    m_fFlickerAlphaMax = pParent->m_fFlickerAlphaMax;
    m_fFadeInTime = pParent->m_fFadeInTime;
    m_fTurnOnDistance = pParent->m_fTurnOnDistance;
    m_eType = pParent->m_eType;

    sHologramClassTable.Store(uHash, this);
}

// FUNCTION: SWBF2 0x004c0500
static RedColor ReadColor(const char* pszValue)
{
    unsigned int r = 255, g = 255, b = 255, a = 255;
    if (sscanf(pszValue, "%u %u %u %u", &r, &g, &b, &a) < 3)
    {
        return RedColor::BLACK;
    }
    RedColor color;
    color.b = (unsigned char)b;
    color.g = (unsigned char)g;
    color.r = (unsigned char)r;
    color.a = (unsigned char)a;
    return color;
}

// FUNCTION: SWBF2 0x004c0590
void EntityHologramClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x02565BD1u:
        if (!_strnicmp(pszValue, "aligned", 5))
        {
            m_eType = TYPE_ICON_ALIGNED;
        }
        else if (!_strnicmp(pszValue, "build", 5))
        {
            m_eType = TYPE_ICON_BUILD;
        }
        break;
    case 0x09F126C2u:
        sscanf(pszValue, "%f", &m_fVisibleDistance);
        break;
    case 0x186A4586u:
        sscanf(pszValue, "%f", &m_fPopRate);
        break;
    case 0x1D7B0711u:
        sscanf(pszValue, "%f", &m_fLightRadius);
        break;
    case 0x1DE206A6u:
        sscanf(pszValue, "%f", &m_fBeamInitialWidth);
        break;
    case 0x3D60BCC8u:
        sscanf(pszValue, "%f", &m_fSize);
        break;
    case 0x3D7E6258u:
        m_color = ReadColor(pszValue);
        break;
    case 0x4BF3F668u:
        sscanf(pszValue, "%f", &m_fTurnOnDistance);
        break;
    case 0x4CECAC28u:
        sscanf(pszValue, "%f", &m_fFlareIntensity);
        break;
    case 0x543722AFu:
        sscanf(pszValue, "%f", &m_fImageIntensity);
        break;
    case 0x78FD34B7u:
        sscanf(pszValue, "%f", &m_fFlickerAlphaMin);
        break;
    case 0x8710D279u:
        sscanf(pszValue, "%f", &m_fFlickerAlphaMax);
        break;
    case 0x9FE971E5u:
        sscanf(pszValue, "%f", &m_fFadeInTime);
        break;
    case 0xA0F13D29u:
        sscanf(pszValue, "%f", &m_fFlickerRate);
        break;
    case 0xAFF1F98Cu:
        sscanf(pszValue, "%f", &m_fRotateRate);
        m_fRotateRate *= PblMath::fTwoPi;
        break;
    case 0xB64210D2u:
        m_pHoloModel = RedModel::_HashTable.Find(PblHash(pszValue));
        break;
    case 0xB761134Eu:
        sscanf(pszValue, "%f", &m_fHeight);
        break;
    case 0xF0AC6DE0u:
        sscanf(pszValue, "%f", &m_fLightIntensity);
        break;
    case 0xF31C745Fu:
        sscanf(pszValue, "%f", &m_fBeamIntensity);
        break;
    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004c07d0
Entity* EntityHologramClass::Build(const EntityDesc& desc)
{
    return new EntityHologram(this, desc);
}

// FUNCTION: SWBF2 0x004c0840
EntityHologramClass* EntityHologramClass::Find(unsigned int uHash)
{
    return sHologramClassTable.Find(uHash);
}

// FUNCTION: SWBF2 0x004c0860
EntityClass* EntityHologramClass::Derive(unsigned int uHash)
{
    return new EntityHologramClass(this, uHash);
}

// FUNCTION: SWBF2 0x004c08d0
void EntityHologramClass::Reset()
{
    sHologramClassTable.Clear();
}

// SYNTHETIC: SWBF2 0x00402b60
// `dynamic initializer for 'EntityHologramClass::sHologramClassTable''

// GLOBAL: SWBF2 0x01ebbcc8
PblHashTable<EntityHologramClass, 32> EntityHologramClass::sHologramClassTable;
