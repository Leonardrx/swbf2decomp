#include <stdlib.h>

#include "AIDifficulty.h"
#include "PebbleFL/Common/PblMath.h"

#include "AIUtil.h"
#include "Controllable.h"
#include "EventInfo.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "UnitController.h"
#include "GameLoop.h"
#include "ControllerManager.h"
#include "CollisionObject.h"
#include "CameraManager.h"
#include "CombatUtil.h"
#include "Damageable.h"
#include "EntityCarrier.h"
#include "FlagItem.h"
#include "Ordnance.h"
#include "SpaceAssault.h"
#include "Team.h"
#include "WeaponCannonInstance.h"

// GLOBAL: SWBF2 0x007887a0
// sDifficultyPlayer
// GLOBAL: SWBF2 0x007887ac
// sDifficultyEnemy

namespace AIDifficulty
{
static const int sDifficultyPlayer[3] = { 6, 11, 11 };

static const int sDifficultyEnemy[3] = { 4, 14, 17 };

// GLOBAL: SWBF2 0x01e2fda4
int mProfileDifficulty;

// GLOBAL: SWBF2 0x01e2fd90
int mScriptPlayerModifier[3];

// GLOBAL: SWBF2 0x01e2fd7c
int mScriptEnemyModifier[3];

// GLOBAL: SWBF2 0x01e2fda8
int mAutoBalancePlayerModifier;

// GLOBAL: SWBF2 0x01e2fda0
int mAutoBalanceEnemyModifier;

// GLOBAL: SWBF2 0x007e5886
bool mAutoBalanceEnabled = true;

// GLOBAL: SWBF2 0x007e5887
bool gAutoBalanceEnabled = true;

// GLOBAL: SWBF2 0x01e2fd88
float mNextFlyerKillTime;

// GLOBAL: SWBF2 0x01e2fd9c
float mNextCritSysDamageTime;

// GLOBAL: SWBF2 0x01e2fd8c
float mCritSysBias;

static float GetFloatSkill_RangedLinear(const UnitController* pController, float fMin,
                                        float fMax, bool bClamp);

// FUNCTION: SWBF2 0x004395e0
static void SpaceAssault_Init()
{
    mNextFlyerKillTime = SpaceAssault_GetRandomFlyerKillTime();
    mNextCritSysDamageTime = SpaceAssault_GetRandomCritSysDamageTime();
    mCritSysBias = AIUtil::GetRandomFloat(0.0f, 1.0f);
}
}

// FUNCTION: SWBF2 0x00438bd0
void AIDifficulty::Init()
{
    if (NetGame::IsEnabled())
    {
        mProfileDifficulty = PlayerPrefs::mAllUserPrefs[0].mDifficulty;
    }
    else
    {
        mProfileDifficulty = PlayerProfile::GetProfile(0)->mGameOptions.mCurDiffLevel;
    }

    for (int i = 0; i < 3; ++i)
    {
        mScriptPlayerModifier[i] = 0;
        mScriptEnemyModifier[i] = 0;
    }
    EnableAutoBalance(true);
    SpaceAssault_Init();
}

// FUNCTION: SWBF2 0x00438c70
int AIDifficulty::GetProfileDifficulty()
{
    return mProfileDifficulty;
}

// FUNCTION: SWBF2 0x00438c80
int AIDifficulty::GetIntSkill(const UnitController* pController, bool bClamp)
{
    bool bPlayer = false;
    if (pController == (const UnitController*)1)
    {
        bPlayer = true;
    }
    else if (pController)
    {
        if (pController->GetMe() && pController->GetMe()->GetGameObject())
        {
            bPlayer = AIUtil::IsPlayerTeam(pController->GetMe()->GetGameObject()->mTeam);
        }
    }

    int iDifficulty = GetProfileDifficulty();
    int iSkill;
    if (bPlayer)
    {
        iSkill = sDifficultyPlayer[iDifficulty - 1] + mScriptPlayerModifier[iDifficulty - 1] +
                 mAutoBalancePlayerModifier;
    }
    else
    {
        iSkill = sDifficultyEnemy[iDifficulty - 1] + mScriptEnemyModifier[iDifficulty - 1] +
                 mAutoBalanceEnemyModifier;
    }

    int iModifier = 0;
    if (pController > (const UnitController*)1)
    {
        AI::EventInfo info(AI::EVQ_GetSkillModifier);
        if (pController->SendEvent(&info))
        {
            iModifier = info.mInt;
        }
    }

    iSkill += iModifier;
    iSkill = (iSkill > 1) ? iSkill : 1;
    if (bClamp)
    {
        return (iSkill < 20) ? iSkill : 20;
    }
    return iSkill;
}

