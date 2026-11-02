#include "PebbleFL/Common/PblMath.h"
#include <d3dx9math.h>
#include "LockOnManager.h"
#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "GameSound.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "NetGame.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "AIUtil.h"
#include "EntityCarrier.h"
#include "GameModel.h"
#include "TargetManager.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"

#include <math.h>

#include "EntityDroideka.h"
#include "EntityHover.h"
#include "EntitySoldier.h"
#include "EntityWalker.h"
#include "MountedTurret.h"
#include <float.h>
#include "CollisionManager.h"


// GLOBAL: SWBF2 0x01e573f8
unsigned int LockOnManager::sNumLockOnManagers;

// GLOBAL: SWBF2 0x01e57400
LockOnManager* LockOnManager::sLockOnManagers[4];

// SYNTHETIC: SWBF2 0x00404180
// `dynamic initializer for 'LockOnManager::sTargetLockSound''
// SYNTHETIC: SWBF2 0x00768c80
// `dynamic atexit destructor for 'LockOnManager::sTargetLockSound''

// GLOBAL: SWBF2 0x01ec7d30
// LockOnManager::sTargetLockSound
GameSound LockOnManager::sTargetLockSound;

static inline void ClearHandle(PblHandle<GameObject>& handle)
{
    handle.mObject = 0;
    handle.mSavedHandleId = 0;
}

// FUNCTION: SWBF2 0x0057acf0
LockOnManager::LockOnManager(int iPlayerIndex)
    : mPlayerIndex(iPlayerIndex), mPlayerCharacter(0)
{
    mPlayerCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mPlayerIndex));

    ClearHandle(mNewAttacker);
    mAutoLoseVisibilityCheck = false;
    mAutoLoseLockedVisibilityCheck = false;
    mNumNearestTargets = 0;
    mNumAttackerTargets = 0;
    mNewNumNearestTargets = 0;
    mNewNumAttackerTargets = 0;
    mTargetsAllowedFilter = 0;

    mAutoLoseTime = 0.0f;
    mAutoLoseDotProd = -1.0f;
    mAutoLoseDistSquare = 1e10f;
    mAutoLoseLockedTime = 0.0f;
    mAutoLoseLockedDotProd = -1.0f;
    mAutoLoseLockedDistSquare = 1e10f;
    mLockDotProdFilter = -1.0f;
    mLockDotProdFilterLocked = -1.0f;
    mLockDistFilter = 10000.0f;
    mLockDistSquareFilter = 1e8f;
    mSafeZone = RedInterfaceScreen::GetScreenSafeZone();
    mLockVisibleFilter = false;
    mAutoTarget = true;
    mAutoLock = false;

    ClearHandle(mLockedObj);
    Controllable* pControllable = mPlayerCharacter ? mPlayerCharacter->GetControllable() : 0;
    if (pControllable)
    {
        ClearHandle(pControllable->mTargetLockedObj);
    }

    mLockedObjChangedTurn = 0;
    mForceNewLock = 0;
    ClearHandle(mForceNewLockTarget);
    mDisplayIconsOnEverything = false;
    mTimeSinceLastForceLock = 0.0f;
    mTargetVisibilityUpdate = true;

    ++sNumLockOnManagers;
    sLockOnManagers[mPlayerIndex] = this;
}

// FUNCTION: SWBF2 0x0057afa0
LockOnManager::~LockOnManager()
{
    --sNumLockOnManagers;
    sLockOnManagers[mPlayerIndex] = 0;
}

inline void LockOnManager::Target::SetObj(GameObject* const pObject, TargetInfo* const pInfo)
{
    obj = pObject;
    if (pInfo)
    {
        pInfo->mAttacker = false;
        pInfo->mLocked = false;
        pInfo->mHintTarget = false;
        pInfo->mPositionValid = false;
    }
    if (!(GameObject*)obj)
    {
        teamMask = 0;
    }
}

inline void LockOnManager::TargetHistory::Add(GameObject* const pObject)
{
    if (mCount < 64)
    {
        mCount++;
    }
    mHistory[mNextIndexToFill++] = pObject;
    if (mNextIndexToFill >= 64)
    {
        mNextIndexToFill = 0;
    }
}

// FUNCTION: SWBF2 0x0057d5a0
void LockOnManager::SetDisplayIconsOnEverything(bool bEverything)
{
    if (!bEverything)
    {
        for (unsigned int i = 0; i < 64; i++)
        {
            mTargetPlayer[i].SetObj(0, &mTargetPlayerInfo[i]);
            if (mPlayerIndex >= 0)
            {
                HUD::Event event(HUD::GameEvents::gPlayerEvents[mPlayerIndex].targetResetPlayer, i);
                event.Send();
            }
        }
    }
    if (NetGame::IsEnabled())
    {
        mDisplayIconsOnEverything = false;
    }
    else
    {
        mDisplayIconsOnEverything = bEverything;
    }
}

// FUNCTION: SWBF2 0x0057d8f0
void LockOnManager::CreateAll()
{
    sTargetLockSound.SetID(0x6a942216u /* "shell_menu_accept" */, false, false);
    const unsigned int uNumCameras = CameraManager::Instance()->GetNumCameras();
    for (unsigned int i = 0; i < uNumCameras; i++)
    {
        new LockOnManager(i);
    }
    LockOnManager::Target* pTarget = sTargetCommon;
    for (int n = 32; n != 0; --n, ++pTarget)
    {
        pTarget->obj.mObject = 0;
        pTarget->teamMask = 0;
        pTarget->obj.mSavedHandleId = 0;
    }
}

