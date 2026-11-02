#include "FLEffect.h"

#include <d3dx9math.h>

#include "CollisionObject.h"
#include "Entity.h"
#include "GameModel.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"

#include "BlurEffect.h"
#include "ColorControl.h"
#include "Decal.h"
#include "EntityLight.h"
#include "Godray.h"
#include "HDREffect.h"
#include "HeatShimmerEffect.h"
#include "LensFlare.h"
#include "LightBeam.h"
#include "LightningBoltEffect.h"
#include "LightningEffect.h"
#include "LightsaberTrailEffect.h"
#include "MotionBlurEffect.h"
#include "PE_ChunkLoader.h"
#include "ScopeBlurEffect.h"
#include "ScreenFilterEffect.h"
#include "ShadowEffect.h"
#include "ShieldEffect.h"
#include "SpaceDustEffect.h"
#include "SunFlare.h"
#include "WaterEffect.h"
#include "WeatherEffect.h"
#include "WorldShadowMapEffect.h"

#include "GameObject.h"

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

extern bool netFrameLock;

extern PblVector3 gVectorUnitX;
extern PblVector3 gVectorUnitY;
extern PblVector3 gVectorUnitZ;

class PblMatrix;
extern PblMatrix gMatrixIdentity;
#define _IdentityMatrix (gMatrixIdentity)

// GLOBAL: SWBF2 0x01e51e26
bool FLEffect::s_bInitRender;

// SYNTHETIC: SWBF2 0x004038f0
// `dynamic initializer for 'FLEffect::s_EffectFactories''

// GLOBAL: SWBF2 0x01ebd020
PblHashTable<FLEffect, 32> FLEffect::s_EffectFactories;

// SYNTHETIC: SWBF2 0x004038d0
// `dynamic initializer for 'FLEffect::_Thread''
// SYNTHETIC: SWBF2 0x00768740
// `dynamic atexit destructor for 'FLEffect::_Thread''

// GLOBAL: SWBF2 0x01ebd124
// FLEffect::_Thread
FLEffect::FLEffectThread FLEffect::_Thread;

// SYNTHETIC: SWBF2 0x00403910
// `dynamic initializer for 'FLEffect::s_EffectClasses''

// GLOBAL: SWBF2 0x01ebd140
PblHashTable<FLEffectClass, 256> FLEffect::s_EffectClasses;

// SYNTHETIC: SWBF2 0x00403930
// `dynamic initializer for 'FLEffectObject::s_OffsetMatrixPool''
// SYNTHETIC: SWBF2 0x00768750
// `dynamic atexit destructor for 'FLEffectObject::s_OffsetMatrixPool''

// GLOBAL: SWBF2 0x01ebd960
// FLEffectObject::s_OffsetMatrixPool
MemoryPool FLEffectObject::s_OffsetMatrixPool("FLEffectObject::OffsetMatrix", sizeof(PblMatrix));

// FUNCTION: SWBF2 0x00526b20
void FLEffect::InitAll(bool bMission)
{
    FLEffectObject::Init();

    LightBeamEffect::Create();
    LensFlareEffect::Create();
    SunFlareEffect::Create();
    FogCloudEffect::Create();
    PrecipitationEffect::Create();
    WindEffect::Create();
    LightningEffect::Create();
    WaterEffect::Create();
    GodrayManager::Create();
    BlurEffect::Create();
    ColorControl::Create();
    LightningBoltEffect::Create();
    WorldShadowMapEffect::Create();
    MotionBlurEffect::Create();
    ScopeBlurEffect::Create();
    HeatShimmerEffect::Create();
    DecalManager::Create();
    ShieldEffectManager::Create();
    ScreenFilterEffect::Create();
    SpaceDustEffect::Create();
    LightsaberTrailEffect::Create();
    HDREffect::Create();
    ShadowEffect::Create();
    EntityLightFXParser::Create();
    ParticleEmitterEffect::Create();

    s_bInitRender = true;

    for (PblHashTable<FLEffect, 32>::Itor i = s_EffectFactories.Begin();
         i != s_EffectFactories.End(); ++i)
    {
        (*i).Init();
        if ((*i).m_bEnabled)
        {
            (*i).Activate((*i).m_renderPriority);
        }
    }

    _Thread.ActivateThread(0, PER_FRAME);
}

