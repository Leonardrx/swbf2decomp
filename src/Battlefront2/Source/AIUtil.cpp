#include <d3dx9math.h>
#include "AIUtil.h"

#include <math.h>

#include "AILowLevel.h"
#include "Aimer.h"
#include "Controllable.h"
#include "Entity.h"
#include "Controller.h"
#include "UnitController.h"
#include "CollisionManager.h"
#include "EventInfo.h"
#include "Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"
#include "AIGoal.h"
#include "AIGoalManager.h"
#include "AIVisRegion.h"
#include "FlyRegion.h"
#include "HintManager.h"
#include "Navigator.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "PathManager.h"
#include "UnitAgent.h"
#include "CombatUtil.h"
#include "ConnectivityGraphFollower.h"
#include "VehicleManager.h"
#include "CameraManager.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "EntityBuilding.h"
#include "EntityBuildingArmed.h"
#include "EntityDroid.h"
#include "EntityDroideka.h"
#include "EntityFlyer.h"
#include "EntityHover.h"
#include "EntityItem.h"
#include "EntityRemoteTerminal.h"
#include "EntitySoldier.h"
#include "EntityWalker.h"
#include "GameLoop.h"
#include "GameModel.h"
#include "GameObject.h"
#include "MountedTurret.h"
#include "NetGame.h"
#include "PassengerSlot.h"
#include "SnipeHelper.h"
#include "Team.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "PlayerController.h"

#include "EntityCarrier.h"

#include "ListPool.h"
#include "TreeGrid.h"
#include "PebbleFL/Common/PblHeap.h"
#include "TeamManager.h"


extern PblVector3 gVectorZero;

namespace AIDifficulty
{
    float GetAimerMaxAngAcc(const UnitController* pUnit);
    float GetAimerDamping(const UnitController* pUnit);
    float GetAimerSpread(const UnitController* pUnit);
    float GetAimerYawSpread(const UnitController* pUnit);

    float GetGrenadeReactionTime();
}

namespace AI
{
enum VOType
{
    VOT_Grenade = 10,
    VOT_SC_GetOutResponse = 32
};
namespace VOHelper
{
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const PblVector3& vPos, float fDelay);
}
}

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace
{

    inline const PblVector3& GetVelocity(const GameObject* pObj)
    {
        return pObj->GetVelocity();
    }

    inline bool IsRtti(const GameObject* pObj, unsigned int uRtti)
    {
        return pObj->IsRtti(uRtti);
    }

    inline float GetMaxSpeed(const GameObject* pObj)
    {
        return pObj->GetMaxSpeed();
    }

    inline const Controllable* GetControllable(const GameObject* pObj)
    {
        return pObj->GetControllable();
    }

    int GetEntityType(const GameObject* pObj)
    {
        return pObj->GetEntityType();
    }

    inline int GetFlyerState(const GameObject* pObj)
    {
        return static_cast<const EntityFlyer*>(pObj)->GetState();
    }

    PblVector3 GetTargetPoint(const GameObject* pObj, const PblVector3& vFrom,
                              const PblVector3& vDir, int iBodyId)
    {
        return pObj->GetTargetPoint(vFrom, vDir, iBodyId);
    }
}

namespace ControllerManager
{
    extern const int UPDATES_PER_SECOND;
}

// GLOBAL: SWBF2 0x01e2fdb2
bool AIUtil::sUberMode;

// FUNCTION: SWBF2 0x004433c0
bool AIUtil::IsUberMode()
{
    return sUberMode;
}

// FUNCTION: SWBF2 0x0043ef50
float AIUtil::GetRandomFloat(float fMin, float fMax)
{
    if (fMax == fMin)
    {
        return fMin;
    }
    return PblRandom::GetSim().GetFloat() * (fMax - fMin) + fMin;
}

// FUNCTION: SWBF2 0x0043efc0
int AIUtil::GetRandomInt(int iMin, int iMax)
{
    if (iMax > iMin)
    {
        return (int)(PblRandom::GetSim().GetFloat() * (float)(iMax - iMin + 1) + (float)iMin);
    }
    return iMin;
}

namespace VisionManager
{
    class RayRequest
    {
    public:
        static MemoryPool sMemoryPool;
    };
}

namespace VisionManager
{
    void Reset();
}
namespace AudibleManager
{
    void Reset();
}
namespace HintManager
{
    void Reset();
}
namespace ReserveManager
{
    void Reset();
}

// FUNCTION: SWBF2 0x0043f280
void AIUtil::Reset()
{
    AI::AIGoalManager::Destroy();
    VisionManager::Reset();
    AudibleManager::Reset();
    PathManager::Reset();
    PathFinder::Reset();
    HintManager::Reset();
    ReserveManager::Reset();
    FlyRegionManager::Reset();
    AIVisRegionManager::Reset();
    UnitAgent::sMemoryPool.Reset();
    Navigator::sMemoryPool.Reset();
    UnitController::sMemoryPool.Reset();
    Navigator::sMemoryPool.Reset();
    PathFollower::sMemoryPool.Reset();
    BaseHint::sMemoryPool.Reset();
    AI::AIGoal::sMemoryPool.Reset();
    VisionManager::RayRequest::sMemoryPool.Reset();
}

// GLOBAL: SWBF2 0x01e2fdcc
float AIUtil::gMinFlyHeight;
// GLOBAL: SWBF2 0x01e2fdc4
float AIUtil::gMaxFlyHeight;
// GLOBAL: SWBF2 0x01e2fdc8
float AIUtil::gMinPlayerFlyHeight;
// GLOBAL: SWBF2 0x01e2fdd0
float AIUtil::gMaxPlayerFlyHeight;
// GLOBAL: SWBF2 0x01e2fdba
bool AIUtil::gGroundFlyerMap;
// GLOBAL: SWBF2 0x007e5eec
bool AIUtil::gAIAllowBlindJetJumps = true;
// GLOBAL: SWBF2 0x01e2fdb4
float AIUtil::gStationaryWaitTime;
// GLOBAL: SWBF2 0x01e2fdb8
bool AIUtil::gStayInTurrets;
// GLOBAL: SWBF2 0x01e2fdb9
bool AIUtil::gSkipPlayer;
// GLOBAL: SWBF2 0x007e5ef4
float AIUtil::gSnipeSuitabilityDist = 70.0f;

// GLOBAL: SWBF2 0x01f9af80
static unsigned __int64 sSpied[600];

// FUNCTION: SWBF2 0x004417d0
void AIUtil::ResetSpied(Character* pWatcher)
{
    sSpied[Character::GetIndex(pWatcher)] = 0;
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00441780
void __fastcall AddSpied(Character* aSpy, Character* aSpotter)
{
    sSpied[Character::GetIndex(aSpy)] |= 1 << Character::GetIndex(aSpotter);
}
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00441800
bool __fastcall IsSpied(Character* aSpy, Character* aSpotter)
{
    return (sSpied[Character::GetIndex(aSpy)] & (1 << Character::GetIndex(aSpotter))) != 0;
}
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00442b30
bool __fastcall ShouldStayInVehicle(Controllable* pMe)
{
    static_cast<const Controllable*>(pMe)->GetGameObject();
    if (!AIUtil::IsEndGame(0))
    {
        return true;
    }
    if (!pMe->IsPilotable())
    {
        return true;
    }
    if (pMe->mPilotType == PILOT_VEHICLESELF && pMe->mPilot == pMe)
    {
        return true;
    }
    return pMe->mPilot == 0;
}
}

// FUNCTION: SWBF2 0x00441690
bool AIUtil::IsNearNonAllyCommandPost(const GameObject* pObject, const CommandPost** ppPost)
{
    float fNearest = 3.402823466e38f;
    *ppPost = 0;
    PblVector3 vObject;
    vObject = pObject->GetMatrix().GetTranslation();
    for (int i = 0; i < CommandPostManager::Instance()->GetCount(); ++i)
    {
        CommandPost* pPost = CommandPostManager::Instance()->FindPostFromIndex(i);
        const GameObject* pPostObject = pPost->GetGameObject();
        if (!pObject->IsMyFriend(pPostObject))
        {
            const PblVector3 vDelta = GetCommandPostPos(pPost) - vObject;
            const float dz = vDelta.z;
            const float dx = vDelta.x;
            const float fDistanceSq = dx * dx + dz * dz;
            if (fDistanceSq < fNearest && fDistanceSq < 5625.0f)
            {
                fNearest = fDistanceSq;
                *ppPost = pPost;
            }
        }
    }
    return *ppPost != 0;
}

namespace AIDifficulty
{
    void Init();
}
namespace VisionManager
{
    void Initialize();
}
namespace AudibleManager
{
    void Initialize();
}
namespace HintManager
{
    void Initialize();
}
namespace ReserveManager
{
    void Init();
}
namespace AI
{
    namespace VOHelper
    {
        void Reset();
    }
}