// FUNCTION: SWBF2 0x0057b030
void LockOnManager::Update(float fDeltaTime)
{
    if (fDeltaTime == 0.0f)
    {
        return;
    }

    if (FlagItem::sFlagGameplayType != FlagItem::FLAG_GAMEPLAY_1FLAG &&
        FlagItem::sFlagGameplayType != FlagItem::FLAG_GAMEPLAY_2FLAG)
    {
        if (mNewNumNearestTargets > 1 && mNewNumNearestTargets <= 3)
        {
            mNewNumNearestTargets = 1;
        }
    }

    const unsigned int uTeamMask = 1 << (mPlayerCharacter ? mPlayerCharacter->mTeamNumber : 0);
    RedCamera* pCamera = CameraManager::Instance()->GetCamera(mPlayerIndex);
    const bool bMyFrame = (unsigned int)RedRenderer::GetFrameNumber() % CameraManager::Instance()->GetNumCameras() ==
                     (unsigned int)mPlayerIndex;
    Controllable* pCtrl = mPlayerCharacter ? mPlayerCharacter->GetControllable() : 0;

    if (bMyFrame || mForceNewLock)
    {
        if (!pCtrl)
        {
            mNewNumNearestTargets = 0;
            mNewNumAttackerTargets = 0;
            LockOnManager::ClearPositionValid(mTargetPlayerInfo, 64);
            LockOnManager::ClearPositionValid(mTargetCommonInfo, 32);
        }
        if (mNewNumAttackerTargets != mNumAttackerTargets || mNewNumNearestTargets != mNumNearestTargets)
        {
            for (unsigned int i = 0; i < 64; i++)
            {
                mTargetPlayer[i].SetObj(0, &mTargetPlayerInfo[i]);
                mTargetPlayerInfo[i].Clean();
            }
            mNumAttackerTargets = mNewNumAttackerTargets;
            mNumNearestTargets = mNewNumNearestTargets;
            ClearHandle(mNewAttacker);
            mForceNewLock = 0;
            SetLockedTarget(0, 0);
        }
        if (pCtrl)
        {
            GameObject* pObject = pCtrl->GetGameObject();

            if (mNumNearestTargets)
            {
                SetupNearestTargets(pCtrl, pObject, 0, mNumNearestTargets, fDeltaTime, uTeamMask);
            }
        }
    }

    PblVector3 vEye = pCamera->GetMatrix().GetTranslation();
    bool bOffFrame = !bMyFrame;
    unsigned int uChanged = 0;
    if (mTargetVisibilityUpdate && pCtrl)
    {
        for (unsigned int i = 0; i < 64; i++)
        {
            UpdateTargetVisibility(pCtrl, pCamera, vEye, mTargetPlayer[i], mTargetPlayerInfo[i], false, uTeamMask,
                                   bOffFrame, uChanged);
        }
        for (unsigned int i = 0; i < 32; i++)
        {
            UpdateTargetVisibility(pCtrl, pCamera, vEye, sTargetCommon[i], mTargetCommonInfo[i], true, uTeamMask,
                                   bOffFrame, uChanged);
        }
    }
    if (!bOffFrame && uChanged)
    {
        LockOnManager::ClearVisibilityChecked(mTargetPlayerInfo, 64);
        LockOnManager::ClearVisibilityChecked(mTargetCommonInfo, 32);
    }
}

// FUNCTION: SWBF2 0x0057bd80
void LockOnManager::ClearPositionValid(LockOnManager::TargetInfo* const pInfo, unsigned int uCount)
{
    for (unsigned int i = 0; i < uCount; i++)
    {
        pInfo[i].Clean();
    }
}

// FUNCTION: SWBF2 0x0057d9f0
void LockOnManager::UpdateAll(float fDeltaTime)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        if (sLockOnManagers[i])
        {
            sLockOnManagers[i]->Update(fDeltaTime);
        }
    }
}

extern bool netFrameLock;
namespace GameLoop
{
extern bool mbHistoricalRules;
}

// FUNCTION: SWBF2 0x0057b430
void LockOnManager::CalculateScreenCoordinates(Controllable* const pCtrl, RedCamera* const pCamera, const PblVector3* pPos, float* pScreenX, float* pScreenY, bool* pOffScreen, bool bAimRay)
{
    LockOnManager* pThis = (LockOnManager*)this;
    PblVector3 vCamera;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vCamera), reinterpret_cast<const D3DXVECTOR3*>(pPos), reinterpret_cast<const D3DXMATRIX*>(&pCamera->GetMatrixInverse()));
    const PblVector3 vProjected = pCamera->TransformCameraPointToProjectionSpace(vCamera);

    float fDot;
    if (bAimRay)
    {
        const PblMatrix& mCamera = pCamera->GetMatrix();
        const PblVector3 vForward(-mCamera.m[2][0], -mCamera.m[2][1], -mCamera.m[2][2]);
        const PblVector3 vDelta(pPos->x - mCamera.m[3][0], pPos->y - mCamera.m[3][1], pPos->z - mCamera.m[3][2]);
        PblVector3 vToTarget;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vToTarget), reinterpret_cast<const D3DXVECTOR3*>(&vDelta));
        fDot = vForward.x * vToTarget.x + vForward.y * vToTarget.y + vForward.z * vToTarget.z;
    }
    else
    {
        const PblVector3 vDelta(pPos->x - pCtrl->mEyePoint.x, pPos->y - pCtrl->mEyePoint.y, pPos->z - pCtrl->mEyePoint.z);
        PblVector3 vToTarget;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vToTarget), reinterpret_cast<const D3DXVECTOR3*>(&vDelta));
        fDot = pCtrl->mEyeDir.x * vToTarget.x + pCtrl->mEyeDir.y * vToTarget.y + pCtrl->mEyeDir.z * vToTarget.z;
    }

    if (vCamera.z > 0.0f)
    {
        *pScreenX = -vProjected.x;
        *pScreenY = -vProjected.y;
    }
    else
    {
        *pScreenX = vProjected.x;
        *pScreenY = vProjected.y;
    }
    const float fAbsX = fabsf(*pScreenX);
    float fAbsY = fabsf(*pScreenY);
    if (!pCtrl->GetEntity() || !pCtrl->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        if (fDot < 0.2f && fAbsY > fAbsX * 0.3f)
        {
            float fLimit = fAbsX * 0.3f;
            if (!(fDot <= 0.0f))
            {
                fLimit += (fAbsY - fLimit) * (fDot / 0.2f);
            }
            const float fScale = fLimit / fAbsY;
            fAbsY = fScale * fAbsY;
            *pScreenY *= fScale;
        }
    }

    if (fDot <= 0.0f || fAbsX > pThis->mSafeZone || fAbsY > pThis->mSafeZone)
    {
        const float fScale = !(fAbsX >= fAbsY) ? pThis->mSafeZone / fAbsY : pThis->mSafeZone / fAbsX;
        *pScreenX *= fScale;
        *pScreenY *= fScale;
        *pOffScreen = true;
    }
    else
    {
        *pOffScreen = false;
    }
}

