#include "PebbleFL/Common/PblMath.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GameObject.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Controller.h"
#include "Damageable.h"
#include "EventManager_Object.h"
#include "FLEffect.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "PlayerStats.h"
#include "PropModelMesh.h"
#include "TargetManager.h"
#include "VOHelper.h"
#include "Team.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Portals/Portal.h"
#include "RedEngineFL/Util/RedSceneObject.h"
#include "TeamManager.h"

namespace EventManager
{
template <>
void Event<GameObject, char>::Callback::Trigger(GameObject& item, char& param);
}

// FUNCTION: SWBF2 0x00536120
GameObjectClass::GameObjectClass(unsigned int uHash)
    : EntityGeometryClass(uHash)
{
    mAINoRepair = false;
    mHeroFlag = false;
    mAvailableForAnyTeam = false;
    mIsNotTargetableByPlayer = false;
    mbHideHealthBar = false;

    mHealthType = HEALTH_TYPE_PERSON;
    mHealthTypeForLockOn = NUM_HEALTH_TYPES;
    mIsNeutralWhenEmpty = true;
    mMaxHealth = FLT_MAX;
    mAddHealth = 0.0f;
    mMaxShield = 0.0f;
    mAddShield = 0.0f;
    mPilotSkillRepairScale = 0.005f;
    mScanningRange = 0.0f;
    mTransmitRange = 0.0f;
    mAIType = 1;
    mSwitchClassRadius = -1.0f;
    mHoldForPlayer = false;

    memset(mDamageEffectClass, 0, sizeof(mDamageEffectClass));

    mVOSoundList = 0;
    mVOUnitType = 0;
    mVORadius = 50.0f;
    mAICollisionRadius = 0.0f;
    mLockOnMinDist = 20.0f;
    mLockOnMaxDist = 100.0f;
    mLockOnMaxTrackDist = 200000.0f;
    mPointsToUnlock = 0;
    mDisabledEffect = 0xBB7E863Du /* "com_sfx_hackedvehicle" */;
    mRadiusDamageSizeScale = 1.0f;
}

// FUNCTION: SWBF2 0x005365a0
GameObjectClass::~GameObjectClass()
{
}

// SYNTHETIC: SWBF2 0x00536270
// GameObjectClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00403dd0
// `dynamic initializer for 'GameObjectClass::rttiHashGameObjectClass''
// GameObjectClass::rttiHashGameObjectClass

// GLOBAL: SWBF2 0x01ec3234
// GameObjectClass::rttiHashGameObjectClass
PblHash GameObjectClass::rttiHashGameObjectClass("GameObjectClass");
// GLOBAL: SWBF2 0x00789998
// GameObjectClass::rttiNameGameObjectClass
const char GameObjectClass::rttiNameGameObjectClass[] = "GameObjectClass";







// SYNTHETIC: SWBF2 0x00403dc0
// `dynamic initializer for 'GameObject::rttiHashGameObject''
// GameObject::rttiHashGameObject

// GLOBAL: SWBF2 0x01ec0c7c
// GameObject::rttiHashGameObject
PblHash GameObject::rttiHashGameObject("GameObject");
// GLOBAL: SWBF2 0x007899ac
// GameObject::rttiNameGameObject
const char GameObject::rttiNameGameObject[] = "GameObject";







// FUNCTION: SWBF2 0x00444250
const Controllable* GameObject::GetControllable() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00444260
Controllable* GameObject::GetControllable()
{
    return 0;
}

// FUNCTION: SWBF2 0x004442b0
void GameObject::ApplyRadiusPush(const PblVector3& vPos, float fInnerRadius, float fOuterRadius, float fPush,
                                 bool bArg)
{
}

// FUNCTION: SWBF2 0x00444370
void GameObject::HeardEnemy()
{
}

// FUNCTION: SWBF2 0x004443b0
bool GameObject::Deflect(const Ordnance* pOrdnance, const PblVector3& vPos, const PblVector3& vDir)
{
    return false;
}

namespace AIUtil
{
bool __fastcall IsSpied(Character* pCharacter, Character* pOther);
}


// FUNCTION: SWBF2 0x00535480
void GameObject::Init()
{
    EventManager::ObjectEvent::mInit.Trigger(*this, EventManager::noContext);
}

// FUNCTION: SWBF2 0x005354c0
void GameObject::ClientKill()
{
    TargetManager::RemoveMarker(this, 0, (unsigned int)-1);
}

// FUNCTION: SWBF2 0x005354d0
void GameObject::Kill()
{
    EventManager::ObjectEvent::mKill.Trigger(*this, mDamageOwner);
    TargetManager::RemoveMarker(this, 0, (unsigned int)-1);
    SetPortalActive(false);
}

// FUNCTION: SWBF2 0x00535540
void GameObject::Respawn()
{
    EventManager::ObjectEvent::mRespawn.Trigger(*this, EventManager::noContext);
}