// FUNCTION: SWBF2 0x0043f060
void AIUtil::Init()
{
    AIDifficulty::Init();

    gMinFlyHeight = -200.0f;
    gMaxFlyHeight = 200.0f;
    gMinPlayerFlyHeight = -200.0f;
    gMaxPlayerFlyHeight = 200.0f;
    gGroundFlyerMap = false;
    gAIAllowBlindJetJumps = true;
    gStationaryWaitTime = 10.0f;
    gStayInTurrets = false;
    gSkipPlayer = false;
    sUberMode = false;
    gSnipeSuitabilityDist = 70.0f;

    UnitController::sMemoryPool.Setup(600, sizeof(UnitController), UnitController::sMemoryPool.GetGrow());
    UnitAgent::sMemoryPool.Setup(600, 0x358, UnitAgent::sMemoryPool.GetGrow());
    Navigator::sMemoryPool.Setup(600, 0x1e8, Navigator::sMemoryPool.GetGrow());
    PathFollower::sMemoryPool.Setup(600, PathFollower::sMemoryPool.GetSize(),
                                    PathFollower::sMemoryPool.GetGrow());
    ConnectivityGraphFollower::sMemoryPool.Setup(600, ConnectivityGraphFollower::sMemoryPool.GetSize(),
                                                 ConnectivityGraphFollower::sMemoryPool.GetGrow());
    BaseHint::sMemoryPool.Setup(350, sizeof(BaseHint), BaseHint::sMemoryPool.GetGrow());
    AI::AIGoal::sMemoryPool.Setup(20, 0x3c, AI::AIGoal::sMemoryPool.GetGrow());
    VisionManager::RayRequest::sMemoryPool.Setup(201, VisionManager::RayRequest::sMemoryPool.GetSize(),
                                                 VisionManager::RayRequest::sMemoryPool.GetGrow());

    AI::AIGoalManager::Create();
    VisionManager::Initialize();
    AudibleManager::Initialize();
    PathManager::Initialize();
    PathFinder::Initialize();
    VehicleManager::Initialize();
    HintManager::Initialize();
    ReserveManager::Init();
    FlyRegionManager::Init();
    AIVisRegionManager::Init();
    CombatUtil::Init();

    for (int i = 0; i < 600; i++)
    {
        sSpied[i] = 0;
    }

    AI::VOHelper::Reset();
}

// FUNCTION: SWBF2 0x0043f040
int AIUtil::GetRandomIntPerSecond(int iMin, int iMax)
{
    if (iMax == iMin)
    {
        return iMax;
    }
    return GetRandomInt(iMin, iMax * ControllerManager::UPDATES_PER_SECOND);
}

// FUNCTION: SWBF2 0x0043f300
PblVector3 AIUtil::UnrotateVector(const PblMatrix& rMatrix, const PblVector3& vDir)
{
    return rMatrix.GetInverseFast().ApplyRotation(vDir);
}

// FUNCTION: SWBF2 0x0043f340
PblVector3 AIUtil::GetSafeNormalXZ(const PblVector3& v)
{
    PblVector3 vNormal = v;
    vNormal.y = 0.0f;
    if (vNormal.x == 0.0f && vNormal.z == 0.0f)
    {
        vNormal.x = 1.0f;
        return vNormal;
    }
    PblVector3 vTmp;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vTmp), reinterpret_cast<const D3DXVECTOR3*>(&vNormal));
    vNormal = vTmp;
    return vNormal;
}

// FUNCTION: SWBF2 0x0043f3b0
PblVector3 AIUtil::GetSafeNormal(const PblVector3& v)
{
    PblVector3 vNormal = v;
    if (vNormal.x == 0.0f && vNormal.y == 0.0f && vNormal.z == 0.0f)
    {
        vNormal.x = 1.0f;
        return vNormal;
    }
    PblVector3 vTmp;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vTmp), reinterpret_cast<const D3DXVECTOR3*>(&vNormal));
    vNormal = vTmp;
    return vNormal;
}

// FUNCTION: SWBF2 0x0043f430
bool AIUtil::GetLeadPosition(float fTime, const Controllable* pMe, const GameObject* pTarget,
                             PblVector3* pvLead)
{
    PblVector3 vTarget = pTarget->GetTargetPoint(pMe->mEyePoint, pMe->mEyeDir, -1);
    const PblVector3& vMyVel = GetVelocity(pMe->GetGameObject());
    const PblVector3& vHisVel = GetVelocity(pTarget);
    vTarget += (vHisVel - vMyVel) * fTime;
    const Aimer* pAimer = pMe->GetCurAimer();
    return pAimer->GetLeadPos(vTarget, GetVelocity(pTarget), pvLead);
}

namespace AIUtil
{
// FUNCTION: SWBF2 0x0043f4f0
static void sCalculateTurnControls(const Controllable* pMe, float fScale, const PblVector3& vAim,
                                   float fTurnRate, float* pfTurn, float fPitchRate, float* pfPitch,
                                   bool bFromEye)
{
    const PblVector3& vDest = vAim;

    PblVector3 vToDest = vDest - pMe->GetMatrix().GetTranslation();
    if (bFromEye)
    {
        vToDest = vDest - pMe->mEyePoint;
    }

    if (pMe->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer)
        || pMe->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover)
        || (pMe->GetEntity()->IsRtti(MountedTurret::rttiHashMountedTurret) && fTurnRate > 0.5f))
    {
        vToDest.y = 0.0f;
        const PblVector3 vLocal = UnrotateVector(pMe->GetMatrix(), vToDest);
        const PblVector3 vDir = pMe->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer)
                                    ? Normalize(vLocal)
                                    : GetSafeNormalXZ(vLocal);

        float fSide = -vDir.x;
        if (vDir.z < 0.0f)
        {
            fSide = fSide < 0.0f ? -1.0f : 1.0f;
        }

        const float fRate = Clamp(fTurnRate, 1.0f, 3.0f);
        const float fGain = 4.5f - fRate;

        float fTurn = 0.0f;
        if (fSide < 0.0f)
        {
            fTurn = Max(fGain * fSide, -1.0f);
        }
        else if (fSide > 0.0f)
        {
            fTurn = Min(fGain * fSide, 1.0f);
        }
        *pfTurn = fTurn * fTurn * (fTurn < 0.0f ? -1.0f : 1.0f);
    }
    else
    {
        const PblVector3 vUp = pMe->GetMatrix().GetYAxis();
        const PblVector3 vFlat = vToDest - vUp * DotProduct(vUp, vToDest);
        PblVector3 vFwd = pMe->GetMatrix().GetZAxis();
        if (bFromEye)
        {
            vFwd = pMe->mEyeDir;
        }
        float fTurn = -DotProduct(vUp, CrossProduct(vFwd, GetSafeNormal(vFlat)));
        const float fDiv = fScale * fTurnRate;
        if (fDiv > 0.0f)
        {
            fTurn /= fDiv;
        }
        *pfTurn = fTurn;

        const UnitController* pUnit = UnitControllerPtr(static_cast<const Controller*>(pMe->mCtrl));
        const float fMax =
            pUnit->mLowLevel.UseFastTurn() ? 2.0f : 1.0f;
        *pfTurn = Clamp(*pfTurn, -fMax, fMax);
    }

    const PblMatrix mEye = DirectionMatrix(pMe->mEyeDir, gVectorUnitY, pMe->mEyePoint);
    const PblVector3& vEye = mEye.GetTranslation();
    const PblVector3 vToDest2(vDest.x - vEye.x, vDest.y - vEye.y, vDest.z - vEye.z);

    if (pMe->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer)
        || pMe->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover)
        || (pMe->GetEntity()->IsRtti(MountedTurret::rttiHashMountedTurret) && fPitchRate > 0.5f))
    {
        const PblVector3 vLocal = UnrotateVector(mEye, vToDest2);
        PblVector3 vDir;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vLocal));

        float fPitch = 0.0f;
        const float fRate = Clamp(fPitchRate, 1.0f, 3.0f);
        const float fGain = 5.0f - fRate;

        if (vDir.y < 0.0f)
        {
            fPitch = Max(vDir.y * fGain, -1.0f);
        }
        else if (vDir.y > 0.0f)
        {
            fPitch = Min(vDir.y * fGain, 1.0f);
        }
        *pfPitch = fPitch * fPitch * (fPitch < 0.0f ? -1.0f : 1.0f);
    }
    else
    {
        const PblVector3& vRight = mEye.GetXAxis();
        const PblVector3& vFwd = mEye.GetZAxis();
        const PblVector3 vFlat = vToDest2 - vRight * DotProduct(vRight, vToDest2);
        if (DotProduct(vFwd, vFlat) > 0.0f)
        {
            float fPitch = -DotProduct(vRight, CrossProduct(vFwd, GetSafeNormal(vFlat)));
            const float fDiv = fScale * fPitchRate;
            if (fDiv > 0.0f)
            {
                fPitch /= fDiv;
            }
            *pfPitch = fPitch;
            *pfPitch = Clamp(*pfPitch, -1.0f, 1.0f);
        }
        else
        {
            *pfPitch = 0.0f;
        }
    }
}
}

