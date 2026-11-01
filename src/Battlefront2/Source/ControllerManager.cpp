#include <stdlib.h>

#include "ControllerManager.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "PlayerController.h"
#include "Thread.h"
#include "UnitController.h"
#include "AIUtil.h"
#include "AIVisRegion.h"
#include "Adrenaline.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "GameObject.h"
#include "Team.h"
#include "PathManager.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "AIDifficulty.h"
#include "FlagItem.h"
#include "PlayerProfile.h"
#include "SpaceAssault.h"
#include "SpawnManager.h"
#include "CameraManager.h"
#include "EntityCarrier.h"
#include "WeaponCannonInstance.h"
#include "Damageable.h"
#include "Ordnance.h"
#include "CombatUtil.h"

namespace ControllerManager
{
// GLOBAL: SWBF2 0x01e30930
PblList<PlayerController>* sPlayerCtrls;
// GLOBAL: SWBF2 0x01e30934
PblOrderedList<UnitController, float>* sAICtrls;
// GLOBAL: SWBF2 0x01e30928
int sFrameCount;
// GLOBAL: SWBF2 0x01e3093c
float sPeriodicUpdateTime;
// GLOBAL: SWBF2 0x01e30924
int sHostFrameCount;
// GLOBAL: SWBF2 0x01e30938
float sHostPeriodicUpdateTime;
// GLOBAL: SWBF2 0x01e30920
int sClientFrameCount;
// GLOBAL: SWBF2 0x01e3092c
float sClientPeriodicUpdateTime;
}

namespace ControllerManager
{
    // GLOBAL: SWBF2 0x0078e770
    extern const int UPDATES_PER_SECOND = 2;
}

// GLOBAL: SWBF2 0x01f9c290
PblList<PlayerController>* sHostPlayerCtrls;
// GLOBAL: SWBF2 0x01f9c284
PblOrderedList<UnitController, float>* sHostAICtrls;
// GLOBAL: SWBF2 0x01f9c288
PblList<PlayerController>* sClientPlayerCtrls;
// GLOBAL: SWBF2 0x01f9c28c
PblOrderedList<UnitController, float>* sClientAICtrls;

// FUNCTION: SWBF2 0x00485b30
void ControllerManager::EnterClient()
{
    sPlayerCtrls = sClientPlayerCtrls;
    sAICtrls = sClientAICtrls;
    sHostPeriodicUpdateTime = sPeriodicUpdateTime;
    sHostFrameCount = sFrameCount;
    sFrameCount = sClientFrameCount;
    sPeriodicUpdateTime = sClientPeriodicUpdateTime;
}

// FUNCTION: SWBF2 0x00485b80
void ControllerManager::LeaveClient()
{
    sPlayerCtrls = sHostPlayerCtrls;
    sAICtrls = sHostAICtrls;
    sClientPeriodicUpdateTime = sPeriodicUpdateTime;
    sClientFrameCount = sFrameCount;
    sFrameCount = sHostFrameCount;
    sPeriodicUpdateTime = sHostPeriodicUpdateTime;
}

// FUNCTION: SWBF2 0x00485bd0
void ControllerManager::Init()
{
    sPlayerCtrls = new PblList<PlayerController>;
    sAICtrls = new PblOrderedList<UnitController, float>;
    sFrameCount = 0;
    sPeriodicUpdateTime = 0.0f;
    sHostPlayerCtrls = sPlayerCtrls;
    sHostAICtrls = sAICtrls;
    EnterClient();

    sPlayerCtrls = new PblList<PlayerController>;
    sAICtrls = new PblOrderedList<UnitController, float>;
    sClientPlayerCtrls = sPlayerCtrls;
    sClientAICtrls = sAICtrls;
    LeaveClient();
}

// FUNCTION: SWBF2 0x00485ce0
void ControllerManager::Reset()
{
    sPlayerCtrls = 0;
    sAICtrls = 0;
}

