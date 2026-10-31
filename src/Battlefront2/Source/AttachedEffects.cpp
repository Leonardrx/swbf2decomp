#include "AttachedEffects.h"
#include "PebbleFL/Common/PblVector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Entity.h"
#include "EntityGeometry.h"
#include "EntityLight.h"
#include "FLEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"

class GameObject
{
public:

    char mInheritedObjectStorage[0x204];
    unsigned int mHandleId;
};



AttachedEffectsClass::AttachClassData AttachedEffectsClass::s_aAttachClassData[MAX_ATTACHED];
unsigned int AttachedEffectsClass::s_uiNumAttached;
bool AttachedEffectsClass::s_bDynamic;



// FUNCTION: SWBF2 0x00447910
AttachedEffectsClass* AttachedEffectsClass::BuildAttachedEffectsClass()
{
    if (!s_uiNumAttached)
    {
        return 0;
    }

    AttachedEffectsClass* pClass = new AttachedEffectsClass;
    pClass->m_uiNumAttached = s_uiNumAttached;
    pClass->m_aAttachClassData = new AttachClassData[s_uiNumAttached];
    memcpy(pClass->m_aAttachClassData, s_aAttachClassData, s_uiNumAttached * sizeof(AttachClassData));
    pClass->m_bDynamic = s_bDynamic;

    s_uiNumAttached = 0;
    s_aAttachClassData[0].bIsOdf = 0;
    s_aAttachClassData[0].pOdfClass = 0;
    s_bDynamic = false;
    return pClass;
}

// FUNCTION: SWBF2 0x00446e60
AttachedEffects::AttachedEffects(const AttachedEffectsClass* pClass, const EntityGeometry* pGeometry,
                                 bool bDynamic)
    : m_pClass(const_cast<AttachedEffectsClass*>(pClass)),
      m_pGameObject(pGeometry->GetGameObject()),
      m_pEntityGeometry(0),
      m_bDynamic(bDynamic)
{
    if (!(const GameObject*)m_pGameObject)
    {
        m_pEntityGeometry = const_cast<EntityGeometry*>(pGeometry);
        if (!bDynamic)
        {
            unsigned int i = 0;
            while (i < m_pClass->m_uiNumAttached &&
                   (m_pClass->m_aAttachClassData[i].bIsOdf ||
                    m_pClass->m_aAttachClassData[i].uiNumParams < 1))
            {
                ++i;
            }
            if (i == m_pClass->m_uiNumAttached)
            {
                return;
            }
        }
    }
    ActivateThread((int)0x80000002, PER_FRAME);
}

// FUNCTION: SWBF2 0x00447220
AttachedEffects* AttachedEffectsClass::BuildEffects(const EntityGeometry* pGeometry) const
{
    if (!m_uiNumAttached)
    {
        return 0;
    }

    AttachedEffects* pEffects = new AttachedEffects(this, pGeometry, m_bDynamic);
    pEffects->m_aAttachData = new AttachedEffects::AttachData[m_uiNumAttached];

    for (unsigned int i = 0; i < m_uiNumAttached; ++i)
    {
        if (m_aAttachClassData[i].bIsOdf)
        {
            EntityDesc desc;
            desc.mMatrix = pGeometry->GetMatrix();
            Entity* pEntity = m_aAttachClassData[i].pOdfClass->Build(desc);
            if (pEntity->IsRtti(EntityLight::GetRtti()))
            {
                pEffects->m_aAttachData[i].pEffect = static_cast<EntityLight*>(pEntity);
            }
            pEffects->m_aAttachData[i].fTimer = 0.0f;
            if (pEffects->m_aAttachData[i].pEffect && m_aAttachClassData[i].uiNumParams >= 1)
            {
                char acBuffer[64];
                sprintf(acBuffer, "%f",
                        m_aAttachClassData[i].afParam[0] > 0.0f ? m_aAttachClassData[i].afParam[0] : 0.0f);
                pEntity->SetProperty(0xE6915CF6, acBuffer);
            }
        }
        else
        {
            bool bDelayed = false;
            if (m_aAttachClassData[i].uiNumParams >= 1)
            {
                const float fMin = m_aAttachClassData[i].afParam[0];
                const float fMax = m_aAttachClassData[i].uiNumParams >= 2 ? m_aAttachClassData[i].afParam[1] : fMin;
                if (fMax > 0.0f && fMin >= 0.0f)
                {
                    pEffects->m_aAttachData[i].fTimer = -(PblRandom::_Global.GetFloat() * fMax);
                    pEffects->m_aAttachData[i].pEffect = 0;
                    bDelayed = true;
                }
            }
            if (!bDelayed)
            {
                pEffects->m_aAttachData[i].fTimer = 0.0f;
                pEffects->m_aAttachData[i].pEffect = m_aAttachClassData[i].pEffectClass->BuildEffect();
            }
        }

        if (pEffects->m_aAttachData[i].pEffect)
        {
            pEffects->m_aAttachData[i].pEffect->AttachEffectToObject(pGeometry, m_aAttachClassData[i].uiBoneName,
                                                                     false);
        }
    }
    return pEffects;
}