// SYNTHETIC: SWBF2 0x00526af0
// FLEffect::FLEffectThread::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00526c30
void FLEffect::CleanupAll()
{
    _Thread.DeactivateThread();
    s_bInitRender = false;

    for (PblHashTable<FLEffect, 32>::Itor i = s_EffectFactories.Begin();
         i != s_EffectFactories.End(); ++i)
    {
        (*i).Deactivate();
        (*i).Cleanup();
    }

    while (s_EffectClasses.GetNumEntries())
    {
        FLEffectClass* pClass = s_EffectClasses.Begin();
        if (pClass)
        {
            delete pClass;
        }
    }
    s_EffectClasses.Clear();

    ParticleEmitterEffect::Destroy();
    LightBeamEffect::Destroy();
    LensFlareEffect::Destroy();
    SunFlareEffect::Destroy();
    FogCloudEffect::Destroy();
    PrecipitationEffect::Destroy();
    WindEffect::Destroy();
    LightningEffect::Destroy();
    WaterEffect::Destroy();
    GodrayManager::Destroy();
    BlurEffect::Destroy();
    ColorControl::Destroy();
    LightningBoltEffect::Destroy();
    WorldShadowMapEffect::Destroy();
    MotionBlurEffect::Destroy();
    ScopeBlurEffect::Destroy();
    HeatShimmerEffect::Destroy();
    DecalManager::Destroy();
    ShieldEffectManager::Destroy();
    ScreenFilterEffect::Destroy();
    SpaceDustEffect::Destroy();
    LightsaberTrailEffect::Destroy();
    HDREffect::Destroy();
    ShadowEffect::Destroy();
    EntityLightFXParser::Destroy();

    s_EffectFactories.Clear();
}