// FUNCTION: SWBF2 0x00485d00
void ControllerManager::AddController(Controller* pCtrl)
{
    if (pCtrl->IsRtti(PlayerController::rttiHashPlayerController))
    {
        PlayerController* pPlayer = (PlayerController*)pCtrl;
        sPlayerCtrls->AddLast(pPlayer->GetManagerNode(), pPlayer);
    }
    else if (pCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        UnitController* pUnit = (UnitController*)pCtrl;
        sAICtrls->Add(pUnit->GetManagerNode(), pUnit, 0.0f);
    }
}

// FUNCTION: SWBF2 0x00485d60
void ControllerManager::RemoveController(Controller* pCtrl)
{
    if (pCtrl->IsRtti(PlayerController::rttiHashPlayerController))
    {
        PlayerController* pPlayer = (PlayerController*)pCtrl;
        sPlayerCtrls->Remove(pPlayer->GetManagerNode());
    }
    else if (pCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        sAICtrls->Remove(((UnitController*)pCtrl)->GetManagerNode());
    }
}

// FUNCTION: SWBF2 0x004862f0
void ControllerManager::RequestControllerUpdateASAP(UnitController* pCtrl)
{
    const float fScheduledTime = pCtrl->mNextUpdateTime;
    const float fSoon = GameLoop::GetMissionTime() + 0.0001f;
    pCtrl->mNextUpdateTime = (fSoon > fScheduledTime) ? fScheduledTime : fSoon;

    PblOrderedList<UnitController, float>::Node* pNode = pCtrl->GetManagerNode();
    sAICtrls->Remove(pNode);
    sAICtrls->Add(pNode, pCtrl, pCtrl->mNextUpdateTime);
}

// FUNCTION: SWBF2 0x004865a0
void ControllerManager::BroadcastEvent(AI::EventInfo* pInfo, int iMaxCount)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }

    PblOrderedList<UnitController, float>::Itor it(*sAICtrls);
    while (UnitController* pCtrl = it)
    {
        ++it;
        if (pCtrl->SendEvent(pInfo) == 1 && --iMaxCount <= 0)
        {
            break;
        }
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

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

namespace VehicleManager
{

void Update();
}

namespace TeamManager
{

int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);
}

namespace AI
{
namespace VOHelper
{
void Update();
}
}
namespace AIDifficulty
{
void UpdateAutoBalance();
}
namespace VisionManager
{
bool RayTests(int i);
}

namespace AI
{
enum VOType
{
    VOT_NONE = 0
};

namespace VOHelper
{
struct QueuedSound
{
    VOType mType;
    PblHandle<const GameObject> mSpeaker;
    PblHandle<const GameObject> mPlayer;
    PblHandle<const GameObject> mTarget;
    PblVector3 mPos;
    bool mUseTarget;
    float mTime;
    int mHandle;

    ~QueuedSound();
};
extern QueuedSound mSndQueue[20];

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const PblVector3& vPos, float fDelay);
}
}

namespace ControllerManager
{
int GetPlayersInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const Team* pTeam, AI::AIAffil eAffil);
int GetAIInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                 int iMax, const Team* pTeam, AI::AIAffil eAffil);
int GetUnitsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                    int iMax, const Team* pTeam, AI::AIAffil eAffil);
void BroadcastEvent(AI::EventInfo* pInfo, const PblVector3& vPos, float fRadius,
                    const UnitController* pSender, const Team* pTeam, AI::AIAffil eAffil);

template <class T> void UpdateControllerAudibles(T* pCtrl);
}

namespace AudibleManager
{
void UpdateAudience(Controller* pController);
}