// FUNCTION: SWBF2 0x00447670
bool AttachedEffectsClass::SetProperty(unsigned int uiProperty, const char* pszValue)
{
    switch (uiProperty)
    {
    case 0x51e2c845:
        s_bDynamic = atoi(pszValue) != 0;
        return true;

    case 0x3be7b80a:
        if (s_uiNumAttached < MAX_ATTACHED && s_aAttachClassData[s_uiNumAttached].pOdfClass)
        {
            char acBone[256];
            const int iNumRead =
                sscanf(pszValue, "%s %f %f", acBone,
                       &s_aAttachClassData[s_uiNumAttached].afParam[0],
                       &s_aAttachClassData[s_uiNumAttached].afParam[1]);
            if (iNumRead >= 1)
            {
                s_aAttachClassData[s_uiNumAttached].uiBoneName = PblTEMPHash(acBone);
                s_aAttachClassData[s_uiNumAttached].uiNumParams = iNumRead - 1;
                s_aAttachClassData[s_uiNumAttached].uiFlags = 0;
                ++s_uiNumAttached;
                if (s_uiNumAttached < MAX_ATTACHED)
                {
                    s_aAttachClassData[s_uiNumAttached].bIsOdf =
                        s_aAttachClassData[s_uiNumAttached - 1].bIsOdf;
                    s_aAttachClassData[s_uiNumAttached].pOdfClass =
                        s_aAttachClassData[s_uiNumAttached - 1].pOdfClass;
                }
            }
        }
        return true;

    case 0x576b09cd:
        if (s_uiNumAttached < MAX_ATTACHED)
        {
            char acEffect[256];
            char acBone[256];
            if (sscanf(pszValue, "%s %s", acEffect, acBone) >= 2)
            {
                s_aAttachClassData[s_uiNumAttached].bIsOdf = 0;
                s_aAttachClassData[s_uiNumAttached].pEffectClass =
                    FLEffect::s_EffectClasses.Find(PblHash(acEffect));
                if (s_aAttachClassData[s_uiNumAttached].pEffectClass)
                {
                    s_aAttachClassData[s_uiNumAttached].uiBoneName = PblTEMPHash(acBone);
                    s_aAttachClassData[s_uiNumAttached].uiNumParams = 0;
                    s_aAttachClassData[s_uiNumAttached].uiFlags = 0;
                    ++s_uiNumAttached;
                    if (s_uiNumAttached < MAX_ATTACHED)
                    {
                        s_aAttachClassData[s_uiNumAttached].bIsOdf = 0;
                        s_aAttachClassData[s_uiNumAttached].pOdfClass = 0;
                    }
                }
            }
        }
        return true;

    case 0xa9d0d48b:
        if (s_uiNumAttached < MAX_ATTACHED)
        {
            s_aAttachClassData[s_uiNumAttached].bIsOdf = 1;
            EntityClass* pOdfClass = EntityClass::Find(PblHash(pszValue));
            s_aAttachClassData[s_uiNumAttached].pOdfClass = pOdfClass;
            if (pOdfClass && !pOdfClass->IsRtti(EntityLightClass::GetRtti()))
            {
                s_aAttachClassData[s_uiNumAttached].bIsOdf = 0;
                s_aAttachClassData[s_uiNumAttached].pOdfClass = 0;
            }
        }
        return true;

    case 0x6a6c7e0d:
        if (s_uiNumAttached < MAX_ATTACHED)
        {
            s_aAttachClassData[s_uiNumAttached].bIsOdf = 0;
            s_aAttachClassData[s_uiNumAttached].pEffectClass =
                FLEffect::s_EffectClasses.Find(PblHash(pszValue));
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00446f70
bool AttachedEffects::Update(float fDeltaTime)
{
    const EntityGeometry* pGeometry = m_pEntityGeometry;
    if (!pGeometry)
    {
        if (!(const GameObject*)m_pGameObject)
        {
            return false;
        }
        pGeometry = reinterpret_cast<const EntityGeometry*>((const GameObject*)m_pGameObject);
    }

    for (unsigned int i = 0; i < m_pClass->m_uiNumAttached; ++i)
    {
        if (!m_pClass->m_aAttachClassData[i].bIsOdf && !m_aAttachData[i].pEffect)
        {
            m_aAttachData[i].fTimer += fDeltaTime;
            if (m_pClass->m_aAttachClassData[i].uiNumParams >= 1)
            {
                if (m_aAttachData[i].fTimer - fDeltaTime > 0.0f)
                {
                    const float fMin = m_pClass->m_aAttachClassData[i].afParam[0] > 0.0f
                                           ? m_pClass->m_aAttachClassData[i].afParam[0]
                                           : 0.0f;
                    const float fMax = (m_pClass->m_aAttachClassData[i].uiNumParams >= 2 &&
                                        m_pClass->m_aAttachClassData[i].afParam[1] > fMin)
                                           ? m_pClass->m_aAttachClassData[i].afParam[1]
                                           : fMin;
                    m_aAttachData[i].fTimer = -fMin;
                    if (fMax > fMin)
                    {
                        m_aAttachData[i].fTimer -= PblRandom::GetGlobalFloat() * (fMax - fMin);
                    }
                }
                else if (m_aAttachData[i].fTimer > 0.0f)
                {
                    m_aAttachData[i].pEffect = m_pClass->m_aAttachClassData[i].pEffectClass->BuildEffect();
                    if (m_aAttachData[i].pEffect)
                    {
                        m_aAttachData[i].pEffect->AttachEffectToObject(
                            pGeometry, m_pClass->m_aAttachClassData[i].uiBoneName, false);
                    }
                }
            }
        }
        else if (m_pClass->m_bDynamic && m_aAttachData[i].pEffect)
        {
            m_aAttachData[i].pEffect->AttachEffectToObject(
                pGeometry, m_pClass->m_aAttachClassData[i].uiBoneName, false);
        }
    }
    return true;
}