// FUNCTION: SWBF2 0x00535590
const PblMatrix& GameObject::GetSmoothedMatrix() const
{
    return GetMatrix();
}

// FUNCTION: SWBF2 0x005355a0
void GameObject::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float fValue;
    switch (uProperty)
    {
    case 0x19971f1b:
        if (sscanf(pszValue, "%f", &fValue))
        {
            mMaxHealth = fValue;
        }
        break;
    case 0x3f9ab262:
        if (sscanf(pszValue, "%f", &fValue))
        {
            mAddHealth = fValue;
        }
        break;
    case 0xf2bf9d5f:
        if (sscanf(pszValue, "%f", &fValue))
        {
            SetCurHealth(fValue, true, 0);
        }
        break;
    case 0xda3491bc:
        if (sscanf(pszValue, "%f", &fValue))
        {
            mMaxShield = fValue;
        }
        break;
    case 0xbad94c15:
        if (sscanf(pszValue, "%f", &fValue))
        {
            mAddShield = fValue;
        }
        break;
    case 0xcbbafa80:
        if (sscanf(pszValue, "%f", &fValue))
        {
            SetCurShield(fValue, true, 0);
        }
        break;
    case 0x1c1694fa:
        if (sscanf(pszValue, "%f", &fValue))
        {
            mDisableTime = fValue;
        }
        break;
    case 0xa2fd7d0c:
        SetTeam(atoi(pszValue));
        mPerceivedTeam = atoi(pszValue);
        break;
    case 0x52a9ef0f:
        mPerceivedTeam = atoi(pszValue);
        break;
    case 0xb54cdd84:
        if (atoi(pszValue))
        {
            ActivatePhysics();
        }
        else
        {
            DeactivatePhysics();
        }
        break;
    default:
        EntityGeometry::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00535830
void GameObject::SetTeam(int iTeam)
{
    bool bChanged = GetCollisionObject() && GetCollisionObject()->IsInCollisionList();
    bChanged &= mTeam != iTeam;
    if (bChanged)
    {
        TeamManager::RemoveObject(this);
    }
    mTeam = iTeam;
    if (bChanged)
    {
        TeamManager::AddObject(this);
    }
}

// FUNCTION: SWBF2 0x005358a0
float GameObject::CalcRadiusEffect(float fDist, float fInnerRadius, float fOuterRadius) const
{
    if (const CollisionObject* pCollision = GetCollisionObject())
    {
        fDist -= pCollision->GetCollisionRadius() * 0.5f * GetGameObjectClass()->mRadiusDamageSizeScale;
    }
    if (fInnerRadius > fDist)
    {
        return 1.0f;
    }
    if (fOuterRadius > fDist)
    {
        return (fOuterRadius - fDist) / (fOuterRadius - fInnerRadius);
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x00535940
const PblList<Controllable>& GameObject::GetControllableList() const
{
    return mControllableList;
}

// FUNCTION: SWBF2 0x00535950
void GameObject::ApplyRadiusDamage(const PblVector3& vPos, float fInnerRadius, float fOuterRadius,
                                   const DamageDesc& damage)
{
    if (NetGame::IsPreGame())
    {
        return;
    }
    CollisionObject* pCollision = GetCollisionObject();
    if (!pCollision)
    {
        return;
    }
    float fEffect = CalcRadiusEffect(Length(pCollision->GetCollisionCenter() - vPos), fInnerRadius,
                                     fOuterRadius);
    if (fEffect > 0.0f)
    {
        DamageDesc desc = damage;
        desc.mBaseValue *= fEffect;
        Damageable::ApplyDamage(desc, (unsigned int)-1);
    }
}

// FUNCTION: SWBF2 0x00535a50
bool GameObject::IsVisibleToMe(const GameObject*) const
{
    return true;
}

// FUNCTION: SWBF2 0x00535a60
int GameObject::GetAffiliationToMe(const GameObject* pObject) const
{
    Team* pTeam = Team::sTeams[mTeam];
    int iPerceived = pTeam->GetAffiliation(pObject->mPerceivedTeam);
    int iReal = pTeam->GetAffiliation(pObject->mTeam);
    if (iReal > 0)
    {
        return iReal;
    }
    if (iReal < iPerceived)
    {
        Character* pMine = GetControllable() ? GetControllable()->mCharacter : 0;
        Character* pTheirs =
            pObject->GetControllable() ? pObject->GetControllable()->mCharacter : 0;
        if (pMine && pTheirs && AIUtil::IsSpied(pMine, pTheirs))
        {
            return iReal;
        }
    }
    return iPerceived;
}

// FUNCTION: SWBF2 0x00535b10
bool GameObject::IsMyFriend(const GameObject* pObject) const
{
    return GetAffiliationToMe(pObject) > 0;
}

// FUNCTION: SWBF2 0x00535b30
bool GameObject::IsMyEnemy(const GameObject* pObject) const
{
    return GetAffiliationToMe(pObject) < 0;
}

// FUNCTION: SWBF2 0x00535b50
void GameObject::AddPilotSkillCount(int iCount)
{
    mAddHealth += iCount * mMaxHealth * GetGameObjectClass()->mPilotSkillRepairScale;
}

// FUNCTION: SWBF2 0x00535b90
void GameObject::PlayDeathSound()
{
    if (mGameObjectAttachedSound.mFlags & GameSoundControllable::flags_playing)
    {
        mGameObjectAttachedSound.Stop(true);
    }
    const PblMatrix& matrix = GetMatrix();
    const PblVector3& vVelocity = GetVelocity();
    mGameObjectAttachedSound.Play(&GetGameObjectClass()->mSoundDeath, &matrix.GetTranslation(),
                                  &vVelocity, true);
}

// FUNCTION: SWBF2 0x00535be0
void GameObject::PlayHurtSound(bool bByCharacter)
{
    if (mGameObjectAttachedSound.mFlags & GameSoundControllable::flags_playing)
    {
        mGameObjectAttachedSound.Stop(true);
    }
    const PblMatrix& matrix = GetMatrix();
    const PblVector3& vVelocity = GetVelocity();
    if (!bByCharacter)
    {
        mGameObjectAttachedSound.Play(&GetGameObjectClass()->mSoundHurtFall, &matrix.GetTranslation(),
                                      &vVelocity, true);
    }
    else
    {
        mGameObjectAttachedSound.Play(&GetGameObjectClass()->mSoundHurt, &matrix.GetTranslation(),
                                      &vVelocity, true);
    }
}

// FUNCTION: SWBF2 0x00535c50
void GameObject::PlayDamageRegionSound(unsigned int uRegionId)
{
    PblListSingle& list = GetGameObjectClass()->mSoundDamageRegionList;
    PblListSingle::Node* pFirst = list.mHead.mNext;
    if (pFirst == &list.mHead ||
        (mGameObjectAttachedSound.mFlags & GameSoundControllable::flags_playing))
    {
        return;
    }
    PblListSingle::Node* pNode = pFirst;
    DamageRegionSound* pSound;
    for (;;)
    {
        pSound = (pNode == &list.mHead)
                     ? 0
                     : (DamageRegionSound*)((char*)pNode - offsetof(DamageRegionSound, mNode));
        if (pSound->mRegionID == uRegionId)
        {
            break;
        }
        pNode = pNode->mNext;
        if (pNode == &list.mHead)
        {
            pSound = (DamageRegionSound*)((char*)pFirst - offsetof(DamageRegionSound, mNode));
            break;
        }
    }
    const PblMatrix& matrix = GetMatrix();
    mGameObjectAttachedSound.Play(&pSound->mSound, &matrix.GetTranslation(), &GetVelocity(), true);
}

// FUNCTION: SWBF2 0x00535cc0
FlagItem* GameObject::GetCarriedFlagItem() const
{
    if (GetControllable())
    {
        if ((GetControllable()->IsPilotable() || GetControllable()->GetPilotType() == PILOT_SELF) &&
            GetControllable()->mCharacter && GetControllable()->mCharacter->mCarriedFlagPtr)
        {
            return GetControllable()->mCharacter->mCarriedFlagPtr;
        }
    }
    for (PblList<Controllable>::Itor it(GetControllableList()); (Controllable*)it; ++it)
    {
        if ((it->IsPilotable() || it->GetPilotType() == PILOT_SELF) && it->mCharacter &&
            it->mCharacter->mCarriedFlagPtr)
        {
            return it->mCharacter->mCarriedFlagPtr;
        }
    }
    return 0;
}

namespace PropGenerator
{
const PropObject* FindPropObject(const PblVector3& vPos, bool bParam);
}

// FUNCTION: SWBF2 0x00535d90
bool GameObject::FoliageCollision()
{
    bool bPlayed = false;
    const PblVector3& vPos = GetMatrix().GetTranslation();
    const PropObjectMesh* pFoliage = static_cast<const PropObjectMesh*>(
        PropGenerator::FindPropObject(vPos, false));
    float fCrouch = 0.8f;
    float fStand = 1.0f;
    if (pFoliage)
    {
        PropModelMesh* pModel = pFoliage->mModel;
        fCrouch = pModel->mAIVisFactorCrouch;
        fStand = pModel->mAIVisFactorStand;
        if (pFoliage != mPrevFoliageObj)
        {
            GameSound sound;
            sound = pModel->mSoundCollision;
            sound.Play(&vPos, 0, false, true);
            bPlayed = true;
        }
    }
    mPrevFoliageObj = (void*)pFoliage;
    if (GetControllable() && GetControllable()->mCtrl)
    {
        GetControllable()->mCtrl->SetFoliageVisFactor(fCrouch, fStand);
    }
    return bPlayed;
}

// FUNCTION: SWBF2 0x00535e60
float GameObject::GetDamageMultiplier(int iBodyId) const
{
    if (iBodyId == -1)
    {
        return 1.0f;
    }
    const GameObjectClass* pClass = GetGameObjectClass();
    for (int i = 0; i < pClass->mNumHitLocations; ++i)
    {
        if (pClass->mHitLocations[i].mBodyId == iBodyId)
        {
            return pClass->mHitLocations[i].mDamageMultiplier;
        }
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x00535eb0
void GameObject::AwardVehicleKillPoints(int iVehicleType, bool bEmpty)
{
    if (Character* pCharacter = mDamageOwner.mCharacter)
    {
        int iTeam = mDamageOwner.mTeam ? mDamageOwner.mTeam : pCharacter->mTeamNumber;
        if (mOwningTeam == iTeam)
        {
            return;
        }
        if (bEmpty)
        {
            Character::GetPlayerStats(pCharacter).PointsProcess(13, 0);
        }
        else
        {
            Character::GetPlayerStats(pCharacter).AddVehicleKill(iVehicleType);
        }
        return;
    }
    if (!(GameObject*)mDamageOwner.mGameObject)
    {
        return;
    }
    int iTeam = mDamageOwner.mTeam ? mDamageOwner.mTeam : mDamageOwner.mGameObject->mTeam;
    if (mOwningTeam == iTeam)
    {
        return;
    }
    if (bEmpty)
    {
        PlayerStats::PointsProcessTeam(iTeam, 13);
        return;
    }
    int iKillerType = 0;
    if (Controllable* pControllable = mDamageOwner.mGameObject->GetControllable())
    {
        iKillerType = pControllable->GetGameObject()->GetGameObjectClass()->GetVehicleType();
    }
    if (iVehicleType == 8)
    {
        PlayerStats::PointsProcessTeam(iTeam, 12);
    }
    else if (iKillerType >= 1 && iKillerType <= 3)
    {
        if (iKillerType > iVehicleType)
        {
            PlayerStats::PointsProcessTeam(iTeam, 9);
        }
        else
        {
            PlayerStats::PointsProcessTeam(iTeam, 6);
        }
    }
    else
    {
        PlayerStats::PointsProcessTeam(iTeam, 5);
    }
}

// FUNCTION: SWBF2 0x00536020
void GameObject::TriggerDisabledEffect()
{
    if (mDisableTime > 0.0f && IsAlive())
    {
        if (mDisabledEffectHandle)
        {
            return;
        }
        unsigned int uEffect = GetGameObjectClass()->mDisabledEffect;
        if (!uEffect)
        {
            return;
        }
        FLEffectClass* pClass = FLEffect::Get(uEffect);
        if (!pClass)
        {
            return;
        }
        FLEffectObject* pEffect = pClass->BuildEffect();
        if (!pEffect)
        {
            return;
        }
        pEffect->AttachEffectToObject(this, true);
        mDisabledEffectHandle = pEffect;
    }
    else if (mDisabledEffectHandle)
    {
        mDisabledEffectHandle->StopAndFinish();
        mDisabledEffectHandle = 0;
    }
}


// SYNTHETIC: SWBF2 0x00403de0
// `dynamic initializer for 'gameObjectInterpolator''

// GLOBAL: SWBF2 0x01ec3240
// gameObjectInterpolator
GameObjectInterpolator gameObjectInterpolator;

// FUNCTION: SWBF2 0x00537460
void GameObjectInterpolator::GetMatrix(int iNetUniqueId, PblMatrix& mMatrix)
{
    if ((unsigned int)iNetUniqueId >= 64)
    {
        return;
    }
    const int iCount = count[iNetUniqueId];
    if (iCount < 1)
    {
        return;
    }
    if (iCount < 2)
    {
        mMatrix = mat[iNetUniqueId][(iCount - 1) & 1];
        return;
    }

    const float fRatio = NetGame::GetTurnRatio();
    const int iSampleCount = count[iNetUniqueId];
    const PblMatrix m0 = mat[iNetUniqueId][iSampleCount & 1];
    const PblMatrix m1 = mat[iNetUniqueId][(iSampleCount - 1) & 1];
    PblQuaternion q0(m0), q1(m1);
    const PblVector3 vBlend = Interpolate(m0.GetTranslation(), m1.GetTranslation(), fRatio);
    const PblQuaternion qBlend = Interpolate(q0, q1, fRatio);
    mMatrix = PblMatrix(qBlend, vBlend);
}

// FUNCTION: SWBF2 0x00537400
void GameObjectInterpolator::SetNext(int iNetUniqueId, const PblMatrix* pMatrix)
{
    if (iNetUniqueId < 0)
    {
        return;
    }
    if (pMatrix)
    {
        mat[iNetUniqueId][count[iNetUniqueId] & 1] = *pMatrix;
        ++count[iNetUniqueId];
    }
    else
    {
        count[iNetUniqueId] = 0;
    }
}

// FUNCTION: SWBF2 0x00537630
void GameObjectInterpolator::SetNextFloat(int iNetUniqueId, int iIndex, float fValue)
{
    if (iNetUniqueId >= 0)
    {
        mFloats[iNetUniqueId][iIndex][(count[iNetUniqueId] - 1) & 1] = fValue;
    }
}

// FUNCTION: SWBF2 0x005376f0
float GameObjectInterpolator::GetCurFloat(int iNetUniqueId, int iIndex)
{
    if (iNetUniqueId < 0)
    {
        return 0.0f;
    }
    if (count[iNetUniqueId] < 1)
    {
        return 0.0f;
    }
    return mFloats[iNetUniqueId][iIndex][(count[iNetUniqueId] - 1) & 1];
}

// FUNCTION: SWBF2 0x00537730
float GameObjectInterpolator::GetPrevFloat(int iNetUniqueId, int iIndex)
{
    if (iNetUniqueId < 0)
    {
        return 0.0f;
    }
    if (count[iNetUniqueId] < 1)
    {
        return 0.0f;
    }
    if (count[iNetUniqueId] < 2)
    {
        return mFloats[iNetUniqueId][iIndex][(count[iNetUniqueId] - 1) & 1];
    }
    return mFloats[iNetUniqueId][iIndex][count[iNetUniqueId] & 1];
}

// FUNCTION: SWBF2 0x005378b0
void GameObjectInterpolator::SetNextVector(int iNetUniqueId, const PblVector3* pVector)
{
    if (iNetUniqueId >= 0 && pVector)
    {
        vec[iNetUniqueId][(count[iNetUniqueId] - 1) & 1] = *pVector;
    }
}

// FUNCTION: SWBF2 0x005378f0
void GameObjectInterpolator::GetVector(int iNetUniqueId, PblVector3& vVector)
{
    if (iNetUniqueId < 0 || count[iNetUniqueId] < 2)
    {
        return;
    }
    const float fRatio = NetGame::GetTurnRatio();
    const int iSampleCount = count[iNetUniqueId];
    const PblVector3 v0 = vec[iNetUniqueId][iSampleCount & 1];
    const PblVector3 v1 = vec[iNetUniqueId][(iSampleCount - 1) & 1];
    vVector = Interpolate(v0, v1, fRatio);
}

// FUNCTION: SWBF2 0x00537660
float GameObjectInterpolator::GetFloat(int iNetUniqueId, int iIndex)
{
    if (iNetUniqueId < 0)
    {
        return 0.0f;
    }
    if (count[iNetUniqueId] < 1)
    {
        return 0.0f;
    }

    const float fPrevious = mFloats[iNetUniqueId][iIndex][(count[iNetUniqueId] - 1) & 1];
    if (count[iNetUniqueId] < 2)
    {
        return fPrevious;
    }
    const float fRatio = NetGame::GetTurnRatio();
    return Interpolate(mFloats[iNetUniqueId][iIndex][count[iNetUniqueId] & 1], fPrevious, fRatio);
}

// FUNCTION: SWBF2 0x005377a0
float GameObjectInterpolator::GetLoopingFloat(int iNetUniqueId, int iIndex, float fDirection,
                                               float fMax, bool& bWrapped)
{
    bWrapped = false;
    if (iNetUniqueId < 0)
    {
        return 0.0f;
    }
    if (count[iNetUniqueId] < 1)
    {
        return 0.0f;
    }

    const float fPrevious = mFloats[iNetUniqueId][iIndex][(count[iNetUniqueId] - 1) & 1];
    if (count[iNetUniqueId] < 2)
    {
        return fPrevious;
    }

    const float fRatio = NetGame::GetTurnRatio();
    float fCurrent = mFloats[iNetUniqueId][iIndex][count[iNetUniqueId] & 1];
    if ((fPrevious - fCurrent) * fDirection < 0.0f)
    {
        if (fDirection > 0.0f)
        {
            float fValue = Interpolate(fCurrent, fPrevious + fMax, fRatio);
            if (fValue <= fMax)
            {
                bWrapped = true;
                return fValue;
            }
            return fValue - fMax;
        }
        fCurrent += fMax;
        float fValue = Interpolate(fCurrent, fPrevious, fRatio);
        if (fValue > fMax)
        {
            bWrapped = true;
            fValue -= fMax;
        }
        return fValue;
    }
    return Interpolate(fCurrent, fPrevious, fRatio);
}

extern bool netHeroesEnabled;

// FUNCTION: SWBF2 0x005350d0
GameObject::GameObject(GameObjectClass* pClass, const EntityDesc& desc, CollisionObjectType eType,
                       const RedLodData* pLodData)
    : EntityGeometry(pClass, desc, eType, pLodData), mNetUniqueId(-1), mPrevFoliageObj(0)
{
    mCreatedTime = GameLoop::GetMissionTime();
    mTeam = 0;
    mPerceivedTeam = 0;
    mOwningTeam = 0;
    mIsTargetable = pClass->mMaxHealth < FLT_MAX;

    mMaxHealth = pClass->mMaxHealth;
    SetCurHealth(pClass->mMaxHealth, false, 0);
    if (!netHeroesEnabled || !pClass->mHeroFlag)
    {
        mAddHealth = pClass->mAddHealth;
    }
    SetCurShield(pClass->mMaxShield, false, 0);
    mMaxShield = pClass->mMaxShield;
    if (!netHeroesEnabled || !pClass->mHeroFlag)
    {
        mAddShield = pClass->mAddShield;
    }
    mHealthType = pClass->mHealthType;
    mHealthTypeForLockOn = pClass->mHealthTypeForLockOn;

    EventManager::ObjectEvent::mCreate.Trigger(*this, EventManager::noContext);
}

// FUNCTION: SWBF2 0x005352e0
GameObject::~GameObject()
{
    if (IsAlive())
    {
        for (PblList<Controllable>::Itor it(GetControllableList()); (Controllable*)it; ++it)
        {
            it->SetPilot(0);
        }
    }
    TargetManager::RemoveMarker(this, 0, (unsigned int)-1);
    EventManager::ObjectEvent::mDelete.Trigger(*this, EventManager::noContext);
    pvs::IManager& pvsManager = pvs::IManager::GetInstance();
    SetPortalActive(false);
    pvsManager.UnregisterDynamic(*this);
    TeamManager::RemoveObject(this);
    if (mDisabledEffectHandle)
    {
        mDisabledEffectHandle->StopAndFinish();
    }
}

// SYNTHETIC: SWBF2 0x005350a0
// GameObject::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00537ae7
// ??_EGameObject@@WBEA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00537af2
// ??_EGameObject@@WM@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00537afa
// ??_EGameObject@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x00537b05
// ??_EGameObject@@WJE@AEPAXI@Z

// FUNCTION: SWBF2 0x005362a0
GameObjectClass::GameObjectClass(GameObjectClass* pBase, unsigned int uHash)
    : EntityGeometryClass(pBase, uHash)
{
    mHealthType = pBase->mHealthType;
    mHealthTypeForLockOn = pBase->mHealthTypeForLockOn;
    mMaxHealth = pBase->mMaxHealth;
    mAddHealth = pBase->mAddHealth;
    mMaxShield = pBase->mMaxShield;
    mAddShield = pBase->mAddShield;
    mbHideHealthBar = pBase->mbHideHealthBar;
    mPilotSkillRepairScale = pBase->mPilotSkillRepairScale;
    mScanningRange = pBase->mScanningRange;
    mTransmitRange = pBase->mTransmitRange;
    mSoundHurt = pBase->mSoundHurt;
    mSoundHurtFall = pBase->mSoundHurtFall;
    mSoundDeath = pBase->mSoundDeath;

    for (PblListSingle::Itor it(pBase->mSoundDamageRegionList); it.GetNode() != pBase->mSoundDamageRegionList.GetTerminator(); ++it)
    {
        const DamageRegionSound* pSound =
            (const DamageRegionSound*)((char*)it.GetNode() - offsetof(DamageRegionSound, mNode));
        new DamageRegionSound(pSound->mRegionID, pSound->mSound, mSoundDamageRegionList);
    }

    mAIType = pBase->mAIType;
    mAINoRepair = pBase->mAINoRepair;
    mHeroFlag = pBase->mHeroFlag;
    mSwitchClassRadius = pBase->mSwitchClassRadius;
    mIsNeutralWhenEmpty = pBase->mIsNeutralWhenEmpty;
    mIsNotTargetableByPlayer = pBase->mIsNotTargetableByPlayer;
    mHoldForPlayer = pBase->mHoldForPlayer;
    mAvailableForAnyTeam = pBase->mAvailableForAnyTeam;

    memcpy(mDamageEffectClass, pBase->mDamageEffectClass, sizeof(mDamageEffectClass));

    mVOSoundList = pBase->mVOSoundList;
    mVOUnitType = pBase->mVOUnitType;
    mVORadius = pBase->mVORadius;
    mAICollisionRadius = pBase->mAICollisionRadius;
    mLockOnMinDist = pBase->mLockOnMinDist;
    mLockOnMaxDist = pBase->mLockOnMaxDist;
    mLockOnMaxTrackDist = pBase->mLockOnMaxTrackDist;
    mPointsToUnlock = pBase->mPointsToUnlock;
    mDisabledEffect = pBase->mDisabledEffect;
    mRadiusDamageSizeScale = pBase->mRadiusDamageSizeScale;
}

// GLOBAL: SWBF2 0x01e56388
static DamageEffectClass* sCurDamageEffect;

// FUNCTION: SWBF2 0x005365c0
void GameObjectClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    char acRegion[256];
    char acSound[256];
    char acBuffer[512];
    switch (uProperty)
    {
    case 0x19971f1b:
        sscanf(pszValue, "%f", &mMaxHealth);
        break;
    case 0x3f9ab262:
        sscanf(pszValue, "%f", &mAddHealth);
        break;
    case 0xda3491bc:
        sscanf(pszValue, "%f", &mMaxShield);
        break;
    case 0xbad94c15:
        sscanf(pszValue, "%f", &mAddShield);
        break;
    case 0x0842b6a7:
        sscanf(pszValue, "%f", &mPilotSkillRepairScale);
        break;
    case 0x465909fb:
        sscanf(pszValue, "%f", &mScanningRange);
        break;
    case 0xeb460a76:
        sscanf(pszValue, "%f", &mTransmitRange);
        break;
    case 0x56dbdfe1:
        sscanf("%f", pszValue, &mLockOnMinDist);
        break;
    case 0x0aa7e7a3:
        sscanf("%f", pszValue, &mLockOnMaxDist);
        break;
    case 0x596d0d56:
        sscanf("%f", pszValue, &mLockOnMaxTrackDist);
        break;
    case 0x714b2463:
        sscanf(pszValue, "%d", &mPointsToUnlock);
        break;
    case 0x4bc9b119:
        mSwitchClassRadius = (float)atof(pszValue);
        break;
    case 0x4217f84f:
        mAICollisionRadius = (float)atof(pszValue);
        break;
    case 0x9c1a4523:
        mRadiusDamageSizeScale = (float)atof(pszValue);
        break;
    case 0xc9239720:
        mVORadius = (float)atof(pszValue);
        mVORadius = __max(mVORadius, 0.0f);
        break;
    case 0x6196493c:
        mVOUnitType = atoi(pszValue);
        mVOUnitType = Clamp(mVOUnitType, 0, 255);
        break;
    case 0xa8da4b94:
        mDisabledEffect = PblHash(pszValue);
        break;
    case 0xadf73cb1:
        mHoldForPlayer = atoi(pszValue) != 0;
        break;

    case 0xe8821677:
        switch (PblHash(pszValue))
        {
        case 0xd397b280: mHealthType = HEALTH_TYPE_PERSON; break;
        case 0xc238e05d: mHealthType = HEALTH_TYPE_ANIMAL; break;
        case 0xeddb2a39: mHealthType = HEALTH_TYPE_DROID; break;
        case 0x71cd882f: mHealthType = HEALTH_TYPE_VEHICLE; break;
        case 0x38df0375: mHealthType = HEALTH_TYPE_BUILDING; break;
        case 0x7e89edb6: mHealthType = HEALTH_TYPE_MINE; break;
        }
        break;
    case 0x82f3bb78:
        switch (PblHash(pszValue))
        {
        case 0xd397b280: mHealthTypeForLockOn = HEALTH_TYPE_PERSON; break;
        case 0xc238e05d: mHealthTypeForLockOn = HEALTH_TYPE_ANIMAL; break;
        case 0xeddb2a39: mHealthTypeForLockOn = HEALTH_TYPE_DROID; break;
        case 0x71cd882f: mHealthTypeForLockOn = HEALTH_TYPE_VEHICLE; break;
        case 0x38df0375: mHealthTypeForLockOn = HEALTH_TYPE_BUILDING; break;
        case 0x7e89edb6: mHealthTypeForLockOn = HEALTH_TYPE_MINE; break;
        }
        break;

    case 0xaff22806:
        if (!_stricmp(pszValue, "SOLDIER"))
        {
            mAIType = 1;
        }
        else if (!_stricmp(pszValue, "HOVER"))
        {
            mAIType = 2;
        }
        else if (!_stricmp(pszValue, "SMALL"))
        {
            mAIType = 4;
        }
        else if (!_stricmp(pszValue, "MEDIUM"))
        {
            mAIType = 8;
        }
        else if (!_stricmp(pszValue, "HUGE"))
        {
            mAIType = 16;
        }
        else if (!_stricmp(pszValue, "FLYER"))
        {
            mAIType = 32;
        }
        else
        {
            mAIType = 1;
        }
        break;

    case 0xbbbf02b1:
        mAINoRepair = atoi(pszValue) != 0;
        break;
    case 0x0ffe3c06:
        mAvailableForAnyTeam = atoi(pszValue) != 0;
        break;
    case 0xe921a7d8:
        mbHideHealthBar = atol(pszValue) != 0;
        break;
    case 0xa1c0f27b:
        if (atoi(pszValue))
        {
            mbHideHealthBar = true;
        }
        break;
    case 0x1b2d820b:
        if (atoi(pszValue))
        {
            mIsNotTargetableByPlayer = true;
        }
        break;

    case 0x6a9d502c:
        mSoundDeath.SetID(pszValue, false, false);
        break;
    case 0x72981af7:
        mSoundHurt.SetID(pszValue, false, false);
        break;
    case 0xf3697818:
        mSoundHurtFall.SetID(pszValue, false, false);
        break;

    case 0x7241c3bb:
    {
        acSound[0] = 0;
        acRegion[0] = 0;
        sscanf(pszValue, "%255s %255s", acSound, acRegion);
        GameSound sound;
        sound.SetID(acSound, false, false);
        if (sound.GetProperties())
        {
            new DamageRegionSound(PblHash(acRegion), sound, mSoundDamageRegionList);
        }
        break;
    }

    case 0x172645b9:
    {
        strncpy(acBuffer, pszValue, 511);
        acBuffer[511] = 0;
        const char* pszSound = strtok(acBuffer, " ,+-;");
        const char* pszType = strtok(0, " ,+-;");
        AI::VOModifier modifiers;
        *(unsigned int*)&modifiers = 0;
        for (const char* pszWord = strtok(0, " ,+-;"); pszWord; pszWord = strtok(0, " ,+-;"))
        {
            AI::VOHelper::SetModifier(modifiers, pszWord, pszValue);
        }
        AI::VOSound* pSound = AI::VOHelper::GetNewVOSound(pszSound, pszType, modifiers);
        if (!mVOSoundList)
        {
            mVOSoundList = pSound;
        }
        else
        {
            AI::VOSound* pLast = mVOSoundList;
            while (pLast->mNext)
            {
                pLast = pLast->mNext;
            }
            pLast->mNext = pSound;
        }
        break;
    }

    case 0x06182385:
        sCurDamageEffect = 0;
        for (int i = 0; i < 10; ++i)
        {
            if (!mDamageEffectClass[i])
            {
                mDamageEffectClass[i] = new DamageEffectClass;
                sCurDamageEffect = mDamageEffectClass[i];
                if (sCurDamageEffect)
                {
                    sCurDamageEffect->mPercent = (float)atof(pszValue) / 100.0f;
                    sCurDamageEffect->mPercentStop = 0.0f;
                }
                break;
            }
        }
        break;
    case 0x37fe5ca9:
        if (sCurDamageEffect)
        {
            float fPercent = (float)atof(pszValue) / 100.0f;
            sCurDamageEffect->mPercentStop = fPercent;
            if (fPercent > 0.0f && fPercent <= 0.0002f)
            {
                sCurDamageEffect->mPercentStop = 0.0002f;
            }
        }
        break;
    case 0xae2bfdd1:
        if (sCurDamageEffect)
        {
            sCurDamageEffect->mNameHash = PblHash(pszValue);
        }
        break;
    case 0xca698181:
        if (sCurDamageEffect)
        {
            sCurDamageEffect->mHardPoint = PblTEMPHash(pszValue);
        }
        break;
    case 0xdd5c94da:
        if (sCurDamageEffect)
        {
            float x, y, z;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            sCurDamageEffect->mAttachOffset = PblVector3(x, y, z);
        }
        break;
    case 0x57ce6c85:
        if (sCurDamageEffect)
        {
            sCurDamageEffect->mScale = (float)atof(pszValue);
        }
        break;
    case 0xd06d0b7c:
        if (sCurDamageEffect)
        {
            sCurDamageEffect->mInheritVelocity = (float)atof(pszValue);
        }
        break;
    case 0x27cd02e8:
        if (sCurDamageEffect)
        {
            sCurDamageEffect->mSound.SetID(pszValue, false, false);
        }
        break;

    default:
        EntityGeometryClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00537330
GameObjectClass::VehicleType GameObjectClass::FindVehicleType(const char* pszName)
{
    switch (PblHash(pszName))
    {
    case 0xc322ad05: return VEHICLE_SCOUT;
    case 0xe29d1e2f: return VEHICLE_LIGHT;
    case 0x35afd776: return VEHICLE_MEDIUM;
    case 0x119f9d40: return VEHICLE_HEAVY;
    case 0x79610f82: return VEHICLE_FIGHTER;
    case 0xd32f6312: return VEHICLE_TRANSPORT;
    case 0xe5264e25: return VEHICLE_CAPITOL;
    case 0x8908744f: return VEHICLE_REMOTE_TERMINAL;
    case 0x2078f3ad: return VEHICLE_ATAT;
    default: return VEHICLE_UNKNOWN;
    }
}

namespace EventManager
{

// FUNCTION: SWBF2 0x005379d0
template <>
void Event<GameObject, char>::Callback::Trigger(GameObject& item, char& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild)
            {
                const bool bAlive = pChild->mAliveFlag;
                if (!bAlive)
                {
                    break;
                }
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

// SYNTHETIC: SWBF2 0x00446dfe
// ?GetGameObject@GameObject@@WBEA@AEPAV1@XZ

// SYNTHETIC: SWBF2 0x00446e14
// ?GetGameObject@GameObject@@WM@AEPAV1@XZ