namespace AIDifficulty
{
// FUNCTION: SWBF2 0x00438d60
static float GetFloatSkill(const UnitController* pController, bool bClamp)
{
    return (float)(GetIntSkill(pController, bClamp) - 1) / 19.0f;
}

// FUNCTION: SWBF2 0x00438d80
static float GetFloatSkill_RangedLinear(const UnitController* pController, float fMin,
                                        float fMax, bool bClamp)
{
    return GetFloatSkill(pController, bClamp) * (fMax - fMin) + fMin;
}
}

// FUNCTION: SWBF2 0x00438df0
float AIDifficulty::GetCoverPercentage()
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x00438e00
float AIDifficulty::GetBarrageCountMultiplier(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.0f, 1.0f, false);
}

// FUNCTION: SWBF2 0x00438e20
float AIDifficulty::GetBarrageWaitMultiplier(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.1f, 0.9f, false);
}

// FUNCTION: SWBF2 0x00438e40
float AIDifficulty::GetTriggerSingleWaitDelayMultiplier(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.1f, 0.9f, false);
}

// FUNCTION: SWBF2 0x00438e60
float AIDifficulty::GetChargeWeaponBot(const UnitController* pController)
{
    return 0.15f;
}

// FUNCTION: SWBF2 0x00438e70
float AIDifficulty::GetChargeWeaponTop(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.0f, 1.0f, false);
}

// FUNCTION: SWBF2 0x00438e90
float AIDifficulty::GetCrowdGrenadeThrowPercentage(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.0f, 1.0f, true);
}

// FUNCTION: SWBF2 0x00438eb0
float AIDifficulty::GetPlayerThreatAngle()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.7f, true);
}

// FUNCTION: SWBF2 0x00438ed0
float AIDifficulty::GetTargetBubbleShrinkRate(bool bPlayer)
{
    return GetFloatSkill_RangedLinear((const UnitController*)bPlayer, -0.1f, -1.1f, false);
}

// FUNCTION: SWBF2 0x00438ef0
float AIDifficulty::GetMinBubbleSize(bool bPlayer)
{
    float fSize = GetFloatSkill_RangedLinear((const UnitController*)bPlayer, 1.6f, 0.4f, false);
    if (fSize < 0.0f)
    {
        fSize = 0.0f;
    }
    return fSize;
}

// FUNCTION: SWBF2 0x00438f20
float AIDifficulty::GetMaxBubbleSize(bool bPlayer)
{
    float fSize = GetFloatSkill_RangedLinear((const UnitController*)bPlayer, 2.2f, 1.2f, false);
    if (fSize < 0.0f)
    {
        fSize = 0.0f;
    }
    return fSize;
}

// FUNCTION: SWBF2 0x00438f50
float AIDifficulty::GetMaxBubbleSize_Flyer(bool bPlayer)
{
    float fSize = GetFloatSkill_RangedLinear((const UnitController*)bPlayer, 4.0f, 1.2f, false);
    if (fSize < 0.0f)
    {
        fSize = 0.0f;
    }
    return fSize;
}

// FUNCTION: SWBF2 0x00439220
float AIDifficulty::SpaceAssault_GetRandomFlyerKillTime()
{
    float fMin = GetFloatSkill_RangedLinear(0, 45.0f, 15.0f, true);
    float fMax = GetFloatSkill_RangedLinear(0, 100.0f, 60.0f, true);
    return AIUtil::GetRandomFloat(fMin, fMin > fMax ? fMin : fMax);
}

// FUNCTION: SWBF2 0x00439280
float AIDifficulty::SpaceAssault_GetRandomCritSysDamageTime()
{
    float fMin = GetFloatSkill_RangedLinear(0, 25.0f, 10.0f, true);
    float fMax = GetFloatSkill_RangedLinear(0, 45.0f, 12.0f, true);
    return AIUtil::GetRandomFloat(fMin, fMin > fMax ? fMin : fMax);
}

// FUNCTION: SWBF2 0x00439310
void AIDifficulty::EnableAutoBalance(bool bEnable)
{
    mAutoBalanceEnabled = bEnable;
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00438db0
static float GetFloatSkill_RangedCubed(const UnitController* pController, float fMin, float fMax, bool bClamp)
{
    float fSkill = GetFloatSkill(pController, bClamp);
    return (fMax - fMin) * (fSkill * fSkill * fSkill) + fMin;
}
}

