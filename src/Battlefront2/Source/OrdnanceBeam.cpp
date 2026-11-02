#include "OrdnanceBeam.h"

#include <float.h>

#include "Aimer.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "FLEffect.h"
#include "GameObject.h"
#include "NetGame.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"


extern bool netIsLocalTurn;
extern bool netIsLateTurn;

class Weapon
{
public:

    char mWeaponState[0x70];
    Aimer* mAimer;
};

// SYNTHETIC: SWBF2 0x00405a30
// `dynamic initializer for 'OrdnanceBeam::rttiHashOrdnanceBeam''
// OrdnanceBeam::rttiHashOrdnanceBeam

// GLOBAL: SWBF2 0x01ef2614
// OrdnanceBeam::rttiHashOrdnanceBeam
PblHash OrdnanceBeam::rttiHashOrdnanceBeam("OrdnanceBeam");
// GLOBAL: SWBF2 0x0078a7b8
// OrdnanceBeam::rttiNameOrdnanceBeam
const char OrdnanceBeam::rttiNameOrdnanceBeam[] = "OrdnanceBeam";

// FUNCTION: SWBF2 0x005f9670
unsigned int OrdnanceBeamClass::GetDerivedRtti() const { return rttiHashOrdnanceBeamClass; }

// FUNCTION: SWBF2 0x005f9680
const char* OrdnanceBeamClass::GetDerivedRttiName() const { return rttiNameOrdnanceBeamClass; }

// SYNTHETIC: SWBF2 0x00405a40
// `dynamic initializer for 'OrdnanceBeamClass::rttiHashOrdnanceBeamClass''
// OrdnanceBeamClass::rttiHashOrdnanceBeamClass

// GLOBAL: SWBF2 0x01ef2618
// OrdnanceBeamClass::rttiHashOrdnanceBeamClass
PblHash OrdnanceBeamClass::rttiHashOrdnanceBeamClass("OrdnanceBeamClass");
// GLOBAL: SWBF2 0x0078a79c
// OrdnanceBeamClass::rttiNameOrdnanceBeamClass
const char OrdnanceBeamClass::rttiNameOrdnanceBeamClass[] = "OrdnanceBeamClass";

// FUNCTION: SWBF2 0x005fa180
OrdnanceBeamClass::OrdnanceBeamClass(unsigned int uHash)
    : OrdnanceClass(uHash)
{
    mLifeSpan = 0.0f;

    mRange = 150.0f;
    mBeamWidth = 0.0f;
    mBeamTextureId = 0;
    mBeamColor = RedColor::WHITE;
    mGlowWidth = 0.0f;
    mGlowTextureId = 0x716BD81Au;
    mGlowColor = RedColor::WHITE;
    mPassThrough = false;
    mImpactEffect = 0;
}

// FUNCTION: SWBF2 0x005fa230
OrdnanceBeamClass::OrdnanceBeamClass(OrdnanceBeamClass* pBase, unsigned int uHash)
    : OrdnanceClass(pBase, uHash)
{
    mRange = pBase->mRange;
    mBeamWidth = pBase->mBeamWidth;
    mBeamTextureId = pBase->mBeamTextureId;
    mBeamColor = pBase->mBeamColor;
    mGlowWidth = pBase->mGlowWidth;
    mGlowTextureId = pBase->mGlowTextureId;
    mGlowColor = pBase->mGlowColor;
    mImpactEffect = pBase->mImpactEffect;
    mPassThrough = pBase->mPassThrough;
}

// FUNCTION: SWBF2 0x005fa700
OrdnanceClass* OrdnanceBeamClass::Derive(unsigned int uHash)
{
    return new OrdnanceBeamClass(this, uHash);
}

// GLOBAL: SWBF2 0x007ec258

static BeamLodData beamloddata;



// FUNCTION: SWBF2 0x005f96d0
OrdnanceBeam::OrdnanceBeam(OrdnanceBeamClass* pClass, const OrdnanceDesc& desc)
    : Ordnance(pClass, desc),
      RedSceneObject(&beamloddata),
      mDirection(desc.mDirection)
{
    mLength = 0.0f;
    mAttachAimer = 0;
    m_pRicochet = 0;
    mDeadFlag = false;

    if (desc.mFireWeapon)
    {
        Aimer* pFirst = desc.mFireWeapon->mAimer;
        Aimer* pAimer = pFirst;
        float fBestDistSq = FLT_MAX;
        do
        {
            const float fDistSq = SquaredLength(mPosition - pAimer->mFirePos);
            if (fBestDistSq > fDistSq)
            {
                fBestDistSq = fDistSq;
                mAttachAimer = pAimer;
            }
            pAimer = pAimer->mNextAimer;
        } while (pAimer != pFirst);
    }

    SetRenderPos(mPosition);
    SetRenderRadius(0.0f);
    AddToScene();
}