// FUNCTION: SWBF2 0x0043fbf0
PblVector3 AIUtil::CalculateBubbleOffset(const Controllable* pControllable, const PblVector3& vPos,
                                         const GameObject* pTarget, const WeaponClass* pWeaponClass)
{
    const GameObject* pMyObj = pControllable->GetGameObject();
    const Controllable* pTargetCtrl = GetControllable(pTarget);
    PblVector3 vOffset(0.0f, 0.0f, 0.0f);

    if (pTargetCtrl && pTargetCtrl->mCtrl)
    {
        bool bCircle = pWeaponClass && pWeaponClass->mAIUseBubbleCircle;
        if (!bCircle && pMyObj)
        {
            const PblVector3& vVel = GetVelocity(pMyObj);
            const float fSpeedSq = vVel.x * vVel.x + vVel.y * vVel.y + vVel.z * vVel.z;
            const float fFast = GetMaxSpeed(pMyObj) * 0.95f;
            if (fSpeedSq > fFast * fFast)
            {
                bCircle = true;
            }
        }

        if (bCircle)
        {
            const PblVector3 vCircle = pControllable->mCtrl->GetTargetBubbleCircle();
            const PblVector3& vMe = pControllable->GetMatrix().GetTranslation();
            const PblVector3 vTo(vPos.x - vMe.x, vPos.y - vMe.y, vPos.z - vMe.z);

            const PblVector3 vWorldUp(0.0f, 1.0f, 0.0f);
            const PblVector3 vSide = Normalize(CrossProduct(vTo, vWorldUp));
            const PblVector3 vUp = Normalize(CrossProduct(vTo, vSide));

            vOffset = vSide * vCircle.x + vUp * vCircle.y;
        }
        else
        {
            vOffset = pControllable->mCtrl->mTargetBubbleOffset;
        }

        const float fSize = pTargetCtrl->mCtrl->mTargetBubbleSize_Cached;
        vOffset *= fSize;

        const float fMultiplier = pWeaponClass ? pWeaponClass->mAIBubbleSizeMultiplier : 1.0f;
        vOffset *= fMultiplier;

        if (!IsRtti(pTarget, EntityFlyer::rttiHashEntityFlyer) || GetFlyerState(pTarget) != 2)
        {
            const float fY = vOffset.y;
            const float fMaxY = GetRandomFloat(2.0f, 3.0f);
            vOffset.y = 0.0f > fY ? 0.0f : (fY > fMaxY ? fMaxY : fY);
        }
    }

    if (pMyObj && GetControllable(pMyObj) && CountHumansInVehicle(GetControllable(pMyObj)) >= 1)
    {
        vOffset.SetAll(0.0f);
    }
    else if (pTargetCtrl && CountHumansInVehicle(pTargetCtrl) == 0)
    {
        vOffset.x *= 0.3f;
        vOffset.y *= 0.3f;
        vOffset.z *= 0.3f;
    }

    if (IsRtti(pTarget, EntityFlyer::rttiHashEntityFlyer) && pMyObj->GetEntityType() == 1)
    {
        vOffset.x *= 0.1f;
        vOffset.y *= 0.1f;
        vOffset.z *= 0.1f;
    }
    return vOffset;
}

// FUNCTION: SWBF2 0x0043ffb0
void AIUtil::CalculateHeadingControls(const Controllable* pMe, float fScale,
                                      const ControllableDestination& rDest,
                                      const ControllableHeading& rHeading, float fTurnRate,
                                      float* pfTurn, float fPitchRate, float* pfPitch, bool bFromEye)
{
    const GameObject* pTarget = 0;
    float fWander = 0.0f;
    PblVector3 vAim;

    if (rHeading.mType == ControllableHeading::HEADING_OBJ)
    {
        const GameObject* pObj = GetAlivePtr(rHeading.mObj, true);
        const Aimer* pAimer = pMe->GetCurAimer();
        if (pObj && pAimer)
        {
            GetLeadPosition(fScale, pMe, pObj, &vAim);
            pTarget = pObj;
        }
        else
        {
            vAim = pMe->GetMatrix().GetTranslation() + pMe->GetMatrix().GetZAxis() * 30.0f;
        }
    }
    else if (rHeading.mType == ControllableHeading::HEADING_POS)
    {
        vAim = rHeading.mDest;
        fWander = rHeading.mDestWander;
        pTarget = UnitControllerPtr(static_cast<const Controller*>(pMe->mCtrl))->GetAttackHelperTarget();
    }
    else
    {
        const PblVector3& vPos = pMe->GetMatrix().GetTranslation();
        const PblVector3 vToDest(rDest.mPos.x - vPos.x, rDest.mPos.y - vPos.y, rDest.mPos.z - vPos.z);
        const float fStop = StopDist(pMe);
        PblVector3 vAhead;
        if (rDest.mHasDest && vToDest.x * vToDest.x + vToDest.z * vToDest.z > fStop * fStop)
        {
            vAhead = GetSafeNormal(vToDest);
        }
        else
        {
            vAhead = pMe->GetMatrix().GetZAxis();
        }
        vAim = pMe->GetMatrix().GetTranslation() + vAhead * 30.0f;
    }

    PblVector3 vBubble(0.0f, 0.0f, 0.0f);
    const Aimer* pAimer = pMe->GetCurAimer();

    const WeaponClass* pWeaponClass =
        pAimer ? *(const WeaponClass* const*)((const char*)pAimer->GetWeapon() + 0x64) : 0;
    if (pTarget)
    {
        vBubble = CalculateBubbleOffset(pMe, vAim, pTarget, pWeaponClass);
    }

    const float fDist = Length(vAim - pMe->GetMatrix().GetTranslation());

    float fDivider = 20.0f;
    float fClamp = 10.0f;
    if (pWeaponClass)
    {
        fDivider = pWeaponClass->mAIBubbleScaleDistDivider;
        fClamp = pWeaponClass->mAIBubbleScaleClamp;
    }
    float fBubbleScale;
    if (fDivider * 0.5f > fDist)
    {
        fBubbleScale = fDist / fDivider + 0.5f;
    }
    else
    {
        fBubbleScale = Clamp(fDist / fDivider + 0.5f, 1.0f, fClamp);
    }

    float fBubbleY = vBubble.y;
    if (fWander > 0.0f)
    {
        vBubble = pMe->mCtrl->mTargetBubbleOffset;
        fBubbleY = vBubble.y * vBubble.y * vBubble.y;
        if (fDist > 1.0f)
        {
            fBubbleScale = fDist * fWander;
        }
    }

    vAim.x += vBubble.x * fBubbleScale;
    vAim.y += fBubbleY * fBubbleScale;
    vAim.z += vBubble.z * fBubbleScale;
    sCalculateTurnControls(pMe, fScale, vAim, fTurnRate, pfTurn, fPitchRate, pfPitch, bFromEye);
}

static float FOV_ANGLE = 2.70526028f;

// GLOBAL: SWBF2 0x01e2fdb0
bool gAllSoldiers;



namespace
{
    const Obstacle* HoverGetObstacle(const Entity* pHover)
    {
        return static_cast<const EntityHover*>(pHover)->mObstacle;
    }

    const Obstacle* FlyerGetObstacle(const Entity* pFlyer)
    {
        return static_cast<const EntityFlyer*>(pFlyer)->mObstacle;
    }

    inline const AILowLevel* GetLowLevel(const UnitController* pUnit)
    {
        return &pUnit->mLowLevel;
    }
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00440380
void UpdatePhysics(const Controllable* pMe, float fScale, const ControllableDestination& rDest,
                const ControllableHeading& rHeading, float fThrustRate, float fStrafeRate,
                float fTurnRate, float fPitchRate, float* pfThrust, float* pfStrafe,
                float* pfTurn, float* pfPitch)
{
    bool bStopping = true;
    const bool bCanStrafe = fStrafeRate > 0.0001f;
    bool bNoDest = !rDest.mHasDest;

    if (rDest.mHasDest)
    {
        PblVector3 vToDest = rDest.mPos - pMe->GetMatrix().GetTranslation();
        vToDest.y = 0.0f;
        const bool bAtDest = fabsf(vToDest.x) < 0.0001f && fabsf(vToDest.y) < 0.0001f &&
                             fabsf(vToDest.z) < 0.0001f;

        const float fStop = AIUtil::StopDist(pMe);
        const float fDistSq = vToDest.z * vToDest.z + vToDest.x * vToDest.x;
        const bool bInStopDist = fStop * fStop > fDistSq;
        bStopping = bInStopDist &&
                    !GetLowLevel(UnitControllerPtr(static_cast<const Controller*>(pMe->mCtrl)))->HasPath();

        if (bAtDest ||
            (bStopping && (pMe->GetEntity()->IsRtti(EntityWalker::rttiHashEntityWalker) ||
                           (pMe->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover) &&
                            0.0001f > fStrafeRate))) ||
            (0.2f * 0.2f > fDistSq && pMe->GetEntity()->IsRtti(EntityDroideka::rttiHashEntityDroideka)))
        {
            bNoDest = true;
            *pfThrust = 0.0f;
            *pfStrafe = 0.0f;
        }
        else
        {
            PblVector3 vDir;
            D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vToDest));

            PblVector3 vFwd = pMe->GetMatrix().GetZAxis();
            PblVector3 vFwdXZ;
            if (0.0001f > vFwd.z * vFwd.z + vFwd.x * vFwd.x)
            {
                vFwdXZ = PblVector3(1.0f, 0.0f, 0.0f);
            }
            else
            {
                vFwd.y = 0.0f;
                vFwdXZ = Normalize(vFwd);
            }
            const float fForward = vFwdXZ.x * vDir.x + vDir.y * vFwdXZ.y + vDir.z * vFwdXZ.z;
            const float fSide = -(vFwdXZ.z * vDir.x - vDir.z * vFwdXZ.x);

            float fSpeed = 1.0f;
            if (bStopping)
            {
                fSpeed = LengthXZ(vToDest) / fStop;
                if (0.0f > fSpeed)
                {
                    fSpeed = 0.0f;
                }
                else if (fSpeed > 1.0f)
                {
                    fSpeed = 1.0f;
                }
            }