// FUNCTION: SWBF2 0x0057b720
void LockOnManager::UpdateTargetVisibility(Controllable* const pCtrl, RedCamera* const pCamera, PblVector3& vEye,
                                           Target& rTarget, TargetInfo& rInfo, bool bCommon,
                                           unsigned int uTeamMask, bool& bArg8, unsigned int& uArg9)
{
    GameObject* pObject = rTarget.obj;
    if (!pObject)
    {
        return;
    }
    if (!(rTarget.teamMask & uTeamMask))
    {
        return;
    }

    PblVector3 vPos = pObject->GetCollisionCenter();
    if (pObject->GetObjectData(0) == COLL_ATTACHED)
    {
        vPos += pObject->GetMatrix()[1] * 0.4f;
    }
    if (const RedPose* pPose = pObject->GetPose())
    {
        if (const PblMatrix* pBone = pPose->Find(0x3483A489u))
        {
            PblVector3 vBonePos;
            D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vBonePos), reinterpret_cast<const D3DXVECTOR3*>((const PblVector3*)pBone->m[3]), reinterpret_cast<const D3DXMATRIX*>(&pObject->GetMatrix()));
            vPos = vBonePos;
        }
    }
    else if (const GameModel* pModel = pObject->GetModel())
    {
        PblMatrix mBone;
        if (pModel->GetRedModel(0) && pModel->GetRedModel(0)->GetBoneMatrix(0x3483A489u, mBone))
        {
            PblVector3 vBonePos;
            D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vBonePos), reinterpret_cast<const D3DXVECTOR3*>((const PblVector3*)mBone.m[3]), reinterpret_cast<const D3DXMATRIX*>(&pObject->GetMatrix()));
            vPos = vBonePos;
        }
    }

    PblVector3 vLift;
    if (bCommon)
    {
        PblVector3 vUp(0.0f, 1.3f, 0.0f);
        PblVector3 vOut;
        D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vOut), reinterpret_cast<const D3DXVECTOR3*>(&vUp), reinterpret_cast<const D3DXMATRIX*>(&pObject->GetMatrix()));
        vLift = vOut;
    }
    else if (rInfo.mLocked)
    {
        vLift = PblVector3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        PblVector3 vUp;
        if (pObject == pCtrl->GetGameObject())
        {
            vUp = PblVector3(0.0f, 0.3f, pObject->GetCollisionRadius() * -0.2f);
        }
        else
        {
            float fUp = pObject->GetCollisionRadius() * 0.7f;
            vUp = PblVector3(0.0f, 4.0f > fUp ? fUp : 4.0f, 0.0f);
        }
        PblVector3 vOut;
        D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vOut), reinterpret_cast<const D3DXVECTOR3*>(&vUp), reinterpret_cast<const D3DXMATRIX*>(&pObject->GetMatrix()));
        vLift = vOut;
    }

    float fDist = PblMath::Sqrt((vPos.y - vEye.y) * (vPos.y - vEye.y) + (vPos.x - vEye.x) * (vPos.x - vEye.x) +
                        (vPos.z - vEye.z) * (vPos.z - vEye.z));
    if (fDist > rTarget.maxDistTrack)
    {
        return;
    }

    GameObject* pVehicle;
    if (pObject->GetObjectData(0) == COLL_ATTACHED)
    {
        pVehicle = Controllable::GetVehicleIfAttached(pObject);
        if (pVehicle == pObject)
        {
            pVehicle = 0;
        }
    }
    else
    {
        pVehicle = pObject;
    }
    PblVector3 vLag;
    if (!pVehicle || !pVehicle->IsInScene())
    {
        vLag = PblVector3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        PblMatrix mSmoothed = pVehicle->GetSmoothedMatrix();
        if (!netFrameLock && NetGame::IsEnabled())
        {
            gameObjectInterpolator.GetMatrix(pVehicle->mNetUniqueId, mSmoothed);
        }
        const PblMatrix& mVehicle = pVehicle->GetMatrix();
        vLag = PblVector3(mSmoothed.m[3][0] - mVehicle.m[3][0], mSmoothed.m[3][1] - mVehicle.m[3][1],
                          mSmoothed.m[3][2] - mVehicle.m[3][2]);
    }

    const bool bAimRay = !rInfo.mLocked || (pCtrl->GetEntity() && pCtrl->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer));

    PblVector3 vPoint(vLag.x + vPos.x + vLift.x, vLag.y + vPos.y + vLift.y, vLag.z + vPos.z + vLift.z);
    float fScreenX;
    float fScreenY;
    bool bOffScreen;
    CalculateScreenCoordinates(pCtrl, pCamera, &vPoint, &fScreenX, &fScreenY, &bOffScreen, bAimRay);
    ++uArg9;
    if (!bArg8 && !rInfo.mVisibilityChecked)
    {
        rInfo.mVisible = AIUtil::CanSeePosition(vEye, pCtrl, vPos, pObject);
        rInfo.mVisibilityChecked = true;
        rInfo.mPositionValid = true;
        bArg8 = true;
    }
    rInfo.mPosition = PblVector3(fScreenX, fScreenY, fDist);
    rInfo.mOnScreen = !bOffScreen;

    bool bFlagCarrier;
    if (bCommon)
    {
        TargetManager::Marker* pMarker = 0;
        for (unsigned int i = 0; i < TargetManager::MAX_MARKERS; ++i)
        {
            if (pObject == (GameObject*)TargetManager::gMarkers[i].actor)
            {
                pMarker = &TargetManager::gMarkers[i];
                break;
            }
        }
        bFlagCarrier = pMarker && ((GameObject*)pMarker->actor)->IsRtti(FlagItem::rttiHashFlagItem) && !GameLoop::mbHistoricalRules;
    }
    else
    {
        bFlagCarrier = pObject->GetCarriedFlagItem() != 0;
    }
    rInfo.mFlagCarrier = bFlagCarrier;

    rInfo.mFriendly = pCtrl->GetGameObject() && pCtrl->GetGameObject()->GetAffiliationToMe(pObject) >= 0;
}