// FUNCTION: SWBF2 0x00438f80
float AIDifficulty::GetAimerMaxAngAcc(const UnitController* pController)
{
    return AIDifficulty::GetFloatSkill_RangedCubed(pController, 0.8f, 14.0f, false);
}

// FUNCTION: SWBF2 0x00438fa0
float AIDifficulty::GetAimerDamping(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.8f, 2.4f, false);
}

// FUNCTION: SWBF2 0x00438fc0
float AIDifficulty::GetAimerSpread(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 3.0f, 1.0f, true);
}

// FUNCTION: SWBF2 0x00438fe0
float AIDifficulty::GetAimerYawSpread(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 2.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439000
float AIDifficulty::GetGrenadeReactionTime()
{
    return GetFloatSkill_RangedLinear(0, 2.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439020
float AIDifficulty::GetFlyerTurnAndAttackFrequency()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439040
float AIDifficulty::GetFlyerEvasiveFrequency()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439060
float AIDifficulty::GetFlyerTrickOnDamage()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 1.5f, true);
}

// FUNCTION: SWBF2 0x00439080
float AIDifficulty::GetFlyerFreeFlyOnDamage()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.2f, true);
}

// FUNCTION: SWBF2 0x004390a0
float AIDifficulty::GetFlyerReactionTime()
{
    return GetFloatSkill_RangedLinear(0, 0.5f, 1.5f, true);
}

// FUNCTION: SWBF2 0x004390c0
float AIDifficulty::GetRangedSpeed(float fSkill, float fMin, float fMid, float fMax, float fBoost)
{
    fSkill = Clamp(fSkill, 0.0f, 1.5f);

    if (fSkill < 0.5f)
    {
        return Interpolate(fMin, fMid, fSkill * 2.0f);
    }
    if (fSkill < 1.0f)
    {
        return Interpolate(fMid, fMax, (fSkill - 0.5f) * 2.0f);
    }
    return Interpolate(fMax, fBoost, (fSkill - 1.0f) * 2.0f);
}

// FUNCTION: SWBF2 0x00439160
void AIDifficulty::GetFlyerSpeedRange(const UnitController* pController, float fMin, float fMid, float fMax,
                                      float fBoost, float& rLow, float& rHigh)
{
    rLow = GetRangedSpeed(GetFloatSkill_RangedLinear(pController, 0.3f, 0.8f, true), fMin, fMid, fMax, fBoost);
    float fHigh = GetRangedSpeed(GetFloatSkill_RangedLinear(pController, 0.5f, 1.2f, true), fMin, fMid, fMax, fBoost);
    rHigh = fHigh;
    if (rLow > fHigh)
    {
        rHigh = rLow;
    }
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x004392e0
float SpaceAssault_GetFlyerKillForPlayerTeam()
{
    return 0.4f;
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x004392f0
float SpaceAssault_GetCritSysKillForPlayerTeam()
{
    return AIDifficulty::GetFloatSkill_RangedLinear(0, 0.4f, 0.25f, true);
}
}

namespace
{

inline const CollisionObject* GetCollisionObject(const GameObject* pObject)
{
    return static_cast<const CollisionObject*>(pObject);
}

}

namespace AI
{

enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};
}