namespace ControllerManager
{
// FUNCTION: SWBF2 0x00485e00
static void sUpdateFriendlyFire()
{
    if (sPlayerCtrls->GetCount() > 8)
    {
        return;
    }

    for (PblOrderedList<UnitController, float>::Itor it(*sAICtrls); it; ++it)
    {
        it->ResetFriendlyList();
    }

    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); (PlayerController*)it; ++it)
    {
        Controllable* pOwner = it->mOwner;
        if (!pOwner || !pOwner->GetGameObject())
        {
            continue;
        }

        GameObject* pPlayer = it->mOwner->GetGameObject();
        GameObject* apObjects[20];
        int iCount = TeamManager::GetObjectsInRange(
            pPlayer->GetMatrix().GetTranslation(),
            50.0f, apObjects, 20, pPlayer, AI::AFFIL_ALLYNEUTRAL);
        for (int i = 0; i < iCount; ++i)
        {
            GameObject* pObject = apObjects[i];
            if (!pObject->GetControllable() || !pObject->GetControllable()->mCtrl ||
                !pObject->GetControllable()->mCtrl->IsRtti(UnitController::rttiHashUnitController))
            {
                continue;
            }

            for (PblList<Controllable>::Itor itC(pObject->GetControllableList()); (Controllable*)itC; ++itC)
            {
                if (itC->mCtrl && itC->mCtrl->IsRtti(UnitController::rttiHashUnitController))
                {
                    ((UnitController*)itC->mCtrl)->AddFriendlyFire(pPlayer);
                }
            }
        }
    }
}
}

// FUNCTION: SWBF2 0x00485f60
void ControllerManager::UpdatePilotSelf()
{
    for (PblOrderedList<UnitController, float>::Itor it(*sAICtrls); it; ++it)
    {
        Controllable* pOwner = it->mOwner;
        if (!pOwner)
        {
            continue;
        }
        if (!pOwner->IsSelfPilotable() || pOwner->GetPilot())
        {
            continue;
        }
        if (pOwner->mCharacter || it->GetCurCmdType() || it->IsNewCommand())
        {
            continue;
        }

        UnitCmdInfo cmd(CMD_DEATHMATCH, NULL, PblVector3(0.0f, 0.0f, 0.0f), NULL);
        it->QueueCommand(cmd);
    }
}

// FUNCTION: SWBF2 0x00486040
void ControllerManager::UpdateCrowd(Controller* pCtrl)
{
    if (GameLoop::GetMissionTime() < pCtrl->mCrowdTestTimer + 1.0f)
    {
        return;
    }

    Controllable* pOwner = pCtrl->mOwner;
    pCtrl->mCrowdEnemies = 0;
    pCtrl->mCrowdAllies = 0;
    if (!pOwner || !pOwner->GetGameObject())
    {
        return;
    }

    const Team* pTeam = Team::sTeams[pOwner->GetGameObject()->GetTeam()];
    const PblVector3& vPos = pOwner->GetMatrix().GetTranslation();

    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); it; ++it)
    {
        Controllable* pOtherOwner = it->mOwner;
        if (it == pCtrl || !pOtherOwner || !pOtherOwner->GetGameObject() ||
            !pOtherOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            continue;
        }
        if (SquaredLength(pOtherOwner->GetMatrix().GetTranslation() - vPos) < 81.0f)
        {
            if (pTeam->GetAffiliation(pOtherOwner->GetGameObject()->GetTeam()) < 0)
            {
                ++pCtrl->mCrowdEnemies;
            }
            else
            {
                ++pCtrl->mCrowdAllies;
            }
        }
    }

    for (PblOrderedList<UnitController, float>::Itor it(*sAICtrls); it; ++it)
    {
        Controllable* pOtherOwner = it->mOwner;
        if (it == pCtrl || !pOtherOwner || !pOtherOwner->GetGameObject() ||
            !pOtherOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            continue;
        }
        if (SquaredLength(pOtherOwner->GetMatrix().GetTranslation() - vPos) < 81.0f)
        {
            if (pTeam->GetAffiliation(pOtherOwner->GetGameObject()->GetTeam()) < 0)
            {
                ++pCtrl->mCrowdEnemies;
            }
            else
            {
                ++pCtrl->mCrowdAllies;
            }
        }
    }
}

namespace AIDifficulty
{
float SpaceAssault_GetFlyerKillForPlayerTeam();
}
namespace AIDifficulty
{
float SpaceAssault_GetCritSysKillForPlayerTeam();
}