            if (bCanStrafe)
            {
                const float fThrust = fForward / fThrustRate;
                const float fStrafe = fSide / fStrafeRate;
                const float fAbsThrust = fabsf(fThrust);
                const float fAbsStrafe = fabsf(fStrafe);
                if (fAbsThrust > fAbsStrafe)
                {
                    *pfStrafe = fStrafe / fAbsThrust * fSpeed;
                    *pfThrust = (fThrust > 0.0f ? 1.0f : -1.0f) * fSpeed;
                }
                else
                {
                    *pfThrust = fThrust / fAbsStrafe * fSpeed;
                    *pfStrafe = (fStrafe > 0.0f ? 1.0f : -1.0f) * fSpeed;
                }
            }
            else
            {
                float fThrust = 0.0f;
                if (!(0.0f > fForward))
                {
                    fThrust = fForward > 1.0f ? 1.0f : fForward;
                }
                *pfThrust = fThrust * fSpeed;

                if (!bInStopDist && pMe->GetEntity()->IsRtti(EntityWalker::rttiHashEntityWalker) &&
                    PblMath::Cos(0.0872664601f) > fForward)
                {
                    *pfThrust = 0.0f;
                }
                *pfStrafe = 0.0f;
            }
        }
    }

    bool bUseHeading;
    if (pMe->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover))
    {
        bUseHeading = (bStopping && bCanStrafe) ||
                      GetLowLevel(UnitControllerPtr(static_cast<const Controller*>(pMe->mCtrl)))->mAllowHoverStrafe;
    }
    else
    {
        bUseHeading = bCanStrafe;
    }
    if (bUseHeading || bNoDest)
    {
        AIUtil::CalculateHeadingControls(pMe, fScale, rDest, rHeading, fTurnRate, pfTurn, fPitchRate,
                                         pfPitch, false);
    }
    else
    {
        bool bFromEye = false;
        if (pMe->GetEntity()->IsRtti(EntityDroideka::rttiHashEntityDroideka))
        {
            if (GetLowLevel(UnitControllerPtr(static_cast<const Controller*>(pMe->mCtrl)))->mFoldStatus ==
                FOLD_UNFOLD)
            {
                bFromEye = true;
            }
        }
        AIUtil::sCalculateTurnControls(pMe, fScale, rDest.mPos, fTurnRate, pfTurn, fPitchRate, pfPitch,
                                       bFromEye);
    }
}
}

// FUNCTION: SWBF2 0x004408b0
bool AIUtil::GetTangentPointToCircle(const PblVector3& vCenter, const PblVector3& vPoint, float fRadius,
                                     PblVector3& vTangent, PblVector3& vDir, bool bClockwise)
{
    const float dx = vPoint.x - vCenter.x;
    const float dz = vPoint.z - vCenter.z;
    const float fDist = PblMath::Sqrt(dz * dz + dx * dx);
    if (fRadius > fDist)
    {
        return false;
    }

    const float fHalfAngle = acosf(fRadius / fDist);
    const float fAngle = atan2f(dz, dx);
    float fSin;
    float fCos;
    PblMath::SinCos(&fSin, &fCos, bClockwise ? fAngle - fHalfAngle : fAngle + fHalfAngle);
    vDir = PblVector3(fCos, 0.0f, fSin);
    const PblVector3 vOffset(vDir.x * fRadius, vDir.y * fRadius, vDir.z * fRadius);
    vTangent = PblVector3(vCenter.x + vOffset.x, vCenter.y + vOffset.y, vCenter.z + vOffset.z);
    return true;
}

// FUNCTION: SWBF2 0x00440a20
void AIUtil::CalculateFlyerSteerPoint(const PblVector3& vPos, const PblVector3& vDir, const PblVector3& vGoal,
                const PblVector3& vGoalDir, float fRadius, PblVector3& vSteer, int iTurn)
{
    const float fGoalX = vGoal.x;
    const float fGoalZ = vGoal.z;
    const float fPosX = vPos.x;
    const float fPosZ = vPos.z;

    PblVector3 vFlat = vDir;
    vFlat.y = 0.0f;
    PblVector3 vFwd;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vFwd), reinterpret_cast<const D3DXVECTOR3*>(&vFlat));
    vFlat = vGoalDir;
    vFlat.y = 0.0f;
    vFlat = Normalize(vFlat);
    const PblVector3& vGoalFwd = vFlat;

    float fGoalCircleX = fGoalX + vGoalFwd.z * fRadius;
    float fGoalCircleZ = fGoalZ - vGoalFwd.x * fRadius;
    const float fGoalOtherX = fGoalX - vGoalFwd.z * fRadius;
    const float fGoalOtherZ = fGoalZ + vGoalFwd.x * fRadius;
    bool bGoalSide;
    if ((fGoalOtherX - fPosX) * (fGoalOtherX - fPosX) + (fGoalOtherZ - fPosZ) * (fGoalOtherZ - fPosZ) >
        (fGoalCircleX - fPosX) * (fGoalCircleX - fPosX) + (fGoalCircleZ - fPosZ) * (fGoalCircleZ - fPosZ))
    {
        bGoalSide = true;
    }
    else
    {
        bGoalSide = false;
        fGoalCircleX = fGoalOtherX;
        fGoalCircleZ = fGoalOtherZ;
    }

    const float fRightX = fPosX + vFwd.z * fRadius;
    const float fRightZ = fPosZ - vFwd.x * fRadius;
    const float fLeftX = fPosX - vFwd.z * fRadius;
    const float fLeftZ = fPosZ + vFwd.x * fRadius;
    float fCircleX;
    float fCircleZ;
    bool bSide;
    if (iTurn == 1)
    {
        bSide = true;
        fCircleX = fRightX;
        fCircleZ = fRightZ;
    }
    else if (iTurn == -1)
    {
        bSide = false;
        fCircleX = fLeftX;
        fCircleZ = fLeftZ;
    }
    else
    {
        const float fRightDist = (fRightX - fGoalX) * (fRightX - fGoalX) + (fRightZ - fGoalZ) * (fRightZ - fGoalZ);
        const float fLeftDist = (fLeftX - fGoalX) * (fLeftX - fGoalX) + (fLeftZ - fGoalZ) * (fLeftZ - fGoalZ);
        if (fLeftDist > fRightDist)
        {
            bSide = true;
            fCircleX = fRightX;
            fCircleZ = fRightZ;
        }
        else
        {
            bSide = false;
            fCircleX = fLeftX;
            fCircleZ = fLeftZ;
        }
        if ((fRadius + fRadius) * (fRadius + fRadius) >
                (fCircleX - fGoalCircleX) * (fCircleX - fGoalCircleX) +
                    (fCircleZ - fGoalCircleZ) * (fCircleZ - fGoalCircleZ) &&
            bSide != bGoalSide)
        {
            if (fLeftDist > fRightDist)
            {
                bSide = false;
                fCircleX = fLeftX;
                fCircleZ = fLeftZ;
            }
            else
            {
                bSide = true;
                fCircleX = fRightX;
                fCircleZ = fRightZ;
            }
        }
    }

    PblVector3 vCircle(fCircleX, 0.0f, fCircleZ);
    PblVector3 vGoalFlat(fGoalX, 0.0f, fGoalZ);
    PblVector3 vTangent;
    PblVector3 vTangentDir;
    float fTangentX;
    float fTangentZ;
    if (AIUtil::GetTangentPointToCircle(vCircle, vGoalFlat, fRadius, vTangent, vTangentDir, bSide))
    {
        fTangentX = vTangent.x;
        fTangentZ = vTangent.z;
    }
    else
    {
        const float dz = fGoalZ - fCircleZ;
        const float dx = fGoalX - fCircleX;
        const float fLen = sqrtf(dz * dz + dx * dx) + 1e-5f;
        fTangentX = dx / fLen * fRadius + fCircleX;
        fTangentZ = dz / fLen * fRadius + fCircleZ;
    }

    const float fOffX = fTangentX - fCircleX;
    float fOffZ = fTangentZ - fCircleZ;
    if (bSide)
    {
        fOffZ = -fOffZ;
        fTangentZ -= fOffX;
    }
    else
    {
        fTangentZ += fOffX;
    }
    vSteer.y = vPos.y;
    vSteer.x = fTangentX - fOffZ;
    vSteer.z = fTangentZ;
}

// FUNCTION: SWBF2 0x00440db0
void AIUtil::CalculateSoldierSteerPoint(const PblVector3& vPrev, const PblVector3& vGoto,
                                        const PblVector3& vPos, float fSpeed, PblVector3& vSteer)
{
    PblVector3 vTo;
    PblVector3 vFrom;
    vFrom = vPrev;
    vFrom.y = vPos.y;
    vTo = vGoto;
    vTo.y = vPos.y;
    const PblVector3 vLeg(vTo.x - vFrom.x, vTo.y - vFrom.y, vTo.z - vFrom.z);
    if (0.0001f > vLeg.x * vLeg.x + vLeg.y * vLeg.y + vLeg.z * vLeg.z)
    {
        vSteer = vTo;
        return;
    }

    const PblVector3 vDir = Normalize(vTo - vFrom);
    const PblVector3 vRel(vPos.x - vFrom.x, vPos.y - vFrom.y, vPos.z - vFrom.z);
    const float fAhead = vDir.y * vRel.y + vRel.x * vDir.x + vDir.z * vRel.z + fSpeed + 0.2f;
    if (fAhead * fAhead > vLeg.z * vLeg.z + vLeg.x * vLeg.x)
    {
        vSteer = vTo;
        return;
    }
    vSteer = PblVector3(vDir.x * fAhead + vFrom.x, vDir.y * fAhead + vFrom.y,
                        vDir.z * fAhead + vFrom.z);
}