// FUNCTION: SWBF2 0x0057bda0
void LockOnManager::SetLockedTarget(GameObject* const pObject, TargetInfo* const pInfo)
{
    mLockedObj = pObject;
    Controllable* pCtrl = mPlayerCharacter ? mPlayerCharacter->GetControllable() : 0;
    if (pCtrl)
    {
        pCtrl->mTargetLockedObj = pObject;
    }
    mLockedObjChangedTurn = NetGame::GetClientTurnNumber();
    if (!pInfo)
    {
        return;
    }
    if (pObject)
    {
        sTargetLockSound.Play(0, 0, false, true);
        for (unsigned int i = 0; i < 64; ++i)
        {
            mTargetPlayerInfo[i].mLocked = (pInfo == &mTargetPlayerInfo[i]);
        }
    }
    else
    {
        pInfo->mVisibilityChecked = false;
        pInfo->mLocked = false;
    }
}

// FUNCTION: SWBF2 0x0057b320
void LockOnManager::RemoveTargetLock()
{
    if (!(GameObject*)mLockedObj)
    {
        return;
    }

    TargetInfo* pInfo = 0;
    for (unsigned int i = 0; i < 64; i++)
    {
        Target& target = mTargetPlayer[i];
        if (target.obj == (GameObject*)mLockedObj)
        {
            pInfo = &mTargetPlayerInfo[i];
            target.SetObj(0, 0);
            pInfo->Clean();
            break;
        }
    }
    SetLockedTarget(0, pInfo);
    mForceNewLock = 0;
    mForceNewLockTarget = 0;
}