// FUNCTION: SWBF2 0x005f9810
OrdnanceBeam::~OrdnanceBeam()
{
    if (m_pRicochet)
    {
        m_pRicochet->SetDead();
    }
}

// SYNTHETIC: SWBF2 0x005fa7ff
// ??_EOrdnanceBeam@@WBI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005fa807
// ??_EOrdnanceBeam@@WJI@AEPAXI@Z
// SYNTHETIC: SWBF2 0x005f96a0
// OrdnanceBeam::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f9890
bool OrdnanceBeam::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }
    if (mDeadFlag)
    {
        return false;
    }

    if (GetOwner() && mAttachAimer)
    {
        mPosition = mAttachAimer->mFirePos;
        mDirection = mAttachAimer->mDirection;
        mLength = 0.0f;
        if (!mAttachAimer->WeaponShouldFire())
        {
            return Ordnance::Update(fDeltaTime);
        }
    }

    const GameObject* pIgnore[128];
    pIgnore[0] = GetOwner();
    int iIgnoreCount = 1;
    mLength = 0.0f;
    PblVector3 vPos = mPosition;
    float fRemaining = ((OrdnanceBeamClass*)mClass)->mRange;
    bool bStopped = !((OrdnanceBeamClass*)mClass)->mPassThrough;
    CollisionObject* pHit;
    do
    {
        pHit = 0;
        const float fDist =
            CollisionManager::RayHit(vPos, mDirection, fRemaining, pHit, 0, pIgnore, iIgnoreCount, 0, true) *
            fRemaining;
        mLength += fDist;
        fRemaining -= fDist;
        vPos += mDirection * fDist;

        SetRenderPos(mPosition + mDirection * (mLength * 0.5f));
        SetRenderRadius(mLength + 1.0f);

        if (pHit)
        {
            GameObject* pObject = pHit->GetGameObject();
            if (pObject)
            {
                if (iIgnoreCount < 128)
                {
                    pIgnore[iIgnoreCount++] = pObject;
                }
                else
                {
                    pHit = 0;
                }

                if (pObject->Deflect(this, vPos, mDirection))
                {
                    goto update;
                }

                DamageDesc damage = GetDamage();
                if (mClass->mLifeSpan > 0.0f)
                {
                    damage.mBaseValue = fDeltaTime / mClass->mLifeSpan * damage.mBaseValue;
                }
                pObject->ApplyDamage(damage, mDirection, vPos, CollisionManager::GetLastRayHitBodyId(),
                                     0xffffffffu);

                const CollisionObjectType eType = pObject->GetCollisionType();
                if (eType == COLL_TERRAIN || eType == COLL_STATIC || eType == COLL_ASTEROID)
                {
                    bStopped = true;
                }
            }
            else
            {
                pHit = 0;
            }

            if (!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn))
            {
                if (((OrdnanceBeamClass*)mClass)->mImpactEffect)
                {
                    FLEffectObject* pEffect = ((OrdnanceBeamClass*)mClass)->mImpactEffect->BuildEffect();
                    if (pEffect)
                    {
                        PblVector3 vEffectPos = vPos;
                        pEffect->AttachEffectToVector(&vEffectPos, false);
                    }
                }
            }
        }
    } while (((OrdnanceBeamClass*)mClass)->mPassThrough && !bStopped && pHit && fRemaining > 0.0f);

    if (m_pRicochet)
    {
        m_pRicochet->SetDead();
        m_pRicochet = 0;
    }

update:
    if (Ordnance::Update(fDeltaTime))
    {
        return true;
    }
    SetDead();
    return false;
}

// FUNCTION: SWBF2 0x005f9c90
void OrdnanceBeam::SetDead()
{
    mDeadFlag = true;
    if (m_pRicochet)
    {
        m_pRicochet->SetDead();
        m_pRicochet = 0;
    }
}