// FUNCTION: SWBF2 0x00486270
void ControllerManager::DoPeriodicUpdate(float fDeltaTime)
{
    UpdatePilotSelf();
    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); it; ++it)
    {
        UpdateControllerAudibles<PlayerController>(it);
        it->UpdateFreeSpace();
        UpdateCrowd(it);
        Controllable* pOwner = it->mOwner;
        Character* pCharacter = pOwner->mCharacter;
        pCharacter->mAdrenaline.Update(fDeltaTime, pOwner->GetGameObject());
    }
    sUpdateFriendlyFire();
    AIDifficulty::UpdateAutoBalance();
}

// FUNCTION: SWBF2 0x00486370
void ControllerManager::Update(float fDeltaTime)
{
    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); PlayerController* pCtrl = it; )
    {
        ++it;
        pCtrl->Update(fDeltaTime);
    }

    AI::VOHelper::Update();

    if (++sFrameCount > 15)
    {
        sFrameCount = 1;
        float fTime = GameLoop::GetMissionTime();
        DoPeriodicUpdate(fTime - sPeriodicUpdateTime);
        sPeriodicUpdateTime = fTime;
    }

    VehicleManager::Update();
    AIVisRegionManager::UpdateNextRegion();

    int iMaxUpdates = AIUtil::IsUberMode() ? 100 : 10;
    float fTime = GameLoop::GetMissionTime();
    for (int i = 0; i < iMaxUpdates; ++i)
    {
        UnitController* pCtrl = sAICtrls->Begin();
        if (!pCtrl || pCtrl->GetUpdateTime() > fTime)
        {
            break;
        }
        pCtrl->UpdateHighLevel();
        UpdateCrowd(pCtrl);
        PblOrderedList<UnitController, float>::Node* pNode = pCtrl->GetManagerNode();
        sAICtrls->Remove(pNode);
        sAICtrls->Add(pNode, pCtrl, pCtrl->mNextUpdateTime);
    }

    for (PblOrderedList<UnitController, float>::Itor it(*sAICtrls); UnitController* pCtrl = it; )
    {
        ++it;
        pCtrl->UpdateLowLevel(fDeltaTime);
    }

    int iScale = 1;
    if (!PathManager::IsDone())
    {
        if (AIUtil::IsUberMode())
        {
            PathManager::Update(10.0f);
        }
        else
        {
            PathManager::Update(1.0f);
        }
    }
    else
    {
        iScale = 2;
    }
    if (!VisionManager::RayTests((AIUtil::IsUberMode() ? 50 : 10) * iScale))
    {
        PathManager::Update(1.0f);
    }
}

// FUNCTION: SWBF2 0x00486730
void ControllerManager::LogVODeath(int iVictimTeam, int iKillerTeam, const PblVector3& vPos)
{
    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); (PlayerController*)it; ++it)
    {
        PlayerController* pPlayer = it;
        Controllable* pOwner = pPlayer->mOwner;
        if (pOwner && SquaredLength(vPos - pOwner->GetMatrix().GetTranslation()) < 625.0f)
        {
            pPlayer->LogVODeath(iVictimTeam, iKillerTeam);
        }
    }
}

