#include "UnitAttackPatterns.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityFlyer.h"
#include "GameObject.h"
#include "PathManager.h"
#include "UnitAgent.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblMath.h"

class GameObject;

namespace
{


    const PblVector3& GetFront(const PblMatrix& m)
    {
        return m.GetZAxis();
    }

    const PblVector3& GetTargetVelocity(const GameObject* pTarget)
    {
        typedef const Entity*(__thiscall * EntityFn)(const GameObject*);
        typedef const PblVector3&(__thiscall * VelocityFn)(const Entity*);
        const Entity* pEntity = pTarget->GetEntity();
        return static_cast<const EntityGeometry*>(pEntity)->GetVelocity();
    }
}

// FUNCTION: SWBF2 0x00667a30
void AttackPatterns::DoFlyDirect()
{
    const EntityFlyerClass* pClass =
        static_cast<const EntityFlyerClass*>(GetMe()->GetEntity()->GetEntityClass());

    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();
    PblVector3 vToTarget;
    vToTarget.x = vTarget.x - vPos.x;
    vToTarget.y = vTarget.y - vPos.y;
    vToTarget.z = vTarget.z - vPos.z;
    PblVector3 vFront = GetMe()->GetMatrix().GetZAxis();

    if (vFront.y * vToTarget.y + vFront.x * vToTarget.x + vFront.z * vToTarget.z > 0.0f ||
        AIUtil::GetRandomIntPerSecond(1, 3) == 1)
    {
        mOwner->GetLowLevel().SetFireRequest(true, false);
        mOverrideFire = true;
    }

    if (!mDoLoop && AIUtil::GetRandomIntPerSecond(1, 13) == 1)
    {
        mDoLoop = true;
    }

    if (mDoLoop)
    {
        if (GetMe()->GetMatrix().m[1][1] < 0.0f)
        {
            mUpsideDown = true;
        }
        else if (mUpsideDown && GetMe()->GetMatrix().m[1][1] > 0.0f)
        {
            mDoLoop = false;
            mUpsideDown = false;
        }
    }
    else
    {
        PblVector3 vDest;
        float fDistSq = vToTarget.y * vToTarget.y + vToTarget.x * vToTarget.x + vToTarget.z * vToTarget.z;
        if (fDistSq > (pClass->mMaxFlyerSpeed * 2.0f) * (pClass->mMaxFlyerSpeed * 2.0f))
        {
            const PblVector3& vVelocity =
                static_cast<const EntityGeometry*>(GetAttackHelper()->GetTarget()->GetEntity())->GetVelocity();
            vDest = vVelocity + vTarget;
            vDest.y = Min(vDest.y, AIUtil::gMaxFlyHeight);
        }
        else
        {
            vDest = vFront * (pClass->mMaxFlyerSpeed * 5.0f) + vPos;
            vDest.y = Min(vDest.y, (AIUtil::gMinFlyHeight + AIUtil::gMaxFlyHeight) * 0.5f);
        }
        mOwner->GetLowLevel().SetNavigator_GotoDirect(vDest, false, false);
    }

    mOwner->GetLowLevel().SetMinControlSpeed(pClass->mMaxFlyerSpeed * 0.5f);
}

// FUNCTION: SWBF2 0x00667550
void AttackPatterns::GeneratePath()
{
    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();
    PblVector3 vToTarget;
    vToTarget.x = vTarget.x - vPos.x;
    vToTarget.y = vTarget.y - vPos.y;
    vToTarget.z = vTarget.z - vPos.z;
    PblVector3 vDir = AIUtil::GetSafeNormalXZ(vToTarget);

    const EntityFlyerClass* pClass =
        static_cast<const EntityFlyerClass*>(GetMe()->GetEntity()->GetEntityClass());
    // GLOBAL: SWBF2 0x01fac374
    static const float fBack = Max(160.0f, pClass->mMaxFlyerSpeed * 4.0f);
    // GLOBAL: SWBF2 0x01fac36c
    static const float fSide = Max(40.0f, pClass->mMaxFlyerSpeed);

    PblVector3 vBehind = vTarget - vDir * fBack + gVectorUnitY * 90.0f;
    PblVector3 vNear = vTarget - vDir * fSide + gVectorUnitY * 0.0f;
    PblVector3 vPast = vDir * fSide + vTarget + gVectorUnitY * 30.0f;

    vBehind.y = Min(vBehind.y, AIUtil::gMaxFlyHeight);
    vNear.y = Min(vNear.y, AIUtil::gMaxFlyHeight);
    vPast.y = Min(vPast.y, AIUtil::gMaxFlyHeight);

    AIPath* pPath = new AIPath();
    if (pPath)
    {
        pPath->Prepend(vPast, 0);
        pPath->Prepend(vNear, 0);
        pPath->Prepend(vBehind, 0);
        pPath->Prepend(vPos, 0);
        mOwner->GetLowLevel().SetNavigator_FollowPath(pPath, false, false, false);
        pPath->Release();
    }
}

// FUNCTION: SWBF2 0x006678b0
void AttackPatterns::DoFlyStrafe()
{
    const GameObject* pTarget = GetAttackHelper()->GetTarget();
    PblVector3 vPos = GetMe()->GetMatrix().GetTranslation();
    PblVector3 vTarget = pTarget->GetMatrix().GetTranslation();
    PblVector3 vToTarget;
    vToTarget.x = vTarget.x - vPos.x;
    vToTarget.y = vTarget.y - vPos.y;
    vToTarget.z = vTarget.z - vPos.z;
    PblVector3 vFront = GetMe()->GetMatrix().GetZAxis();

    if (vFront.y * vToTarget.y + vFront.x * vToTarget.x + vFront.z * vToTarget.z > 0.5f)
    {
        mOwner->GetLowLevel().SetFireRequest(true, false);
        mOverrideFire = true;
    }

    if (GetCtrl()->GetLowLevel().IsAtDest())
    {
        GeneratePath();
        return;
    }

    const EntityFlyerClass* pClass =
        static_cast<const EntityFlyerClass*>(GetMe()->GetEntity()->GetEntityClass());
    mOwner->GetLowLevel().SetMinControlSpeed(pClass->mMaxFlyerSpeed * 0.5f);
}

// FUNCTION: SWBF2 0x006679f0
void AttackPatterns::CleanFlyStrafe()
{
    GetAttackHelper()->ResetWaitDelay();
}

// FUNCTION: SWBF2 0x00667530
void AttackPatterns::InitFlyStrafe()
{
    GeneratePath();
    GetAttackHelper()->SetWaitDelay(0.5f);
}

// FUNCTION: SWBF2 0x00667a00
void AttackPatterns::InitFlyDirect()
{
    GetAttackHelper()->SetWaitDelay(0.5f);
    mDoLoop = false;
    mUpsideDown = false;
}

// FUNCTION: SWBF2 0x00667ce0
void AttackPatterns::CleanFlyDirect()
{
    GetAttackHelper()->ResetWaitDelay();
}