// FUNCTION: SWBF2 0x005f9cc0
void OrdnanceBeam::CreateRicochet(const PblVector3& vPos, const PblVector3& vNormal,
                                 GameObject* pObject) const
{
    OrdnanceBeam* pThis = const_cast<OrdnanceBeam*>(this);
    if (m_pRicochet)
    {
        GameObject* pOwner = m_pRicochet->mOwner;
        if (pOwner == pObject)
        {
            m_pRicochet->mPosition = vPos;
            m_pRicochet->mDirection = vNormal;
            return;
        }
        delete m_pRicochet;
    }

    OrdnanceDesc desc;
    desc.mPosition = vPos;
    desc.mDirection = vNormal;
    desc.mStrength = 1.0f;
    desc.mOwner = pObject;
    desc.mDamageOwner.mGameObject = pObject;
    if (pObject)
    {
        Controllable* pControl = pObject->GetControllable();
        desc.mDamageOwner.mCharacter = pControl->mCharacter;
        desc.mDamageOwner.mTeam = pObject->GetTeam();
        desc.mDamageOwner.mClass = pObject->GetEntityClass();
        desc.mDamageOwner.mGameObject = pObject;
        desc.mDamageOwner.mDamageMultiplier = pControl->GetOffenseMultiplier();
    }
    else
    {
        desc.mDamageOwner.mCharacter = 0;
        desc.mDamageOwner.mTeam = 0;
        desc.mDamageOwner.mClass = 0;
        desc.mDamageOwner.mGameObject = 0;
        desc.mDamageOwner.mDamageMultiplier = 1.0f;
    }
    desc.mDamageOwner.mWeapon = 0;
    desc.mTarget = 0;
    desc.mFireWeapon = 0;
    desc.mBodyID = -1;

    bool bBuild = true;
    if (NetGame::IsEnabled())
    {
        if (NetGame::IsClient())
        {
            bBuild = netIsLocalTurn;
        }
        else
        {
            NetGame::CreateEvent_CreateOrdnance(mClass, desc);
        }
    }
    if (bBuild)
    {
        pThis->m_pRicochet = (OrdnanceBeam*)mClass->Build(desc);
    }
}

// FUNCTION: SWBF2 0x005f9ef0
void OrdnanceBeam::Render(unsigned char, float, unsigned int)
{
    const PblVector3 vStart = mPosition;
    const PblVector3 vEnd = mPosition + mLength * mDirection;

    const float fBeamWidth = ((OrdnanceBeamClass*)mClass)->mBeamWidth;
    const float fGlowWidth = ((OrdnanceBeamClass*)mClass)->mGlowWidth;
    const float fExtent = (fBeamWidth > fGlowWidth ? fBeamWidth : fGlowWidth) * 2.0f;

    PblVector3 vMin;
    vMin.z = (vEnd.z > vStart.z ? vStart.z : vEnd.z);
    float fMinY;
    if (vEnd.y > vStart.y)
    {
        fMinY = vStart.y;
    }
    else
    {
        fMinY = vEnd.y;
    }
    vMin.y = fMinY;
    vMin.x = (vEnd.x > vStart.x ? vStart.x : vEnd.x);
    vMin.x -= fExtent;
    vMin.y -= fExtent;
    vMin.z -= fExtent;
    PblVector3 vMax;
    vMax.z = (vStart.z > vEnd.z ? vStart.z : vEnd.z);
    vMax.y = (vStart.y > vEnd.y ? vStart.y : vEnd.y);
    vMax.x = (vStart.x > vEnd.x ? vStart.x : vEnd.x);
    vMax.x += fExtent;
    vMax.y += fExtent;
    vMax.z += fExtent;
    RedParticleRenderer::SubmitBounds(vMin, vMax);

    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;

    RedColor beamColor = ((OrdnanceBeamClass*)mClass)->mBeamColor;
    RedParticleRenderer::SubmitTexture(((OrdnanceBeamClass*)mClass)->mBeamTextureId, BLEND_NORMAL, 0);
    RedParticleRenderer::SubmitParticle(PIT_LASER_START, vStart, beamColor, fBeamWidth, 1.0f, texCoords,
                                        gVectorUnitX, gVectorUnitY);
    RedParticleRenderer::SubmitParticle(PIT_LASER_END, vEnd, beamColor, fBeamWidth, 1.0f, texCoords,
                                        gVectorUnitX, gVectorUnitY);

    RedColor glowColor = ((OrdnanceBeamClass*)mClass)->mGlowColor;
    RedParticleRenderer::SubmitTexture(((OrdnanceBeamClass*)mClass)->mGlowTextureId, BLEND_NORMAL, 0);
    RedParticleRenderer::SubmitParticle(PIT_LASER_START, vStart, glowColor, fGlowWidth, 1.0f, texCoords,
                                        gVectorUnitX, gVectorUnitY);
    RedParticleRenderer::SubmitParticle(PIT_LASER_END, vEnd, glowColor, fGlowWidth, 1.0f, texCoords,
                                        gVectorUnitX, gVectorUnitY);
}