// FUNCTION: SWBF2 0x00526db0
void FLEffect::Read(PblFileChunk& chunk)
{
    PblConfig config(chunk);
    unsigned int uiDefaultName = config.m_uiNameHash;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        unsigned int uiId = data.GetId();
        if (uiId == 0x6E6E8D54)
        {
            FLEffect* pEffect = s_EffectFactories.Find(PblHash(data.GetStringArg(0)));
            PblConfig scope(config.ReadNextScope(), true);
            if (pEffect)
            {
                pEffect->ReadConfig(scope);
            }
        }
        else
        {
            FLEffect* pEffect = s_EffectFactories.Find(uiId);
            PblConfig scope(config.ReadNextScope(), true);
            if (pEffect)
            {
                FLEffectClass* pClass = pEffect->BuildClass(scope);
                if (pClass)
                {
                    unsigned int uiName = uiDefaultName;
                    if (uiId != 0xCEBB215F && data.GetNumArgs() == 1 &&
                        PblHash(data.GetStringArg(0)))
                    {
                        uiName = PblHash(data.GetStringArg(0));
                    }

                    if (!s_EffectClasses.Find(uiName))
                    {
                        pClass->SetClassName(uiName);
                        s_EffectClasses.Store(uiName, pClass);
                    }
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x00526f50
void FLEffect::RegisterManager(FLEffect* pEffect, unsigned int uiName)
{
    s_EffectFactories.Store(uiName, pEffect);
}

// FUNCTION: SWBF2 0x00526f70
void FLEffect::UnregisterManager(FLEffect* pEffect)
{
    for (PblHashTable<FLEffect, 32>::Itor i = s_EffectFactories.Begin();
         i != s_EffectFactories.End(); ++i)
    {
        if (i == pEffect)
        {
            s_EffectFactories.Remove(i.GetKey());
            return;
        }
    }
}

// FUNCTION: SWBF2 0x00527000
void FLEffect::Enable(bool bEnable)
{
    if (s_bInitRender)
    {
        if (bEnable)
        {
            Activate(m_renderPriority);
        }
        else
        {
            Deactivate();
        }
    }
    m_bEnabled = bEnable;
}

// FUNCTION: SWBF2 0x00527e40
FLEffectClass::FLEffectClass()
{
    mType = 0;
    m_uiClassName = 0;
}

// FUNCTION: SWBF2 0x00527e90
FLEffectClass::~FLEffectClass()
{
    FLEffect::Unregister(m_uiClassName);
}

// FUNCTION: SWBF2 0x00527040
bool FLEffect::FLEffectThread::Update(float fDeltaTime)
{
    for (PblHashTable<FLEffect, 32>::Itor i = s_EffectFactories.Begin();
         i != s_EffectFactories.End(); ++i)
    {
        if ((*i).m_bEnabled)
        {
            (*i).Update(fDeltaTime);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x005270d0
void FLEffect::Unregister(unsigned int uiClassName)
{
    if (s_EffectClasses.Find(uiClassName))
    {
        s_EffectClasses.Remove(uiClassName);
    }
}

// FUNCTION: SWBF2 0x00527110
void FLEffectObject::Init()
{
    s_OffsetMatrixPool.Setup(0x40, 0x40, 0);
}

// SYNTHETIC: SWBF2 0x00433dd0
// ??_EFLEffectObject@@WBI@AEPAXI@Z
// FUNCTION: SWBF2 0x00527130
FLEffectObject::~FLEffectObject()
{
    if (m_pmLocalOffsetMatrix)
    {
        s_OffsetMatrixPool.Free(m_pmLocalOffsetMatrix);
        m_pmLocalOffsetMatrix = 0;
    }
}

// FUNCTION: SWBF2 0x005271b0
void FLEffectObject::ActivateEffect()
{
    if (!IsThreadActive() && (m_uiAttachFlags & (ATTACHED_DYNAMIC | UPDATE_REQUIRED)))
    {
        if (FLEffectObject::Update(0.0f))
        {
            ActivateThread(0, PER_FRAME);
        }
    }
}

// FUNCTION: SWBF2 0x005271f0
void FLEffectObject::DeactivateEffect()
{
    if (IsThreadActive())
    {
        DeactivateThread();
    }
}

// FUNCTION: SWBF2 0x00527210
bool FLEffectObject::Update(float fDeltaTime)
{
    if (m_uiAttachFlags & KILL_ON_NEXT_UPDATE)
    {
        return false;
    }

    if (m_uiAttachFlags & ATTACHED_DYNAMIC)
    {
        if ((m_uiAttachFlags & ATTACHED_TO_GAMEOBJECT) && m_hobjAttachedToObject == 0)
        {
            m_uiAttachFlags = 0;
            m_hobjAttachedToObject = 0;
            m_pmAttachedToMatrix = 0;
            DeactivateEffect();
            return false;
        }
        _UpdateWorldMatrix();
    }
    return true;
}

// FUNCTION: SWBF2 0x00527270
void FLEffectObject::SetLocalOffsetMatrix(const PblMatrix& mOffset)
{
    if (&mOffset == &_IdentityMatrix)
    {
        if (m_pmLocalOffsetMatrix)
        {
            s_OffsetMatrixPool.Free(m_pmLocalOffsetMatrix);
            m_pmLocalOffsetMatrix = 0;
        }
    }
    else
    {
        if (!m_pmLocalOffsetMatrix)
        {
            m_pmLocalOffsetMatrix = (PblMatrix*)s_OffsetMatrixPool.Allocate(sizeof(PblMatrix));
            if (!m_pmLocalOffsetMatrix)
            {
                return;
            }
        }
        *m_pmLocalOffsetMatrix = mOffset;
    }

    if ((m_uiAttachFlags & ATTACHED_TO_GAMEOBJECT) && m_hobjAttachedToObject == 0)
    {
        return;
    }
    _UpdateWorldMatrix();
}

void FLEffectObject::AttachEffectToObject(const EntityGeometry* pObject, const PblMatrix* pMatrix,
                                          const PblMatrix& mOffset, bool bDynamic)
{
    if (!pObject)
    {
        return;
    }

    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    if (pObject->GetGameObject() && pObject->GetGameObject()->mHandleId)
    {
        m_hobjAttachedToObject = PblHandle<GameObject const>(pObject->GetGameObject());
        m_uiAttachFlags |= ATTACHED_TO_GAMEOBJECT;
    }
    else
    {
        _SetObject(pObject);
    }

    m_pmAttachedToMatrix = const_cast<PblMatrix*>(pMatrix);
    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(mOffset);
}

// FUNCTION: SWBF2 0x00527320
void FLEffectObject::AttachEffectToObject(const EntityGeometry* pObject, const PblMatrix* pMatrix,
                                          bool bDynamic)
{
    if (!pObject)
    {
        return;
    }

    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    if (pObject->GetGameObject() && pObject->GetGameObject()->mHandleId)
    {
        m_hobjAttachedToObject = PblHandle<GameObject const>(pObject->GetGameObject());
        m_uiAttachFlags |= ATTACHED_TO_GAMEOBJECT;
    }
    else
    {
        _SetObject(pObject);
    }

    m_pmAttachedToMatrix = const_cast<PblMatrix*>(pMatrix);
    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(_IdentityMatrix);
}

// FUNCTION: SWBF2 0x005273a0
void FLEffectObject::AttachEffectToObject(const EntityGeometry* pObject, unsigned int uiBone,
                                          const PblMatrix& mOffset, bool bDynamic)
{
    if (!pObject)
    {
        return;
    }

    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    if (pObject->GetGameObject() && pObject->GetGameObject()->mHandleId)
    {
        m_hobjAttachedToObject = PblHandle<GameObject const>(pObject->GetGameObject());
        m_uiAttachFlags |= ATTACHED_TO_GAMEOBJECT;
    }
    else
    {
        _SetObject(pObject);
    }

    m_pmAttachedToMatrix = 0;
    if (uiBone)
    {
        if (pObject->GetPose())
        {
            m_pmAttachedToMatrix = const_cast<PblMatrix*>(pObject->GetPose()->Find(uiBone));
        }

        if (!m_pmAttachedToMatrix && pObject->GetModel() && pObject->GetModel()->GetRedModel(0))
        {
            const RedSkeleton* pSkeleton = pObject->GetModel()->GetRedModel(0)->GetSkeleton();
            if (pSkeleton)
            {
                m_pmAttachedToMatrix = pSkeleton->GetBoneMatrixByName(uiBone);
            }
        }
    }

    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(mOffset);
}

// FUNCTION: SWBF2 0x005274a0
void FLEffectObject::AttachEffectToObject(const EntityGeometry* pObject, unsigned int uiBone,
                                          bool bDynamic)
{
    if (!pObject)
    {
        return;
    }

    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    if (pObject->GetGameObject() && pObject->GetGameObject()->mHandleId)
    {
        m_hobjAttachedToObject = PblHandle<GameObject const>(pObject->GetGameObject());
        m_uiAttachFlags |= ATTACHED_TO_GAMEOBJECT;
    }
    else
    {
        _SetObject(pObject);
    }

    m_pmAttachedToMatrix = 0;
    if (uiBone)
    {
        if (pObject->GetPose())
        {
            m_pmAttachedToMatrix = const_cast<PblMatrix*>(pObject->GetPose()->Find(uiBone));
        }

        if (!m_pmAttachedToMatrix && pObject->GetModel() && pObject->GetModel()->GetRedModel(0))
        {
            const RedSkeleton* pSkeleton = pObject->GetModel()->GetRedModel(0)->GetSkeleton();
            if (pSkeleton)
            {
                m_pmAttachedToMatrix = pSkeleton->GetBoneMatrixByName(uiBone);
            }
        }
    }

    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(_IdentityMatrix);
}

// FUNCTION: SWBF2 0x00527590
void FLEffectObject::AttachEffectToObject(const EntityGeometry* pObject, const PblMatrix& mOffset,
                                          bool bDynamic)
{
    if (!pObject)
    {
        return;
    }

    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    if (pObject->GetGameObject() && pObject->GetGameObject()->mHandleId)
    {
        m_hobjAttachedToObject = PblHandle<GameObject const>(pObject->GetGameObject());
        m_uiAttachFlags |= ATTACHED_TO_GAMEOBJECT;
    }
    else
    {
        _SetObject(pObject);
    }

    m_pmAttachedToMatrix = 0;
    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(mOffset);
}

// FUNCTION: SWBF2 0x00527610
void FLEffectObject::AttachEffectToObject(const EntityGeometry* pObject, bool bDynamic)
{
    if (!pObject)
    {
        return;
    }

    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    if (pObject->GetGameObject() && pObject->GetGameObject()->mHandleId)
    {
        m_hobjAttachedToObject = PblHandle<GameObject const>(pObject->GetGameObject());
        m_uiAttachFlags |= ATTACHED_TO_GAMEOBJECT;
    }
    else
    {
        _SetObject(pObject);
    }

    m_pmAttachedToMatrix = 0;
    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(_IdentityMatrix);
}

// FUNCTION: SWBF2 0x005276a0
void FLEffectObject::AttachEffectToMatrix(const PblMatrix* pMatrix, const PblMatrix& mOffset,
                                          bool bDynamic)
{
    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    m_hobjAttachedToObject = 0;
    m_pmAttachedToMatrix = const_cast<PblMatrix*>(pMatrix);
    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(mOffset);
}

// FUNCTION: SWBF2 0x005276e0
void FLEffectObject::AttachEffectToMatrix(const PblMatrix* pMatrix, bool b)
{
    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    m_hobjAttachedToObject = 0;
    m_pmAttachedToMatrix = const_cast<PblMatrix*>(pMatrix);
    if (b)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }
    _Attach(_IdentityMatrix);
}

// FUNCTION: SWBF2 0x00527720
void FLEffectObject::AttachEffectToVector(const PblVector3* pVector, bool b)
{
    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    m_hobjAttachedToObject = 0;
    m_pvAttachedToVector = const_cast<PblVector3*>(pVector);
    m_uiAttachFlags |= ATTACHED_TO_VECTOR;
    if (b)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }

    m_mWorldMatrix[0] = gVectorUnitX;
    m_mWorldMatrix.m[0][3] = 0.0f;
    m_mWorldMatrix[1] = gVectorUnitY;
    m_mWorldMatrix.m[1][3] = 0.0f;
    m_mWorldMatrix[2] = gVectorUnitZ;
    m_mWorldMatrix.m[2][3] = 0.0f;
    _Attach(_IdentityMatrix);
}

// FUNCTION: SWBF2 0x005277c0
void FLEffectObject::AttachEffectToVector(const PblVector3* pVector, const PblVector3& vDir,
                                          bool bDynamic)
{
    m_uiAttachFlags &= ~(ATTACHED_DYNAMIC | ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    m_hobjAttachedToObject = PblHandle<GameObject const>();
    m_pvAttachedToVector = const_cast<PblVector3*>(pVector);
    m_uiAttachFlags |= ATTACHED_TO_VECTOR;
    if (bDynamic)
    {
        m_uiAttachFlags |= ATTACHED_DYNAMIC;
    }

    m_mWorldMatrix = DirectionMatrix(vDir, gVectorUnitY, m_mWorldMatrix.GetTranslation());
    _Attach(_IdentityMatrix);
}

// FUNCTION: SWBF2 0x00527860
void FLEffectObject::SetOrientation(const PblVector3& vDir)
{
    m_mWorldMatrix = DirectionMatrix(vDir, gVectorUnitY, m_mWorldMatrix.GetTranslation());
    UpdatedWorldMatrix();
}

// FUNCTION: SWBF2 0x005278c0
void FLEffectObject::SetMatrix(const PblMatrix& mMatrix)
{
    m_mWorldMatrix = mMatrix;
    UpdatedWorldMatrix();
}

// FUNCTION: SWBF2 0x00527900
void FLEffectObject::SetPosition(const PblVector3& vPos)
{
    m_mWorldMatrix[3] = vPos;
    m_mWorldMatrix.m[3][3] = 1.0f;
    UpdatedWorldMatrix();
}

// FUNCTION: SWBF2 0x00527930
void FLEffectObject::_Attach(const PblMatrix& mOffset)
{
    if (m_uiAttachFlags & ATTACHED_DYNAMIC)
    {
        SetLocalOffsetMatrix(mOffset);
        if (!IsThreadActive())
        {
            ActivateThread(0, PER_FRAME);
        }
        return;
    }

    if (!m_uiAttachFlags && IsThreadActive())
    {
        DeactivateThread();
    }

    if (m_pmLocalOffsetMatrix)
    {
        SetLocalOffsetMatrix(mOffset);
    }
    else if (&mOffset != &_IdentityMatrix)
    {
        m_pmLocalOffsetMatrix = const_cast<PblMatrix*>(&mOffset);
        _UpdateWorldMatrix();
        m_pmLocalOffsetMatrix = 0;
    }
    else
    {
        _UpdateWorldMatrix();
    }

    m_uiAttachFlags &= ~(ATTACHED_TO_GAMEOBJECT | ATTACHED_TO_VECTOR);
    m_hobjAttachedToObject = PblHandle<GameObject const>();
    m_pmAttachedToMatrix = 0;
}

// FUNCTION: SWBF2 0x005279d0
void FLEffectObject::InterpolateWorldMatrix()
{
    if (netFrameLock)
    {
        return;
    }

    if (NetGame::IsEnabled() && _GetObject() && (m_uiAttachFlags & ATTACHED_TO_GAMEOBJECT))
    {
        PblMatrix mSmoothed(m_hobjAttachedToObject->GetSmoothedMatrix());
        PblMatrix mInterpolated = mSmoothed;
        const int iNetId = m_hobjAttachedToObject->mNetUniqueId;
        gameObjectInterpolator.GetMatrix(iNetId, mInterpolated);

        m_mWorldMatrix = m_mWorldMatrix * mSmoothed.GetInverseFast();
        m_mWorldMatrix = m_mWorldMatrix * mInterpolated;
        UpdatedWorldMatrix();
    }
}

// FUNCTION: SWBF2 0x00527b40
void FLEffectObject::_UpdateWorldMatrix()
{
    if (_GetObject())
    {
        if (m_uiAttachFlags & ATTACHED_TO_GAMEOBJECT)
        {
            m_mWorldMatrix = m_hobjAttachedToObject->GetSmoothedMatrix();
        }
        else
        {
            m_mWorldMatrix = _GetObject()->GetMatrix();
        }

        if (m_uiAttachFlags & ATTACHED_TO_VECTOR)
        {
            if (m_pvAttachedToVector)
            {
                PblVector3 vOffset = *m_pvAttachedToVector;
                if (m_pmLocalOffsetMatrix)
                {
                    vOffset.x += m_pmLocalOffsetMatrix->m[3][0];
                    vOffset.y += m_pmLocalOffsetMatrix->m[3][1];
                    vOffset.z += m_pmLocalOffsetMatrix->m[3][2];
                }
                PblMatrix& mWorld = m_mWorldMatrix;
                PblVector3 vWorld;
                D3DXVec3TransformNormal((D3DXVECTOR3*)&vWorld, (const D3DXVECTOR3*)&vOffset,
                                        (const D3DXMATRIX*)&mWorld);
                mWorld[3] += vWorld;
                UpdatedWorldMatrix();
            }
            return;
        }

        if (m_pmAttachedToMatrix)
        {
            m_mWorldMatrix = *m_pmAttachedToMatrix * m_mWorldMatrix;
        }
        if (m_pmLocalOffsetMatrix)
        {
            m_mWorldMatrix = *m_pmLocalOffsetMatrix * m_mWorldMatrix;
        }
        UpdatedWorldMatrix();
    }
    else if (m_uiAttachFlags & ATTACHED_TO_VECTOR)
    {
        if (m_pvAttachedToVector)
        {
            if (m_pmLocalOffsetMatrix)
            {
                m_mWorldMatrix[3] = m_pmLocalOffsetMatrix->GetTranslation() + *m_pvAttachedToVector;
            }
            else
            {
                m_mWorldMatrix[3] = *m_pvAttachedToVector;
            }
            m_mWorldMatrix.m[3][3] = 1.0f;
            UpdatedWorldMatrix();
        }
    }
    else if (m_pmAttachedToMatrix)
    {
        if (m_pmLocalOffsetMatrix)
        {
            m_mWorldMatrix = *m_pmLocalOffsetMatrix * *m_pmAttachedToMatrix;
        }
        else
        {
            m_mWorldMatrix = *m_pmAttachedToMatrix;
        }
        UpdatedWorldMatrix();
    }
}

// FUNCTION: SWBF2 0x00527e10
const EntityGeometry* FLEffectObject::_GetObject() const
{
    return (const EntityGeometry*)m_hobjAttachedToObject.mObject;
}

// FUNCTION: SWBF2 0x00527e20
void FLEffectObject::_SetObject(const EntityGeometry* pObject)
{
    m_hobjAttachedToObject = PblHandle<GameObject const>();
    m_hobjAttachedToObject.mObject = (const GameObject*)pObject;
}

// SYNTHETIC: SWBF2 0x00527e60
// FLEffectClass::`scalar deleting destructor'