// FUNCTION: SWBF2 0x004867c0
int ControllerManager::GetPlayersInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                                         int iMax, const Team* pTeam, AI::AIAffil eAffil)
{
    if (iMax <= 0)
    {
        return 0;
    }

    float fRadiusSq = fRadius * fRadius;
    int iCount = 0;
    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); (PlayerController*)it; ++it)
    {
        Controllable* pOwner = it->mOwner;
        if (!pOwner)
        {
            continue;
        }
        GameObject* pObject = pOwner->GetGameObject();
        if (!pObject)
        {
            continue;
        }
        int iAffil = pTeam ? pTeam->GetAffiliation(pObject->GetPerceivedTeam()) : 0;
        if (!(eAffil & (1 << (iAffil + 1))))
        {
            continue;
        }
        const PblVector3& vObject = pObject->GetPosition();
        if (SquaredLength(vPos - vObject) < fRadiusSq)
        {
            ppObjects[iCount++] = pObject;
            if (iCount >= iMax)
            {
                break;
            }
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x004868c0
int ControllerManager::GetAIInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                                    int iMax, const Team* pTeam, AI::AIAffil eAffil)
{
    if (iMax <= 0)
    {
        return 0;
    }

    float fRadiusSq = fRadius * fRadius;
    int iCount = 0;
    for (PblOrderedList<UnitController, float>::Itor it(*sAICtrls); it; ++it)
    {
        Controllable* pOwner = it->mOwner;
        if (!pOwner)
        {
            continue;
        }
        GameObject* pObject = pOwner->GetGameObject();
        if (!pObject)
        {
            continue;
        }
        int iAffil = pTeam ? pTeam->GetAffiliation(pObject->GetPerceivedTeam()) : 0;
        if (!(eAffil & (1 << (iAffil + 1))))
        {
            continue;
        }
        const PblVector3& vObject = pObject->GetPosition();
        if (SquaredLength(vPos - vObject) < fRadiusSq)
        {
            ppObjects[iCount++] = pObject;
            if (iCount >= iMax)
            {
                break;
            }
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x004869c0
int ControllerManager::GetUnitsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                                       int iMax, const Team* pTeam, AI::AIAffil eAffil)
{
    int iPlayers = GetPlayersInRange(vPos, fRadius, ppObjects, iMax, pTeam, eAffil);
    return iPlayers + GetAIInRange(vPos, fRadius, ppObjects + iPlayers, iMax - iPlayers, pTeam, eAffil);
}

// FUNCTION: SWBF2 0x00486610
void ControllerManager::BroadcastEvent(AI::EventInfo* pInfo, const PblVector3& vPos, float fRadius,
                                      const UnitController* pSender, const Team* pTeam, AI::AIAffil eAffil)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }

    float fRadiusSq = fRadius * fRadius;
    PblOrderedList<UnitController, float>::Itor it(*sAICtrls);
    while (UnitController* pCtrl = it)
    {
        ++it;
        if (!pCtrl->mOwner || !pCtrl->mOwner->GetGameObject())
        {
            continue;
        }
        GameObject* pObject = pCtrl->mOwner->GetGameObject();
        if (pTeam && !(eAffil & (1 << (pTeam->GetAffiliation(pObject->GetTeam()) + 1))))
        {
            continue;
        }
        if (!pCtrl->mOwner || pCtrl == pSender)
        {
            continue;
        }
        if (SquaredLength(pCtrl->mOwner->GetMatrix().GetTranslation() - vPos) < fRadiusSq)
        {
            pCtrl->SendEvent(pInfo);
        }
    }
}

// TEMPLATE: SWBF2 0x00486a10
// PblOrderedList<UnitController,float>::Add
template void PblOrderedList<UnitController, float>::Add(PblOrderedList<UnitController, float>::Node*, UnitController*, float);

namespace ControllerManager
{
template <class T> void UpdateControllerAudibles(T* pCtrl)
{
    AudibleManager::UpdateAudience(pCtrl);
}

// TEMPLATE: SWBF2 0x00486a80
// ControllerManager::UpdateControllerAudibles<PlayerController>
template void UpdateControllerAudibles<PlayerController>(PlayerController* pCtrl);
}

namespace AI
{
namespace VOHelper
{

// FUNCTION: SWBF2 0x00672640
void Update()
{
    using namespace AI::VOHelper;

    const float fTime = GameLoop::GetMissionTime();
    for (int i = 0; i < 20; ++i)
    {
        QueuedSound& rSound = mSndQueue[i];
        if (rSound.mType == AI::VOT_NONE || !(fTime > rSound.mTime))
        {
            continue;
        }
        if (rSound.mUseTarget)
        {
            Play(rSound.mType, rSound.mSpeaker, rSound.mPlayer, rSound.mTarget, 0.0f);
        }
        else
        {
            Play(rSound.mType, rSound.mSpeaker, rSound.mPlayer, rSound.mPos, 0.0f);
        }
        rSound.mType = AI::VOT_NONE;
    }
}
}
}