// FUNCTION: SWBF2 0x005fa770
Ordnance* OrdnanceBeamClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceBeam* pBeam = new OrdnanceBeam(this, desc);
    pBeam->ActivateThread(-40, PER_LOCAL_TURN);
    return pBeam;
}

// FUNCTION: SWBF2 0x005f9690
float OrdnanceBeamClass::GetRange()
{
    return mRange;
}

// FUNCTION: SWBF2 0x005fa2e0
void OrdnanceBeamClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x30580110:
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        RedColor color;
        color.b = (unsigned char)b;
        color.g = (unsigned char)g;
        color.r = (unsigned char)r;
        color.a = (unsigned char)a;
        mGlowColor = color;
        break;
    }
    case 0x11727A92:
        if (*pszValue)
        {
            mImpactEffect = FLEffect::Get(PblHash(pszValue));
        }
        break;
    case 0x398B9AD9:
        sscanf(pszValue, "%f", &mGlowWidth);

    case 0xC6BFC336:
        mGlowTextureId = PblHash(pszValue);
        break;
    case 0x604E2441:
        mPassThrough = atoi(pszValue) != 0;
        break;
    case 0x3F30370C:
        switch (PblHash(pszValue))
        {
        case 0x4A4A69FF:
            mBeamWidth = 0.2f;
            mBeamTextureId = 0xF0E687E7;
            mBeamColor = RedColor::WHITE;
            mGlowWidth = 0.5f;
            mGlowTextureId = 0x716BD81A;
            mGlowColor = RedColor(0x0a, 0x0a, 0x7f, 0x64);
            break;
        case 0x09D3224C:
            mBeamWidth = 0.35f;
            mBeamTextureId = 0xE855B12C;
            mBeamColor = RedColor::WHITE;
            mGlowWidth = 0.875f;
            mGlowTextureId = 0x716BD81A;
            mGlowColor = RedColor(0x64, 0xff, 0x64, 0x50);
            break;
        case 0x5BFD7D80:
            mBeamWidth = 0.1f;
            mBeamTextureId = 0x4A040ACC;
            mBeamColor = RedColor::WHITE;
            mGlowWidth = 0.25f;
            mGlowTextureId = 0x716BD81A;
            mGlowColor = RedColor(0xff, 0x0a, 0x0a, 0x64);
            break;
        case 0x8AB31EE0:
            mBeamWidth = 0.2f;
            mBeamTextureId = 0xE855B12C;
            mBeamColor = RedColor::WHITE;
            mGlowWidth = 0.5f;
            mGlowTextureId = 0x716BD81A;
            mGlowColor = RedColor(0x64, 0xff, 0x64, 0x50);
            break;
        case 0xD6578027:
            mBeamWidth = 0.35f;
            mBeamTextureId = 0xF0E687E7;
            mBeamColor = RedColor::WHITE;
            mGlowWidth = 0.875f;
            mGlowTextureId = 0x716BD81A;
            mGlowColor = RedColor(0x0a, 0x0a, 0x7f, 0x64);
            break;
        case 0xDD9D59EC:
            mBeamWidth = 0.35f;
            mBeamTextureId = 0x4A040ACC;
            mBeamColor = RedColor::WHITE;
            mGlowWidth = 0.875f;
            mGlowTextureId = 0x716BD81A;
            mGlowColor = RedColor(0xff, 0x0a, 0x0a, 0x64);
            break;
        }
        break;
    case 0x6F179AB8:
        sscanf(pszValue, "%f", &mBeamWidth);
        mGlowWidth = mBeamWidth * 2.5f;
        break;
    case 0xB709B5E7:
        mBeamTextureId = PblHash(pszValue);
        break;
    case 0xA3093ACD:
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        RedColor color;
        color.b = (unsigned char)b;
        color.g = (unsigned char)g;
        color.r = (unsigned char)r;
        color.a = (unsigned char)a;
        mBeamColor = color;
        break;
    }
    case 0xFADC0CD2:
        sscanf(pszValue, "%f", &mRange);
        break;
    default:
        OrdnanceClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x005fa2d0
OrdnanceBeamClass::~OrdnanceBeamClass()
{
}
