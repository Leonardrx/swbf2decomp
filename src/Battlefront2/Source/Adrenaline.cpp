#include "PebbleFL/Common/PblMath.h"
#include "Adrenaline.h"

#include <math.h>

#include "Character.h"
#include "Controllable.h"
#include "GameObject.h"
#include "Team.h"

// FUNCTION: SWBF2 0x00438020
Adrenaline::Adrenaline()
{
    mLevel = 0.0f;
}

// FUNCTION: SWBF2 0x00438060
void Adrenaline::Update(float fDeltaTime, const GameObject* pObject)
{
    pObject->GetVelocity();
    float fSpeedFactor;
    if (pObject->GetMaxSpeed() != 0.0f)
    {
        const PblVector3& vVel = pObject->GetVelocity();
        float fRawFactor = 1.0f - PblMath::Sqrt(vVel.x * vVel.x + vVel.y * vVel.y + vVel.z * vVel.z) /
                                  pObject->GetMaxSpeed();
        fSpeedFactor = fRawFactor < 0.7f ? 0.7f : (fRawFactor > 1.0f ? 1.0f : fRawFactor);
    }
    else
    {
        fSpeedFactor = 1.0f;
    }

    float fHealthRatio = pObject->mCurHealth / pObject->mMaxHealth;
    float fHealthFactor = fHealthRatio < 0.3f ? 0.3f : (fHealthRatio > 1.0f ? 1.0f : fHealthRatio);

    AdjustLevel(fDeltaTime * -0.04f * fSpeedFactor * fHealthFactor);

    const PblVector3& vPos = pObject->GetMatrix().GetTranslation();
    const Team* pTeam = Team::GetPtr(pObject->GetTeam());
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pCharacter = Character::GetPtr(i);
        if (!pCharacter || !pCharacter->GetControllable())
        {
            continue;
        }
        if (pTeam->GetAffiliation(pCharacter->mTeamNumber) >= 0)
        {
            continue;
        }
        PblVector3 vOther = pCharacter->GetControllable()->GetMatrix().GetTranslation();
        if (SquaredLength(vOther - vPos) < 625.0f)
        {
            pCharacter->mAdrenaline.NearbyEnemy(fDeltaTime);
        }
    }
}

// FUNCTION: SWBF2 0x004382a0
void Adrenaline::AdjustLevel(float fAmount)
{
    float fLevel = mLevel + fAmount;
    mLevel = fLevel < 0.0f ? 0.0f : (fLevel > 1.0f ? 1.0f : fLevel);
}

// FUNCTION: SWBF2 0x004384b0
void Adrenaline::NearbyEnemy(float fDeltaTime)
{
    AdjustLevel(fDeltaTime * 0.005f);
}

// FUNCTION: SWBF2 0x00438030
AdrenalineLevel Adrenaline::GetCurrentLevel()
{
    if (mLevel < 0.4f)
    {
        return ADRENALINE_LOW;
    }
    if (mLevel < 0.8f)
    {
        return ADRENALINE_MEDIUM;
    }
    return ADRENALINE_HIGH;
}

// FUNCTION: SWBF2 0x004382d0
void Adrenaline::AddKill()
{
    AdjustLevel(0.05f);
}

// FUNCTION: SWBF2 0x004382e0
void Adrenaline::AddHit(float fAmount)
{
    float fClamped;
    if (fAmount < 0.0f)
    {
        fClamped = 0.0f;
    }
    else if (fAmount > 1.0f)
    {
        fClamped = 1.0f;
    }
    else
    {
        fClamped = fAmount;
    }
    AdjustLevel(fClamped * 0.1f);
}

// FUNCTION: SWBF2 0x00438310
void Adrenaline::GetHit(float fAmount)
{
    float fClamped;
    if (fAmount < 0.0f)
    {
        fClamped = 0.0f;
    }
    else if (fAmount > 1.0f)
    {
        fClamped = 1.0f;
    }
    else
    {
        fClamped = fAmount;
    }
    AdjustLevel(fClamped);
}

// FUNCTION: SWBF2 0x00438330
void Adrenaline::AddDied(const GameObject* pObject)
{
    AdjustLevel(0.2f);
    Team* pTeam = Team::GetPtr(pObject->GetTeam());
    const PblVector3& vPos = pObject->GetPosition();
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pCharacter = Character::GetPtr(i);
        if (pCharacter && pCharacter->GetControllable())
        {
            PblVector3 vOther = pCharacter->GetControllable()->GetMatrix().GetTranslation();
            float fDx = vOther.x - vPos.x;
            float fDy = vOther.y - vPos.y;
            float fDz = vOther.z - vPos.z;
            float fDistSq = fDx * fDx;
            fDistSq += fDy * fDy;
            fDistSq += fDz * fDz;
            if (fDistSq < 225.0f)
            {
                pCharacter->mAdrenaline.NearbyDeath(pTeam->GetAffiliation(pCharacter->mTeamNumber) > 0);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00438480
void Adrenaline::NearbyDeath(bool bAlly)
{
    if (bAlly)
    {
        AdjustLevel(0.08f);
    }
    else
    {
        AdjustLevel(0.03f);
    }
}