// FUNCTION: SWBF2 0x00440f20
bool AIUtil::IsATAT(const Controllable* pControllable)
{
    return pControllable->GetEntity()->GetEntityClass()->mId == 0x6fbcdecc;
}

// FUNCTION: SWBF2 0x00440f40
AIUtil::EntityType AIUtil::GetEntityType(const Entity* pEntity)
{
    if (pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return ENTITY_SOLDIER;
    }
    if (pEntity->IsRtti(EntityDroid::rttiHashEntityDroid))
    {
        return ENTITY_DROID;
    }
    if (pEntity->IsRtti(EntityWalker::rttiHashEntityWalker))
    {
        return ENTITY_WALKER;
    }
    if (pEntity->IsRtti(EntityDroideka::rttiHashEntityDroideka))
    {
        return ENTITY_DROIDEKA;
    }
    if (pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        return ENTITY_FLYER;
    }
    if (pEntity->IsRtti(EntityHover::rttiHashEntityHover))
    {
        return ENTITY_HOVER;
    }
    if (pEntity->IsRtti(MountedTurret::rttiHashMountedTurret))
    {
        return ENTITY_TURRET;
    }
    if (pEntity->IsRtti(EntityBuildingArmed::rttiHashEntityBuildingArmed))
    {
        return ENTITY_BUILDING_ARMED;
    }
    if (pEntity->IsRtti(EntityBuilding::rttiHashEntityBuilding))
    {
        return ENTITY_BUILDING;
    }
    if (pEntity->IsRtti(EntityItem::rttiHashEntityItem))
    {
        return ENTITY_ITEM;
    }
    if (pEntity->IsRtti(PassengerSlot::rttiHashPassengerSlot))
    {
        return ENTITY_TURRET;
    }
    if (pEntity->IsRtti(EntityRemoteTerminal::rttiHashEntityRemoteTerminal))
    {
        return ENTITY_REMOTE_TERMINAL;
    }
    return ENTITY_SOLDIER;
}

// FUNCTION: SWBF2 0x00441070
bool AIUtil::IsHintAvailable(const GameObject* pObj, const BaseHint* pHint)
{
    return !pHint->IsReserved(pObj->GetTeam());
}

// FUNCTION: SWBF2 0x00441090
float AIUtil::StopDistIntermediate(const Controllable* pControllable)
{
    switch (GetEntityType(pControllable->GetEntity()))
    {
    case ENTITY_FLYER:
        return static_cast<const EntityFlyerClass*>(pControllable->GetEntity()->GetEntityClass())
                   ->mMaxFlyerSpeed * 0.25f;
    case ENTITY_HOVER:
        return static_cast<const EntityHoverClass*>(pControllable->GetEntity()->GetEntityClass())
                   ->mForwardSpeed * 0.5f;
    default:
        return 1.0f;
    }
}

// FUNCTION: SWBF2 0x00441100
float AIUtil::StopDist(const Controllable* pControllable)
{
    switch (GetEntityType(pControllable->GetEntity()))
    {
    case ENTITY_SOLDIER:
    case ENTITY_DROID:
        return 0.5f;
    case ENTITY_HOVER:
        return static_cast<const EntityHoverClass*>(pControllable->GetEntity()->GetEntityClass())
                   ->mForwardSpeed * 0.5f;
    default:
        return 4.0f;
    }
}

// FUNCTION: SWBF2 0x00441160
bool AIUtil::GetAlivePtrCommon(const GameObject* pObj, bool bIncludeBuildings)
{
    if (!pObj)
    {
        return false;
    }
    if (pObj->IsAlive())
    {
        return true;
    }
    if (bIncludeBuildings)
    {
        const unsigned int uHealthType = pObj->mHealthType;
        if (uHealthType == HEALTH_TYPE_BUILDING_DEAD || uHealthType == HEALTH_TYPE_BUILDING_UNBUILT)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004411d0
GameObject const* AIUtil::GetAlivePtr(PblHandle<GameObject const> const& hObj, bool bIncludeBuildings)
{
    const GameObject* pObj = hObj;
    return GetAlivePtrCommon(pObj, bIncludeBuildings) ? pObj : 0;
}

// FUNCTION: SWBF2 0x00441190
GameObject* AIUtil::GetAlivePtr(PblHandle<GameObject> const& hObj, bool bIncludeBuildings)
{
    GameObject* pObj = hObj;
    return GetAlivePtrCommon(pObj, bIncludeBuildings) ? pObj : 0;
}

// FUNCTION: SWBF2 0x00441210
PblVector3 AIUtil::GetRandomPtInRegion(const RedRegion* pRegion)
{
    PblVector3 vPoint;
    if (!pRegion)
    {
        vPoint = gMatrixIdentity.GetTranslation();
        return vPoint;
    }

    switch (pRegion->mType)
    {
    case PblRegion::type_box:
    case PblRegion::type_box_aligned:
    {
        const PblVector3 v = PblRandom::GetGlobalVector3();
        const PblVector3 vLocal(pRegion->mSize.x * v.x, pRegion->mSize.y * v.y, pRegion->mSize.z * v.z);
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPoint), reinterpret_cast<const D3DXVECTOR3*>(&vLocal), reinterpret_cast<const D3DXMATRIX*>(&pRegion->mMatrix));
        return vPoint;
    }
    case PblRegion::type_sphere:
    {
        PblVector3 v;
        do
        {
            v = PblRandom::GetGlobalVector3();
        } while (v.x * v.x + v.y * v.y + v.z * v.z > 1.0f);
        const PblVector3& vCenter = pRegion->mMatrix.GetTranslation();
        const float fRadius = pRegion->mSize.x;
        const PblVector3 vOffset(v.x * fRadius, v.y * fRadius, v.z * fRadius);
        vPoint.x = vCenter.x + vOffset.x;
        vPoint.y = vCenter.y + vOffset.y;
        vPoint.z = vCenter.z + vOffset.z;
        return vPoint;
    }
    case PblRegion::type_cylinder:
    case PblRegion::type_cylinder_xaligned:
    case PblRegion::type_cylinder_yaligned:
    case PblRegion::type_cylinder_zaligned:
    {
        PblVector3 v;
        do
        {
            v = PblRandom::GetGlobalVector3();
        } while (v.x * v.x + v.z * v.z > 1.0f);
        const PblVector3 vLocal(pRegion->mSize.x * v.x, pRegion->mSize.y * v.y, pRegion->mSize.z * v.z);
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPoint), reinterpret_cast<const D3DXVECTOR3*>(&vLocal), reinterpret_cast<const D3DXMATRIX*>(&pRegion->mMatrix));
        return vPoint;
    }
    default:
        vPoint.x = 0.0f;
        vPoint.y = 0.0f;
        vPoint.z = 0.0f;
        return vPoint;
    }
}