namespace ControllerManager
{
int GetAIInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                 int iMax, const Team* pTeam, AI::AIAffil eAffil);
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00439620
static GameObject* GetRandomFlyer(const Team* pTeam, AI::AIAffil eAffil)
{
    GameObject* apObjects[40];
    const PblVector3 vOrigin(0.0f, 0.0f, 0.0f);
    int iRemaining = ControllerManager::GetAIInRange(vOrigin, 3.402823466e38f,
                                                     apObjects, 40, pTeam, eAffil);
    while (iRemaining != 0)
    {
        GameObject* pObject = apObjects[--iRemaining];
        if (!pObject || !pObject->GetControllable() ||
            !pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            continue;
        }
        if (!CameraManager::Instance()->IsInsideAnyCamera(
                GetCollisionObject(pObject)->GetPosition(), 100.0f, 3000.0f) &&
            AIUtil::CountHumansInVehicle(pObject->GetControllable()) <= 0)
        {
            return pObject;
        }
    }
    return 0;
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x004396e0
static void SpaceAssault_RandomFlyerKill(const Team* pTeam)
{
    GameObject* pVictim = AIDifficulty::GetRandomFlyer(pTeam, AI::AFFIL_ALLY);
    if (!pVictim)
    {
        return;
    }
    GameObject* pKiller = AIDifficulty::GetRandomFlyer(pTeam, AI::AFFIL_ENEMY);
    if (!pKiller)
    {
        return;
    }
    Controllable* pCtrl = pKiller->GetControllable();
    Character* pCharacter = pCtrl->mCharacter;
    const Weapon* pWeapon = pCtrl->GetCurWpn();
    DamageOwner owner(pCharacter, pKiller, pKiller->GetTeam(),
                      pKiller->mEntityClass, pWeapon->mClass, 1.0f);
    pVictim->InstantDeath(owner, true);
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00439770
static void SpaceAssault_RandomCritSysDamage(const Team* pTeam)
{
    GameObject* pFlyer = AIDifficulty::GetRandomFlyer(pTeam, AI::AFFIL_ENEMY);
    if (!pFlyer)
    {
        return;
    }
    GameObject* pTarget = SpaceAssault::mInstance->GetRandomCritSys(
        pTeam, AIDifficulty::mCritSysBias);
    if (!pTarget)
    {
        return;
    }

    Controllable* pCtrl = pFlyer->GetControllable();
    DamageOwner owner(pCtrl->mCharacter, pFlyer, pFlyer->GetTeam(),
                      pFlyer->mEntityClass, 0, 1.0f);
    float fBestDamage = 0.0f;
    const WeaponClass* pBestWeapon = 0;
    for (int i = 0; i < pCtrl->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = pCtrl->GetWeaponPtr(i);
        const WeaponClass* pClass = pWeapon->mClass;
        OrdnanceClass* pOrdnance = pClass->mOrdnanceClass;
        float fDamage = CombatUtil::CalcDamagePerSecond(pCtrl, pClass, pTarget);
        owner.mWeapon = pClass;
        float fMinDamage = 0.0f;
        float fShieldDamage = 0.0f;
        CombatUtil::GetAverageDamageValues(owner, pOrdnance, pTarget,
                                           fMinDamage, fShieldDamage);
        fDamage = (fDamage > fMinDamage * 2.0f) ? fDamage : fMinDamage * 2.0f;
        if (fDamage > fBestDamage)
        {
            fBestDamage = fDamage;
            pBestWeapon = pClass;
        }
    }
    if (!pBestWeapon)
    {
        return;
    }

    if (pTarget->mCurShield > 0.0f)
    {
        float fCap = fBestDamage * 5.0f;
        fCap = (fCap > 15000.0f) ? 15000.0f : fCap;
        fBestDamage = (pTarget->mCurShield > fCap) ? fCap : pTarget->mCurShield;
    }
    owner.mWeapon = pBestWeapon;
    DamageDesc damage(owner, fBestDamage);
    pTarget->ApplyDamage(damage, ~0u);
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00439930
void SpaceAssault_CheatLikeABastard()
{
    if (GameLoop::GetMissionTime() < 30.0f)
    {
        return;
    }

    float fIntervalScale = 1.0f;
    if (AIDifficulty::mAutoBalancePlayerModifier < 0)
    {
        fIntervalScale = -1.0f / (AIDifficulty::mAutoBalancePlayerModifier - 1);
    }

    const float fTime = GameLoop::GetMissionTime();
    if (fTime > AIDifficulty::mNextFlyerKillTime)
    {
        AIDifficulty::mNextFlyerKillTime = fTime + AIDifficulty::SpaceAssault_GetRandomFlyerKillTime() * fIntervalScale;
        const int bRoll = AIUtil::GetRandomInt(0, 100) > AIDifficulty::SpaceAssault_GetFlyerKillForPlayerTeam() * 100.0f;
        const int bEnemy = AIUtil::IsPlayerTeam(1) ^ bRoll;
        AIDifficulty::SpaceAssault_RandomFlyerKill(bEnemy ? Team::GetPtr(2) : Team::GetPtr(1));
    }

    if (fTime > AIDifficulty::mNextCritSysDamageTime)
    {
        AIDifficulty::mNextCritSysDamageTime = fTime + AIDifficulty::SpaceAssault_GetRandomCritSysDamageTime() * fIntervalScale;
        const float fRoll = (float)AIUtil::GetRandomInt(0, 100);
        const int bRoll = fRoll > AIDifficulty::SpaceAssault_GetCritSysKillForPlayerTeam() * 100.0f;
        const int bEnemy = AIUtil::IsPlayerTeam(1) ^ bRoll;
        AIDifficulty::SpaceAssault_RandomCritSysDamage(bEnemy ? Team::GetPtr(2) : Team::GetPtr(1));
    }
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00439320
int GetAutoBalanceMode()
{
    if (FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_1FLAG ||
        FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_2FLAG)
    {
        return 0;
    }
    if (FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_CAMPAIGN)
    {
        return 3;
    }
    if (SpaceAssault::mInstance && SpaceAssault::mInstance->mbEnabled)
    {
        return 1;
    }
    if (Team::GetPtr(1)->GetCurReinforcements() >= 1000 || Team::GetPtr(2)->GetCurReinforcements() >= 1000)
    {
        return 3;
    }
    return 2;
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00439380
bool AutoBalanceActive()
{
    if (AIDifficulty::gAutoBalanceEnabled && AIDifficulty::mAutoBalanceEnabled)
    {
        return AIDifficulty::GetAutoBalanceMode() != 3;
    }
    return false;
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x004393b0
void GetAutoBalanceScores(int& iPlayerScore, int& iEnemyScore, int& iThreshold, int& iDivisor)
{
    bool bPlayerTeam1 = AIUtil::IsPlayerTeam(1);
    Team* pPlayer = bPlayerTeam1 ? Team::GetPtr(1) : Team::GetPtr(2);
    Team* pEnemy = bPlayerTeam1 ? Team::GetPtr(2) : Team::GetPtr(1);
    switch (AIDifficulty::GetAutoBalanceMode())
    {
    case 0:
        iPlayerScore = pPlayer->GetPoints();
        iEnemyScore = pEnemy->GetPoints();
        iThreshold = 0;
        iDivisor = 1;
        break;
    case 1:
        iPlayerScore = pPlayer->GetPoints();
        iEnemyScore = pEnemy->GetPoints();
        iThreshold = 20;
        iDivisor = 10;
        break;
    case 2:
        iPlayerScore = pPlayer->GetCurReinforcements();
        iEnemyScore = pEnemy->GetCurReinforcements();
        iThreshold = 15;
        iDivisor = 10;
        break;
    }
}
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00439470
void UpdateAutoBalance()
{
    int iDifficulty = NetGame::IsEnabled() ? PlayerPrefs::mAllUserPrefs[0].mDifficulty
                                           : PlayerProfile::GetProfile(0)->mGameOptions.mCurDiffLevel;
    if (iDifficulty != AIDifficulty::GetProfileDifficulty())
    {
        AIDifficulty::mProfileDifficulty = iDifficulty;
    }
    AIDifficulty::mAutoBalancePlayerModifier = 0;
    AIDifficulty::mAutoBalanceEnemyModifier = 0;

    if (!AIDifficulty::AutoBalanceActive())
    {
        return;
    }
    bool bPlayerTeam1 = AIUtil::IsPlayerTeam(1);
    if (!(bPlayerTeam1 ^ AIUtil::IsPlayerTeam(2)))
    {
        return;
    }

    int iPlayerScore, iEnemyScore, iThreshold, iDivisor;
    AIDifficulty::GetAutoBalanceScores(iPlayerScore, iEnemyScore, iThreshold, iDivisor);

    Team* pEnemy = bPlayerTeam1 ? Team::GetPtr(2) : Team::GetPtr(1);
    float fMax = (float)pEnemy->GetMaxReinforcements();
    float fCur = (float)pEnemy->GetCurReinforcements();
    int iHandicap = 0;
    if (fMax * 0.25f > fCur)
    {
        iHandicap = -20;
    }
    else if (fMax * 0.5f > fCur)
    {
        iHandicap = -10;
    }

    int iDiff = iPlayerScore - iHandicap - iEnemyScore;
    if (abs(iDiff) > iThreshold)
    {
        int iStep = (iDiff - (iDiff > 0 ? iThreshold : -iThreshold)) / iDivisor;
        int iPlayer = -iStep;
        AIDifficulty::mAutoBalancePlayerModifier = iPlayer < -4 ? -4 : (iPlayer > 4 ? 4 : iPlayer);
        AIDifficulty::mAutoBalanceEnemyModifier = iStep < -4 ? -4 : (iStep > 4 ? 4 : iStep);
    }

    if (AIDifficulty::GetAutoBalanceMode() == 1)
    {
        AIDifficulty::SpaceAssault_CheatLikeABastard();
    }
}
}

// FUNCTION: SWBF2 0x00438c50
void AIDifficulty::SetAILuaModifier(int iPlayer, int iEnemy, int iDifficulty)
{
    if ((unsigned int)(iDifficulty - 1) <= 2)
    {
        AIDifficulty::mScriptPlayerModifier[iDifficulty - 1] = iPlayer;
        AIDifficulty::mScriptEnemyModifier[iDifficulty - 1] = iEnemy;
    }
}