// FUNCTION: SWBF2 0x0057db80
LockOnManager::Target* LockOnManager::FindTargetByGameObject(Target* const pTargets, const unsigned int uNumTargets,
                                                             const GameObject* const pObject)
{
    for (unsigned int i = 0; i < uNumTargets; ++i)
    {
        Target* const target = &pTargets[i];
        if (pObject == (GameObject*)target->obj)
        {
            return target;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057b300
bool LockOnManager::IsGameObjectTargeted(const GameObject* const pObject)
{
    return FindTargetByGameObject(mTargetPlayer, 64, pObject) != 0;
}

// FUNCTION: SWBF2 0x0057dbd0
bool LockOnManager::IsGameObjectTargetedAll(const GameObject* const pObject)
{
    if (pObject)
    {
        if (FindTargetByGameObject(sTargetCommon, 32, pObject))
        {
            return true;
        }
        for (unsigned int i = 0; i < 4; ++i)
        {
            if (sLockOnManagers[i] && sLockOnManagers[i]->IsGameObjectTargeted(pObject))
            {
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0057d680
bool LockOnManager::GetDisplayIconsEverything() const
{
    return !NetGame::IsEnabled() && mDisplayIconsOnEverything;
}

// FUNCTION: SWBF2 0x0057d6b0
bool LockOnManager::IsTargetObjectTypeAllowed(GameObject* gameObject, GameObject* me, bool noLockon,
                                              unsigned int targetsAllowedFilter, unsigned int newNumNearestTargets,
                                              bool displayIconsOnEverything)
{
    if (!gameObject || !gameObject->mIsTargetable || !(gameObject->mCurHealth > 0.0f) || !gameObject->IsAlive())
    {
        return false;
    }

    if (noLockon)
    {
        if ((FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_1FLAG ||
             FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_2FLAG) &&
            gameObject->GetCarriedFlagItem())
        {
            if (gameObject != me)
            {
                return true;
            }
            if (me->GetControllable() && me->GetControllable()->mCharacter &&
                me->GetControllable()->CheckFirstPerson(me->GetControllable()->mCharacter->mPlayerId))
            {
                return false;
            }
            return true;
        }
        if (newNumNearestTargets <= 3)
        {
            return false;
        }
    }

    if (gameObject == me)
    {
        return false;
    }

    if (gameObject->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier)
    {
        if (static_cast<EntitySoldier*>(gameObject)->GetInvisibilityFactor() > 0.0f &&
            me->GetAffiliationToMe(gameObject) < 0)
        {
            return false;
        }
    }

    int healthType = gameObject->mHealthTypeForLockOn;
    if (healthType == 8)
    {
        healthType = gameObject->mHealthType;
    }

    bool typeAllowed = ((1 << healthType) & targetsAllowedFilter) != 0;

    if (noLockon && displayIconsOnEverything && !NetGame::IsEnabled() && me->GetAffiliationToMe(gameObject) < 0)
    {
        if (gameObject->IsRtti(EntitySoldier::rttiHashEntitySoldier) ||
            gameObject->IsRtti(EntityDroideka::rttiHashEntityDroideka) ||
            gameObject->IsRtti(EntityWalker::rttiHashEntityWalker) ||
            gameObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) ||
            gameObject->IsRtti(EntityHover::rttiHashEntityHover) ||
            gameObject->GetEntity()->IsRtti(MountedTurret::rttiHashMountedTurret))
        {
            return true;
        }
    }

    if (typeAllowed)
    {
        int affiliationToMe = me->GetAffiliationToMe(gameObject);
        if (affiliationToMe == 0)
        {
            return false;
        }
        if (affiliationToMe > 0)
        {
            if (!noLockon)
            {
                return false;
            }
            bool isFlyer = gameObject->IsRtti(EntityFlyer::rttiHashEntityFlyer);
            if (!isFlyer)
            {
                return false;
            }
        }
    }
    return typeAllowed;
}


// FUNCTION: SWBF2 0x0057be70
void LockOnManager::SetupNearestTargets(Controllable* const playerControllable, GameObject* const playerObject,
                                        unsigned int arrayStartIndex, unsigned int arrayEndIndex, float deltaTime,
                                        unsigned int teamMask)
{
    mForceNewLock--;
    if (mForceNewLock == 0)
    {
        mForceNewLockTarget = NULL;
    }
    else if (mForceNewLock < 0)
    {
        mForceNewLock = 0;
    }

    if (mForceNewLockTarget)
    {
        GameObject* vehicle = Controllable::GetVehicleIfAttached(mForceNewLockTarget);
        if (vehicle && vehicle != mForceNewLockTarget)
        {
            bool vehicleAllowed = IsTargetObjectTypeAllowed(vehicle, playerObject, false, mTargetsAllowedFilter,
                                                            mNewNumNearestTargets, GetDisplayIconsEverything());
            if (vehicleAllowed)
            {
                mForceNewLockTarget = vehicle;
                mForceNewLock = 5;
            }
            else if (mForceNewLockTarget->GetCollisionType() == COLL_ATTACHED)
            {
                mForceNewLock = IsTargetObjectTypeAllowed(mForceNewLockTarget, playerObject, false,
                                                          mTargetsAllowedFilter, mNewNumNearestTargets,
                                                          GetDisplayIconsEverything())
                                    ? 5
                                    : 0;
            }
            else
            {
                mForceNewLock = 0;
            }
        }
        else
        {
            mForceNewLock = IsTargetObjectTypeAllowed(mForceNewLockTarget, playerObject, false, mTargetsAllowedFilter,
                                                      mNewNumNearestTargets, GetDisplayIconsEverything())
                                ? 5
                                : 0;
        }
        if (!mForceNewLock)
        {
            mForceNewLockTarget = NULL;
        }
    }

    if (!playerObject)
    {
        return;
    }

    for (unsigned int i = arrayStartIndex; i < arrayEndIndex; ++i)
    {
        GameObject* targetObj = mTargetPlayer[i].obj;
        if (targetObj)
        {
            if (mLockedObj && targetObj != mLockedObj && mTargetPlayerInfo[i].mLocked)
            {
                mTargetPlayerInfo[i].SetLocked(false);
            }

            for (unsigned int j = i + 1; j < arrayEndIndex; ++j)
            {
                if (mTargetPlayer[j].obj == targetObj)
                {
                    mTargetPlayer[j].SetObj(NULL, NULL);
                    mTargetPlayerInfo[j].Clean();
                }
            }

            GameObject* vehicleTarget = Controllable::GetVehicleIfAttached(targetObj);
            if (vehicleTarget && vehicleTarget != targetObj)
            {
                bool vehicleAllowed =
                    IsTargetObjectTypeAllowed(vehicleTarget, playerObject, targetObj != mLockedObj,
                                              mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything());
                if (vehicleAllowed)
                {
                    if (targetObj == mLockedObj)
                    {
                        SetLockedTarget(vehicleTarget, &mTargetPlayerInfo[i]);
                    }
                    mTargetPlayer[i].SetObj(vehicleTarget, NULL);
                    mTargetPlayerInfo[i].ClearPositionValid();
                    mTargetPlayerInfo[i].SetFriendly(playerControllable && playerControllable->GetGameObject() &&
                                                     playerControllable->GetGameObject()->GetAffiliationToMe(
                                                         vehicleTarget) >= 0);
                    mTargetPlayerInfo[i].SetFlagCarrier(vehicleTarget->GetCarriedFlagItem() != NULL);
                }
                else if (targetObj->GetCollisionType() != COLL_ATTACHED ||
                         !IsTargetObjectTypeAllowed(targetObj, playerObject, targetObj != mLockedObj,
                                                    mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything()))
                {
                    if (targetObj == mLockedObj)
                    {
                        SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                    }
                    mTargetPlayer[i].SetObj(NULL, NULL);
                    mTargetPlayerInfo[i].Clean();
                }
            }
            else if (!IsTargetObjectTypeAllowed(targetObj, playerObject, targetObj != mLockedObj,
                                                mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything()))
            {
                if (targetObj == mLockedObj)
                {
                    SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                }
                mTargetPlayer[i].SetObj(NULL, NULL);
                mTargetPlayerInfo[i].Clean();
            }
        }
        else
        {
            if (mTargetPlayerInfo[i].mLocked)
            {
                mTargetPlayerInfo[i].SetLocked(false);
            }
            if (mTargetPlayerInfo[i].mPositionValid)
            {
                mTargetPlayerInfo[i].ClearPositionValid();
            }
            if (mTargetPlayerInfo[i].mHintTarget)
            {
                mTargetPlayerInfo[i].SetHint(false);
            }
            if (mTargetPlayerInfo[i].GetFlagCarrier())
            {
                mTargetPlayerInfo[i].SetFlagCarrier(false);
            }
            if (mTargetPlayerInfo[i].mAttacker)
            {
                mTargetPlayerInfo[i].SetAttacker(false);
            }
            if (mTargetPlayerInfo[i].mOnScreen)
            {
                mTargetPlayerInfo[i].SetOnScreen(false);
            }
        }
    }

    if (mLockedObj && !mForceNewLock)
    {
        mTimeSinceLastForceLock += deltaTime;
    }
    else
    {
        mTimeSinceLastForceLock = 0.0f;
    }

    if (mForceNewLock && mForceNewLockTarget)
    {
        mTargetHistory.Clear();
    }
    else
    {
        if (!mLockedObj || mTimeSinceLastForceLock > 0.5f)
        {
            mTargetHistory.Clear();
        }
        if (mLockedObj)
        {
            mTargetHistory.Add(mLockedObj);
        }
    }

    const PblVector3& playerPosition = playerObject->GetMatrix().GetTranslation();
    float maxDot = 0.0f;
    unsigned int maxDotIndex = 64;
    unsigned int emptySlot = 64;
    for (unsigned int i = arrayStartIndex; i < arrayEndIndex; ++i)
    {
        Target& target = mTargetPlayer[i];
        bool targetIsVisible = mTargetPlayerInfo[i].GetVisible();
        if (IsTargetObjectTypeAllowed(target.obj, playerObject, target.obj != mLockedObj,
                                      mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything()))
        {
            bool targetIsLocked = target.obj == mLockedObj;
            float autoLoseTime;
            float autoLoseDotProd;
            float autoLoseDistSquare;
            bool autoLoseVisibilityCheck;
            if (targetIsLocked)
            {
                autoLoseTime = mAutoLoseLockedTime;
                autoLoseDotProd = mAutoLoseLockedDotProd;
                autoLoseDistSquare = mAutoLoseLockedDistSquare;
                autoLoseVisibilityCheck = mAutoLoseLockedVisibilityCheck;
            }
            else
            {
                autoLoseTime = mAutoLoseTime;
                autoLoseDotProd = mAutoLoseDotProd;
                autoLoseDistSquare = mAutoLoseDistSquare;
                autoLoseVisibilityCheck = mAutoLoseVisibilityCheck;
            }

            if (0.0f > autoLoseTime)
            {
                target.timer = 0.0f;
            }
            else
            {
                PblVector3 targetPos = target.obj->GetMatrix().GetTranslation();
                PblVector3 toTargetVector(targetPos.x - playerControllable->mEyePoint.x,
                                          targetPos.y - playerControllable->mEyePoint.y,
                                          targetPos.z - playerControllable->mEyePoint.z);
                float dot;
                if (target.obj == playerObject)
                {
                    dot = 1.0f;
                }
                else
                {
                    PblVector3 direction;
                    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&direction), reinterpret_cast<const D3DXVECTOR3*>(&toTargetVector));
                    dot = playerControllable->mEyeDir.x * direction.x + direction.y * playerControllable->mEyeDir.y +
                          direction.z * playerControllable->mEyeDir.z;
                }

                if (dot >= autoLoseDotProd &&
                    autoLoseDistSquare >= toTargetVector.x * toTargetVector.x + toTargetVector.y * toTargetVector.y +
                                              toTargetVector.z * toTargetVector.z &&
                    (!autoLoseVisibilityCheck || targetIsVisible))
                {
                    target.timer = 0.0f;
                }
                else
                {
                    target.timer += deltaTime;
                }

                if (target.timer >= autoLoseTime)
                {
                    if (targetIsLocked)
                    {
                        SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                    }
                    target.SetObj(NULL, NULL);
                    mTargetPlayerInfo[i].Clean();
                }
                else if (dot > 0.98f && mForceNewLock && !mForceNewLockTarget &&
                         IsTargetObjectTypeAllowed(target.obj, playerObject, false, mTargetsAllowedFilter,
                                                   mNewNumNearestTargets, GetDisplayIconsEverything()) &&
                         (mTargetHistory.IsEmpty() || !mTargetHistory.Check(target.obj)))
                {
                    if (maxDotIndex == 64 || dot > maxDot)
                    {
                        maxDot = dot;
                        maxDotIndex = i;
                    }
                }
            }
        }
        else if (target.obj)
        {
            if (target.obj == mLockedObj)
            {
                SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
            }
            target.SetObj(NULL, NULL);
            mTargetPlayerInfo[i].Clean();
        }

        if (emptySlot == 64 && !target.obj)
        {
            emptySlot = i;
        }
    }

    if (maxDotIndex < 64)
    {
        SetLockedTarget(mTargetPlayer[maxDotIndex].obj, &mTargetPlayerInfo[maxDotIndex]);
        mForceNewLock = 0;
        mTargetHistory.Add(mLockedObj);
    }

    unsigned int startIdx = 0;
    unsigned int endIdx = 0;
    if (mForceNewLock)
    {
        startIdx = emptySlot < 64 ? emptySlot : arrayStartIndex;
        endIdx = startIdx + 1;
    }
    else if (emptySlot < 64 && mAutoTarget)
    {
        startIdx = arrayStartIndex;
        endIdx = arrayEndIndex;
    }

    if (startIdx != endIdx)
    {
        GameObject* objectArray[384];
        unsigned int objectCount;
        if (mForceNewLock && mForceNewLockTarget)
        {
            objectArray[0] = mForceNewLockTarget;
            objectCount = 1;
        }
        else
        {
            objectCount = CollisionManager::GetObjectsInRange(playerObject->GetMatrix().GetTranslation(),
                                                              mLockDistFilter, objectArray, 384, NULL,
                                                              COLL_SOFT | COLL_RIGID | COLL_STATIC | COLL_ATTACHED);
        }

        if (objectCount)
        {
            int objectLeft = objectCount;
            for (unsigned int i = startIdx; i < endIdx && objectLeft > 0; ++i)
            {
                Target& target = mTargetPlayer[i];
                if (target.obj && !mForceNewLock)
                {
                    continue;
                }

                float maxDistSquare = FLT_MAX;
                int newGameObjectIdx = -1;
                objectLeft = 0;
                for (unsigned int objI = 0; objI < objectCount; ++objI)
                {
                    GameObject*& object = objectArray[objI];
                    if (object)
                    {
                        GameObject* targetGameObjectVehicle = Controllable::GetVehicleIfAttached(object);
                        if (targetGameObjectVehicle && targetGameObjectVehicle != object)
                        {
                            if (IsTargetObjectTypeAllowed(targetGameObjectVehicle, playerObject, !mForceNewLock && !mAutoLock,
                                                          mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything()))
                            {
                                object = targetGameObjectVehicle;
                            }
                            else if (object->GetCollisionType() != COLL_ATTACHED ||
                                     !IsTargetObjectTypeAllowed(object, playerObject,
                                                                !mForceNewLock && !mAutoLock, mTargetsAllowedFilter,
                                                                mNewNumNearestTargets, GetDisplayIconsEverything()))
                            {
                                object = NULL;
                            }
                        }
                        else if (!IsTargetObjectTypeAllowed(object, playerObject, !mForceNewLock && !mAutoLock,
                                                            mTargetsAllowedFilter, mNewNumNearestTargets,
                                                            GetDisplayIconsEverything()))
                        {
                            object = NULL;
                        }

                        if (object && (!mForceNewLock || !mTargetHistory.Check(object)))
                        {
                            if (!mForceNewLock)
                            {
                                for (unsigned int k = arrayStartIndex; k < arrayEndIndex; ++k)
                                {
                                    if (object == mTargetPlayer[k].obj)
                                    {
                                        object = NULL;
                                        break;
                                    }
                                }
                            }

                            if (object)
                            {
                                PblVector3 targetPos = object->GetCollisionCenter();
                                if (object->GetCollisionType() == COLL_ATTACHED)
                                {
                                    const PblMatrix& objMatrix = object->GetMatrix();
                                    targetPos.x += objMatrix.m[1][0] * 0.4f;
                                    targetPos.y += objMatrix.m[1][1] * 0.4f;
                                    targetPos.z += objMatrix.m[1][2] * 0.4f;
                                }

                                float distSquare = (targetPos.x - playerPosition.x) * (targetPos.x - playerPosition.x) +
                                                   (targetPos.y - playerPosition.y) * (targetPos.y - playerPosition.y) +
                                                   (targetPos.z - playerPosition.z) * (targetPos.z - playerPosition.z);
                                float dot;
                                if (object == playerObject)
                                {
                                    dot = 1.0f;
                                }
                                else
                                {
                                    PblVector3 toTarget(targetPos.x - playerControllable->mEyePoint.x,
                                                        targetPos.y - playerControllable->mEyePoint.y,
                                                        targetPos.z - playerControllable->mEyePoint.z);
                                    PblVector3 direction;
                                    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&direction), reinterpret_cast<const D3DXVECTOR3*>(&toTarget));
                                    dot = direction.x * playerControllable->mEyeDir.x +
                                          direction.y * playerControllable->mEyeDir.y +
                                          direction.z * playerControllable->mEyeDir.z;
                                }

                                if (mLockDistSquareFilter >= distSquare &&
                                    ((dot >= mLockDotProdFilter && mAutoLoseDistSquare > distSquare &&
                                      !mForceNewLock && !mAutoLock) ||
                                     (dot >= mLockDotProdFilterLocked && mAutoLoseLockedDistSquare > distSquare &&
                                      (mForceNewLock || mAutoLock))))
                                {
                                    float dotDiv = dot > 0.1f ? dot : 0.1f;
                                    if (dotDiv > 0.998f)
                                    {
                                        distSquare *= 1e-8f;
                                    }
                                    if (dotDiv > 0.98f)
                                    {
                                        distSquare *= 1e-6f;
                                    }
                                    dotDiv *= dotDiv;
                                    distSquare /= dotDiv;
                                    if (maxDistSquare > distSquare)
                                    {
                                        if (!mLockVisibleFilter ||
                                            AIUtil::CanSeePosition(playerControllable->mEyePoint, playerControllable,
                                                                   targetPos, object))
                                        {
                                            maxDistSquare = distSquare;
                                            newGameObjectIdx = objI;
                                        }
                                        else
                                        {
                                            object = NULL;
                                        }
                                    }
                                }
                                else
                                {
                                    object = NULL;
                                }
                            }
                        }
                    }
                    if (object)
                    {
                        objectLeft++;
                    }
                }

                if (newGameObjectIdx >= 0 && objectArray[newGameObjectIdx])
                {
                    if (mLockedObj == target.obj)
                    {
                        SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                    }
                    if (mForceNewLock)
                    {
                        for (unsigned int k = arrayStartIndex; k < arrayEndIndex; ++k)
                        {
                            if (objectArray[newGameObjectIdx] == mTargetPlayer[k].obj)
                            {
                                mTargetPlayer[k].SetObj(NULL, NULL);
                                mTargetPlayerInfo[k].Clean();
                            }
                        }
                    }

                    bool isFriendly = playerControllable->GetGameObject() &&
                                      playerControllable->GetGameObject()->GetAffiliationToMe(
                                          objectArray[newGameObjectIdx]) >= 0;
                    target.InitPlayer(objectArray[newGameObjectIdx], teamMask, mTargetPlayerInfo[i], isFriendly);
                    if (mPlayerIndex >= 0)
                    {

                        HUD::Event resetTargetPlayer(HUD::GameEvents::gPlayerEvents[mPlayerIndex].targetResetPlayer, i);
                        resetTargetPlayer.Send();
                    }

                    if (mForceNewLock || mAutoLock)
                    {
                        if (!mTargetHistory.Check(objectArray[newGameObjectIdx]))
                        {
                            mTargetHistory.Add(objectArray[newGameObjectIdx]);
                        }
                        SetLockedTarget(objectArray[newGameObjectIdx], &mTargetPlayerInfo[i]);
                        mForceNewLock = 0;
                        mForceNewLockTarget = NULL;
                    }

                    objectArray[newGameObjectIdx] = NULL;
                    objectLeft--;
                }
            }
        }
    }

    if (mForceNewLock)
    {
        if (mForceNewLockTarget)
        {
            mForceNewLockTarget = NULL;
            mForceNewLock = 0;
        }
        else if (mTargetHistory.IsEmpty())
        {
            mForceNewLock = 0;
        }
        else
        {
            mTargetHistory.Clear();
        }
    }
}

// FUNCTION: SWBF2 0x0057ab70
bool LockOnManager::TargetHistory::Check(GameObject* const pObject) const
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        if (pObject == (GameObject*)mHistory[i])
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0057abf0
void LockOnManager::Target::InitPlayer(GameObject* const initObj, unsigned int initTeamMask,
                                       TargetInfo& targetInfo, bool isFriendly)
{
    GameObjectClass* pClass = initObj ? static_cast<GameObjectClass*>(initObj->mEntityClass) : 0;
    obj = initObj;
    teamMask = initTeamMask;
    timer = 0.0f;
    if (pClass)
    {
        maxDistTrack = pClass->mLockOnMaxTrackDist;
        minDist = pClass->mLockOnMinDist;
        maxDist = pClass->mLockOnMaxDist;
    }
    else
    {
        maxDistTrack = 200000.0f;
        minDist = 100.0f;
        maxDist = 500.0f;
    }
    targetInfo.mVisibilityChecked = false;
    targetInfo.mVisible = false;
    targetInfo.mPositionValid = true;
    targetInfo.mFriendly = isFriendly;
    targetInfo.mFlagCarrier = initObj && initObj->GetCarriedFlagItem();
}

// FUNCTION: SWBF2 0x0057bd60
void LockOnManager::ClearVisibilityChecked(LockOnManager::TargetInfo* const pInfo, unsigned int uCount)
{
    for (unsigned int i = 0; i < uCount; ++i)
    {
        pInfo[i].mVisibilityChecked = false;
    }
}

// FUNCTION: SWBF2 0x0057d9a0
void LockOnManager::DestroyAll()
{
    sTargetLockSound.SetID(0u, false, false);

    for (unsigned int i = 0; i < 4; ++i)
    {
        if (sLockOnManagers[i])
        {
            delete sLockOnManagers[i];
            sLockOnManagers[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x0057db10
void LockOnManager::RemoveTargetCommon(GameObject* const pActor, unsigned int uTeamMask)
{
    unsigned int teamMaskLeft = uTeamMask;
    for (unsigned int i = 0; i < 32; ++i)
    {
        LockOnManager::Target& target = LockOnManager::sTargetCommon[i];
        if (!(target.obj == pActor))
        {
            continue;
        }
        unsigned int turnOff = target.teamMask & teamMaskLeft;
        if (!turnOff)
        {
            continue;
        }
        target.teamMask &= ~turnOff;
        teamMaskLeft &= ~turnOff;
        if (!target.teamMask)
        {
            target.SetObj(0, 0);
        }
        if (!teamMaskLeft)
        {
            break;
        }
    }
}

// SYNTHETIC: SWBF2 0x004041a0
// `dynamic initializer for 'LockOnManager::sTargetCommon''
// SYNTHETIC: SWBF2 0x00768c90
// `dynamic atexit destructor for 'LockOnManager::sTargetCommon''

// GLOBAL: SWBF2 0x01ec7d50
// LockOnManager::sTargetCommon
LockOnManager::Target LockOnManager::sTargetCommon[32];