// FUNCTION: SWBF2 0x004413e0
PblVector3 AIUtil::GetCommandPostOffset(const CommandPost* pPost)
{
    const RedRegion* pRegion = pPost->mCaptureRegion;
    if (pRegion)
    {
        return GetRandomPtInRegion(pRegion) - pRegion->mMatrix.GetTranslation();
    }
    return PblVector3(0.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x00441450
PblVector3 AIUtil::GetCommandPostPos(const CommandPost* pPost)
{
    if (pPost->mCaptureRegion)
    {
        return pPost->mCaptureRegion->mMatrix.GetTranslation();
    }
    return pPost->GetMatrix().GetTranslation();
}

// FUNCTION: SWBF2 0x00441480
bool AIUtil::IsNearPost(const Controllable* pMe, const CommandPost* pPost, const PblVector3& vOffset)
{
    const PblVector3 vMyPos = pMe->GetMatrix().GetTranslation();
    const PblVector3 vDelta = GetCommandPostPos(pPost) + vOffset - vMyPos;
    const float fDistSq = vDelta.x * vDelta.x + vDelta.z * vDelta.z;

    float fRadius = pMe->GetGameObject()->GetMaxSpeed() * 0.5f;
    if (pPost->mCaptureRegion)
    {
        if (pPost->mCaptureRegion->IsPointInside(vMyPos))
        {
            return true;
        }
    }
    else
    {
        const float fMyRadius = pMe->GetGameObject()->GetCollisionObject()->GetCollisionRadius();
        const float fPostRadius = pPost->GetGameObject()->GetCollisionObject()->GetCollisionRadius();
        const float fReach = fMyRadius + fPostRadius + 5.0f;
        fRadius = 10.0f > fReach ? 10.0f : fReach;
        if (pPost->GetGameObject()->GetMaxSpeed() > 0.0f)
        {
            const float fMoving = fRadius + 20.0f;
            return fMoving * fMoving > fDistSq;
        }
    }
    return fRadius * fRadius > fDistSq;
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00441860
bool __cdecl CanPotentiallySee(Controllable* pMe, const GameObject* pObject, float* pOut)
{
    // GLOBAL: SWBF2 0x01faa7d0
    static const float sCosHalfFOV = PblMath::Cos(FOV_ANGLE * 0.5f);

    const PblVector3& vEye = pMe->mEyePoint;
    const PblVector3& vDir = pMe->mEyeDir;
    const PblVector3 vTarget = pObject->GetTargetPoint(vEye, vDir, -1);
    const PblVector3 vTo(vTarget.x - vEye.x, vTarget.y - vEye.y, vTarget.z - vEye.z);
    const float fDistSq = vTo.y * vTo.y + vTo.x * vTo.x + vTo.z * vTo.z;
    if (100.0f > fDistSq)
    {
        if (pOut)
        {
            *pOut = fDistSq;
        }
        return true;
    }

    bool bResult;
    if (fDistSq > 0.0001f)
    {
        const float fDot = vDir.x * vTo.x + vDir.y * vTo.y + vDir.z * vTo.z;
        if (fDot > 0.0f && fDot * fDot > sCosHalfFOV * fDistSq * sCosHalfFOV)
        {
            bResult = true;
        }
        else
        {
            bResult = false;
        }
    }
    else
    {
        bResult = true;
    }
    if (pOut)
    {
        *pOut = fDistSq;
    }
    return bResult;
}
}

// FUNCTION: SWBF2 0x00441620
void AIUtil::DeployCargo(GameObject* pObject, const CommandPost*  )
{
    const PblList<Controllable>& list = pObject->GetControllableList();
    PblList<Controllable>::Itor it = list.Begin();
    ++it;
    for (; (Controllable*)it; ++it)
    {
        Controller* pCtrl = ((Controllable*)it)->mCtrl;
        if (pCtrl && !pCtrl->IsRtti(PlayerController::rttiHashPlayerController))
        {
            pCtrl->mOwner->GetPilot()->ExitVehicle(true, true);
        }
    }
}

// FUNCTION: SWBF2 0x00441c10
float AIUtil::GetCollisionRadius(const GameObject* pObj)
{
    const GameObjectClass* pClass = pObj->GetGameObjectClass();
    const float fCached = pClass->mAICollisionRadius;
    if (fCached > 0.0f)
    {
        return fCached;
    }

    const EntityType eType = GetEntityType(pObj);
    if (eType == ENTITY_HOVER)
    {
        const EntityHoverClass* pHover = static_cast<const EntityHoverClass*>(pObj->GetEntityClass());
        const float fBarrierX = pHover->mBarrierX;
        if (fBarrierX > 0.0f)
        {
            return sqrtf(pHover->mBarrierZ * pHover->mBarrierZ + fBarrierX * fBarrierX) * 0.5f;
        }
    }

    const PblVector3& vMin = pObj->GetModel()->_mModel->_TotalBoundingBox._vMin;
    const PblVector3& vMax = pObj->GetModel()->_mModel->_TotalBoundingBox._vMax;
    const float dx = vMax.x - vMin.x;
    const float dz = vMax.z - vMin.z;
    const float fDiag = PblMath::Sqrt(dx * dx + dz * dz);
    if (eType == ENTITY_HOVER)
    {
        return fDiag * 0.5f;
    }
    if (Team::GetPtr(pObj->GetTeam())->mSide == Team::side_CIS && eType == ENTITY_WALKER)
    {
        return fDiag * 0.5f;
    }
    pClass->mAICollisionRadius = fDiag * 0.25f;
    return pClass->mAICollisionRadius;
}

// FUNCTION: SWBF2 0x00441d40
int AIUtil::GetAIType(const GameObject* pObj)
{
    const int eType = pObj->GetGameObjectClass()->mAIType;
    if (eType == ConnectivityGraph::CGT_FLAG_HOVER && pObj->GetCarriedFlagItem())
    {
        return ConnectivityGraph::CGT_FLAG_SOLDIER;
    }
    return eType;
}

// FUNCTION: SWBF2 0x00441d60
bool AIUtil::AvoidBarriers(EntityFlyer* pFlyer, const PblVector3&  , PblVector3* pNewDest)
{
    PblVector3 vPos;
    vPos = pFlyer->GetMatrix().GetTranslation();
    const PblVector3 vForward = GetSafeNormalXZ(pFlyer->GetMatrix().GetZAxis());
    const float fAhead = pFlyer->mClass->mMaxFlyerSpeed;
    const PblVector3 vAhead = vPos + vForward * fAhead;
    const float fCollisionRadius = GetCollisionRadius(pFlyer);
    const float fRadius = 15.0f > fCollisionRadius ? 15.0f : fCollisionRadius;

    PblVector3 vStart;
    vStart = vPos;
    const Obstacle* pBarrier = GetGameObjectBarrier(pFlyer->GetGameObject());
    bool bBlocked = !PathFinder::GetClosestValidPoint(vStart, vStart, fRadius, GetAIType(pFlyer), &pBarrier, 1);
    if (!bBlocked)
    {
        PblVector3 vEnd;
        vEnd = vAhead;
        PblVector3 vNormal;
        bBlocked = PathFinder::PushValidPoint(vStart, vEnd, vNormal, fRadius, GetAIType(pFlyer), &pBarrier, 1) ||
                   FlyRegionManager::RayTest(vPos, vAhead, fRadius);
    }
    if (!bBlocked)
    {
        return false;
    }

    const float fSide = pFlyer->mClass->mMaxFlyerSpeed * 2.0f;
    const float fDist = 30.0f > fSide ? 30.0f : fSide;
    const PblVector3 vSide = CrossProduct(vForward, gVectorUnitY);
    *pNewDest = PblVector3(vSide.x * fDist + vPos.x, vSide.y * fDist + vPos.y, vSide.z * fDist + vPos.z);
    return true;
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00441f60
float GetTargetSpeedForRealSpeed(float aSpd, float aMinSpd, float aMidSpd, float aMaxSpd)
{
    float targetSpeed = aSpd / aMaxSpd;
    return Clamp(targetSpeed, 0.0f, 1.0f);
}
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00441f80
float GetThrustForTargetSpeed(float fT, float fA, float fB, float fC)
{
    float fPivot;
    if (fA > 0.0f)
    {
        fPivot = -1.0f;
    }
    else if (fB > 0.0f)
    {
        fPivot = fA / (fB - fA);
    }
    else if (fC > 0.0f)
    {
        fPivot = (0.0f - fB) / (fC - fB);
    }
    else
    {
        fPivot = 1.0f;
    }
    if (0.0f > fT)
    {
        return (fPivot - -1.0f) * fT - 1.0f;
    }
    return (1.0f - fPivot) * fT + fPivot;
}
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00442000
bool ShouldSoldierDodgeThisType(int eType)
{
    switch (eType)
    {
    case COLL_RIGID:
    case COLL_STATIC:
    case COLL_ATTACHED:
    case COLL_SPECIAL:
        return true;
    case COLL_TERRAIN:
    case COLL_WATER:
    case COLL_LEAFPATCH:
        return false;
    default:
        return false;
    }
}
}

// FUNCTION: SWBF2 0x00442070
bool AIUtil::IsValidEnemy(const GameObject* pMe, const GameObject* pObj)
{
    return pObj->IsAlive() && pObj->mIsTargetable && pObj->IsSuitableEnemy() && pMe->IsMyEnemy(pObj);
}

// FUNCTION: SWBF2 0x004420c0
bool AIUtil::DirectLineToTarget(const GameObject* pObj, const PblVector3& vPos)
{
    PblVector3 vFrom = pObj->GetMatrix().GetTranslation();
    PblVector3 vTo = vPos;
    PathFinder::GetClosestValidPoint(vFrom, vFrom, pObj);
    PathFinder::GetClosestValidPoint(vTo, vTo, pObj);
    return !PathFinder::TestTrivialStraightLineHit(vFrom, vTo, pObj);
}

// FUNCTION: SWBF2 0x00442130
void AIUtil::Slide(GameObject* pMe, const GameObject* pTarget, bool bUseAttackHelper,
                   bool* pbIsPathing, float* pfPathTime, float fMinDist, float fOffset)
{
    if (!pMe || !pMe->GetControllable() || !pMe->GetControllable()->mCtrl)
    {
        return;
    }

    AILowLevel& lowLevel = UnitControllerPtr(pMe->GetControllable()->mCtrl)->GetLowLevel();
    if (DirectLineToTarget(pMe, pTarget->GetMatrix().GetTranslation()))
    {
        if (!lowLevel.IsNavigatorInProgress())
        {
            const PblVector3& vTarget = pTarget->GetMatrix().GetTranslation();
            const PblVector3& vMe = pMe->GetMatrix().GetTranslation();
            const float dx = vMe.x - vTarget.x;
            const float dz = vMe.z - vTarget.z;
            if (!(dx * dx + dz * dz > 1.0f))
            {
                return;
            }
            lowLevel.SetNavigator_Stationary();
        }
        lowLevel.SetNavigator_Slide(pTarget, fOffset, bUseAttackHelper, 0);
        *pbIsPathing = false;
    }
    else
    {
        if (*pbIsPathing)
        {
            if (lowLevel.IsAtDest())
            {
                *pbIsPathing = false;
            }
            else if (*pbIsPathing)
            {
                return;
            }
        }
        if (GameLoop::GetMissionTime() > *pfPathTime + fMinDist)
        {
            *pbIsPathing = true;
            *pfPathTime = GameLoop::GetMissionTime();
            PblVector3 vDest;
            vDest = pTarget->GetMatrix().GetTranslation();
            lowLevel.SetNavigator_Goto(vDest, 0, false, false);
        }
    }
}

// FUNCTION: SWBF2 0x00442280
int AIUtil::CountUnitsInVehicle(const Controllable* pControllable)
{
    int iCount = 0;
    const PblList<Controllable>& list = pControllable->GetGameObject()->GetControllableList();
    for (PblList<Controllable>::Itor it = list.Begin(); it; ++it)
    {
        const Controllable* pRider = it;
        if (pRider->mCtrl)
        {
            ++iCount;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x004422c0
int AIUtil::CountHumansInVehicle(const Controllable* pControllable)
{
    int iCount = 0;
    const PblList<Controllable>& list = pControllable->GetGameObject()->GetControllableList();
    for (PblList<Controllable>::Itor it = list.Begin(); it; ++it)
    {
        const Controllable* pRider = it;
        if (pRider->mCtrl && pRider->mPlayerId >= 0)
        {
            ++iCount;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x00442300
bool AIUtil::HasCPMoved(const CommandPost* pPost, Controllable* pMe, const PblVector3& vLastPos,
                        float fLastTime, const PblVector3& vDir)
{
    const UnitController* pUnit = UnitControllerPtr(pMe->mCtrl);
    const PblVector3 vPost = GetCommandPostPos(pPost);
    if (SquaredLength(vPost - vLastPos) > 400.0f && GameLoop::GetMissionTime() > fLastTime + 15.0f)
    {
        PblVector3 vMe;
        vMe = pMe->GetMatrix().GetTranslation();
        const PblVector3 vToPost(vPost.x - vMe.x, vPost.y - vMe.y, vPost.z - vMe.z);
        const float fDot = (vMe.y - vDir.y) * vToPost.y + (vMe.x - vDir.x) * vToPost.x +
                           (vMe.z - vDir.z) * vToPost.z;
        if (0.0f > fDot)
        {
            return true;
        }
        if (GetLowLevel(pUnit)->IsAtDest() && !GetLowLevel(pUnit)->IsStuck())
        {
            return true;
        }
    }
    return false;
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00442430
bool IsImportant(const GameObject* pObj)
{
    if (!gAllSoldiers && !NetGame::IsEnabled())
    {
        const PblVector3& vVel = pObj->GetVelocity();
        if (100.0f > vVel.x * vVel.x + vVel.y * vVel.y + vVel.z * vVel.z)
        {
            PblVector3 vPos;
            vPos = static_cast<const CollisionObject*>(pObj)->GetMatrix().GetTranslation();
            PblVector3 vCam;
            vCam = CameraManager::Instance()->GetCamera(0)->GetMatrix().GetTranslation();
            if (SquaredLength(vCam - vPos) > 2025.0f &&
                !CameraManager::Instance()->IsInsideAnyCamera(vPos, -1.0f, 70.0f))
            {
                return false;
            }
        }
    }
    return true;
}
}

// FUNCTION: SWBF2 0x00442540
bool AIUtil::IsPlayerTeam(int iTeam)
{
    const Character* pPlayer1 = Character::FindPlayer(0);
    const Character* pPlayer2 = Character::FindPlayer(1);
    if (NetGame::IsEnabled() || !pPlayer1)
    {
        return false;
    }
    if (!pPlayer2)
    {
        if (iTeam == pPlayer1->mTeamNumber)
        {
            return true;
        }
    }
    else if (pPlayer1->mTeamNumber == pPlayer2->mTeamNumber && iTeam == pPlayer1->mTeamNumber)
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x004425d0
PblVector3 AIUtil::DumbDown(const GameObject* pObject, float fDeltaTime, Aimer* pAimer,
                            const PblVector3& vDir)
{
    const UnitController* pUnit = UnitControllerPtr(pObject->GetControllable());
    if (pUnit && pUnit->mLodState < 3)
    {
        const GameObject* pTarget = pUnit->GetAttackHelperTarget();
        if (!pTarget || !pTarget->GetControllable() || pTarget->GetControllable()->mPlayerId < 0)
        {
            return vDir;
        }
    }
    if (pObject->GetControllable() && CountHumansInVehicle(pObject->GetControllable()) >= 1)
    {
        return vDir;
    }

    Weapon* pWeapon = pAimer->GetWeapon();
    float fMaxAngAcc = AIDifficulty::GetAimerMaxAngAcc(pUnit);
    float fDamping = AIDifficulty::GetAimerDamping(pUnit);
    if (pWeapon)
    {
        pWeapon->SetKickSpreadFactor(AIDifficulty::GetAimerSpread(pUnit));
        pWeapon->SetYawSpreadFactor(AIDifficulty::GetAimerYawSpread(pUnit));
        if (!pWeapon->IsRtti(WeaponCannon::rttiHashWeaponCannon))
        {
            fMaxAngAcc *= 0.5f;
            fDamping *= 0.5f;
        }
    }

    PblVector3 vAim = pAimer->mDirection;
    if (vDir.y * vAim.y + vDir.x * vAim.x + vAim.z * vDir.z < (float)cos(0.523598790f))
    {
        const PblVector3 vUp = CrossProduct(vDir, vAim);
        const PblVector3 vRight = CrossProduct(vDir, vUp);
        PblMatrix mAim(vRight, vUp, vDir, gVectorZero);
        PblMatrix mRotate;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mAim), reinterpret_cast<const D3DXMATRIX*>(&mAim),
                           D3DXMatrixRotationY(reinterpret_cast<D3DXMATRIX*>(&mRotate),
                                               vUp.y > 0.0f ? 0.523598790f : -0.523598790f));
        vAim = mAim.GetZAxis();
        pAimer->SetAngVel(gVectorZero);
    }

    if (fDeltaTime > 0.0f)
    {
        const float fInv = 1.0f / fDeltaTime;
        const PblVector3 vWant = (vDir - vAim) * fInv;
        const PblVector3& vAngVel = pAimer->GetAngVel();
        const PblVector3 vAcc = (vWant - vAngVel) * fInv;
        const float fAcc = Length(vAcc);
        const float fScale = fMaxAngAcc > fAcc ? 1.0f : fMaxAngAcc / fAcc;
        const PblVector3 vNewVel = vAngVel + (vAcc * fScale - vAngVel * fDamping) * fDeltaTime;
        pAimer->SetAngVel(vNewVel);
    }

    return Normalize(pAimer->GetAngVel() * fDeltaTime + vAim);
}

// FUNCTION: SWBF2 0x00442ab0
bool AIUtil::IsSnipeHintNearby(const GameObject* pObj, const SnipeHelper& helper,
                               const SnipeHint** ppHint, float* pfNextCheckTime)
{
    if (GameLoop::GetMissionTime() > *pfNextCheckTime)
    {
        *pfNextCheckTime = GetRandomFloat(0.0f, 4.0f) + (GameLoop::GetMissionTime() + 8.0f);
        *ppHint = 0;
        const SnipeHint* pHint = helper.HELPERFindSnipeHint();
        if (pHint)
        {
            *ppHint = pHint;
            pHint->Reserve(pObj);
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004419c0
PblVector3 AIUtil::GetMyVisionPos(const Controllable* pMe)
{
    return pMe->GetEyePoint();
}

// FUNCTION: SWBF2 0x004419e0
PblVector3 AIUtil::GetHisVisionTargetPos(const Controllable* pMe, const GameObject* pObj)
{
    const PblVector3 vMyPos = GetMyVisionPos(pMe);
    if (pObj->GetControllable())
    {
        PblVector3 vHisEye;
        vHisEye = pObj->GetControllable()->GetEyePoint();
        PblVector3 vDir;
        vDir = pMe->mEyeDir;
        const PblVector3 vTarget = pObj->GetTargetPoint(vMyPos, vDir, -1);
        const PblVector3 vSum(vHisEye.x + vTarget.x, vHisEye.y + vTarget.y, vHisEye.z + vTarget.z);
        return PblVector3(vSum.x * 0.5f, vSum.y * 0.5f, vSum.z * 0.5f);
    }
    return pObj->GetTargetPoint(vMyPos, pMe->mEyeDir, -1);
}

// FUNCTION: SWBF2 0x00441ac0
bool AIUtil::CanSeePosition(Controllable* pMe, const PblVector3& vPos, const GameObject* pObj)
{
    const PblVector3 vFrom = GetMyVisionPos(pMe);
    const GameObject* ignore[2];
    ignore[0] = pMe->GetGameObject();
    ignore[1] = pObj;
    return !CollisionManager::RayTest(vFrom, vPos, ignore, 2, 0x9e);
}

// FUNCTION: SWBF2 0x00441b10
bool AIUtil::CanSeePosition(const PblVector3& vFrom, Controllable* pMe, const PblVector3& vTo,
                            const GameObject* pIgnore)
{
    const GameObject* ignore[3];
    ignore[0] = pMe->GetGameObject();
    ignore[1] = pIgnore;
    int iIgnoreCount;
    if (pIgnore->GetCollisionType() == COLL_ATTACHED && pIgnore->GetControllable() &&
        pIgnore->GetControllable()->mCharacter && pIgnore->GetControllable()->mCharacter->mVehicle)
    {
        ignore[2] = const_cast<GameObject*>(pIgnore)->GetControllable()->mCharacter->mVehicle->GetGameObject();
        iIgnoreCount = 3;
    }
    else
    {
        iIgnoreCount = 2;
    }
    return !CollisionManager::RayTest(vFrom, vTo, ignore, iIgnoreCount, 0x9e);
}

// FUNCTION: SWBF2 0x00441bd0
bool AIUtil::CanSee(Controllable* pMe, const GameObject* pObj, PblVector3* pPos)
{
    *pPos = GetHisVisionTargetPos(pMe, pObj);
    return CanSeePosition(pMe, *pPos, pObj);
}

// FUNCTION: SWBF2 0x00442b80
void AIUtil::GrenadeAlert(const PblVector3& vPos, const GameObject* pObject, float* pBeginTime,
                          float* pLastEventTime)
{
    const float fReactionTime = AIDifficulty::GetGrenadeReactionTime();
    const float fDelay = fReactionTime + GetRandomFloat(0.0f, 0.3f);
    if (GameLoop::GetMissionTime() > *pBeginTime + fDelay &&
        GameLoop::GetMissionTime() > *pLastEventTime + 1.0f)
    {
        GameObject* objects[1200];
        const int iCount = CollisionManager::GetObjectsInRange(vPos, 7.5f, objects, 1200, pObject, 0x3a);
        bool bSoldierAlerted = false;
        for (int i = 0; i < iCount; ++i)
        {
            GameObject* pTarget = objects[i];
            const PblList<Controllable>& list = pTarget->GetControllableList();
            for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
            {
                if (!it->mCtrl)
                {
                    continue;
                }
                if (!bSoldierAlerted && pTarget->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
                {
                    bSoldierAlerted = true;
                    pTarget->Flee();
                    AI::VOHelper::Play(AI::VOT_Grenade, pTarget, 0, vPos, 0.0f);
                }
                if (UnitController* pUnit = UnitControllerPtr(it->mCtrl))
                {
                    AI::EventInfo info(AI::EVT_Grenade, vPos);
                    pUnit->SendEvent(&info);
                }
            }
        }
        *pLastEventTime = GameLoop::GetMissionTime();
    }
}

// FUNCTION: SWBF2 0x00442d20
bool AIUtil::IsEndGame(int)
{
    return false;
}

// FUNCTION: SWBF2 0x00442d30
bool AIUtil::ResolveCollision(const GameObject* pMe, const GameObject* pOther)
{
    if (pOther->GetEntity()->IsRtti(EntityWalker::rttiHashEntityWalker) ||
        pOther->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover))
    {
        const CollisionObject* pMyCollision = pMe->GetCollisionObject();
        const CollisionObject* pOtherCollision = pOther->GetCollisionObject();
        if (!(pMyCollision->GetMass() > pOtherCollision->GetMass() * 3.0f))
        {
            if (pOtherCollision->GetMass() > pMyCollision->GetMass() * 3.0f)
            {
                return true;
            }

            const PblVector3& vMyVel = pMe->GetVelocity();
            if (vMyVel.x * vMyVel.x + vMyVel.z * vMyVel.z > 0.0001f)
            {
                const PblVector3& vOtherVel = pOther->GetVelocity();
                if (vOtherVel.x * vOtherVel.x + vOtherVel.z * vOtherVel.z > 0.0001f)
                {
                    const PblVector3 vMyDir = GetSafeNormal(pMe->GetVelocity());
                    const PblVector3 vOtherDir = GetSafeNormal(pOther->GetVelocity());
                    PblVector3 vMyPos;
                    vMyPos = pMe->GetMatrix().GetTranslation();
                    PblVector3 vOtherPos;
                    vOtherPos = pOther->GetMatrix().GetTranslation();
                    const PblVector3 vDelta(vOtherPos.x - vMyPos.x, vOtherPos.y - vMyPos.y,
                                            vOtherPos.z - vMyPos.z);
                    const PblVector3 vToOther = GetSafeNormal(vDelta);
                    const float fMyDot = DotProduct(vMyDir, -vToOther);
                    const float fOtherDot = vOtherDir.y * vToOther.y + vOtherDir.x * vToOther.x + vOtherDir.z * vToOther.z;
                    return fOtherDot > fMyDot;
                }
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00442f30
bool AIUtil::IsSniperGuy(const GameObject* pObj)
{
    return pObj->GetGameObjectClass()->GetSoldierType() == GameObjectClass::SOLDIER_SCOUT;
}

// FUNCTION: SWBF2 0x00442f40
bool AIUtil::IsRocketGuy(const GameObject* pObj)
{
    return pObj->GetGameObjectClass()->GetSoldierType() == GameObjectClass::SOLDIER_ROCKET;
}

// FUNCTION: SWBF2 0x00442f50
bool AIUtil::IsHeroGuy(const GameObject*)
{
    return false;
}

// FUNCTION: SWBF2 0x00442f60
bool AIUtil::IsVehicle(const GameObject* pObj)
{
    return pObj->IsRtti(EntityHover::rttiHashEntityHover) ||
           pObj->IsRtti(EntityWalker::rttiHashEntityWalker) ||
           pObj->IsRtti(EntityFlyer::rttiHashEntityFlyer);
}

// FUNCTION: SWBF2 0x00442fa0
const Obstacle* AIUtil::_GetGameObjectBarrier(const GameObject* pObj)
{
    if (!pObj)
    {
        return 0;
    }
    if (pObj->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover))
    {
        return static_cast<const EntityHover*>(pObj->GetEntity())->mObstacle;
    }
    if (pObj->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        return static_cast<const EntityFlyer*>(pObj->GetEntity())->mObstacle;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00443000
Obstacle* AIUtil::GetGameObjectBarrier(GameObject* pObj)
{
    return const_cast<Obstacle*>(_GetGameObjectBarrier(pObj));
}

// FUNCTION: SWBF2 0x00443010
const Obstacle* AIUtil::GetGameObjectBarrier(const GameObject* pObj)
{
    return _GetGameObjectBarrier(pObj);
}

// FUNCTION: SWBF2 0x004430b0
PblVector3 AIUtil::GetRandomAccessiblePoint(const UnitController* pCtrl)
{
    if (!pCtrl || !pCtrl->mOwner)
    {
        return PblVector3(0.0f, 0.0f, 0.0f);
    }

    const GameObject* pObj = static_cast<const Controllable*>(pCtrl->mOwner)->GetGameObject();
    const ConnectivityGraph* pGraph = ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1);
    const ConnectivityNode* pNode;
    if (pGraph && (pNode = pGraph->GetRandomAccessibleNode(pCtrl)) != 0)
    {
        PblVector3 vPoint;
        int iTries = 40;
        do
        {
            vPoint = pNode->GetRandomPointInside(10.0f);
        } while (--iTries > 0 && !PathFinder::IsPointValid(vPoint, 0, pObj));
        return vPoint;
    }

    const PblVector3 vPos = pCtrl->mOwner->GetMatrix().GetTranslation();
    PblVector3 vStart = vPos;
    PathFinder::GetClosestValidPoint(vStart, vStart, pObj);
    PblVector3 vDest = vStart + PblRandom::GetGlobalVector3() * 100.0f;
    vDest.y = vPos.y;
    PathFinder::PushValidPoint(vStart, vDest, pObj);
    return vDest;
}

namespace TeamManager
{
int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
               const Team* pTeam, int iAffilMask); }

// FUNCTION: SWBF2 0x00443220
bool AIUtil::AreEnemiesInRange(int iTeam, const PblVector3& vPos, float fRange)
{
    GameObject* objects[30];
    const int iCount = TeamManager::GetObjectsInRange(vPos, fRange, objects, 30, Team::GetPtr(iTeam), 1);
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObj = objects[i];
        if (pObj->GetControllable() && pObj->GetControllable()->mCtrl)
        {
            return true;
        }
    }
    return false;
}

namespace AIUtil
{

// FUNCTION: SWBF2 0x00443290
float GetThreatSwitchTime(const UnitController* pCtrl)
{
    const Controllable* pMe = pCtrl->mOwner;
    if (pMe)
    {
        const GameObject* pObj = pMe->GetGameObject();
        if (pObj && AIUtil::IsVehicle(pObj) && !pObj->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            return 1.5f;
        }
    }
    return 0.0f;
}
}

// FUNCTION: SWBF2 0x004432e0
void AIUtil::TellAIToExit(GameObject* pObject)
{
    const PblList<Controllable>& list = pObject->GetControllableList();
    for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
    {
        UnitController* pUnit = UnitControllerPtr((Controllable*)it);
        if (!pUnit)
        {
            continue;
        }
        Controllable* pMe = pUnit->mOwner;
        if (pMe && pMe->mCharacter)
        {
            Controllable* pSpeakerUnit = pMe->mCharacter->mUnit;
            if (pSpeakerUnit && !pUnit->IsNewCommand())
            {
                const GameObject* pSpeaker = pSpeakerUnit->GetGameObject();
                AI::VOHelper::Play(AI::VOT_SC_GetOutResponse, pSpeaker, 0, pObject,
                                   AIUtil::GetRandomFloat(0.5f, 1.2f) + GameLoop::GetMissionTime());
            }
        }
        AI::EventInfo info(AI::EVT_SC_GetOutOfVehicle, pObject);
        pUnit->SendEvent(&info);
    }
}

namespace
{
    // FUNCTION: SWBF2 0x00443020
    GameObject* FindFirstInVehicle(GameObject* pVehicle, bool bPlayer)
    {
        const PblList<Controllable>& list = pVehicle->GetControllableList();
        for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
        {
            Controllable* pRider = it;
            if (!pRider->mCtrl || (pRider->mPlayerId >= 0) != bPlayer || !pRider->mCharacter)
            {
                continue;
            }
            Controllable* pUnit = pRider->mCharacter->mUnit;
            if (pUnit)
            {
                return pUnit->GetGameObject();
            }
        }
        return 0;
    }
}

// FUNCTION: SWBF2 0x00443090
GameObject* AIUtil::FindFirstPlayerInVehicle(GameObject* pVehicle)
{
    return FindFirstInVehicle(pVehicle, true);
}

// FUNCTION: SWBF2 0x004430a0
GameObject* AIUtil::FindFirstAIInVehicle(GameObject* pVehicle)
{
    return FindFirstInVehicle(pVehicle, false);
}

namespace ReserveManager
{
struct ReserveStruct;

// GLOBAL: SWBF2 0x01eaeff8
ListPool<ReserveStruct, int>* sList;
}
