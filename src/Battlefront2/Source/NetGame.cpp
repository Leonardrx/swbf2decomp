#include "AmmoCounter.h"
#define _CRT_SECURE_NO_WARNINGS

#include "GameVoiceChat.h"
#include "GameVoiceChatNode.h"
#include "PlayerController.h"
#include "RedEngineFL/Util/RedSceneObject.h"
#include <stdio.h>
#include <new>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <windows.h>
#include <d3dx9math.h>
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblHeap.h"
#include "RedEngineFL/Graphics/RedColor.h"

#include "NetGame.h"
#include "PebbleFL/Common/PblAngle.h"
#include "CameraManager.h"
#include "pcNetSocket.h"
#include "LockOnManager.h"
#include "EnergyBar.h"
#include "VoiceChatTransport.h"
#include "CollisionModel.h"
#include "CollisionObject.h"
#include "CommandLine.h"
#include "ControllerManager.h"
#include "EntityDoor.h"
#include "EntityDroideka.h"
#include "Steam/galaxy_api.h"
#include "FLJoystick.h"
#include "NetComm.h"
#include "NetPacket.h"
#include "TreeGrid.h"
#include "EntityItem.h"
#include "PowerupItem.h"
#include "WeaponDetonator.h"
#include "WeaponCannonInstance.h"
#include "Aimer.h"
#include "CollisionPrimitive.h"
#include "NetSetup.h"
#include "ShellLoop.h"
#include "HeroMessageDisplay.h"
#include "HUDObjectiveList.h"
#include "HUDGameEvents.h"
#include "VoiceOverManager.h"
#include "GameState.h"
#include "PlayerStats.h"
#include "Team.h"
#include "RedEngineFL/Util/RedConsole.h"
#include "Thread.h"
#include "NetMatch.h"
#include "Tracker.h"
#include "Trackable.h"
#include "MissionPlayList.h"
#include "GameLoop.h"
#include "GameMemory.h"
#include "GameObject.h"
#include "Factory.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "CaptureDisplay.h"
#include "MissionTimer.h"
#include "Timer.h"
#include "TargetManager.h"
#include "SpaceAssault.h"
#include "EntityBuilding.h"
#include "FirstPerson.h"
#include "Chunk.h"
#include "EntityGeometry.h"
#include "FlagItem.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "NetPredict.h"
#include "TreeGridCache.h"
#include "FLInputManager.h"
#include "MessageDisplay.h"
#include "TeamAssist.h"
#include "MedalsMgr.h"
#include "VoiceChatNode.h"
#include "BootDisplay.h"
#include "Weapon.h"
#include "WeaponTowCable.h"
#include "WeaponMeleeThrow.h"
#include "Ordnance.h"
#include "PauseMenu.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "LuaHelper.h"
#include "PlayerProfile.h"
#include "FLGuiManager.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "AIUtil.h"
#include "Asteroid.h"
#include "CommandFlyer.h"
#include "CommandHover.h"
#include "CommandWalker.h"
#include "EntityWalker.h"
#include "LuaCallbacks_Options.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "GameSoundEngine.h"
#include "SoundFL/source/sndfile.h"
#include "SoundFL/source/sndglobals.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "RedEngineFL/Util/RedCommandConsole.h"
#include "PebbleFL/Common/PblJournal.h"
#include "PebbleFL/Common/PblValues.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblTime.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "TeamManager.h"
#include "EntityBuildingArmed.h"
#include "EntityBuildingAnimatedArmed.h"
#include "EntityRemoteTerminal.h"

#include "Character.h"

#include <float.h>
#include <limits.h>
#include "EntityPropAnimated.h"
#include "EntityWater.h"

inline void NetEventVector::Set(const PblVector3& value)
{
    x = value.GetX();
    y = value.GetY();
    z = value.GetZ();
}

inline PblVector3 NetEventVector::Get()
{
    return PblVector3(x, y, z);
}

inline void NetGameObjectHandle::Set(const GameObject* object)
{
    ptr = object;
    if (object)
    {
        id = object->GetHandleId();
    }
    else
    {
        id = 0;
    }
}

inline void NetGameObjectHandle::Get(const GameObject*& object)
{
    if (ptr && ptr->GetHandleId() != id)
    {
        ptr = 0;
        id = 0;
    }
    object = ptr;
}

namespace gf
{
bool isInitialized(); bool checkError();
}
 

class NetPktGroup;

namespace StatsManager
{
    void WriteStatsChallenge(int iPlayer, NetPktGroup& group);
    void WriteStatsResponse(int iLocal, NetPktGroup& group);
}

 
 
 
 
// GLOBAL: SWBF2 0x01fa3d88
NetEvent sServerNetEvents[128];

// GLOBAL: SWBF2 0x007e8007
// netInShell
bool netInShell = true;
// GLOBAL: SWBF2 0x01e62ea8
// netEnabled
bool netEnabled;
// GLOBAL: SWBF2 0x01e62ea9
// netEnabledNext
bool netEnabledNext;

// GLOBAL: SWBF2 0x007e802f
bool netTeamDamage = true;

// GLOBAL: SWBF2 0x01e62f58
int netCurMaxPlayers;
// GLOBAL: SWBF2 0x01e62f54
int netCurMaxHosts;
// GLOBAL: SWBF2 0x01e62f0c
// netHostIndex
int netHostIndex;
// GLOBAL: SWBF2 0x01e62f40
int journalMode;
// GLOBAL: SWBF2 0x01e62f91
bool netIsSplit;
// GLOBAL: SWBF2 0x01fa8788
// netPreGameEnabled
bool netPreGameEnabled;
// GLOBAL: SWBF2 0x01fa8784
float netPreGameStartTime;

// GLOBAL: SWBF2 0x01e62f51
bool netClientCreated;
// GLOBAL: SWBF2 0x01fa65b8
bool cockpitCams[64];
// GLOBAL: SWBF2 0x01e64b58
bool netHeroSndPlayHostUnlock[3];
// GLOBAL: SWBF2 0x01e64cd4
bool netHeroSndPlayHostSelectable[3];
// GLOBAL: SWBF2 0x01e64b8c
bool netHeroSndPlayHostSpawn[3];
// GLOBAL: SWBF2 0x01e64b9c
bool netHeroSndPlayHostDefeat[3];
// GLOBAL: SWBF2 0x01e62fb8
float netHeroUnlockStartTimer;
// GLOBAL: SWBF2 0x01fa6590
float singleTime;
// GLOBAL: SWBF2 0x01e64ba4
unsigned int clientTurnNumber;
// GLOBAL: SWBF2 0x01fa87b8
float turnRatio;
// GLOBAL: SWBF2 0x01e62eac
float netSecondsPerTurn;

// GLOBAL: SWBF2 0x01e62f44
int netGameCount;

// SYNTHETIC: SWBF2 0x00404830
// `dynamic initializer for 'netPlayers''

// GLOBAL: SWBF2 0x01ecdaa0
// netPlayers
NetPlayer netPlayers[64];
// GLOBAL: SWBF2 0x01fa8858
NetHost netHosts[3];

// GLOBAL: SWBF2 0x01fa8850
SwitchResponse clientSwitchObjects;

// GLOBAL: SWBF2 0x01e64cd0
NetObjStateMap* NetObjStateMap::topFree;

// GLOBAL: SWBF2 0x01e62f90
bool netSetForceHumansToTeam1;

class HostStats
{
public:
    class Player
    {
    public:
        void Init()
        {
            lastUpdateCount = 0;
            lastFilledCount = 0;
        }

        unsigned int lastUpdateCount;
        unsigned int lastFilledCount;
    };

    void Init()
    {
        nextTime = 0.0f;
    }

    Player players[64];
    float nextTime;
};

// GLOBAL: SWBF2 0x01e63018
static HostStats hostStats;

// GLOBAL: SWBF2 0x01fa871c
int clientBootPlayer;
// GLOBAL: SWBF2 0x01fa87b0
int hostBootPlayer;
// GLOBAL: SWBF2 0x01fa3d84
int hostForceBootPlayer;

enum BootRequest
{
    BR_NOMINATE = 0,
    BR_AFFIRM = 1
};

struct BootInfo
{
    unsigned int changedTurn;
    int dst;
    int reason;
    int joystick;
    BootRequest bootRequest;
};

// GLOBAL: SWBF2 0x01e64b6c
BootInfo bootInfo;
// GLOBAL: SWBF2 0x01fa878c
int netEventHead;
// GLOBAL: SWBF2 0x01fa884c
unsigned int turnHighBits;
// GLOBAL: SWBF2 0x01fa6598
unsigned __int64 hostPlayingMask;
// GLOBAL: SWBF2 0x01fa87a8
unsigned __int64 clientPlayingMask;
// GLOBAL: SWBF2 0x01e62eab
// netClientActive
bool netClientActive;

// GLOBAL: SWBF2 0x007e802c
bool netIsOneWorld = true;
// GLOBAL: SWBF2 0x01e62eaa
bool netIsDedicated;

// SYNTHETIC: SWBF2 0x00405550
// `dynamic initializer for 'netGameChatMsgClient''

// GLOBAL: SWBF2 0x01ee7614
// netGameChatMsgClient
PblList<NG_ChatMsg> netGameChatMsgClient;
// SYNTHETIC: SWBF2 0x00405560
// `dynamic initializer for 'netGameChatMsgServer''

// GLOBAL: SWBF2 0x01ecb25c
// netGameChatMsgServer
PblList<NG_ChatMsg> netGameChatMsgServer;

// TEMPLATE: SWBF2 0x005d3a60
// PblList<NG_ChatMsg>::PblList<NG_ChatMsg>

// GLOBAL: SWBF2 0x01e64efd
bool isJournalOpened;

// SYNTHETIC: SWBF2 0x00404880
// `dynamic initializer for 'NetObj::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768e10
// `dynamic atexit destructor for 'NetObj::sMemoryPool''

// GLOBAL: SWBF2 0x01ee7490
// NetObj::sMemoryPool
MemoryPool NetObj::sMemoryPool("NetObj", 0xC);

// SYNTHETIC: SWBF2 0x004048e0
// `dynamic initializer for 'clientSoldierPool''
// SYNTHETIC: SWBF2 0x00768e30
// `dynamic atexit destructor for 'clientSoldierPool''

// GLOBAL: SWBF2 0x01ee74e0
// clientSoldierPool
MemoryPool clientSoldierPool("ClientSoldier", 0);

// SYNTHETIC: SWBF2 0x00404910
// `dynamic initializer for 'clientDroidPool''
// SYNTHETIC: SWBF2 0x00768e40
// `dynamic atexit destructor for 'clientDroidPool''

// GLOBAL: SWBF2 0x01ed77c8
// clientDroidPool
MemoryPool clientDroidPool("ClientDroid", 0);

// SYNTHETIC: SWBF2 0x00404940
// `dynamic initializer for 'clientWalkerPool''
// SYNTHETIC: SWBF2 0x00768e50
// `dynamic atexit destructor for 'clientWalkerPool''

// GLOBAL: SWBF2 0x01ee5360
// clientWalkerPool
MemoryPool clientWalkerPool("ClientWalker", 0);

// SYNTHETIC: SWBF2 0x00404970
// `dynamic initializer for 'clientCommandWalkerPool''
// SYNTHETIC: SWBF2 0x00768e60
// `dynamic atexit destructor for 'clientCommandWalkerPool''

// GLOBAL: SWBF2 0x01ed70a0
// clientCommandWalkerPool
MemoryPool clientCommandWalkerPool("ClientCommandWalker", 0);

// SYNTHETIC: SWBF2 0x004049a0
// `dynamic initializer for 'clientCommandFlyerPool''
// SYNTHETIC: SWBF2 0x00768e70
// `dynamic atexit destructor for 'clientCommandFlyerPool''

// GLOBAL: SWBF2 0x01ed8890
// clientCommandFlyerPool
MemoryPool clientCommandFlyerPool("ClientCommandFlyer", 0);

// SYNTHETIC: SWBF2 0x004049d0
// `dynamic initializer for 'clientDroidekaPool''
// SYNTHETIC: SWBF2 0x00768e80
// `dynamic atexit destructor for 'clientDroidekaPool''

// GLOBAL: SWBF2 0x01ed86f8
// clientDroidekaPool
MemoryPool clientDroidekaPool("ClientDroideka", 0);

// SYNTHETIC: SWBF2 0x00404a00
// `dynamic initializer for 'clientFlyerPool''
// SYNTHETIC: SWBF2 0x00768e90
// `dynamic atexit destructor for 'clientFlyerPool''

// GLOBAL: SWBF2 0x01ee5480
// clientFlyerPool
MemoryPool clientFlyerPool("ClientFlyer", 0);

// SYNTHETIC: SWBF2 0x00404a30
// `dynamic initializer for 'clientCarrierPool''
// SYNTHETIC: SWBF2 0x00768ea0
// `dynamic atexit destructor for 'clientCarrierPool''

// GLOBAL: SWBF2 0x01ed5ca0
// clientCarrierPool
MemoryPool clientCarrierPool("ClientCarrier", 0);

// SYNTHETIC: SWBF2 0x00404a60
// `dynamic initializer for 'clientHoverPool''
// SYNTHETIC: SWBF2 0x00768eb0
// `dynamic atexit destructor for 'clientHoverPool''

// GLOBAL: SWBF2 0x01ee7730
// clientHoverPool
MemoryPool clientHoverPool("ClientHover", 0);

// SYNTHETIC: SWBF2 0x00404a90
// `dynamic initializer for 'clientCommandHoverPool''
// SYNTHETIC: SWBF2 0x00768ec0
// `dynamic atexit destructor for 'clientCommandHoverPool''

// GLOBAL: SWBF2 0x01ed7848
// clientCommandHoverPool
MemoryPool clientCommandHoverPool("ClientCommandHover", 0);

// SYNTHETIC: SWBF2 0x00404ac0
// `dynamic initializer for 'clientMountedTurretPool''
// SYNTHETIC: SWBF2 0x00768ed0
// `dynamic atexit destructor for 'clientMountedTurretPool''

// GLOBAL: SWBF2 0x01ed7778
// clientMountedTurretPool
MemoryPool clientMountedTurretPool("ClientMountedTurret", 0);

// SYNTHETIC: SWBF2 0x00404af0
// `dynamic initializer for 'clientPowerupPool''
// SYNTHETIC: SWBF2 0x00768ee0
// `dynamic atexit destructor for 'clientPowerupPool''

// GLOBAL: SWBF2 0x01ee73a0
// clientPowerupPool
MemoryPool clientPowerupPool("ClientPowerup", 0);

// SYNTHETIC: SWBF2 0x00404b20
// `dynamic initializer for 'clientArmedDynamicPool''
// SYNTHETIC: SWBF2 0x00768ef0
// `dynamic atexit destructor for 'clientArmedDynamicPool''

// GLOBAL: SWBF2 0x01ed86a8
// clientArmedDynamicPool
MemoryPool clientArmedDynamicPool("ClientArmedDynamic", 0);

// SYNTHETIC: SWBF2 0x00404b50
// `dynamic initializer for 'clientWalkerLegPairPool''
// SYNTHETIC: SWBF2 0x00768f00
// `dynamic atexit destructor for 'clientWalkerLegPairPool''

// GLOBAL: SWBF2 0x01ee4aa0
// clientWalkerLegPairPool
MemoryPool clientWalkerLegPairPool("ClientWalkerLegPair", 0);

// SYNTHETIC: SWBF2 0x00404b80
// `dynamic initializer for 'clientWalkerBlendPool''
// SYNTHETIC: SWBF2 0x00768f10
// `dynamic atexit destructor for 'clientWalkerBlendPool''

// GLOBAL: SWBF2 0x01ee53b0
// clientWalkerBlendPool
MemoryPool clientWalkerBlendPool("ClientWalkerBlend", 0);

// SYNTHETIC: SWBF2 0x00404bb0
// `dynamic initializer for 'clientWalkerBlendUnitPool''
// SYNTHETIC: SWBF2 0x00768f20
// `dynamic atexit destructor for 'clientWalkerBlendUnitPool''

// GLOBAL: SWBF2 0x01ed7000
// clientWalkerBlendUnitPool
MemoryPool clientWalkerBlendUnitPool("ClientWalkerBlendUnit", 0);

// SYNTHETIC: SWBF2 0x00404be0
// `dynamic initializer for 'clientAimerPool''
// SYNTHETIC: SWBF2 0x00768f30
// `dynamic atexit destructor for 'clientAimerPool''

// GLOBAL: SWBF2 0x01ee5140
// clientAimerPool
MemoryPool clientAimerPool("ClientAimer", 0);

// SYNTHETIC: SWBF2 0x00404c10
// `dynamic initializer for 'clientWeaponPool''
// SYNTHETIC: SWBF2 0x00768f40
// `dynamic atexit destructor for 'clientWeaponPool''

// GLOBAL: SWBF2 0x01ed8840
// clientWeaponPool
MemoryPool clientWeaponPool("ClientWeapon", 0);

// SYNTHETIC: SWBF2 0x00404c40
// `dynamic initializer for 'clientAmmoCounterPool''
// SYNTHETIC: SWBF2 0x00768f50
// `dynamic atexit destructor for 'clientAmmoCounterPool''

// GLOBAL: SWBF2 0x01ed6f60
// clientAmmoCounterPool
MemoryPool clientAmmoCounterPool("ClientAmmoCounter", 0);

// SYNTHETIC: SWBF2 0x00404c70
// `dynamic initializer for 'clientEnergyBarPool''
// SYNTHETIC: SWBF2 0x00768f60
// `dynamic atexit destructor for 'clientEnergyBarPool''

// GLOBAL: SWBF2 0x01ee7560
// clientEnergyBarPool
MemoryPool clientEnergyBarPool("ClientEnergyBar", 0);

// SYNTHETIC: SWBF2 0x00404ca0
// `dynamic initializer for 'clientTreeGridPool''
// SYNTHETIC: SWBF2 0x00768f70
// `dynamic atexit destructor for 'clientTreeGridPool''

// GLOBAL: SWBF2 0x01ee7678
// clientTreeGridPool
MemoryPool clientTreeGridPool("ClientTreeGrid", 0);

// SYNTHETIC: SWBF2 0x00404cd0
// `dynamic initializer for 'clientOrdnanceTowCablePool''
// SYNTHETIC: SWBF2 0x00768f80
// `dynamic atexit destructor for 'clientOrdnanceTowCablePool''

// GLOBAL: SWBF2 0x01ee7628
// clientOrdnanceTowCablePool
MemoryPool clientOrdnanceTowCablePool("ClientOrdnanceTowCable", 0);

// SYNTHETIC: SWBF2 0x00404d00
// `dynamic initializer for 'clientAsteroidPool''
// SYNTHETIC: SWBF2 0x00768f90
// `dynamic atexit destructor for 'clientAsteroidPool''

// GLOBAL: SWBF2 0x01ee7ba8
// clientAsteroidPool
MemoryPool clientAsteroidPool("ClientAsteroid", 0);

// SYNTHETIC: SWBF2 0x00404d30
// `dynamic initializer for 'clientFlagPool''
// SYNTHETIC: SWBF2 0x00768fa0
// `dynamic atexit destructor for 'clientFlagPool''

// GLOBAL: SWBF2 0x01ee4a50
// clientFlagPool
MemoryPool clientFlagPool("ClientFlag", 0);

// SYNTHETIC: SWBF2 0x00404d60
// `dynamic initializer for 'clientRemoteTerminalPool''
// SYNTHETIC: SWBF2 0x00768fb0
// `dynamic atexit destructor for 'clientRemoteTerminalPool''

// GLOBAL: SWBF2 0x01ee73f0
// clientRemoteTerminalPool
MemoryPool clientRemoteTerminalPool("ClientRemoteTerminal", 0);

// SYNTHETIC: SWBF2 0x00404d90
// `dynamic initializer for 'hostSoldierPool''
// SYNTHETIC: SWBF2 0x00768fc0
// `dynamic atexit destructor for 'hostSoldierPool''

// GLOBAL: SWBF2 0x01ee7aa0
// hostSoldierPool
MemoryPool hostSoldierPool("HostSoldier", 0);

// SYNTHETIC: SWBF2 0x00404dc0
// `dynamic initializer for 'hostDroidPool''
// SYNTHETIC: SWBF2 0x00768fd0
// `dynamic atexit destructor for 'hostDroidPool''

// GLOBAL: SWBF2 0x01ee7c48
// hostDroidPool
MemoryPool hostDroidPool("HostDroid", 0);

// SYNTHETIC: SWBF2 0x00404df0
// `dynamic initializer for 'hostWalkerPool''
// SYNTHETIC: SWBF2 0x00768fe0
// `dynamic atexit destructor for 'hostWalkerPool''

// GLOBAL: SWBF2 0x01ed87f0
// hostWalkerPool
MemoryPool hostWalkerPool("HostWalker", 0);

// SYNTHETIC: SWBF2 0x00404e20
// `dynamic initializer for 'hostCommandWalkerPool''
// SYNTHETIC: SWBF2 0x00768ff0
// `dynamic atexit destructor for 'hostCommandWalkerPool''

// GLOBAL: SWBF2 0x01ed7100
// hostCommandWalkerPool
MemoryPool hostCommandWalkerPool("HostCommandWalker", 0);

// SYNTHETIC: SWBF2 0x00404e50
// `dynamic initializer for 'hostCommandFlyerPool''
// SYNTHETIC: SWBF2 0x00769000
// `dynamic atexit destructor for 'hostCommandFlyerPool''

// GLOBAL: SWBF2 0x01ed8940
// hostCommandFlyerPool
MemoryPool hostCommandFlyerPool("HostCommandFlyer", 0);

// SYNTHETIC: SWBF2 0x00404e80
// `dynamic initializer for 'hostDroidekaPool''
// SYNTHETIC: SWBF2 0x00769010
// `dynamic atexit destructor for 'hostDroidekaPool''

// GLOBAL: SWBF2 0x01ee7320
// hostDroidekaPool
MemoryPool hostDroidekaPool("HostDroideka", 0);

// SYNTHETIC: SWBF2 0x00404eb0
// `dynamic initializer for 'hostFlyerPool''
// SYNTHETIC: SWBF2 0x00769020
// `dynamic atexit destructor for 'hostFlyerPool''

// GLOBAL: SWBF2 0x01ee7b40
// hostFlyerPool
MemoryPool hostFlyerPool("HostFlyer", 0);

// SYNTHETIC: SWBF2 0x00404ee0
// `dynamic initializer for 'hostCarrierPool''
// SYNTHETIC: SWBF2 0x00769030
// `dynamic atexit destructor for 'hostCarrierPool''

// GLOBAL: SWBF2 0x01ee7cf0
// hostCarrierPool
MemoryPool hostCarrierPool("HostCarrier", 0);

// SYNTHETIC: SWBF2 0x00404f10
// `dynamic initializer for 'hostHoverPool''
// SYNTHETIC: SWBF2 0x00769040
// `dynamic atexit destructor for 'hostHoverPool''

// GLOBAL: SWBF2 0x01ee51e0
// hostHoverPool
MemoryPool hostHoverPool("HostHover", 0);

// SYNTHETIC: SWBF2 0x00404f40
// `dynamic initializer for 'hostCommandHoverPool''
// SYNTHETIC: SWBF2 0x00769050
// `dynamic atexit destructor for 'hostCommandHoverPool''

// GLOBAL: SWBF2 0x01ed7150
// hostCommandHoverPool
MemoryPool hostCommandHoverPool("HostCommandHover", 0);

// SYNTHETIC: SWBF2 0x00404f70
// `dynamic initializer for 'hostMountedTurretPool''
// SYNTHETIC: SWBF2 0x00769060
// `dynamic atexit destructor for 'hostMountedTurretPool''

// GLOBAL: SWBF2 0x01ed8748
// hostMountedTurretPool
MemoryPool hostMountedTurretPool("HostMountedTurret", 0);

// SYNTHETIC: SWBF2 0x00404fa0
// `dynamic initializer for 'hostPowerupPool''
// SYNTHETIC: SWBF2 0x00769070
// `dynamic atexit destructor for 'hostPowerupPool''

// GLOBAL: SWBF2 0x01ee7af0
// hostPowerupPool
MemoryPool hostPowerupPool("HostPowerup", 0);

// SYNTHETIC: SWBF2 0x00404fd0
// `dynamic initializer for 'hostArmedDynamicPool''
// SYNTHETIC: SWBF2 0x00769080
// `dynamic atexit destructor for 'hostArmedDynamicPool''

// GLOBAL: SWBF2 0x01ee75c0
// hostArmedDynamicPool
MemoryPool hostArmedDynamicPool("HostArmedDynamic", 0);

// SYNTHETIC: SWBF2 0x00405000
// `dynamic initializer for 'hostWalkerLegPairPool''
// SYNTHETIC: SWBF2 0x00769090
// `dynamic atexit destructor for 'hostWalkerLegPairPool''

// GLOBAL: SWBF2 0x01ee7bf8
// hostWalkerLegPairPool
MemoryPool hostWalkerLegPairPool("HostWalkerLegPair", 0);

// SYNTHETIC: SWBF2 0x00405030
// `dynamic initializer for 'hostWalkerBlendPool''
// SYNTHETIC: SWBF2 0x007690a0
// `dynamic atexit destructor for 'hostWalkerBlendPool''

// GLOBAL: SWBF2 0x01ee5430
// hostWalkerBlendPool
MemoryPool hostWalkerBlendPool("HostWalkerBlend", 0);

// SYNTHETIC: SWBF2 0x00405060
// `dynamic initializer for 'hostWalkerBlendUnitPool''
// SYNTHETIC: SWBF2 0x007690b0
// `dynamic atexit destructor for 'hostWalkerBlendUnitPool''

// GLOBAL: SWBF2 0x01ed5cf0
// hostWalkerBlendUnitPool
MemoryPool hostWalkerBlendUnitPool("HostWalkerBlendUnit", 0);

// SYNTHETIC: SWBF2 0x00405090
// `dynamic initializer for 'hostAimerPool''
// SYNTHETIC: SWBF2 0x007690c0
// `dynamic atexit destructor for 'hostAimerPool''

// GLOBAL: SWBF2 0x01ee5230
// hostAimerPool
MemoryPool hostAimerPool("HostAimer", 0);

// SYNTHETIC: SWBF2 0x004050c0
// `dynamic initializer for 'hostWeaponPool''
// SYNTHETIC: SWBF2 0x007690d0
// `dynamic atexit destructor for 'hostWeaponPool''

// GLOBAL: SWBF2 0x01ed6fb0
// hostWeaponPool
MemoryPool hostWeaponPool("HostWeapon", 0);

// SYNTHETIC: SWBF2 0x004050f0
// `dynamic initializer for 'hostAmmoCounterPool''
// SYNTHETIC: SWBF2 0x007690e0
// `dynamic atexit destructor for 'hostAmmoCounterPool''

// GLOBAL: SWBF2 0x01ee54d0
// hostAmmoCounterPool
MemoryPool hostAmmoCounterPool("HostAmmoCounter", 0);

// SYNTHETIC: SWBF2 0x00405120
// `dynamic initializer for 'hostEnergyBarPool''
// SYNTHETIC: SWBF2 0x007690f0
// `dynamic atexit destructor for 'hostEnergyBarPool''

// GLOBAL: SWBF2 0x01ee49d0
// hostEnergyBarPool
MemoryPool hostEnergyBarPool("HostEnergyBar", 0);

// SYNTHETIC: SWBF2 0x00405150
// `dynamic initializer for 'hostTreeGridPool''
// SYNTHETIC: SWBF2 0x00769100
// `dynamic atexit destructor for 'hostTreeGridPool''

// GLOBAL: SWBF2 0x01ee7ca0
// hostTreeGridPool
MemoryPool hostTreeGridPool("HostTreeGrid", 0);

// SYNTHETIC: SWBF2 0x00405180
// `dynamic initializer for 'hostOrdnanceTowCablePool''
// SYNTHETIC: SWBF2 0x00769110
// `dynamic atexit destructor for 'hostOrdnanceTowCablePool''

// GLOBAL: SWBF2 0x01ee76e0
// hostOrdnanceTowCablePool
MemoryPool hostOrdnanceTowCablePool("HostOrdnanceTowCable", 0);

// SYNTHETIC: SWBF2 0x004051b0
// `dynamic initializer for 'hostAsteroidPool''
// SYNTHETIC: SWBF2 0x00769120
// `dynamic atexit destructor for 'hostAsteroidPool''

// GLOBAL: SWBF2 0x01ed7050
// hostAsteroidPool
MemoryPool hostAsteroidPool("HostAsteroid", 0);

// SYNTHETIC: SWBF2 0x004051e0
// `dynamic initializer for 'hostFlagPool''
// SYNTHETIC: SWBF2 0x00769130
// `dynamic atexit destructor for 'hostFlagPool''

// GLOBAL: SWBF2 0x01ed71a0
// hostFlagPool
MemoryPool hostFlagPool("HostFlag", 0);

// SYNTHETIC: SWBF2 0x00405210
// `dynamic initializer for 'hostRemoteTerminalPool''
// SYNTHETIC: SWBF2 0x00769140
// `dynamic atexit destructor for 'hostRemoteTerminalPool''

// GLOBAL: SWBF2 0x01ed87a0
// hostRemoteTerminalPool
MemoryPool hostRemoteTerminalPool("HostRemoteTerminal", 0);

// GLOBAL: SWBF2 0x01e64a30
ProcessedInputs* hostJoysticks[64];

// SYNTHETIC: SWBF2 0x00404860
// `dynamic initializer for 'netLocalPos''

// GLOBAL: SWBF2 0x01ee7b90
// netLocalPos
PblVector3 netLocalPos[2];

class NetTurn
{
public:
    ProcessedInputs moves[64];
};
 

// SYNTHETIC: SWBF2 0x005b4df0
// NetTurn::NetTurn

// SYNTHETIC: SWBF2 0x005b4e20
// NetTurn::~NetTurn

struct SpawnInfo
{
    int changeCount;
    unsigned int changedTurn;
    bool cancel;
    bool spectate;
    bool autoassign;
    CommandPost* post;
    PblHandle<GameObject> postHandle;
    int team;
    int classIdx;
};
 

// SYNTHETIC: SWBF2 0x005b6470
// SpawnInfo::SpawnInfo

// SYNTHETIC: SWBF2 0x005b6490
// SpawnInfo::~SpawnInfo

struct RelChunkClusterData
{
    PblMatrix mMatrix;
    PblVector3 mVelocity;
    PblVector3 mOmega;
};
 

// SYNTHETIC: SWBF2 0x005c0b60
// RelChunkClusterData::RelChunkClusterData

// SYNTHETIC: SWBF2 0x00404750
// `dynamic initializer for 'remoteClientTurns''
// SYNTHETIC: SWBF2 0x00768d90
// `dynamic atexit destructor for 'remoteClientTurns''

// GLOBAL: SWBF2 0x01ed6960
// remoteClientTurns
NetTurn remoteClientTurns;
// SYNTHETIC: SWBF2 0x00404780
// `dynamic initializer for 'localClientTurns''
// SYNTHETIC: SWBF2 0x00768db0
// `dynamic atexit destructor for 'localClientTurns''

// GLOBAL: SWBF2 0x01ee4b40
// localClientTurns
ProcessedInputs localClientTurns[32][2];
// SYNTHETIC: SWBF2 0x004047b0
// `dynamic initializer for 'hostTurns''
// SYNTHETIC: SWBF2 0x00768de0
// `dynamic atexit destructor for 'hostTurns''

// GLOBAL: SWBF2 0x01ed89d0
// hostTurns
NetTurn hostTurns[32];
// SYNTHETIC: SWBF2 0x00404800
// `dynamic initializer for 'noInputs''
// SYNTHETIC: SWBF2 0x00768e00
// `dynamic atexit destructor for 'noInputs''

// GLOBAL: SWBF2 0x01ee7a88
// noInputs
ProcessedInputs noInputs;

// SYNTHETIC: SWBF2 0x004047e0
// `dynamic initializer for 'lateState''

// GLOBAL: SWBF2 0x01ee5280
// lateState
NetPktGroup lateState[8];

class NetNdObj
{
public:
    GameObject* obj;
    GameObject* orig;
    unsigned int uniqId;
    unsigned int index;
};
class NetNdObjMap
{
public:
    NetNdObjMap();
    void Clear();
    void Add(NetNdObj* object, unsigned int hash);

    NetNdObj* FindId(unsigned int uniqId)
    {
        return objMap.Find(uniqId);
    }

    NetNdObj* FindIndex(unsigned int index)
    {
        return array[index];
    }

    NetNdObj* FindLocal(GameObject* object);

    PblHashTable<NetNdObj, 256> objMap;
    unsigned int count;
    NetNdObj* array[256];
};
 
 

// FUNCTION: SWBF2 0x005b4520
NetNdObjMap::NetNdObjMap()
{
    count = 0;
}

// FUNCTION: SWBF2 0x005b4570
void NetNdObjMap::Clear()
{
    while (count)
    {
        delete array[--count];
    }
    objMap.Clear();
}

// SYNTHETIC: SWBF2 0x00404820
// `dynamic initializer for 'clientNdObjMap''

// GLOBAL: SWBF2 0x01ed5d58
// clientNdObjMap
NetNdObjMap clientNdObjMap;

// GLOBAL: SWBF2 0x01e62f52
bool listNearObjects;

// FUNCTION: SWBF2 0x005c2d20
static int RequestNearObjects(RedConsole*, unsigned int, char const*)
{
    listNearObjects = true;
    return 0;
}

// SYNTHETIC: SWBF2 0x00405240
// `dynamic initializer for 'nearObjects''

// GLOBAL: SWBF2 0x01ed86a6
// nearObjects
RedCommandConsole nearObjects("net.nearobjects", RequestNearObjects);

// SYNTHETIC: SWBF2 0x00404850
// `dynamic initializer for 'netCenter''

// GLOBAL: SWBF2 0x01ee7450
// netCenter
PblMatrix netCenter;
// SYNTHETIC: SWBF2 0x00405260
// `dynamic initializer for 'spawnInfo''
// SYNTHETIC: SWBF2 0x00769150
// `dynamic atexit destructor for 'spawnInfo''

// GLOBAL: SWBF2 0x01ed8990
// spawnInfo
SpawnInfo spawnInfo[2];

// SYNTHETIC: SWBF2 0x00405290
// `dynamic initializer for 'sendCenter''

// GLOBAL: SWBF2 0x01ed5d4c
// sendCenter
PblVector3 sendCenter;
// SYNTHETIC: SWBF2 0x004052a0
// `dynamic initializer for 'sendFront''

// GLOBAL: SWBF2 0x01ee5424
// sendFront
PblVector3 sendFront;
// SYNTHETIC: SWBF2 0x004052b0
// `dynamic initializer for 'relChunkClusterData''

// GLOBAL: SWBF2 0x01ee5520
// relChunkClusterData
RelChunkClusterData relChunkClusterData[8][10];
// SYNTHETIC: SWBF2 0x004052d0
// `dynamic initializer for 'farToNearCenter''

// GLOBAL: SWBF2 0x01ed8934
// farToNearCenter
PblVector3 farToNearCenter;
// SYNTHETIC: SWBF2 0x00405500
// `dynamic initializer for 'gCameraFrontPos''

// GLOBAL: SWBF2 0x01ed5d40
// gCameraFrontPos
PblVector3 gCameraFrontPos(0.0f, 0.0f, 0.0f);

// SYNTHETIC: SWBF2 0x00405540
// `dynamic initializer for 'trackDesc''

// GLOBAL: SWBF2 0x01ee5190
// trackDesc
EntityDesc trackDesc;

// SYNTHETIC: SWBF2 0x004052e0
// `dynamic initializer for 'aloReinforcements''

// GLOBAL: SWBF2 0x01ee4a20
// aloReinforcements
AloReinforcements aloReinforcements;
// SYNTHETIC: SWBF2 0x00405320
// `dynamic initializer for 'aloHeroData0''

// GLOBAL: SWBF2 0x01ee7530
// aloHeroData0
AloHeroData aloHeroData0(0);
// SYNTHETIC: SWBF2 0x00405360
// `dynamic initializer for 'aloHeroData1''

// GLOBAL: SWBF2 0x01ed7818
// aloHeroData1
AloHeroData aloHeroData1(1);
// SYNTHETIC: SWBF2 0x004053a0
// `dynamic initializer for 'aloHeroData2''

// GLOBAL: SWBF2 0x01ee7370
// aloHeroData2
AloHeroData aloHeroData2(2);
// SYNTHETIC: SWBF2 0x00405400
// `dynamic initializer for 'aloMiniStats''

// GLOBAL: SWBF2 0x01ed71f0
// aloMiniStats
AloMiniStats aloMiniStats;
// SYNTHETIC: SWBF2 0x00405410
// `dynamic initializer for 'aloTrackable''

// GLOBAL: SWBF2 0x01ee7780
// aloTrackable
AloTrackable aloTrackable;
// SYNTHETIC: SWBF2 0x00405420
// `dynamic initializer for 'aloStats''

// GLOBAL: SWBF2 0x01ed70f0
// aloStats
AloStats aloStats;
// SYNTHETIC: SWBF2 0x00405460
// `dynamic initializer for 'aloNextGameInfo''

// GLOBAL: SWBF2 0x01ee75b0
// aloNextGameInfo
AloNextGameInfo aloNextGameInfo;
// SYNTHETIC: SWBF2 0x004054a0
// `dynamic initializer for 'aloMissionTimer''

// GLOBAL: SWBF2 0x01ee5400
// aloMissionTimer
AloMissionTimer aloMissionTimer;

// FUNCTION: SWBF2 0x005c52f0
void AloReinforcements::Reset()
{
    for (int i = 0; i < 2; i++)
    {
        teamCount[i] = 0;
        teamActive[i] = 0;
        teamBleed[i] = 0.0f;
        teamPoints[i] = 0;
    }
    changeTurn = (unsigned int)-1;
}

// FUNCTION: SWBF2 0x005c56e0
void AloHeroData::Reset()
{
    heroUnlocked = false;
    heroPlayerID = -1;
    nextHeroPlayerID = -1;
    lastHeroPlayerID = -1;
    heroKillerID = -1;
    heroSelectedTurn = 0;
    heroSpawnedTurn = 0;
    heroDiedTurn = 0;
    changeTurn = (unsigned int)-1;
}

// FUNCTION: SWBF2 0x005c74c0
void AloMissionTimer::Reset()
{
    timerValid = false;
    timerTime = 0.0f;
    timerRate = 1.0f;
    timerActive = true;
    timerShow = 0;
    timerTeam = 0;
    changeTurn = (unsigned int)-1;
}

// FUNCTION: SWBF2 0x005c6880
AloTrackable::AloTrackable()
{
    sig = ALO_TRACKABLE;
    deltaSends = false;
    Reset();
}

// FUNCTION: SWBF2 0x005c68e0
void AloTrackable::Reset()
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            mTrackable[i][j] = (Trackable*)-1;
        }
    }
    for (int i = 0; i < 64; i++)
    {
        changeTurn[i] = (unsigned int)-1;
    }
}

// FUNCTION: SWBF2 0x005c63e0
AloMiniStats::AloMiniStats()
{
    sig = ALO_MINI_STATS;
    deltaSends = false;
    Reset();
}

// FUNCTION: SWBF2 0x005c6440
void AloMiniStats::Reset()
{
    for (unsigned int i = 0; i < 64; i++)
    {
        mEnemyKills[i] = 0;
        mTeamKills[i] = 0;
        mDeaths[i] = 0;
        mCps[i] = 0;
        mPoints[i] = 0;
    }
    for (int i = 0; i < 64; i++)
    {
        changeTurn[i] = (unsigned int)-1;
    }
}

extern AloMapDisplayMarkers aloMapDisplayMarkers;
void InitBootPlayer();

// FUNCTION: SWBF2 0x005c7c70
void AtleastOnce::ResetAll()
{
    aloTrackable.Reset();
    aloReinforcements.Reset();
    aloHeroData0.Reset();
    aloHeroData1.Reset();
    aloHeroData2.Reset();
    aloMapDisplayMarkers.Reset();
    aloStats.lastGameOver = false;
    aloStats.gotGameOver = false;
    aloStats.changeTurn = (unsigned int)-1;
    aloMiniStats.Reset();
    InitBootPlayer();
    aloNextGameInfo.lastGameOver = false;
    aloNextGameInfo.gotGameOver = false;
    aloNextGameInfo.changeTurn = (unsigned int)-1;
    aloMissionTimer.Reset();
}

class NetEntityList;
// GLOBAL: SWBF2 0x01fa8848
NetEntityList* netEntities;
// GLOBAL: SWBF2 0x01e64dd8
FILE* chatLog;

// GLOBAL: SWBF2 0x007e802d
bool netAutoAssignTeams = true;
// GLOBAL: SWBF2 0x007e804c
int netHeroRuleUnlockVal = 1;

// GLOBAL: SWBF2 0x007e8044
int netHeroRuleTeam = 1;
// GLOBAL: SWBF2 0x007e8048
int netHeroRuleUnlock = 1;
// GLOBAL: SWBF2 0x007e823c
int netHeroRuleRespawn = 1;
// GLOBAL: SWBF2 0x007e8254
int netUpdateSize = 1024;
// GLOBAL: SWBF2 0x007e853c
int netNumBots = 16;
// GLOBAL: SWBF2 0x007e8e60
// netPreGameTimeMax
int netPreGameTimeMax = 45;
// GLOBAL: SWBF2 0x01e62ee3
bool netHeroesEnabled;

// GLOBAL: SWBF2 0x01e62f3a
bool netHiRes;
// GLOBAL: SWBF2 0x01e62f3b
bool gNoPregame;
// GLOBAL: SWBF2 0x01e64a2e
bool netFrameLock;
// GLOBAL: SWBF2 0x01e62f18
int netHeroRuleRespawnVal;
// GLOBAL: SWBF2 0x01e62f48
int netVoiceMode;

// GLOBAL: SWBF2 0x007e8006
bool dragEnabledHost = true;
// GLOBAL: SWBF2 0x01e57936
bool g_SpectatorEnabled[1];
// GLOBAL: SWBF2 0x01e62ebc
unsigned int netHeroDiedTurn[3];
// GLOBAL: SWBF2 0x01e62ec8
int netHeroNextPlayerID[3];
// GLOBAL: SWBF2 0x01e62ee0
bool netHeroUnlocked[3];
// GLOBAL: SWBF2 0x01e62ef0
int netHeroPlayerID[3];
// GLOBAL: SWBF2 0x01e62efc
int gDebugCount;
// GLOBAL: SWBF2 0x01e62f00
int netHeroAIID[3];

// GLOBAL: SWBF2 0x01e62ee4
unsigned int netHeroSelectedTurn[3];
// GLOBAL: SWBF2 0x01e62eb0
unsigned int netHeroSpawnedTurn[3];
// GLOBAL: SWBF2 0x01e64b38
float spHeroDiedTime[3];
// GLOBAL: SWBF2 0x01e62f10
bool netIsLocalTurn;
// GLOBAL: SWBF2 0x01e62f11
bool netIsLateTurn;
// GLOBAL: SWBF2 0x01e62f14
bool netHeroFirstUnlocked[3];
// GLOBAL: SWBF2 0x01e62f1c
int netHeroKillerID[4];
// GLOBAL: SWBF2 0x01e62f2c
int netHeroLastPlayerID[3];
// GLOBAL: SWBF2 0x01e62f93
bool clientHostCommProblems;
// GLOBAL: SWBF2 0x01e62fc0
unsigned __int64 lateRecv[8];
// GLOBAL: SWBF2 0x01e63001
bool netIsLastUpdateTurn;

// GLOBAL: SWBF2 0x01e64a2c
bool netIsUpdateTurn;
// GLOBAL: SWBF2 0x01e64b34
int clientLag;
// GLOBAL: SWBF2 0x01e64b48
float miniMoveTime;
// GLOBAL: SWBF2 0x01e64b54
int netTurnsPerSecond;
// GLOBAL: SWBF2 0x01e64b60
int clientAhead;
// GLOBAL: SWBF2 0x01e64b9f
bool netIsLastLocalTurn;
// GLOBAL: SWBF2 0x01e64cd7
bool netGameOver;
// GLOBAL: SWBF2 0x01e64de4
unsigned int clientHostTurnNumber;
// GLOBAL: SWBF2 0x01e64de8
unsigned __int64 hostRecv[32];
// GLOBAL: SWBF2 0x01e64eec
int clientUpdateCount;

// GLOBAL: SWBF2 0x01e64efe
bool clientTurnHeldBack;
// GLOBAL: SWBF2 0x01e64eff
bool netIsClient;
// GLOBAL: SWBF2 0x01fa2512
bool timeLocked;
// GLOBAL: SWBF2 0x01fa3d7c
float adjustTime;
// GLOBAL: SWBF2 0x01fa6594
unsigned int clientAckedTurnNumber;
// GLOBAL: SWBF2 0x01fa65a0
unsigned int reliableNetEventTail;
// GLOBAL: SWBF2 0x01fa65b0
unsigned int updateTurnNumber;
// GLOBAL: SWBF2 0x01fa65f8
bool clientBootVoted[2];
// GLOBAL: SWBF2 0x01fa65fa
bool destroyOk;
// GLOBAL: SWBF2 0x01fa65fb
bool finishedTimeScale;
// GLOBAL: SWBF2 0x01fa65fc
unsigned int clientReliableBatchNumber;
// GLOBAL: SWBF2 0x01fa8720
bool clientSkipped[64];
// GLOBAL: SWBF2 0x01fa8760
unsigned int sendTurnNumber;

// GLOBAL: SWBF2 0x01fa8764
float pendingTimeAdjust;
// GLOBAL: SWBF2 0x01fa8770
NetObjStateMap* clientNextObjState;
// GLOBAL: SWBF2 0x01fa877c
float timeScale;
// GLOBAL: SWBF2 0x01fa8780
int timeStableCount;
// GLOBAL: SWBF2 0x01fa8789
bool hostQuit;
// GLOBAL: SWBF2 0x01fa878a
bool canQuit;
// GLOBAL: SWBF2 0x01fa8790
unsigned int predictLocalTurnNumber;
// GLOBAL: SWBF2 0x01fa8794
unsigned int relayTurnNumber;
// GLOBAL: SWBF2 0x01fa8798
unsigned int hostTurnNumber;
// GLOBAL: SWBF2 0x01fa87a0
float hostCycleTime;

// FUNCTION: SWBF2 0x005b87f0
bool NetGameP::HasHostCycled()
{
    if (!netEnabledNext || NetComm::IsHost())
    {
        return true;
    }
    return NetComm::GetTime() > hostCycleTime;
}
// GLOBAL: SWBF2 0x01fa87b4
unsigned int predictTurnNumber;
// GLOBAL: SWBF2 0x01fa87bc
NetObjStateMap* clientLastObjState;
// GLOBAL: SWBF2 0x01fa87c4
unsigned int lateTurnNumber;
// GLOBAL: SWBF2 0x01fa88a4
bool hostCommProblems;
// GLOBAL: SWBF2 0x01fa88a8
unsigned __int64 hostNextPlayingMask;
// GLOBAL: SWBF2 0x01fa88bc
float canQuitTimeOut;

// GLOBAL: SWBF2 0x01fa88c0
float clientDisconnectStartTime;
// GLOBAL: SWBF2 0x01fa88cc
unsigned int reliableNetEventHead;

namespace FrameUpdate
{
void Create();
}

class NetDrag
{
public:
    static void Create(bool bMission);
    static void Reset();

    static void BeginShell();
    static void EndShell();
};

class NetAdmin
{
public:
    static void Create(bool bMission);
    static void AddPlayer(int iPlayer);
    static void RemovePlayer(int iPlayer);
    static bool ParseLineRemote(int player, unsigned short* line, bool console, bool prefixed);
    static bool ParseLineLocal(unsigned short* line, bool console);
    static void ReceiveConsole();
    static void __fastcall ChatMessage(int iPlayer, const unsigned short* pusMessage);
};

void ClearNetEvents();

void InitFrameLock();
void InitBootPlayer();
void SetTurnHighBits(unsigned int uBits);

class NetEntity
{
public:
    EntityType type;
    GameObject* obj;
    PblVector3 pos;
    unsigned int updateTurn;
};

template <class T, int N>
class BblList
{
public:
    class T1 : public T
    {
    public:
        int next;
    };

    // TEMPLATE: SWBF2 0x005d3b30
    // ??0T1@?$BblList@VNetEntity@@$0CAA@@@QAE@XZ
    // BblList<NetEntity,512>::T1::T1()

    class Itor
    {
    public:
        Itor(BblList& source);
        T& operator*() { return list->array[idx]; }
        Itor& operator++();
        operator bool() { return idx < list->lastUsed + 1; }
        void Advance();

        BblList* list;
        int idx;
    };

    BblList();

    // TEMPLATE: SWBF2 0x005d3870
    // BblList<NetEntity,512>::Add
    T& Add()
    {
        T1& item = array[nextFree];
        if (nextFree > lastUsed)
        {
            lastUsed = nextFree;
        }
        nextFree = item.next;
        item.next = -2;
        return item;
    }

    void Remove(T& item);

    int nextFree;
    int lastUsed;
    T1 array[N];
    int next[N];
};

template <class T, int N>
BblList<T, N>::BblList()
{
    nextFree = 0;
    lastUsed = -1;
    for (int i = 0; i < N; ++i)
    {
        array[i].next = i + 1;
    }
    array[N - 1].next = -1;
}

// FUNCTION: SWBF2 0x005d3940
template <>
BblList<NetEntity, 512>::Itor::Itor(BblList<NetEntity, 512>& source)
    : list(&source), idx(0)
{
    Advance();
}

// FUNCTION: SWBF2 0x005d3970
template <>
BblList<NetEntity, 512>::Itor& BblList<NetEntity, 512>::Itor::operator++()
{
    ++idx;
    Advance();
    return *this;
}

class NetEntityList : public BblList<NetEntity, 512>
{
};
 

namespace GameVoiceChat
{
class Transport;
Transport* GetTransport();
}

extern PblVector3 gVectorZero;

class TreeGrid;
class CollisionManager { public: static void EnterClient(); static void LeaveClient();
    static bool CreateClient(); static bool DestroyClient();
    static int GetObjectsInRange(const PblVector3& vCenter, float fRadius,
                                 GameObject** const pObjects, int iMaxCount,
                                 const GameObject* pIgnore, int iFilter);
    static TreeGrid* GetTreeGrid();
    static void BeginList(PblList<CollisionObject>::Itor& itor);
    static void ListObjects(PblList<CollisionObject>::Itor& itor, CollisionObject** const pObjects,
                            int& iCount); };

class PAManager { public: static void EnterClient(); static void LeaveClient();
    static void CreateClient(); static void DestroyClient(); static void ReadObjects(NetPktGroup& group);
    static void WriteObjects(NetPktGroup& group); };
class SpawnManager { public: static SpawnManager* sInstance; static SpawnManager* Instance() { return sInstance; }   static void EnterClient(); static void LeaveClient();
    static void CreateClient(); static void DestroyClient();
    CommandPost* GetRandomSpawnPoint(int iTeam); };
namespace MissionTimer
{
void EnterClient(); void LeaveClient();
}
namespace NetRandom
{
void EnterClient(); void LeaveClient();
}
namespace ObjectiveManager
{
void EnterClient(); void LeaveClient(); void CreateClient();
}
namespace TargetManager
{
void EnterClient(); void LeaveClient(); void CreateClient(); void DestroyClient();
}
namespace TeamManager
{
void EnterClient(); void LeaveClient();
}
namespace VehicleManager
{
void EnterClient(); void LeaveClient();
}

class EntityDroid { public: static MemoryPool sMemoryPool; };
class MountedTurret { public: static MemoryPool sMemoryPool; };
class EntityBuildingArmedDynamic { public: static MemoryPool sMemoryPool; };
class WalkerLegPair { public: static MemoryPool sMemoryPool; };


class OrdnanceTowCable { public: static MemoryPool sMemoryPool; };

class TriggerRegionManager { public: static void CreateClient(); static void DestroyClient(); };

class DustEffectClass { public: static void CreateClient(); };
class LeafParticleManager { public: static bool CreateClient(); };
class BirdParticleManager { public: static bool CreateClient(); };
class FishParticleManager { public: static bool CreateClient(); };
class LightningBoltEffect { public: static void CreateClient(); };
class LeafPatchClass { public: static void CreateClient(); };
class GrassPatchClass { public: static void CreateClient(); };
class RedShadowProxy { public: static void AddToScene2(); static void RemoveFromScene2(); };

class PAAnimationGroup { public: static void ClientGeometryCreated(CollisionObject* pHost, CollisionObject* pClient); };

void CreateClientCollision(bool bNetCollision);
bool SentStats();
bool SentNextGameInfo();
bool ReceivedStats();
bool ReceivedNextGameInfo();

extern int netMinPlayersToAutoLaunchDedicated;
extern int netTraceDst;

class NetObjState
{
public:
    ~NetObjState() {}

    static void* operator new(size_t uSize) { return sMemoryPool.Allocate(uSize); }
    // FUNCTION: SWBF2 0x005b46a0
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    PblHandle<GameObject> handle;
    unsigned int usedTurn;
    unsigned char classIdx;
    unsigned char playerId;
    unsigned char initialPlayerId;
    unsigned char character;
    unsigned char team;
    unsigned char perceivedTeam;
    PblQuaternion rotation;
    PblVector3 position;

    static MemoryPool sMemoryPool;
};
 

// SYNTHETIC: SWBF2 0x004048b0
// `dynamic initializer for 'NetObjState::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768e20
// `dynamic atexit destructor for 'NetObjState::sMemoryPool''

// GLOBAL: SWBF2 0x01ee4af0
// NetObjState::sMemoryPool
MemoryPool NetObjState::sMemoryPool("NetObjState", 0x30);

// VTABLE: SWBF2 0x007ab8c0
class NetCollisionObject : public CollisionObject
{
public:
    NetCollisionObject(CollisionObject* pHostObject);

    virtual const PblVector3& GetPosition() const;
    virtual void SetPosition(const PblVector3& rPosition);
    virtual void SetMatrix(const PblMatrix& rMatrix);
    virtual bool CanProcedurallyAnimate();
    virtual const PblMatrix& GetMatrix() const;

    virtual void RecalculateAABB();

    virtual const CollisionObject* GetCollisionObject() const;
    virtual CollisionObject* GetCollisionObject();

    CollisionObject* mHostObject;
};
 
 

// GLOBAL: SWBF2 0x01fa878b
bool redMemoryTrace;

// FUNCTION: SWBF2 0x005b46c0
NetObjStateMap* NetObjStateMap::Alloc()
{
    if (!topFree)
    {
        return new NetObjStateMap;
    }

    NetObjStateMap* pMap = topFree;
    topFree = topFree->next;
    pMap->turn = (unsigned int)-1;
    pMap->next = 0;
    return pMap;
}

// FUNCTION: SWBF2 0x005b4770
void NetObjStateMap::Free()
{
    for (unsigned int i = 0; i < 64; i++)
    {
        NetObjState* state = stateMap[i];
        if (state)
        {
            delete state;
            stateMap[i] = 0;
        }
    }
    next = topFree;
    topFree = this;
}

// FUNCTION: SWBF2 0x005b4a30
NetCollisionObject::NetCollisionObject(CollisionObject* pHostObject)
    : CollisionObject(pHostObject->GetCollisionType(), pHostObject->GetCollisionModel(),
                      pHostObject->GetFoleyFXColliderClass(), pHostObject->GetFoleyFXCollideeClass())
{
    mHostObject = pHostObject;
    RecalculateAABB();
}

// FUNCTION: SWBF2 0x005b4ac0
bool NetCollisionObject::CanProcedurallyAnimate()
{
    return !NetGame::IsHost();
}

// FUNCTION: SWBF2 0x005b4ae0
const PblVector3& NetCollisionObject::GetPosition() const
{
    return mHostObject->GetPosition();
}

// FUNCTION: SWBF2 0x005b4b10
const PblMatrix& NetCollisionObject::GetMatrix() const
{
    return mHostObject->GetMatrix();
}

// FUNCTION: SWBF2 0x005b4b40
void NetCollisionObject::SetPosition(const PblVector3& rPosition)
{
    mHostObject->SetPosition(rPosition);
}

// FUNCTION: SWBF2 0x005b4b70
void NetCollisionObject::SetMatrix(const PblMatrix& rMatrix)
{
    mHostObject->SetMatrix(rMatrix);
}

// FUNCTION: SWBF2 0x005b4ba0
const CollisionObject* NetCollisionObject::GetCollisionObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x005b4bb0
CollisionObject* NetCollisionObject::GetCollisionObject()
{
    return this;
}

// FUNCTION: SWBF2 0x005b4bc0
void NetCollisionObject::RecalculateAABB()
{
    mHostObject->RecalculateAABB();
    SetCollisionCenter(mHostObject->GetCollisionCenter());
    float fRadius = mHostObject->GetCollisionRadius();
    SetCollisionRadius(fRadius);
    mAABB = mHostObject->mAABB;
}

// SYNTHETIC: SWBF2 0x005b4c60
// NetCollisionObject::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005b4c90
void AddNetCollision(CollisionObject* pHostObject)
{
    NetCollisionObject* pNetObject = new NetCollisionObject(pHostObject);
    pNetObject->AddStaticToCollisionManager();
    PAAnimationGroup::ClientGeometryCreated(pHostObject, pNetObject);
}

// FUNCTION: SWBF2 0x005b4d20
NetPlayer::NetPlayer()
{
    memset(name, 0, sizeof(name));
    memset(prevName, 0, sizeof(prevName));
    mGSPlayerID = 0;
    mBooted = false;
    mBootTime = -1.0f;
    mDeclinedHero = false;
    mIsSpawning = false;
    mSpawnOrder = -1;
    mLastStatusTurn = 0;
}

// FUNCTION: SWBF2 0x005b4e40
bool IsNeverDies(EntityType eType)
{
    switch (eType)
    {
    case ENT_POST:
    case ENT_ARMED:
    case ENT_BUILDING:
    case ENT_POWERUPSTATION:
    case ENT_TRAP:
        return true;
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x005b4e90
unsigned int NetGame::GetClientAckedTurnNumber()
{
    return clientAckedTurnNumber;
}

// FUNCTION: SWBF2 0x005b4ea0
unsigned int NetGame::GetClientTurnNumber()
{
    return clientTurnNumber;
}

// FUNCTION: SWBF2 0x005b4eb0
bool NetGame::IsCockpitCam(int iPlayer)
{
    return cockpitCams[iPlayer];
}

// FUNCTION: SWBF2 0x005b4ec0
void InitSoundStuff()
{
    Snd::Globals::smAlloc = SndAlloc;
    Snd::Globals::smFree = SndFree;
    Snd::Globals::smTempAlloc = SndTempAlloc;
    Snd::Globals::smTempFree = SndTempFree;
    Snd::Globals::smWarningFunction = PrintDebugString;

    GameSoundEngine::SetHeap(GameMemory::RunTimeHeap);

    Snd::File::SystemOpen(0x7FFFFFFF);
}

// FUNCTION: SWBF2 0x005b4f10
void NetGame::SetForceHumansToTeam1(bool bForce)
{
    netSetForceHumansToTeam1 = bForce;
    if (bForce)
    {
        GameLoop::SetCanSwitchSides(!bForce);
        for (int i = 0; i < 64; i++)
        {
            netPlayers[i].isTeam1 = true;
        }
    }
}

// FUNCTION: SWBF2 0x005b4f70
bool NetGame::GetForceHumansToTeam1()
{
    return netSetForceHumansToTeam1;
}

// FUNCTION: SWBF2 0x005b4f80
bool NetGame::DoesHostKnowILoaded()
{
    if (IsHost() || !loadGame)
    {
        return true;
    }
    return loadGame->HostKnowsILoaded();
}

// FUNCTION: SWBF2 0x005b4fc0
bool NetGame::IsReadyToPlay()
{
    if (loadGame)
    {
        return false;
    }

    if (!IsHost() || IsClient())
    {
        return clientHostTurnNumber != 0;
    }

    if (hostTurnNumber != 0)
    {
        return true;
    }

    int iPlaying = 0;
    for (int i = 0; i < 64; i++)
    {
        bool bPlaying = IsPlaying(i, false);
        if (!IsClient())
        {
            bPlaying |= NetGame::IsPlayingNext(i);
        }
        if (bPlaying)
        {
            iPlaying++;
        }
    }
    if (iPlaying < netMinPlayersToAutoLaunchDedicated)
    {
        return false;
    }

    if (!gNoPregame)
    {
        SetPreGame(true);
    }
    return true;
}

// FUNCTION: SWBF2 0x005b50c0
bool NetGame::IsGameOver()
{
    if (!netEnabledNext)
    {
        return true;
    }

    if (NetComm::IsHost())
    {
        float fTime = NetComm::GetTime();
        if (clientDisconnectStartTime == 0.0f)
        {
            clientDisconnectStartTime = fTime;
        }
        for (int i = 0; i < netCurMaxPlayers; i++)
        {
            if (IsPlaying(i, false))
            {
                continue;
            }
            if (NetComm::IsConnected(i))
            {
                return false;
            }
        }
        if (!SentStats())
        {
            return false;
        }
        if (!SentNextGameInfo())
        {
            return false;
        }
    }
    else
    {
        if (!ReceivedStats())
        {
            return false;
        }
        if (!ReceivedNextGameInfo())
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x005b51b0
void NetGame::Init()
{
    netClientCreated = false;
    netInShell = true;

    InitSoundStuff();
    NetComm::Init();
    NetQOS::Init();

    netCurMaxPlayers = 1;
    netCurMaxHosts = 1;
    netHostIndex = -1;
    netIsSplit = GameLoop::GetNumCameras() > 1;

    memset(cockpitCams, 1, sizeof(cockpitCams));

    for (int i = 0; i < 3; i++)
    {
        netHeroSndPlayHostUnlock[i] = false;
        netHeroSndPlayHostSelectable[i] = false;
        netHeroSndPlayHostSpawn[i] = false;
        netHeroSndPlayHostDefeat[i] = false;
    }

    SetPreGame(false);
}

// FUNCTION: SWBF2 0x005b5280
void NetPlayer::Init(bool bMission)
{
    unsigned int i;
    lastObjState = 0;
    nextObjState = 0;

    if (!bMission)
    {
        hostIndex = -1;
        clientOfIndex = 0;
        canReplica = false;
        canRelay = false;
        for (i = 0; i < 1; i++)
        {
            myGuest[i] = -1;
        }
        guestOf = -1;
        if (netSetForceHumansToTeam1)
        {
            isTeam1 = true;
        }
        else
        {
            isTeam1 = ((this - netPlayers) & 1) != 0;
        }
        name[0] = 0;
        prevName[0] = 0;
        key = 0;
    }

    mSpectateID = -1;
    mIsSpawning = false;
    mSpawnOrder = -1;
    reliableEventTail = 0;
    reliableEventHead = 0;
    reliableBatchNumber = 1;
    reliableSendTurn = (unsigned int)-1;
    ackedUpdateTurn = 0;
    clientTurn = 0;
    nextUpdateTurn = 0;
    mLastReceivedTurn = 0;
    lastEnterVehicleTurn = 0;
    nextSendPingIndex = 0;
    netEventTail = 0;
    lagSample = 0;
    for (i = 0; i < 20; i++)
    {
        lagSamples[i] = 0;
        lagTurns[i] = 0;
    }

    SwitchResponseRelayInfo& relay = objSwitchRelay;
    relay.switchResponse = SR_NONE;
    relay.switchTurn = (unsigned int)-1;

    avgLag = 0.0f;
    errLag = 2.0f;
    updateCount = 0;
    filledCount = 0;
    dropCount = 0;
    packetCount = 0;

    hostStats.players[this - netPlayers].Init();

    requestedQuit = false;
    for (i = 0; i < 64; i++)
    {
        skippedSends[i] = 0;
    }
    updateWindowSize = 1;
    for (i = 0; i < 2; i++)
    {
        sentUpdateTurn[i] = 0;
        sentUpdateTime[i] = 0.0f;
    }
    averageRtt = 0.25f;
    deviationRtt = 0.0f;
    spawnChangeCount = 0;
    selectedSide = false;
    spawnRequested = false;
    spawnPost = 0;
    spawnClass = 0;
    nextStats = 0.0f;
    hasStats = false;
    lateFenceTurn = 0;
    mLastStatusTurn = 0;
    mDeclinedHero = false;
    spawnCenter.SetAll(0.0f);
    spawnFront = PblVector3(1.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x005b5690
void NetPlayer::Clean()
{
    if (this - netPlayers == netTraceDst)
    {
        gDebugCount++;
    }

    if (lastObjState)
    {
        lastObjState->Free();
        lastObjState = 0;
        nextObjState->Free();
        nextObjState = 0;
    }

    mGSPlayerID = 0;
    strlcpy(prevName, name, sizeof(prevName));
    hostIndex = -1;
    clientOfIndex = 0;
    canReplica = false;
    canRelay = false;
    for (unsigned int i = 0; i < 1; i++)
    {
        myGuest[i] = -1;
    }
    guestOf = -1;
    if (netSetForceHumansToTeam1)
    {
        isTeam1 = true;
    }
    else
    {
        isTeam1 = ((this - netPlayers) & 1) != 0;
    }
    name[0] = 0;
    key = 0;
    mSpectateID = -1;
    mIsSpawning = false;
    mSpawnOrder = -1;
}

// FUNCTION: SWBF2 0x005b5820
void NetHost::Init(bool bMission)
{
    if (!bMission)
    {
        hostMode = HM_NONE;
        isDedicated = true;
        updateTurn = 0;
        hostAckedUpdateTurn = 0;
        clientMask = 0;
        key = 0;
    }
}

// FUNCTION: SWBF2 0x005b5870
void CloseJournal()
{
    if (isJournalOpened)
    {
        isJournalOpened = false;
    }
}

// FUNCTION: SWBF2 0x005b5890
void NetGame::Create(bool bMission)
{
    unsigned int uiScreenWidth = RedInterfaceScreen::GetScreenWidth();
    unsigned int uiScreenHeight = RedInterfaceScreen::GetScreenHeight();
    unsigned int i;

    PblJournal::Get(PJT_FULL)->RecordLow(&uiScreenWidth, sizeof(uiScreenWidth));
    PblJournal::Get(PJT_FULL)->RecordLow(&uiScreenHeight, sizeof(uiScreenHeight));
    char nameLen = (char)strlen(gGamespyAutoName) + 1;
    PblJournal::Get(PJT_FULL)->RecordLow(&nameLen, sizeof(nameLen));
    PblJournal::Get(PJT_FULL)->RecordLow(gGamespyAutoName, nameLen);

    PblValues::Record('E', PVT_FULL);
    PblJournal::Record(PblTime::ticksPerSecond, PJT_FULL);
    PblTimeBase::PblJournal_RecordInit();
    PblJournal::Record(netNumBots, PJT_FULL);
    PblJournal::Record(netAutoAssignTeams, PJT_FULL);
    PblJournal::Record(netFrameLock, PJT_FULL);
    PblJournal::Record(gNoPregame, PJT_FULL);
    PblJournal::Record(netPreGameTimeMax, PJT_FULL);
    PblJournal::Record(netHeroesEnabled, PJT_FULL);
    PblJournal::Record(netHeroRuleUnlockVal, PJT_FULL);
    PblJournal::Record(netHeroRuleRespawn, PJT_FULL);
    PblJournal::Record(netHeroRuleRespawnVal, PJT_FULL);
    PblJournal::Record(netHeroUnlockStartTimer, PJT_FULL);
    PblJournal::Record(netUpdateSize, PJT_FULL);
    PblJournal::Record(netHiRes, PJT_FULL);
    PblJournal::Record(netVoiceMode, PJT_FULL);

    FrameUpdate::Create();
    NetObj::sMemoryPool.Create(0x80, NetObj::sMemoryPool.GetSize(),
                               NetObj::sMemoryPool.GetGrow());
    NetObjStateMap::topFree = 0;
    EntityDoor::ResetDoorList();

    netInShell = !bMission;

    if (bMission)
    {
        netEnabledNext = netEnabled;
        netEnabled = false;
        netGameCount++;
    }
    else
    {
        netEnabledNext = false;
        netGameCount = 0;
    }

    if (netGameCount == 0 && netVoiceMode > 0)
    {
        VoiceChat::Transport* const transport = GameVoiceChat::GetTransport();
        VoiceChat::Transport::Method method((VoiceChat::Transport::Method::Type)netVoiceMode,
            transport->GetMethod().GetNumRelayNodes());
        transport->SetMethod(method, true);
    }

    if (netEnabledNext && bMission)
    {
        InitFrameLock();
    }

    netIsLocalTurn = true;
    netIsUpdateTurn = true;
    netIsLastLocalTurn = true;
    netIsLastUpdateTurn = true;
    singleTime = 0.0f;
    clientTurnNumber = 0;
    hostTurnNumber = 0;
    sendTurnNumber = 0;
    relayTurnNumber = 0;
    predictTurnNumber = 0;
    updateTurnNumber = 0;
    predictLocalTurnNumber = 0;
    clientAckedTurnNumber = 0;
    clientAhead = 0;
    hostCommProblems = false;
    clientHostTurnNumber = 0;
    clientHostCommProblems = false;
    miniMoveTime = 0.0f;
    SetTurnHighBits(0);
    netClientActive = false;
    if (!bMission)
    {
        clientPlayingMask = 0;
    }
    memset(hostRecv, 0, sizeof(hostRecv));
    memset(lateRecv, 0, sizeof(lateRecv));
    hostPlayingMask = 1;
    turnRatio = 0.0f;
    destroyOk = false;
    netIsLocalTurn = true;
    netIsUpdateTurn = true;
    clientUpdateCount = 0;
    canQuit = false;
    canQuitTimeOut = 0.0f;
    hostQuit = false;
    hostStats.Init();
    memset(clientSkipped, 0, sizeof(clientSkipped));

    for (int teamNum = 0; teamNum < 3; teamNum++)
    {
        netHeroFirstUnlocked[teamNum] = false;
        netHeroUnlocked[teamNum] = false;
        netHeroPlayerID[teamNum] = -1;
        netHeroNextPlayerID[teamNum] = -1;
        netHeroLastPlayerID[teamNum] = -1;
        netHeroKillerID[teamNum] = -1;
        netHeroAIID[teamNum] = -1;
        netHeroDiedTurn[teamNum] = 0;
    }

    netGameOver = false;
    hostCycleTime = 3.0f;
    InitBootPlayer();

    for (i = 0; i < 64; i++)
    {
        hostJoysticks[i] = new ProcessedInputs;
        netPlayers[i].Init(bMission);
        Tracker* pTracker = Tracker::GetTracker(i);
        if (pTracker)
        {
            pTracker->SetSpectating(IsSpectator(i));
        }
    }

    for (i = 0; i < 3; i++)
    {
        netHosts[i].Init(bMission);
    }

    if (!bMission)
    {
        netHostIndex = -1;
    }
    clientSwitchObjects = SR_NONE;
    clientLastObjState = NetObjStateMap::Alloc();
    clientNextObjState = NetObjStateMap::Alloc();
    ClearNetEvents();
    reliableNetEventHead = reliableNetEventTail = 0;
    clientReliableBatchNumber = 0;
    adjustTime = 0.0f;
    timeStableCount = 0;
    timeScale = 1.0f;
    pendingTimeAdjust = 0.0f;
    finishedTimeScale = true;
    timeLocked = false;
    clientLag = 0;
    clientDisconnectStartTime = 0.0f;
    clientTurnHeldBack = false;

    for (i = 0; i < 2; i++)
    {
        netLocalPos[i] = gVectorZero;
    }
    for (i = 0; i < 2; i++)
    {
        clientBootVoted[i] = false;
    }

    NetQOS::Create(bMission);
    NetDrag::Create(bMission);
    NetComm::Create(bMission);
    NetSetup::Create(bMission);
    NetAdmin::Create(bMission);

    if (IsOneWorld())
    {
        dragEnabledHost = false;
    }

    PblValues::Record('x', PVT_FULL);

    if (IsEnabled())
    {
        if (NetComm::IsHost() && IsDedicated())
        {
            int aValue[2];
            char const* pcName;
            MissionPlayList::GetEntry(MissionPlayList::GetCurrentEntry(), pcName,
                                      aValue[0], aValue[1]);
            for (int j = 0; j < 2; j++)
            {
                if (aValue[j] != 0)
                {
                    Team::sTeamMaxReinforcementsOverride[j] = aValue[j];
                }
            }
        }

        hostGame = 0;
        joinGame = 0;

        if (netGameCount == 1)
        {
            NetLaunchGame* pLaunch = NetSetup::LaunchGame(IsHost());
            while (true)
            {
                if (pLaunch->IsDone())
                {
                    break;
                }
                pLaunch->Update();
            }
            delete pLaunch;
        }
    }

    netIsClient = !(NetComm::IsHost() && IsDedicated());

    if (netEnabledNext)
    {
        PblRandom::GetSim().SetSeed(0);
        netEntities = new NetEntityList;
        hostPlayingMask = 0;
        for (i = 0; i < 64; i++)
        {
            if (!NetComm::IsConnected(i))
            {
                continue;
            }

            bool bPlaying = false;
            for (int j = 0; j < 2; j++)
            {
                if (i == (unsigned int)NetComm::GetLocalIndex(j))
                {
                    bPlaying = true;
                }
            }
            if (netGameCount == 1)
            {
                bPlaying = true;
            }
            if (bPlaying)
            {
                hostPlayingMask |= (unsigned __int64)1 << i;
            }
        }
    }
    else
    {
        netEntities = 0;
    }

    if (IsHost())
    {
        PblJournal::Record(hostPlayingMask, PJT_MOVES);
        for (i = 0; i < 64; i++)
        {
            if (IsPlaying(i, false))
            {
                NetArch arch = NetComm::GetArch(i);
                NetCommP::SetArch(i, arch);
            }
        }
        clientPlayingMask = hostPlayingMask;
        hostNextPlayingMask = hostPlayingMask;

        if (bMission)
        {
            for (i = 0; i < PIPE_MAX; i++)
            {
                NetComm::GetTicket(TKT_UPDATE, (PipeType)i);
            }
        }

        for (i = 0; i < 1; i++)
        {
            int iIndex = GetLocalIndex(i);
            if (iIndex >= 0 && g_SpectatorEnabled[i])
            {
                SetSpectator(iIndex, 0);
            }
        }
    }

    if (netHiRes)
    {
        NetPacket::SetHiRes(true);
    }

    FreeAllChatMessageQue();

    if (bMission)
    {
        char acFileName[64];
        int iFile = 1;
        while (true)
        {
            _snprintf(acFileName, sizeof(acFileName) - 1, "chat%d.txt", iFile);
            acFileName[sizeof(acFileName) - 1] = 0;
            FILE* pFile = fopen(acFileName, "rt");
            if (!pFile)
            {
                break;
            }
            fseek(pFile, 0, SEEK_END);
            bool bEmpty = ftell(pFile) == 0;
            fclose(pFile);
            if (bEmpty && remove(acFileName) == 0)
            {
                break;
            }
            iFile++;
        }

        if (iFile <= 20)
        {
            chatLog = fopen(acFileName, "wt");
        }
        else
        {
            chatLog = 0;
        }
    }

    if (!netEnabledNext && bMission)
    {
        SetPreGame(false);
    }
}

class NetObjMap
{
public:
    void Add(NetObj* object, unsigned int uniqId)
    {
        objMap[uniqId] = object;
        object->uniqId = uniqId;
    }

    NetObj* Find(unsigned int uniqId)
    {
        if (uniqId == 63)
        {
            return 0;
        }
        return objMap[uniqId];
    }

    void Remove(NetObj* object)
    {
        objMap[object->uniqId] = 0;
    }

    void Clear()
    {
        memset(objMap, 0, sizeof(objMap));
    }

    int GetIndex(unsigned int uniqId);

    NetObj* objMap[64];
};

// GLOBAL: SWBF2 0x01fa6608
NetObjMap clientObjMap;
// GLOBAL: SWBF2 0x01e64bd0
unsigned int suicideChangeCounts[64];
struct SuicideInfo
{
    int changeCount;
    unsigned int changedTurn;
    bool killMeNow;
};

// GLOBAL: SWBF2 0x01e62fa0
SuicideInfo suicideInfo[2];

extern bool netAutoAssignTeamsOriginalSetting;

void RestoreSessionRuleDefaults();

void CleanFrameLock();

// FUNCTION: SWBF2 0x005b64b0
void NetGame::Reset()
{
    int i;
    clientObjMap.Clear();
    memset(clientSkipped, 0, sizeof(clientSkipped));
    clientNdObjMap.Clear();
    AtleastOnce::ResetAll();
    memset(spawnInfo, 0, sizeof(spawnInfo));
    memset(suicideChangeCounts, 0, sizeof(suicideChangeCounts));
    memset(suicideInfo, 0, sizeof(suicideInfo));

    for (i = 0; i < 64; i++)
    {
        delete hostJoysticks[i];
    }

    for (i = 0; i < 8; i++)
    {
        lateState[i].Free();
    }

    netEntities = 0;
    NetQOS::Reset();
    NetDrag::Reset();
    NetComm::Reset();
    CleanFrameLock();

    PblValues::Record(0xdd, PVT_FULL);

    CloseJournal();

    if (!InShell() && !netEnabled)
    {
        if (!IsHost())
        {
            RestoreSessionRuleDefaults();
        }
        netCurMaxPlayers = 1;
        netCurMaxHosts = 1;
        netIsClient = true;
        netIsDedicated = false;
    }

    FreeAllChatMessageQue();

    if (chatLog)
    {
        fclose(chatLog);
        chatLog = 0;
    }

    netAutoAssignTeams = netAutoAssignTeamsOriginalSetting;
    netSetForceHumansToTeam1 = false;
    SetPreGame(false);
}

// FUNCTION: SWBF2 0x005b6690
void NetGame::BeginShell(int iStage)
{
    switch (iStage)
    {
    case 0:
        break;
    case 1:
        NetComm::BeginShell();
        NetQOS::BeginShell();
        NetDrag::BeginShell();
        NetSetup::BeginShell();
        break;
    }
}

// FUNCTION: SWBF2 0x005b66c0
void NetGame::EndShell(int iStage)
{
    switch (iStage)
    {
    case 0:
        break;
    case 1:
        NetDrag::EndShell();
        NetQOS::EndShell();
        NetComm::EndShell();
        netEnabledNext = false;
        netEnabled = false;
        netIsDedicated = false;
        break;
    }
}

// FUNCTION: SWBF2 0x005b6700
void NetGame::CreateClient()
{
    NetComm::CreateClient();
    loadGame = 0;
    postLoadHostGame = 0;
    postLoadJoinGame = 0;

    if (!InShell())
    {
        redMemoryTrace = true;

        hostSoldierPool.Set(EntitySoldier::sMemoryPool);
        hostDroidPool.Set(EntityDroid::sMemoryPool);
        hostWalkerPool.Set(EntityWalker::sMemoryPool);
        hostCommandWalkerPool.Set(CommandWalker::sMemoryPool);
        hostCommandFlyerPool.Set(CommandFlyer::sMemoryPool);
        hostDroidekaPool.Set(EntityDroideka::sMemoryPool);
        hostFlyerPool.Set(EntityFlyer::sMemoryPool);
        hostCarrierPool.Set(EntityCarrier::sMemoryPool);
        hostHoverPool.Set(EntityHover::sMemoryPool);
        hostCommandHoverPool.Set(CommandHover::sMemoryPool);
        hostMountedTurretPool.Set(MountedTurret::sMemoryPool);
        hostPowerupPool.Set(PowerupItem::sMemoryPool);
        hostWalkerLegPairPool.Set(WalkerLegPair::sMemoryPool);
        hostWalkerBlendPool.Set(WalkerBlend::sMemoryPool);
        hostWalkerBlendUnitPool.Set(WalkerBlendUnit::sMemoryPool);
        hostAimerPool.Set(Aimer::sMemoryPool);
        hostWeaponPool.Set(Weapon::sMemoryPool);
        hostAmmoCounterPool.Set(AmmoCounter::sMemoryPool);
        hostEnergyBarPool.Set(EnergyBar::sMemoryPool);
        hostTreeGridPool.Set(TreeGridStack::sMemoryPool);
        hostOrdnanceTowCablePool.Set(OrdnanceTowCable::sMemoryPool);
        hostAsteroidPool.Set(Asteroid::sMemoryPool);
        hostFlagPool.Set(FlagItem::sMemoryPool);
        hostRemoteTerminalPool.Set(EntityRemoteTerminal::sMemoryPool);

        if (!IsOneWorld())
        {
            unsigned int uTurrets = hostMountedTurretPool.GetCount();
            unsigned int uAimers = hostAimerPool.GetCount();
            unsigned int uWeapons = hostWeaponPool.GetCount();
            unsigned int uAmmoCounters = hostAmmoCounterPool.GetCount();
            unsigned int uEnergyBars = hostEnergyBarPool.GetCount();
            unsigned int uTreeGrids = hostTreeGridPool.GetCount();
            unsigned int uRemoteTerminals = hostRemoteTerminalPool.GetCount();
            clientMountedTurretPool.Setup(uTurrets, 0x2C0, 0);
            clientAimerPool.Setup(uAimers, 0x220, 0);
            clientWeaponPool.Setup(uWeapons, hostWeaponPool.GetSize(), 0);
            clientAmmoCounterPool.Setup(uAmmoCounters, hostAmmoCounterPool.GetSize(), 0);
            clientEnergyBarPool.Setup(uEnergyBars, hostEnergyBarPool.GetSize(), 0);
            clientTreeGridPool.Setup(uTreeGrids, hostTreeGridPool.GetSize(), 0);
            clientRemoteTerminalPool.Setup(uRemoteTerminals, hostRemoteTerminalPool.GetSize(), 0);
        }

        if (!netEnabledNext)
        {
            if (!IsOneWorld())
            {
                CreateClientCollision(true);
                ChunkClusterManager::CreateClient(true);
            }
            return;
        }

        if (NetComm::IsHost())
        {
            postLoadHostGame = NetSetup::PostLoadHostGame();
        }
        else
        {
            postLoadJoinGame = NetSetup::PostLoadJoinGame();
        }

        if (IsOneWorld() && IsHost())
        {
            CreateClientCollision(false);
            return;
        }

        Thread::EnterClient();
        netClientActive = true;
        CollisionManager::CreateClient();
        netClientActive = false;
        ChunkClusterManager::CreateClient(false);
        Team::CreateClient();
        SpawnManager::CreateClient();
        Tracker::CreateClient();
        if (!IsOneWorld())
        {
            RedSceneManager::LeaveClient();
        }
        CameraManager::CreateClient();
        if (!IsOneWorld())
        {
            CommandPost::CreateClient();
        }
        PAManager::CreateClient();
        TriggerRegionManager::CreateClient();
        Thread::LeaveClient();

        SpawnDisplay::CreateClient();
        DustEffectClass::CreateClient();
        LeafParticleManager::CreateClient();
        BirdParticleManager::CreateClient();
        FishParticleManager::CreateClient();
        LightningBoltEffect::CreateClient();
        LeafPatchClass::CreateClient();
        GrassPatchClass::CreateClient();
        TargetManager::CreateClient();
        ObjectiveManager::CreateClient();
        RedShadowProxy::RemoveFromScene2();
        EnterClient();
        RedShadowProxy::AddToScene2();
        LeaveClient();
        CreateClientCollision(false);
    }

    netClientCreated = true;
}

// FUNCTION: SWBF2 0x005b6b00
void NetGame::DestroyClient()
{
    netClientCreated = false;
    if (InShell())
    {
        return;
    }

    hostSoldierPool.Reset();
    hostDroidPool.Reset();
    hostWalkerPool.Reset();
    hostCommandWalkerPool.Reset();
    hostCommandFlyerPool.Reset();
    hostDroidekaPool.Reset();
    hostFlyerPool.Reset();
    hostCarrierPool.Reset();
    hostHoverPool.Reset();
    hostCommandHoverPool.Reset();
    hostMountedTurretPool.Reset();
    hostPowerupPool.Reset();
    hostWalkerLegPairPool.Reset();
    hostWalkerBlendPool.Reset();
    hostWalkerBlendUnitPool.Reset();
    hostAimerPool.Reset();
    hostWeaponPool.Reset();
    hostAmmoCounterPool.Reset();
    hostEnergyBarPool.Reset();
    hostTreeGridPool.Reset();
    hostOrdnanceTowCablePool.Reset();
    hostAsteroidPool.Reset();
    hostFlagPool.Reset();

    if (!netEnabledNext)
    {
        return;
    }

    if (!IsOneWorld() || !IsHost())
    {
        EnterClient();
        SpawnManager::DestroyClient();
        Team::DestroyClient();
        ChunkClusterManager::DestroyClient();
        CollisionManager::DestroyClient();
        if (!IsOneWorld())
        {
            CommandPost::DestroyClient();
        }
        TargetManager::DestroyClient();
        PAManager::DestroyClient();
        TriggerRegionManager::DestroyClient();
        FlagItem::DestroyClient();
        LeaveClient();
    }

    if (postLoadHostGame)
    {
        delete postLoadHostGame;
        postLoadHostGame = 0;
    }
    if (postLoadJoinGame)
    {
        delete postLoadJoinGame;
        postLoadJoinGame = 0;
    }
}

// FUNCTION: SWBF2 0x005b6d00
void NetGame::CreateVeryLast()
{
    FlagItem::CreateClient();
    Thread::EnterClient();
    SpaceAssault::CreateClient();
    Thread::LeaveClient();
}

// GLOBAL: SWBF2 0x01e64b88
float netMaxWorldXZ;
// GLOBAL: SWBF2 0x01e62f98
float netMaxWorldY;


namespace AIUtil
{
extern float gMaxFlyHeight;
}

// GLOBAL: SWBF2 0x007e8e74
int clientUploadState = 100;

// FUNCTION: SWBF2 0x005c49d0
void DisableWriteUpload()
{
    clientUploadState = 100;
}

// FUNCTION: SWBF2 0x005b6d20
void NetGame::CreateLast()
{
    int i;

    if (InShell())
    {
        return;
    }

    NetObjState::sMemoryPool.Create(0x2080, NetObjState::sMemoryPool.GetSize(),
                                    NetObjState::sMemoryPool.GetGrow());

    unsigned int uSoldiers = hostSoldierPool.GetCount();
    unsigned int uDroids = hostDroidPool.GetCount();
    unsigned int uWalkers = hostWalkerPool.GetCount();
    unsigned int uCommandWalkers = hostCommandWalkerPool.GetCount();
    unsigned int uCommandFlyers = hostCommandFlyerPool.GetCount();
    unsigned int uDroidekas = hostDroidekaPool.GetCount();
    unsigned int uFlyers = hostFlyerPool.GetCount();
    unsigned int uCarriers = hostCarrierPool.GetCount();
    unsigned int uHovers = hostHoverPool.GetCount();
    unsigned int uCommandHovers = hostCommandHoverPool.GetCount();
    unsigned int uPowerups = hostPowerupPool.GetCount();
    unsigned int uArmedDynamics = hostArmedDynamicPool.GetCount();
    unsigned int uWalkerLegPairs = hostWalkerLegPairPool.GetCount();
    unsigned int uWalkerBlends = hostWalkerBlendPool.GetCount();
    unsigned int uWalkerBlendUnits = hostWalkerBlendUnitPool.GetCount();
    unsigned int uOrdnanceTowCables = hostOrdnanceTowCablePool.GetCount();
    unsigned int uAsteroids = hostAsteroidPool.GetCount();
    unsigned int uFlags = hostFlagPool.GetCount();
    uWalkerBlends++;

    uWalkerBlendUnits++;
    uWalkerBlendUnits++;
    uWalkerBlendUnits++;

    if (!IsOneWorld())
    {
        clientSoldierPool.Setup(uSoldiers, 0xFD0, 0);
        clientDroidPool.Setup(uDroids, 0x460, 0);
        clientWalkerPool.Setup(uWalkers, 0x2100, 0);
        clientCommandWalkerPool.Setup(uCommandWalkers, 0x2110, 0);
        clientCommandFlyerPool.Setup(uCommandFlyers, 0x1DA0, 0);
        clientDroidekaPool.Setup(uDroidekas, 0x1B70, 0);
        clientFlyerPool.Setup(uFlyers, 0x1D90, 0);
        clientCarrierPool.Setup(uCarriers, 0x1DF0, 0);
        clientHoverPool.Setup(uHovers, 0x1D10, 0);
        clientCommandHoverPool.Setup(uCommandHovers, 0x1D20, 0);
        clientPowerupPool.Setup(uPowerups, 0x290, 0);
        clientArmedDynamicPool.Setup(uArmedDynamics, 0x6B0, 0);
        clientWalkerLegPairPool.Setup(uWalkerLegPairs, 0x130, 0);
        clientWalkerBlendPool.Setup(uWalkerBlends, 0x4C, 0);
        clientWalkerBlendUnitPool.Setup(uWalkerBlendUnits, 0x133C, 0);
        clientOrdnanceTowCablePool.Setup(uOrdnanceTowCables, 0x670, 0);
        clientAsteroidPool.Setup(uAsteroids, 0x280, 0);
        clientFlagPool.Setup(uFlags, 0x450, 0);
    }

    hostSoldierPool.Reset();
    hostDroidPool.Reset();
    hostWalkerPool.Reset();
    hostCommandWalkerPool.Reset();
    hostCommandFlyerPool.Reset();
    hostDroidekaPool.Reset();
    hostFlyerPool.Reset();
    hostCarrierPool.Reset();
    hostHoverPool.Reset();
    hostCommandHoverPool.Reset();
    hostPowerupPool.Reset();
    hostArmedDynamicPool.Reset();
    hostWalkerLegPairPool.Reset();
    hostWalkerBlendPool.Reset();
    hostWalkerBlendUnitPool.Reset();
    hostMountedTurretPool.Reset();
    hostAimerPool.Reset();
    hostWeaponPool.Reset();
    hostAmmoCounterPool.Reset();
    hostEnergyBarPool.Reset();
    hostTreeGridPool.Reset();
    hostOrdnanceTowCablePool.Reset();
    hostAsteroidPool.Reset();
    hostFlagPool.Reset();

    NetObjStateMap* apLast[64];
    NetObjStateMap* apNext[64];
    for (i = 0; i < 64; i++)
    {
        apLast[i] = NetObjStateMap::Alloc();
        apNext[i] = NetObjStateMap::Alloc();
    }
    for (i = 0; i < 64; i++)
    {
        apLast[i]->Free();
        apNext[i]->Free();
    }

    if (netEnabledNext && IsHost())
    {
        for (i = 0; i < netCurMaxPlayers; i++)
        {
            if (IsPlaying(i, false))
            {
                netPlayers[i].lastObjState = NetObjStateMap::Alloc();
                netPlayers[i].nextObjState = NetObjStateMap::Alloc();
            }
        }
    }

    redMemoryTrace = false;

    PblBox box = RedTerrain::GetBoundingBox();
    netMaxWorldXZ = Max(Max(-box.GetMin().GetX(), -box.GetMin().GetZ()),
                               Max(box.GetMax().GetX(), box.GetMax().GetZ()));
    netMaxWorldY = Max(Max(500.0f, AIUtil::gMaxFlyHeight),
                              Max(-box.GetMin().GetY(), box.GetMax().GetY()));

    if (netEnabledNext)
    {
        loadGame = NetSetup::LoadGame(IsHost());
    }

    NetArch eArch = NetComm::GetArch(64);
    if (eArch != NET_ARCH_PC && eArch != NET_ARCH_XBOX)
    {
        DisableWriteUpload();
    }
}

// FUNCTION: SWBF2 0x005b7430
unsigned int NetGame::GetNumLocalPlayers()
{
    return GameLoop::GetNumCameras();
}

// FUNCTION: SWBF2 0x005b73b0
int NetGame::GetLocalIndex(int iLocal)
{
    return NetComm::GetLocalIndex(iLocal);
}

// FUNCTION: SWBF2 0x005b7440
Character* NetGame::GetLocalPlayer(unsigned int uLocal)
{
    if (uLocal >= GameLoop::GetNumCameras())
    {
        return 0;
    }
    return Character::FindPlayer(GetLocalIndex(uLocal));
}

// FUNCTION: SWBF2 0x005b75b0
void NetGame::EnterClient()
{
    if (!netEnabledNext)
    {
        return;
    }

    if (IsOneWorld() && IsHost())
    {
        return;
    }

    netClientActive = true;

    Thread::EnterClient();
    if (!IsOneWorld())
    {
        RedSceneManager::EnterClient();
    }
    CollisionManager::EnterClient();
    ChunkClusterManager::EnterClient();
    ControllerManager::EnterClient();
    Tracker::EnterClient();
    Team::EnterClient();
    Character::EnterClient();
    MissionTimer::EnterClient();
    SpawnManager::EnterClient();
    CollisionModel::EnterClient();
    CollisionPrimitive::EnterClient();
    VehicleManager::EnterClient();
    TeamManager::EnterClient();
    NetRandom::EnterClient();
    if (!IsOneWorld())
    {
        CommandPost::EnterClient();
    }
    TargetManager::EnterClient();
    PAManager::EnterClient();
    EntityDoor::EnterClient();
    ObjectiveManager::EnterClient();
    FlagItem::EnterClient();

    if (!IsOneWorld())
    {
        hostSoldierPool.Set(EntitySoldier::sMemoryPool);
        hostDroidPool.Set(EntityDroid::sMemoryPool);
        hostWalkerPool.Set(EntityWalker::sMemoryPool);
        hostCommandWalkerPool.Set(CommandWalker::sMemoryPool);
        hostCommandFlyerPool.Set(CommandFlyer::sMemoryPool);
        hostDroidekaPool.Set(EntityDroideka::sMemoryPool);
        hostFlyerPool.Set(EntityFlyer::sMemoryPool);
        hostCarrierPool.Set(EntityCarrier::sMemoryPool);
        hostHoverPool.Set(EntityHover::sMemoryPool);
        hostCommandHoverPool.Set(CommandHover::sMemoryPool);
        hostMountedTurretPool.Set(MountedTurret::sMemoryPool);
        hostPowerupPool.Set(PowerupItem::sMemoryPool);
        hostArmedDynamicPool.Set(EntityBuildingArmedDynamic::sMemoryPool);
        hostWalkerLegPairPool.Set(WalkerLegPair::sMemoryPool);
        hostWalkerBlendPool.Set(WalkerBlend::sMemoryPool);
        hostWalkerBlendUnitPool.Set(WalkerBlendUnit::sMemoryPool);
        hostAimerPool.Set(Aimer::sMemoryPool);
        hostWeaponPool.Set(Weapon::sMemoryPool);
        hostAmmoCounterPool.Set(AmmoCounter::sMemoryPool);
        hostEnergyBarPool.Set(EnergyBar::sMemoryPool);
        hostTreeGridPool.Set(TreeGridStack::sMemoryPool);
        hostOrdnanceTowCablePool.Set(OrdnanceTowCable::sMemoryPool);
        hostAsteroidPool.Set(Asteroid::sMemoryPool);
        hostFlagPool.Set(FlagItem::sMemoryPool);
        hostRemoteTerminalPool.Set(EntityRemoteTerminal::sMemoryPool);
    }

    if (!IsOneWorld())
    {
        EntitySoldier::sMemoryPool.Set(clientSoldierPool);
        EntityDroid::sMemoryPool.Set(clientDroidPool);
        EntityWalker::sMemoryPool.Set(clientWalkerPool);
        CommandWalker::sMemoryPool.Set(clientCommandWalkerPool);
        CommandFlyer::sMemoryPool.Set(clientCommandFlyerPool);
        EntityDroideka::sMemoryPool.Set(clientDroidekaPool);
        EntityFlyer::sMemoryPool.Set(clientFlyerPool);
        EntityCarrier::sMemoryPool.Set(clientCarrierPool);
        EntityHover::sMemoryPool.Set(clientHoverPool);
        CommandHover::sMemoryPool.Set(clientCommandHoverPool);
        MountedTurret::sMemoryPool.Set(clientMountedTurretPool);
        PowerupItem::sMemoryPool.Set(clientPowerupPool);
        EntityBuildingArmedDynamic::sMemoryPool.Set(clientArmedDynamicPool);
        WalkerLegPair::sMemoryPool.Set(clientWalkerLegPairPool);
        WalkerBlend::sMemoryPool.Set(clientWalkerBlendPool);
        WalkerBlendUnit::sMemoryPool.Set(clientWalkerBlendUnitPool);
        Aimer::sMemoryPool.Set(clientAimerPool);
        Weapon::sMemoryPool.Set(clientWeaponPool);
        AmmoCounter::sMemoryPool.Set(clientAmmoCounterPool);
        EnergyBar::sMemoryPool.Set(clientEnergyBarPool);
        TreeGridStack::sMemoryPool.Set(clientTreeGridPool);
        OrdnanceTowCable::sMemoryPool.Set(clientOrdnanceTowCablePool);
        Asteroid::sMemoryPool.Set(clientAsteroidPool);
        FlagItem::sMemoryPool.Set(clientFlagPool);
        EntityRemoteTerminal::sMemoryPool.Set(clientRemoteTerminalPool);
    }
}

// FUNCTION: SWBF2 0x005b7980
void NetGame::LeaveClient()
{
    if (!netEnabledNext)
    {
        return;
    }

    if (IsOneWorld() && IsHost())
    {
        return;
    }

    VehicleManager::LeaveClient();
    TeamManager::LeaveClient();
    CollisionModel::LeaveClient();
    CollisionPrimitive::LeaveClient();
    ChunkClusterManager::LeaveClient();
    Team::LeaveClient();
    Character::LeaveClient();
    MissionTimer::LeaveClient();
    SpawnManager::LeaveClient();
    Tracker::LeaveClient();
    ControllerManager::LeaveClient();
    CollisionManager::LeaveClient();
    if (!IsOneWorld())
    {
        RedSceneManager::LeaveClient();
    }
    Thread::LeaveClient();
    NetRandom::LeaveClient();
    if (!IsOneWorld())
    {
        CommandPost::LeaveClient();
    }
    TargetManager::LeaveClient();
    PAManager::LeaveClient();
    EntityDoor::LeaveClient();
    ObjectiveManager::LeaveClient();
    FlagItem::LeaveClient();

    if (!IsOneWorld())
    {
        clientSoldierPool.Set(EntitySoldier::sMemoryPool);
        clientDroidPool.Set(EntityDroid::sMemoryPool);
        clientWalkerPool.Set(EntityWalker::sMemoryPool);
        clientCommandWalkerPool.Set(CommandWalker::sMemoryPool);
        clientCommandFlyerPool.Set(CommandFlyer::sMemoryPool);
        clientDroidekaPool.Set(EntityDroideka::sMemoryPool);
        clientFlyerPool.Set(EntityFlyer::sMemoryPool);
        clientCarrierPool.Set(EntityCarrier::sMemoryPool);
        clientHoverPool.Set(EntityHover::sMemoryPool);
        clientCommandHoverPool.Set(CommandHover::sMemoryPool);
        clientMountedTurretPool.Set(MountedTurret::sMemoryPool);
        clientPowerupPool.Set(PowerupItem::sMemoryPool);
        clientArmedDynamicPool.Set(EntityBuildingArmedDynamic::sMemoryPool);
        clientWalkerLegPairPool.Set(WalkerLegPair::sMemoryPool);
        clientWalkerBlendPool.Set(WalkerBlend::sMemoryPool);
        clientWalkerBlendUnitPool.Set(WalkerBlendUnit::sMemoryPool);
        clientAimerPool.Set(Aimer::sMemoryPool);
        clientWeaponPool.Set(Weapon::sMemoryPool);
        clientAmmoCounterPool.Set(AmmoCounter::sMemoryPool);
        clientEnergyBarPool.Set(EnergyBar::sMemoryPool);
        clientTreeGridPool.Set(TreeGridStack::sMemoryPool);
        clientOrdnanceTowCablePool.Set(OrdnanceTowCable::sMemoryPool);
        clientAsteroidPool.Set(Asteroid::sMemoryPool);
        clientFlagPool.Set(FlagItem::sMemoryPool);
        clientRemoteTerminalPool.Set(EntityRemoteTerminal::sMemoryPool);
    }

    if (!IsOneWorld())
    {
        EntitySoldier::sMemoryPool.Set(hostSoldierPool);
        EntityDroid::sMemoryPool.Set(hostDroidPool);
        EntityWalker::sMemoryPool.Set(hostWalkerPool);
        CommandWalker::sMemoryPool.Set(hostCommandWalkerPool);
        CommandFlyer::sMemoryPool.Set(hostCommandFlyerPool);
        EntityDroideka::sMemoryPool.Set(hostDroidekaPool);
        EntityFlyer::sMemoryPool.Set(hostFlyerPool);
        EntityCarrier::sMemoryPool.Set(hostCarrierPool);
        EntityHover::sMemoryPool.Set(hostHoverPool);
        CommandHover::sMemoryPool.Set(hostCommandHoverPool);
        MountedTurret::sMemoryPool.Set(hostMountedTurretPool);
        PowerupItem::sMemoryPool.Set(hostPowerupPool);
        EntityBuildingArmedDynamic::sMemoryPool.Set(hostArmedDynamicPool);
        WalkerLegPair::sMemoryPool.Set(hostWalkerLegPairPool);
        WalkerBlend::sMemoryPool.Set(hostWalkerBlendPool);
        WalkerBlendUnit::sMemoryPool.Set(hostWalkerBlendUnitPool);
        Aimer::sMemoryPool.Set(hostAimerPool);
        Weapon::sMemoryPool.Set(hostWeaponPool);
        AmmoCounter::sMemoryPool.Set(hostAmmoCounterPool);
        EnergyBar::sMemoryPool.Set(hostEnergyBarPool);
        TreeGridStack::sMemoryPool.Set(hostTreeGridPool);
        OrdnanceTowCable::sMemoryPool.Set(hostOrdnanceTowCablePool);
        Asteroid::sMemoryPool.Set(hostAsteroidPool);
        FlagItem::sMemoryPool.Set(hostFlagPool);
        EntityRemoteTerminal::sMemoryPool.Set(hostRemoteTerminalPool);
    }

    netClientActive = false;
}

void BeginNetLOD();
int SetNetLOD(int precision);
namespace GameLoop
{
extern float sDeltaTime;
}

// GLOBAL: SWBF2 0x01e63220
NetPktGroupOffset lateStateOffset[8][64];
// GLOBAL: SWBF2 0x01fa87c8
GameObject* lateObjects[32];
// GLOBAL: SWBF2 0x01fa670c
unsigned int lateObjCount;
// GLOBAL: SWBF2 0x01e62f50
bool netWaitLate;

// GLOBAL: SWBF2 0x01faa6c4
int lateRecvPlayerIndex;

// GLOBAL: SWBF2 0x01faa719
bool hasPendingLateRecv;

// FUNCTION: SWBF2 0x005ba110
void NetGame::ApplyLateMoves()
{
    if (!netEnabledNext || netWaitLate || (unsigned int)hostTurnNumber < 3)
    {
        return;
    }

    if (hasPendingLateRecv)
    {
        int iSlot = (hostTurnNumber - 1) & 7;
        lateRecv[iSlot] |= (unsigned __int64)1 << lateRecvPlayerIndex;
    }

    unsigned int uTurn;
    for (uTurn = hostTurnNumber - 3; uTurn < (unsigned int)hostTurnNumber; uTurn++)
    {
        int iSlot = uTurn & 7;
        if (lateRecv[iSlot] & hostPlayingMask)
        {
            break;
        }
    }
    if (uTurn == (unsigned int)hostTurnNumber)
    {
        return;
    }

    lateTurnNumber = uTurn;
    unsigned __int64 uMask = 0;
    for (uTurn = lateTurnNumber; uTurn < (unsigned int)hostTurnNumber; uTurn++)
    {
        int iSlot = uTurn & 7;
        uMask |= lateRecv[iSlot] & hostPlayingMask;
    }

    int i;
    for (i = 0; i < netCurMaxPlayers; i++)
    {
        if ((uMask & ((unsigned __int64)1 << i)) && netPlayers[i].lateFenceTurn >= (unsigned int)lateTurnNumber)
        {
            lateTurnNumber = netPlayers[i].lateFenceTurn + 1;
        }
    }
    if ((unsigned int)lateTurnNumber >= (unsigned int)hostTurnNumber)
    {
        return;
    }

    int iGroup = lateTurnNumber & 7;
    NetPktGroup& group = lateState[iGroup];
    group.BeginRead();
    BeginNetLOD();
    SetNetLOD(1);
    lateObjCount = 0;
    for (i = 0; i < netCurMaxPlayers; i++)
    {
        if (uMask & ((unsigned __int64)1 << i))
        {
            Character* pCharacter = Character::FindPlayer(i);
            if (!pCharacter)
            {
                continue;
            }
            Controllable* pControllable = pCharacter->GetUnit();
            if (!pControllable)
            {
                continue;
            }
            if (pCharacter->GetVehicle())
            {
                pControllable = pCharacter->GetVehicle();
            }
            GameObject* pObject = pControllable->GetGameObject();
            lateObjects[lateObjCount++] = pObject;
            group.Seek(lateStateOffset[iGroup][i]);
            pObject->ReadLateState(group);
        }
    }
    group.EndRead();

    netIsLateTurn = true;
    float fDeltaTime = netSecondsPerTurn;
    GameLoop::sDeltaTime = fDeltaTime;
    while ((unsigned int)lateTurnNumber < (unsigned int)hostTurnNumber)
    {
        int iTurn = lateTurnNumber & 0x1f;
        for (i = 0; i < netCurMaxPlayers; i++)
        {
            if (NetGame::IsPlaying(i, false))
            {
                *hostJoysticks[i] = hostTurns[iTurn].moves[i];
            }
        }
        ControllerManager::Update(fDeltaTime);
        Thread::UpdateAll(fDeltaTime, false, false);
        iTurn = lateTurnNumber & 7;
        lateRecv[iTurn] &= ~uMask;
        lateTurnNumber++;
    }
    netIsLateTurn = false;
}

// FUNCTION: SWBF2 0x005ba530
void NetGame::AddLateFence(int iPlayer)
{
    if (netEnabledNext && iPlayer >= 0 && !netClientActive)
    {
        netPlayers[iPlayer].lateFenceTurn = hostTurnNumber;
    }
}

// FUNCTION: SWBF2 0x005bad20
void NetGame::Created(GameObject* pObject)
{
    if (netEnabledNext && !IsClient())
    {
        NetEntity& entity = netEntities->Add();
        entity.obj = pObject;
        entity.type = pObject->GetEntityType();
        entity.pos = pObject->GetMatrix().GetTranslation();
        entity.updateTurn = hostTurnNumber;
    }
}

// FUNCTION: SWBF2 0x005badb0
void NetGame::Destroyed(GameObject* pObject)
{
    if (!netEnabledNext)
    {
        return;
    }

    if (IsClient())
    {
        if (!destroyOk)
        {
            for (;;)
            {
            }
        }
        return;
    }

    for (NetEntityList::Itor it(*netEntities); it; ++it)
    {
        NetEntity& entity = *it;
        if (entity.obj == pObject)
        {
            netEntities->Remove(entity);
            return;
        }
    }
}

// FUNCTION: SWBF2 0x005ba750
float NetGame::GetLocalTime()
{
    if (!netEnabledNext)
    {
        return singleTime;
    }

    return ((float)clientTurnNumber + turnRatio) * netSecondsPerTurn;
}

// FUNCTION: SWBF2 0x005bf3c0
void ClearNetEvents()
{
    netEventHead = 0;
}

// FUNCTION: SWBF2 0x005be550
void NetGame::CreateEvent_CreateOrdnance(const OrdnanceClass* pClass,
                                         const OrdnanceDesc& desc)
{
    sServerNetEvents[netEventHead].mType = 0;
    NetEvent::U::OrdnanceEvent& data = sServerNetEvents[netEventHead].u.ordnance;
    data.mPosition.Set(desc.mPosition);
    data.mDirection.Set(desc.mDirection);
    data.mStrength = desc.mStrength;
    data.mOwner.Set(desc.mOwner);
    data.mCharacter = desc.mDamageOwner.mCharacter;
    data.mGameObject = desc.mDamageOwner.mGameObject;
    data.mGameObjectId = data.mGameObject ? data.mGameObject->GetHandleId() : 0;
    data.mDamageMultiplier = desc.mDamageOwner.mDamageMultiplier;
    data.mOrdClassIndex = pClass->GetNetIndex();
    data.mWpnClassIndex = desc.mDamageOwner.mWeapon ? desc.mDamageOwner.mWeapon->GetNetIndex() : -1;
    data.mEntClassIndex = desc.mDamageOwner.mClass ? desc.mDamageOwner.mClass->GetNetIndex() : -1;
    data.mTarget = desc.mTarget;
    data.mTargetId = data.mTarget ? data.mTarget->GetHandleId() : 0;
    data.mFireWeapon = desc.mFireWeapon;
    netEventHead = (netEventHead + 1) % 128;
}

extern int curDst;
void WriteCharacter(NetPktGroup& group, const Character* pCharacter);
Character* ReadCharacter(NetPktGroup& group);

// FUNCTION: SWBF2 0x005c28e0
void EntityItem::WriteCreate(NetPktGroup& group)
{
    bool bAttach = false;
    if (mOwner && (mOwner->GetPlayerId() == curDst ||
                   NetGame::GetGuestOf(mOwner->GetPlayerId()) == curDst))
    {
        bAttach = true;
    }
    if (bAttach && mOwner)
    {
        Controllable* ctlb = mOwner->GetUnit();
        const Entity* ent = ctlb ? ctlb->GetEntity() : 0;
        if (ent && ent->IsRtti(EntitySoldier::GetRtti()))
        {
            EntitySoldier* sold = (EntitySoldier*)ent;
            int wpnCount = sold->GetWeaponCount();
            for (int i = 0; i < wpnCount; ++i)
            {
                Weapon* wpnPtr = sold->GetWeaponPtr(i);
                if (wpnPtr && wpnPtr->IsRtti(WeaponDetonator::GetRtti()))
                {
                    WeaponDetonator* detonator = static_cast<WeaponDetonator*>(wpnPtr);
                    int itemCount = *detonator->GetItemCount();
                    for (int j = 0; j < itemCount; ++j)
                    {
                        const GameObject* obj = detonator->GetItem(j);
                        if (obj == GetGameObject())
                        {
                            group.Write(bAttach);
                            WriteCharacter(group, mOwner);
                            group.WriteBits((unsigned int)i, 4);
                            return;
                        }
                    }
                }
            }
        }
    }
    group.Write(false);
}

// FUNCTION: SWBF2 0x005c2ad0
void EntityItem::ReadCreate(NetPktGroup& group)
{
    bool bAttach;
    group.Read(bAttach);
    if (bAttach)
    {
        mOwner = ReadCharacter(group);
        unsigned int wpnIndex;
        group.ReadBits(wpnIndex, 4);
        if (mOwner)
        {
            Controllable* ctlb = mOwner->GetUnit();
            Entity* ent = ctlb ? ctlb->GetEntity() : 0;
            if (ent && ent->IsRtti(EntitySoldier::GetRtti()))
            {
                EntitySoldier* sold = static_cast<EntitySoldier*>(ent);
                Weapon* wpnPtr = sold->GetWeaponPtr((int)wpnIndex);
                if (wpnPtr && wpnPtr->IsRtti(WeaponDetonator::GetRtti()))
                {
                    WeaponDetonator* detonator = static_cast<WeaponDetonator*>(wpnPtr);
                    detonator->NetAttachItem(this);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c3840
int NetGame::GetGuestOf(int iPlayer)
{
    if (iPlayer < 0 || iPlayer >= 64)
    {
        return -1;
    }
    return netPlayers[iPlayer].guestOf;
}

// FUNCTION: SWBF2 0x005ce870
void NetGame::HeroKilled(int iTeam)
{
    if (!NetGame::IsHost() || NetGame::IsClient())
    {
        return;
    }
    if (iTeam < 0 || iTeam >= 3)
    {
        return;
    }

    if (netHeroPlayerID[iTeam] >= 0)
    {
        if (NetGame::IsEnabled())
        {
            netHeroSndPlayHostDefeat[iTeam] = true;
        }
        else
        {
            Character* c = Character::FindPlayer(netHeroPlayerID[iTeam]);
            if (c)
            {
                HeroMessageDisplay::PlayHeroSoundDefeat(iTeam);
            }
        }

        if (netHeroRuleTeam == 4 && (iTeam == 1 || iTeam == 2))
        {
            netHeroUnlocked[iTeam] = false;
            netHeroUnlocked[3 - iTeam] = true;
            netHeroFirstUnlocked[3 - iTeam] = true;
        }
    }
    else if (!NetGame::IsEnabled() && netHeroAIID[iTeam] != -1)
    {
        Character* c = Character::GetPtr(netHeroAIID[iTeam]);
        if (c)
        {
            HeroMessageDisplay::PlayHeroSoundDefeat(iTeam);
        }
    }

    netHeroLastPlayerID[iTeam] = netHeroPlayerID[iTeam];
    netHeroPlayerID[iTeam] = -1;
    netHeroNextPlayerID[iTeam] = -1;
    netHeroAIID[iTeam] = -1;
    netHeroDiedTurn[iTeam] = NetGame::GetTurnNumber();
    spHeroDiedTime[iTeam] = NetGame::GetLocalTime();
    netHeroSpawnedTurn[iTeam] = 0;
}

enum ReliableEventType
{
    REL_MESSAGE = 0,
    REL_CHANGETEAM = 1,
    REL_CHANGECOMMANDPOSTTEAMS = 2,
    REL_CREATE_CHUNKCLUSTER = 3,
    REL_CORRECT_CHUNKCLUSTER = 4,
    REL_CONNECT_TOWCABLE = 5,
    REL_DESTROY_TOWCABLE = 6,
    REL_GAMEOVER = 7,
    REL_SCRIPTMESSAGE = 8,
    REL_PLAYVOICEOVER = 9,
    REL_UPDATEDOORSTATE = 10,
    REL_ADDOBJECTIVE = 11,
    REL_UPDATEOBJECTIVE = 12,
    REL_UPDATEINVINCIBILITY = 13
};

struct ReliableNetEvent
{
    ReliableEventType mEvType;
    unsigned __int64 mSendMask;
    union U
    {
        struct MessageEvent
        {
            NetGame::MessageType mType;
            const Character* mCharacter1;
            const Character* mCharacter2;
            CommandPost* mCommandPost;
            EntityClass* mEntClass;
            WeaponClass* mWpnClass;
            Team* mTeam;
        } message;

        struct ChangeTeamEvent
        {
            CommandPost* mCommandPost;
            Team* mTeam;
            bool mAlive;
        } changeteam;

        struct ConnectTowCableEvent
        {
            NetEventVector mDirection;
            NetGameObjectHandle mOwnerObject;
            Weapon* mFireWeapon;
            GameObject* mCollideWalker;
            NetEventVector mCollidePosition;
            int mCollideBodyID;
        } connectTowCable;
        struct DestroyTowCableEvent
        {
            NetGameObjectHandle mOwnerObject;
            Weapon* mFireWeapon;
        } destroyTowCable;
        struct GameOverEvent
        {
            Team* mTeam;
        } gameover;
        struct PlayVoiceOverEvent
        {
            unsigned int mSoundNameHashID;
            unsigned int mTeamMask;
        } playVoiceOver;
        struct ScriptMessageEvent
        {
            unsigned int mLocalizeKey;
            unsigned int mTeamMask;
        } scriptMessage;
        struct UpdateDoorStateEvent
        {
            unsigned int mHashID;
            bool mIsLocked;
        } updateDoorState;
        struct UpdtateObjectiveEvent
        {
            unsigned int mHashID;
            HUD::Objective::ObjectiveState state;
        } updateObjective;
        struct AddObjectiveEvent
        {
            unsigned int mHashID;
            unsigned int mPopupTextID;
            int team;
            HUD::Objective::ObjectiveState state;
        } addObjective;
        struct UpdateInvincibilityEvent
        {
            Character* mCharacter;
            bool mIsInvincible;
        } updateInvincibility;
        struct ChangeCommandPostTeamsEvent
        {
            unsigned int mTeamBits;
            unsigned int mAliveBits;
        } changecommandpostteams;
        struct CreateChunkClusterEvent
        {
            EntityGeometryClass* mEntClass;
            unsigned int mChunkClusterIndex;
        } createchunkcluster;
        struct CorrectChunkClusterEvent
        {
            EntityClass* mEntClass;
            unsigned int mChunkClusterIndex;
            unsigned int mChunkIndex;
        } correctchunkcluster;
    } u;
};
 
 
 

// GLOBAL: SWBF2 0x01fa6710
ReliableNetEvent reliableEvents[128];

// FUNCTION: SWBF2 0x005bf490
void AdvanceReliableTail(unsigned int uIndex)
{
    if (uIndex != (unsigned int)reliableNetEventTail)
    {
        return;
    }
    while (reliableNetEventTail != reliableNetEventHead &&
           !reliableEvents[reliableNetEventTail].mSendMask)
    {
        if ((unsigned int)++reliableNetEventTail >= 128)
        {
            reliableNetEventTail = 0;
        }
    }
}

// FUNCTION: SWBF2 0x005bf3d0
ReliableNetEvent& GetNextReliableEvent()
{
    AdvanceReliableTail(reliableNetEventTail);
    ReliableNetEvent& event = reliableEvents[reliableNetEventHead++];
    if ((unsigned int)reliableNetEventHead >= 128)
    {
        reliableNetEventHead = 0;
    }

    unsigned __int64 uMask = 0;
    for (int i = 0; i < netCurMaxPlayers; i++)
    {
        if (NetGame::IsPlaying(i, false))
        {
            uMask |= (unsigned __int64)1 << i;
        }
    }
    event.mSendMask = uMask;
    return event;
}

void HandleEvent_Message(ReliableNetEvent& event);

// FUNCTION: SWBF2 0x005bfd50
void NetGame::CreateEvent_Message(MessageType eType, const Character* pA, const Character* pB,
                                  EntityClass* pClass, WeaponClass* pWeapon, Team* pTeam,
                                  CommandPost* pPost)
{
    ReliableNetEvent& event = GetNextReliableEvent();
    event.mEvType = REL_MESSAGE;
    event.u.message.mType = eType;
    event.u.message.mCharacter1 = pA;
    event.u.message.mCharacter2 = pB;
    event.u.message.mEntClass = pClass;
    event.u.message.mWpnClass = pWeapon;
    event.u.message.mTeam = pTeam;
    event.u.message.mCommandPost = pPost;
    if (!IsEnabled() || (IsOneWorld() && IsHost() && !IsDedicated()))
    {
        HandleEvent_Message(event);
    }
}

// FUNCTION: SWBF2 0x005c0670
void NetGame::CreateEvent_ChangeTeam(CommandPost* pPost, unsigned __int64 uMask)
{
    ReliableNetEvent& event = GetNextReliableEvent();
    event.mEvType = REL_CHANGETEAM;
    event.mSendMask &= uMask;
    event.u.changeteam.mCommandPost = pPost;
    event.u.changeteam.mTeam = Team::GetPtr(pPost->GetGameObject()->GetTeam());
    event.u.changeteam.mAlive = pPost->GetGameObject()->IsAlive();
}

// FUNCTION: SWBF2 0x005c12b0
void NetGame::CreateEvent_ConnectTowCable(GameObject* pOwner, Weapon* pWeapon, const PblVector3& vDir,
                                          const PblVector3& vPos, int iBodyId, GameObject* pWalker,
                                          unsigned __int64)
{
    ReliableNetEvent& event = GetNextReliableEvent();
    ReliableNetEvent::U::ConnectTowCableEvent& data = event.u.connectTowCable;
    event.mEvType = REL_CONNECT_TOWCABLE;
    data.mDirection.Set(vDir);
    data.mOwnerObject.Set(pOwner);
    data.mFireWeapon = pWeapon;
    data.mCollideWalker = pWalker;
    data.mCollideBodyID = iBodyId;
    data.mCollidePosition.Set(vPos);
}

// FUNCTION: SWBF2 0x005c1bc0
void NetGame::CreateEvent_DestroyTowCable(GameObject* pOwner, Weapon* pWeapon, unsigned __int64)
{
    ReliableNetEvent& event = GetNextReliableEvent();
    ReliableNetEvent::U::DestroyTowCableEvent& data = event.u.destroyTowCable;
    event.mEvType = REL_DESTROY_TOWCABLE;
    data.mOwnerObject.Set(pOwner);
    data.mFireWeapon = pWeapon;
}

namespace PblUnicode
{
unsigned int ustrncnv(unsigned short* pDest, const unsigned char* pSrc, unsigned int uiCount);
}

namespace GameVoiceChat
{
void PlayerChange();
}

// FUNCTION: SWBF2 0x005bfdf0
void HandleEvent_Message(ReliableNetEvent& event)
{
    const Character* pCharacter1 = event.u.message.mCharacter1;
    const Character* pCharacter2 = event.u.message.mCharacter2;
    CommandPost* pPost = event.u.message.mCommandPost;
    Team* pTeam = event.u.message.mTeam;
    WeaponClass* pWpnClass = event.u.message.mWpnClass;
    EntityClass* pEntClass = event.u.message.mEntClass;

    const int kMaxMessage = 256;
    unsigned short message[kMaxMessage];
    unsigned short playerName[64];
    unsigned short heroName[64];
    const unsigned short* pFormat;
    message[0] = 0;
    bool bKill = false;

    switch (event.u.message.mType)
    {
    case NetGame::MSG_SUICIDE:
        if (pCharacter1->IsHero())
        {
            pFormat = RedLocalizeDB::Find(0x07aad4c1, false);
            pCharacter1->GetHeroName(heroName, 127);
            PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, heroName);
        }
        else
        {
            pFormat = RedLocalizeDB::Find(0x616333ea, false);
            PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, pCharacter2->GetName());
        }
        break;

    case NetGame::MSG_KILLED_TEAMMATE:
    case NetGame::MSG_KILLED:
        bKill = true;
        break;

    case NetGame::MSG_DIED:
        if (pCharacter1->IsHero())
        {
            pCharacter1->GetHeroName(heroName, 127);
            if (pEntClass && pEntClass->IsRtti(EntityBuildingArmedClass::GetRtti()))
            {
                pFormat = RedLocalizeDB::Find(0x664ffe37, false);
                PblUnicode::usnprintf(message, kMaxMessage - 1, RedLocalizeDB::Find(0xacc1eef7, false),
                                      pFormat, heroName);
            }
            else
            {
                pFormat = RedLocalizeDB::Find(0x07aad4c1, false);
                PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, heroName);
            }
        }
        else if (pEntClass && pEntClass->IsRtti(EntityBuildingArmedClass::GetRtti()))
        {
            pFormat = RedLocalizeDB::Find(0x664ffe37, false);
            PblUnicode::usnprintf(message, kMaxMessage - 1, RedLocalizeDB::Find(0xacc1eef7, false),
                                  pFormat, pCharacter1->GetName());
        }
        else
        {
            pFormat = RedLocalizeDB::Find(0x0a9f4178, false);
            PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, pCharacter1->GetName());
        }
        break;

    case NetGame::MSG_CAPTURED:
        if (pPost && pTeam)
        {
            pPost->PlayVoiceOver(pTeam->GetSide(), CommandPost::VoiceOver::stream_Capture, pCharacter1 != 0);
        }
        break;

    case NetGame::MSG_LOST:
        if (pPost && pTeam)
        {
            pPost->PlayVoiceOver(pTeam->GetSide(), CommandPost::VoiceOver::stream_Lost, pCharacter1 != 0);
        }
        break;

    case NetGame::MSG_DISPUTED:
        pPost->PlayVoiceOver(pTeam->GetSide(), CommandPost::VoiceOver::stream_Dispute, false);
        break;

    case NetGame::MSG_SAVED:
        pPost->PlayVoiceOver(pTeam->GetSide(), CommandPost::VoiceOver::stream_Saved, false);
        break;

    case NetGame::MSG_JOINED:
    {
        Character* pFirst = Character::GetPtr(0);
        int iPlayer = pCharacter1 - pFirst;
        pTeam = Team::GetPtr(pCharacter1->GetTeamNumber());
        pFormat = RedLocalizeDB::Find(0xdd433dcf, false);
        if (!pFormat)
        {
            PblUnicode::ustrncnv(heroName, "%s Joined", 64);
            pFormat = heroName;
        }
        if (pFirst->IsPlayer())
        {
            GameVoiceChat::PlayerChange();
        }
        PblUnicode::ustrncnv(playerName, (const unsigned char*)NetGame::GetPlayerName(iPlayer), 64);
        const unsigned short* pName = playerName;
        PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, pName);
        if (pCharacter1 && event.u.message.mTeam && pCharacter1->GetTeamPtr() != event.u.message.mTeam)
        {
            int iIndex = pCharacter1 - pFirst;
            NetGame::SetStartTeam(iIndex, event.u.message.mTeam->GetNumber());
        }
        break;
    }

    case NetGame::MSG_LEFT:
    {
        Character* pFirst = Character::GetPtr(0);
        int iPlayer = pCharacter1 - pFirst;
        if (!pCharacter1)
        {
            return;
        }
        pTeam = Team::GetPtr(pCharacter1->GetTeamNumber());
        pFormat = RedLocalizeDB::Find(0xf2898f4f, false);
        if (!pFormat)
        {
            PblUnicode::ustrncnv(heroName, "%s Left", 64);
            pFormat = heroName;
        }

        const char* pszName = netPlayers[iPlayer].prevName;
        PblUnicode::ustrncnv(playerName, pszName, 64);
        const unsigned short* pName = playerName;
        PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, pName);
        break;
    }

    case NetGame::MSG_BOOTED:
        if (pCharacter1)
        {
            pFormat = RedLocalizeDB::Find(0xd6a2b4a1, false);
            PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, pCharacter1->GetName(),
                                  BootDisplay::Instance(0)->GetBootReasonString(NetGame::GetBootReason()));
            pTeam = pCharacter1->GetTeamPtr();
        }
        break;

    case NetGame::MSG_NOTBOOT:
        if (pCharacter1)
        {
            pFormat = RedLocalizeDB::Find(0xd5a2b30e, false);
            PblUnicode::usnprintf(message, kMaxMessage - 1, pFormat, pCharacter1->GetName());
            pTeam = pCharacter1->GetTeamPtr();
        }
        break;
    }

    if (bKill)
    {
        for (unsigned int i = 0; i < NetGame::GetNumLocalPlayers(); i++)
        {
            if (!NetGame::GetLocalPlayer(i))
            {
                continue;
            }

            unsigned int uIcon = 0;
            if (pEntClass)
            {
                if (pEntClass->IsRtti(EntitySoldierClass::GetRtti()))
                {
                    uIcon = pWpnClass ? pWpnClass->mIconTexture : 0;
                }
                else
                {
                    uIcon = pEntClass->mHealthTexture;
                }
            }
            else
            {
                uIcon = pWpnClass ? pWpnClass->mIconTexture : 0;
            }
            MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)
                ->AddKillMessage(pCharacter2, pCharacter1, uIcon, NetGame::GetLocalPlayer(i)->GetTeamPtr());
        }
    }
    else if (message[0])
    {

        if (CanProcessChatMessage() && (event.u.message.mType == NetGame::MSG_SUICIDE ||
                             event.u.message.mType == NetGame::MSG_DIED))
        {
            int iLocal = NetGame::GetLocalIndex(0);
            if (iLocal >= 0 && iLocal < 64)
            {
                if (Character::GetPtr(iLocal) == pCharacter1)
                {
                    FLInputManager::Instance()->ProcessChatMessageInput(message);
                }
            }
        }
        for (unsigned int j = 0; j < NetGame::GetNumLocalPlayers(); j++)
        {
            if (NetGame::GetLocalPlayer(j))
            {
                MessageDisplay::Instance(j, MessageDisplay::DISPLAY_SYSTEM_MSG)
                    ->AddMessage(message, NetGame::GetLocalPlayer(j)->GetTeamPtr()->GetColor(pTeam->GetNumber()));
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c35a0
bool NetGame::IsPlaying(int iPlayer, bool bNext)
{
    if (!IsEnabled())
    {
        return iPlayer < (int)GameLoop::GetNumCameras() ? 1 : 0;
    }
    unsigned __int64 mask;
    if (IsClient() || bNext)
    {
        mask = clientPlayingMask;
    }
    else
    {
        mask = hostPlayingMask;
    }
    return (mask & ((unsigned __int64)1 << iPlayer)) ? 1 : 0;
}

// FUNCTION: SWBF2 0x005c3800
void NetGame::ClearPlayerCDKey(int)
{
}

// FUNCTION: SWBF2 0x005c3810
void NetGame::ClearAllCDKeyInfo()
{
    for (int i = 0; i < 64; ++i)
    {
        NetGame::ClearPlayerCDKey(i);
    }
}

// FUNCTION: SWBF2 0x005c3950
int NetGame::GetSpectator(int iPlayer)
{
    if (iPlayer < 0)
    {
        return -1;
    }
    return netPlayers[iPlayer].mSpectateID;
}

// FUNCTION: SWBF2 0x005c3970
bool NetGame::IsSpectator(int iPlayer)
{
    return GetSpectator(iPlayer) != -1;
}

// FUNCTION: SWBF2 0x005c3ea0
bool NetGame::IsPreGame()
{
    return netPreGameEnabled;
}

// FUNCTION: SWBF2 0x005c3eb0
void NetGame::SetPreGame(bool bEnable)
{
    netPreGameEnabled = bEnable;

    float fTime = IsEnabled() ? NetComm::GetTime() : GetLocalTime();

    if (netPreGameEnabled)
    {
        netPreGameStartTime = fTime;
    }
    else
    {
        netHeroUnlockStartTimer = fTime;
    }
}

// FUNCTION: SWBF2 0x005c4060
int NetGame::GetMaxPreGameTime()
{
    return netPreGameTimeMax;
}

// FUNCTION: SWBF2 0x005c4070
void NetGame::SetMaxPreGameTime(int iSeconds)
{
    netPreGameTimeMax = iSeconds;
}

// GLOBAL: SWBF2 0x007e8e7a
bool netAwardsEnabled = true;

// FUNCTION: SWBF2 0x005d3690
bool GetNetAwardsEnabled()
{
    return netAwardsEnabled;
}

// FUNCTION: SWBF2 0x005d36a0
void SetNetAwardsEnabled(int value)
{
    *(unsigned char*)&netAwardsEnabled = (unsigned char)value;
}

// FUNCTION: SWBF2 0x005c4610
bool NetGame::IsJournal()
{
    return journalMode != 0;
}

// FUNCTION: SWBF2 0x005cec20
bool NetGame::AreHeroRulesRunning(int iTeam)
{
    if (IsPreGame())
    {
        return false;
    }
    if (!Team::GetPtr(iTeam))
    {
        return false;
    }
    if (!Team::GetPtr(iTeam)->GetSpecialClassCount())
    {
        return false;
    }
    if (!netHeroesEnabled)
    {
        return false;
    }

    if (!IsEnabled())
    {
        for (unsigned int i = 0; i < GameLoop::GetNumCameras(); i++)
        {
            if (SpawnDisplay::Instance(i)->IsSelectingTeam())
            {
                return false;
            }
        }
    }
    return true;
}

float GetTimeSinceHeroSelected(int iTeam);
float GetTimeSinceHeroDied(int iTeam);

void ClearTeamDeclinedHero(int iTeam);

// FUNCTION: SWBF2 0x005cea20
void ClearTeamDeclinedHero(int iTeam)
{
    for (int i = 0; i < 64; ++i)
    {
        Character* pCharacter = Character::FindPlayer(i);
        if (pCharacter && pCharacter->GetTeamNumber() == iTeam)
        {
            netPlayers[i].mDeclinedHero = false;
        }
    }
}



extern int netHeroRulePlayer;

// FUNCTION: SWBF2 0x005cedc0
void NetGame::HeroRulesUpdate(int iTeam)
{
    if (!IsHost() || IsClient())
    {
        return;
    }
    if (!AreHeroRulesRunning(iTeam))
    {
        return;
    }

    if (netHeroUnlocked[iTeam])
    {
        if (netHeroPlayerID[iTeam] == -1 && netHeroNextPlayerID[iTeam] != -1)
        {
            Character* pPlayer = Character::FindPlayer(netHeroNextPlayerID[iTeam]);
            if (pPlayer && !pPlayer->GetUnit())
            {
                AcceptHero(iTeam, netHeroNextPlayerID[iTeam]);
            }
        }
        if (netHeroSelectedTurn[iTeam])
        {
            float fTime = GetTimeSinceHeroSelected(iTeam);
            if (fTime > 30.0f)
            {
                DeclineHero(iTeam, netHeroNextPlayerID[iTeam]);
            }
        }

        if (netHeroNextPlayerID[iTeam] == -1 && netHeroAIID[iTeam] == -1)
        {
            if (netHeroRulePlayer == 8 && netHeroKillerID[iTeam] != -1)
            {
                Character* pNext = Character::GetPtr(netHeroKillerID[iTeam]);
                if (pNext && pNext->GetTeamNumber() == iTeam)
                {
                    netHeroNextPlayerID[iTeam] = netHeroKillerID[iTeam];
                    HeroSelected(iTeam);
                    netHeroKillerID[iTeam] = -1;
                }
            }

            if (netHeroNextPlayerID[iTeam] == -1 && netHeroAIID[iTeam] == -1 &&
                (netHeroFirstUnlocked[iTeam] || netHeroRuleRespawn != 1))
            {
                if (netHeroFirstUnlocked[iTeam] ||
                    (netHeroDiedTurn[iTeam] && GetTimeSinceHeroDied(iTeam) > (float)netHeroRuleRespawnVal))
                {
                    int iMaxPlayers = IsEnabled() ? netCurMaxPlayers : GameLoop::GetNumCameras();

                    bool bAllDeclined = true;
                    int i = 0;
                    for (i = 0; i < iMaxPlayers; i++)
                    {
                        Character* pCharacter = Character::GetPtr(i);
                        if (pCharacter && pCharacter->GetTeamNumber() == iTeam &&
                            !netPlayers[i].mDeclinedHero)
                        {
                            bAllDeclined = false;
                            break;
                        }
                    }

                    if (bAllDeclined)
                    {
                        netHeroPlayerID[iTeam] = -1;
                        netHeroNextPlayerID[iTeam] = -1;
                        netHeroAIID[iTeam] = -1;
                        netHeroDiedTurn[iTeam] = NetGame::GetTurnNumber();
                        spHeroDiedTime[iTeam] = GetLocalTime();
                        netHeroSpawnedTurn[iTeam] = 0;
                        if (netHeroRuleRespawn == 1)
                        {
                            netHeroUnlocked[iTeam] = false;
                            netHeroFirstUnlocked[iTeam] = false;
                        }
                        else
                        {
                            netHeroFirstUnlocked[iTeam] = false;
                        }
                        ClearTeamDeclinedHero(iTeam);
                        return;
                    }

                    int aiBest[2] = {INT_MIN, INT_MIN};
                    int aiCount[2] = {0, 0};
                    int iNumHumans = 0;
                    int aaiCandidates[2][600];
                    for (int j = 0; j < 600; j++)
                    {
                        Character* pCharacter = Character::GetPtr(j);
                        if (!pCharacter || pCharacter->GetTeamNumber() != iTeam)
                        {
                            continue;
                        }
                        if (j < iMaxPlayers &&
                            (!IsPlaying(j, false) || netPlayers[j].mDeclinedHero))
                        {
                            continue;
                        }

                        int bPlayer = pCharacter->IsPlayer();
                        if (bPlayer)
                        {
                            iNumHumans++;
                            if (!IsPlaying(j, false) || netPlayers[j].mDeclinedHero)
                            {
                                continue;
                            }
                        }
                        else if (IsEnabled())
                        {
                            continue;
                        }

                        int iScore;
                        switch (netHeroRulePlayer)
                        {
                        case 1:
                            iScore = Character::GetPlayerStats(pCharacter).GetPoints(true);
                            break;
                        case 2:
                            iScore = Character::GetPlayerStats(pCharacter).GetKills(-1, 0);
                            break;
                        case 3:
                            iScore = Character::GetPlayerStats(pCharacter).GetDeaths(-1, 0);
                            break;
                        case 4:
                            iScore = -Character::GetPlayerStats(pCharacter).GetPoints(true);
                            break;
                        case 5:
                            iScore = -Character::GetPlayerStats(pCharacter).GetKills(-1, 0);
                            break;
                        case 6:
                            iScore = -Character::GetPlayerStats(pCharacter).GetDeaths(-1, 0);
                            break;
                        default:
                            iScore = 0;
                            break;
                        }

                        if (aiBest[bPlayer] < iScore)
                        {
                            aiBest[bPlayer] = iScore;
                            aiCount[bPlayer] = 0;
                        }
                        if (aiBest[bPlayer] == iScore)
                        {
                            aaiCandidates[bPlayer][aiCount[bPlayer]++] = j;
                        }
                    }

                    if (aiCount[1] > 0)
                    {
                        if (aiCount[1] == 1)
                        {
                            netHeroNextPlayerID[iTeam] = aaiCandidates[1][0];
                        }
                        else
                        {
                            netHeroNextPlayerID[iTeam] = aaiCandidates[1][PblRandom::GetGlobalInt() % aiCount[1]];
                        }

                        Character* pHero = Character::GetPtr(netHeroNextPlayerID[iTeam]);
                        if (pHero && pHero->GetUnit())
                        {
                            HeroSelected(iTeam);
                        }
                        else
                        {
                            netHeroNextPlayerID[iTeam] = -1;
                        }
                    }
                    else if (aiCount[0] > 0)
                    {
                        if (aiCount[0] == 1)
                        {
                            netHeroAIID[iTeam] = aaiCandidates[0][0];
                        }
                        else
                        {
                            netHeroAIID[iTeam] = aaiCandidates[0][PblRandom::GetGlobalInt() % aiCount[0]];
                        }
                        HeroSelected(iTeam);
                    }
                }
            }

            if (netHeroNextPlayerID[iTeam] != -1)
            {
                if (IsEnabled())
                {
                    netHeroSndPlayHostSelectable[iTeam] = true;
                }
                else
                {
                    Character* pPlayer = Character::FindPlayer(netHeroNextPlayerID[iTeam]);
                    if (pPlayer)
                    {
                        HeroMessageDisplay::PlayHeroSoundSelectable(iTeam);
                    }
                }
            }
            else if (netHeroAIID[iTeam] != -1)
            {
                Character* pHero = Character::GetPtr(netHeroAIID[iTeam]);
                if (pHero)
                {
                    HeroMessageDisplay::PlayHeroSoundSelectable(iTeam);
                }
            }
        }
    }
    else
    {
        switch (netHeroRuleUnlock)
        {
        case 1:
            netHeroUnlocked[iTeam] = true;
            break;
        case 2:
        {
            int iCur = Team::GetPtr(iTeam)->GetCurReinforcements();
            int iMax = Team::GetPtr(iTeam)->GetMaxReinforcements();
            if (iMax > 0 && 100 * iCur <= netHeroRuleUnlockVal * iMax)
            {
                netHeroUnlocked[iTeam] = true;
            }
            break;
        }
        case 3:
        case 4:
        {
            int iTotal = 0;
            int iMaxPlayers = IsEnabled() ? netCurMaxPlayers : GameLoop::GetNumCameras();
            int iCount = 0;
            for (int k = 0; k < iMaxPlayers; k++)
            {
                if (IsPlaying(k, false))
                {
                    Character* pCharacter = Character::GetPtr(k);
                    if (pCharacter && pCharacter->GetTeamNumber() == iTeam)
                    {
                        int iScore;
                        if (netHeroRuleUnlock == 3)
                        {
                            iScore = Character::GetPlayerStats(pCharacter).GetPoints(true);
                        }
                        else
                        {
                            iScore = Character::GetPlayerStats(pCharacter).GetKills(-1, 0);
                        }
                        iTotal = Max(iTotal, iScore);
                        iCount++;
                    }
                }
            }

            if (!IsEnabled() && !iCount)
            {
                int n = 0;
                for (Character* pCharacter = Character::GetPtr(n); pCharacter;
                     pCharacter = Character::GetPtr(n))
                {
                    if (!pCharacter->IsPlayer() && pCharacter->GetTeamNumber() == iTeam)
                    {
                        int iScore;
                        if (netHeroRuleUnlock == 3)
                        {
                            iScore = Character::GetPlayerStats(pCharacter).GetPoints(true);
                        }
                        else
                        {
                            iScore = Character::GetPlayerStats(pCharacter).GetKills(-1, 0);
                        }
                        iTotal += iScore;
                    }
                    n++;
                }
            }

            if (iTotal >= netHeroRuleUnlockVal)
            {
                netHeroUnlocked[iTeam] = true;
            }
            break;
        }
        case 5:
        {
            float fTime = IsEnabled() ? NetComm::GetTime() : GetLocalTime();
            if (fTime > (float)netHeroRuleUnlockVal + netHeroUnlockStartTimer)
            {
                netHeroUnlocked[iTeam] = true;
            }
            break;
        }
        case 6:
        {
            Team* pTeam = Team::GetPtr(iTeam);
            if (pTeam && pTeam->GetPoints() >= netHeroRuleUnlockVal)
            {
                netHeroUnlocked[iTeam] = true;
            }
            break;
        }
        }

        if (!netHeroUnlocked[iTeam] && netHeroRuleTeam == 2)
        {
            for (int n = 0; n < 3; n++)
            {
                if (netHeroUnlocked[n])
                {
                    netHeroUnlocked[iTeam] = true;
                }
            }
        }

        if (netHeroUnlocked[iTeam])
        {
            if (IsEnabled())
            {
                netHeroSndPlayHostUnlock[iTeam] = true;
            }
            else
            {
                HeroMessageDisplay::PlayHeroSoundUnlock(iTeam);
            }
            netHeroFirstUnlocked[iTeam] = true;
        }
    }
}

// FUNCTION: SWBF2 0x005cf930
int NetGame::GetRandomHeroTeam()
{
    if (!IsHost() || IsClient())
    {
        return -1;
    }
    if (IsPreGame())
    {
        return -1;
    }

    Team* pTeam1 = Team::GetPtr(1);
    Team* pTeam2 = Team::GetPtr(2);
    int iCount1 = pTeam1 ? pTeam1->GetSpecialClassCount() : 0;
    int iCount2 = pTeam2 ? pTeam2->GetSpecialClassCount() : 0;

    if (!iCount1)
    {
        if (!iCount2)
        {
            return -1;
        }
        else
        {
            return 2;
        }
    }
    else if (!iCount2)
    {
        return 1;
    }

    return PblRandom::GetGlobalInt() % 2 + 1;
}

// FUNCTION: SWBF2 0x005cfa10
void NetGame::Update()
{
    int i;
    PblValues::Record('$', PVT_FULL);
    NetMatch::UpdateLogin(-1);

    int iHeroTeam = -1;
    for (i = 0; i < 3; i++)
    {
        if (netHeroUnlocked[i] && (netHeroRuleTeam == 1 || netHeroRuleTeam == 4))
        {
            iHeroTeam = i;
        }
    }

    if (iHeroTeam == -1 && netHeroRuleTeam == 1 && netHeroRuleUnlock == 1)
    {
        iHeroTeam = GetRandomHeroTeam();
        if (iHeroTeam != -1)
        {
            netHeroUnlocked[iHeroTeam] = true;
        }
    }

    if (netEnabledNext)
    {
        for (i = 0; i < 3; i++)
        {
            if (iHeroTeam == -1 || iHeroTeam == i)
            {
                HeroRulesUpdate(i);
            }
            if (netHeroUnlocked[i] && (netHeroRuleTeam == 1 || netHeroRuleTeam == 4))
            {
                iHeroTeam = i;
            }
        }

        NetComm::Update();
        NetQOS::Update(false);
        if (g_bNoRender)
        {
            AutoBootLagger();
        }

        PblValues::Record('*', PVT_FULL);
    }
    else if (netHeroesEnabled)
    {
        for (i = 0; i < 3; i++)
        {
            if (iHeroTeam == -1 || iHeroTeam == i)
            {
                HeroRulesUpdate(i);
            }
            if (netHeroUnlocked[i] && (netHeroRuleTeam == 1 || netHeroRuleTeam == 4))
            {
                iHeroTeam = i;
            }
        }
    }
}

// FUNCTION: SWBF2 0x005d0c40
void InitBootPlayer()
{
    clientBootPlayer = -1;
    hostBootPlayer = -1;
    hostForceBootPlayer = -1;
    bootInfo.changedTurn = 0;
}

// FUNCTION: SWBF2 0x005d1340
void FreeChatMessageQue(PblList<NG_ChatMsg>* pList)
{
    int iCount = pList->GetCount();
    while (iCount > 0)
    {
        PblList<NG_ChatMsg>::Node* pNode = pList->GetFirst();
        NG_ChatMsg* pMsg = pNode->_pObject;
        pList->Remove(pNode);
        delete pMsg;
        iCount--;
    }
}

// FUNCTION: SWBF2 0x005d13a0
void NetGame::FreeAllChatMessageQue()
{
    FreeChatMessageQue(&netGameChatMsgClient);
    FreeChatMessageQue(&netGameChatMsgServer);
}

// FUNCTION: SWBF2 0x005d2250
void SetTurnHighBits(unsigned int uBits)
{
    turnHighBits = uBits;
}

// GLOBAL: SWBF2 0x01e64a2d
bool frameLockEnabled;

void DisableFrameLock();

// FUNCTION: SWBF2 0x005d26e0
void CleanFrameLock()
{
    if (frameLockEnabled && netFrameLock)
    {
        DisableFrameLock();
    }
    frameLockEnabled = false;
}

bool IsNetObjectFeatureEnabled();
void AdvanceTimeBase();
extern PblTimeBase::Stamp frameLockStamp;

// FUNCTION: SWBF2 0x005d25e0
static void WaitForWakeup()
{
    if (IsNetObjectFeatureEnabled())
    {
        return;
    }
    AdvanceTimeBase();
    float elapsed = frameLockStamp.GetElapsed();
    float remaining = netSecondsPerTurn - elapsed;
    if (remaining > 0.0f)
    {
        unsigned int milliseconds = (unsigned int)(remaining * 1000.0f);
        if (milliseconds > 15)
        {
            SleepEx(milliseconds - 15, FALSE);
        }
        do
        {
            AdvanceTimeBase();
            elapsed = frameLockStamp.GetElapsed();
        }
        while (elapsed < netSecondsPerTurn);
    }
}

static void WakeMeNextFrame();

// FUNCTION: SWBF2 0x005d2710
void NetGame::FrameLock()
{
    if (frameLockEnabled && netFrameLock)
    {
        WaitForWakeup();
        WakeMeNextFrame();
    }
}

// FUNCTION: SWBF2 0x005b73c0
int NetGame::GetJoystickIndex(int iPlayer)
{
    return NetComm::GetJoystickIndex(iPlayer);
}

// FUNCTION: SWBF2 0x005c3770
void NetGame::SetPlayerGSID(int iPlayer, unsigned __int64 uId)
{
    netPlayers[iPlayer].mGSPlayerID = uId;
}

bool ReadNetIsLocalTurn();

// FUNCTION: SWBF2 0x0043cf50
bool NetGame::IsLateTurn()
{
    return netIsLateTurn;
}

bool IsSkippedND(const GameObject* pObject);
bool IsLateObject(const GameObject* pObject);
void SpeedGame();
void UpdateBootVote();
void SendToClients();
void SendUpdate(int iPlayer);
void SendToReplicas();
void SendToRelays();
void AdjustClock();
void UpdateHostStats();
void ListNearObjects();

// FUNCTION: SWBF2 0x005c50b0
unsigned int NetGame::GetTurnNumber()
{
    if (NetGame::IsClient())
    {
        return predictTurnNumber;
    }
    else if (NetGame::IsLateTurn())
    {
        return lateTurnNumber;
    }
    else
    {
        return hostTurnNumber;
    }
}

// GLOBAL: SWBF2 0x007e8e7b
bool netOnTurn = true;
// GLOBAL: SWBF2 0x01fa8768
int stepTurns;
// GLOBAL: SWBF2 0x01fa8774
unsigned int skipToTurnNumber;
// GLOBAL: SWBF2 0x01fa6588
float skipTurnsDt;
// GLOBAL: SWBF2 0x01fa65a4
unsigned int clientBootTurn;

// GLOBAL: SWBF2 0x01e62f68
unsigned int sJustEndedPreGameTurn;

// GLOBAL: SWBF2 0x01e64b94
int netPreGameTime;

// FUNCTION: SWBF2 0x005b7d50
void NetGame::LeaveTurn()
{
    netOnTurn = false;
}

// FUNCTION: SWBF2 0x005b7d60
void NetGame::EnterTurn()
{
    netOnTurn = true;
}

// FUNCTION: SWBF2 0x005b7d70
bool NetGame::IsSkipped(const GameObject* pObject, float& fDeltaTime)
{
    if (!netEnabledNext)
    {
        return false;
    }
    if (!pObject)
    {
        return false;
    }

    if (IsClient())
    {
        if (!ReadNetIsLocalTurn())
        {
            if (IsNeverDies(pObject->GetEntityType()))
            {
                return IsSkippedND(pObject);
            }

            int iId = pObject->GetNetUniqueId();
            if (iId < 0)
            {
                return true;
            }
            if (clientSkipped[iId])
            {
                return true;
            }

            bool bLocal = false;
            for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
            {
                Controllable& ctrl = *it;
                if (GetJoystickIndex(ctrl.GetPlayerId()) >= 0)
                {
                    bLocal = true;
                    break;
                }
            }
            if (bLocal)
            {
                return false;
            }

            if (updateTurnNumber + 1 == skipToTurnNumber)
            {
                fDeltaTime = skipTurnsDt;
                return false;
            }
            return true;
        }
    }
    else if (NetGame::IsLateTurn())
    {
        return !IsLateObject(pObject);
    }
    return false;
}

void WriteTurnLowBits(NetPktGroup& group, unsigned int uTurn);
void WriteSwitchResponse(NetPktGroup& group, SwitchResponse response, unsigned int turn);
void WriteMiniMove(NetPktGroup& group, ProcessedInputs* inputs);
void WriteMove(NetPktGroup& group, ProcessedInputs* previous, ProcessedInputs* current);
bool IsNetObjectFeatureEnabled();
int SetNetLOD(int precision);
void WriteSpawnSelect(NetPktGroup& group);
void WriteLockedTargets(NetPktGroup& group);
void WriteBootRequest(NetPktGroup& group);
void WriteSpawnCamera(NetPktGroup& group);
void WriteUpload(NetPktGroup& group);
void WriteSuicideRequest(NetPktGroup& group);

// GLOBAL: SWBF2 0x01fa6708
int curIndex;
// GLOBAL: SWBF2 0x01fa88a0
unsigned int clientSwitchObjTurn;

// FUNCTION: SWBF2 0x005b8920
void NetGame::SendClient()
{
    if (!netEnabledNext)
    {
        return;
    }

    NetComm::AdvanceTime(true);
    float fTime = NetComm::GetTime();

    if (loadGame && !NetGame::IsHost())
    {
        loadGame->Update();
        if (loadGame->IsDone())
        {
            delete loadGame;
            loadGame = 0;
        }
    }
    if (postLoadJoinGame)
    {
        postLoadJoinGame->Update();
    }

    AdjustClock();
    NetQOS::Update(false);

    if ((unsigned int)sendTurnNumber >= clientTurnNumber)
    {
        return;
    }

    if (!netGameOver)
    {
        netGameOver = GameLoop::IsGameOver() && !NetGame::IsHost() && NetGame::IsGameOver();
    }

    if (netGameOver)
    {
        NetPktGroup group(true);
        group.BeginWrite();
        group.Write(true);
        group.WriteBits(clientHostTurnNumber, 19);
        NetGame::SendChatMessageToServer(group);
        group.EndWrite();
        NetComm::Send(64, 19, &group);
        sendTurnNumber = clientTurnNumber;
    }
    else if (fTime - 1.0f > NetComm::GetLastRecv(64) || clientHostCommProblems)
    {
        if (fTime > miniMoveTime)
        {
            NetPktGroup group(true);
            group.BeginWrite();
            group.Write(true);
            int iSlot = (clientTurnNumber - 1) & 0x1f;
            ProcessedInputs* inputs = &localClientTurns[iSlot][0];
            WriteMiniMove(group, inputs);
            group.EndWrite();
            NetComm::Send(64, 11, &group);
            sendTurnNumber = clientTurnNumber;
            miniMoveTime = fTime + 0.2f;
        }
    }
    else
    {
        NetPktGroup group(true);
        group.BeginWrite();
        group.Write(false);
        unsigned int uTurn = clientTurnNumber - 1;
        WriteTurnLowBits(group, uTurn);
        IsNetObjectFeatureEnabled();
        if (clientTurnNumber >= 0x3d9a)
        {
            gDebugCount++;
        }
        group.Write(GameLoop::IsQuitRequested());

        if (netHostIndex >= 0)
        {
            switch (netHosts[netHostIndex].hostMode)
            {
            case HM_RELAY:
                WriteTurnLowBits(group, hostTurnNumber);
                break;
            case HM_REPLICA:
                WriteTurnLowBits(group, hostTurnNumber);
                break;
            }
        }
        WriteTurnLowBits(group, clientHostTurnNumber ? clientHostTurnNumber : -1);
        WriteSwitchResponse(group, clientSwitchObjects, clientSwitchObjTurn);
        SetNetLOD(1);
        WriteSpawnSelect(group);
        WriteLockedTargets(group);
        WriteBootRequest(group);
        WriteSpawnCamera(group);
        NetGame::SendChatMessageToServer(group);
        WriteUpload(group);
        WriteSuicideRequest(group);

        unsigned int uLastTurn = uTurn;
        curIndex = 0;
        while (1)
        {
            int iLocal = NetComm::GetLocalIndex(curIndex);
            StatsManager::WriteStatsResponse(iLocal, group);

            int iJoystick = NetGame::GetJoystickIndex(iLocal);
            const int iProfile = iJoystick >= 0 && iJoystick < 1
                ? PlayerPrefs::mAllUserPrefs[iJoystick].mJoystick : -1;
            bool bCockpitCam;
            if (iProfile >= 0)
            {
                PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(iProfile);
                if (pProfile)
                {
                    Character* pCharacter = Character::FindPlayer(iLocal);
                    if (pCharacter && pCharacter->GetControllable()
                        && pCharacter->GetControllable()->GetEntity())
                    {
                        Entity* pEntity = pCharacter->GetControllable()->GetEntity();
                        if (pEntity->IsRtti(EntityFlyer::GetRtti()))
                        {
                            bCockpitCam = pProfile->mGameOptions.mCockpitCamFlyer;
                        }
                        else
                        {
                            bCockpitCam = pProfile->mGameOptions.mCockpitCam;
                        }
                    }
                    else
                    {
                        bCockpitCam = pProfile->mGameOptions.mCockpitCam;
                    }
                }
                else
                {
                    bCockpitCam = false;
                }
            }
            else
            {
                bCockpitCam = false;
            }
            group.Write(bCockpitCam);

            uTurn = uLastTurn;
            int iSlot = uTurn & 0x1f;
            ProcessedInputs* pPrev = &localClientTurns[iSlot][curIndex];
            WriteMove(group, &noInputs, pPrev);
            unsigned int uOldest = sendTurnNumber - 4;
            if (uOldest > (unsigned int)sendTurnNumber)
            {
                uOldest = 0;
            }
            while (uTurn > uOldest)
            {
                uTurn--;
                iSlot = uTurn & 0x1f;
                ProcessedInputs* pCur = &localClientTurns[iSlot][curIndex];
                WriteMove(group, pPrev, pCur);
                pPrev = pCur;
            }
            WriteMove(group, pPrev, 0);

            curIndex++;
            bool bMore = curIndex < 2 && NetComm::GetLocalIndex(curIndex) >= 0;
            group.Write(bMore);
            if (!bMore)
            {
                break;
            }
            group.WriteBits(NetComm::GetLocalIndex(curIndex), 5);
        }

        group.EndWrite();
        NetComm::Send(64, 11, &group);
        sendTurnNumber = clientTurnNumber;
    }
}

// FUNCTION: SWBF2 0x005b9250
void NetGame::SendHost()
{
    NetMatch::UpdatePresence(false);
    if (!netEnabledNext)
    {
        return;
    }

    NetComm::AdvanceTime(true);
    SpeedGame();
    UpdateBootVote();
    SendToClients();
    SendToReplicas();
    SendToRelays();

    if (loadGame)
    {
        loadGame->Update();
        if (loadGame->IsDone())
        {
            delete loadGame;
            loadGame = 0;
        }
    }
    if (postLoadHostGame)
    {
        postLoadHostGame->Update();
    }

    AdjustClock();
    NetQOS::Update(false);
    UpdateHostStats();
    ListNearObjects();
}

unsigned int GetUniqObjectId(const GameObject* pObject);
GameObject* FindClientObject(unsigned int uId);
struct HandleToIndex
{
    int index;
    bool isNew;
};

// GLOBAL: SWBF2 0x01e64ef4
PblHashTable<HandleToIndex, 128>* handleToIndexMapPtr;
// GLOBAL: SWBF2 0x01fa2513
bool isFullState;
// GLOBAL: SWBF2 0x01fa88c8
int alignCount;

// FUNCTION: SWBF2 0x005c3870
int NetGame::GetMyGuest(int iPlayer, int iSlot)
{
    if (iPlayer < 0 || iPlayer >= 64)
    {
        return -1;
    }
    if (iSlot < 0 || iSlot >= 1)
    {
        return -1;
    }
    return netPlayers[iPlayer].myGuest[iSlot];
}

// FUNCTION: SWBF2 0x005c38b0
int NetGame::GetGuestIndex(int iPlayer, int iSlot)
{
    if (iSlot < 0 || iSlot > 1)
    {
        return -1;
    }
    if (iSlot == 0)
    {
        return iPlayer;
    }
    else
    {
        return netPlayers[iPlayer].myGuest[iSlot - 1];
    }
}

// FUNCTION: SWBF2 0x005b73d0
bool NetGame::IsLocalTo(int iPlayer, int iOther)
{
    if (iOther < 0)
    {
        return false;
    }
    if (iOther == iPlayer)
    {
        return true;
    }
    else
    {
        for (unsigned int i = 0; i < 1; ++i)
        {
            if (iOther == GetMyGuest(iPlayer, i))
            {
                return true;
            }
        }
    }
    return false;
}

void WriteEntityClass(NetPktGroup& group, const EntityClass* pClass);
void WriteWeaponClass(NetPktGroup& group, const WeaponClass* pClass);
EntityClass* ReadEntityClass(NetPktGroup& group);
WeaponClass* ReadWeaponClass(NetPktGroup& group);
void WriteObjectPtr(NetPktGroup& group, const GameObject* pObject);
GameObject* ReadObjectPtr(NetPktGroup& group);

// FUNCTION: SWBF2 0x005bc1a0
void WriteDamageOwner(NetPktGroup& group, const DamageOwner& owner)
{
    WriteCharacter(group, owner.mCharacter);
    if (static_cast<GameObject*>(owner.mGameObject))
    {
        group.Write(true);
        WriteObjectPtr(group, static_cast<GameObject*>(owner.mGameObject));
    }
    else
    {
        group.Write(false);
    }
    WriteWeaponClass(group, owner.mWeapon);
    WriteEntityClass(group, owner.mClass);
    if (owner.mDamageMultiplier != 1.0f)
    {
        group.Write(true);
        group.WriteFixed(owner.mDamageMultiplier, 0.0f, 10.0f, 16);
    }
    else
    {
        group.Write(false);
    }
}

// FUNCTION: SWBF2 0x005bc270
void ReadDamageOwner(NetPktGroup& group, DamageOwner& owner)
{
    owner.mCharacter = ReadCharacter(group);
    bool bObject;
    group.Read(bObject);
    owner.mGameObject = bObject ? ReadObjectPtr(group) : 0;
    if (owner.mCharacter)
    {
        owner.mTeam = owner.mCharacter->GetTeamNumber();
    }
    else if (static_cast<GameObject*>(owner.mGameObject))
    {
        owner.mTeam = owner.mGameObject->GetTeam();
    }
    else
    {
        owner.mTeam = 0;
    }
    owner.mWeapon = ReadWeaponClass(group);
    owner.mClass = ReadEntityClass(group);
    bool bMultiplier;
    group.Read(bMultiplier);
    if (bMultiplier)
    {
        group.ReadFixed(owner.mDamageMultiplier, 0.0f, 10.0f, 16);
    }
    else
    {
        owner.mDamageMultiplier = 1.0f;
    }
}

// FUNCTION: SWBF2 0x005bc380
void WriteObjectPtr(NetPktGroup& group, const GameObject* pObject)
{
    EntityType entType = pObject->GetEntityType();
    const bool bSpecial = IsNeverDies(entType);
    group.Write(bSpecial);
    if (bSpecial)
    {
        NetNdObj* pEntry = clientNdObjMap.FindId(pObject->GetHandleId());
        group.WriteBits(pEntry->index, 8);
    }
    else
    {
        group.WriteBits(GetUniqObjectId(pObject), 6);
    }
}

// FUNCTION: SWBF2 0x005bd340
GameObject* ReadObjectPtr(NetPktGroup& group)
{
    bool neverDies;
    group.Read(neverDies);
    if (neverDies)
    {
        unsigned int uniqId;
        group.ReadBits(uniqId, 8);
        NetNdObj* netNdObj = clientNdObjMap.FindIndex(uniqId);
        GameObject* obj = isFullState ? netNdObj->orig : netNdObj->obj;
        return obj;
    }
    else
    {
        unsigned int uniqId;
        group.ReadBits(uniqId, 6);
        return FindClientObject(uniqId);
    }
}

// FUNCTION: SWBF2 0x005bd440
void MarkWriteAlign(NetPktGroup& group)
{
    group.WriteBits(alignCount, (alignCount & 31) + 1);
    ++alignCount;
}

// FUNCTION: SWBF2 0x005bd490
void CheckReadAlign(NetPktGroup& group)
{
    const unsigned int uBits = (alignCount & 31) + 1;
    unsigned int uRead;
    group.ReadBits(uRead, uBits);
    unsigned int uExpected = alignCount;
    if (uBits != 32)
    {
        uExpected &= (1u << uBits) - 1;
    }
    ++alignCount;
}

// FUNCTION: SWBF2 0x005be390
void AddSendLate(GameObject*, GameObject*)
{
}

// FUNCTION: SWBF2 0x005c5090
int NetGame::GetCurDst()
{
    return curDst;
}

// FUNCTION: SWBF2 0x005c50a0
bool NetGame::IsFullState()
{
    return isFullState;
}

// FUNCTION: SWBF2 0x005c8c30
GameObject* FindClientObject(unsigned int uId)
{
    NetObj* netObj = clientObjMap.Find(uId);
    GameObject* obj = netObj ? netObj->obj : 0;
    return obj;
}

// FUNCTION: SWBF2 0x005cc6a0
unsigned int GetUniqObjectId(const GameObject* pObject)
{
    if (!pObject)
    {
        return 63;
    }
    unsigned int id = pObject->GetHandleId();
    HandleToIndex* h2i = handleToIndexMapPtr->Find(id);
    if (!h2i)
    {
        return 63;
    }
    return h2i->index;
}

// FUNCTION: SWBF2 0x005ce480
int ComputeNetLOD(const PblVector3&, int iPlayer)
{
    if (iPlayer < 0)
    {
        return 0;
    }
    for (unsigned int i = 0; i < 2; ++i)
    {
        const int local = NetGame::GetGuestIndex(curDst, i);
        if (iPlayer == local)
        {
            return 1;
        }
        if (local < 0)
        {
            continue;
        }
        if (iPlayer == NetGame::GetSpectator(local))
        {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005ba600
float NetGame::GetTurnStep()
{
    if (!netEnabledNext)
    {
        return GameLoop::GetDeltaTime();
    }
    return (float)stepTurns * netSecondsPerTurn;
}

// FUNCTION: SWBF2 0x005ba640
void NetGame::FinishTurn()
{
    if (!netEnabledNext)
    {
        return;
    }

    unsigned int uEnd = hostTurnNumber + stepTurns;
    while ((unsigned int)hostTurnNumber < uEnd)
    {
        if ((unsigned int)hostTurnNumber > 3)
        {
            int iSlot = (hostTurnNumber - 3) & 0x1f;
            hostRecv[iSlot] = 0;
            iSlot = (hostTurnNumber - 3) & 7;
            lateRecv[iSlot] = 0;
        }
        hostTurnNumber++;
    }
}

// FUNCTION: SWBF2 0x005ba6e0
void NetGame::SetTurnRatio(float fTime)
{
    if (clientTurnHeldBack)
    {
        if (fTime > turnRatio)
        {
            turnRatio = fTime;
        }
    }
    else
    {
        turnRatio = fTime;
    }
    if (netFrameLock)
    {
        turnRatio = 0.0f;
    }
}

// FUNCTION: SWBF2 0x005ba740
float NetGame::GetTurnRatio()
{
    return turnRatio;
}

// FUNCTION: SWBF2 0x005ba7b0
bool NetGame::FinishedTimeScale()
{
    bool bResult = finishedTimeScale;
    finishedTimeScale = false;
    return bResult;
}

// FUNCTION: SWBF2 0x005ba7d0
float NetGame::GetTimeScale()
{
    return timeScale;
}

// FUNCTION: SWBF2 0x005ba7e0
float NetGame::GetTimeAdjust()
{
    float fResult = pendingTimeAdjust;
    pendingTimeAdjust = 0.0f;
    return fResult;
}

void PredictTurn();

extern bool journalNeedsClose;
extern int debugPredictClientTurn;
extern int debugPredictStepCount;
namespace GameLoop
{
extern float sDeltaTime;
}









// GLOBAL: SWBF2 0x01fa876c
int MAX_PREDICT_STEPS;
// GLOBAL: SWBF2 0x01fa8778
int curDst;

inline void AtleastOnceSame::Change()
{
    changeTurn = hostTurnNumber;
}

inline void AtleastOnceUniq::Change()
{
    changeTurn[curDst] = hostTurnNumber;
}

// FUNCTION: SWBF2 0x005ba810
bool NetGame::Predict()
{
    if (!netEnabledNext)
    {
        return true;
    }
    if (journalNeedsClose)
    {
        CloseJournal();
    }
    if (NetGame::IsOneWorld() && NetGame::IsHost())
    {
        return true;
    }
    if (clientTurnNumber == debugPredictClientTurn)
    {
        gDebugCount++;
    }

    PblVector3 vCenter(0.0f, 0.0f, 0.0f);
    Tracker* pTracker = Tracker::GetTracker(curDst);
    Trackable* pTrackable = pTracker->GetTrackable();
    if (pTrackable)
    {
        GameObject* pObject = pTrackable->GetGameObject();
        netCenter = pObject->GetMatrix();
        vCenter = pObject->GetMatrix().GetTranslation();
    }
    PblVector3 vRange(200.0f, 200.0f, 200.0f);
    PblVector3 vMin = vCenter - vRange;
    PblVector3 vMax = vCenter + vRange;
    TreeGridCache::Prime(CollisionManager::GetTreeGrid(), vMin, vMax);

    if ((unsigned int)predictLocalTurnNumber < (unsigned int)updateTurnNumber)
    {
        predictLocalTurnNumber = updateTurnNumber;
    }
    predictTurnNumber = updateTurnNumber;
    int iSteps = debugPredictStepCount;
    int iOwed = predictLocalTurnNumber - updateTurnNumber;
    int iTurns = (iOwed + iSteps - 1) / iSteps;
    skipToTurnNumber = updateTurnNumber + iTurns;
    skipTurnsDt = (float)iTurns * netSecondsPerTurn;
    iSteps--;

    int iLocalTurns = 0;
    int iUpdateTurns = 0;
    int iCount = MAX_PREDICT_STEPS;
    while (iCount-- && (unsigned int)predictTurnNumber < clientTurnNumber)
    {
        PredictTurn();
        netIsLocalTurn = predictTurnNumber == predictLocalTurnNumber;
        netIsUpdateTurn = predictTurnNumber == updateTurnNumber;
        float fDeltaTime = netSecondsPerTurn;
        GameLoop::sDeltaTime = fDeltaTime;
        NetPredict::SetRenderTime((float)(predictTurnNumber + 1) * netSecondsPerTurn);
        netIsLastLocalTurn = netIsLocalTurn && predictTurnNumber + 1 == clientTurnNumber;
        netIsLastUpdateTurn = netIsUpdateTurn && predictTurnNumber + 1 == predictLocalTurnNumber;
        ControllerManager::Update(fDeltaTime);
        Thread::UpdateAll(fDeltaTime, false, netIsLocalTurn);
        if (netIsLocalTurn)
        {
            predictLocalTurnNumber++;
        }
        if (netIsUpdateTurn)
        {
            updateTurnNumber++;
        }
        if (netIsLocalTurn)
        {
            iLocalTurns++;
        }
        if (netIsUpdateTurn)
        {
            iUpdateTurns++;
        }
        predictTurnNumber++;
        if (netIsUpdateTurn && (unsigned int)predictTurnNumber == skipToTurnNumber && iSteps > 0)
        {
            iOwed = predictLocalTurnNumber - updateTurnNumber;
            iTurns = (iOwed + iSteps - 1) / iSteps;
            skipToTurnNumber = updateTurnNumber + iTurns;
            skipTurnsDt = (float)iTurns * netSecondsPerTurn;
            iSteps--;
        }
    }

    bool bCaughtUp = (unsigned int)predictTurnNumber == clientTurnNumber;
    if (iUpdateTurns > 0 && iLocalTurns == 0)
    {
        netIsLocalTurn = true;
        netIsUpdateTurn = false;
        netIsLastUpdateTurn = false;
        Thread::UpdateAll(0.0001f, false, netIsLocalTurn);
    }
    netIsLocalTurn = true;
    netIsLastLocalTurn = true;
    netIsUpdateTurn = true;
    netIsLastUpdateTurn = true;
    TreeGridCache::Flush();
    return bCaughtUp;
}

// FUNCTION: SWBF2 0x005bace0
ProcessedInputs* NetGame::GetJoystick(int iPlayer)
{
    if (iPlayer < 0)
    {
        return 0;
    }
    if (!netEnabledNext)
    {
        return hostJoysticks[iPlayer & 1];
    }
    return hostJoysticks[iPlayer];
}

// FUNCTION: SWBF2 0x005ce600
bool NetGame::IsMouseMode(int iPlayer)
{
    if (iPlayer < 0)
    {
        return false;
    }
    ProcessedInputs* joystick = GetJoystick(iPlayer);
    return joystick->GetMouseMode() > 0;
}

// FUNCTION: SWBF2 0x005be490
GameObject* NetGame::GetClientNDObject(GameObject* pObject)
{
    if (!netEnabledNext)
    {
        return pObject;
    }
    if (!pObject)
    {
        return pObject;
    }

    unsigned int uId = pObject->GetHandleId();
    NetNdObj* pNdObj = clientNdObjMap.FindId(uId);
    if (!pNdObj)
    {
        return pObject;
    }
    return pNdObj->obj;
}

// FUNCTION: SWBF2 0x005be500
CommandPost* NetGame::ConvertCP(CommandPost* pPost)
{
    if (!netEnabledNext)
    {
        return pPost;
    }
    if (!netClientActive)
    {
        return pPost;
    }
    if (!pPost)
    {
        return pPost;
    }
    return CommandPost::FindPostByIndex(pPost->GetIndex());
}

// FUNCTION: SWBF2 0x005c3cc0
void NetGame::HostChangeSpectator()
{
    for (int i = 0; i < netCurMaxPlayers; i++)
    {
        int iSpectate = NetGame::GetSpectator(i);
        ProcessedInputs* pInputs = NetGame::GetJoystick(i);
        if (iSpectate != -1)
        {
            NetGame::SetNextSpectator(i, iSpectate, pInputs);
        }
    }
}

// FUNCTION: SWBF2 0x005c3f20
bool NetGame::JustEndedPreGame()
{
    if (!sJustEndedPreGameTurn)
    {
        return false;
    }
    else
    {
        return hostTurnNumber - sJustEndedPreGameTurn <= 10;
    }
}

// FUNCTION: SWBF2 0x005c4480
int NetGame::GetStartTeam(int iPlayer)
{
    return netPlayers[iPlayer].isTeam1 ? 1 : 2;
}

// FUNCTION: SWBF2 0x005c3f60
void NetGame::UpdatePreGame()
{
    if (!NetGame::IsEnabled())
    {
        return;
    }
    if (NetGame::IsClient())
    {
        return;
    }
    if (!NetGame::IsPreGame())
    {
        return;
    }

    int iElapsed = (int)(NetComm::GetTime() - netPreGameStartTime);
    if (iElapsed > netPreGameTimeMax)
    {
        NetGame::VanishAllPlayer();
        NetGame::SetPreGame(false);
        sJustEndedPreGameTurn = hostTurnNumber;
    }
    else
    {
        netPreGameTime = netPreGameTimeMax - iElapsed;
    }
}

// FUNCTION: SWBF2 0x005c4080
void NetGame::HostChangePregame()
{
    ProcessedInputs* pInputs = 0;
    if (NetGame::IsEnabled() && NetGame::IsHost() && NetGame::IsDedicated())
    {
        pInputs = FLInputManager::Instance()->GetPlayerInputs(0);
    }
    else
    {
        pInputs = NetGame::GetJoystick(NetGame::GetLocalIndex(0));
    }

    if (pInputs && pInputs->IsJumpButtonDown())
    {
        netPreGameStartTime = (float)-netPreGameTimeMax - 1.0f;
    }
}

// FUNCTION: SWBF2 0x005d0cf0
int NetGame::GetBootPlayer()
{
    if (clientBootPlayer >= 0 && clientTurnNumber > clientBootTurn)
    {
        clientBootPlayer = -1;
    }
    return clientBootPlayer;
}

// FUNCTION: SWBF2 0x005d0dc0
void NetGame::SetBootVoted(int iJoystick, bool bVoted)
{
    clientBootVoted[iJoystick] = bVoted;
}

// FUNCTION: SWBF2 0x005cbc00
int SpawnDisplay::GetTeamSelection()
{
    int team = 0;
    char buff[64];
    _snprintf(buff, sizeof(buff), "ifs_sideselect%d", mJoystickIndex + 1);
    LuaHelper::GetGlobal(buff);
    char CurSelection[32];
    LuaHelper::TableReadVal("CurButton", CurSelection, sizeof(CurSelection), true);
    LuaHelper::Pop(1);

    if (strcmp(CurSelection, "team1") == 0)
    {
        team = 1;
    }
    else if (strcmp(CurSelection, "team2") == 0)
    {
        team = 2;
    }
    else if (strcmp(CurSelection, "auto") == 0)
    {
        team = 3;
    }
    else if (strcmp(CurSelection, "spec") == 0)
    {
        team = 4;
    }
    return team;
}

// FUNCTION: SWBF2 0x005cbe10
void SpawnDisplay::AcceptSideSelect()
{
    int iSelection = GetTeamSelection();
    mSpectateMode = false;
    switch (iSelection)
    {
    case 1:
    case 2:
        if (NetGame::IsEnabled())
        {
            NetGame::AutoAssignSpawn(mJoystickIndex, false);
            NetGame::SpectateSpawn(mJoystickIndex, false);
        }
        SpawnDisplay::Instance(mJoystickIndex)->SelectTeam(iSelection);
        break;
    case 3:
        if (NetGame::IsEnabled())
        {
            NetGame::AutoAssignSpawn(mJoystickIndex, true);
        }
        if (GameLoop::AreMetagameRulesOn())
        {
            int iSide = GameLoop::GetMetagameSide(mJoystickIndex);
            int iTeam = -1;
            for (int i = 0; i < 8; ++i)
            {
                if (Team::sTeams[i]->GetSide() == iSide)
                {
                    iTeam = i;
                    break;
                }
            }
            if (iTeam < 0)
            {
                iTeam = Team::GetAutoAssignTeamNumber();
            }
            SpawnDisplay::Instance(mJoystickIndex)->SelectTeam(iTeam);
        }
        else if (NetGame::IsEnabled())
        {
            SpawnDisplay::Instance(mJoystickIndex)->SetWaitingForTeam(true);
        }
        break;
    case 4:
    {
        if (NetGame::IsEnabled())
        {
            NetGame::AutoAssignSpawn(mJoystickIndex, false);
            NetGame::SpectateSpawn(mJoystickIndex, true);
        }
        mSpectateMode = true;

        SpawnDisplay* pDisplay = SpawnDisplay::Instance(mJoystickIndex);
        pDisplay->mMode = eFINISHED;
        break;
    }
    }
}

// FUNCTION: SWBF2 0x005cc000
void SpawnDisplay::UpdateSideSelectText(int iScreen)
{
    int aiCount[2] = {0, 0};

    if (NetGame::IsEnabled())
    {
        for (int i = 0; i < netCurMaxPlayers; i++)
        {
            if (NetGame::IsPlaying(i, false))
            {
                bool bSelecting = false;
                int iJoystick = NetGame::GetJoystickIndex(i);
                if (iJoystick >= 0)
                {
                    bSelecting = SpawnDisplay::Instance(iJoystick)->IsSelectingTeam();
                }
                if (!bSelecting)
                {
                    if (netPlayers[i].isTeam1)
                    {
                        aiCount[0]++;
                    }
                    else
                    {
                        aiCount[1]++;
                    }
                }
            }
        }
    }
    else
    {
        Team::GetTeamMemberCount(aiCount[0], aiCount[1]);
    }

    for (int j = 0; j < 2; j++)
    {
        char acName[64];
        _snprintf(acName, sizeof(acName), "ifs_sideselect%d.buttons.team%d.label", iScreen, j + 1);
        RedTextElement* pLabel = (RedTextElement*)RedInterfaceElement::Find(acName);
        if (pLabel)
        {
            Team* pTeam = Team::GetPtr(j + 1);
            unsigned short ausText[128];
            if (NetGame::IsEnabled())
            {
                const unsigned short* pusFormat = RedLocalizeDB::Find(0x5071dc6b, false);
                PblUnicode::usnprintf(ausText, 127, pusFormat, pTeam->GetName(), aiCount[j]);
            }
            else
            {
                PblUnicode::usnprintf(ausText, 127, pTeam->GetName());
            }
            pLabel->SetText(ausText, true);
        }
    }
}

// GLOBAL: SWBF2 0x007ec4e8
int debugReadyGameCount = 4;

// GLOBAL: SWBF2 0x007ec4f0
unsigned int debugReadyHostTurn = 0x26c3;

// GLOBAL: SWBF2 0x007e8e79
bool doTimeOut = true;

// GLOBAL: SWBF2 0x01faa6a8
bool journalNeedsCloseOnSubmit;

// GLOBAL: SWBF2 0x01faa6b0
int debugMultiTurnSubmitCount;

// GLOBAL: SWBF2 0x01faa6b4
unsigned int clientTurnLeadLimit;

// GLOBAL: SWBF2 0x01faa6bc
int multiTurnSubmitCount;

void WriteWaiting(NetPktGroup& group);
void ReceivePrimary();
void ReceiveReplica();
void ReceiveRelay();
void ClampMove(ProcessedInputs* pInputs);
void ClearPressed(ProcessedInputs* pInputs);
bool CanSendWaitingAck();
void FillTurn(unsigned int uTurn);
void UseTurn(unsigned int uTurn);
void RecordTurn(unsigned int uTurn);

// FUNCTION: SWBF2 0x005b9340
void NetGame::ReceiveHost()
{
    int i;
    if (!netEnabledNext)
    {
        return;
    }

    NetComm::AdvanceTime(true);
    NetComm::Update();
    NetAdmin::ReceiveConsole();
    if ((unsigned int)hostTurnNumber > 0x7F800)
    {
        int iShift;
        if (netRandom.GetInt() & 1)
        {
            iShift = 1;
        }
        else
        {
            iShift = 2;
        }
        int iBit = iShift;
        Team::SetWinningTeams(1 << iBit);
    }

    bool bReady = NetGame::IsReadyToPlay();
    if (!loadGame && !bReady)
    {
        while (true)
        {
            NetPacket* pPkt = NetComm::Recv(PKT_WAITING_ACK, PKT_NONE);
            if (!pPkt)
            {
                break;
            }
            int iSrc = GetSrc(pPkt);
            NetPktGroup group;
            group.Set(pPkt);
            group.BeginRead();
            group.EndRead();
            pPkt->Free();
            group.BeginWrite();
            WriteWaiting(group);
            group.EndWrite();
            NetComm::Send(iSrc, PKT_WAITING, &group);
        }
    }
    if (!loadGame && bReady)
    {
        while (true)
        {
            NetPacket* pPkt = NetComm::Recv(PKT_WAITING_ACK, PKT_NONE);
            if (!pPkt)
            {
                break;
            }
            pPkt->Free();
        }
    }

    ReceivePrimary();
    ReceiveReplica();
    ReceiveRelay();
    if (!loadGame && netHosts[netHostIndex].hostMode == HM_HOST)
    {
        float fNow = NetComm::GetTime();
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            if (!NetComm::IsConnected(i))
            {
                continue;
            }
            if (!NetGame::IsPlaying(i, false))
            {
                continue;
            }
            float fSilent = fNow - NetComm::GetLastRecv(i);
            float fLimit = 15.0f;
            if (netPlayers[i].requestedQuit)
            {
                fLimit = 5.0f;
            }
            else if (!doTimeOut)
            {
                fLimit = 1000000.0f;
            }
            if (fSilent > fLimit)
            {
                NetGameP::SetNotPlaying(i);
            }
        }
    }

    if (GameLoop::IsQuitRequested())
    {
        bool bAllQuit = true;
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            if (!NetGame::IsPlaying(i, false))
            {
                continue;
            }
            if (NetGame::GetJoystickIndex(i) >= 0)
            {
                continue;
            }
            if (netPlayers[i].requestedQuit)
            {
                continue;
            }
            bAllQuit = false;
            break;
        }
        if (bAllQuit)
        {
            canQuit = true;
        }
    }
}

// FUNCTION: SWBF2 0x005b9620
void NetGame::SubmitMove(int iTurns)
{
    if (journalNeedsCloseOnSubmit)
    {
        CloseJournal();
    }

    if (!netEnabledNext)
    {
        clientTurnNumber += iTurns;
        singleTime = GameLoop::GetDeltaTime() + singleTime;
        return;
    }

    if (!NetGame::IsReadyToPlay())
    {
        if (!loadGame && CanSendWaitingAck())
        {
            NetPktGroup group;
            group.BeginWrite();
            group.WriteBits(0, 1);
            group.EndWrite();
            NetComm::Send(64, PKT_WAITING_ACK, group.pktPtr);
        }
        return;
    }

    if (NetGame::IsHost() && NetGame::IsDedicated())
    {
        clientTurnNumber += iTurns;
        singleTime = GameLoop::GetDeltaTime() + singleTime;
        return;
    }

    if (iTurns > 1)
    {
        ++multiTurnSubmitCount;
        if (multiTurnSubmitCount == debugMultiTurnSubmitCount)
        {
            ++gDebugCount;
        }
    }
    if (iTurns > 3)
    {
        iTurns = 3;
    }

    if (NetGame::IsHost() && NetGame::IsOneWorld())
    {
        clientHostTurnNumber = hostTurnNumber;
        float fNow = NetComm::GetTime();
        NetCommP::GetLastRecv(64) = fNow;
    }

    const unsigned int iWindow = netTurnsPerSecond * 3;
    unsigned int iAcked = clientHostTurnNumber;
    bool bFree = iAcked == 0 && clientTurnNumber > iWindow + 10;
    if (clientTurnNumber + iTurns > iAcked + iWindow && !bFree)
    {
        clientTurnNumber = iAcked + iWindow - iTurns;
        clientTurnHeldBack = true;
        sendTurnNumber = clientTurnNumber - 1;
    }
    else
    {
        clientTurnHeldBack = false;
    }

    ProcessedInputs* apInputs[2];
    unsigned int i;
    for (i = 0; i < 2; ++i)
    {
        if (NetComm::GetLocalIndex(i) >= 0)
        {
            apInputs[i] = FLInputManager::Instance()->GetPlayerInputs(i);
        }
        else
        {
            apInputs[i] = 0;
        }
    }

    int iTurn;
    for (iTurn = 0; iTurn < iTurns; ++iTurn)
    {
        int iSlot = clientTurnNumber & 0x1f;
        for (i = 0; i < 2; ++i)
        {
            if (apInputs[i])
            {
                localClientTurns[iSlot][i] = *apInputs[i];
                ClampMove(&localClientTurns[iSlot][i]);
                if (iTurn > 0)
                {
                    ClearPressed(&localClientTurns[iSlot][i]);
                }
                if (PauseMenu::GetPaused(i) || FLInputManager::Instance()->AnyMissing() || CameraManager::Instance()->IsFreeLook(i) ||
                    !((PlayerController::sEnabled & (1 << i)) != 0))
                {
                    localClientTurns[iSlot][i].Clear();
                }
            }
        }
        ++clientTurnNumber;
    }
}

// FUNCTION: SWBF2 0x005b99c0
int NetGame::GetMaxSimTurns()
{
    return 4;
}

// FUNCTION: SWBF2 0x005b99d0
bool NetGame::IsTurnReady()
{
    int i;
    if (netGameCount == debugReadyGameCount && (unsigned int)hostTurnNumber >= debugReadyHostTurn)
    {
        ++gDebugCount;
    }

    if (!netEnabledNext)
    {
        ProcessedInputs* pInputs = FLInputManager::Instance()->GetPlayerInputs(0);
        ProcessedInputs* pHost = hostJoysticks[0];
        *pHost = *pInputs;
        if (clientTurnNumber == (unsigned int)hostTurnNumber)
        {
            return false;
        }
        hostTurnNumber = clientTurnNumber;
        return true;
    }

    unsigned int uTurn = hostTurnNumber;
    stepTurns = 0;
    while (true)
    {
        if (uTurn > clientTurnNumber - clientTurnLeadLimit)
        {
            break;
        }
        int iSlot = uTurn & 0x1f;
        bool bReady = (hostRecv[iSlot] & hostNextPlayingMask) == hostNextPlayingMask;
        bReady = bReady & NetGame::IsReadyToPlay();
        unsigned int uDelay = 3;
        if (!netWaitLate)
        {
            uDelay = 0;
        }
        if (!bReady && netHosts[netHostIndex].hostMode == HM_HOST && clientTurnNumber > uDelay)
        {
            bool bLate = uTurn < clientTurnNumber - uDelay;
            NetGame::IsDedicated();
            if (bLate)
            {
                FillTurn(uTurn);
                for (i = 0; i < netCurMaxPlayers; ++i)
                {
                    if ((hostNextPlayingMask & ((unsigned __int64)1 << i)) &&
                        !(hostRecv[iSlot] & ((unsigned __int64)1 << i)))
                    {
                        if (hostPlayingMask & ((unsigned __int64)1 << i))
                        {
                            int previousSlot = (uTurn - 1) & 0x1f;
                            hostTurns[iSlot].moves[i] = hostTurns[previousSlot].moves[i];
                            ++netPlayers[i].filledCount;
                        }
                        else
                        {
                            hostTurns[iSlot].moves[i] = noInputs;
                        }
                    }
                }
                bReady = true;
            }
        }
        if (!bReady)
        {
            break;
        }
        ++uTurn;
        ++stepTurns;
        if (stepTurns >= 1)
        {
            break;
        }
    }

    if (uTurn == (unsigned int)hostTurnNumber)
    {
        return false;
    }

    SetTurnHighBits(uTurn);
    for (uTurn = hostTurnNumber; uTurn < (unsigned int)(hostTurnNumber + stepTurns); ++uTurn)
    {
        UseTurn(uTurn);
        RecordTurn(uTurn);
    }

    if (hostPlayingMask != hostNextPlayingMask)
    {
        unsigned __int64 uLeft = hostPlayingMask & ~hostNextPlayingMask;
        unsigned __int64 uJoined = hostNextPlayingMask & ~hostPlayingMask;
        hostPlayingMask = hostNextPlayingMask;
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            if (uLeft & ((unsigned __int64)1 << i))
            {
                NetGame::RemovePlayer(i);
            }
        }
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            if (uJoined & ((unsigned __int64)1 << i))
            {
                NetGame::AddPlayer(i);
            }
        }
    }

    int iSlot = hostTurnNumber & 0x1f;
    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        if (hostPlayingMask & ((unsigned __int64)1 << i))
        {
            ProcessedInputs* pHost = hostJoysticks[i];
            *pHost = hostTurns[iSlot].moves[i];
        }
    }

    PblValues::Record(Team::GetPtr(1)->GetCurReinforcements(), PVT_MOVES_HIGH);
    PblValues::Record(Team::GetPtr(2)->GetCurReinforcements(), PVT_MOVES_HIGH);
    return true;
}

// FUNCTION: SWBF2 0x005b9fa0
void NetGame::SaveLateState()
{
    if (!netEnabledNext)
    {
        return;
    }
    if (netWaitLate)
    {
        return;
    }

    int iSlot = hostTurnNumber & 7;
    NetPktGroup* pGroup = &lateState[iSlot];
    pGroup->Free();
    pGroup->BeginWrite();
    BeginNetLOD();
    SetNetLOD(1);
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!NetGame::IsPlaying(i, false))
        {
            continue;
        }
        Character* pChr = Character::FindPlayer(i);
        if (!pChr)
        {
            continue;
        }
        Controllable* pCtrl = pChr->GetUnit();
        if (!pCtrl)
        {
            continue;
        }
        if (pChr->GetVehicle())
        {
            pCtrl = pChr->GetVehicle();
        }
        GameObject* pObject = pCtrl->GetGameObject();
        pGroup->Tell(lateStateOffset[iSlot][i]);
        pObject->WriteLateState(*pGroup);
    }
    pGroup->EndWrite();
}

// FUNCTION: SWBF2 0x005c4510
bool NetGame::CanQuit()
{
    if (!netEnabledNext)
    {
        return true;
    }
    float fTime = NetComm::GetTime();
    if (canQuitTimeOut == 0.0f)
    {
        canQuitTimeOut = fTime + (IsHost() ? 6.0f : 3.0f);
    }
    if (fTime > canQuitTimeOut)
    {
        canQuit = true;
    }
    return canQuit;
}

// FUNCTION: SWBF2 0x005c45b0
bool NetGame::DetectError()
{
    if (!netEnabledNext)
    {
        return false;
    }
    const char* pcError;
    int iSeverity;
    NetSetup::GetLoginError(pcError, iSeverity);
    if (pcError)
    {
        return true;
    }
    NetSetup::GetGameError(pcError, iSeverity);
    if (pcError)
    {
        return true;
    }
    return false;
}

// SYNTHETIC: SWBF2 0x005bf360
// OrdnanceDesc::~OrdnanceDesc

extern RecentGuestObjectRecord recentGuestObjects[64][3];

// FUNCTION: SWBF2 0x005ceb30
void NetGame::DeclineHero(int iTeam, int iPlayer)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (iTeam < 0 || iTeam >= 3)
    {
        return;
    }
    if (netHeroPlayerID[iTeam] != iPlayer && netHeroNextPlayerID[iTeam] != iPlayer)
    {
        return;
    }

    if (netHeroPlayerID[iTeam] == iPlayer)
    {
        netHeroPlayerID[iTeam] = -1;
        if (!netHeroDiedTurn[iTeam])
        {
            netHeroDiedTurn[iTeam] = 1;
        }
    }
    if (netHeroNextPlayerID[iTeam] == iPlayer)
    {
        netHeroNextPlayerID[iTeam] = -1;
    }
    if (iTeam == 1 || iTeam == 2)
    {
        netHeroKillerID[3 - iTeam] = -1;
    }
    netHeroSelectedTurn[iTeam] = 0;
    netPlayers[iPlayer].mDeclinedHero = true;
}

// FUNCTION: SWBF2 0x005d3480
void RecordRecentPlayerObject(int iPlayer, const GameObject* pObject)
{
    if (!NetGame::IsEnabled() || NetGame::IsClient())
    {
        return;
    }
    if (iPlayer < 0)
    {
        return;
    }
    if (!pObject)
    {
        return;
    }

    float fOldest = FLT_MAX;
    unsigned int uSlot = 0;
    for (unsigned int i = 0; i < 3; ++i)
    {
        const GameObject* pSlot =
            recentGuestObjects[iPlayer][i].mObjectHandle;
        if (!pSlot)
        {
            uSlot = i;
            break;
        }
        if (pSlot == pObject)
        {
            uSlot = i;
            break;
        }
        if (fOldest > recentGuestObjects[iPlayer][i].mRecordedTime)
        {
            fOldest = recentGuestObjects[iPlayer][i].mRecordedTime;
            uSlot = i;
        }
    }
    recentGuestObjects[iPlayer][uSlot].mObjectHandle = pObject;
    recentGuestObjects[iPlayer][uSlot].mRecordedTime = NetComm::GetTime();
}

// FUNCTION: SWBF2 0x005ced40
float GetTimeSinceHeroDied(int iTeam)
{
    if (NetGame::IsEnabled())
    {
        return (float)(unsigned int)(NetGame::GetTurnNumber() - netHeroDiedTurn[iTeam]) * netSecondsPerTurn;
    }
    else
    {
        return NetGame::GetLocalTime() - spHeroDiedTime[iTeam];
    }
}

namespace Lua_Callbacks
{
extern int gHuntShowScoreLimit;
}

// FUNCTION: SWBF2 0x005d3160
void NetGame::GetScore(int* count1, int* count2, int* limit1, int* limit2, bool* limits, bool* points)
{
    if (SpaceAssault::mInstance && SpaceAssault::mInstance->IsEnabled())
    {
        *limits = true;
        *points = true;
        *count1 = Team::GetPtr(1)->GetPoints();
        *count2 = Team::GetPtr(2)->GetPoints();
        *limit1 = Lua_Callbacks::gSpaceAssaultScoreLimit[Lua_Callbacks::gCurRules];
        *limit2 = Lua_Callbacks::gSpaceAssaultScoreLimit[Lua_Callbacks::gCurRules];
    }
    else if ((int)FlagItem::GetGameplayType() == 2 || (int)FlagItem::GetGameplayType() == 3 ||
             Lua_Callbacks::gHuntShowScoreLimit == 1 || AIUtil::sUberMode)
    {
        *limits = false;
        *points = true;
        *count1 = Team::GetPtr(1)->GetPoints();
        *count2 = Team::GetPtr(2)->GetPoints();
    }
    else
    {
        *limits = false;
        *points = false;
        *count1 = Team::GetPtr(1)->GetCurReinforcements();
        *count2 = Team::GetPtr(2)->GetCurReinforcements();
    }
}

void ReadWaiting(NetPktGroup& group);
unsigned int GetUpdateTurn(NetPktGroup& group);
void ReceiveUpdate(NetPktGroup& group);
extern bool netInGame;

// FUNCTION: SWBF2 0x005b8f70
void NetGame::ReceiveClient()
{
    if (!netEnabledNext)
    {
        return;
    }
    NetComm::Update();
    if (loadGame)
    {
        return;
    }
    bool isReadyToPlay = IsReadyToPlay();
    if (!isReadyToPlay)
    {
        while (true)
        {
            NetPktGroup group;
            if (!NetComm::Recv(group, PKT_WAITING))
            {
                break;
            }
            group.BeginRead();
            ReadWaiting(group);
            group.EndRead();
            group.Free();
        }
    }
    int host = netPlayers[NetComm::GetLocalIndex(0)].clientOfIndex;
    PblValues::Record('5', PVT_FULL);
    float now = NetComm::GetTime();
    while (true)
    {
        NetPktGroup group[2];
        if (!NetComm::Recv(group[0], PKT_GAME_UPDATE))
        {
            break;
        }
        int discard = 1;
        while (true)
        {
            if (!NetComm::Recv(group[discard], PKT_GAME_UPDATE))
            {
                break;
            }
            unsigned int previous = GetUpdateTurn(group[discard ^ 1]);
            unsigned int received = GetUpdateTurn(group[discard]);
            if (received > previous)
            {
                discard ^= 1;
            }
            group[discard].Free();
        }
        PblValues::Record('6', PVT_FULL);
        ReceiveUpdate(group[discard ^ 1]);
        NetCommP::GetLastRecv(host + 64) = now;
        hostCycleTime = now + 3.0f;
    }
    PblValues::Record('7', PVT_FULL);
    if (!GameLoop::IsGameOver())
    {
        float silent = NetComm::GetTime() - NetComm::GetLastRecv(host + 64);
        if (doTimeOut && silent > 15.0f)
        {
            netInGame = false;
            NetMatch::SetGameError("ifs.mp.joinerrors.connectlost", 6);
            NetComm::Disconnect(host + 64);
        }
    }
    while (true)
    {
        NetPktGroup group;
        if (!NetComm::Recv(group, PKT_STATS_UPDATE))
        {
            break;
        }
        group.BeginRead();
        bool active;
        group.Read(active);
        NetGame::ReadChatMessageFromServer(group);
        if (active)
        {
            hostCycleTime = now + 3.0f;
        }
        group.EndRead();
        group.Free();
    }
    NetGame::ClientUpdateSpectator();
    ListNearObjects();
}

// FUNCTION: SWBF2 0x005bd3c0
bool IsSame(ProcessedInputs* first, ProcessedInputs* second)
{
    if (first->bits.mMouseMode != second->bits.mMouseMode)
    {
        return false;
    }
    if (first->bits.mBoolInputs != second->bits.mBoolInputs)
    {
        return false;
    }
    return memcmp(first->mFloatInputs, second->mFloatInputs, 16) == 0;
}

// FUNCTION: SWBF2 0x005bf930
unsigned int FillHighBits(unsigned int lowBits, unsigned int lastBits, int bitCount)
{
    unsigned int delta = 1u << bitCount;
    unsigned int halfDelta = delta >> 1;
    unsigned int mask = delta - 1;
    unsigned int highBits = lastBits & ~mask;
    unsigned int current = lowBits | highBits;
    unsigned int prev = current - delta;
    unsigned int next = current + delta;
    unsigned int diff = lastBits - prev;
    if (diff < halfDelta)
    {
        return prev;
    }
    diff = next - lastBits;
    if (diff < halfDelta)
    {
        return next;
    }
    return current;
}

// FUNCTION: SWBF2 0x005d22d0
void ReadTurnLowBits(NetPktGroup& group, unsigned int& turn)
{
    unsigned int lowBits;
    group.ReadBits(lowBits, 11);
    if (lowBits == 0x7ff)
    {
        turn = (unsigned int)-1;
        return;
    }
    if (lowBits == 0x7fe)
    {
        bool escape;
        group.Read(escape);
        if (escape)
        {
            lowBits = 0x7fe;
        }
        else
        {
            lowBits = 0x7ff;
        }
    }
    turn = (turnHighBits & 0xfffff800) | lowBits;
    unsigned int previousLow = turnHighBits & 0x7ff;
    if (previousLow < 0x400)
    {
        if (lowBits > previousLow + 0x400)
        {
            turn -= 0x800;
        }
    }
    else
    {
        if (lowBits < previousLow - 0x400)
        {
            turn += 0x800;
        }
    }
}

// FUNCTION: SWBF2 0x005d24b0
void WriteDestMask(NetPktGroup& group, unsigned __int64 mask)
{
    unsigned int lowBits = (unsigned int)mask;
    unsigned int highBits = (unsigned int)(mask >> 32);
    group.WriteBits(lowBits, 32);
    group.WriteBits(highBits, 32);
}

// FUNCTION: SWBF2 0x005d24f0
void ReadDestMask(NetPktGroup& group, unsigned __int64& mask)
{
    unsigned int low, high;
    group.ReadBits(low, 32);
    group.ReadBits(high, 32);
    mask = ((unsigned __int64)high << 32) | low;
}

void ReadString(NetPktGroup& group, char* text, unsigned int capacity);

// FUNCTION: SWBF2 0x005d2430
void ReadWaiting(NetPktGroup& group)
{
    unsigned int playerCount;
    group.ReadBits(playerCount, 7);
    if (!NetGame::IsHost())
    {
        netMinPlayersToAutoLaunchDedicated = playerCount;
    }
    unsigned int nameCount;
    group.ReadBits(nameCount, 7);
    for (int i = 0; i < (int)nameCount; ++i)
    {
        bool hasName;
        group.Read(hasName);
        if (hasName)
        {
            char name[32];
            ReadString(group, name, sizeof(name));
        }
    }
}

// FUNCTION: SWBF2 0x005b85b0
unsigned int NetGame::GetKey(int index)
{
    if (index < 64)
    {
        return netPlayers[index].key;
    }
    index -= 64;
    return index < 3 ? netHosts[index].key : 0;
}

// FUNCTION: SWBF2 0x005b8630
void NetGameP::SetGuestOf(int player, int host)
{
    netPlayers[player].guestOf = host;
    if (host >= 0)
    {
        for (unsigned int i = 0; i < 1; ++i)
        {
            if (netPlayers[host].myGuest[i] == player)
            {
                break;
            }
            if (netPlayers[host].myGuest[i] == -1)
            {
                netPlayers[host].myGuest[i] = player;
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x005b8530
void NetGameP::SetPlayerName(int player, const char* name)
{
    strlcpy(netPlayers[player].name, name, sizeof(netPlayers[player].name));
}

// FUNCTION: SWBF2 0x005b86c0
int& NetGameP::GetHostIndex()
{
    return netHostIndex;
}

// FUNCTION: SWBF2 0x005c2020
void ReadEvent_ScriptMessage(NetPktGroup& group, ReliableNetEvent& event)
{
    group.ReadBits(event.u.scriptMessage.mLocalizeKey, 32);
    group.ReadBits(event.u.scriptMessage.mTeamMask, 3);
}

// FUNCTION: SWBF2 0x005c2110
void ReadEvent_PlayVoiceOver(NetPktGroup& group, ReliableNetEvent& event)
{
    group.ReadBits(event.u.playVoiceOver.mSoundNameHashID, 32);
    group.ReadBits(event.u.playVoiceOver.mTeamMask, 3);
}

// FUNCTION: SWBF2 0x005c2200
void ReadEvent_UpdateDoorState(NetPktGroup& group, ReliableNetEvent& event)
{
    group.ReadBits(event.u.updateDoorState.mHashID, 32);
    group.Read(event.u.updateDoorState.mIsLocked);
}

// FUNCTION: SWBF2 0x005c23f0
void ReadEvent_UpdateObjective(NetPktGroup& group, ReliableNetEvent& event)
{
    group.ReadBits(event.u.updateObjective.mHashID, 32);
    group.ReadBits((unsigned int&)event.u.updateObjective.state, 8);
}

// FUNCTION: SWBF2 0x005c3650
bool NetGame::IsPlayingNext(int player)
{
    return (hostNextPlayingMask & ((unsigned __int64)1 << player)) != 0;
}

// FUNCTION: SWBF2 0x005c36a0
const char* NetGame::GetPlayerName(int player)
{
    return netPlayers[player].name;
}

// FUNCTION: SWBF2 0x005c3750
unsigned __int64 NetGame::GetPlayerGSID(int player)
{
    return netPlayers[player].mGSPlayerID;
}

// FUNCTION: SWBF2 0x005c4230
void NetGame::SetSpawning(int player, bool spawning)
{
    netPlayers[player].mIsSpawning = spawning;
}

// FUNCTION: SWBF2 0x005c4250
bool NetGame::IsSpawning(int player)
{
    return netPlayers[player].mIsSpawning;
}

// FUNCTION: SWBF2 0x005c4010
void NetGame::ReadPreGame(NetPktGroup& group)
{
    group.Read(netPreGameEnabled);
    unsigned int time = 0;
    group.ReadBits(time, 10);
    netPreGameTime = time;
}

// GLOBAL: SWBF2 0x01e64cd8
GameObject* hostWorldObjects[64];

// FUNCTION: SWBF2 0x005d30d0
int NetGame::GetHostWorldUniqueId(int index, GameObject* object)
{
    if (index >= 0 && hostWorldObjects[index] == object)
    {
        return index;
    }
    return -1;
}

// FUNCTION: SWBF2 0x005c0990
void ReadEvent_ChangeCommandPostTeams(NetPktGroup& group, ReliableNetEvent& event)
{
    ReliableNetEvent::U::ChangeCommandPostTeamsEvent& data = event.u.changecommandpostteams;
    group.ReadBits(data.mTeamBits, 32);
    group.ReadBits(data.mAliveBits, 16);
}

// FUNCTION: SWBF2 0x005c0b00
void NetGame::CreateEvent_CreateChunkCluster(const ChunkCluster* cluster,
                                             EntityGeometryClass* entityClass,
                                             unsigned __int64 mask)
{
    if (cluster->mID == -1)
    {
        return;
    }
    ReliableNetEvent& event = GetNextReliableEvent();
    ReliableNetEvent::U::CreateChunkClusterEvent& data = event.u.createchunkcluster;
    event.mEvType = REL_CREATE_CHUNKCLUSTER;
    event.mSendMask &= mask;
    data.mEntClass = entityClass;
    data.mChunkClusterIndex = cluster->mID;
}

// FUNCTION: SWBF2 0x005b7350
void NetGame::SetEnterVehicleTurn(unsigned int player)
{
    netPlayers[player].lastEnterVehicleTurn = hostTurnNumber;
}

// FUNCTION: SWBF2 0x005bd480
void BeginReadAlign(NetPktGroup& group)
{
    alignCount = 0;
}

// FUNCTION: SWBF2 0x005caf60
void ClearPressed(ProcessedInputs* inputs)
{
    inputs->bits.mBoolInputs &= ~0x100u;
}

// FUNCTION: SWBF2 0x005cc710
int GetUniqObjectBits()
{
    return 6;
}

// FUNCTION: SWBF2 0x005bf500
void CleanReliableNetEvents(int player)
{
    unsigned int head = reliableNetEventHead;
    unsigned int index = reliableNetEventTail;
    while (index != head)
    {
        reliableEvents[index].mSendMask &= ~(1ui64 << player);
        if (++index >= 128)
        {
            index = 0;
        }
    }
}

// GLOBAL: SWBF2 0x007e8e80
static unsigned int lastFill = 123;

// FUNCTION: SWBF2 0x005c8b00
void FillTurn(unsigned int value)
{
    lastFill = value;
}

// FUNCTION: SWBF2 0x005c8b10
void UseTurn(unsigned int)
{
    return;
}

// FUNCTION: SWBF2 0x005cb530
void UpdateCurMaxPlayers(unsigned __int64 playingMask, bool includeLast)
{
    if (includeLast)
    {
        playingMask |= (unsigned __int64)1 << (netCurMaxPlayers - 1);
    }
    netCurMaxPlayers = 1;
    for (int player = 63; player >= 0; --player)
    {
        if (playingMask & ((unsigned __int64)1 << player))
        {
            netCurMaxPlayers = player + 1;
            return;
        }
    }
}

// GLOBAL: SWBF2 0x01e62f8c
unsigned int clientHostUploadSeqNo;
// GLOBAL: SWBF2 0x01e62f94
unsigned int clientUploadSeqNo;

// FUNCTION: SWBF2 0x005c5030
void ReadUploadAck(NetPktGroup& group)
{
    bool hasTurn;
    group.Read(hasTurn);
    if (hasTurn)
    {
        group.ReadBits(clientHostUploadSeqNo, 32);
    }
    else if (clientUploadSeqNo > 1)
    {
        clientHostUploadSeqNo = clientUploadSeqNo;
    }
}

// FUNCTION: SWBF2 0x005c64f0
void AloMiniStats::Drop(int player)
{
    mEnemyKills[player] = 0;
    mTeamKills[player] = 0;
    mDeaths[player] = 0;
    mCps[player] = 0;
    mPoints[player] = 0;
    changeTurn[player] = (unsigned int)-1;
}

// FUNCTION: SWBF2 0x005c7ee0
Trackable* ReadTrackable(NetPktGroup& group)
{
    Trackable* trackable = 0;
    bool hasTrackable;
    group.Read(hasTrackable);
    if (hasTrackable)
    {
        bool isArmed;
        group.Read(isArmed);
        if (isArmed)
        {
            unsigned int uniqId;
            group.ReadBits(uniqId, 8);
            NetNdObj* netNdObj = clientNdObjMap.FindIndex(uniqId);
            if (netNdObj)
            {
                Entity* ent = netNdObj->obj->GetEntity();
                if (ent->IsRtti(EntityBuildingArmed::GetRtti()))
                {
                    EntityBuildingArmed* b = static_cast<EntityBuildingArmed*>(netNdObj->obj);
                    trackable = b->GetTrackable();
                }
                else if (ent->IsRtti(EntityRemoteTerminal::GetRtti()))
                {
                    EntityRemoteTerminal* b = static_cast<EntityRemoteTerminal*>(netNdObj->obj);
                    trackable = b->GetTrackable();
                }
                else if (ent->IsRtti(EntityBuildingAnimatedArmed::GetRtti()))
                {
                    EntityBuildingAnimatedArmed* b = static_cast<EntityBuildingAnimatedArmed*>(netNdObj->obj);
                    trackable = b->GetTrackable();
                }
            }
        }
        else
        {
            unsigned int uniqId;
            group.ReadBits(uniqId, GetUniqObjectBits());
            NetObj* netObj = clientObjMap.Find(uniqId);
            bool firstCtrl;
            group.Read(firstCtrl);
            unsigned int ctrlIndex;
            if (firstCtrl)
            {
                ctrlIndex = 0;
            }
            else
            {
                group.ReadBits(ctrlIndex, 4);
            }
            if (netObj)
            {
                GameObject* o = netObj->obj;
                if (firstCtrl)
                {
                    trackable = o->GetControllable()->GetTrackable();
                }
                else
                {
                    const PblList<Controllable>& ctrlList = o->GetControllableList();
                    PblList<Controllable>::Itor itor(ctrlList);
                    while ((Controllable*)itor)
                    {
                        Controllable* ctrl = itor;
                        if (!ctrlIndex)
                        {
                            trackable = ctrl->GetTrackable();
                            break;
                        }
                        --ctrlIndex;
                        ++itor;
                    }
                }
            }
        }
    }
    return trackable;
}

// FUNCTION: SWBF2 0x005c6e10
void AloTrackable::Read(NetPktGroup& group)
{
    Trackable* trackable = ReadTrackable(group);
    Tracker* tracker = Tracker::GetTracker(curDst);
    if (trackable && trackable != tracker->GetTrackable())
    {
        if (trackable == reinterpret_cast<Trackable*>(-1))
        {
            Tracker* tracker = Tracker::GetTracker(curDst);
            if (tracker)
            {
                Tracker::SetTrackable(0, curDst);
            }
        }
        else
        {
            trackable->Track(curDst);
        }
    }
    for (unsigned int i = 0; i < 1; ++i)
    {
        int player = NetComm::GetLocalIndex(i + 1);
        if (player >= 0)
        {
            trackable = ReadTrackable(group);
            if (trackable)
            {
                tracker = Tracker::GetTracker(player);
                if (trackable != tracker->GetTrackable())
                {
                    if (trackable == reinterpret_cast<Trackable*>(-1))
                    {
                        Tracker* tracker = Tracker::GetTracker(player);
                        if (tracker)
                        {
                            Tracker::SetTrackable(0, player);
                        }
                    }
                    else
                    {
                        trackable->Track(player);
                    }
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c8b20
void FillTurns(int player, unsigned int endTurn)
{
    for (unsigned int turn = hostTurnNumber; turn < endTurn; ++turn)
    {
        int slot = turn & 31;
        if (hostRecv[slot] & ((unsigned __int64)1 << player))
        {
            continue;
        }
        int previousSlot = (turn - 1) & 31;
        hostTurns[slot].moves[player] = hostTurns[previousSlot].moves[player];
        hostRecv[slot] |= (unsigned __int64)1 << player;
        ++netPlayers[player].filledCount;
    }
}

// FUNCTION: SWBF2 0x005d38c0
template <>
void BblList<NetEntity, 512>::Remove(NetEntity& item)
{
    T1* node = (T1*)&item;
    node->next = nextFree;
    int index = (int)(node - array);
    nextFree = index;
    if (index == lastUsed)
    {
        while (index >= 0 && array[index].next != -2)
        {
            --index;
        }
        lastUsed = index;
    }
}

// FUNCTION: SWBF2 0x005d39a0
template <>
void BblList<NetEntity, 512>::Itor::Advance()
{
    int end = list->lastUsed + 1;
    while (idx < end && list->array[idx].next != -2)
    {
        ++idx;
    }
}

class StickAxisConvert : public ConvertValues
{
public:
    float GetMin() const { return -1.0f; }
    float GetMax() const { return 1.0f; }
    int GetBits() const { return 7; }
};

class MouseAxisConvert : public ConvertValues
{
public:
    float GetMin() const { return -16.0f; }
    float GetMax() const { return 16.0f; }
    int GetBits() const { return 11; }
};

// GLOBAL: SWBF2 0x01e64b84
StickAxisConvert stickAxisConvert;
// GLOBAL: SWBF2 0x01e64b5b
MouseAxisConvert mouseAxisConvert;

template <class CONVERT> void ClampFixed(CONVERT& convert, float& value);

// FUNCTION: SWBF2 0x005d3c00
template <> void ClampFixed<StickAxisConvert>(StickAxisConvert&, float& value)
{
    NetPktGroupClampFixed(value, -1.0f, 1.0f, 7);
}

// FUNCTION: SWBF2 0x005d3c30
template <> void ClampFixed<MouseAxisConvert>(MouseAxisConvert&, float& value)
{
    NetPktGroupClampFixed(value, -16.0f, 16.0f, 11);
}

// FUNCTION: SWBF2 0x00530df0
bool CanSendWaitingAck()
{
    return netIsClient;
}


// FUNCTION: SWBF2 0x005bc420
void Dump()
{
}

// FUNCTION: SWBF2 0x005c18a0
void HandleEvent_ConnectTowCable(ReliableNetEvent& event)
{
    ReliableNetEvent::U::ConnectTowCableEvent& data = event.u.connectTowCable;
    const GameObject* recordedOwner = 0;
    data.mOwnerObject.Get(recordedOwner);
    GameObject* owner = const_cast<GameObject*>(recordedOwner);
    if (data.mCollideWalker && data.mFireWeapon && owner)
    {
        WeaponTowCable* weapon = static_cast<WeaponTowCable*>(data.mFireWeapon);
        weapon->KillMyCable();
        OrdnanceDesc desc;
        OrdnanceClass* ordnanceClass = data.mFireWeapon->GetClass()->mOrdnanceClass;
        const WeaponClass* weaponClass = data.mFireWeapon->GetClass();
        EntityClass* ownerClass = owner->GetEntityClass();
        Character* character = owner->GetControllable()->GetCharacter();
        int team = character ? character->GetTeamNumber() : 0;
        desc.mStrength = weapon->mStrength;
        desc.mDamageOwner.mTeam = team;
        desc.mPosition = data.mCollidePosition.Get();
        desc.mDirection = data.mDirection.Get();
        desc.mFireWeapon = data.mFireWeapon;
        desc.mOwner = owner;
        desc.mDamageOwner.mCharacter = character;
        desc.mTarget = data.mCollideWalker;
        desc.mDamageOwner.mClass = ownerClass;
        desc.mDamageOwner.mWeapon = const_cast<WeaponClass*>(weaponClass);
        Ordnance* ordnance = ordnanceClass->Build(desc);
        weapon->SetOrdnance(ordnance);
        ordnance->Collide(data.mCollideWalker->GetCollisionObject(),
                          data.mCollidePosition.Get(),
                          gVectorUnitY, data.mCollideBodyID);
    }
}

// FUNCTION: SWBF2 0x005c1f00
void HandleEvent_DestroyTowCable(ReliableNetEvent& event)
{
    ReliableNetEvent::U::DestroyTowCableEvent& data = event.u.destroyTowCable;
    if (data.mFireWeapon)
    {
        WeaponTowCable* weapon = static_cast<WeaponTowCable*>(data.mFireWeapon);
        weapon->KillMyCable();
    }
}

Team* ReadTeam(NetPktGroup& group);

// FUNCTION: SWBF2 0x005c1f50
void ReadEvent_GameOver(NetPktGroup& group, ReliableNetEvent& event)
{
    event.u.gameover.mTeam = ReadTeam(group);
}

// FUNCTION: SWBF2 0x005c1f70
void HandleEvent_GameOver(ReliableNetEvent& event)
{
    (void)event;
    GameState::SetState(0x11E1FC01);
}

// FUNCTION: SWBF2 0x005c2230
void HandleEvent_UpdateDoorState(ReliableNetEvent& event)
{
    EntityDoor* door = EntityDoor::Find(event.u.updateDoorState.mHashID);
    door->SetIsLocked(event.u.updateDoorState.mIsLocked);
}

// FUNCTION: SWBF2 0x005c36e0
void NetGame::GetPlayerQOS(int player, unsigned int& updates, unsigned int& fills, int& loss)
{
    updates = netPlayers[player].updateCount;
    fills = netPlayers[player].filledCount;
    loss = 0;
    if (netPlayers[player].packetCount)
    {
        loss = netPlayers[player].dropCount * 100 / netPlayers[player].packetCount;
    }
}

// FUNCTION: SWBF2 0x005c38f0
void NetGame::SetSpectator(int player, int spectator)
{
    if (netPlayers[player].mSpectateID != spectator)
    {
        netPlayers[player].mSpectateID = spectator;
        Tracker* tracker = Tracker::GetTracker(player);
        if (tracker)
        {
            tracker->SetSpectating(spectator != -1);
        }
    }
}

// FUNCTION: SWBF2 0x005c44b0
void NetGame::SetStartTeam(int player, int team)
{
    int spectator = NetGame::GetSpectator(player);
    if (spectator != -1)
    {
        team = NetGame::GetStartTeam(spectator);
    }
    netPlayers[player].isTeam1 = team == 1;
}

// GLOBAL: SWBF2 0x007e8e51
unsigned char BOOT_REASON_BITS = 4;
// GLOBAL: SWBF2 0x01e64a2f
unsigned char clientBootReason;
// GLOBAL: SWBF2 0x01fa658c
int clientBootVotes;

// FUNCTION: SWBF2 0x005c7450
void AloBootVote::Read(NetPktGroup& group)
{
    unsigned int reason = 0;
    group.ReadBits((unsigned int&)clientBootPlayer, 6);
    group.ReadBits(reason, BOOT_REASON_BITS);
    clientBootReason = (unsigned char)reason;
    group.ReadBits(clientBootTurn, 32);
    int votes;
    group.ReadBits((unsigned int&)votes, 6);
    clientBootVotes = votes;
}

// FUNCTION: SWBF2 0x005c22d0
void ReadEvent_AddObjective(NetPktGroup& group, ReliableNetEvent& event)
{
    group.ReadBits(event.u.addObjective.mHashID, 32);
    group.ReadBits(event.u.addObjective.mPopupTextID, 32);
    group.Read(event.u.addObjective.team);
    group.ReadBits((unsigned int&)event.u.addObjective.state, 8);
}

namespace ObjectiveManager
{
void AddObjective(unsigned int nameHash, unsigned int popupHash, int team);
}
namespace ObjectiveManager
{
void SetObjectiveState(unsigned int nameHash, int state);
}

// FUNCTION: SWBF2 0x005c2320
void HandleEvent_AddObjective(ReliableNetEvent& event)
{
    ObjectiveManager::AddObjective(event.u.addObjective.mHashID,
                event.u.addObjective.mPopupTextID,
                event.u.addObjective.team);
    ObjectiveManager::SetObjectiveState(event.u.addObjective.mHashID,
                (int)event.u.addObjective.state);
}

// FUNCTION: SWBF2 0x005c2420
void HandleEvent_UpdateObjective(ReliableNetEvent& event)
{
    ObjectiveManager::SetObjectiveState(event.u.updateObjective.mHashID,
                (int)event.u.updateObjective.state);
}

// FUNCTION: SWBF2 0x005c2360
void NetGame::CreateEvent_UpdateObjective(HUD::Objective* objective, unsigned __int64 sendMask)
{
    if (!objective)
    {
        return;
    }
    ReliableNetEvent& event = GetNextReliableEvent();
    event.mEvType = REL_UPDATEOBJECTIVE;
    event.mSendMask &= sendMask;
    event.u.updateObjective.mHashID = objective->GetTextHashID();
    event.u.updateObjective.state = objective->GetState();
}

// FUNCTION: SWBF2 0x005c24e0
void ReadEvent_UpdateInvincibility(NetPktGroup& group, ReliableNetEvent& event)
{
    unsigned int player;
    group.ReadBits(player, 7);
    event.u.updateInvincibility.mCharacter = Character::FindPlayer(player);
    group.Read(event.u.updateInvincibility.mIsInvincible);
}

// FUNCTION: SWBF2 0x005c26c0
CommandPost* ReadCommandPost(NetPktGroup& group)
{
    bool present;
    group.Read(present);
    if (!present)
    {
        return 0;
    }
    unsigned int index;
    group.ReadBits(index, 4);
    return CommandPost::FindPostByIndex(index);
}

// GLOBAL: SWBF2 0x01fa6600
char uploadDir[8];
// GLOBAL: SWBF2 0x01e64ba0
HANDLE uploadFileHandle;

// FUNCTION: SWBF2 0x005c4760
int CreateUploadDirectory(const char* path)
{
    uploadDir[0] = 0;
    strlcat(uploadDir, path, sizeof(uploadDir));
    uploadDir[7] = 0;
    for (int i = 0; i < sizeof(uploadDir); ++i)
    {
        char c = uploadDir[i];
        if (c > 32 && c < 122)
        {
            continue;
        }
        uploadDir[i] = 0;
        break;
    }
    int ok = CreateDirectoryA(uploadDir, 0);
    return ok;
}

// FUNCTION: SWBF2 0x005c4800
void OpenUploadFile(const char* name)
{
    char fileName[32];
    strcpy(fileName, uploadDir);
    strcat(fileName, "\\");
    if (strlen(name) < 16)
    {
        strcat(fileName, name);
    }
    else
    {
        strcat(fileName, "toolong.dat");
    }
    uploadFileHandle = CreateFileA(fileName, GENERIC_READ | GENERIC_WRITE,
                                    0, 0, CREATE_ALWAYS,
                                    FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, 0);
}

// FUNCTION: SWBF2 0x005c4960
int WriteUploadFile(const char* buffer, int size)
{
    DWORD written = 0;
    WriteFile(uploadFileHandle, buffer, size, &written, 0);
    size = written;
    return size;
}

// FUNCTION: SWBF2 0x005c49a0
void CloseUploadFile()
{
    CloseHandle(uploadFileHandle);
}

// GLOBAL: SWBF2 0x01e62f6c
unsigned int ndSkippedMask[8];

// FUNCTION: SWBF2 0x005b44d0
void ClearSkippedND(GameObject* object)
{
    unsigned int netId = object->GetNetUniqueId() - 64;
    unsigned int word = netId >> 5;
    unsigned int bit = netId % 32;
    ndSkippedMask[word] &= ~(1u << bit);
}

// FUNCTION: SWBF2 0x005b8490
void NetGameP::SetNotPlaying(int player)
{
    hostNextPlayingMask &= ~((unsigned __int64)1 << player);
    NetComm::Disconnect(player);
    int guest = NetGame::GetMyGuest(player, 0);
    if (guest >= 0)
    {
        hostNextPlayingMask &= ~((unsigned __int64)1 << guest);
    }
}

// FUNCTION: SWBF2 0x005cb040
void ReadMiniMove(NetPktGroup& group, ProcessedInputs* inputs)
{
    *inputs = noInputs;
    group.ReadFixed(inputs->mFloatInputs[0], -1.0f, 1.0f, 3);
    group.ReadFixed(inputs->mFloatInputs[1], -1.0f, 1.0f, 3);
    group.ReadFixed(inputs->mFloatInputs[2], -1.0f, 1.0f, 3);
    group.ReadFixed(inputs->mFloatInputs[3], -1.0f, 1.0f, 3);
}

// TEMPLATE: SWBF2 0x005d36b0
// Factory<Entity,EntityClass,EntityDesc>::FindNetIndex
// TEMPLATE: SWBF2 0x005d3740
// Factory<Ordnance,OrdnanceClass,OrdnanceDesc>::FindNetIndex
// TEMPLATE: SWBF2 0x005d37a0
// Factory<Weapon,WeaponClass,WeaponDesc>::FindNetIndex
template <class TYPE, class CLASS, class DESC>
CLASS* Factory<TYPE, CLASS, DESC>::FindNetIndex(int index)
{
    typedef Factory<TYPE, CLASS, DESC> FactoryType;
    for (typename PblList<FactoryType>::Itor it(sList.GetFirst()); it; ++it)
    {
        if (it->mNetIndex == index)
        {
            return static_cast<CLASS*>(&*it);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005cfbb0
void NetGame::SelectSpawn(int player, int team, int classIndex, CommandPost* post)
{
    ++spawnInfo[player].changeCount;
    spawnInfo[player].changedTurn = clientTurnNumber;
    spawnInfo[player].cancel = false;
    spawnInfo[player].autoassign = false;
    spawnInfo[player].classIdx = classIndex;
    spawnInfo[player].team = team;
    spawnInfo[player].post = post;
    spawnInfo[player].postHandle = post ? post->GetGameObject() : 0;
}

// FUNCTION: SWBF2 0x005cfc60
void NetGame::CancelSpawn(int player)
{
    ++spawnInfo[player].changeCount;
    spawnInfo[player].changedTurn = clientTurnNumber;
    spawnInfo[player].cancel = true;
}

// FUNCTION: SWBF2 0x005cfca0
void NetGame::SpectateSpawn(int player, bool spectate)
{
    if (spectate)
    {
        ++spawnInfo[player].changeCount;
        spawnInfo[player].changedTurn = clientTurnNumber;
        spawnInfo[player].cancel = false;
    }
    spawnInfo[player].spectate = spectate;
}

// FUNCTION: SWBF2 0x005cfd00
void NetGame::AutoAssignSpawn(int player, bool autoassign)
{
    if (autoassign)
    {
        ++spawnInfo[player].changeCount;
        spawnInfo[player].changedTurn = clientTurnNumber;
        spawnInfo[player].cancel = false;
        spawnInfo[player].spectate = false;
    }
    spawnInfo[player].autoassign = autoassign;
}

// FUNCTION: SWBF2 0x005c0850
void NetGame::CreateEvent_ChangeCommandPostTeams(unsigned __int64 sendMask)
{
    ReliableNetEvent& event = GetNextReliableEvent();
    ReliableNetEvent::U::ChangeCommandPostTeamsEvent& data = event.u.changecommandpostteams;
    event.mEvType = REL_CHANGECOMMANDPOSTTEAMS;
    event.mSendMask &= sendMask;
    data.mTeamBits = 0;
    data.mAliveBits = 0;
    for (CommandPostItor it = CommandPostManager::Instance()->GetItor(); it; ++it)
    {
        CommandPost* post = *it;
        GameObject* object = post->GetGameObject();
        if (object->GetEntityType() != ENT_POST)
        {
            continue;
        }
        data.mTeamBits |= object->GetTeam() << (2 * post->GetIndex());
        data.mAliveBits |= object->IsAlive() << post->GetIndex();
    }
}

// FUNCTION: SWBF2 0x005c1fd0
void HandleEvent_ScriptMessage(ReliableNetEvent& event)
{
    HUD::GameEvents::DisplayCenterMessage(event.u.scriptMessage.mLocalizeKey, event.u.scriptMessage.mTeamMask, 0);
}

// FUNCTION: SWBF2 0x005c20c0
void HandleEvent_PlayVoiceOver(ReliableNetEvent& event)
{
    VoiceOverManager::PlayVoiceOver(event.u.playVoiceOver.mSoundNameHashID, event.u.playVoiceOver.mTeamMask);
}

// FUNCTION: SWBF2 0x005c6960
void AloTrackable::Drop(int player)
{
    for (int i = 0; i < 2; ++i)
    {
        mTrackable[player][i] = reinterpret_cast<Trackable*>(-1);
    }
    Tracker* tracker = Tracker::GetTracker(player);
    if (tracker)
    {
        Tracker::SetTrackable(0, player);
    }
    changeTurn[player] = (unsigned int)-1;
}

// FUNCTION: SWBF2 0x005c3a60
int NetGame::FindSpectateTarget(int player)
{
    int previous = netPlayers[player].mSpectateID;
    int result = -1;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        int candidate = i + previous + 1;
        if (candidate >= netCurMaxPlayers)
        {
            candidate -= netCurMaxPlayers;
        }
        if (NetGame::IsPlaying(candidate, false) && netPlayers[candidate].mSpectateID == -1)
        {
            Character* character = Character::FindPlayer(candidate);
            if (character && character->GetUnit())
            {
                result = candidate;
                break;
            }
        }
    }
    if (result == -1)
    {
        result = previous;
        if (previous == -1)
        {
            result = player;
        }
    }
    return result;
}

// FUNCTION: SWBF2 0x005c3b30
int NetGame::FindSpectateTarget(int player, int team)
{
    int result = -1;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        int candidate = i + player + 1;
        if (candidate >= netCurMaxPlayers)
        {
            candidate -= netCurMaxPlayers;
        }
        if (NetGame::IsPlaying(candidate, false) && netPlayers[candidate].mSpectateID == -1)
        {
            Character* character = Character::FindPlayer(candidate);
            if (character && character->GetUnit() && character->GetTeamNumber() == team)
            {
                result = candidate;
                break;
            }
        }
    }
    return result;
}

// FUNCTION: SWBF2 0x005c2c70
void RecordMove(ProcessedInputs& inputs)
{
    unsigned int bits = inputs.bits.mBoolInputs;
    PblJournal::Record(bits, PJT_MOVES);
    inputs.bits.mBoolInputs = bits;
    bits = inputs.bits.mMouseMode;
    PblJournal::Record(bits, PJT_MOVES);
    inputs.bits.mMouseMode = bits;
    for (int i = 0; i < 4; ++i)
    {
        PblJournal::Record(inputs.mFloatInputs[i], PJT_MOVES);
    }
}

// FUNCTION: SWBF2 0x005cbba0
void SpawnDisplay::Read(NetPktGroup& group)
{
    group.Read((int&)mBaseTurn);
    group.Read(mBaseCycleTime);
    if (mBaseCycleTime > mLastSpawnTime)
    {
        mLastSpawnTime = mBaseCycleTime;
    }
}

// GLOBAL: SWBF2 0x01faa6d0
unsigned int trackedEntityCreateCount;

// GLOBAL: SWBF2 0x01faa6c0
unsigned int trackedEntityDeleteCount;

class NetPowerupItem : public PowerupItem
{
public:
    int GetIndex() { return sMemoryPool.GetIndex(this); }
};

// FUNCTION: SWBF2 0x005ce500
void TrackCreate(GameObject* object)
{
    if (!IsNetObjectFeatureEnabled())
    {
        return;
    }
    if (object->GetEntityType() == ENT_POWERUP)
    {
        NetPowerupItem* track = (NetPowerupItem*)object;
        int idx = track->GetIndex();
        if (trackedEntityCreateCount == 265)
        {
            ++gDebugCount;
        }
        ++trackedEntityCreateCount;
    }
}

// FUNCTION: SWBF2 0x005ce580
void TrackDelete(GameObject* object)
{
    if (!IsNetObjectFeatureEnabled())
    {
        return;
    }
    if (object->GetEntityType() == ENT_POWERUP)
    {
        NetPowerupItem* track = (NetPowerupItem*)object;
        int idx = track->GetIndex();
        if (trackedEntityDeleteCount == 246)
        {
            ++gDebugCount;
        }
        ++trackedEntityDeleteCount;
    }
}

// FUNCTION: SWBF2 0x005c2140
void NetGame::CreateEvent_UpdateDoorState(unsigned int entityID, unsigned __int64 sendMask)
{
    EntityEx* entity = EntityEx::Find(entityID);
    if (entity && entity->IsRtti(EntityDoor::GetRtti()))
    {
        EntityDoor* door = reinterpret_cast<EntityDoor*>(entity);
        ReliableNetEvent& event = GetNextReliableEvent();
        event.mEvType = REL_UPDATEDOORSTATE;
        event.mSendMask &= sendMask;
        event.u.updateDoorState.mHashID = entityID;
        event.u.updateDoorState.mIsLocked = door->mIsLocked;
    }
}

// GLOBAL: SWBF2 0x007e8e78
static unsigned char BOOT_JOYSTICK_BITS = 2;
// GLOBAL: SWBF2 0x01fa65a8
unsigned __int64 hostBootAgreeMask;
// GLOBAL: SWBF2 0x01fa87c0
unsigned int hostBootTurn;
// GLOBAL: SWBF2 0x01e64b86
unsigned char hostBootReason;

// FUNCTION: SWBF2 0x005d1240
void ReadBootRequest(NetPktGroup& group, int player)
{
    bool hasInfo;
    group.Read(hasInfo);
    if (!hasInfo)
    {
        return;
    }
    bool selectsHost;
    group.Read(selectsHost);
    unsigned int guestSlot;
    group.ReadBits(guestSlot, BOOT_JOYSTICK_BITS);
    int guest = NetGame::GetGuestIndex(player, guestSlot);
    if (selectsHost)
    {
        unsigned int host;
        group.ReadBits(host, 6);
        if (hostBootPlayer < 0)
        {
            hostBootPlayer = host;
            hostBootAgreeMask = (unsigned __int64)1 << guest;
            hostBootTurn = hostTurnNumber + 60 * netTurnsPerSecond;
        }
        unsigned int spectator = 0;
        group.ReadBits(spectator, BOOT_REASON_BITS);
        hostBootReason = spectator;
    }
    else
    {
        hostBootAgreeMask |= (unsigned __int64)1 << guest;
    }
}

// FUNCTION: SWBF2 0x005c0df0
void HandleEvent_CreateChunkCluster(ReliableNetEvent& event, PblMatrix&)
{
    ReliableNetEvent::U::CreateChunkClusterEvent& data = event.u.createchunkcluster;
    unsigned char clusterIndex = (unsigned char)data.mChunkClusterIndex;
    EntityGeometryClass* entityClass = data.mEntClass;
    if (entityClass)
    {
        ChunkCluster* cluster = ChunkClusterManager::GetClusterByID(clusterIndex, true);
        if (cluster)
        {
            cluster->ClearChunks();
            cluster->mParentClass = entityClass;
            for (int i = 0; i < 10; ++i)
            {
                if (entityClass->mChunkDescriptors[i])
                {
                    ChunkDescriptor descriptor;
                    descriptor = *entityClass->mChunkDescriptors[i];
                    if (entityClass->mFullChunkCollisions[i]->GetNumBodies() == 0)
                    {
                        descriptor.mFullCollisionModel = 0;
                    }
                    else
                    {
                        descriptor.mFullCollisionModel = entityClass->mFullChunkCollisions[i];
                    }
                    descriptor.mMatrix = relChunkClusterData[clusterIndex][i].mMatrix;
                    descriptor.mVelocity = relChunkClusterData[clusterIndex][i].mVelocity;
                    descriptor.mOmega = relChunkClusterData[clusterIndex][i].mOmega;
                    cluster->AddChunk(descriptor);
                }
            }
            if (!cluster->IsInScene())
            {
                cluster->AddToScene();
            }
            cluster->RecalculateCenterAndRadius();
        }
    }
}

// FUNCTION: SWBF2 0x005c1160
void HandleEvent_CorrectChunkCluster(ReliableNetEvent& event, PblMatrix& matrix)
{
    ReliableNetEvent::U::CorrectChunkClusterEvent& data = event.u.correctchunkcluster;
    if (data.mEntClass)
    {
        ChunkCluster* cluster = ChunkClusterManager::GetClusterByID(
            (unsigned char)data.mChunkClusterIndex, false);
        int index = (int)data.mChunkIndex;
        if (cluster && index < 10)
        {
            EntityGeometryClass* entityClass = static_cast<EntityGeometryClass*>(data.mEntClass);
            if (entityClass->mChunkDescriptors[index])
            {
                ChunkDescriptor descriptor;
                descriptor = *entityClass->mChunkDescriptors[index];
                if (entityClass->mFullChunkCollisions[index]->GetNumBodies() == 0)
                {
                    descriptor.mFullCollisionModel = 0;
                }
                else
                {
                    descriptor.mFullCollisionModel = entityClass->mFullChunkCollisions[index];
                }
                descriptor.mMatrix = matrix;
                if (!cluster->IsInScene())
                {
                    cluster->AddToScene();
                }
                cluster->mChunks[index].ClearAll();
                cluster->mChunks[index].SetData(descriptor);
                cluster->mChunks[index].mIsMoving = false;
                cluster->RecalculateCenterAndRadius();
            }
        }
    }
}


 
 

// FUNCTION: SWBF2 0x005d13c0
void NetGame::AddChatMessageToQue(bool local, int player, unsigned __int64 mask,
                int code, const unsigned short* text, int flag)
{
    if (!local && NetGame::IsHost() && NetGame::IsDedicated())
    {
        if (NetAdmin::ParseLineRemote(player, const_cast<unsigned short*>(text), true, true) ||
            NetAdmin::ParseLineLocal(const_cast<unsigned short*>(text), true))
        {
            return;
        }
        player = 64;
        mask = ~(unsigned __int64)0;
        local = true;
    }
    NG_ChatMsg* message = new NG_ChatMsg;
    message->player_id = player;
    message->mask = mask;
    PblUnicode::ustrncpy(message->msg, text, 128);
    message->msg[127] = 0;
    message->flag = flag;
    message->code = code;
    PblList<NG_ChatMsg>& queue = local ? netGameChatMsgServer : netGameChatMsgClient;
    queue.AddLast(&message->mNode, message);
}

// FUNCTION: SWBF2 0x005d1cb0
void LogChatMessage(const NG_ChatMsg& message)
{
    if (!message.flag)
    {
        NetAdmin::ChatMessage(message.player_id, message.msg);
    }
    if (!chatLog)
    {
        return;
    }
    const char* name = message.player_id == 64 ? "ADMIN" : netPlayers[message.player_id].name;
    for (; *name; ++name)
    {
        fputc(*name, chatLog);
    }
    fputc(':', chatLog);
    fputc(' ', chatLog);
    for (const unsigned short* ch = message.msg; *ch; ++ch)
    {
        fputc(*ch, chatLog);
    }
    fputc('\n', chatLog);
}

void ReadPositionVector(NetPktGroup& group, PblVector3& value);
void ReadCosSin(NetPktGroup& group, PblAngle& angle);

// FUNCTION: SWBF2 0x005d2fd0
void ReadSpawnCamera(NetPktGroup& group)
{
    bool hasCamera;
    group.Read(hasCamera);
    if (hasCamera)
    {
        NetPlayer& player = netPlayers[curDst];
        ReadPositionVector(group, player.spawnCenter);
        PblAngle frontAngle;
        ReadCosSin(group, frontAngle);
        player.spawnFront = PblVector3(frontAngle.GetCos(), 0.0f, frontAngle.GetSin());
    }
}

// FUNCTION: SWBF2 0x005d2d50
void ReadPingData(NetPktGroup& group)
{
    unsigned int player;
    group.ReadBits(player, 6);
    if (player == (unsigned int)curDst)
    {
        unsigned int drops = 0;
        group.ReadBits(drops, 7);
        if (!NetGame::IsHost())
        {
            netPlayers[player].dropCount = drops;
            netPlayers[player].packetCount = 100;
        }
    }
    unsigned int ping = 250;
    group.ReadBits(ping, 10);
    for (int slot = -1; slot < 1; ++slot)
    {
        int destination = slot == -1 ? player : netPlayers[player].myGuest[slot];
        if (destination >= 0)
        {
            NetComm::SetPing(destination, ping);
            unsigned int loss;
            group.ReadBits(loss, 10);
            NetComm::SetPLKills(destination, (int)loss - 50);
        }
    }
}

// GLOBAL: SWBF2 0x007e8e84
int gDebugDst = 1;

// FUNCTION: SWBF2 0x005d1f30
void ReceivedUpdateAck(int player, unsigned int turn)
{
    if (player == gDebugDst)
    {
        ++gDebugCount;
    }
    unsigned int updateTurn = turn;
    if (!updateTurn)
    {
        return;
    }
    NetPlayer& netPlayer = netPlayers[player];
    unsigned int slot;
    for (slot = 0; slot < 2; ++slot)
    {
        if (netPlayer.sentUpdateTurn[slot] == updateTurn)
        {
            break;
        }
    }
    if (slot == 2)
    {
        return;
    }
    netPlayer.sentUpdateTurn[slot] = 0;
    float now = NetComm::GetTime();
    float rtt = now - netPlayer.sentUpdateTime[slot];
    float error = rtt - netPlayer.averageRtt;
    netPlayer.averageRtt += error * 0.125f;
    if (error < 0.0f)
    {
        error = -error;
        if (netPlayer.updateWindowSize < 2)
        {
            ++netPlayer.updateWindowSize;
        }
    }
    float devErr = error - netPlayer.deviationRtt;
    netPlayer.deviationRtt += devErr * 0.25f;
    const float minDev = netSecondsPerTurn / 2.0f;
    if (netPlayer.deviationRtt < minDev)
    {
        netPlayer.deviationRtt = minDev;
    }
    NetComm::SetPing(player, (int)(netPlayer.averageRtt * 1000.0f));
}

// FUNCTION: SWBF2 0x005c55f0
void AloReinforcements::Read(NetPktGroup& group)
{
    unsigned int bits;
    for (int i = 0; i < 2; ++i)
    {
        group.ReadBits(bits, 10);
        int count = bits;
        group.ReadBits(bits, 6);
        int active = bits;
        bool bleeding;
        group.Read(bleeding);
        float bleed;
        if (bleeding)
        {
            group.ReadFixed(bleed, 0.0f, 8.0f, 5);
        }
        else
        {
            bleed = 0.0f;
        }
        int points;
        group.ReadBits((unsigned int&)points, 10);
        points -= 128;
        Team::GetPtr(i + 1)->SetValues(count, active, points);
        Team::GetPtr(i + 1)->SetBleedRate(bleed);
    }
}

// FUNCTION: SWBF2 0x005c67d0
void AloMiniStats::Read(NetPktGroup& group)
{
    unsigned int kills, teamKills, deaths, commandPostPoints, points;
    group.ReadBits(kills, 12);
    group.ReadBits(teamKills, 8);
    group.ReadBits(deaths, 8);
    group.ReadBits(commandPostPoints, 16);
    group.ReadBits(points, 16);
    Character* character = Character::FindPlayer(NetComm::GetLocalIndex(0));
    if (character && !NetGame::IsHost())
    {
        PlayerStats& stats = Character::GetPlayerStats(character);
        stats.SetClientMiniStats(kills, teamKills, deaths, commandPostPoints);
        stats.SetPoints((short)points);
    }
}

extern bool netDropLagJumps;

// FUNCTION: SWBF2 0x005c5a20
void AloHeroData::Read(NetPktGroup& group)
{
    if (!netHeroesEnabled)
    {
        return;
    }
    bool unlocked;
    unsigned int player, nextPlayer, lastPlayer, killer;
    unsigned int selectedTurn, spawnedTurn, diedTurn;
    group.Read(unlocked);
    group.ReadBits(player, 7);
    group.ReadBits(nextPlayer, 7);
    group.ReadBits(lastPlayer, 7);
    group.ReadBits(killer, 7);
    group.ReadBits(selectedTurn, 19);
    group.ReadBits(spawnedTurn, 19);
    group.ReadBits(diedTurn, 19);
    if (!NetGame::IsHost())
    {
        netHeroKillerID[mTeamNum] = (int)killer - 1;
        if (!netHeroUnlocked[mTeamNum] && unlocked)
        {
            HeroMessageDisplay::PlayHeroSoundUnlock(mTeamNum);
        }
        if (netHeroNextPlayerID[mTeamNum] == -1 && nextPlayer && NetGame::GetJoystickIndex(nextPlayer - 1) != -1)
        {
            HeroMessageDisplay::PlayHeroSoundSelectable(mTeamNum);
        }
        if (!netHeroSpawnedTurn[mTeamNum] && spawnedTurn && player)
        {
            HeroMessageDisplay::PlayHeroSoundSpawn(mTeamNum);
        }
        if (!netHeroDiedTurn[mTeamNum] && diedTurn && lastPlayer)
        {
            HeroMessageDisplay::PlayHeroSoundDefeat(mTeamNum);
        }
        netHeroPlayerID[mTeamNum] = (int)player - 1;
        netHeroNextPlayerID[mTeamNum] = (int)nextPlayer - 1;
        netHeroLastPlayerID[mTeamNum] = (int)lastPlayer - 1;
        netHeroUnlocked[mTeamNum] = unlocked;
        netHeroSelectedTurn[mTeamNum] = selectedTurn;
        netHeroSpawnedTurn[mTeamNum] = spawnedTurn;
        netHeroDiedTurn[mTeamNum] = diedTurn;
    }
    else
    {
        if (netHeroSndPlayHostUnlock[mTeamNum])
        {
            HeroMessageDisplay::PlayHeroSoundUnlock(mTeamNum);
            netHeroSndPlayHostUnlock[mTeamNum] = false;
        }
        if (netHeroSndPlayHostSelectable[mTeamNum] && NetGame::GetJoystickIndex(netHeroNextPlayerID[mTeamNum]) != -1)
        {
            HeroMessageDisplay::PlayHeroSoundSelectable(mTeamNum);
            netHeroSndPlayHostSelectable[mTeamNum] = false;
        }
        if (netHeroSndPlayHostSpawn[mTeamNum])
        {
            HeroMessageDisplay::PlayHeroSoundSpawn(mTeamNum);
            netHeroSndPlayHostSpawn[mTeamNum] = false;
        }
        if (netHeroSndPlayHostDefeat[mTeamNum])
        {
            HeroMessageDisplay::PlayHeroSoundDefeat(mTeamNum);
            netHeroSndPlayHostDefeat[mTeamNum] = false;
        }
    }
}

// FUNCTION: SWBF2 0x005c7840
void AloMissionTimer::Read(NetPktGroup& group)
{
    Timer* timer = MissionTimer::GetTimer();
    bool valid;
    group.Read(valid);
    if (valid)
    {
        if (!timer)
        {
            timer = new Timer("mission");
        }
        float time;
        group.ReadFixed(time, 0.0f, 4096.0f, 16);
        timer->SetTime(time);
        bool hasRate;
        group.Read(hasRate);
        float rate = 1.0f;
        if (hasRate)
        {
            group.ReadFixed(rate, -16.0f, 16.0f, 8);
        }
        timer->SetRate(rate);
        bool active;
        group.Read(active);
        if (active)
        {
            timer->Start();
        }
        else
        {
            timer->Stop();
        }
        unsigned int show, team;
        group.ReadBits(show, 2);
        group.ReadBits(team, 3);
        MissionTimer::Set(timer, (MissionTimer::Show)((int)show - 1), team);
    }
    else
    {
        if (timer)
        {
            delete timer;
        }
        MissionTimer::Set(0, MissionTimer::SHOW_NEUTRAL, 0);
    }
}

// FUNCTION: SWBF2 0x005c6140
void AloMapDisplayMarkers::Read(NetPktGroup& group)
{
    for (int i = 0; i < TargetManager::MAX_MARKERS; ++i)
    {
        TargetManager::Marker* tmpMarker = &TargetManager::gMarkers[i];
        bool bRemoveIt = tmpMarker->actor || tmpMarker->region;
        if (bRemoveIt)
        {
            TargetManager::RemoveMarker(tmpMarker->actor, tmpMarker->region,
                                        tmpMarker->teamMask);
        }
    }
    bool more = false;
    group.Read(more);
    while (more)
    {
        GameObject* obj = 0;
        RedRegion* region = 0;
        unsigned int teamMask = 0;
        float fScale = 1.0f;
        unsigned int uTexture = 0;
        unsigned int uColorHash = 0;
        float pulseStartScale = 1.0f;
        bool showHudTarget = false;
        bool flashAlpha;
        bool pulseSize;
        bool bHasObject = false;
        group.Read(bHasObject);
        if (bHasObject)
        {
            obj = ReadObjectPtr(group);
        }
        bool bHasRegion = false;
        group.Read(bHasRegion);
        if (bHasRegion)
        {
            unsigned int regionId;
            group.ReadBits(regionId, 32);
            region = RedRegion::Find(regionId);
        }
        group.ReadBits(teamMask, 3);
        group.ReadFixed(fScale, 0.0f, 16.0f, 8);
        group.ReadBits(uTexture, 32);
        group.ReadBits(uColorHash, 32);
        group.Read(flashAlpha);
        group.Read(pulseSize);
        group.ReadFixed(pulseStartScale, 0.0f, 16.0f, 8);
        group.Read(showHudTarget);
        if (obj || region)
        {
            TargetManager::AddMarker(obj, region, uTexture, fScale, teamMask,
                                     uColorHash, showHudTarget, flashAlpha,
                                     pulseSize, pulseStartScale);
        }
        group.Read(more);
    }
}

// GLOBAL: SWBF2 0x007ec50c
int debugUnlinkHandleId = 138;

// FUNCTION: SWBF2 0x005cb140
void Unlink(GameObject* object)
{
    if (object->GetHandleId() == (unsigned int)debugUnlinkHandleId)
    {
        ++gDebugCount;
    }
    Controllable* active = object->GetControllable();
    if (active)
    {
        Character* character = active->GetCharacter();
        if (character)
        {
            Controllable* remote = character->GetRemote();
            if (remote && remote->GetPilot() == active)
            {
                remote->SetPilot(0);
                character->SetRemote(0);
            }
            Controllable* vehicle = character->GetVehicle();
            if (vehicle && vehicle->GetPilot() == active)
            {
                vehicle->SetPilot(0);
                character->SetVehicle(0);
            }
        }
    }
    eViewObjectType viewType;
    switch (object->GetEntityType())
    {
    case 1: viewType = TYPE_SOLDIER; break;
    case 4: viewType = TYPE_DROIDEKA; break;
    case 7: viewType = TYPE_HOVER; break;
    default: viewType = TYPE_COCKPIT; break;
    }
    for (PblList<Controllable>::Itor it(object->GetControllableList()); it; ++it)
    {
        Controllable* controllable = it;
        if (controllable && controllable->WasPlayer())
        {
            bool affectsPlayer = true;
            Character* player = Character::FindPlayer(controllable->GetInitialPlayerId());
            if (player && player->GetControllable() != controllable)
            {
                affectsPlayer = false;
            }
            const int joystick = NetGame::GetJoystickIndex(controllable->GetInitialPlayerId());
            if (joystick >= 0 && affectsPlayer)
            {
                Tracker* tracker = Tracker::GetTracker(controllable->GetInitialPlayerId());
                if (tracker->IsFirstPersonView())
                {
                    FirstPerson::Activate(joystick, false);
                    FirstPerson::Update(joystick, viewType, -2, 0, 0);
                    viewType = TYPE_COCKPIT;
                }
            }
        }
        Character* character = controllable->GetCharacter();
        if (character)
        {
            if (character->GetRemote() && character->GetRemote()->GetGameObject() == object)
            {
                character->SetRemote(0);
            }
            if (character->GetVehicle() && character->GetVehicle()->GetGameObject() == object)
            {
                character->SetVehicle(0);
            }
            if (character->GetUnit() && character->GetUnit()->GetGameObject() == object)
            {
                character->SetUnit(0);
            }
        }
        controllable->SetPilot(0);
    }
}

// FUNCTION: SWBF2 0x005d0a60
void ReadSuicideRequest(NetPktGroup& pktGroup, int src)
{
    for (int i = 0; i < 2; ++i)
    {
        bool hasData;
        int idx;
        if (i == 0)
        {
            idx = src;
        }
        else
        {
            idx = NetGame::GetMyGuest(src, i - 1);
        }
        pktGroup.Read(hasData);
        if (hasData)
        {
            unsigned int changeCount;
            pktGroup.ReadBits(changeCount, 8);
            bool newBatch = changeCount != suicideChangeCounts[idx];
            suicideChangeCounts[idx] = changeCount;
            bool killMeNow = false;
            pktGroup.Read(killMeNow);
            if (newBatch && killMeNow)
            {
                Character* c = Character::FindPlayer(idx);
                if (c && c->GetUnit())
                {
                    if (c->GetRemote())
                    {
                        c->GetUnit()->ExitRemote();
                    }
                    if (c->GetVehicle())
                    {
                        c->GetUnit()->ExitVehicle(true, true);
                    }
                    GameObject* gameObj = c->GetUnit()->GetGameObject();
                    gameObj->InstantDeath(DamageOwner(c, 0, 0, 0, 0, 1.0f), true);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c8100
void RecordLag(int player, int turn, unsigned int sampleTurn)
{
    NetPlayer& netPlayer = netPlayers[player];
    float thisLag = (float)turn * netSecondsPerTurn;
    float error = thisLag - netPlayer.avgLag;
    float absErr = fabsf(error);
    bool drop = netDropLagJumps && absErr >= netPlayer.errLag * 2.0f;
    if (drop)
    {
        ++gDebugCount;
    }
    netPlayer.avgLag += error * 0.125f;
    float devErr = absErr - netPlayer.errLag;
    netPlayer.errLag += devErr * 0.25f;
    if (!drop)
    {
        unsigned int& n = netPlayer.lagSample;
        netPlayer.lagSamples[n] = turn;
        netPlayer.lagTurns[n] = sampleTurn;
        if (++n >= 20)
        {
            n = 0;
        }
    }
}

static const unsigned int CAPTURE_RATIO_BITS = 6;
static const unsigned int CAPTURE_RATIO_MAX = (1U << CAPTURE_RATIO_BITS) - 1U;

// SYNTHETIC: SWBF2 0x004054e0
// `dynamic initializer for 'CAPTURE_RATIO_SCALE''

// GLOBAL: SWBF2 0x01ecb9bc
// CAPTURE_RATIO_SCALE
static const float CAPTURE_RATIO_SCALE = 1.0f / CAPTURE_RATIO_MAX;

// FUNCTION: SWBF2 0x005cb970
void ReadCaptureDisplay(NetPktGroup& group, int player)
{
    int view = NetGame::GetJoystickIndex(player);
    unsigned int showCaptureFlag;
    group.ReadBits(showCaptureFlag, 3);
    if (showCaptureFlag == 0)
    {
        CaptureDisplay* display = (unsigned int)view < 1 ? CaptureDisplay::sInstance[view] : 0;
        display->UpdateClear();
    }
    else if (showCaptureFlag == 1 || showCaptureFlag == 3)
    {
        unsigned int showTeam;
        unsigned int otherTeam;
        unsigned int ratio;
        float fRatio;
        group.ReadBits(showTeam, 3);
        group.ReadBits(otherTeam, 3);
        group.ReadBits(ratio, CAPTURE_RATIO_BITS);
        fRatio = (float)ratio * CAPTURE_RATIO_SCALE;
        CaptureDisplay* display = (unsigned int)view < 1 ? CaptureDisplay::sInstance[view] : 0;
        display->UpdateCapture(showTeam, otherTeam, fRatio);
    }
    else if (showCaptureFlag == 2 || showCaptureFlag == 4)
    {
        unsigned int team;
        unsigned int ratio;
        float fRatio;
        group.ReadBits(team, 3);
        group.ReadBits(ratio, CAPTURE_RATIO_BITS);
        fRatio = (float)ratio * CAPTURE_RATIO_SCALE;
        CaptureDisplay* display = (unsigned int)view < 1 ? CaptureDisplay::sInstance[view] : 0;
        display->UpdateNeutralize(team, fRatio);
    }
}

// FUNCTION: SWBF2 0x005cff00
void ReadLockedTargets(NetPktGroup& group, int src)
{
    for (int i = 0; i < 2; ++i)
    {
        int playerId = NetGame::GetGuestIndex(src, i);
        Character* player = Character::FindPlayer(playerId);
        Controllable* controllable = player ? player->GetControllable() : 0;
        bool hasData;
        group.Read(hasData);
        if (hasData)
        {
            bool hasLockTargetObj;
            group.Read(hasLockTargetObj);
            if (hasLockTargetObj)
            {
                bool neverDies;
                group.Read(neverDies);
                if (neverDies)
                {
                    unsigned int uniqId;
                    group.ReadBits(uniqId, 8);
                    NetNdObj* netNdObj = clientNdObjMap.FindIndex(uniqId);
                    GameObject* gameObj = netNdObj ? netNdObj->orig : 0;
                    if (controllable)
                    {
                        controllable->SetTargetLockedObj(gameObj);
                    }
                }
                else
                {
                    unsigned int uniqId;
                    group.ReadBits(uniqId, 6);
                    GameObject* gameObj = 0;
                    if (uniqId != 63)
                    {
                        NetObjState* objState = netPlayers[src].lastObjState->Find(uniqId);
                        if (objState)
                        {
                            gameObj = objState->handle;
                        }
                    }
                    else
                    {
                        gameObj = 0;
                    }
                    if (controllable)
                    {
                        controllable->SetTargetLockedObj(gameObj);
                    }
                }
            }
            else if (controllable)
            {
                controllable->SetTargetLockedObj(0);
            }
        }
    }
}

namespace ObjectiveManager
{
extern PblList<HUD::ObjectiveList>* gObjectiveLists; HUD::ObjectiveList* GetObjectiveList();
}


// FUNCTION: SWBF2 0x005c33e0
SwitchResponse ReadSwitchResponse(NetPktGroup& group, unsigned int turn, unsigned int)
{
    bool canSwitch;
    unsigned int responseTurn;
    group.Read(canSwitch);
    ReadTurnLowBits(group, responseTurn);
    SwitchResponse response = SR_NONE;
    if (canSwitch)
    {
        if (responseTurn == turn)
        {
            response = SR_CAN;
        }
    }
    else if (responseTurn != (unsigned int)-1 && responseTurn == turn)
    {
        response = SR_CANT;
    }
    return response;
}

// FUNCTION: SWBF2 0x005bb570
unsigned int GetUpdateTurn(NetPktGroup& group)
{
    group.BeginRead();
    BeginReadAlign(group);
    unsigned int turn;
    ReadTurnLowBits(group, turn);
    group.EndRead();
    return turn;
}

// TEMPLATE: SWBF2 0x005d3cc0
// ReadFixed<StickAxisConvert>
template void ReadFixed<StickAxisConvert>(StickAxisConvert&, NetPktGroup&, float&);

// TEMPLATE: SWBF2 0x005d3cf0
// ReadFixed<MouseAxisConvert>
template void ReadFixed<MouseAxisConvert>(MouseAxisConvert&, NetPktGroup&, float&);

// FUNCTION: SWBF2 0x005c25c0
EntityClass* ReadEntityClass(NetPktGroup& group)
{
    unsigned int index;
    group.ReadBits(index, 8);
    return index < 0xff ? Factory<Entity, EntityClass, EntityDesc>::FindNetIndex(index) : 0;
}

// FUNCTION: SWBF2 0x005c2640
WeaponClass* ReadWeaponClass(NetPktGroup& group)
{
    unsigned int index;
    group.ReadBits(index, 8);
    return index < 0xff ? Factory<Weapon, WeaponClass, WeaponDesc>::FindNetIndex(index) : 0;
}

Character* ReadCharacter(NetPktGroup& group);

// FUNCTION: SWBF2 0x005c05f0
void ReadEvent_Message(NetPktGroup& group, ReliableNetEvent& event)
{
    unsigned int messageType;
    group.ReadBits(messageType, 4);
    event.u.message.mType = (NetGame::MessageType)messageType;
    event.u.message.mCharacter1 = ReadCharacter(group);
    event.u.message.mCharacter2 = ReadCharacter(group);
    event.u.message.mEntClass = ReadEntityClass(group);
    event.u.message.mWpnClass = ReadWeaponClass(group);
    event.u.message.mCommandPost = ReadCommandPost(group);
    event.u.message.mTeam = ReadTeam(group);
}

// FUNCTION: SWBF2 0x005c0720
void ReadEvent_ChangeTeam(NetPktGroup& group, ReliableNetEvent& event)
{
    event.u.changeteam.mCommandPost = ReadCommandPost(group);
    event.u.changeteam.mTeam = ReadTeam(group);
    group.Read(event.u.changeteam.mAlive);
}

void ReadMatrix(NetPktGroup& group, PblMatrix& matrix);

// FUNCTION: SWBF2 0x005c10e0
void ReadEvent_CorrectChunkCluster(NetPktGroup& group, ReliableNetEvent& event, PblMatrix& matrix)
{
    ReliableNetEvent::U::CorrectChunkClusterEvent& data = event.u.correctchunkcluster;
    bool hasData = false;
    group.Read(hasData);
    if (hasData)
    {
        group.ReadBits(data.mChunkClusterIndex, 4);
        group.ReadBits(data.mChunkIndex, 4);
        data.mEntClass = ReadEntityClass(group);
        ReadMatrix(group, matrix);
    }
    else
    {
        data.mEntClass = 0;
    }
}

// FUNCTION: SWBF2 0x005cb120
void NetGame::Unlink(GameObject* pObject)
{
    ::Unlink(pObject);
}

// FUNCTION: SWBF2 0x005c7c40
void AtleastOnce::DropAll(int player)
{
    aloTrackable.Drop(player);
    aloMiniStats.Drop(player);
}

// FUNCTION: SWBF2 0x005c3370
void ReadRelaySwitchResponse(NetPktGroup& group, SwitchResponseRelayInfo& relay)
{
    bool canSwitch;
    unsigned int turn;
    group.Read(canSwitch);
    ReadTurnLowBits(group, turn);
    if (canSwitch)
    {
        relay.switchResponse = SR_CAN;
        relay.switchTurn = turn;
    }
    else if (turn == (unsigned int)-1)
    {
        relay.switchResponse = SR_NONE;
        relay.switchTurn = (unsigned int)-1;
    }
    else
    {
        relay.switchResponse = SR_CANT;
        relay.switchTurn = turn;
    }
}

// FUNCTION: SWBF2 0x005cac90
void ClampMove(ProcessedInputs* inputs)
{
    ClampFixed(stickAxisConvert, inputs->mFloatInputs[0]);
    ClampFixed(stickAxisConvert, inputs->mFloatInputs[1]);
    ClampFixed(mouseAxisConvert, inputs->mFloatInputs[2]);
    ClampFixed(mouseAxisConvert, inputs->mFloatInputs[3]);
}

// GLOBAL: SWBF2 0x01fa88a5
bool saveObjState;

// FUNCTION: SWBF2 0x005c34c0
void ReadSwitchObjRequest(NetPktGroup& group)
{
    unsigned int lastTurn, nextTurn;
    ReadTurnLowBits(group, lastTurn);
    ReadTurnLowBits(group, nextTurn);
    if (lastTurn != clientLastObjState->turn)
    {
        clientLastObjState->Free();
        clientLastObjState = clientNextObjState;
        clientNextObjState = NetObjStateMap::Alloc();
    }
    if (nextTurn == (unsigned int)clientHostTurnNumber)
    {
        saveObjState = true;
        clientSwitchObjects = SR_CAN;
        clientSwitchObjTurn = clientHostTurnNumber;
        clientNextObjState->turn = clientHostTurnNumber;
    }
    else
    {
        saveObjState = false;
        if (nextTurn == (unsigned int)-1)
        {
            clientSwitchObjects = SR_NONE;
        }
        else if (clientSwitchObjects != SR_CAN || nextTurn != (unsigned int)clientSwitchObjTurn)
        {
            clientSwitchObjects = SR_CANT;
            clientSwitchObjTurn = nextTurn;
        }
    }
}

void WriteString(NetPktGroup& group, const char* text, unsigned int capacity);

// FUNCTION: SWBF2 0x005d23a0
void WriteWaiting(NetPktGroup& group)
{
    group.WriteBits(netMinPlayersToAutoLaunchDedicated, 7);
    group.WriteBits(netCurMaxPlayers, 7);
    for (int player = 0; player < netCurMaxPlayers; ++player)
    {
        bool connected = NetComm::IsConnected(player);
        group.Write(connected);
        if (NetComm::IsConnected(player))
        {
            WriteString(group, NetGame::GetPlayerName(player), 32);
        }
    }
}

// FUNCTION: SWBF2 0x00654e10
bool AcceptLocalPlayerMessage(const Character* character, int value)
{
    if (value == -1 || !character->IsHero())
    {
        return true;
    }
    return value != 1 && value != 3;
}

// FUNCTION: SWBF2 0x005c3d30
void NetGame::ClientUpdateSpectator()
{
    for (int player = 0; player < netCurMaxPlayers; ++player)
    {
        if (!NetGame::IsPlaying(player, false) || NetGame::IsSpectator(player))
        {
            continue;
        }
        Character* character = Character::FindPlayer(player);
        if (character && character->GetPlayerId() != player)
        {
            Character::AddPlayer(player);
        }
    }
}

// FUNCTION: SWBF2 0x005c3bf0
bool NetGame::ChangeSpectateTrack(int player, int spectator)
{
    bool result = false;
    int previous = NetGame::GetSpectator(player);
    if (previous != -1 && spectator != -1 && previous != spectator)
    {
        Tracker* oldTracker = Tracker::GetTracker(previous);
        Tracker* newTracker = Tracker::GetTracker(spectator);
        Trackable* newTrackable = newTracker->GetTrackable();
        if (newTrackable != oldTracker->GetTrackable())
        {
            newTrackable->Track(player);
        }
    }
    return result;
}

// FUNCTION: SWBF2 0x005bb030
void ReadSpectatorInfo(NetPktGroup& group, int player)
{
    if (player >= 0)
    {
        unsigned int encoded = 0;
        group.ReadBits(encoded, 6);
        int spectator = (int)encoded - 1;
        NetGame::SetSpectator(player, spectator);
        NetGame::ChangeSpectateTrack(player, spectator);
        for (unsigned int slot = 0; slot < 1; ++slot)
        {
            int guest = NetGame::GetMyGuest(player, slot);
            if (guest >= 0)
            {
                group.ReadBits(encoded, 6);
                int spectator = (int)encoded - 1;
                NetGame::SetSpectator(guest, spectator);
                NetGame::ChangeSpectateTrack(guest, spectator);
            }
        }
        CheckReadAlign(group);
    }
}

// FUNCTION: SWBF2 0x005d2a80
void ReadPlayerMoves(NetPktGroup& group)
{
    int player;
    while (true)
    {
        group.ReadBits((unsigned int&)player, 6);
        if (player == curDst)
        {
            break;
        }
        ProcessedInputs* move = &remoteClientTurns.moves[player];
        ReadFixed<StickAxisConvert>(stickAxisConvert, group, move->mFloatInputs[0]);
        ReadFixed<StickAxisConvert>(stickAxisConvert, group, move->mFloatInputs[1]);
        ReadFixed<MouseAxisConvert>(mouseAxisConvert, group, move->mFloatInputs[2]);
        ReadFixed<MouseAxisConvert>(mouseAxisConvert, group, move->mFloatInputs[3]);
        unsigned int buttons;
        group.ReadBits(buttons, 4);
        remoteClientTurns.moves[player].bits.mBoolInputs = buttons;
        remoteClientTurns.moves[player].bits.mMouseMode = NetComm::GetArch(player) == NET_ARCH_PC ? 1 : 0;
    }
}

// FUNCTION: SWBF2 0x005c3c60
void NetGame::SetPlayerToSpectator(int player)
{
    if (NetGame::IsHost() && !NetGame::IsClient())
    {
        Character* character = Character::FindPlayer(player);
        int team = character->GetTeamNumber();
        NetGame::SetSpectator(player, NetGame::FindSpectateTarget(player, team));
    }
}

extern int gSpawnInvicibility;

// FUNCTION: SWBF2 0x005c2520
void HandleEvent_UpdateInvincibility(ReliableNetEvent& event)
{
    if (event.u.updateInvincibility.mCharacter)
    {
        if (event.u.updateInvincibility.mIsInvincible)
        {
            event.u.updateInvincibility.mCharacter->SetSpawnTime(NetComm::GetTime());
        }
        else
        {
            event.u.updateInvincibility.mCharacter->SetSpawnTime(
                NetComm::GetTime() - (float)gSpawnInvicibility - 0.1f);
        }
    }
}

// FUNCTION: SWBF2 0x005c2be0
void RecordTurn(unsigned int turn)
{
    int slot = turn & 31;
    NetTurn& turnData = hostTurns[slot];
    PblJournal::Record(hostPlayingMask, PJT_MOVES);
    for (unsigned int player = 0; player < 64; ++player)
    {
        if (hostPlayingMask & (1ui64 << player))
        {
            RecordMove(turnData.moves[player]);
        }
    }
}

// FUNCTION: SWBF2 0x005c0760
void HandleEvent_ChangeTeam(ReliableNetEvent& event)
{
    CommandPost* post = event.u.changeteam.mCommandPost;
    Team* team = event.u.changeteam.mTeam;
    GameObject* object = post->GetGameObject();
    if (!object)
    {
        return;
    }

    object->SetTeam(team->GetNumber());
    post->GetGameObject()->SetPerceivedTeam(team->GetNumber());
    if (event.u.changeteam.mAlive)
    {
        if (!post->GetGameObject()->IsAlive())
        {
            post->GetGameObject()->Respawn();
        }
    }
    else if (post->GetGameObject()->IsAlive())
    {
        post->GetGameObject()->Kill();
    }
}

// FUNCTION: SWBF2 0x005c09c0
void HandleEvent_ChangeCommandPostTeams(ReliableNetEvent& event)
{
    ReliableNetEvent::U::ChangeCommandPostTeamsEvent& data = event.u.changecommandpostteams;
    for (int i = 0; i < 16; ++i)
    {
        CommandPost* post = CommandPost::FindPostByIndex(i);
        if (!post || !post->GetGameObject() || post->GetGameObject()->GetEntityType() != ENT_POST)
        {
            continue;
        }

        int team = (data.mTeamBits >> (2 * i)) & 3;
        post->GetGameObject()->SetTeam(team);
        post->GetGameObject()->SetPerceivedTeam(team);
        if ((data.mAliveBits >> i) & 1)
        {
            if (!post->GetGameObject()->IsAlive())
            {
                post->GetGameObject()->Respawn();
            }
        }
        else if (post->GetGameObject()->IsAlive())
        {
            post->GetGameObject()->Kill();
        }
    }
}

// FUNCTION: SWBF2 0x005cae30
bool ReadMove(NetPktGroup& group, ProcessedInputs* previous, ProcessedInputs* current)
{
    bool same;
    group.Read(same);
    if (same)
    {
        *current = *previous;
    }
    else
    {
        bool end;
        group.Read(end);
        if (end)
        {
            return false;
        }
        ReadFixed<StickAxisConvert>(stickAxisConvert, group, current->mFloatInputs[0]);
        ReadFixed<StickAxisConvert>(stickAxisConvert, group, current->mFloatInputs[1]);
        ReadFixed<MouseAxisConvert>(mouseAxisConvert, group, current->mFloatInputs[2]);
        ReadFixed<MouseAxisConvert>(mouseAxisConvert, group, current->mFloatInputs[3]);
        unsigned int bits;
        group.ReadBits(bits, 30);
        current->bits.mBoolInputs = bits;
        group.ReadBits(bits, 2);
        current->bits.mMouseMode = bits;
    }
    return true;
}

// FUNCTION: SWBF2 0x005c0d00
void ReadEvent_CreateChunkCluster(NetPktGroup& group, ReliableNetEvent& event, PblMatrix&)
{
    ReliableNetEvent::U::CreateChunkClusterEvent& data = event.u.createchunkcluster;
    bool hasData = false;
    group.Read(hasData);
    if (hasData)
    {
        group.ReadBits(data.mChunkClusterIndex, 4);
        data.mEntClass = static_cast<EntityGeometryClass*>(ReadEntityClass(group));
        if (data.mEntClass)
        {
            for (int i = 0; i < 10; ++i)
            {
                if (data.mEntClass->mChunkDescriptors[i])
                {
                    ReadMatrix(group, relChunkClusterData[data.mChunkClusterIndex][i].mMatrix);
                    ReadPositionVector(group, relChunkClusterData[data.mChunkClusterIndex][i].mVelocity);
                    ReadPositionVector(group, relChunkClusterData[data.mChunkClusterIndex][i].mOmega);
                }
            }
        }
    }
    else
    {
        data.mEntClass = 0;
    }
}

// FUNCTION: SWBF2 0x005c99c0
unsigned int ReadMoves(NetPktGroup& group, int player, unsigned int turn)
{
    ProcessedInputs* previous = &noInputs;
    while (true)
    {
        int slot = turn & 31;
        ProcessedInputs* current = &hostTurns[slot].moves[player];
        if (!ReadMove(group, previous, current))
        {
            break;
        }
        if (turn >= (unsigned int)hostTurnNumber)
        {
            hostRecv[slot] |= 1ui64 << player;
        }
        previous = current;
        ++turn;
    }
    return turn - 1;
}

extern unsigned int netBroadcastSendSocket;
extern unsigned int netBroadcastRecvSocket;
extern "C" int __stdcall closesocket(unsigned int s);

// FUNCTION: SWBF2 0x005c7160
void AloNextGameInfo::Read(NetPktGroup& group)
{
    bool hasInfo;
    group.Read(hasInfo);
    if (!hasInfo)
    {
        return;
    }

    gotGameOver = true;
    char luaFile[16];
    ReadString(group, luaFile, sizeof(luaFile));
    unsigned int team1Hash, team2Hash, winningTeams = 0;
    group.ReadBits(team1Hash, 32);
    group.ReadBits(team2Hash, 32);
    group.ReadBits(winningTeams, 32);
    if (!NetComm::IsHost())
    {
        Team::SetWinningTeams(winningTeams);
        MissionPlayList::SetClientEntry(luaFile, team1Hash, team2Hash);
    }
}

// GLOBAL: SWBF2 0x007e8e7c
int uploadSrc = -1;
// GLOBAL: SWBF2 0x01e62f9c
unsigned int uploadSeqNo;
// GLOBAL: SWBF2 0x01e64b5c
int uploadState;
// GLOBAL: SWBF2 0x01e64ef8
int uploadSizeLeft;

// FUNCTION: SWBF2 0x005c4da0
void ReadUpload(NetPktGroup& group)
{
    bool hasData;
    char fileName[32];
    char buffer[1012];
    int src;
    bool moreFiles;
    unsigned int fileSize;
    unsigned int clientSeqNo;
    bool isHeader;

    group.Read(hasData);
    if (!hasData)
    {
        return;
    }
    group.Read(isHeader);
    src = GetSrc(group.pktPtr);
    if (src != uploadSrc && uploadSrc >= 0)
    {
        group.ReadBits(clientSeqNo, 32);
        group.Read(moreFiles);
        if (moreFiles)
        {
            ReadString(group, fileName, sizeof(fileName));
            group.ReadBits(fileSize, 32);
        }
    
        return;
    }
    group.ReadBits(clientSeqNo, 32);
    if (clientSeqNo != uploadSeqNo + 1)
    {
        if (isHeader)
        {
            group.Read(moreFiles);
            if (moreFiles)
            {
                ReadString(group, fileName, sizeof(fileName));
                group.ReadBits(fileSize, 32);
            }
        }
        else
        {
            group.ReadBits(fileSize, 10);
            group.ReadBytes(buffer, fileSize);
        }
    }
    else
    {
        if (uploadSrc != src)
        {
            uploadSrc = src;
            CreateUploadDirectory(NetGame::GetPlayerName(src));
        }
        uploadSeqNo = clientSeqNo;
        moreFiles = true;
        switch (uploadState)
        {
        case 0:
            group.Read(moreFiles);
            if (!moreFiles)
            {
                uploadSrc = -1;
                uploadSeqNo = 0;
                NetGameP::SetNotPlaying(src);
            }
            else
            {
                ReadString(group, fileName, sizeof(fileName));
                group.ReadBits(fileSize, 32);
                OpenUploadFile(fileName);
                uploadState = 1;
                uploadSizeLeft = fileSize;
            }
            break;
        case 1:
            group.ReadBits(fileSize, 10);
            group.ReadBytes(buffer, fileSize);
            uploadSizeLeft -= fileSize;
            WriteUploadFile(buffer, fileSize);
            if (!uploadSizeLeft)
            {
                CloseUploadFile();
                uploadState = 0;
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x005cc440
void ReadSpawnDisplay(NetPktGroup& group)
{
    unsigned int numLiveHumans1;
    unsigned int numLiveHumans2;
    group.ReadBits(numLiveHumans1, 6);
    group.ReadBits(numLiveHumans2, 6);
    SpawnDisplay::sNumLiveHumansTeam1 = numLiveHumans1;
    SpawnDisplay::sNumLiveHumansTeam2 = numLiveHumans2;
    bool hasData;
    group.Read(hasData);
    if (hasData)
    {
        SpawnDisplay::Instance(0)->Read(group);
        Character* player = Character::FindPlayer(NetGame::GetLocalIndex(0));
        bool hasDelay;
        group.Read(hasDelay);
        unsigned int bits;
        group.ReadBits(bits, 8);
        if (player)
        {
            if (hasDelay)
            {
                player->mDelayTimer = 60.0f;
            }
            else
            {
                player->mDelayTimer = -60.0f;
            }
            netPlayers[NetGame::GetLocalIndex(0)].mSpawnOrder = bits - 1;
        }
    }
    for (unsigned int guestIdx = 0; guestIdx < 1; ++guestIdx)
    {
        group.Read(hasData);
        if (hasData)
        {
            SpawnDisplay* instance = SpawnDisplay::Instance(guestIdx + 1);
            instance->Read(group);
            Character* player = Character::FindPlayer(NetGame::GetLocalIndex(guestIdx + 1));
            bool hasDelay;
            group.Read(hasDelay);
            unsigned int bits;
            group.ReadBits(bits, 8);
            if (player)
            {
                if (hasDelay)
                {
                    player->mDelayTimer = 60.0f;
                }
                else
                {
                    player->mDelayTimer = -60.0f;
                }
                netPlayers[NetGame::GetLocalIndex(guestIdx + 1)].mSpawnOrder = bits - 1;
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c1d90
void ReadEvent_DestroyTowCable(NetPktGroup& group, ReliableNetEvent& event)
{
    ReliableNetEvent::U::DestroyTowCableEvent& data = event.u.destroyTowCable;
    data.mOwnerObject.Set(0);
    unsigned int objectId;
    group.ReadBits(objectId, GetUniqObjectBits());
    NetObj* netObject = clientObjMap.Find(objectId);
    const GameObject* owner = 0;
    if (netObject)
    {
        data.mOwnerObject.Set(netObject->obj);
        owner = netObject->obj;
    }
    data.mFireWeapon = 0;
    unsigned int hasWeapon;
    group.ReadBits(hasWeapon, 1);
    if (hasWeapon)
    {
        unsigned int controlIndex, weaponIndex;
        group.ReadBits(controlIndex, 3);
        group.ReadBits(weaponIndex, 2);
        if (owner)
        {
            PblList<Controllable>::Itor controlItor(owner->GetControllableList());
            for (unsigned int i = 0; i < controlIndex; ++i)
            {
                ++controlItor;
            }
            if (controlItor)
            {
                data.mFireWeapon = controlItor->GetWeaponPtr(weaponIndex);
            }
        }
    }
}

void ReadUnitVector(NetPktGroup& group, PblVector3& value);

// FUNCTION: SWBF2 0x005c1600
void ReadEvent_ConnectTowCable(NetPktGroup& group, ReliableNetEvent& event)
{
    PblVector3 position, direction;
    ReliableNetEvent::U::ConnectTowCableEvent& data = event.u.connectTowCable;
    data.mCollideWalker = 0;
    data.mOwnerObject.Set(0);
    ReadUnitVector(group, direction);
    data.mDirection.Set(direction);

    unsigned int objectId, collideId;
    group.ReadBits(objectId, GetUniqObjectBits());
    NetObj* netObject = clientObjMap.Find(objectId);
    if (netObject)
    {
        data.mOwnerObject.Set(netObject->obj);
    }
    group.ReadBits(collideId, GetUniqObjectBits());
    netObject = clientObjMap.Find(collideId);
    if (netObject)
    {
        data.mCollideWalker = netObject->obj;
    }
    const GameObject* owner = 0;
    data.mOwnerObject.Get(owner);
    data.mFireWeapon = 0;
    unsigned int hasWeapon;
    group.ReadBits(hasWeapon, 1);
    if (hasWeapon)
    {
        unsigned int controlIndex, weaponIndex;
        group.ReadBits(controlIndex, 3);
        group.ReadBits(weaponIndex, 2);
        if (owner)
        {
            PblList<Controllable>::Itor controlItor(owner->GetControllableList());
            for (unsigned int i = 0; i < controlIndex; ++i)
            {
                ++controlItor;
            }
            if (controlItor)
            {
                data.mFireWeapon = controlItor->GetWeaponPtr(weaponIndex);
            }
        }
    }
    unsigned int bodyId = 0;
    group.ReadBits(bodyId, 7);
    data.mCollideBodyID = bodyId;
    ReadPositionVector(group, position);
    data.mCollidePosition.Set(position);
}

void ReadUpdate(NetPktGroup& group);

// FUNCTION: SWBF2 0x005bb530
void ReceiveUpdate(NetPktGroup& group)
{
    group.BeginRead();
    BeginReadAlign(group);
    isFullState = false;
    ReadUpdate(group);
    group.EndRead();
    group.Free();
}

Character* ReadCharacter(NetPktGroup& group);

// FUNCTION: SWBF2 0x005bef70
void ReadNetEvent(NetPktGroup& group)
{
    unsigned int type;
    group.ReadBits(type, 3);
    switch (type)
    {
    case 0:
    {
        OrdnanceDesc desc;
        ReadPositionVector(group, desc.mPosition);
        ReadUnitVector(group, desc.mDirection);
        group.ReadFixed(desc.mStrength, 0.0f, 1.0f, 10);
        bool hasOwner = false;
        group.Read(hasOwner);
        if (hasOwner)
        {
            desc.mOwner = ReadObjectPtr(group);
        }
        else
        {
            desc.mOwner = 0;
        }
        Character* character = ReadCharacter(group);
        bool hasDamageObject;
        group.Read(hasDamageObject);
        desc.mDamageOwner.mGameObject = hasDamageObject ? ReadObjectPtr(group) : 0;
        int team;
        if (character)
        {
            team = character->GetTeamNumber();
        }
        else if (desc.mDamageOwner.mGameObject)
        {
            team = desc.mDamageOwner.mGameObject->GetTeam();
        }
        else
        {
            team = 0;
        }
        bool hasMultiplier;
        group.Read(hasMultiplier);
        if (hasMultiplier)
        {
            group.ReadFixed(desc.mDamageOwner.mDamageMultiplier, 0.0f, 10.0f, 16);
        }
        else
        {
            desc.mDamageOwner.mDamageMultiplier = 1.0f;
        }
        unsigned int ordnanceIndex;
        group.ReadBits(ordnanceIndex, 8);
        OrdnanceClass* ordnanceClass = Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::FindNetIndex(ordnanceIndex);
        unsigned int weaponIndex;
        group.ReadBits(weaponIndex, 8);
        WeaponClass* weaponClass = weaponIndex < 0xff ? Factory<Weapon, WeaponClass, WeaponDesc>::FindNetIndex(weaponIndex) : 0;
        EntityClass* entityClass = ReadEntityClass(group);
        desc.mDamageOwner.mCharacter = character;
        desc.mDamageOwner.mTeam = team;
        desc.mDamageOwner.mClass = entityClass;
        desc.mDamageOwner.mWeapon = weaponClass;

        unsigned int targetIndex;
        group.ReadBits(targetIndex, GetUniqObjectBits());
        NetObj* target = targetIndex != 63 ? clientObjMap.Find(targetIndex) : 0;
        if (target)
        {
            desc.mTarget = target->obj;
        }
        else
        {
            desc.mTarget = 0;
        }

        desc.mFireWeapon = 0;
        unsigned int hasWeapon;
        group.ReadBits(hasWeapon, 1);
        if (hasWeapon)
        {
            unsigned int controllableIndex, fireWeaponIndex;
            group.ReadBits(controllableIndex, 3);
            group.ReadBits(fireWeaponIndex, 2);
            if (desc.mOwner)
            {
                PblList<Controllable>::Itor it(desc.mOwner->GetControllableList());
                for (unsigned int i = 0; i < controllableIndex; ++i)
                {
                    ++it;
                }
                if ((Controllable*)it)
                {
                    desc.mFireWeapon = it->GetWeaponPtr(fireWeaponIndex);
                }
            }
        }

        if (weaponClass &&
            (weaponClass->IsRtti(WeaponTowCableClass::GetRtti()) ||
             weaponClass->IsRtti(WeaponMeleeThrowClass::GetRtti())))
        {
            if (!desc.mFireWeapon)
            {
                ordnanceClass = 0;
            }
            else if (weaponClass->IsRtti(WeaponTowCableClass::GetRtti()))
            {
                WeaponTowCable* towCableWeap = static_cast<WeaponTowCable*>(desc.mFireWeapon);
                if (towCableWeap->HasOrdnance())
                {
                    ordnanceClass = 0;
                }
            }
        }
        if (ordnanceClass)
        {
            ordnanceClass->Build(desc);
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x005bf380
void ReceiveNetEvents(NetPktGroup& group)
{
    while (true)
    {
        bool hasEvent;
        group.Read(hasEvent);
        if (!hasEvent)
        {
            return;
        }
        ReadNetEvent(group);
    }
}

// FUNCTION: SWBF2 0x005cb630
void ReadSwitchResponses(NetPktGroup& group, NetPlayer& player)
{
    SwitchResponse response = ReadSwitchResponse(group, player.nextObjState->turn,
                                                player.lastObjState->turn);
    switch (response)
    {
    case SR_CAN:
        player.lastObjState->Free();
        player.lastObjState = player.nextObjState;
        player.nextObjState = NetObjStateMap::Alloc();
        break;
    case SR_CANT:
        player.nextObjState->Free();
        player.nextObjState = NetObjStateMap::Alloc();
        break;
    }
}

// FUNCTION: SWBF2 0x005cb6b0
SwitchResponse SkipSwitchResponses(NetPktGroup& group)
{
    return ReadSwitchResponse(group, (unsigned int)-1, (unsigned int)-1);
}

extern unsigned __int64 netCreatedLobbyId;
extern unsigned __int64 netEnteredLobbyId;
extern int netLanRegisterState;
extern bool netLeavingLobby;
void EndRegisterLanGame();
void* GetGalaxyMatchmaking();
namespace gf
{
bool isInitialized(); namespace utils
{
const char* ToString(bool);
}
}

// GLOBAL: SWBF2 0x01e62e88
int netDirectPort;

// FUNCTION: SWBF2 0x005d1570
void DisplayMessage(NG_ChatMsg& message)
{
    const unsigned int requestKeys[TeamAssistManager::Max] = {
        0x0e6cfcde, 0x0f6cfe71, 0x0c6cf9b8, 0x0d6cfb4b,
        0x126d032a, 0x136d04bd, 0x106d0004, 0x116d0197
    };
    Character* sender = 0;
    RedColor color(RedColor::YELLOW);
    unsigned short separator[10];
    PblUnicode::usnprintf(separator, 10, "%s", ": ");
    unsigned short text[128];
    PblUnicode::usnprintf(text, 128, "%s", "");
    if (message.player_id == 64)
    {
        message.flag = 0;
        const unsigned short* admin = RedLocalizeDB::Find(0x10e2f4e1, true);
        if (admin)
        {
            PblUnicode::ustrlcat(text, admin, 128);
        }
        else
        {
            unsigned short* end = text + PblUnicode::ustrlen(text);
            PblUnicode::ustrncnv(end, "Admin", 128 - (end - text));
        }
    }
    else
    {
        sender = Character::FindPlayer(message.player_id);
        if (sender)
        {
            PblUnicode::ustrlcat(text, sender->GetName(), 128);
        }
    }
    PblUnicode::ustrlcat(text, separator, 128);
    if (message.code == TeamAssistManager::None)
    {
        PblUnicode::ustrlcat(text, message.msg, 128);
    }
    else
    {
        PblUnicode::ustrlcat(text, RedLocalizeDB::Find(requestKeys[message.code], false), 128);
    }

    const unsigned int teamOnly = 1;
    Character* local = Character::FindPlayer(NetGame::GetLocalIndex(0));
    if (message.flag == teamOnly &&
        local->GetTeamNumber() != (sender ? sender->GetTeamNumber() : 0))
    {
        return;
    }
    if (!AcceptLocalPlayerMessage(local, message.code))
    {
        return;
    }
    MessageDisplay::Instance(0, MessageDisplay::DISPLAY_SYSTEM_MSG)->AddMessage(text, color);
    if (message.code > TeamAssistManager::None)
    {
        TeamAssistManager::AddRequest((TeamAssistManager::Code)message.code, sender);
    }
}
void LogChatMessage(const NG_ChatMsg& message);

// FUNCTION: SWBF2 0x005d1900
void NetGame::ReadChatMessageFromServer(NetPktGroup& group)
{
    unsigned int length = 0;
    NG_ChatMsg message;
    group.ReadBytes((char*)&length, sizeof(length));
    if ((length & 0xffff0000u) == 0xffff0000u && (length &= 0xffffu) > 0)
    {
        group.ReadBytes((char*)&message, length);
        if (!NetGame::IsHost())
        {
            LogChatMessage(message);
        }
        DisplayMessage(message);
    }
}


// FUNCTION: SWBF2 0x005b7ee0
void NetGame::AddPlayer(int iPlayer)
{
    Character* character = 0;
    int spectator = IsSpectator(iPlayer);
    int flag = NetGame::IsSpawning(iPlayer);
    bool makeCharacter = true;
    if (spectator && !flag)
    {
        makeCharacter = false;
    }
    if (makeCharacter)
    {
        Character::AddPlayer(iPlayer);
        character = Character::FindPlayer(iPlayer);
    }
    NetSetup::AddGamePlayer(iPlayer);
    const int joystick = GetJoystickIndex(iPlayer);
    if (joystick >= 0)
    {
        GameVoiceChat::Node* node = GameVoiceChat::FindNodeByDeviceID(joystick);
        if (node)
        {
            ((GameVoiceChat::Node*)node)->SetID((unsigned char)iPlayer);
        }
    }
    if (IsHost() && !IsClient())
    {
        Character* player = Character::FindPlayer(iPlayer);
        if (player)
        {
            MedalsMgr* medals = player->GetMedalsManager();
            if (medals)
            {
                medals->Init();
            }
        }
        unsigned char team1 = 0, team2 = 0;
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (i == iPlayer)
            {
                continue;
            }
            if (IsPlaying(i, false))
            {
                if (GetStartTeam(i) == 1)
                {
                    ++team1;
                }
                else
                {
                    ++team2;
                }
            }
        }
        if (team1 < team2)
        {
            NetGame::SetStartTeam(iPlayer, 1);
        }
        else
        {
            NetGame::SetStartTeam(iPlayer, 2);
        }
        netPlayers[iPlayer].lastObjState = NetObjStateMap::Alloc();
        netPlayers[iPlayer].nextObjState = NetObjStateMap::Alloc();
        netPlayers[iPlayer].reliableEventTail = reliableNetEventTail;
        if (makeCharacter)
        {
            CreateEvent_Message((MessageType)8, character, 0, 0, 0, 0, 0);
            NetGame::CreateEvent_ChangeCommandPostTeams(1ui64 << iPlayer);
            EntityDoor::BeginIter();
            EntityDoor* door = EntityDoor::GetCurrentIter();
            while (door)
            {
                NetGame::CreateEvent_UpdateDoorState(door->GetId(), 1ui64 << iPlayer);
                EntityDoor::IncrementIter();
                door = EntityDoor::GetCurrentIter();
            }
            HUD::ObjectiveList* objectives = ObjectiveManager::GetObjectiveList();
            objectives->BeginIter();
            HUD::Objective* objective = objectives->GetCurrentIter();
            while (objective)
            {
                NetGame::CreateEvent_UpdateObjective(objective, 1ui64 << iPlayer);
                objectives->IncrementIter();
                objective = objectives->GetCurrentIter();
            }
        }
        netPlayers[iPlayer].mLastReceivedTurn = hostTurnNumber;
        NetAdmin::AddPlayer(iPlayer);
    }
}

// FUNCTION: SWBF2 0x005b8190
void NetGame::RemovePlayer(int iPlayer)
{
    Tracker::SetTrackable(0, iPlayer);
    for (int i = 0; i < 3; ++i)
    {
        if (iPlayer == netHeroNextPlayerID[i])
        {
            netHeroNextPlayerID[i] = -1;
        }
        if (iPlayer == netHeroPlayerID[i])
        {
            netHeroPlayerID[i] = -1;
            netHeroNextPlayerID[i] = -1;
            netHeroAIID[i] = -1;
            netHeroDiedTurn[i] = NetGame::GetTurnNumber();
            spHeroDiedTime[i] = GetLocalTime();
            netHeroSpawnedTurn[i] = 0;
        }
    }
    Character* character = Character::FindPlayer(iPlayer);
    Character::RemovePlayer(iPlayer);
    NetSetup::RemoveGamePlayer(iPlayer);
    VoiceChat::Node* node = VoiceChat::Node::FindByID((unsigned char)iPlayer);
    if (node)
    {
        ((GameVoiceChat::Node*)node)->SetID(0xff);
    }
    NetGame::ClearPlayerCDKey(iPlayer);
    if (IsHost() && !IsClient())
    {
        NetAdmin::RemovePlayer(iPlayer);
        const bool mission = !InShell();
        netPlayers[iPlayer].Clean();
        netPlayers[iPlayer].Init(mission);
        Tracker* tracker = Tracker::GetTracker(iPlayer);
        if (tracker)
        {
            tracker->SetSpectating(IsSpectator(iPlayer));
        }
        AtleastOnce::DropAll(iPlayer);
        CleanReliableNetEvents(iPlayer);
        CreateEvent_Message((MessageType)9, character, 0, 0, 0, 0, 0);
        NetComm::Disconnect(iPlayer);
        Character* player = Character::FindPlayer(iPlayer);
        if (player)
        {
            MedalsMgr* medals = player->GetMedalsManager();
            if (medals)
            {
                medals->Init();
            }
        }
    }
    if (!IsHost())
    {
        const bool mission = !InShell();
        netPlayers[iPlayer].Clean();
        netPlayers[iPlayer].Init(mission);
        Tracker* tracker = Tracker::GetTracker(iPlayer);
        if (tracker)
        {
            tracker->SetSpectating(IsSpectator(iPlayer));
        }
        NetComm::Disconnect(iPlayer);
    }
    if (IsClient())
    {
        FLInputManager::Instance()->StopAllRumble();
        if (clientBootPlayer == iPlayer)
        {
            clientBootPlayer = -1;
        }
    }
}

// GLOBAL: SWBF2 0x01e64b85
AloBootVote aloBootVote;

// FUNCTION: SWBF2 0x005c7ad0
void AtleastOnce::ReadAll(NetPktGroup& group)
{
    while (true)
    {
        unsigned int value;
        group.ReadBits(value, 4);
        if (!value)
        {
            return;
        }
        switch (value)
        {
        case 1:
            aloTrackable.Read(group);
            break;
        case 2:
            aloReinforcements.Read(group);
            break;
        case 3:
        {
            group.ReadBits(value, 2);
            if (value == 0)
            {
                aloHeroData0.Read(group);
            }
            if (value == 1)
            {
                aloHeroData1.Read(group);
            }
            if (value == 2)
            {
                aloHeroData2.Read(group);
            }
            break;
        }
        case 4:
            aloMapDisplayMarkers.Read(group);
            break;
        case 5:
        {
            bool hasStats;
            group.Read(hasStats);
            if (hasStats)
            {
                aloStats.gotGameOver = true;
                PlayerStats::ReadAll(group);
            }
            break;
        }
        case 6:
            aloMiniStats.Read(group);
            break;
        case 7:
            aloBootVote.Read(group);
            break;
        case 8:
            aloNextGameInfo.Read(group);
            break;
        case 9:
            aloMissionTimer.Read(group);
            break;
        }
    }
}

// GLOBAL: SWBF2 0x01faa6cc
bool relayReadsOwnPacket;

// FUNCTION: SWBF2 0x005ce220
void ClearRelay()
{
    NetEntityList::Itor it(*netEntities);
    while (it)
    {
        NetEntity& entity = *it;
        ++it;
        switch (entity.type)
        {
        case ENT_DROIDEKA:
            ++gDebugCount;
        case ENT_SOLDIER:
        case ENT_FLYER:
        case ENT_WALKER:
        case ENT_TAUNTAUN:
        case ENT_DROID:
        case ENT_HOVER:
        case ENT_MINE:
        case ENT_POWERUP:
        case ENT_ASTEROID:
        case ENT_FLAG:
            delete entity.obj;
            it.Advance();
            break;
        default:
            break;
        }
    }
}

// FUNCTION: SWBF2 0x005c92e0
void ReceiveRelay()
{
    int i;
    if (netHosts[netHostIndex].hostMode != HM_RELAY)
    {
        return;
    }
    unsigned int otherHosts = 0;
    for (i = 0; i < netCurMaxHosts; ++i)
    {
        if (i != netHostIndex && NetComm::IsConnected(i + 64) && netHosts[i].hostMode == HM_RELAY)
        {
            otherHosts |= 1u << i;
        }
    }
    while (true)
    {
        NetPacket* packet = NetComm::Recv(PKT_GAME_RELAY_UPDATE, PKT_NONE);
        if (!packet)
        {
            return;
        }
        if (GetSrc(packet) == 64 && otherHosts)
        {
            for (i = 0; i < netCurMaxHosts; ++i)
            {
                if (otherHosts & (1u << i))
                {
                    NetComm::Send(i + 64, PKT_GAME_RELAY_UPDATE, packet);
                }
            }
        }
        if (!relayReadsOwnPacket)
        {
            NetComm::Send(netHostIndex + 64, PKT_GAME_RELAY_UPDATE_LOCAL, packet);
        }
        NetPktGroup group;
        if (relayReadsOwnPacket)
        {
            group.Set(packet);
        }
        if (relayReadsOwnPacket || NetComm::Recv(group, PKT_GAME_RELAY_UPDATE_LOCAL))
        {
            ClearRelay();
            group.BeginRead();
            BeginReadAlign(group);
            isFullState = true;
            curDst = -1;
            saveObjState = false;
            ReadUpdate(group);
            for (i = 0; i < netCurMaxPlayers; ++i)
            {
                if (hostPlayingMask & (1ui64 << i))
                {
                    curDst = i;
                    ReadUpdate(group);
                }
            }
            CheckReadAlign(group);
            group.EndRead();
            group.Free();
        }
    }
}

unsigned int ReadMoves(NetPktGroup& group, int player, unsigned int turn);

// FUNCTION: SWBF2 0x005c96f0
void ReceiveReplica()
{
    int j;
    if (netHosts[netHostIndex].hostMode != HM_REPLICA)
    {
        return;
    }
    while (true)
    {
        unsigned int lastTurn;
        unsigned int updateTurn;
        NetPktGroup pktGroup;
        if (!NetComm::Recv(pktGroup, PKT_GAME_REPLICA_UPDATE))
        {
            return;
        }
        pktGroup.BeginRead();
        ReadDestMask(pktGroup, hostPlayingMask);
        UpdateCurMaxPlayers(hostPlayingMask, true);
        ReadTurnLowBits(pktGroup, updateTurn);
        for (j = 0; j < netCurMaxPlayers; ++j)
        {
            NetPlayer& netPlayer = netPlayers[j];
            if (netPlayer.clientOfIndex != netHostIndex)
            {
                continue;
            }
            ReadTurnLowBits(pktGroup, netPlayer.clientTurn);
            unsigned int lagBits;
            pktGroup.ReadBits(lagBits, 8);
            int lag = (int)lagBits - 128;
            netPlayer.lagSamples[0] = lag;
            ReadSwitchResponses(pktGroup, netPlayer);
        }
        for (j = 0; j < netCurMaxPlayers; ++j)
        {
            if (hostPlayingMask & (1ui64 << j))
            {
                lastTurn = ReadMoves(pktGroup, j, updateTurn);
            }
        }
        if (IsNetObjectFeatureEnabled())
        {
            unsigned int highTurn = updateTurn;
            while (true)
            {
                int idx = highTurn & 31;
                if ((hostRecv[idx] & hostPlayingMask) != hostPlayingMask)
                {
                    break;
                }
                ++highTurn;
                if (highTurn > updateTurn + 128)
                {
                    break;
                }
            }
        }
        pktGroup.EndRead();
        pktGroup.Free();
    }
}

// GLOBAL: SWBF2 0x01faa71c
int debugReliableEventOrdinal;

// GLOBAL: SWBF2 0x01faa6c8
unsigned int debugReliableEventBatch;
void HandleEvent_ConnectTowCable(ReliableNetEvent& event);

// FUNCTION: SWBF2 0x005bf9b0
void ReceiveReliableNetEvents(NetPktGroup& group)
{
    bool hasBatch;
    group.Read(hasBatch);
    if (hasBatch)
    {
        unsigned int batch;
        group.ReadBits(batch, 6);
        batch = FillHighBits(batch, clientReliableBatchNumber, 6);
        bool apply = false;
        if (batch > (unsigned int)clientReliableBatchNumber)
        {
            apply = true;
            clientReliableBatchNumber = batch;
        }
        int count = 0;
        while (true)
        {
            bool hasEvent;
            group.Read(hasEvent);
            if (!hasEvent)
            {
                break;
            }
            ++count;
            if (count == debugReliableEventOrdinal && batch == debugReliableEventBatch)
            {
                ++gDebugCount;
            }
            PblMatrix matrix;
            ReliableNetEvent event;
            unsigned int type;
            group.ReadBits(type, 4);
            event.mEvType = (ReliableEventType)type;
            switch (event.mEvType)
            {
            case 0: ReadEvent_Message(group, event); break;
            case 1: ReadEvent_ChangeTeam(group, event); break;
            case 2: ReadEvent_ChangeCommandPostTeams(group, event); break;
            case 3: ReadEvent_CreateChunkCluster(group, event, matrix); break;
            case 4: ReadEvent_CorrectChunkCluster(group, event, matrix); break;
            case 5: ReadEvent_ConnectTowCable(group, event); break;
            case 6: ReadEvent_DestroyTowCable(group, event); break;
            case 7: ReadEvent_GameOver(group, event); break;
            case 8: ReadEvent_ScriptMessage(group, event); break;
            case 9: ReadEvent_PlayVoiceOver(group, event); break;
            case 10: ReadEvent_UpdateDoorState(group, event); break;
            case 11: ReadEvent_AddObjective(group, event); break;
            case 12: ReadEvent_UpdateObjective(group, event); break;
            case 13: ReadEvent_UpdateInvincibility(group, event); break;
            }
            if (apply)
            {
                switch (event.mEvType)
                {
                case 0: HandleEvent_Message(event); break;
                case 1: HandleEvent_ChangeTeam(event); break;
                case 2: HandleEvent_ChangeCommandPostTeams(event); break;
                case 3: HandleEvent_CreateChunkCluster(event, matrix); break;
                case 4: HandleEvent_CorrectChunkCluster(event, matrix); break;
                case 5: HandleEvent_ConnectTowCable(event); break;
                case 6: HandleEvent_DestroyTowCable(event); break;
                case 7: HandleEvent_GameOver(event); break;
                case 8: HandleEvent_ScriptMessage(event); break;
                case 9: HandleEvent_PlayVoiceOver(event); break;
                case 10: HandleEvent_UpdateDoorState(event); break;
                case 11: HandleEvent_AddObjective(event); break;
                case 12: HandleEvent_UpdateObjective(event); break;
                case 13: HandleEvent_UpdateInvincibility(event); break;
                }
            }
        }
    }
}

inline void* NetObj::operator new(size_t uSize)
{
    return sMemoryPool.Allocate(uSize);
}

// GLOBAL: SWBF2 0x007ec4cc
int debugCreateInitialPlayer = 1;

// GLOBAL: SWBF2 0x007ec4d0
int debugCreateHandleId = 90;

// GLOBAL: SWBF2 0x007ec4e4
int debugReceivePlayer = -1;

// GLOBAL: SWBF2 0x007ec4ec
int debugReadHostTurn = 655;

// GLOBAL: SWBF2 0x007ec4fc
int debugReadCreateOrdinal = 15;

// GLOBAL: SWBF2 0x007ec510
int debugReadObjectOrdinal = 15;
// GLOBAL: SWBF2 0x007e8e88
int netTraceIndex = -1;

// GLOBAL: SWBF2 0x01e63014
unsigned int netTraceObjectHandleId;
// GLOBAL: SWBF2 0x01fa8718
NetObj** createPoolPtr;
// GLOBAL: SWBF2 0x01fa8854
unsigned int createPoolCount;
// GLOBAL: SWBF2 0x01fa3d80
GameObject* readObj;

// FUNCTION: SWBF2 0x005cb410
void Remove(NetObj* netObj, bool flag)
{
    destroyOk = true;
    GameObject* object = netObj->obj;
    TrackDelete(object);
    if (object->GetHandleId() == 110)
    {
        ++gDebugCount;
    }
    Unlink(object);
    if (netObj->uniqId != 63)
    {
        clientObjMap.Remove(netObj);
    }
    if (clientHostTurnNumber == 233)
    {
        ++gDebugCount;
    }
    PblValues::Record(object->GetHandleId(), PVT_FULL);
    PblValues::Record(object->GetCollisionObject()->mLastPosition.GetX(),
                      PVT_MOVES_HIGH);
    if (flag)
    {
        object->ClientKill();
    }
    delete object;
    NetObj::sMemoryPool.Free(netObj);
    destroyOk = false;
}

// FUNCTION: SWBF2 0x005bc430
void FindCreateObj(EntityClass* entityClass, int player)
{
    int found = -1;
    for (unsigned int i = 0; i < createPoolCount; ++i)
    {
        NetObj* netObj = createPoolPtr[i];
        if (!netObj)
        {
            continue;
        }
        GameObject* object = netObj->obj;
        EntityClass* entClass = object->GetEntityClass();
        if (entClass->GetDerivedRtti() == entityClass->GetDerivedRtti())
        {
            found = i;
            if (player >= 0 && object->GetControllable() &&
                object->GetControllable()->GetPlayerId() == player)
            {
                break;
            }
        }
    }
    if (found != -1)
    {
        NetObj* netObj = createPoolPtr[found];
        netObj->uniqId = 63;
        Remove(netObj, false);
        createPoolPtr[found] = 0;
    }
}

// FUNCTION: SWBF2 0x005d02a0
void ReadSpawnSelect(NetPktGroup& group, int player)
{
    unsigned int bits;
    for (int i = 0; i < 2; ++i)
    {
        int id;
        if (i == 0)
        {
            id = player;
        }
        else
        {
            id = NetGame::GetMyGuest(player, i - 1);
        }
        NetPlayer& np = netPlayers[id];
        bool hasSpawn;
        group.Read(hasSpawn);
        if (hasSpawn)
        {
            unsigned int changeCount;
            group.ReadBits(changeCount, 8);
            bool changed = changeCount != np.spawnChangeCount;
            np.spawnChangeCount = changeCount;
            bool cancel;
            group.Read(cancel);
            int team = 1;
            int classIndex = 0;
            CommandPost* post = 0;
            bool spectate = false;
            bool sideSelect = false;
            if (!cancel)
            {
                group.Read(spectate);
                if (spectate)
                {
                    if (NetGame::IsEnabled() && !NetGame::IsClient())
                    {
                        if (!NetGame::IsSpectator(id))
                        {
                            int spectate_id = NetGame::FindSpectateTarget(id);
                            NetGame::SetSpectator(id, spectate_id);
                        }
                        NetGame::SetSpawning(id, false);
                    }
                }
                else
                {
                    group.Read(sideSelect);
                    if (sideSelect)
                    {
                        if (!netAutoAssignTeams || !np.selectedSide)
                        {
                            int team1 = 0;
                            int team2 = 0;
                            for (int j = 0; j < netCurMaxPlayers; ++j)
                            {
                                if (NetGame::IsPlaying(j, false) && netPlayers[j].selectedSide)
                                {
                                    if (netPlayers[j].isTeam1)
                                    {
                                        ++team1;
                                    }
                                    else
                                    {
                                        ++team2;
                                    }
                                }
                            }
                            if (netSetForceHumansToTeam1)
                            {
                                np.isTeam1 = true;
                            }
                            else
                            {
                                np.isTeam1 = team1 <= team2;
                                int joystick = NetGame::GetJoystickIndex(id);
                                if (joystick >= 0)
                                {
                                    bool waiting = SpawnDisplay::Instance(joystick)->IsWaitingForTeam();
                                    if (waiting)
                                    {
                                        SpawnDisplay::Instance(joystick)->SelectTeam(NetGame::GetStartTeam(id));
                                        SpawnDisplay::Instance(joystick)->SetWaitingForTeam(false);
                                    }
                                }
                            }
                            np.selectedSide = true;
                            NetSetup::PlayerDataChanged(id);
                        }
                    }
                    else
                    {
                        bool isTeam1;
                        group.Read(isTeam1);
                        team = isTeam1 ? 1 : 2;
                        group.ReadBits(bits, 4);
                        classIndex = bits;
                        bool randomPost;
                        group.Read(randomPost);
                        if (!randomPost)
                        {
                            group.ReadBits(bits, 4);
                            post = CommandPost::FindPostByIndex(bits);
                        }
                    }
                }
            }
            if (changed)
            {
                GameVoiceChat::PlayerChange();
                if (cancel || spectate || sideSelect)
                {
                    np.spawnRequested = false;
                }
                else
                {
                    bool isTeam1 = team == 1;
                    if (!netAutoAssignTeams && !np.selectedSide)
                    {
                        np.isTeam1 = isTeam1;
                    }
                    if (netAutoAssignTeams)
                    {
                        if (np.selectedSide && np.isTeam1 != isTeam1)
                        {
                            team = np.isTeam1 ? 1 : 2;
                            post = SpawnManager::Instance()->GetRandomSpawnPoint(team);
                        }
                    }
                    else
                    {
                        np.isTeam1 = isTeam1;
                        np.selectedSide = true;
                    }
                    if (!post)
                    {
                        post = SpawnManager::Instance()->GetRandomSpawnPoint(team);
                    }
                    np.spawnRequested = true;
                    np.spawnClass = classIndex;
                    np.spawnPost = post ? post->GetIndex() : 0;
                }
                Character* character = Character::FindPlayer(id);
                if (character && !character->GetUnit() && cancel)
                {
                    character->SetSpawnReady(false);
                    np.spawnRequested = false;
                }
            }
        }
        if (id >= 0 && np.spawnRequested)
        {
            Character* character = Character::FindPlayer(id);
            if (character)
            {
                if (!character->GetUnit() && character->GetDelayTimer() < 0.25f)
                {
                    int spawnTeam = np.isTeam1 ? 1 : 2;
                    int spawnClass = np.spawnClass;
                    CommandPost* spawnPost = CommandPost::FindPostByIndex(np.spawnPost);
                    Team* teamPtr = Team::GetPtr(spawnTeam);
                    if (spawnClass >= teamPtr->GetClassCount())
                    {
                        spawnClass = 0;
                    }
                    if (!spawnPost || !spawnPost->CanTeamSpawnHere(spawnTeam))
                    {
                        spawnPost = SpawnManager::Instance()->GetRandomSpawnPoint(spawnTeam);
                    }
                    character->SelectSpawn(spawnTeam, spawnClass, spawnPost);
                    character->SetSpawnReady(true);
                    np.spawnRequested = false;
                }
                else if (character->GetUnit())
                {
                    if (!character->GetVehicle() && !character->GetRemote())
                    {
                        int spawnTeam = np.isTeam1 ? 1 : 2;
                        int spawnClass = np.spawnClass;
                        Team* teamPtr = Team::GetPtr(spawnTeam);
                        if (spawnClass >= teamPtr->GetClassCount())
                        {
                            spawnClass = 0;
                        }
                        character->ChangeClass(spawnClass);
                    }
                    np.spawnRequested = false;
                }
            }
        }
    }
    if (NetComm::GetArch(player) == NET_ARCH_FAKE)
    {
        Character* character = Character::FindPlayer(player);
        if (character && !character->GetUnit())
        {
            int team = (player & 1) + 1;
            CommandPost* post = SpawnManager::Instance()->GetRandomSpawnPoint(team);
            character->SelectSpawn(team, 0, post);
            character->SetSpawnReady(true);
        }
    }
}

// FUNCTION: SWBF2 0x005d1b50
void NetGame::ReadChatMessageFromClient(NetPktGroup& group)
{
    unsigned int size = 0;
    NG_ChatMsg message;
    group.ReadBytes(reinterpret_cast<char*>(&size), 4);
    if ((size & 0xffff0000) != 0xffff0000)
    {
        return;
    }
    size &= 0xffff;
    if (!size)
    {
        return;
    }
    group.ReadBytes(reinterpret_cast<char*>(&message), size);
    LogChatMessage(message);
    if (NetAdmin::ParseLineRemote(message.player_id, message.msg, false, true))
    {
        return;
    }
    bool accept = true;
    if ((int)message.player_id < netCurMaxPlayers)
    {
        NetPlayer& np = netPlayers[message.player_id];
        if ((int)(NetGame::GetTurnNumber() - np.mLastStatusTurn) > netTurnsPerSecond * 3 / 2)
        {
            np.mLastStatusTurn = NetGame::GetTurnNumber();
        }
        else
        {
            accept = false;
        }
    }
    if (accept)
    {
        NetGame::AddChatMessageToQue(true, message.player_id, message.mask, message.code, message.msg, message.flag);
    }
}

void ReadQuaternion(NetPktGroup& group, PblQuaternion& value);

namespace StatsManager
{
void ReadStatsChallenge(int player, NetPktGroup& group);
}
namespace StatsManager
{
void ReadStatsResponse(int player, NetPktGroup& group);
}

// FUNCTION: SWBF2 0x005bc740
GameObject* CreateObject(unsigned int uniqId, EntityClass* entityClass, int initialPlayerId, int playerId,
                         PblQuaternion& rotation, PblVector3& position, Character* character, Team* team)
{
    SmoothInfo& smooth = NetPredict::GetSmoothInfo(uniqId);
    PblMatrix smoothMat(rotation, position);
    smooth.ResetMat(smoothMat);
    smooth.mPlayerId = initialPlayerId;
    gameObjectInterpolator.SetNext(uniqId, 0);
    if (character && !character->IsThreadActive())
    {
        character->ActivateThread(0, PER_LOCAL_TURN);
    }
    if (initialPlayerId == debugCreateInitialPlayer)
    {
        ++gDebugCount;
    }
    if (!isFullState)
    {
        FindCreateObj(entityClass, playerId);
    }
    if (isFullState)
    {
        PblHandled::SetNextHandleId(uniqId);
    }

    EntityDesc desc;
    desc.mNameId = 0;
    desc.mPlayerId = playerId;
    desc.mMatrix = PblMatrix(rotation, position);
    Entity* entity = entityClass->Build(desc);
    if (!entity)
    {
        CloseJournal();
        Dump();
    }
    GameObject* object = entity->GetGameObject();
    object->SetTeam(team->GetNumber());
    object->SetPerceivedTeam(team->GetNumber());
    object->SetNetUniqueId(uniqId);
    if (object->GetHandleId() == (unsigned int)debugCreateHandleId)
    {
        ++gDebugCount;
    }
    if (initialPlayerId != playerId)
    {
        object->GetControllable()->SetInitialPlayerId(initialPlayerId);
    }
    TrackCreate(object);
    if (uniqId == (unsigned int)netTraceIndex)
    {
        netTraceObjectHandleId = object->GetHandleId();
    }

    if (playerId >= 0)
    {
        Character* player = Character::FindPlayer(playerId);
        if (player)
        {
            player->Validate();
            Controllable* controllable = object->GetControllable();
            switch (controllable->GetPilotType())
            {
            case PILOT_SELF:
                player->SetUnit(controllable);
                controllable->SetCharacter(player);
                break;
            case PILOT_VEHICLE:
            case PILOT_VEHICLESELF:
                if (player->GetVehicle())
                {
                    Controllable* vehicle = player->GetVehicle();
                    vehicle->SetPilot(0);
                    player->SetVehicle(0);
                }
                player->SetVehicle(controllable);
                controllable->SetCharacter(player);
                controllable->SetPilot(player->GetUnit());
                break;
            case PILOT_REMOTE:
                if (player->GetRemote())
                {
                    Controllable* remote = player->GetRemote();
                    remote->SetPilot(0);
                    player->SetRemote(0);
                }
                player->SetRemote(controllable);
                controllable->SetCharacter(player);
                controllable->SetPilot(player->GetUnit());
                break;
            }
            player->Validate();
        }
    }

    if (!isFullState)
    {
        NetObj* netObj = new NetObj;
        netObj->obj = object;
        netObj->usedTurn = clientHostTurnNumber;
        PblValues::Record(uniqId, PVT_FULL);
        PblValues::Record(object->GetHandleId(), PVT_FULL);
        unsigned int index = uniqId;
        if (index == 63)
        {
            ++gDebugCount;
        }
        clientObjMap.Add(netObj, index);
    }

    if (character)
    {
        if (character->GetRemote() && character->GetRemote()->GetPilot())
        {
            Controllable* remote = character->GetRemote();
            remote->SetPilot(0);
            character->SetRemote(0);
        }
        if (character->GetVehicle() && character->GetVehicle()->GetPilot())
        {
            Controllable* vehicle = character->GetVehicle();
            vehicle->SetPilot(0);
            character->SetVehicle(0);
        }
        Controllable* controllable = object->GetControllable();
        if (controllable->GetPilotType() == PILOT_SELF)
        {
            character->SetUnit(controllable);
            controllable->SetPilot(controllable);
            controllable->SetCharacter(character);
            character->Validate();
        }
    }

    object->Init();
    switch (object->GetEntityType())
    {
    case ENT_SOLDIER:
        smooth.type = ST_SOLDIER;
        break;
    case ENT_WALKER:
    case ENT_DROIDEKA:
    case ENT_POST_WALKER:
        smooth.type = ST_WALKER;
        break;
    default:
        smooth.type = ST_VEHICLE;
        break;
    }

    if (saveObjState)
    {
        NetObjState* state = new NetObjState;
        state->handle = object;
        state->classIdx = (unsigned char)entityClass->GetNetIndex();
        state->playerId = playerId >= 0 ? (unsigned char)playerId : (unsigned char)-1;
        state->initialPlayerId = initialPlayerId >= 0 ? (unsigned char)initialPlayerId : (unsigned char)-1;
        state->character = character ? (unsigned char)(character - Character::GetPtr(0)) : (unsigned char)-1;
        state->rotation = rotation;
        state->position = position;
        state->team = (unsigned char)team->GetNumber();
        state->perceivedTeam = state->team;
        clientNextObjState->Add(state, uniqId);
    }
    return object;
}

// FUNCTION: SWBF2 0x005bc530
GameObject* ReadCreate(NetPktGroup& group, unsigned int uniqId)
{
    EntityClass* entityClass = ReadEntityClass(group);
    int initialPlayerId = -1;
    int playerId = -1;
    bool hasPlayer;
    group.Read(hasPlayer);
    if (hasPlayer)
    {
        group.ReadBits((unsigned int&)initialPlayerId, 6);
        bool same;
        group.Read(same);
        if (same)
        {
            playerId = initialPlayerId;
        }
    }
    PblQuaternion rotation;
    ReadQuaternion(group, rotation);
    PblVector3 position;
    ReadPositionVector(group, position);

    Character* character = 0;
    if (entityClass->IsRtti(EntitySoldierClass::GetRtti()) || entityClass->IsRtti(EntityDroidekaClass::GetRtti()))
    {
        character = ReadCharacter(group);
        if (character && character->IsPlayer())
        {
            GameVoiceChat::PlayerChange();
        }
    }
    if (entityClass->IsRtti(CommandWalkerClass::GetRtti()) || entityClass->IsRtti(CommandHoverClass::GetRtti()) ||
        entityClass->IsRtti(CommandFlyerClass::GetRtti()))
    {
        unsigned int buildSlot;
        group.ReadBits(buildSlot, 4);
        CommandPost::ForceIndex(buildSlot);
    }
    Team* team = ReadTeam(group);
    if (character)
    {
        if (hasPlayer)
        {
            Team* playerTeam = Team::GetPtr(NetGame::GetStartTeam(initialPlayerId));
            if (team != playerTeam)
            {
                NetGame::SetStartTeam(initialPlayerId, team->GetNumber());
                character->SelectTeam(team->GetNumber());
            }
        }
        else if (((GameObjectClass*)entityClass)->GetHeroFlag())
        {
            character->SetName(entityClass->mLabel);
        }
    }
    GameObject* object = CreateObject(uniqId, entityClass, initialPlayerId, playerId, rotation, position, character, team);
    object->ReadCreate(group);
    return object;
}

// FUNCTION: SWBF2 0x005bce10
void ReCreate(NetObjState* state, unsigned int uniqId)
{
    EntityClass* entityClass = Factory<Entity, EntityClass, EntityDesc>::FindNetIndex(state->classIdx);
    int playerId = state->playerId == 0xff ? -1 : state->playerId;
    int initialPlayerId = state->initialPlayerId == 0xff ? -1 : state->initialPlayerId;
    PblQuaternion& rotation = state->rotation;
    PblVector3& position = state->position;
    Character* character = state->character == 0xff ? 0 : Character::GetPtr(state->character);
    if (!character || !(entityClass->IsRtti(EntitySoldierClass::GetRtti()) || entityClass->IsRtti(EntityDroidekaClass::GetRtti())))
    {
        character = 0;
    }
    Team* team = Team::GetPtr(state->team);
    if (playerId >= 0)
    {
        GameObjectClass* objectClass = (GameObjectClass*)entityClass;
        const ControllableClass* controllableClass =
            (const ControllableClass*)objectClass->GetControllableClass();
        if (controllableClass && controllableClass->mPilotType != PILOT_SELF)
        {
            Character* player = Character::FindPlayer(playerId);
            Controllable* unit = player ? player->GetUnit() : 0;
            if (!unit)
            {
                initialPlayerId = -1;
                playerId = -1;
            }
        }
    }
    GameObject* object = CreateObject(uniqId, entityClass, initialPlayerId, playerId, rotation, position,
                                      character, team);
    state->handle = object;
}

// FUNCTION: SWBF2 0x005bcfc0
bool ReadObject(NetPktGroup& group)
{
    readObj = 0;
    bool isND;
    group.Read(isND);
    if (isND)
    {
        unsigned int index;
        group.ReadBits(index, 8);
        NetNdObj* ndObj = clientNdObjMap.FindIndex(index);
        GameObject* object = isFullState ? ndObj->orig : ndObj->obj;
        object->ReadObject(group, 0);
        ClearSkippedND(object);
    }
    else
    {
        GameObject* object = 0;
        unsigned int index;
        if (!isFullState)
        {
            group.ReadBits(index, 6);
            if (index == 63)
            {
                return false;
            }
            NetObj* netObj = clientObjMap.Find(index);
            netObj->usedTurn = clientHostTurnNumber;
            object = netObj->obj;
        }
        else
        {
            group.ReadBits(index, 16);
            if (!index)
            {
                return false;
            }
            object = ReadCreate(group, index);
        }
        NetObjState* last = 0;
        if (!isFullState)
        {
            last = clientLastObjState->Find(index);
        }
        object->ReadObject(group, last);
        if (!isFullState && saveObjState)
        {
            NetObjState* state = clientNextObjState->Find(index);
            if (!state)
            {
                int playerId = -1;
                int initialPlayerId = -1;
                Character* character = 0;
                Controllable* controllable = object->GetControllable();
                if (controllable)
                {
                    playerId = controllable->GetPlayerId();
                    initialPlayerId = controllable->GetInitialPlayerId();
                    character = controllable->GetCharacter();
                }
                state = new NetObjState;
                state->handle = object;
                state->classIdx = (unsigned char)object->GetEntityClass()->GetNetIndex();
                state->playerId = playerId >= 0 ? (unsigned char)playerId : (unsigned char)-1;
                state->initialPlayerId = initialPlayerId >= 0 ? (unsigned char)initialPlayerId : (unsigned char)-1;
                state->character = character ? (unsigned char)(character - Character::GetPtr(0)) : (unsigned char)-1;
                state->position = object->GetMatrix().GetTranslation();
                state->team = (unsigned char)object->GetTeam();
                state->perceivedTeam = (unsigned char)object->GetPerceivedTeam();
                clientNextObjState->Add(state, index);
            }
        }
        readObj = object;
    }
    return true;
}

// FUNCTION: SWBF2 0x005cdc70
void ReadObjects(NetPktGroup& group)
{
    int readCount;
    unsigned int i;
    NetObj* deferred[64];
    createPoolPtr = deferred;
    createPoolCount = 0;
    if (clientHostTurnNumber == debugReadHostTurn)
    {
        ++gDebugCount;
    }
    if (!isFullState)
    {
        bool removed[64];
        memset(removed, 0, sizeof(removed));
        bool hasRemoves;
        group.Read(hasRemoves);
        if (hasRemoves)
        {
            readCount = 0;
            while (true)
            {
                unsigned int index;
                group.ReadBits(index, 6);
                if (index == 63)
                {
                    break;
                }
                bool flag;
                group.Read(flag);
                NetObj* netObj = clientObjMap.Find(index);
                if (netObj)
                {
                    Remove(netObj, flag);
                }
                removed[index] = true;
                ++readCount;
            }
        }
        for (i = 0; i < 64; ++i)
        {
            NetObj* netObj = clientObjMap.Find(i);
            if (!netObj)
            {
                continue;
            }
            netObj->obj->GetGameObjectClass();
            NetObjState* state = clientLastObjState->Find(i);
            if (state && (state->handle == netObj->obj))
            {
                continue;
            }
            clientObjMap.Remove(netObj);
            if (netObj->obj->GetEntityType() == ENT_POST_HOVER ||
                netObj->obj->GetEntityType() == ENT_POST_WALKER ||
                netObj->obj->GetEntityType() == ENT_POST_FLYER)
            {
                netObj->uniqId = 63;
                Remove(netObj, false);
            }
            else
            {
                createPoolPtr[createPoolCount++] = netObj;
            }
        }
        bool hasCreates;
        group.Read(hasCreates);
        if (hasCreates)
        {
            readCount = 0;
            while (true)
            {
                unsigned int index;
                group.ReadBits(index, 6);
                if (index == 63)
                {
                    break;
                }
                if (readCount == debugReadCreateOrdinal && clientHostTurnNumber == debugReadHostTurn)
                {
                    ++gDebugCount;
                }
                ReadCreate(group, index);
                ++readCount;
            }
        }
        for (i = 0; i < createPoolCount; ++i)
        {
            NetObj* netObj = createPoolPtr[i];
            if (!netObj)
            {
                continue;
            }
            netObj->uniqId = 63;
            Remove(netObj, false);
        }
        createPoolCount = 0;
        for (i = 0; i < 64; ++i)
        {
            NetObjState* state = clientLastObjState->Find(i);
            if (!state)
            {
                continue;
            }
            NetObj* netObj = clientObjMap.Find(i);
            if (netObj)
            {
                continue;
            }
            if (removed[i])
            {
                continue;
            }
            ReCreate(state, i);
        }
    }

    for (unsigned int k = 0; k < 8; ++k)
    {
        ndSkippedMask[k] = (unsigned int)-1;
    }
    readCount = 0;
    while (true)
    {
        if (readCount == debugReadObjectOrdinal && clientHostTurnNumber == debugReadHostTurn)
        {
            ++gDebugCount;
        }
        if (!ReadObject(group))
        {
            break;
        }
        CheckReadAlign(group);
        ++readCount;
    }

    for (i = 0; i < 64; ++i)
    {
        unsigned int index = i;
        NetObj* netObj = clientObjMap.Find(index);
        if (!netObj)
        {
            continue;
        }
        if (netObj->usedTurn == (unsigned int)clientHostTurnNumber)
        {
            clientSkipped[i] = false;
            continue;
        }
        clientSkipped[i] = true;
        GameObject* object = netObj->obj;
        NetObjState* last = clientLastObjState->Find(index);
        object->SkipReadObject(last);
        if (saveObjState)
        {
            NetObjState* state = new NetObjState;
            state->handle = object;
            state->classIdx = last->classIdx;
            state->playerId = last->playerId;
            state->initialPlayerId = last->initialPlayerId;
            state->character = last->character;
            state->position = last->position;
            state->team = last->team;
            state->perceivedTeam = last->perceivedTeam;
            clientNextObjState->Add(state, index);
        }
    }
}

// FUNCTION: SWBF2 0x005bb5b0
void ReadUpdate(NetPktGroup& group)
{
    BeginNetLOD();
    if (isFullState)
    {
        if (curDst == -1)
        {
            group.ReadBits((unsigned int&)hostTurnNumber, 32);
            SetTurnHighBits(hostTurnNumber);
            ReadDestMask(group, hostPlayingMask);
        }
    }
    else
    {
        curDst = NetComm::GetLocalIndex(0);
        curIndex = -1;
        if (curDst == NetComm::GetLocalIndex(0))
        {
            curIndex = 0;
        }
        else if (curDst == NetComm::GetLocalIndex(1))
        {
            curIndex = 1;
        }
        else
        {
            return;
        }

        unsigned int turn;
        ReadTurnLowBits(group, turn);
        if (turn <= (unsigned int)clientHostTurnNumber)
        {
            return;
        }
        clientHostTurnNumber = turn;
        SetTurnHighBits(clientHostTurnNumber);
        clientAhead = clientTurnNumber - clientHostTurnNumber;
        IsNetObjectFeatureEnabled();
        if (clientHostTurnNumber == 1207)
        {
            ++gDebugCount;
        }
        updateTurnNumber = clientHostTurnNumber;
        ++clientUpdateCount;

        unsigned __int64 lastMask = clientPlayingMask;
        ReadDestMask(group, clientPlayingMask);
        if (clientPlayingMask != lastMask)
        {
            UpdateCurMaxPlayers(clientPlayingMask, true);
        }
        NetSetup::ReadGameUpdate(group);
        if (clientPlayingMask != lastMask)
        {
            unsigned __int64 left = lastMask & ~clientPlayingMask;
            unsigned __int64 joined = clientPlayingMask & ~lastMask;
            int i;
            for (i = 0; i < netCurMaxPlayers; ++i)
            {
                if (left & ((unsigned __int64)1 << i))
                {
                    NetGame::RemovePlayer(i);
                }
            }
            for (i = 0; i < netCurMaxPlayers; ++i)
            {
                if (joined & ((unsigned __int64)1 << i))
                {
                    NetGame::AddPlayer(i);
                }
            }
        }

        group.Read(clientHostCommProblems);
        bool quit;
        group.Read(quit);
        if (quit)
        {
            bool hostQuit;
            group.Read(hostQuit);
            if (hostQuit)
            {
                canQuit = true;
                if (::hostQuit)
                {
                    NetMatch::SetGameError("ifs.mp.joinerrors.hostquit", 6);
                }
            }
            else if (!GameLoop::IsQuitRequested())
            {
                netInGame = false;
                if (!GameLoop::sQuitRequested)
                {
                    GameLoop::sQuitTurn = 0;
                    GameLoop::sQuitRequested = true;
                }
                ::hostQuit = true;
                if (::hostQuit)
                {
                    NetMatch::SetGameError("ifs.mp.joinerrors.hostquit", 6);
                }
            }
        }
        bool booted;
        group.Read(booted);
        if (booted)
        {
            netInGame = false;
            canQuit = true;
            NetMatch::SetGameError("ifs.mp.joinerrors.booted", 6);
        }
    }

    if (curDst >= 0)
    {
        unsigned int lagBits;
        group.ReadBits(lagBits, 8);
        int lag = (int)lagBits - 128;
        if (isFullState)
        {
            netPlayers[curDst].lagSamples[0] = lag;
        }
        else
        {
            clientLag = lag;
            if (clientTurnNumber < (unsigned int)clientHostTurnNumber)
            {
                int ping = NetComm::GetPing(NetComm::GetLocalIndex(0));

                float turns = (float)netTurnsPerSecond * ((float)ping / 1000.0f);
                unsigned int nextTurn = (unsigned int)(turns + 0.9f) + clientHostTurnNumber;
                clientTurnNumber = nextTurn;
                sendTurnNumber = clientTurnNumber - 1;
                clientLag = 0;
                timeScale = 1.0f;
                pendingTimeAdjust = 0.0f;
                finishedTimeScale = true;
                adjustTime = NetComm::GetTime() + 1.0f;
                timeStableCount = 0;
                timeLocked = false;
            }
        }
        unsigned int ackBits;
        group.ReadBits(ackBits, 8);
        clientAckedTurnNumber = clientHostTurnNumber - ((int)ackBits - 128);
        if (isFullState)
        {
            ReadTurnLowBits(group, netPlayers[curDst].clientTurn);
            for (unsigned int slot = 0; slot < 1; ++slot)
            {
                int guest = NetGame::GetMyGuest(curDst, slot);
                if (guest >= 0)
                {
                    netPlayers[guest].clientTurn = netPlayers[curDst].clientTurn;
                }
            }
            if (netPlayers[curDst].clientOfIndex == netHostIndex)
            {
                ReadSwitchResponses(group, netPlayers[curDst]);
            }
            else
            {
                ReadRelaySwitchResponse(group, netPlayers[curDst].objSwitchRelay);
            }
        }
        else
        {
            ReadSwitchObjRequest(group);
        }
    }

    if (group.IsEnd())
    {
        return;
    }
    if (!isFullState || curDst == -1)
    {
        ReadObjects(group);
        CheckReadAlign(group);
    }
    ReadSpectatorInfo(group, curDst);
    AtleastOnce::ReadAll(group);
    if (curDst >= 0)
    {
        ReadPlayerMoves(group);
        CheckReadAlign(group);
        ReadPingData(group);
        CheckReadAlign(group);
        ReadCaptureDisplay(group, curDst);
        for (unsigned int slot = 0; slot < 1; ++slot)
        {
            int guest = NetGame::GetMyGuest(curDst, slot);
            if (guest >= 0)
            {
                ReadCaptureDisplay(group, guest);
            }
        }
        CheckReadAlign(group);
        StatsManager::ReadStatsChallenge(curDst, group);
        ReadSpawnDisplay(group);
        CheckReadAlign(group);
        PAManager::ReadObjects(group);
        CheckReadAlign(group);
        ReceiveReliableNetEvents(group);
        CheckReadAlign(group);
        NetGame::ReadChatMessageFromServer(group);
        ReceiveNetEvents(group);
    }
    if (!isFullState)
    {
        ReadUploadAck(group);
    }
    for (unsigned int local = 0; local < 2; ++local)
    {
        if (NetGame::GetLocalIndex(local) >= 0)
        {
            Tracker* tracker = Tracker::GetTracker(NetGame::GetLocalIndex(local));
            Trackable* trackable = tracker->GetTrackable();
            if (trackable)
            {
                GameObject* object = trackable->GetGameObject();
                netLocalPos[local] = object->GetMatrix().GetTranslation();
            }
        }
    }
    CheckReadAlign(group);
}

// FUNCTION: SWBF2 0x005c9f50
void ReceivePrimary()
{
    if (netHosts[netHostIndex].hostMode != HM_HOST)
    {
        return;
    }

    while (true)
    {
        NetPktGroup group;
        if (!NetComm::Recv(group, PKT_STATS_MOVE))
        {
            break;
        }
        NetPacket* packet = NetPacket::Get(group.firstPkt);
        int player = GetSrc(packet);
        group.BeginRead();
        bool hasTurn;
        group.Read(hasTurn);
        if (hasTurn)
        {
            unsigned int turn;
            group.ReadBits(turn, 19);
            if (GameLoop::IsGameOver())
            {
                netPlayers[player].hasStats = true;
                if (turn > netPlayers[player].clientTurn)
                {
                    netPlayers[player].clientTurn = turn;
                    for (unsigned int slot = 0; slot < 1; ++slot)
                    {
                        int guest = NetGame::GetMyGuest(player, slot);
                        if (guest >= 0)
                        {
                            netPlayers[guest].clientTurn = turn;
                        }
                    }
                }
            }
            NetGame::ReadChatMessageFromClient(group);
        }
        group.EndRead();
        group.Free();
        if (clientDisconnectStartTime != 0.0f && !NetGame::IsPlaying(player, false))
        {
            float elapsed = NetComm::GetTime() - clientDisconnectStartTime;
            if (elapsed > 5.0f)
            {
                NetComm::Disconnect(player);
            }
        }
    }

    int lag[64];
    unsigned int lastTurn[64];
    unsigned int firstTurn[64];
    int i;
    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        lastTurn[i] = 0;
        firstTurn[i] = (unsigned int)-1;
    }
    ProcessedInputs oldMoves[2];

    while (true)
    {
        NetPktGroup group;
        if (!NetComm::Recv(group, PKT_GAME_MOVE))
        {
            break;
        }
        NetPacket* packet = NetPacket::Get(group.firstPkt);
        int player = GetSrc(packet);
        group.BeginRead();
        if (player == gDebugDst)
        {
            ++gDebugCount;
        }
        if (!NetGame::IsPlaying(player, false) && !NetGame::IsPlayingNext(player))
        {
            group.EndRead();
            group.Free();
            continue;
        }

        bool isLate;
        group.Read(isLate);
        if (isLate)
        {
            unsigned int lateTurn = hostTurnNumber;
            int slot = lateTurn & 31;
            for (i = 0; i < 4; ++i)
            {
                if (!(hostRecv[slot] & ((unsigned __int64)1 << player)))
                {
                    break;
                }
                slot = ++lateTurn & 31;
            }
            if (i < 4)
            {
                ProcessedInputs* inputs = &hostTurns[slot].moves[player];
                ReadMiniMove(group, inputs);
                hostRecv[slot] |= (unsigned __int64)1 << player;
            }
            group.EndRead();
            group.Free();
        }
        else
        {
            unsigned int turn;
            ReadTurnLowBits(group, turn);
            if (player == debugReceivePlayer)
            {
                IsNetObjectFeatureEnabled();
            }
            bool stale = false;
            if (turn > netPlayers[player].mLastReceivedTurn)
            {
                netPlayers[player].mLastReceivedTurn = turn;
            }
            else
            {
                if (player == netTraceDst)
                {
                    ++gDebugCount;
                }
                stale = true;
            }
            if (turn >= lastTurn[player])
            {
                lastTurn[player] = turn;
                lag[player] = clientTurnNumber - turn;
            }

            bool quit;
            group.Read(quit);
            if (quit && NetGame::GetJoystickIndex(player) < 0)
            {
                netPlayers[player].requestedQuit = true;
            }

            int hostIndex = netPlayers[player].hostIndex;
            if (hostIndex >= 0)
            {
                unsigned int hostTurn;
                switch (netHosts[hostIndex].hostMode)
                {
                case HM_REPLICA:
                    ReadTurnLowBits(group, hostTurn);
                    if (hostTurn > netHosts[hostIndex].hostAckedUpdateTurn)
                    {
                        netHosts[hostIndex].hostAckedUpdateTurn = hostTurn;
                    }
                    break;
                case HM_RELAY:
                    ReadTurnLowBits(group, hostTurn);
                    if (hostTurn > netHosts[hostIndex].hostAckedUpdateTurn)
                    {
                        netHosts[hostIndex].hostAckedUpdateTurn = hostTurn;
                    }
                    break;
                }
            }

            unsigned int clientTurn;
            ReadTurnLowBits(group, clientTurn);
            if (clientTurn == (unsigned int)-1)
            {
                clientTurn = 0;
            }
            if (clientTurn > netPlayers[player].clientTurn)
            {
                netPlayers[player].clientTurn = clientTurn;
                for (unsigned int slot = 0; slot < 1; ++slot)
                {
                    int guest = NetGame::GetMyGuest(player, slot);
                    if (guest >= 0)
                    {
                        netPlayers[guest].clientTurn = clientTurn;
                    }
                }
            }
            ReceivedUpdateAck(player, clientTurn);
            if (!stale && NetGame::IsPlaying(player, false))
            {
                if (netPlayers[player].clientOfIndex == netHostIndex)
                {
                    ReadSwitchResponses(group, netPlayers[player]);
                }
                else
                {
                    ReadRelaySwitchResponse(group, netPlayers[player].objSwitchRelay);
                }
            }
            else
            {
                SkipSwitchResponses(group);
            }

            curDst = player;
            SetNetLOD(1);
            ReadSpawnSelect(group, player);
            ReadLockedTargets(group, player);
            ReadBootRequest(group, player);
            ReadSpawnCamera(group);
            NetGame::ReadChatMessageFromClient(group);
            ReadUpload(group);
            ReadSuicideRequest(group, player);

            unsigned int moveTurn = turn;
            while (true)
            {
                StatsManager::ReadStatsResponse(curDst, group);
                group.Read(cockpitCams[curDst]);
                turn = moveTurn;
                if (turn + 3 < (unsigned int)hostTurnNumber)
                {
                    break;
                }
                if (turn + 32 > (unsigned int)hostTurnNumber + 48)
                {
                    break;
                }
                if (turn < firstTurn[curDst])
                {
                    firstTurn[curDst] = turn;
                }
                int slot = turn & 31;
                ProcessedInputs* inputs = &hostTurns[slot].moves[curDst];
                if (turn < (unsigned int)hostTurnNumber)
                {
                    inputs = &oldMoves[turn & 1];
                }
                ReadMove(group, &noInputs, inputs);
                if (turn + 3 >= (unsigned int)hostTurnNumber)
                {
                    if (turn >= (unsigned int)hostTurnNumber)
                    {
                        hostRecv[slot] |= (unsigned __int64)1 << curDst;
                    }
                    else
                    {
                        ProcessedInputs* hostInputs = &hostTurns[slot].moves[curDst];
                        if (!(hostRecv[slot] & ((unsigned __int64)1 << curDst)))
                        {
                            if (IsSame(inputs, hostInputs))
                            {
                                hostRecv[slot] |= (unsigned __int64)1 << curDst;
                            }
                            else
                            {
                                Character* character = Character::FindPlayer(curDst);
                                Controllable* unit = character ? character->GetUnit() : 0;
                                if (unit)
                                {
                                    hostRecv[slot] |= (unsigned __int64)1 << curDst;
                                    slot = turn & 7;
                                    lateRecv[slot] |= (unsigned __int64)1 << curDst;
                                    *hostInputs = *inputs;
                                }
                                else
                                {
                                    hostRecv[slot] |= (unsigned __int64)1 << curDst;
                                }
                            }
                        }
                    }
                }

                while (true)
                {
                    if (!turn)
                    {
                        break;
                    }
                    --turn;
                    if (turn < firstTurn[curDst])
                    {
                        firstTurn[curDst] = turn;
                    }
                    slot = turn & 31;
                    ProcessedInputs* next = &hostTurns[slot].moves[curDst];
                    if (turn < (unsigned int)hostTurnNumber)
                    {
                        next = &oldMoves[turn & 1];
                    }
                    if (!ReadMove(group, inputs, next))
                    {
                        break;
                    }
                    inputs = next;
                    if (turn + 3 >= (unsigned int)hostTurnNumber)
                    {
                        if (turn >= (unsigned int)hostTurnNumber)
                        {
                            hostRecv[slot] |= (unsigned __int64)1 << curDst;
                        }
                        else
                        {
                            next = &hostTurns[slot].moves[curDst];
                            if (!(hostRecv[slot] & ((unsigned __int64)1 << curDst)))
                            {
                                if (IsSame(next, inputs))
                                {
                                    hostRecv[slot] |= (unsigned __int64)1 << curDst;
                                }
                                else
                                {
                                    Character* character = Character::FindPlayer(curDst);
                                    Controllable* unit = character ? character->GetUnit() : 0;
                                    if (unit)
                                    {
                                        hostRecv[slot] |= (unsigned __int64)1 << curDst;
                                        slot = turn & 7;
                                        lateRecv[slot] |= (unsigned __int64)1 << curDst;
                                        *next = *inputs;
                                    }
                                    else
                                    {
                                        hostRecv[slot] |= (unsigned __int64)1 << curDst;
                                    }
                                }
                            }
                        }
                    }
                }

                bool more;
                group.Read(more);
                if (!more)
                {
                    break;
                }
                group.ReadBits((unsigned int&)curDst, 5);
            }
            group.EndRead();
            group.Free();
        }
    }

    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        if (lastTurn[i] > 0)
        {
            RecordLag(i, lag[i], lastTurn[i]);
        }
        if (firstTurn[i] != (unsigned int)-1)
        {
            FillTurns(i, firstTurn[i]);
        }
    }
}





// FUNCTION: SWBF2 0x005b7370
bool NetGame::CanExitVehicle(unsigned int player)
{

    return netPlayers[player].clientTurn > (unsigned int)(netPlayers[player].lastEnterVehicleTurn + 10);
}

namespace NetRandom
{
// GLOBAL: SWBF2 0x01e64b64
unsigned int hostRandom;
// GLOBAL: SWBF2 0x01e64b68
unsigned int clientRandom;

// FUNCTION: SWBF2 0x005b7550
void EnterClient()
{
    hostRandom = PblRandom::GetSim().GetSeed();
    PblRandom::GetSim().SetSeed(clientRandom);
}

// FUNCTION: SWBF2 0x005b7580
void LeaveClient()
{
    clientRandom = PblRandom::GetSim().GetSeed();
    PblRandom::GetSim().SetSeed(clientRandom);
}
}

// FUNCTION: SWBF2 0x005c4050
int NetGame::GetPreGameTime()
{
    return netPreGameTime;
}

// FUNCTION: SWBF2 0x005c4270
void NetGame::RemoveFromSpawnQueue(int player)
{
    if (player < 0 || player >= netCurMaxPlayers || !IsPlaying(player, false) || netPlayers[player].mSpawnOrder == -1)
    {
        return;
    }
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (IsPlaying(i, false) && i != player && netPlayers[i].isTeam1 == netPlayers[player].isTeam1 &&
            netPlayers[i].mSpawnOrder > netPlayers[player].mSpawnOrder)
        {
            --netPlayers[i].mSpawnOrder;
        }
    }
    netPlayers[player].mSpawnOrder = -1;
}

// FUNCTION: SWBF2 0x005c4360
void NetGame::AddToSpawnQueue(int player)
{
    if (player < 0 || player >= netCurMaxPlayers || !IsPlaying(player, false) || netPlayers[player].mSpawnOrder != -1)
    {
        return;
    }
    int last = -1;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (IsPlaying(i, false) && netPlayers[i].isTeam1 == netPlayers[player].isTeam1 &&
            netPlayers[i].mSpawnOrder > last)
        {
            last = netPlayers[i].mSpawnOrder;
        }
    }
    netPlayers[player].mSpawnOrder = last + 1;
}

inline bool AtleastOnceSame::IsConfirmed(int player)
{
    if (changeTurn == (unsigned int)-1)
    {
        return true;
    }
    if (netPlayers[player].clientTurn < changeTurn)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x005c5250
bool AtleastOnceSame::IsConfirmed()
{
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (netPlayers[i].clientOfIndex != netHostIndex)
        {
            continue;
        }
        if (NetGame::IsPlaying(i, false) && !IsConfirmed(i))
        {
            return false;
        }
    }
    return true;
}

inline bool AtleastOnceSame::CanChange()
{
    if (!hostTurnNumber)
    {
        return false;
    }
    if (!deltaSends)
    {
        return true;
    }
    if (IsConfirmed())
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x005c6fa0
bool AloStats::Sent()
{
    int players = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && NetGame::IsPlaying(i, false))
        {
            ++players;
        }
    }
    return players == 0 || (lastGameOver && IsConfirmed());
}

// FUNCTION: SWBF2 0x005c7030
bool SentStats()
{
    return aloStats.Sent();
}

// FUNCTION: SWBF2 0x005c7040
bool ReceivedStats()
{
    return aloStats.gotGameOver;
}

// FUNCTION: SWBF2 0x005c7200
bool AloNextGameInfo::Sent()
{
    int players = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && NetGame::IsPlaying(i, false))
        {
            ++players;
        }
    }
    return players == 0 || (lastGameOver && IsConfirmed());
}

// FUNCTION: SWBF2 0x005c7290
bool SentNextGameInfo()
{
    return aloNextGameInfo.Sent();
}

// FUNCTION: SWBF2 0x005c72a0
bool ReceivedNextGameInfo()
{
    return aloNextGameInfo.gotGameOver;
}

// FUNCTION: SWBF2 0x005d0980
void NetGame::RequestSuicide(int index)
{
    ++suicideInfo[index].changeCount;
    suicideInfo[index].changedTurn = clientTurnNumber;
    suicideInfo[index].killMeNow = true;
}

// FUNCTION: SWBF2 0x005d0da0
int NetGame::GetBootVotes()
{
    return clientBootVotes;
}

// FUNCTION: SWBF2 0x005d0db0
bool NetGame::IsBootVoted(int player)
{
    return clientBootVoted[player];
}

// FUNCTION: SWBF2 0x005d0de0
int NetGame::GetBootReason()
{
    return clientBootReason;
}

// FUNCTION: SWBF2 0x005d0df0
void NetGame::ForceBootPlayer(int player)
{
    if (NetGame::IsHost())
    {
        hostForceBootPlayer = player;
    }
}

// FUNCTION: SWBF2 0x005d25d0
void DisableFrameLock()
{
}

// FUNCTION: SWBF2 0x005c3790
int NetGame::FindPlayerByGSID(int low, int high)
{
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && (int)netPlayers[i].mGSPlayerID == low &&
            (int)(netPlayers[i].mGSPlayerID >> 32) == high)
        {
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x005ce9d0
void NetGame::ClearLastHero(int team)
{
    if (NetGame::IsHost() && !NetGame::IsClient() && team >= 0 && team < 3)
    {
        netHeroLastPlayerID[team] = -1;
        netHeroAIID[team] = -1;
    }
}

// GLOBAL: SWBF2 0x01faa6b8
int debugBootVoteCounter;

// GLOBAL: SWBF2 0x007ec4f4
int debugBootVoteIndex = -1;

// FUNCTION: SWBF2 0x005d0cc0
void NetGame::ConfirmBootPlayer(int player)
{
    bootInfo.changedTurn = clientTurnNumber;
    bootInfo.bootRequest = BR_AFFIRM;
    bootInfo.joystick = player;
    SetBootVoted(player, true);
}

// FUNCTION: SWBF2 0x005d0d20
float NetGame::GetBootTime()
{
    return NetComm::GetTime() +
        ((float)clientBootTurn - (float)clientTurnNumber) * netSecondsPerTurn;
}

extern int hostVoteThreshold;

// FUNCTION: SWBF2 0x005d0f10
bool NetGame::GetBootCount(int& votes, int& players, int& required)
{
    votes = 0;
    players = 0;
    const int team = GetStartTeam(GetBootPlayer());
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (IsPlaying(i, false) && GetStartTeam(i) == team)
        {
            ++players;
            if (hostBootAgreeMask & ((unsigned __int64)1 << i))
            {
                ++votes;
            }
        }
    }
    required = (int)((float)players * (float)hostVoteThreshold / 100.0f + 0.5f);
    if (required <= 0)
    {
        required = 1;
    }
    return votes >= required;
}

// FUNCTION: SWBF2 0x005d1db0
bool NetGame::AutoBootLagger()
{
    bool found = false;
    if (gBootLagger)
    {
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (IsPlaying(i, false) && NetComm::GetPing(i) > gBootLaggerPing)
            {
                NetGame::ForceBootPlayer(i);
                found = true;
            }
        }
    }
    return found;
}

// TEMPLATE: SWBF2 0x005d37f0
// BblList<NetEntity,512>::BblList<NetEntity,512>
template BblList<NetEntity, 512>::BblList();


// FUNCTION: SWBF2 0x005c0fa0
void NetGame::CreateEvent_CorrectChunkCluster(const ChunkCluster* cluster, EntityGeometryClass* entityClass,
                                              unsigned int chunkIndex, unsigned __int64 mask)
{
    if (cluster->mID == -1)
    {
        return;
    }
    if (!entityClass->mChunkDescriptors[chunkIndex])
    {
        return;
    }
    ReliableNetEvent& event = GetNextReliableEvent();
    ReliableNetEvent::U::CorrectChunkClusterEvent& data = event.u.correctchunkcluster;
    event.mEvType = REL_CORRECT_CHUNKCLUSTER;
    event.mSendMask &= mask;
    data.mEntClass = entityClass;
    data.mChunkClusterIndex = cluster->mID;
    data.mChunkIndex = chunkIndex;
}

// FUNCTION: SWBF2 0x005c1f80
void NetGame::CreateEvent_ScriptMessage(unsigned int uLocalizeKey, unsigned int uTeamMask)
{
    ReliableNetEvent& event = GetNextReliableEvent();
    event.mEvType = REL_SCRIPTMESSAGE;
    event.u.scriptMessage.mLocalizeKey = uLocalizeKey;
    event.u.scriptMessage.mTeamMask = uTeamMask;
    if (!IsEnabled())
    {
        HandleEvent_ScriptMessage(event);
    }
}

// FUNCTION: SWBF2 0x005c2050
void NetGame::CreateEvent_PlayVoiceOver(unsigned int hash, unsigned int teamMask)
{
    ReliableNetEvent& event = GetNextReliableEvent();
    event.mEvType = REL_PLAYVOICEOVER;
    event.u.playVoiceOver.mSoundNameHashID = hash;
    event.u.playVoiceOver.mTeamMask = teamMask;
    if (!IsEnabled() || (IsHost() && !IsDedicated() && IsOneWorld()))
    {
        HandleEvent_PlayVoiceOver(event);
    }
}

extern int gSpawnInvicibility;

// FUNCTION: SWBF2 0x005c2440
void NetGame::CreateEvent_UpdateInvincibility(Character* character)
{
    if (!character)
    {
        return;
    }
    ReliableNetEvent& event = GetNextReliableEvent();
    event.mEvType = REL_UPDATEINVINCIBILITY;
    event.u.updateInvincibility.mCharacter = character;
    event.u.updateInvincibility.mIsInvincible =
        (float)gSpawnInvicibility > NetComm::GetTime() - character->GetSpawnTime();
}

extern float spHeroSelectedTime[3];

// FUNCTION: SWBF2 0x005cecc0
float GetTimeSinceHeroSelected(int team)
{
    if (NetGame::IsEnabled())
    {
        return (float)(NetGame::GetTurnNumber() - netHeroSelectedTurn[team]) * netSecondsPerTurn;
    }
    else
    {
        return NetGame::GetLocalTime() - spHeroSelectedTime[team];
    }
}

void EnableFrameLock();

// FUNCTION: SWBF2 0x005d2690
void InitFrameLock()
{
    if (!NetGame::IsEnabled() || !NetGame::IsHost() || !NetGame::IsDedicated())
    {
        frameLockEnabled = true;
    }
    if (frameLockEnabled && netFrameLock)
    {
        EnableFrameLock();
    }
}

// FUNCTION: SWBF2 0x005b8840
void NetGameP::FreePackets()
{
    if (!NetGame::IsHost())
    {
        return;
    }
    for (unsigned int turn = hostTurnNumber - Min(hostTurnNumber, 3);
         turn < (unsigned int)hostTurnNumber; ++turn)
    {
        unsigned int slot = turn & 7;
        NetPktGroup& group = lateState[slot];
        if (group.GetFront())
        {
            group.Free();
            for (int i = 0; i < netCurMaxPlayers; ++i)
            {
                if (!NetGame::IsPlaying(i, false))
                {
                    continue;
                }
                if (netPlayers[i].lateFenceTurn < turn)
                {
                    netPlayers[i].lateFenceTurn = turn;
                }
            }
            break;
        }
    }
}

extern PblTimeBase frameLockTimeBase;
extern PblTimeBase::Stamp frameLockStamp;

// GLOBAL: SWBF2 0x01fa8710
static unsigned __int64 oldPerfCount;
// GLOBAL: SWBF2 0x01fa88b0
static LARGE_INTEGER perfCount;

// FUNCTION: SWBF2 0x005d2540
void AdvanceTimeBase()
{
    QueryPerformanceCounter(&perfCount);
    frameLockTimeBase.AdvanceTicks(perfCount.QuadPart - oldPerfCount);
    oldPerfCount = perfCount.QuadPart;
}

// FUNCTION: SWBF2 0x005d25a0
static void WakeMeNextFrame()
{
    AdvanceTimeBase();
    frameLockStamp.Mark(&frameLockTimeBase);
}

// FUNCTION: SWBF2 0x005d25c0
void EnableFrameLock()
{
    WakeMeNextFrame();
}

// FUNCTION: SWBF2 0x005b4300
RedTerrain::Hole::Hole(RedTerrain::Hole* pSource) : TreeGridObject(-1)
{
    SetObjectData(0, COLL_HOLE);
    SetCollisionCenter(pSource->GetCollisionCenter());
    SetCollisionRadius(pSource->GetCollisionRadius());
    iNumPlanes = pSource->iNumPlanes;
    aPlanes = new PblVector4[iNumPlanes];
    for (int i = 0; i < iNumPlanes; ++i)
    {
        aPlanes[i] = pSource->aPlanes[i];
    }
}

// FUNCTION: SWBF2 0x005b45f0
void NetNdObjMap::Add(NetNdObj* object, unsigned int hash)
{
    object->uniqId = hash;
    object->index = count;
    objMap.Store(hash, object);
    array[count++] = object;
    object->obj->SetNetUniqueId(object->index + 0x40);
}

// GLOBAL: SWBF2 0x01e64ddc
int savedHeroRuleUnlock;

// GLOBAL: SWBF2 0x01e64b80
int savedHeroRuleUnlockVal;

// GLOBAL: SWBF2 0x01e64b4c
int savedHeroRuleTeam;

// GLOBAL: SWBF2 0x01e64b90
int savedHeroRulePlayer;

// GLOBAL: SWBF2 0x01e64ef0
int savedHeroRuleRespawn;

// GLOBAL: SWBF2 0x01e64b50
int savedHeroRuleRespawnVal;

// GLOBAL: SWBF2 0x01e64de0
int savedPlayerLimit;

// GLOBAL: SWBF2 0x01e63003
bool savedTeamDamage;

// GLOBAL: SWBF2 0x01e62f53
bool savedAutoAim;

// GLOBAL: SWBF2 0x01e64b87
bool savedShowNames;

// GLOBAL: SWBF2 0x01e64b8f
bool savedAutoAssignTeams;

// GLOBAL: SWBF2 0x01e64efc
bool savedAutoAssignTeamsOriginal;

// GLOBAL: SWBF2 0x01e64a20
int savedDedicatedAutoLaunchMinPlayers;

// GLOBAL: SWBF2 0x01e64ba8
int savedHostVoteThreshold;

// GLOBAL: SWBF2 0x01e62fbc
int savedPreGameTimeMax;

// GLOBAL: SWBF2 0x01e63000
bool savedAwardsEnabled;
extern int netHeroRulePlayer;
extern int netPlayerLimit;
extern bool netAutoAim;
extern bool netShowNames;
extern bool netAutoAssignTeamsOriginalSetting;
extern bool netAwardsEnabled;

// FUNCTION: SWBF2 0x005d3380
void RestoreSessionRuleDefaults()
{
    netHeroRuleUnlock = savedHeroRuleUnlock;
    netHeroRuleUnlockVal = savedHeroRuleUnlockVal;
    netHeroRuleTeam = savedHeroRuleTeam;
    netHeroRulePlayer = savedHeroRulePlayer;
    netHeroRuleRespawn = savedHeroRuleRespawn;
    netHeroRuleRespawnVal = savedHeroRuleRespawnVal;
    netPlayerLimit = savedPlayerLimit;
    netTeamDamage = savedTeamDamage;
    netAutoAim = savedAutoAim;
    netShowNames = savedShowNames;
    netAutoAssignTeams = savedAutoAssignTeams;
    netAutoAssignTeamsOriginalSetting = savedAutoAssignTeamsOriginal;
    netMinPlayersToAutoLaunchDedicated = savedDedicatedAutoLaunchMinPlayers;
    hostVoteThreshold = savedHostVoteThreshold;
    netPreGameTimeMax = savedPreGameTimeMax;
    netAwardsEnabled = savedAwardsEnabled;
}

namespace AI
{
enum AIAffil
{
AFFIL_ENEMY = 1, AFFIL_NEUTRAL = 2,
                             AFFIL_ALLY = 4, AFFIL_ALLYNEUTRAL = 6, AFFIL_ALL = 7 }; }

// FUNCTION: SWBF2 0x005cf820
int NetGame::GetObjectsInRange(const PblVector3& position, float radius, GameObject** const found,
               int capacity, GameObject* excluded, CollisionObjectType type)
{
    int count = 0;
    const float radiusSquared = radius * radius;
    for (int i = 0; i < 64; ++i)
    {
        NetObj* entry = clientObjMap.Find(i);
        if (!entry || !entry->obj || entry->obj == excluded)
        {
            continue;
        }

        CollisionObject* info = entry->obj->GetCollisionObject();
        if (!info || info->GetCollisionType() != type)
        {
            continue;
        }
        const PblVector3& targetPosition = info->GetCollisionSphere().GetCenter();
        const PblVector3 difference = position - targetPosition;
        if (radiusSquared > SquaredLength(difference))
        {
            found[count++] = entry->obj;
            if (count >= capacity)
            {
                break;
            }
        }
    }
    return count;
}

// FUNCTION: SWBF2 0x005c9a70
void AddHoleToClient(const int* source)
{
    int* copy = (int*)::operator new(60);
    if (copy)
    {
        new (copy) RedTerrain::Hole((RedTerrain::Hole*)source);
    }
    CollisionManager::EnterClient();
    const float radius = ((const TreeGridObject*)copy)->GetCollisionRadius();
    const PblVector3& position = ((const TreeGridObject*)copy)->GetCollisionCenter();
    CollisionManager::GetTreeGrid()->Add((TreeGridObject*)copy, position, radius * 2.0f);
    CollisionManager::LeaveClient();
}

class NetEntityPropAnimated : public EntityPropAnimated
{
public:
    void AddToClient();
};
 

class NetEntityDoor : public EntityDoor
{
public:
    void AddToClient();
};
 

// FUNCTION: SWBF2 0x005b4810
void NetEntityPropAnimated::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            DeactivateThread();
            NetGame::EnterClient();
            ActivateThread(0, PER_LOCAL_TURN);
            NetGame::LeaveClient();
        }
        return;
    }
    RemoveFromScene();
    NetGame::EnterClient();

    EntityDesc desc;
    desc.mMatrix = GetMatrix();
    desc.mNameId = 0;
    desc.mPlayerId = -1;
    EntityGeometry* pGeometry = static_cast<EntityGeometry*>(GetEntityClass()->Build(desc));
    pGeometry->Init();
    pGeometry->BuildEffects();
    NetGame::LeaveClient();
}

// FUNCTION: SWBF2 0x005b4920
void NetEntityDoor::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            DeactivateThread();
            Thread::EnterClient();
            ActivateThread(0, PER_LOCAL_TURN);
            Thread::LeaveClient();
        }
        return;
    }
    RemoveFromScene();
    NetGame::EnterClient();

    EntityDesc desc;
    desc.mMatrix = GetMatrix();
    desc.mNameId = GetId();
    desc.mPlayerId = -1;
    EntityGeometry* pGeometry = static_cast<EntityGeometry*>(GetEntityClass()->Build(desc));
    pGeometry->Init();
    pGeometry->BuildEffects();
    NetGame::LeaveClient();
}

// FUNCTION: SWBF2 0x005ce650
void NetGame::HeroSelected(int team)
{
    if (!IsHost() || IsClient())
    {
        return;
    }
    if (team < 0 || team >= 3)
    {
        return;
    }
    netHeroSelectedTurn[team] = NetGame::GetTurnNumber();
    spHeroSelectedTime[team] = GetLocalTime();
    if (!IsEnabled() && netHeroNextPlayerID[team] == -1 && netHeroAIID[team] != -1)
    {
        Character* pChar = Character::GetPtr(netHeroAIID[team]);
        if (pChar && pChar->GetUnit())
        {
            GameObject* gameObj = pChar->GetUnit()->GetGameObject();
            if (pChar->GetRemote())
            {
                pChar->GetUnit()->ExitRemote();
            }
            if (pChar->GetVehicle())
            {
                pChar->GetUnit()->ExitVehicle(true, true);
            }
            gameObj->InstantVanish();
        }
    }
}

// FUNCTION: SWBF2 0x005ce780
void NetGame::HeroSpawned(int team)
{
    if (!NetGame::IsHost() || NetGame::IsClient())
    {
        return;
    }
    if (team < 0 || team >= 3)
    {
        return;
    }
    netHeroDiedTurn[team] = 0;
    netHeroSelectedTurn[team] = 0;
    netHeroSpawnedTurn[team] = NetGame::GetTurnNumber();
    if (netHeroPlayerID[team] >= 0)
    {
        if (NetGame::IsEnabled())
        {
            netHeroSndPlayHostSpawn[team] = true;
        }
        else
        {
            Character* c = Character::FindPlayer(netHeroPlayerID[team]);
            if (c)
            {
                HeroMessageDisplay::PlayHeroSoundSpawn(team);
            }
        }
    }
    else if (!NetGame::IsEnabled() && netHeroAIID[team] != -1)
    {
        Character* c = Character::GetPtr(netHeroAIID[team]);
        if (c)
        {
            HeroMessageDisplay::PlayHeroSoundSpawn(team);
        }
    }
}

// FUNCTION: SWBF2 0x005cea70
void NetGame::AcceptHero(int team, int player)
{
    if (!NetGame::IsHost() || NetGame::IsClient() || team < 0 || team >= 3)
    {
        return;
    }
    if (netHeroNextPlayerID[team] == player)
    {
        netHeroPlayerID[team] = netHeroNextPlayerID[team];
        netHeroSelectedTurn[team] = 0;
        netHeroFirstUnlocked[team] = false;
        ClearTeamDeclinedHero(team);
    }
    else if (!NetGame::IsEnabled() && netHeroAIID[team] == player)
    {
        netHeroSelectedTurn[team] = 0;
        netHeroFirstUnlocked[team] = false;
        ClearTeamDeclinedHero(team);
    }
}

// FUNCTION: SWBF2 0x005c9b20
void CreateClientCollision(bool bNetCollision)
{
    int iCount = 0;
    PblList<CollisionObject>::Itor itor;
    CollisionManager::BeginList(itor);
    while (1)
    {
        const int MAX_OBJECTS = 10;
        CollisionObject* apObjects[MAX_OBJECTS];
        int iNumObjects = MAX_OBJECTS;
        CollisionManager::ListObjects(itor, apObjects, iNumObjects);
        if (!iNumObjects)
        {
            break;
        }
        for (int i = 0; i < iNumObjects; ++i)
        {
            CollisionObject* pCollision = apObjects[i];
            GameObject* pObject = pCollision->GetGameObject();
            if (pObject)
            {
                EntityType entType = pObject->GetEntityType();
                switch (entType)
                {
                case ENT_POWERUPSTATION:
                    ++gDebugCount;
                case ENT_POST:
                case ENT_ARMED:
                case ENT_ANIMATED:
                case ENT_BUILDING:
                case ENT_TRAP:
                    if (bNetCollision)
                    {
                        new NetCollisionObject(pObject->GetCollisionObject());
                    }
                    else
                    {
                        NetNdObj* pNdObj = new NetNdObj;
                        pNdObj->obj = pObject->AddToClient();
                        pNdObj->orig = pObject;
                        clientNdObjMap.Add(pNdObj, pObject->GetHandleId());
                    }
                    ++iCount;
                    break;
                default:
                    ++gDebugCount;
                    break;
                }
            }
            else
            {
                Entity* pEntity = pCollision->GetEntity();
                if (pEntity)
                {
                    if (pEntity->IsRtti(EntityPropAnimated::GetRtti()))
                    {
                        NetEntityPropAnimated* pAnimated = (NetEntityPropAnimated*)pEntity;
                        if (bNetCollision)
                        {
                            new NetCollisionObject(pAnimated->GetCollisionObject());
                        }
                        else
                        {
                            pAnimated->AddToClient();
                        }
                        ++iCount;
                    }
                    else if (pEntity->IsRtti(EntityDoor::GetRtti()))
                    {
                        NetEntityDoor* pDoor = (NetEntityDoor*)pEntity;
                        if (bNetCollision)
                        {
                            new NetCollisionObject(pDoor->GetCollisionObject());
                        }
                        else
                        {
                            pDoor->AddToClient();
                        }
                        ++iCount;
                    }
                    else if (pEntity->IsRtti(EntityProp::GetRtti()))
                    {
                        EntityProp* pProp = (EntityProp*)pEntity;
                        if (bNetCollision)
                        {
                            new NetCollisionObject(pProp->GetCollisionObject());
                        }
                        else if (!NetGame::IsOneWorld())
                        {
                            pProp->AddToClient();
                        }
                        ++iCount;
                    }
                }
            }
        }
    }
    if (!bNetCollision)
    {
        NetGame::EnterClient();
        EntityWater* pWater = new EntityWater;
        NetGame::LeaveClient();
        if (!NetGame::IsOneWorld())
        {
            RedTerrain::ProcessHoles((void (__cdecl*)(void*))AddHoleToClient);
        }
    }
}

// FUNCTION: SWBF2 0x005b7470
bool NetGame::IsNearLocalPlayer(const PblVector3& vPos)
{
    for (int i = 0; i < 2; ++i)
    {
        float dx = vPos.GetX() - netLocalPos[i].GetX();
        float dxSq = dx * dx;
        float dz = vPos.GetZ() - netLocalPos[i].GetZ();
        float dzSq = dz * dz;
        if (dxSq < 10000.0f && dzSq < 10000.0f)
        {
            return true;
        }
    }
    return false;
}








// FUNCTION: SWBF2 0x004171d0
double AbsFloat(float fValue) { return fabs(fValue); }

// FUNCTION: SWBF2 0x005b85f0
int* NetGameP::GetHostIndex(int iPlayer) { return &netPlayers[iPlayer].hostIndex; }

// FUNCTION: SWBF2 0x005b8610
int* NetGameP::GetClientOfIndex(int iPlayer) { return &netPlayers[iPlayer].clientOfIndex; }

// FUNCTION: SWBF2 0x005b86b0
int* NetGameP::GetHostMode(int iHost) { return (int*)&netHosts[iHost]; }

// FUNCTION: SWBF2 0x005b8450
void NetGameP::SetPlaying(unsigned __int64 uMask, bool bNext)
{
    if (bNext)
    {
        hostNextPlayingMask = uMask;
    }
    else
    {
        clientPlayingMask = uMask;
    }
}

// FUNCTION: SWBF2 0x005b8560
void NetGameP::SetKey(int iPlayer, int iKey)
{
    iKey |= 0x3100ac00;
    if (iPlayer < 64)
    {
        netPlayers[iPlayer].key = iKey;
        return;
    }
    iPlayer -= 64;
    if (iPlayer < 3)
    {
        netHosts[iPlayer].key = iKey;
    }
}

// FUNCTION: SWBF2 0x005b86d0
void NetGameP::SetPlayerCanHost(int iPlayer, char bReplica, char bRelay)
{
    *(unsigned char*)&netPlayers[iPlayer].canReplica = (unsigned char)bReplica;
    *(unsigned char*)&netPlayers[iPlayer].canRelay = (unsigned char)bRelay;
}

// FUNCTION: SWBF2 0x005b8790
void NetGameP::SetTurnRate(int iDuration, int iTurns)
{
    if (iTurns)
    {
        netTurnsPerSecond = iDuration / iTurns;
    }
    else
    {
        netTurnsPerSecond = 16666;
    }
    netSecondsPerTurn = 1.0f / (float)netTurnsPerSecond;
    MAX_PREDICT_STEPS = 2 * netTurnsPerSecond / 3;
}

// FUNCTION: SWBF2 0x005c36c0
unsigned int NetGame::GetAckedUpdateTurn(int iPlayer) { return netPlayers[iPlayer].clientTurn; }

// FUNCTION: SWBF2 0x005bd430
void BeginWriteAlign(NetPktGroup& group) { alignCount = 0; }

// GLOBAL: SWBF2 0x01fa87a4
int sendObjCount;

// FUNCTION: SWBF2 0x005be380
int GetSendObjCount() { return sendObjCount; }



// FUNCTION: SWBF2 0x005caf90
void WriteMiniMove(NetPktGroup& group, ProcessedInputs* inputs)
{
    group.WriteFixed(inputs->mFloatInputs[0], -1.0f, 1.0f, 3);
    group.WriteFixed(inputs->mFloatInputs[1], -1.0f, 1.0f, 3);
    group.WriteFixed(inputs->mFloatInputs[2], -1.0f, 1.0f, 3);
    group.WriteFixed(inputs->mFloatInputs[3], -1.0f, 1.0f, 3);
}

// FUNCTION: SWBF2 0x005c1ff0
void WriteEvent_ScriptMessage(NetPktGroup& group, ReliableNetEvent& event)
{
    group.WriteBits(event.u.scriptMessage.mLocalizeKey, 32);
    group.WriteBits(event.u.scriptMessage.mTeamMask, 3);
}

// FUNCTION: SWBF2 0x005c20e0
void WriteEvent_PlayVoiceOver(NetPktGroup& group, ReliableNetEvent& event)
{
    group.WriteBits(event.u.playVoiceOver.mSoundNameHashID, 32);
    group.WriteBits(event.u.playVoiceOver.mTeamMask, 3);
}

// FUNCTION: SWBF2 0x005c21d0
void WriteEvent_UpdateDoorState(NetPktGroup& group, ReliableNetEvent& event)
{
    group.WriteBits(event.u.updateDoorState.mHashID, 32);
    group.Write(event.u.updateDoorState.mIsLocked);
}

// FUNCTION: SWBF2 0x005c23c0
void WriteEvent_UpdateObjective(NetPktGroup& group, ReliableNetEvent& event)
{
    group.WriteBits(event.u.updateObjective.mHashID, 32);
    group.WriteBits(event.u.updateObjective.state, 8);
}

// FUNCTION: SWBF2 0x005c39a0
void NetGame::WriteSpectator(NetPktGroup& group)
{
    for (int iPlayer = 0; iPlayer < 64; ++iPlayer)
    {
        int spectator = GetSpectator(iPlayer) + 1;
        group.WriteBits(spectator, 6);
    }
}

// FUNCTION: SWBF2 0x005c39f0
void NetGame::ReadSpectator(NetPktGroup& group)
{
    for (int iPlayer = 0; iPlayer < 64; ++iPlayer)
    {
        unsigned int uEncoded = 0;
        group.ReadBits(uEncoded, 6);
        int iSpectator = (int)uEncoded - 1;
        SetSpectator(iPlayer, iSpectator);
        NetGame::ChangeSpectateTrack(iPlayer, iSpectator);
    }
}

// FUNCTION: SWBF2 0x005ba0d0
bool IsLateObject(const GameObject* pObject)
{
    for (unsigned int i = 0; i < lateObjCount; ++i)
    {
        if (lateObjects[i] == pObject)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005ba570
void NetWaitLate(bool bWait)
{
    netWaitLate = bWait;
    if (netWaitLate)
    {
        for (int i = 0; i < 8; ++i)
        {
            NetPktGroup& pktGroup = lateState[i];
            pktGroup.Free();
        }
    }
    else
    {
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetGame::IsPlaying(i, false))
            {
                NetGame::AddLateFence(i);
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c4700
int OpenJournalFile(char* fileName)
{
    int fileSize = -1;
    return fileSize;
}

// FUNCTION: SWBF2 0x005c4730
void CloseJournalFile() {}

// FUNCTION: SWBF2 0x005c4740
void DeleteJournalFile(const char* fileName) {}

// FUNCTION: SWBF2 0x005c4750
void SyncJournalChanges() {}

// FUNCTION: SWBF2 0x0043cf60
bool ReadNetIsLocalTurn() { return netIsLocalTurn; }

// FUNCTION: SWBF2 0x005b4470
bool IsSkippedND(const GameObject* pObject)
{
    unsigned int netId = pObject->GetNetUniqueId() - 64;
    unsigned int word = netId >> 5;
    unsigned int bit = netId % 32;
    return (ndSkippedMask[word] & (1u << bit)) ? true : false;
}

// FUNCTION: SWBF2 0x005be3a0
void PredictTurn()
{
    int iTurn = predictTurnNumber & 31;
    for (int iPlayer = 0; iPlayer < netCurMaxPlayers; ++iPlayer)
    {
        if (NetGame::IsPlaying(iPlayer, false))
        {
            int iLocal = NetComm::GetJoystickIndex(iPlayer);
            if (iLocal >= 0)
            {
                *hostJoysticks[iPlayer] = localClientTurns[iTurn][iLocal];
            }
            else
            {
                *hostJoysticks[iPlayer] = remoteClientTurns.moves[iPlayer];
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c5450
void AloReinforcements::Set(int iTeam, int iCount, int iActive,
                                   float fBleed, int iPoints)
{
    if (iCount < 0)
    {
        iCount = 0;
    }
    bool changed = iCount != teamCount[iTeam] || iActive < teamActive[iTeam] ||
        fBleed != teamBleed[iTeam] || iPoints != teamPoints[iTeam];
    if (changed)
    {
        Change();
        teamCount[iTeam] = iCount;
        teamActive[iTeam] = iActive;
        teamBleed[iTeam] = fBleed;
        teamPoints[iTeam] = iPoints;
    }
}

// FUNCTION: SWBF2 0x005c6650
void AloMiniStats::Set(unsigned int uEnemyKills, unsigned int uTeamKills,
                              unsigned int uDeaths, unsigned int uCps, short iPoints)
{
    if (uEnemyKills != mEnemyKills[curDst] || uTeamKills != mTeamKills[curDst] ||
        uDeaths != mDeaths[curDst] || uCps != mCps[curDst] || iPoints != mPoints[curDst])
    {
        Change();
        mEnemyKills[curDst] = uEnemyKills;
        mTeamKills[curDst] = uTeamKills;
        mDeaths[curDst] = uDeaths;
        mCps[curDst] = uCps;
        mPoints[curDst] = iPoints;
    }
}

// FUNCTION: SWBF2 0x005c57d0
void AloHeroData::Set()
{
    bool changed = netHeroUnlocked[mTeamNum] != heroUnlocked ||
        netHeroPlayerID[mTeamNum] != heroPlayerID ||
        netHeroNextPlayerID[mTeamNum] != nextHeroPlayerID ||
        netHeroLastPlayerID[mTeamNum] != lastHeroPlayerID ||
        netHeroKillerID[mTeamNum] != heroKillerID ||
        netHeroSelectedTurn[mTeamNum] != heroSelectedTurn ||
        (unsigned int)netHeroSpawnedTurn[mTeamNum] != heroSpawnedTurn ||
        netHeroDiedTurn[mTeamNum] != heroDiedTurn;
    if (changed)
    {
        Change();
        heroUnlocked = netHeroUnlocked[mTeamNum];
        heroPlayerID = netHeroPlayerID[mTeamNum];
        nextHeroPlayerID = netHeroNextPlayerID[mTeamNum];
        lastHeroPlayerID = netHeroLastPlayerID[mTeamNum];
        heroKillerID = netHeroKillerID[mTeamNum];
        heroSelectedTurn = netHeroSelectedTurn[mTeamNum];
        heroSpawnedTurn = netHeroSpawnedTurn[mTeamNum];
        heroDiedTurn = netHeroDiedTurn[mTeamNum];
    }
}



inline bool AtleastOnceUniq::IsConfirmed()
{
    if (changeTurn[curDst] == (unsigned int)-1)
    {
        return true;
    }
    if (netPlayers[curDst].clientTurn < changeTurn[curDst])
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x005c50f0
bool AtleastOnceUniq::Send(NetPktGroup& group)
{
    if (!isFullState && IsConfirmed())
    {
        return false;
    }
    group.WriteBits(sig, 4);
    return true;
}

// FUNCTION: SWBF2 0x005c5170
bool AtleastOnceUniq::CanChange()
{
    if (!deltaSends)
    {
        return true;
    }
    if (IsConfirmed())
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x005c51e0
bool AtleastOnceSame::Send(NetPktGroup& group)
{
    if (!isFullState && IsConfirmed(curDst))
    {
        return false;
    }
    group.WriteBits(sig, 4);
    return true;
}

// FUNCTION: SWBF2 0x005bc160
void WriteNullObject(NetPktGroup& group)
{
    group.Write(false);
    if (!isFullState)
    {
        group.WriteBits(63, 6);
    }
    else
    {
        group.WriteBits(0, 16);
    }
}

// FUNCTION: SWBF2 0x005c3fe0
void NetGame::WritePreGame(NetPktGroup& group)
{
    unsigned int bits = netPreGameTime;
    group.Write(netPreGameEnabled);
    group.WriteBits(bits, 10);
}

// FUNCTION: SWBF2 0x005c4ff0
void WriteUploadAck(NetPktGroup& group)
{
    if (curDst != uploadSrc)
    {
        group.Write(false);
        return;
    }
    group.Write(true);
    group.WriteBits(uploadSeqNo, 32);
}

// FUNCTION: SWBF2 0x005cb5c0
void UpdateCurMaxHosts(unsigned int uMask, bool bIncludeCurrent)
{
    if (bIncludeCurrent)
    {
        uMask |= 1u << (netCurMaxHosts - 1);
    }
    netCurMaxHosts = 1;
    for (int i = 2; i >= 0; --i)
    {
        if (uMask & (1u << i))
        {
            netCurMaxHosts = i + 1;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x005c5520
void AloReinforcements::Write(NetPktGroup& group)
{
    for (int i = 0; i < 2; ++i)
    {
        group.WriteBits(teamCount[i], 10);
        group.WriteBits(teamActive[i], 6);
        if (teamBleed[i] > 0.0f)
        {
            group.Write(true);
            group.WriteFixed(teamBleed[i], 0.0f, 8.0f, 5);
        }
        else
        {
            group.Write(false);
        }
        group.WriteBits(teamPoints[i] + 128, 10);
    }
}

// FUNCTION: SWBF2 0x005c5960
void AloHeroData::Write(NetPktGroup& group)
{
    group.WriteBits(mTeamNum, 2);
    if (netHeroesEnabled)
    {
        group.Write(heroUnlocked);
        group.WriteBits(heroPlayerID + 1, 7);
        group.WriteBits(nextHeroPlayerID + 1, 7);
        group.WriteBits(lastHeroPlayerID + 1, 7);
        group.WriteBits(heroKillerID + 1, 7);
        group.WriteBits(heroSelectedTurn, 19);
        group.WriteBits(heroSpawnedTurn, 19);
        group.WriteBits(heroDiedTurn, 19);
    }
}

// FUNCTION: SWBF2 0x005c6740
void AloMiniStats::Write(NetPktGroup& group)
{
    group.WriteBits(mEnemyKills[curDst], 12);
    group.WriteBits(mTeamKills[curDst], 8);
    group.WriteBits(mDeaths[curDst], 8);
    group.WriteBits(mCps[curDst], 16);
    group.WriteBits(mPoints[curDst], 16);
}

// FUNCTION: SWBF2 0x005c6b00
void AloTrackable::Set(Trackable* pTrackable, Trackable* const* ppOther)
{
    bool same = pTrackable == mTrackable[curDst][0];
    for (unsigned int i = 0; i < 1; ++i)
    {
        if (ppOther[i] != mTrackable[curDst][i + 1])
        {
            same = false;
        }
    }
    if (same)
    {
        return;
    }
    Change();
    mTrackable[curDst][0] = pTrackable;
    for (unsigned int i = 0; i < 1; ++i)
    {
        mTrackable[curDst][i + 1] = ppOther[i];
    }
}

// FUNCTION: SWBF2 0x005c7660
void AloMissionTimer::Set(bool bValid, float fTime, float fRate,
                                  bool bActive, int iShow, int iTeam)
{
    bool changed = timerValid != bValid;
    if (bValid)
    {
        changed = changed || timerTime != fTime || timerRate != fRate ||
            timerActive != bActive || timerShow != iShow || timerTeam != iTeam;
    }
    if (changed)
    {
        Change();
        timerValid = bValid;
        timerTime = fTime;
        timerRate = fRate;
        timerActive = bActive;
        timerShow = iShow;
        timerTeam = iTeam;
    }
}

// FUNCTION: SWBF2 0x005c4650
void GetJournalName(char* pBuffer, int& iFile, int& iKind, unsigned int uBufferSize)
{
    if (iKind > 4)
    {
        iKind = 0;
        ++iFile;
    }
    if (iKind >= 2)
    {
        char c = iKind == 2 ? 'f' : iKind == 3 ? 'h' : 'l';
        _snprintf(pBuffer, uBufferSize, ".\\values_%c_%d.dat", c, iFile);
    }
    else
    {
        _snprintf(pBuffer, uBufferSize, ".\\journal_%c_%d.dat",
                  iKind ? 'm' : 'f', iFile);
    }
}

// FUNCTION: SWBF2 0x005c0950
void WriteEvent_ChangeCommandPostTeams(NetPktGroup& group, ReliableNetEvent& event)
{
    ReliableNetEvent::U::ChangeCommandPostTeamsEvent& data = event.u.changecommandpostteams;
    group.WriteBits(data.mTeamBits, 32);
    group.WriteBits(data.mAliveBits, 16);
}

// FUNCTION: SWBF2 0x005c7760
void AloMissionTimer::Write(NetPktGroup& group)
{
    group.Write(timerValid);
    if (timerValid)
    {
        group.WriteFixed(timerTime, 0.0f, 4096.0f, 16);
        if (timerRate == 1.0f)
        {
            group.Write(false);
        }
        else
        {
            group.Write(true);
            group.WriteFixed(timerRate, -16.0f, 16.0f, 8);
        }
        group.Write(timerActive);
        unsigned int show = timerShow + 1;
        group.WriteBits(show, 2);
        group.WriteBits(timerTeam, 3);
    }
}

// FUNCTION: SWBF2 0x005c24b0
void WriteEvent_UpdateInvincibility(NetPktGroup& group, ReliableNetEvent& event)
{
    group.WriteBits(event.u.updateInvincibility.mCharacter->GetPlayerId(), 7);
    group.Write(event.u.updateInvincibility.mIsInvincible);
}

// FUNCTION: SWBF2 0x005b8700
void NetGameP::SetClientTurnNumber(unsigned int uTurn)
{
    if (uTurn > 0)
    {
        clientTurnNumber = uTurn;
        sendTurnNumber = clientTurnNumber - 1;
        predictLocalTurnNumber = uTurn;
        updateTurnNumber = uTurn;
        clientLag = 0;
        timeScale = 1.0f;
        pendingTimeAdjust = 0.0f;
        adjustTime = NetComm::GetTime() + 1.0f;
        timeStableCount = 0;
        SetTurnHighBits(uTurn);
    }
}

// FUNCTION: SWBF2 0x005bafa0
void WriteSpectatorInfo(NetPktGroup& group, int iPlayer)
{
    if (iPlayer >= 0)
    {
        int spectator = NetGame::GetSpectator(iPlayer) + 1;
        group.WriteBits(spectator, 6);
        for (unsigned int slot = 0; slot < 1; ++slot)
        {
            int guest = NetGame::GetMyGuest(iPlayer, slot);
            if (guest >= 0)
            {
                spectator = NetGame::GetSpectator(guest) + 1;
                group.WriteBits(spectator, 6);
            }
        }
        MarkWriteAlign(group);
    }
}

// GLOBAL: SWBF2 0x01fa65b4
GameObject** sendObjList;

// FUNCTION: SWBF2 0x005be2f0
void AddSendFull(GameObject* pObject)
{
    EntityType entType = pObject->GetEntityType();
    switch (entType)
    {
    case ENT_SOLDIER:
    case ENT_FLYER:
    case ENT_WALKER:
    case ENT_TAUNTAUN:
    case ENT_DROID:
    case ENT_HOVER:
    case ENT_MINE:
    case ENT_POWERUP:
    case ENT_ARMED_DYNAMIC:
    case ENT_POST_HOVER:
    case ENT_POST_WALKER:
    case ENT_POST_FLYER:
    case ENT_ARMED:
    case ENT_ASTEROID:
    case ENT_FLAG:
        break;
    default:
        return;
    }
    sendObjList[sendObjCount++] = pObject;
}

// FUNCTION: SWBF2 0x005d09c0
void WriteSuicideRequest(NetPktGroup& group)
{
    for (int i = 0; i < 2; ++i)
    {
        bool bSend = NetComm::GetLocalIndex(i) >= 0 &&
                     suicideInfo[i].changedTurn >
                         (unsigned int)clientAckedTurnNumber;
        group.Write(bSend);
        if (bSend)
        {
            group.WriteBits(suicideInfo[i].changeCount & 0xff, 8);
            group.Write(suicideInfo[i].killMeNow);
        }
    }
}

// FUNCTION: SWBF2 0x005c72c0
void AloBootVote::Update(NetPktGroup& group)
{
    if (hostBootPlayer >= 0)
    {
        unsigned int bits = hostBootReason;
        group.WriteBits(7, 4);
        group.WriteBits(hostBootPlayer, 6);
        group.WriteBits(bits, BOOT_REASON_BITS);
        group.WriteBits(hostBootTurn, 32);
        int iVotes = 0;
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (!NetGame::IsPlaying(i, false))
            {
                continue;
            }
            if (hostBootAgreeMask & (1ull << i))
            {
                ++iVotes;
            }
        }
        group.WriteBits(iVotes, 6);
        if (NetGame::IsEnabled() && NetGame::IsOneWorld() && NetGame::IsHost())
        {
            clientBootPlayer = hostBootPlayer;
            clientBootReason = hostBootReason;
            clientBootTurn = hostBootTurn;
            clientBootVotes = iVotes;
        }
    }
    else if (NetGame::IsEnabled() && NetGame::IsOneWorld() &&
             NetGame::IsHost() && clientBootPlayer != -1)
    {
        clientBootPlayer = -1;
        clientBootReason = 0;
        clientBootTurn = 0;
        clientBootVotes = 0;
    }
}

void WriteVelocity(NetPktGroup& group, const PblVector3& value,
                float fMaxSpeed, int iBits);
void ReadVelocity(NetPktGroup& group, PblVector3& value,
                float fMaxSpeed, int iBits);

// FUNCTION: SWBF2 0x005c2700
void Asteroid::WriteCreate(NetPktGroup& group)
{
    WriteVelocity(group, mVelocity, mClass->mMaxSpeed, 32);
    WriteVelocity(group, mOmega, mClass->mMaxOmega, 32);
}

// FUNCTION: SWBF2 0x005c2760
void Asteroid::ReadCreate(NetPktGroup& group)
{
    ReadVelocity(group, mVelocity, mClass->mMaxSpeed, 32);
    ReadVelocity(group, mOmega, mClass->mMaxOmega, 32);
}

// FUNCTION: SWBF2 0x005c2850
void CommandHover::WriteCreate(NetPktGroup& group)
{
    GameObject::WriteCreate(group);
}

// FUNCTION: SWBF2 0x005c2870
void CommandHover::ReadCreate(NetPktGroup& group)
{
    GameObject::ReadCreate(group);
}

// FUNCTION: SWBF2 0x005c2890
void CommandHover::WriteObject(NetPktGroup& group, NetObjState* pState)
{
    EntityHover::WriteObject(group, pState);
}

// FUNCTION: SWBF2 0x005c28b0
void CommandHover::ReadObject(NetPktGroup& group, NetObjState* pState)
{
    EntityHover::ReadObject(group, pState);
}

// FUNCTION: SWBF2 0x005c28d0
void CommandHover::SkipReadObject(NetObjState*)
{
}

// FUNCTION: SWBF2 0x005c27c0
void CommandWalker::WriteCreate(NetPktGroup& group)
{
    GameObject::WriteCreate(group);
}

// FUNCTION: SWBF2 0x005c27e0
void CommandWalker::ReadCreate(NetPktGroup& group)
{
    GameObject::ReadCreate(group);
}

// FUNCTION: SWBF2 0x005c2800
void CommandWalker::WriteObject(NetPktGroup& group, NetObjState* pState)
{
    EntityWalker::WriteObject(group, pState);
}

// FUNCTION: SWBF2 0x005c2820
void CommandWalker::ReadObject(NetPktGroup& group, NetObjState* pState)
{
    EntityWalker::ReadObject(group, pState);
}

// FUNCTION: SWBF2 0x005c2840
void CommandWalker::SkipReadObject(NetObjState*)
{
}

// GLOBAL: SWBF2 0x01fa88c4
float* sendScoreList;
// GLOBAL: SWBF2 0x01fa88b8
GameObject* sendLateObj;


// FUNCTION: SWBF2 0x005bd4f0
void ResetSend(GameObject** ppObjects, const PblVector3& center,
                const PblVector3& front, float* pPriorities)
{
    sendObjCount = 0;
    sendObjList = ppObjects;
    sendCenter = center;
    sendFront = Normalize(PblVector3(front.GetX(), 0.0f, front.GetZ()));
    sendScoreList = pPriorities;
    sendLateObj = 0;
}

extern int netLagVersion;

// FUNCTION: SWBF2 0x005c8260
int GetLag(int src)
{
    if (netHosts[netHostIndex].hostMode == 0)
    {
        NetPlayer& netPlayer = netPlayers[src];
        int n = netPlayer.lagSample - 1;
        if (n < 0)
        {
            n = 19;
        }
        int lastTurn = netPlayer.lagTurns[n];
        switch (netLagVersion)
        {
        default:
            n = (int)((netPlayer.avgLag + netPlayer.errLag) * (float)netTurnsPerSecond);
            return n;
        case 1:
        {
            n = 0;
            int maxLag[3] = { -128, -128, -128 };
            for (unsigned int i = 0; i < 20; ++i)
            {
                int age = lastTurn - netPlayer.lagTurns[i];
                if (netPlayer.lagTurns[i] > 0 && age <= netTurnsPerSecond)
                {
                    ++n;
                    int lagSample = netPlayer.lagSamples[i];
                    unsigned int j;
                    for (j = 0; j < 3; ++j)
                    {
                        if (lagSample > maxLag[j])
                        {
                            break;
                        }
                    }
                    if (j < 3)
                    {
                        for (unsigned int k = 2; k > j; --k)
                        {
                            maxLag[k] = maxLag[k - 1];
                        }
                        maxLag[j] = lagSample;
                    }
                }
            }
            if (n == 0)
            {
                return 0;
            }
            if (n > 3)
            {
                n = 3;
            }
            n = maxLag[n - 1];
            return n;
        }
        case 2:
        {
            int total = 0;
            n = 0;
            for (unsigned int i = 0; i < 20; ++i)
            {
                int age = lastTurn - netPlayer.lagTurns[i];
                if (netPlayer.lagTurns[i] > 0 && age < netTurnsPerSecond)
                {
                    ++n;
                    total += netPlayers[src].lagSamples[i];
                }
            }
            if (n == 0)
            {
                return 0;
            }
            int avg = total / n;
            return avg;
        }
        case 3:
        {
            int maxLag = -128;
            for (unsigned int i = 0; i < 20; ++i)
            {
                int age = lastTurn - netPlayer.lagTurns[i];
                if (netPlayer.lagTurns[i] > 0 && age < netTurnsPerSecond &&
                    netPlayers[src].lagSamples[i] > maxLag)
                {
                    maxLag = netPlayers[src].lagSamples[i];
                }
            }
            if (maxLag == -128)
            {
                return 0;
            }
            return maxLag;
        }
        case 4:
            n = (int)((float)netTurnsPerSecond * netPlayer.avgLag);
            return n;
        }
    }
    else
    {
        return netPlayers[src].lagSamples[0];
    }
}





// FUNCTION: SWBF2 0x005c8590
void AdjustClock()
{
    if (netHostIndex >= 0 && !netHosts[netHostIndex].hostMode)
    {
        return;
    }
    float now = NetComm::GetTime();
    // GLOBAL: SWBF2 0x01faa6ac
    static unsigned int adjustStartTurn;
    if (now >= adjustTime)
    {
        if (timeScale != 1.0f)
        {
            timeScale = 1.0f;
            pendingTimeAdjust = 0.0f;
            finishedTimeScale = true;
            adjustTime = now + 0.5f;
            if (clientLag > 0)
            {
                timeStableCount -= 5;
            }
            else
            {
                int diffClamp;
                if (timeStableCount < 5)
                {
                    diffClamp = 3;
                }
                else if (timeStableCount < 10)
                {
                    diffClamp = 2;
                }
                else if (timeStableCount < 15)
                {
                    diffClamp = 1;
                }
                else
                {
                    diffClamp = 0;
                }
                if (clientLag < -diffClamp)
                {
                    timeStableCount -= 5;
                }
            }
            if (timeStableCount < 0)
            {
                timeStableCount = 0;
            }
            unsigned int adjustTurns = clientTurnNumber - adjustStartTurn;
        }
        else
        {
            int diff = clientLag;
            int diffClamp;
            if (timeStableCount < 5)
            {
                diffClamp = 3;
            }
            else if (timeStableCount < 10)
            {
                diffClamp = 2;
            }
            else if (timeStableCount < 15)
            {
                diffClamp = 1;
            }
            else
            {
                diffClamp = 0;
            }
            if (diff < -diffClamp || diff > 0)
            {
                int maxDiff = 2 * netTurnsPerSecond / 3;
                if (diff <= -netTurnsPerSecond / 2 ||
                    diff >= netTurnsPerSecond / 2)
                {
                    timeLocked = false;
                    timeStableCount = 0;
                }
                else if (timeLocked)
                {
                    maxDiff = 1;
                }
                diff = Clamp(diff, -maxDiff, maxDiff);
                diff = netTurnsPerSecond + diff;
                timeScale = (float)diff / (float)netTurnsPerSecond;
                adjustTime = now + 1.0f;
                adjustStartTurn = clientTurnNumber;
            }
            else
            {
                adjustTime = now + 1.0f;
                ++timeStableCount;
                if (timeStableCount >= 5)
                {
                    timeLocked = true;
                }
            }
            clientLag = 0;
        }
    }
}

// FUNCTION: SWBF2 0x005cc600
void GetLastObjState(GameObject* pObject, NetObjState*& pState, unsigned int& uId)
{
    HandleToIndex* pEntry = handleToIndexMapPtr->Find(pObject->GetHandleId());
    uId = pEntry->index;
    if (!pEntry->isNew)
    {
        pState = netPlayers[curDst].lastObjState->Find(uId);
    }
}

// FUNCTION: SWBF2 0x005ce340
void UpdateHostStats()
{
    if (NetComm::GetTime() > hostStats.nextTime)
    {
        hostStats.nextTime = NetComm::GetTime() + 10.0f;
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetGame::IsPlaying(i, false) &&
                netPlayers[i].clientOfIndex == netHostIndex)
            {
                unsigned int& updateCount = netPlayers[i].updateCount;
                unsigned int& lastUpdateCount = hostStats.players[i].lastUpdateCount;
                int updateDiff = updateCount - lastUpdateCount;
                unsigned int& filledCount = netPlayers[i].filledCount;
                unsigned int& lastFilledCount = hostStats.players[i].lastFilledCount;
                int filledDiff = filledCount - lastFilledCount;
                lastUpdateCount = updateCount;
                lastFilledCount = filledCount;
                if (netPlayers[i].packetCount > 1000)
                {
                    netPlayers[i].packetCount >>= 1;
                    netPlayers[i].dropCount >>= 1;
                }
            }
        }
    }
}

void WriteMatrix(NetPktGroup& group, const PblMatrix& matrix);
void WritePositionVector(NetPktGroup& group, const PblVector3& value);

// FUNCTION: SWBF2 0x005c1020
void WriteEvent_CorrectChunkCluster(NetPktGroup& group, ReliableNetEvent& event)
{
    ReliableNetEvent::U::CorrectChunkClusterEvent& data = event.u.correctchunkcluster;
    ChunkCluster* cluster =
        ChunkClusterManager::GetClusterByID((unsigned char)data.mChunkClusterIndex, false);
    bool hasData = cluster != 0;
    group.Write(hasData);
    if (hasData)
    {
        group.WriteBits(data.mChunkClusterIndex, 4);
        group.WriteBits(data.mChunkIndex, 4);
        WriteEntityClass(group, data.mEntClass);
        WriteMatrix(group, cluster->mChunks[data.mChunkIndex].GetMatrix());
    }
}

// FUNCTION: SWBF2 0x005c0b90
void WriteEvent_CreateChunkCluster(NetPktGroup& group, ReliableNetEvent& event)
{
    ReliableNetEvent::U::CreateChunkClusterEvent& data = event.u.createchunkcluster;
    ChunkCluster* cluster =
        ChunkClusterManager::GetClusterByID((unsigned char)data.mChunkClusterIndex, false);
    bool hasData = cluster != 0;
    group.Write(hasData);
    if (hasData)
    {
        unsigned int classIndex = data.mEntClass->GetNetIndex();
        group.WriteBits(data.mChunkClusterIndex, 4);
        WriteEntityClass(group, data.mEntClass);
        EntityGeometryClass* entityClass = classIndex < 0xff
            ? static_cast<EntityGeometryClass*>(Factory<Entity, EntityClass, EntityDesc>::FindNetIndex(classIndex)) : 0;
        if (entityClass)
        {
            for (int i = 0; i < 10; ++i)
            {
                if (entityClass->mChunkDescriptors[i])
                {
                    WriteMatrix(group, cluster->mChunks[i].GetMatrix());
                    WritePositionVector(group, cluster->mChunks[i].GetVelocity());
                    WritePositionVector(group, cluster->mChunks[i].GetOmega());
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005c4140
void NetGame::VanishAllPlayer()
{
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetGame::IsPlaying(i, false))
        {
            Character* pChar = Character::GetPtr(i);
            if (pChar && pChar->GetUnit())
            {
                GameObject* gameObj = pChar->GetUnit()->GetGameObject();
                if (pChar->GetRemote())
                {
                    pChar->GetUnit()->ExitRemote();
                }
                if (pChar->GetVehicle())
                {
                    pChar->GetUnit()->ExitVehicle(true, true);
                }
                gameObj->InstantVanish();
            }
        }
    }
}

// GLOBAL: SWBF2 0x01faa6d8
unsigned char spectatorTargetChanged[64];

// FUNCTION: SWBF2 0x005c3db0
void NetGame::SetNextSpectator(int iPlayer, int iSpectator, ProcessedInputs* pState)
{
    bool switchSpectator = false;
    Character* character = Character::FindPlayer(iSpectator);
    if (!character || !character->GetUnit() || pState->IsPrimaryFireButtonDown())
    {
        switchSpectator = true;
    }
    if (switchSpectator)
    {
        if (!spectatorTargetChanged[iPlayer])
        {
            int next = NetGame::FindSpectateTarget(iPlayer);
            if (NetGame::IsSpawning(iPlayer))
            {
                Character* playerCharacter = Character::FindPlayer(iPlayer);
                next = NetGame::FindSpectateTarget(iSpectator, playerCharacter->GetTeamNumber());
            }
            if (next != iSpectator)
            {
                NetGame::SetSpectator(iPlayer, next);
                spectatorTargetChanged[iPlayer] = 1;
            }
        }
    }
    else
    {
        spectatorTargetChanged[iPlayer] = 0;
    }
}

// FUNCTION: SWBF2 0x005cfd70
void WriteLockedTargets(NetPktGroup& group)
{
    for (int i = 0; i < 2; ++i)
    {
        LockOnManager* lockOnManager = LockOnManager::GetByPlayerIndex(i);
        if (lockOnManager)
        {
            if (lockOnManager->GetTargetLockedChangedTurn() +
                    Max(netTurnsPerSecond / 3, 3) > clientAckedTurnNumber &&
                !(NetGame::IsOneWorld() && NetGame::IsHost()))
            {
                group.Write(true);
                GameObject* gameObj = lockOnManager->GetLockedTarget();
                if (gameObj)
                {
                    group.Write(true);
                    EntityType entType = gameObj->GetEntityType();
                    bool neverDies = IsNeverDies(entType);
                    group.Write(neverDies);
                    if (neverDies)
                    {
                        unsigned int uniqId;
                        for (uniqId = 0; uniqId < clientNdObjMap.count; ++uniqId)
                        {
                            NetNdObj* netNdObj = clientNdObjMap.FindIndex(uniqId);
                            GameObject* obj = netNdObj ? netNdObj->obj : 0;
                            if (obj == gameObj)
                            {
                                break;
                            }
                        }
                        group.WriteBits(uniqId, 8);
                    }
                    else
                    {
                        unsigned int uniqId = gameObj->GetNetUniqueId();
                        group.WriteBits(uniqId, 6);
                    }
                }
                else
                {
                    group.Write(false);
                }
            }
            else
            {
                group.Write(false);
            }
        }
        else
        {
            group.Write(false);
        }
    }
}

// FUNCTION: SWBF2 0x005c1c30
void WriteEvent_DestroyTowCable(NetPktGroup& group, ReliableNetEvent& event)
{
    ReliableNetEvent::U::DestroyTowCableEvent& data = event.u.destroyTowCable;
    const GameObject* owner = 0;
    data.mOwnerObject.Get(owner);
    unsigned int objectId = GetUniqObjectId(owner);
    group.WriteBits(objectId, GetUniqObjectBits());
    if (data.mFireWeapon && owner)
    {
        group.WriteBits(1, 1);
        Controllable* ownerControl = data.mFireWeapon->mOwner;
        PblList<Controllable>::Itor controlItor(owner->GetControllableList());
        int controlIndex;
        for (controlIndex = 0; controlItor; ++controlIndex, ++controlItor)
        {
            if (controlItor == ownerControl)
            {
                break;
            }
        }
        group.WriteBits(controlIndex, 3);
        int weaponIndex;
        for (weaponIndex = 0; weaponIndex < ownerControl->GetWeaponCount(); ++weaponIndex)
        {
            if (data.mFireWeapon == ownerControl->GetWeaponPtr(weaponIndex))
            {
                break;
            }
        }
        group.WriteBits(weaponIndex, 2);
    }
    else
    {
        group.WriteBits(0, 1);
    }
}

void WriteUnitVector(NetPktGroup& group, const PblVector3& value);

// FUNCTION: SWBF2 0x005c1390
void WriteEvent_ConnectTowCable(NetPktGroup& group, ReliableNetEvent& event)
{
    ReliableNetEvent::U::ConnectTowCableEvent& data = event.u.connectTowCable;
    PblVector3 vector;
    vector = data.mDirection.Get();
    WriteUnitVector(group, vector);

    const GameObject* owner = 0;
    data.mOwnerObject.Get(owner);
    unsigned int objectId = GetUniqObjectId(owner);
    group.WriteBits(objectId, GetUniqObjectBits());
    unsigned int collideId = GetUniqObjectId(data.mCollideWalker);
    group.WriteBits(collideId, GetUniqObjectBits());
    if (data.mFireWeapon && owner)
    {
        group.WriteBits(1, 1);
        Controllable* ownerControl = data.mFireWeapon->mOwner;
        PblList<Controllable>::Itor controlItor(owner->GetControllableList());
        int controlIndex;
        for (controlIndex = 0; controlItor; ++controlIndex, ++controlItor)
        {
            if (controlItor == ownerControl)
            {
                break;
            }
        }
        group.WriteBits(controlIndex, 3);
        int weaponIndex;
        for (weaponIndex = 0; weaponIndex < ownerControl->GetWeaponCount(); ++weaponIndex)
        {
            if (data.mFireWeapon == ownerControl->GetWeaponPtr(weaponIndex))
            {
                break;
            }
        }
        group.WriteBits(weaponIndex, 2);
    }
    else
    {
        group.WriteBits(0, 1);
    }
    group.WriteBits(data.mCollideBodyID, 7);
    vector = data.mCollidePosition.Get();
    WritePositionVector(group, vector);
}




// FUNCTION: SWBF2 0x005d2260
void WriteTurnLowBits(NetPktGroup& group, unsigned int uTurn)
{
    bool bLast = false;
    if (uTurn != (unsigned int)-1)
    {
        unsigned int maskedTurn = uTurn & 0x7ff;
        if (maskedTurn == 0x7ff)
        {
            bLast = false;
            uTurn = (unsigned int)-2;
        }
        else if (maskedTurn == 0x7fe)
        {
            bLast = true;
            uTurn = (unsigned int)-2;
        }
    }
    group.WriteBits(uTurn, 11);
    if (uTurn == (unsigned int)-2)
    {
        group.Write(bLast);
    }
}

// FUNCTION: SWBF2 0x005c3320
void WriteSwitchResponse(NetPktGroup& group, SwitchResponse response, unsigned int turn)
{
    bool canSwitch = false;
    switch (response)
    {
    case SR_NONE:
        turn = (unsigned int)-1;
        break;
    case SR_CAN:
        canSwitch = true;
        break;
    }
    group.Write(canSwitch);
    WriteTurnLowBits(group, turn);
}

// FUNCTION: SWBF2 0x005d1e20
void SpeedGame()
{
}

// FUNCTION: SWBF2 0x005d0c70
void NetGame::NominateBootPlayer(int iPlayer, int iReason, int iJoystick)
{
    bootInfo.changedTurn = clientTurnNumber;
    bootInfo.dst = iPlayer;
    bootInfo.reason = iReason;
    bootInfo.joystick = iJoystick;
    bootInfo.bootRequest = BR_NOMINATE;
    NetGame::SetBootVoted(iJoystick, true);
}

// FUNCTION: SWBF2 0x005d1180
void WriteBootRequest(NetPktGroup& group)
{
    bool bPending = bootInfo.changedTurn > (unsigned int)clientAckedTurnNumber;
    group.Write(bPending);
    if (bPending)
    {
        bool bNomination = bootInfo.bootRequest == BR_NOMINATE;
        group.Write(bNomination);
        group.WriteBits(bootInfo.joystick, BOOT_JOYSTICK_BITS);
        if (bNomination)
        {
            group.WriteBits(bootInfo.dst, 6);
            group.WriteBits(bootInfo.reason, BOOT_REASON_BITS);
        }
    }
}

void WriteTeam(NetPktGroup& group, Team* pTeam);

// FUNCTION: SWBF2 0x005c1f30
void WriteEvent_GameOver(NetPktGroup& group, ReliableNetEvent& event)
{
    WriteTeam(group, event.u.gameover.mTeam);
}

// FUNCTION: SWBF2 0x005c2680
void WriteCommandPost(NetPktGroup& group, const CommandPost* pPost)
{
    group.Write(pPost != 0);
    if (pPost)
    {
        group.WriteBits(pPost->GetIndex(), 4);
    }
}

// FUNCTION: SWBF2 0x005d2e40
bool IsMissingMoves(int iPlayer)
{
    if ((unsigned int)hostTurnNumber < 3)
    {
        return false;
    }
    NetPlayer& player = netPlayers[iPlayer];
    unsigned int turn = hostTurnNumber - 3;
    if (turn <= player.lateFenceTurn)
    {
        turn = player.lateFenceTurn + 1;
    }
    for (; turn < (unsigned int)hostTurnNumber; ++turn)
    {
        int index = turn & 31;
        if (hostRecv[index] & (1ui64 << iPlayer))
        {
            continue;
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x005d1e30
void SentUpdate(int iPlayer)
{
    NetPlayer& player = netPlayers[iPlayer];
    unsigned int iSlot;
    for (iSlot = 0; iSlot < 2; ++iSlot)
    {
        if (!player.sentUpdateTurn[iSlot])
        {
            break;
        }
    }
    float now = NetComm::GetTime();
    player.sentUpdateTurn[iSlot] = hostTurnNumber;
    player.sentUpdateTime[iSlot] = now;
    float minSkipTime = 0.1f;
    float skipTime = (player.averageRtt + player.deviationRtt) /
                     (float)player.updateWindowSize;
    if (skipTime < minSkipTime)
    {
        skipTime = minSkipTime;
    }
    int skipTurns = (int)((float)netTurnsPerSecond * skipTime + 0.5f);
    netPlayers[iPlayer].ackedUpdateTurn = hostTurnNumber + skipTurns;
}

// FUNCTION: SWBF2 0x005d20f0
bool IsSendWindowOpen(int iPlayer)
{
    NetPlayer& player = netPlayers[iPlayer];
    float now = NetComm::GetTime();
    int sendingCount = 0;
    for (unsigned int i = 0; i < 2; ++i)
    {
        if (!player.sentUpdateTurn[i])
        {
            continue;
        }
        float age = now - player.sentUpdateTime[i];
        if (age > player.averageRtt + player.deviationRtt)
        {
            if (++debugBootVoteCounter == debugBootVoteIndex)
            {
                ++gDebugCount;
            }
            --netPlayers[iPlayer].updateCount;
            ++netPlayers[iPlayer].dropCount;
            player.sentUpdateTurn[i] = 0;
            if (player.updateWindowSize > 1)
            {
                --player.updateWindowSize;
            }
        }
        else
        {
            ++sendingCount;
        }
    }
    return sendingCount < player.updateWindowSize;
}

// FUNCTION: SWBF2 0x005c2280
void WriteEvent_AddObjective(NetPktGroup& group, ReliableNetEvent& event)
{
    group.WriteBits(event.u.addObjective.mHashID, 32);
    group.WriteBits(event.u.addObjective.mPopupTextID, 32);
    group.Write(event.u.addObjective.team);
    group.WriteBits(event.u.addObjective.state, 8);
}

// FUNCTION: SWBF2 0x005c3440
void WriteSwitchObjRequest(NetPktGroup& group)
{
    NetPlayer& player = netPlayers[curDst];
    WriteTurnLowBits(group, player.lastObjState->turn);
    WriteTurnLowBits(group, player.nextObjState->turn);
    saveObjState = player.nextObjState->turn == (unsigned int)hostTurnNumber;
}

// FUNCTION: SWBF2 0x005c2e70
float FrontDist(const PblVector3& position, const PblMatrix& center)
{
    PblVector3 difference = position - center.GetTranslation();
    float distance = DotProduct(difference, center.GetZAxis());
    return distance;
}

// FUNCTION: SWBF2 0x005d3080
void FakeSendOneWorld(NetPktGroup& group)
{
    NetPlayer& netPlayer = netPlayers[curDst];
    if (netPlayer.nextObjState->turn ==
        (unsigned int)hostTurnNumber)
    {
        clientSwitchObjects = SR_CAN;
        clientSwitchObjTurn = hostTurnNumber;
    }
    group.Free();
}

// FUNCTION: SWBF2 0x005c5750
void AloHeroData::Update(NetPktGroup& group)
{
    if (CanChange())
    {
        Set();
    }
    if (Send(group))
    {
        Write(group);
    }
}

// FUNCTION: SWBF2 0x005c5360
void AloReinforcements::Update(NetPktGroup& group)
{
    if (CanChange())
    {
        for (int i = 0; i < 2; ++i)
        {
            int reinforcements, members, points;
            Team::GetPtr(i + 1)->GetValues(reinforcements, members, points);
            float bleedRate = Team::GetPtr(i + 1)->GetBleedRate();
            Set(i, reinforcements, members, bleedRate, points);
        }
    }
    if (Send(group))
    {
        Write(group);
    }
}

// FUNCTION: SWBF2 0x005c6f20
void AloStats::Update(NetPktGroup& group)
{
    if (IsConfirmed())
    {
        bool bGameOver = GameLoop::IsGameOver();
        if (bGameOver != lastGameOver)
        {
            Change();
            lastGameOver = bGameOver;
        }
    }
    if (Send(group))
    {
        group.Write(lastGameOver);
        if (lastGameOver)
        {
            PlayerStats::WriteAll(group);
        }
    }
}

void WriteString(NetPktGroup& pGroup, const char* pszText, unsigned int uCapacity);

// FUNCTION: SWBF2 0x005c70d0
void AloNextGameInfo::Write(NetPktGroup& group)
{
    group.Write(lastGameOver);
    if (lastGameOver)
    {
        const char* pszName = "";
        unsigned int iFirst, iSecond;
        MissionPlayList::PeekNextEntry(pszName, iFirst, iSecond);
        WriteString(group, pszName, 64);
        group.WriteBits(iFirst, 32);
        group.WriteBits(iSecond, 32);
        group.WriteBits(Team::GetWinningTeams(), 32);
    }
}

// FUNCTION: SWBF2 0x005c6560
void AloMiniStats::Update(NetPktGroup& group)
{
    if (CanChange())
    {
        unsigned int uEnemyKills = 0;
        unsigned int uTeamKills = 0;
        unsigned int uDeaths = 0;
        unsigned int uCps = 0;
        int iPoints = 0;
        Character* pPlayer = Character::FindPlayer(curDst);
        if (pPlayer)
        {
            PlayerStats& stats = Character::GetPlayerStats(pPlayer);
            uEnemyKills = stats.GetEnemyKills(-1, -1);
            uTeamKills = stats.GetTeamKills(-1, -1);
            uDeaths = stats.GetDeaths(-1, -1);
            uCps = stats.GetCommandPostPoints(-1, -1);
            iPoints = stats.GetPoints();
        }
        Set(uEnemyKills, uTeamKills, uDeaths, uCps, (short)iPoints);
    }
    if (Send(group))
    {
        Write(group);
    }
}

// FUNCTION: SWBF2 0x005c7520
void AloMissionTimer::Update(NetPktGroup& group)
{
    if (CanChange())
    {
        bool bFallback = false;
        const Timer* pTimer = MissionTimer::GetTimer();
        if (!pTimer)
        {
            bFallback = true;
            pTimer = HUD::GameEvents::gObjectiveTimer;
        }
        if (pTimer)
        {
            float time = pTimer->GetTime();
            float rate = pTimer->GetRate();
            bool active = pTimer->IsRunning();
            int iShow = MissionTimer::GetShow();
            if (bFallback)
            {
                iShow = 0;
            }
            int team = MissionTimer::GetTeam();
            Set(true, time, rate, active, iShow, team);
        }
        else
        {
            Set(false, 0.0f, 0.0f, false, 0, 0);
        }
    }
    if (Send(group))
    {
        Write(group);
    }
}

// FUNCTION: SWBF2 0x005c5de0
void AloMapDisplayMarkers::Set(int i, const TargetManager::Marker* aMarker)
{
    bool changed =
        aMarker->actor != markers[i].actor ||
        aMarker->region != markers[i].region ||
        aMarker->teamMask != markers[i].teamMask ||
        aMarker->textureHash != markers[i].textureHash ||
        aMarker->scale != markers[i].scale ||
        aMarker->color.GetARGB() != markers[i].color.GetARGB() ||
        aMarker->showHudTarget != markers[i].showHudTarget ||
        aMarker->pulseStartScale != markers[i].pulseStartScale ||
        aMarker->flashAlpha != markers[i].flashAlpha ||
        aMarker->pulseSize != markers[i].pulseSize;
    if (changed)
    {
        Change();
        memcpy(&markers[i], aMarker, sizeof(markers[i]));
    }
}

// FUNCTION: SWBF2 0x005cbb00
void SpawnDisplay::Write(NetPktGroup& group, int iPlayer)
{
    const unsigned int uOldTurn = mBaseTurn;
    const float fOldCycle = mBaseCycleTime;
    Character* pOldOwner = mOwner;
    mOwner = Character::FindPlayer(iPlayer);
    UpdateTimer();
    group.Write((int)mBaseTurn);
    group.Write(mBaseCycleTime);
    mBaseTurn = uOldTurn;
    mBaseCycleTime = fOldCycle;
    mOwner = pOldOwner;
}

// FUNCTION: SWBF2 0x005c5f50
void AloMapDisplayMarkers::Write(NetPktGroup& group)
{
    for (int i = 0; i < TargetManager::MAX_MARKERS; ++i)
    {
        bool bWriteIt = markers[i].actor || markers[i].region;
        if (bWriteIt)
        {
            group.Write(true);
            if (markers[i].actor)
            {
                group.Write(true);
                WriteObjectPtr(group, markers[i].actor);
            }
            else
            {
                group.Write(false);
            }
            if (markers[i].region)
            {
                group.Write(true);
                group.WriteBits(markers[i].region->mId, 32);
            }
            else
            {
                group.Write(false);
            }
            group.WriteBits(markers[i].teamMask, 3);
            group.WriteFixed(markers[i].scale, 0.0f, 16.0f, 8);
            group.WriteBits(markers[i].textureHash, 32);
            group.WriteBits(TargetManager::HashByColor(markers[i].color), 32);
            group.Write(markers[i].flashAlpha);
            group.Write(markers[i].pulseSize);
            group.WriteFixed(markers[i].pulseStartScale, 0.0f, 16.0f, 8);
            group.Write(markers[i].showHudTarget);
        }
    }
    group.Write(false);
}

// FUNCTION: SWBF2 0x005cad00
void WriteMove(NetPktGroup& group, ProcessedInputs* previous, ProcessedInputs* current)
{
    if (!current)
    {
        group.Write(false);
        group.Write(true);
        return;
    }
    if (IsSame(previous, current))
    {
        group.Write(true);
        return;
    }
    group.Write(false);
    group.Write(false);
    WriteFixed<StickAxisConvert>(stickAxisConvert, group, current->mFloatInputs[0]);
    WriteFixed<StickAxisConvert>(stickAxisConvert, group, current->mFloatInputs[1]);
    WriteFixed<MouseAxisConvert>(mouseAxisConvert, group, current->mFloatInputs[2]);
    WriteFixed<MouseAxisConvert>(mouseAxisConvert, group, current->mFloatInputs[3]);
    group.WriteBits(current->bits.mBoolInputs, 30);
    group.WriteBits(current->bits.mMouseMode, 2);
}

// FUNCTION: SWBF2 0x005d0e10
void UpdateBootPlayer(int& player)
{
    if (NetGame::IsPlaying(player, false))
    {
        if (netPlayers[player].mBooted)
        {
            if (NetComm::GetTime() > netPlayers[player].mBootTime)
            {
                NetGameP::SetNotPlaying(player);
                netPlayers[player].mBootTime = -1.0f;
                netPlayers[player].mBooted = false;
                player = -1;
            }
        }
        else
        {
            int guest = NetGame::GetGuestOf(player);
            if (guest < 0)
            {
                guest = player;
            }
            player = guest;
            netPlayers[guest].mBooted = true;
            netPlayers[guest].mBootTime = NetComm::GetTime() + 2.0f;
            NetGame::CreateEvent_Message(NetGame::MSG_BOOTED, Character::GetPtr(guest),
                                         0, 0, 0, 0, 0);
        }
    }
}

// FUNCTION: SWBF2 0x005d1020
void UpdateBootVote()
{
    if (hostBootPlayer >= 0)
    {
        int votes = 0;
        int players = 0;
        const int team = NetGame::GetStartTeam(hostBootPlayer);
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetGame::IsPlaying(i, false) &&
                NetGame::GetStartTeam(i) == team)
            {
                ++players;
                if (hostBootAgreeMask & ((unsigned __int64)1 << i))
                {
                    ++votes;
                }
            }
        }
        int required = (int)((float)players * (float)hostVoteThreshold /
                             100.0f + 0.5f);
        if (required <= 0)
        {
            required = 1;
        }
        if (votes >= required)
        {
            UpdateBootPlayer(hostBootPlayer);
        }
        else if ((unsigned int)hostTurnNumber >= hostBootTurn)
        {
            NetGame::CreateEvent_Message(NetGame::MSG_NOTBOOT,
                                         Character::GetPtr(hostBootPlayer),
                                         0, 0, 0, 0, 0);
            hostBootPlayer = -1;
        }
    }
    if (hostForceBootPlayer >= 0)
    {
        UpdateBootPlayer(hostForceBootPlayer);
    }
}

// FUNCTION: SWBF2 0x005d3590
bool WasRecentlySentToGuest(const GameObject* pObject)
{
    float now = NetComm::GetTime();
    for (int i = 0; i < 2; ++i)
    {
        int guest = NetGame::GetGuestIndex(curDst, i);
        if (guest >= 0)
        {
            for (unsigned int j = 0; j < 3; ++j)
            {
                float recordedTime = recentGuestObjects[guest][j].mRecordedTime;
                if (recordedTime > now || now - 10.0f > recordedTime)
                {
                    recentGuestObjects[guest][j].mObjectHandle = 0;
                    recentGuestObjects[guest][j].mRecordedTime = 0.0f;
                }
                const GameObject* object = recentGuestObjects[guest][j].mObjectHandle;
                if (object == pObject)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005d2bb0
void WritePingData(NetPktGroup& group)
{
    int player = netPlayers[curDst].nextSendPingIndex;
    int j;
    for (j = 0; j < netCurMaxPlayers; ++j)
    {
        if ((hostPlayingMask & ((unsigned __int64)1 << player)) != 0 &&
            netPlayers[player].guestOf == -1)
        {
            break;
        }
        if (++player >= netCurMaxPlayers)
        {
            player = 0;
        }
    }
    group.WriteBits(player, 6);
    if (player == curDst)
    {
        int loss = 0;
        if (netPlayers[player].packetCount)
        {
            loss = 100 * netPlayers[player].dropCount / netPlayers[player].packetCount;
        }
        group.WriteBits(loss, 7);
    }
    group.WriteBits(NetComm::GetPing(player), 10);
    for (j = -1; j < 1; ++j)
    {
        int slot = j == -1 ? player : netPlayers[player].myGuest[j];
        if (slot >= 0)
        {
            group.WriteBits(NetComm::GetPLKills(slot) + 50, 10);
        }
    }
    if (++player >= netCurMaxPlayers)
    {
        player = 0;
    }
    netPlayers[curDst].nextSendPingIndex = player;
}

// FUNCTION: SWBF2 0x005bd590
void AddSendBefore(GameObject* pObject, int iBefore)
{
    int i;
    if (pObject->GetEntityType() == ENT_BUILDING)
    {
        EntityBuildingClass* entClass = (EntityBuildingClass*)pObject->GetGameObjectClass();
        if (!entClass->IsDestructableBuilding())
        {
            return;
        }
    }
    for (i = 0; i < iBefore; ++i)
    {
        if (sendObjList[i] == pObject)
        {
            return;
        }
    }
    if (sendObjCount < 64)
    {
        ++sendObjCount;
    }
    for (i = sendObjCount - 2; i >= iBefore; --i)
    {
        if (sendObjList[i] == pObject)
        {
            sendObjList[i] = 0;
        }
        sendObjList[i + 1] = sendObjList[i];
    }
    sendObjList[iBefore] = pObject;
    pObject->AddSends(AddSendBefore, iBefore);
}

// FUNCTION: SWBF2 0x005bdc70
bool IsUnmanned(const GameObject* pObject)
{
    for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
    {
        if (it->IsSelfPilotable())
        {
            return false;
        }
        if ((it->IsPilotable() || it->IsRemoteVehicle()) && it->GetPilot())
        {
            return false;
        }
    }
    float lifeTime = GameLoop::GetMissionTime() - pObject->GetCreatedTime();
    if (lifeTime < 1.0f)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x005c2dd0
float FrontAngle(const PblVector3& point, const PblMatrix& matrix)
{
    PblVector3 offset = point - matrix.GetTranslation();
    const PblMatrix inverse = matrix.GetInverseFast();
    const PblVector3 local = inverse.ApplyRotation(offset);
    return PblMath::Atan2(local.GetX(), local.GetZ()) * (180.0f / PblMath::fPi);
}

// FUNCTION: SWBF2 0x005c2d30
bool FarToNear(GameObject* pFirst, GameObject* pSecond)
{
    PblVector3 diff1 = pFirst->GetMatrix().GetTranslation() - farToNearCenter;
    PblVector3 diff2 = pSecond->GetMatrix().GetTranslation() - farToNearCenter;
    float dSq1 = DotProduct(diff1, diff1);
    float dSq2 = DotProduct(diff2, diff2);
    if (dSq1 > dSq2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// FUNCTION: SWBF2 0x005be760
static PblVector3 GetClosestOrdnancePos(NetEvent::U::OrdnanceEvent& event)
{
    const PblVector3 start = event.mPosition.Get();
    const PblVector3 direction = event.mDirection.Get();
    OrdnanceClass* pClass = Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::FindNetIndex(event.mOrdClassIndex);
    const float range = pClass->GetRange();
    const float travel = (float)Clamp(DotProduct(sendCenter - start, direction), 0.0f, range);
    return start + direction * travel;
}

// FUNCTION: SWBF2 0x005d3100
void SetHostWorldIndex(int index, GameObject* pObject)
{
    if (NetGame::IsOneWorld() && !NetGame::IsDedicated() &&
        curDst == 0)
    {
        gameObjectInterpolator.SetNext(index, 0);
        hostWorldObjects[index] = pObject;
        pObject->SetNetUniqueId(index);
    }
}

// FUNCTION: SWBF2 0x005c7d10
void WriteTrackable(NetPktGroup& group, Trackable* pTrackable)
{
    if (!pTrackable || pTrackable == (Trackable*)-1 ||
        pTrackable->GetTracker() == (Tracker*)0xdddddddd ||
        !pTrackable->GetGameObject() || !pTrackable->GetGameObject()->IsAlive())
    {
        group.Write(false);
    }
    else
    {
        bool bDontSendData = false;
        GameObject* pObject = pTrackable->GetGameObject();
        if (bDontSendData)
        {
            group.Write(false);
        }
        else
        {
            group.Write(true);
            if (pObject->GetEntityType() == ENT_ARMED)
            {
                group.Write(true);
                NetNdObj* pNdObj = clientNdObjMap.FindId(pObject->GetHandleId());
                group.WriteBits(pNdObj->index, 8);
            }
            else
            {
                group.Write(false);
                unsigned int uniqId = GetUniqObjectId(pObject);
                group.WriteBits(uniqId, GetUniqObjectBits());
                if (pObject->GetControllable()->GetTrackable() == pTrackable)
                {
                    group.Write(true);
                }
                else
                {
                    group.Write(false);
                    const PblList<Controllable>& ctrlList = pObject->GetControllableList();
                    PblList<Controllable>::Itor itor(ctrlList);
                    unsigned int ctrlIndex = 0;
                    while ((Controllable*)itor)
                    {
                        Controllable* ctrl = itor;
                        if (ctrl->GetTrackable() == pTrackable)
                        {
                            break;
                        }
                        ++ctrlIndex;
                        ++itor;
                    }
                    group.WriteBits(ctrlIndex, 4);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005d0080
void WriteSpawnSelect(NetPktGroup& group)
{
    for (int i = 0; i < 2; ++i)
    {
        bool hasData = NetComm::GetLocalIndex(i) >= 0 &&
                       spawnInfo[i].changedTurn > (unsigned int)clientAckedTurnNumber;
        if (NetGame::IsHost() && NetGame::IsOneWorld())
        {
            spawnInfo[i].changedTurn = clientAckedTurnNumber;
        }
        group.Write(hasData);
        if (hasData)
        {
            group.WriteBits(spawnInfo[i].changeCount & 0xff, 8);
            group.Write(spawnInfo[i].cancel);
            if (spawnInfo[i].cancel)
            {
                continue;
            }
            group.Write(spawnInfo[i].spectate);
            if (spawnInfo[i].spectate)
            {
                continue;
            }
            group.Write(spawnInfo[i].autoassign);
            if (spawnInfo[i].autoassign)
            {
                continue;
            }
            bool isTeam1 = spawnInfo[i].team == 1;
            group.Write(isTeam1);
            group.WriteBits(spawnInfo[i].classIdx, 4);
            CommandPost* post = spawnInfo[i].post;
            if (!post || post->GetGameObject() != spawnInfo[i].postHandle)
            {
                post = 0;
            }
            bool isNull = post == 0;
            group.Write(isNull);
            if (!isNull)
            {
                group.WriteBits(post->GetIndex(), 4);
            }
        }
    }
}

struct ScopeInfo
{
    float objRadius;
    float maxDist;
};
 

// GLOBAL: SWBF2 0x007e8e64
ScopeInfo scopeInfo[2] = {{3.0f, 300.0f}, {1000.0f, 2000.0f}};

// FUNCTION: SWBF2 0x005c8810
int GetObjectsInScope(const PblVector3& center, GameObject** ppObjects, int maxCount)
{
    const unsigned int oldTurn = hostTurnNumber >= (unsigned int)netTurnsPerSecond
        ? hostTurnNumber - netTurnsPerSecond / 2 : 0;
    NetEntityList::Itor it(*netEntities);
    int count = 0;
    while (it)
    {
        NetEntity& pEntity = *it;
        ++it;
        if (SpaceAssault::Instance()->IsCriticalSystem(pEntity.obj))
        {
            if (pEntity.updateTurn < oldTurn)
            {
                pEntity.pos = pEntity.obj->GetCollisionCenter();
                pEntity.updateTurn = hostTurnNumber;
            }
            bool bSkip = false;
            if (netPlayers[curDst].nextUpdateTurn >
                    SpaceAssault::Instance()->GetIsAliveTurnNumber(pEntity.obj) &&
                netPlayers[curDst].clientTurn >
                    netPlayers[curDst].nextUpdateTurn)
            {
                bSkip = true;
            }
            if (!bSkip)
            {
                ppObjects[count++] = pEntity.obj;
            }
        }
        else
        {
            CollisionObject* collision = pEntity.obj->GetCollisionObject();
            if (!collision)
            {
                continue;
            }
            if (pEntity.updateTurn < oldTurn)
            {
                pEntity.pos = pEntity.obj->GetCollisionCenter();
                pEntity.updateTurn = hostTurnNumber;
            }
            for (unsigned int i = 0; i < 2; ++i)
            {
                const float radius = pEntity.obj->GetCollisionRadius();
                if (scopeInfo[i].objRadius > radius)
                {
                    const float radSq = radius * radius;
                    const PblVector3 delta = center - pEntity.pos;
                    float distSq = DotProduct(delta, delta);
                    const float range = scopeInfo[i].maxDist;
                    if (range * range < distSq && radSq < distSq)
                    {
                        distSq -= radSq;
                    }
                    if (distSq < range * range)
                    {
                        ppObjects[count++] = pEntity.obj;
                        break;
                    }
                }
            }
        }
        if (count >= maxCount)
        {
            ++gDebugCount;
            break;
        }
    }
    return count;
}

// FUNCTION: SWBF2 0x005cb6d0
void WriteCaptureDisplay(NetPktGroup& group, int playerID)
{
    Character* player = Character::FindPlayer(playerID);
    unsigned char showCaptureFlag = 0;
    if (player)
    {
        for (CommandPostItor it = CommandPostManager::Instance()->GetItor(); it; ++it)
        {
            CommandPost* postPtr = *it;
            if ((postPtr->GetHoldPlayerMask() & (1ULL << player->GetPlayerId())) ||
                (postPtr->mSkippedPlayerMask & (1ULL << player->GetPlayerId())))
            {
                bool skippedPlayer =
                    (postPtr->mSkippedPlayerMask & (1ULL << player->GetPlayerId())) != 0;
                int team = postPtr->GetGameObject()->GetTeam();
                if (team == 0)
                {
                    if (skippedPlayer)
                    {
                        showCaptureFlag = 3;
                    }
                    else
                    {
                        showCaptureFlag = 1;
                    }
                    group.WriteBits(showCaptureFlag, 3);
                    int showTeam = postPtr->GetBiasTeam();
                    group.WriteBits(showTeam, 3);
                    int otherTeam = 0;
                    if (postPtr->GetHoldTeam() < 0)
                    {
                        for (otherTeam = 1; otherTeam < 8; ++otherTeam)
                        {
                            if (otherTeam != showTeam && postPtr->GetHoldCount(otherTeam) > 0)
                            {
                                break;
                            }
                        }
                    }
                    group.WriteBits(otherTeam, 3);
                    const float fRatio = postPtr->GetCaptureRatio();
                    const unsigned int ratio = (unsigned int)(fRatio * CAPTURE_RATIO_MAX + 0.5f);
                    group.WriteBits(ratio, CAPTURE_RATIO_BITS);
                }
                else if (team != player->GetTeamNumber())
                {
                    if (skippedPlayer)
                    {
                        showCaptureFlag = 4;
                    }
                    else
                    {
                        showCaptureFlag = 2;
                    }
                    group.WriteBits(showCaptureFlag, 3);
                    group.WriteBits(team, 3);
                    const float fRatio = postPtr->GetNeutralizeRatio();
                    const unsigned int ratio = (unsigned int)(fRatio * CAPTURE_RATIO_MAX + 0.5f);
                    group.WriteBits(ratio, CAPTURE_RATIO_BITS);
                }
                else
                {
                    showCaptureFlag = 0;
                }
                break;
            }
        }
    }
    if (!showCaptureFlag)
    {
        group.WriteBits(showCaptureFlag, 3);
    }
}

// GLOBAL: SWBF2 0x01e64ee8
int clientUploadCycle;
// GLOBAL: SWBF2 0x01e64a28
int clientUploadType;
// GLOBAL: SWBF2 0x01e63002
bool clientMoreFiles;
// GLOBAL: SWBF2 0x01e63004
char clientFileName[16];
// GLOBAL: SWBF2 0x01e6321c
int clientUploadSizeLeft;
// GLOBAL: SWBF2 0x01e62f5c
int clientUploadTryCount;
// GLOBAL: SWBF2 0x01e64b98
NetPacket* clientUploadPkt;

// FUNCTION: SWBF2 0x005c4720
int __cdecl ReadJournalFile(char*, int size)
{
    return size;
}

// FUNCTION: SWBF2 0x005c49e0
void WriteUpload(NetPktGroup& group)
{
    const bool enabled = clientUploadState < 100;
    group.Write(enabled);
    if (!enabled)
    {
        return;
    }
    bool sent = false;
    while (!sent)
    {
        switch (clientUploadState)
        {
        case 0:
        {
            char filename[32];
            int fileSize = -1;
            for (int attempt = 0; attempt < 10; ++attempt)
            {
                GetJournalName(filename, clientUploadCycle, clientUploadType, 32);
                fileSize = OpenJournalFile(filename);
                if (fileSize >= 0)
                {
                    break;
                }
                ++clientUploadType;
            }
            clientMoreFiles = fileSize >= 0;
            if (fileSize < 0)
            {
                ++gDebugCount;
            }
            else
            {
                const char* name = strrchr(filename, '\\');
                if (!name)
                {
                    name = strrchr(filename, '/');
                }
                if (name)
                {
                    ++name;
                }
                else
                {
                    name = filename;
                }
                strncpy(clientFileName, name, 16);
                clientFileName[15] = 0;
                clientUploadSizeLeft = fileSize;
            }
            clientUploadState = 1;
            clientUploadTryCount = 0;
            ++clientUploadSeqNo;
            break;
        }
        case 1:
            if (clientHostUploadSeqNo == clientUploadSeqNo)
            {
                if (clientMoreFiles)
                {
                    clientUploadState = 2;
                    break;
                }
                DisableWriteUpload();
            }
            group.Write(true);
            group.WriteBits(clientUploadSeqNo, 32);
            group.Write(clientMoreFiles);
            if (clientMoreFiles)
            {
                WriteString(group, clientFileName, 16);
                group.WriteBits(clientUploadSizeLeft, 32);
            }
            ++clientUploadTryCount;
            sent = true;
            break;
        case 2:
        {
            const int chunkSize = clientUploadSizeLeft < 1012 ? clientUploadSizeLeft : 1012;
            clientUploadPkt = NetPacket::Alloc(false);
            ReadJournalFile(clientUploadPkt->mData, chunkSize);
            clientUploadState = 3;
            clientUploadTryCount = 0;
            ++clientUploadSeqNo;
            break;
        }
        case 3:
            if (clientHostUploadSeqNo == clientUploadSeqNo)
            {
                clientUploadPkt->Free();
                const int chunkSize = clientUploadSizeLeft < 1012 ? clientUploadSizeLeft : 1012;
                clientUploadSizeLeft -= chunkSize;
                if (clientUploadSizeLeft)
                {
                    clientUploadState = 2;
                }
                else
                {
                    CloseJournalFile();
                    char filename[32];
                    GetJournalName(filename, clientUploadCycle, clientUploadType, 32);
                    DeleteJournalFile(filename);
                    SyncJournalChanges();
                    clientUploadState = 0;
                    ++clientUploadType;
                }
            }
            else
            {
                group.Write(false);
                group.WriteBits(clientUploadSeqNo, 32);
                const int chunkSize = clientUploadSizeLeft < 1012 ? clientUploadSizeLeft : 1012;
                group.WriteBits(chunkSize, 10);
                group.WriteBytes(clientUploadPkt->mData, chunkSize);
                ++clientUploadTryCount;
                sent = true;
            }
            break;
        default:
            break;
        }
    }
}

// GLOBAL: SWBF2 0x007ec508
int debugWriteHandleId = 0x5166;

// GLOBAL: SWBF2 0x007ec4dc
int debugWriteInitialPlayer = 1;

void WriteQuaternion(NetPktGroup& group, const PblQuaternion& value);

// FUNCTION: SWBF2 0x005bbd50
void WriteCreate(NetPktGroup& group, GameObject* pObject)
{
    if (pObject->GetHandleId() == (unsigned int)debugWriteHandleId)
    {
        ++gDebugCount;
    }
    EntityClass* pClass = pObject->GetEntityClass();
    WriteEntityClass(group, pClass);
    int initialPlayerId = pObject->GetControllable() ?
        pObject->GetControllable()->GetInitialPlayerId() : -1;
    if (initialPlayerId < 0)
    {
        group.Write(false);
    }
    else
    {
        if (initialPlayerId == debugWriteInitialPlayer)
        {
            ++gDebugCount;
        }
        group.Write(true);
        group.WriteBits(initialPlayerId, 6);
        int playerId = pObject->GetControllable()->GetPlayerId();
        bool same = playerId == initialPlayerId;
        group.Write(same);
    }
    PblQuaternion rotation(pObject->GetMatrix());
    WriteQuaternion(group, rotation);
    WritePositionVector(group, pObject->GetMatrix().GetTranslation());
    if (pObject->GetEntityType() == ENT_SOLDIER ||
        pObject->GetEntityType() == ENT_DROIDEKA)
    {
        WriteCharacter(group, pObject->GetControllable()->GetCharacter());
    }
    if (pObject->GetEntityType() == ENT_POST_HOVER ||
        pObject->GetEntityType() == ENT_POST_WALKER ||
        pObject->GetEntityType() == ENT_POST_FLYER)
    {
        CommandPost* pPost = CommandPost::FindPostByObject(pObject);
        group.WriteBits(pPost->GetIndex(), 4);
    }
    WriteTeam(group, Team::GetPtr(pObject->GetTeam()));
    pObject->WriteCreate(group);
}

// FUNCTION: SWBF2 0x005bbf80
void WriteObject(NetPktGroup& group, GameObject* pObject)
{
    EntityType entType = pObject->GetEntityType();
    bool neverDies = IsNeverDies(entType);
    group.Write(neverDies);
    if (neverDies)
    {
        NetNdObj* pNdObj = clientNdObjMap.FindId(pObject->GetHandleId());
        group.WriteBits(pNdObj->index, 8);
        pObject->WriteObject(group, 0);
    }
    else
    {
        NetObjState* lastState = 0;
        unsigned int uniqId = 63;
        if (!isFullState)
        {
            GetLastObjState(pObject, lastState, uniqId);
            group.WriteBits(uniqId, 6);
        }
        else
        {
            group.WriteBits(pObject->GetHandleId(), 16);
            WriteCreate(group, pObject);
        }
        pObject->WriteObject(group, lastState);
        if (saveObjState)
        {
            NetObjState* state = netPlayers[curDst].nextObjState->Find(uniqId);
            state->rotation = PblQuaternion(pObject->GetMatrix());
            state->position = pObject->GetMatrix().GetTranslation();
            state->team = (unsigned char)pObject->GetTeam();
            state->perceivedTeam = (unsigned char)pObject->GetPerceivedTeam();
        }
    }
}

// TEMPLATE: SWBF2 0x005d3c60
// WriteFixed<StickAxisConvert>
template void WriteFixed<StickAxisConvert>(StickAxisConvert&, NetPktGroup&, float);

// TEMPLATE: SWBF2 0x005d3c90
// WriteFixed<MouseAxisConvert>
template void WriteFixed<MouseAxisConvert>(MouseAxisConvert&, NetPktGroup&, float);

 
 

// TEMPLATE: SWBF2 0x005d39f0
// BblList<HandleToIndex,128>::BblList<HandleToIndex,128>
template BblList<HandleToIndex, 128>::BblList();

// TEMPLATE: SWBF2 0x005d3ae0
// PblList<NG_ChatMsg>::Remove
template void PblList<NG_ChatMsg>::Remove(PblList<NG_ChatMsg>::Node*);

// FUNCTION: SWBF2 0x005c0570
void WriteEvent_Message(NetPktGroup& group, ReliableNetEvent& event)
{
    group.WriteBits(event.u.message.mType, 4);
    WriteCharacter(group, event.u.message.mCharacter1);
    WriteCharacter(group, event.u.message.mCharacter2);
    WriteEntityClass(group, event.u.message.mEntClass);
    WriteWeaponClass(group, event.u.message.mWpnClass);
    WriteCommandPost(group, event.u.message.mCommandPost);
    WriteTeam(group, event.u.message.mTeam);
}

// FUNCTION: SWBF2 0x005c06e0
void WriteEvent_ChangeTeam(NetPktGroup& group, ReliableNetEvent& event)
{
    WriteCommandPost(group, event.u.changeteam.mCommandPost);
    WriteTeam(group, event.u.changeteam.mTeam);
    group.Write(event.u.changeteam.mAlive);
}

// FUNCTION: SWBF2 0x005c5d30
void AloMapDisplayMarkers::Update(NetPktGroup& group)
{
    if (CanChange())
    {
        for (int i = 0; i < TargetManager::MAX_MARKERS; ++i)
        {
            TargetManager::Marker* newMarker = &TargetManager::gMarkers[i];
            Set(i, newMarker);
        }
    }
    if (Send(group))
    {
        Write(group);
    }
}

// FUNCTION: SWBF2 0x005c7060
void AloNextGameInfo::Update(NetPktGroup& group)
{
    if (IsConfirmed())
    {
        const bool bGameOver = GameLoop::IsGameOver();
        if (bGameOver != lastGameOver)
        {
            Change();
            lastGameOver = bGameOver;
        }
    }
    if (Send(group))
    {
        Write(group);
    }
}

// FUNCTION: SWBF2 0x005c9900
void WriteMoves(NetPktGroup& group, int iPlayer, unsigned int uTurn)
{
    ProcessedInputs* pPrevious = &noInputs;
    while (true)
    {
        int iSlot = uTurn & 31;
        if (uTurn >= (unsigned int)hostTurnNumber &&
            !(hostRecv[iSlot] & (1ui64 << iPlayer)))
        {
            break;
        }
        ProcessedInputs* pCurrent = &hostTurns[iSlot].moves[iPlayer];
        WriteMove(group, pPrevious, pCurrent);
        pPrevious = pCurrent;
        ++uTurn;
    }
    WriteMove(group, pPrevious, 0);
}

// FUNCTION: SWBF2 0x005c2eb0
int FrontCount(GameObject* const* ppObjects, int iCount,
               int iCurrent, const PblMatrix& camera)
{
    GameObject* object = ppObjects[iCurrent];
    int count = 0;
    float currentDistance = FrontDist(object->GetMatrix().GetTranslation(), camera);
    for (int i = 0; i < iCount; ++i)
    {
        const PblVector3& position = ppObjects[i]->GetMatrix().GetTranslation();
        float angle = FrontAngle(position, camera);
        if (angle < -30.0f || angle > 30.0f)
        {
            continue;
        }
        float distance = FrontDist(position, camera);
        if (distance < 0.0f)
        {
            continue;
        }
        if (currentDistance >= distance)
        {
            ++count;
        }
    }
    return count;
}

// FUNCTION: SWBF2 0x005be890
float GetNetEventScore(NetEvent& event)
{
    switch (event.mType)
    {
    case 0:
    {
        NetEvent::U::OrdnanceEvent& data = event.u.ordnance;
        const GameObject* owner;
        data.mOwner.Get(owner);
        Character* character = data.mCharacter;
        if (character &&
            (character->GetPlayerId() == curDst ||
             NetGame::GetGuestOf(character->GetPlayerId()) == curDst))
        {
            return -1.0f;
        }
        PblVector3 closestPos = GetClosestOrdnancePos(data);
        if (SquaredLength(sendCenter - closestPos) > 22500.0f)
        {
            return -1.0f;
        }
        return Length(sendCenter - closestPos);
    }
    }
    return -1.0f;
}

void WriteDestMask(NetPktGroup& group, unsigned __int64 mask);

// FUNCTION: SWBF2 0x005c9510
void SendToReplicas()
{
    int iHost;
    int iPlayer;
    if (netHosts[netHostIndex].hostMode)
    {
        return;
    }
    for (iHost = 0; iHost < netCurMaxHosts; ++iHost)
    {
        if (!NetComm::IsConnected(iHost + 64))
        {
            continue;
        }
        if (netHosts[iHost].hostMode != 2)
        {
            continue;
        }
        if ((unsigned int)hostTurnNumber <= netHosts[iHost].updateTurn)
        {
            continue;
        }

        NetPktGroup group;
        group.BeginWrite();
        WriteDestMask(group, hostPlayingMask);
        WriteTurnLowBits(group, netHosts[iHost].hostAckedUpdateTurn);
        for (iPlayer = 0; iPlayer < netCurMaxPlayers; ++iPlayer)
        {
            NetPlayer& player = netPlayers[iPlayer];
            if (player.clientOfIndex != iHost)
            {
                continue;
            }
            WriteTurnLowBits(group, player.clientTurn);
            group.WriteBits(GetLag(iPlayer) + 128, 8);
            WriteSwitchResponse(group,
                player.objSwitchRelay.switchResponse,
                player.objSwitchRelay.switchTurn);
        }
        for (iPlayer = 0; iPlayer < netCurMaxPlayers; ++iPlayer)
        {
            if (hostPlayingMask & (1ui64 << iPlayer))
            {
                WriteMoves(group, iPlayer, netHosts[iHost].hostAckedUpdateTurn);
            }
        }
        group.EndWrite();
        NetComm::Send(iHost + 64, 15, &group);
        netHosts[iHost].updateTurn = hostTurnNumber;
    }
}

// FUNCTION: SWBF2 0x005c69d0
void AloTrackable::Update(NetPktGroup& group)
{
    if (CanChange())
    {
        int iSpectator = NetGame::GetSpectator(curDst);
        Tracker* tracker = Tracker::GetTracker(iSpectator >= 0 ? iSpectator : curDst);
        Trackable* pTrackable = tracker->GetTrackable();
        Trackable* other[1];
        unsigned int i;
        for (i = 0; i < 1; ++i)
        {
            other[i] = 0;
        }
        for (i = 0; i < 1; ++i)
        {
            int iGuest = NetGame::GetMyGuest(curDst, i);
            if (iGuest >= 0)
            {
                iSpectator = NetGame::GetSpectator(iGuest);
                tracker = Tracker::GetTracker(iSpectator >= 0 ? iSpectator : iGuest);
                other[i] = tracker->GetTrackable();
            }
        }
        Set(pTrackable, other);
    }
    if (Send(group))
    {
        Write(group);
    }
}

// FUNCTION: SWBF2 0x005c6c00
void AloTrackable::Write(NetPktGroup& group)
{
    if (isFullState)
    {
        Tracker* tracker = Tracker::GetTracker(curDst);
        WriteTrackable(group, tracker->GetTrackable());
    }
    else
    {
        WriteTrackable(group, mTrackable[curDst][0]);
        for (unsigned int i = 0; i < 1; ++i)
        {
            if (NetGame::GetMyGuest(curDst, i) >= 0)
            {
                WriteTrackable(group, mTrackable[curDst][i + 1]);
            }
        }
        if (NetGame::IsOneWorld() && NetGame::IsHost() && curDst == NetGame::GetLocalIndex(0))
        {
            Trackable* pTrackable = mTrackable[curDst][0];
            Tracker* pTracker = Tracker::GetTracker(curDst);
            if (pTrackable && pTrackable != pTracker->GetTrackable())
            {
                if (pTrackable == (Trackable*)-1)
                {
                    Tracker* tracker = Tracker::GetTracker(curDst);
                    if (tracker)
                    {
                        Tracker::SetTrackable(0, curDst);
                    }
                }
                else
                {
                    pTrackable->Track(curDst);
                }
            }
            for (unsigned int i = 0; i < 1; ++i)
            {
                int iGuest = NetComm::GetLocalIndex(i + 1);
                if (iGuest >= 0)
                {
                    pTrackable = mTrackable[curDst][i + 1];
                    if (pTrackable)
                    {
                        pTracker = Tracker::GetTracker(iGuest);
                        if (pTrackable != pTracker->GetTrackable())
                        {
                            if (pTrackable == (Trackable*)-1)
                            {
                                Tracker* tracker = Tracker::GetTracker(iGuest);
                                if (tracker)
                                {
                                    Tracker::SetTrackable(0, iGuest);
                                }
                            }
                            else
                            {
                                pTrackable->Track(iGuest);
                            }
                        }
                    }
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005cc1e0
void WriteSpawnDisplay(NetPktGroup& group)
{
    Character* player = Character::FindPlayer(curDst);
    bool hasData = false;
    if (player && player->IsPlayer() && !player->GetUnit())
    {
        hasData = true;
    }
    unsigned int numLiveHumans1 = 0;
    unsigned int numLiveHumans2 = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetGame::IsPlaying(i, false))
        {
            Character* c = Character::FindPlayer(i);
            if (c && c->GetUnit())
            {
                if (netPlayers[i].isTeam1)
                {
                    ++numLiveHumans1;
                }
                else
                {
                    ++numLiveHumans2;
                }
            }
        }
    }
    SpawnDisplay::sNumLiveHumansTeam1 = numLiveHumans1;
    SpawnDisplay::sNumLiveHumansTeam2 = numLiveHumans2;
    group.WriteBits(numLiveHumans1, 6);
    group.WriteBits(numLiveHumans2, 6);
    group.Write(hasData);
    if (hasData)
    {
        SpawnDisplay::Instance(0)->Write(group, curDst);
        bool hasDelay = false;
        if (player->GetDelayTimer() >= 0.0f)
        {
            hasDelay = true;
        }
        group.Write(hasDelay);
        unsigned int bits = netPlayers[curDst].mSpawnOrder + 1;
        group.WriteBits(bits, 8);
    }
    for (unsigned int guestIdx = 0; guestIdx < 1; ++guestIdx)
    {
        int guestDst = NetGame::GetMyGuest(curDst, guestIdx);
        bool hasData = false;
        if (guestDst >= 0)
        {
            player = Character::FindPlayer(guestDst);
            if (player && player->IsPlayer() && !player->GetUnit())
            {
                hasData = true;
            }
            group.Write(hasData);
            if (hasData)
            {
                SpawnDisplay::Instance(0)->Write(group, guestDst);
                bool hasDelay = false;
                if (player->GetDelayTimer() >= 0.0f)
                {
                    hasDelay = true;
                }
                group.Write(hasDelay);
                unsigned int bits = netPlayers[guestDst].mSpawnOrder + 1;
                group.WriteBits(bits, 8);
            }
        }
        else
        {
            group.Write(hasData);
        }
    }
}

// FUNCTION: SWBF2 0x005d2740
void WritePlayerMoves(NetPktGroup& group)
{
    PblHeap<int, float>::RECORD records[65];
    PblHeap<int, float> nearest(64, 1000000.0f, records);
    int iTracker = curDst;
    const int iSpectator = NetGame::GetSpectator(curDst);
    if (iSpectator != -1)
    {
        iTracker = iSpectator;
    }
    PblVector3 center(0.0f, 0.0f, 0.0f);
    Trackable* pTrackable = Tracker::GetTracker(iTracker)->GetTrackable();
    if (pTrackable && pTrackable != (Trackable*)-1 &&
        pTrackable->GetTracker() != (Tracker*)0xdddddddd)
    {
        center = pTrackable->GetGameObject()->GetMatrix().GetTranslation();
    }

    for (int iPlayer = 0; iPlayer < netCurMaxPlayers; ++iPlayer)
    {
        if ((hostPlayingMask & ((unsigned __int64)1 << iPlayer)) &&
            iPlayer != curDst)
        {
            pTrackable = Tracker::GetTracker(iPlayer)->GetTrackable();
            if (pTrackable && pTrackable != (Trackable*)-1 &&
                pTrackable->GetTracker() != (Tracker*)0xdddddddd)
            {
                nearest.Push(iPlayer,
                    -Length(pTrackable->GetGameObject()->GetMatrix().GetTranslation() - center));
            }
        }
    }

    int iTurn = (hostTurnNumber - 1) & 31;
    int numSendMoves = 5;
    while (!nearest.IsEmpty() && numSendMoves > 0)
    {
        --numSendMoves;
        int iPlayer = nearest.Get(0);
        group.WriteBits(iPlayer, 6);
        ProcessedInputs* move = &hostTurns[iTurn].moves[iPlayer];
        WriteFixed<StickAxisConvert>(stickAxisConvert, group, move->mFloatInputs[0]);
        WriteFixed<StickAxisConvert>(stickAxisConvert, group, move->mFloatInputs[1]);
        WriteFixed<MouseAxisConvert>(mouseAxisConvert, group, move->mFloatInputs[2]);
        WriteFixed<MouseAxisConvert>(mouseAxisConvert, group, move->mFloatInputs[3]);
        group.WriteBits(move->bits.mBoolInputs, 4);
        nearest.Pop();
    }
    group.WriteBits(curDst, 6);
}

// FUNCTION: SWBF2 0x005c2fb0
void ListNearObjects()
{
    if (!listNearObjects)
    {
        return;
    }
    listNearObjects = false;

    PblMatrix center(gMatrixIdentity);
    Tracker* tracker = Tracker::GetTracker(curDst);
    Trackable* trackable = tracker->GetTrackable();
    GameObject* tracked = 0;
    if (trackable)
    {
        tracked = trackable->GetGameObject();
        center = tracked->GetMatrix();
    }

    GameObject* objects[256];
    int n = CollisionManager::GetObjectsInRange(
        center.GetTranslation(), 100.0f, objects, 256, 0, 0);
    farToNearCenter = center.GetTranslation();
    // TEMPLATE: SWBF2 0x005d3d20
    // std::_Sort<GameObject * *,int,bool (__cdecl*)(GameObject *,GameObject *)>
    // TEMPLATE: SWBF2 0x005d3e20
    // std::_Unguarded_partition<GameObject * *,bool (__cdecl*)(GameObject *,GameObject *)>
    // TEMPLATE: SWBF2 0x005d4180
    // std::make_heap<GameObject * *,bool (__cdecl*)(GameObject *,GameObject *)>
    // TEMPLATE: SWBF2 0x005d41f0
    // std::_Median<GameObject * *,bool (__cdecl*)(GameObject *,GameObject *)>
    // TEMPLATE: SWBF2 0x005d42e0
    // std::_Sort_heap<GameObject * *,bool (__cdecl*)(GameObject *,GameObject *)>

    // TEMPLATE: SWBF2 0x005d4340
    // std::_Insertion_sort1<GameObject * *,bool (__cdecl*)(GameObject *,GameObject *),GameObject *>
    // TEMPLATE: SWBF2 0x005d4430
    // std::_Med3<GameObject * *,bool (__cdecl*)(GameObject *,GameObject *)>
    // TEMPLATE: SWBF2 0x005d44e0
    // std::_Adjust_heap<GameObject * *,int,GameObject *,bool (__cdecl*)(GameObject *,GameObject *)>
    std::sort(&objects[0], &objects[n], FarToNear);

    char msg[256];
    for (int i = 0; i < n; ++i)
    {
        GameObject* object = objects[i];
        const PblVector3& pos = object->GetMatrix().GetTranslation();
        int playerId = object->GetControllable() ? object->GetControllable()->GetPlayerId() : -1;
        int frontCount = FrontCount(objects, n, i, center);
        float frontAngle = FrontAngle(pos, center);
        float frontDist = FrontDist(pos, center);
        _snprintf(msg, sizeof(msg),
                  "%d: handleId(%d) type(%s) playerId(%d) pos(%.2f,%.2f,%.2f) frontCount(%d) frontAngle(%.2f) frontDist(%.2f)\n",
                  i, object->GetHandleId(), object->GetDerivedRttiName(),
                  playerId, pos.GetX(), pos.GetY(), pos.GetZ(),
                  frontCount, frontAngle, frontDist);
        RedConsole::Instance()->PrintMsg(msg);
    }
    _snprintf(msg, sizeof(msg), "center(%.2f,%.2f,%.2f)\n",
              center.GetTranslation().GetX(), center.GetTranslation().GetY(), center.GetTranslation().GetZ());
    RedConsole::Instance()->PrintMsg(msg);
    _snprintf(msg, sizeof(msg), "front(%.2f,%.2f,%.2f)\n",
              center.GetZAxis().GetX(), center.GetZAxis().GetY(), center.GetZAxis().GetZ());
    RedConsole::Instance()->PrintMsg(msg);
}

void WritePositionVector(NetPktGroup& group, const PblVector3& value);
void WriteCosSin(NetPktGroup& group, PblAngle& angle);

// FUNCTION: SWBF2 0x005d2ef0
void WriteSpawnCamera(NetPktGroup& group)
{
    bool writeCamera = false;
    SpawnManager::EnterClient();
    Character* character = NetGame::GetLocalPlayer(0);
    SpawnManager::LeaveClient();
    if (character && !character->GetUnit())
    {
        writeCamera = true;
    }
    group.Write(writeCamera);
    if (writeCamera)
    {
        PblMatrix camera = CameraManager::Instance()->GetChaseCameraTransform(0);
        const PblVector3& spawnPos = camera.GetTranslation();
        WritePositionVector(group, spawnPos);
        const PblVector3& front = camera.GetZAxis();
        PblAngle frontAngle(front.GetX(), front.GetZ());
        WriteCosSin(group, frontAngle);
    }
}

// FUNCTION: SWBF2 0x005c7a00
void AtleastOnce::WriteAll(NetPktGroup& group)
{
    if (curDst >= 0)
    {
        aloTrackable.Update(group);
        aloMiniStats.Update(group);
    }
    if (!isFullState || curDst == -1)
    {
        aloReinforcements.Update(group);
        aloHeroData0.Update(group);
        aloHeroData1.Update(group);
        aloHeroData2.Update(group);
        aloMapDisplayMarkers.Update(group);
        aloStats.Update(group);
        aloBootVote.Update(group);
        aloNextGameInfo.Update(group);
        aloMissionTimer.Update(group);
    }
    group.WriteBits(0, 4);
}

// FUNCTION: SWBF2 0x005d17d0
void NetDisplayClientMessage()
{
    PblList<NG_ChatMsg>::Node* pNode = netGameChatMsgClient._head._pPrev;
    NG_ChatMsg* pMessage = pNode->_pObject;
    DisplayMessage(*pMessage);
    netGameChatMsgClient.Remove(pNode);
    delete pMessage;
}

// FUNCTION: SWBF2 0x005d1820
void NetGame::SendChatMessageToServer(NetPktGroup& group)
{
    int count = netGameChatMsgClient.GetCount();
    if (count > 0)
    {
        PblList<NG_ChatMsg>::Node* pNode = netGameChatMsgClient.GetFirst();
        NG_ChatMsg* pMessage = pNode->_pObject;
        if (!NetAdmin::ParseLineLocal(pMessage->msg, false))
        {
            unsigned int length = 2 * PblUnicode::ustrlen(pMessage->msg) + 22;
            unsigned int lengthMask = length | 0xffff0000;
            group.WriteBytes((const char*)&lengthMask, sizeof(lengthMask));
            if (length > 0)
            {
                group.WriteBytes((const char*)pMessage, length);
            }
        }
        else
        {
            unsigned int lengthMask = 0xffff0000;
            group.WriteBytes((const char*)&lengthMask, sizeof(lengthMask));
        }
        netGameChatMsgClient.Remove(pNode);
        delete pMessage;
    }
    else
    {
        unsigned int lengthMask = 0xffff0000;
        group.WriteBytes((const char*)&lengthMask, sizeof(lengthMask));
    }
}

// FUNCTION: SWBF2 0x005d19b0
void NetGame::SendChatMessageToClient(NetPktGroup& group)
{
    int count = netGameChatMsgServer.GetCount();
    if (count > 0)
    {
        PblList<NG_ChatMsg>::Node* pNode = netGameChatMsgServer.GetFirst();
        NG_ChatMsg* pMessage = pNode->_pObject;
        if (!(pMessage->mask & (1ui64 << curDst)))
        {
            unsigned int length = 2 * PblUnicode::ustrlen(pMessage->msg) + 22;
            unsigned int lengthMask = length | 0xffff0000;
            group.WriteBytes((const char*)&lengthMask, sizeof(lengthMask));
            if (length > 0)
            {
                group.WriteBytes((const char*)pMessage, length);
            }
            if (NetGame::IsEnabled() &&
                NetGame::GetLocalIndex(0) == curDst &&
                NetGame::IsOneWorld() && NetGame::IsHost() && !NetGame::IsDedicated())
            {
                DisplayMessage(*pMessage);
            }
            pMessage->mask += 1ui64 << curDst;
            if ((pMessage->mask & hostPlayingMask) == hostPlayingMask)
            {
                netGameChatMsgServer.Remove(pNode);
                delete pMessage;
            }
        }
        else
        {
            unsigned int lengthMask = 0xffff0000;
            group.WriteBytes((const char*)&lengthMask, sizeof(lengthMask));
        }
    }
    else
    {
        unsigned int lengthMask = 0xffff0000;
        group.WriteBytes((const char*)&lengthMask, sizeof(lengthMask));
    }
}

// FUNCTION: SWBF2 0x005bf580
void SendReliableNetEvents(NetPktGroup& group)
{
    NetPlayer& player = netPlayers[curDst];
    const unsigned int uLastStateTurn = player.lastObjState->turn;
    if (uLastStateTurn != (unsigned int)-1 &&
        player.reliableSendTurn != (unsigned int)-1 &&
        uLastStateTurn >= player.reliableSendTurn)
    {
        unsigned int head = player.reliableEventHead;
        unsigned int& iEvent = player.reliableEventTail;
        while (iEvent != head)
        {
            ReliableNetEvent& event = reliableEvents[iEvent];
            event.mSendMask &= ~(1ui64 << curDst);
            if (!event.mSendMask)
            {
                AdvanceReliableTail(iEvent);
            }
            if (++iEvent >= 128)
            {
                iEvent = 0;
            }
        }
        player.reliableSendTurn = (unsigned int)-1;
        ++player.reliableBatchNumber;
    }
    if (player.reliableSendTurn == (unsigned int)-1 &&
        player.reliableEventTail != (unsigned int)reliableNetEventHead)
    {
        player.reliableSendTurn = hostTurnNumber;
        player.reliableEventHead = reliableNetEventHead;
    }
    unsigned int head = player.reliableEventHead;
    unsigned int iEvent = player.reliableEventTail;
    if (player.reliableSendTurn != (unsigned int)-1 && head != iEvent)
    {
        group.Write(true);
        group.WriteBits(player.reliableBatchNumber, 6);
        int iWritten = 0;
        while (iEvent != head)
        {
            ReliableNetEvent& event = reliableEvents[iEvent];
            if (event.mSendMask & (1ui64 << curDst))
            {
                group.Write(true);
                ++iWritten;
                if (player.reliableBatchNumber == 16 && iWritten == 1)
                {
                    ++gDebugCount;
                }
                group.WriteBits(event.mEvType, 4);
                switch (event.mEvType)
                {
                case REL_MESSAGE: WriteEvent_Message(group, event); break;
                case REL_CHANGETEAM: WriteEvent_ChangeTeam(group, event); break;
                case REL_CHANGECOMMANDPOSTTEAMS: WriteEvent_ChangeCommandPostTeams(group, event); break;
                case REL_CREATE_CHUNKCLUSTER: WriteEvent_CreateChunkCluster(group, event); break;
                case REL_CORRECT_CHUNKCLUSTER: WriteEvent_CorrectChunkCluster(group, event); break;
                case REL_CONNECT_TOWCABLE: WriteEvent_ConnectTowCable(group, event); break;
                case REL_DESTROY_TOWCABLE: WriteEvent_DestroyTowCable(group, event); break;
                case REL_GAMEOVER: WriteEvent_GameOver(group, event); break;
                case REL_SCRIPTMESSAGE: WriteEvent_ScriptMessage(group, event); break;
                case REL_PLAYVOICEOVER: WriteEvent_PlayVoiceOver(group, event); break;
                case REL_UPDATEDOORSTATE: WriteEvent_UpdateDoorState(group, event); break;
                case REL_ADDOBJECTIVE: WriteEvent_AddObjective(group, event); break;
                case REL_UPDATEOBJECTIVE: WriteEvent_UpdateObjective(group, event); break;
                case REL_UPDATEINVINCIBILITY: WriteEvent_UpdateInvincibility(group, event); break;
                }
            }
            if (++iEvent >= 128)
            {
                iEvent = 0;
            }
        }
        group.Write(false);
    }
    else
    {
        group.Write(false);
    }
}

void WritePositionVector(NetPktGroup& group, const PblVector3& value);

// FUNCTION: SWBF2 0x005be9b0
void WriteNetEvent(NetEvent& event, NetPktGroup& group)
{
    switch (event.mType)
    {
    case 0:
    {
        NetEvent::U::OrdnanceEvent& data = event.u.ordnance;
        const GameObject* owner;
        data.mOwner.Get(owner);
        unsigned int ownerId = GetUniqObjectId(owner);
        WeaponClass* weaponClass = data.mWpnClassIndex < 0xff ? Factory<Weapon, WeaponClass, WeaponDesc>::FindNetIndex(data.mWpnClassIndex) : 0;
        if (ownerId == 63 && weaponClass &&
            weaponClass->IsRtti(WeaponMeleeThrowClass::GetRtti()))
        {
            break;
        }

        group.Write(true);
        group.WriteBits(event.mType, 3);
        PblVector3 vector;
        vector = data.mPosition.Get();
        WritePositionVector(group, vector);
        vector = data.mDirection.Get();
        WriteUnitVector(group, vector);
        group.WriteFixed(data.mStrength, 0.0f, 1.0f, 10);
        bool hasOwner = owner != 0;
        group.Write(hasOwner);
        if (hasOwner)
        {
            WriteObjectPtr(group, owner);
        }
        WriteCharacter(group, data.mCharacter);
        if (data.mGameObject && data.mGameObject->GetHandleId() == data.mGameObjectId)
        {
            group.Write(true);
            WriteObjectPtr(group, data.mGameObject);
        }
        else
        {
            group.Write(false);
        }
        if (data.mDamageMultiplier != 1.0f)
        {
            group.Write(true);
            group.WriteFixed(data.mDamageMultiplier, 0.0f, 10.0f, 16);
        }
        else
        {
            group.Write(false);
        }
        group.WriteBits(data.mOrdClassIndex, 8);
        group.WriteBits(data.mWpnClassIndex, 8);
        group.WriteBits(data.mEntClassIndex, 8);
        unsigned int targetId = data.mTarget && data.mTarget->GetHandleId() == data.mTargetId
            ? GetUniqObjectId(data.mTarget) : 63;
        group.WriteBits(targetId, GetUniqObjectBits());
        if (owner && data.mFireWeapon)
        {
            group.WriteBits(1, 1);
            Controllable* controllable = data.mFireWeapon->mOwner;
            PblList<Controllable>::Itor it(owner->GetControllableList());
            int controllableIndex;
            for (controllableIndex = 0; (Controllable*)it; ++controllableIndex, ++it)
            {
                if ((Controllable*)it == controllable)
                {
                    break;
                }
            }
            group.WriteBits(controllableIndex, 3);
            int weaponIndex;
            for (weaponIndex = 0; weaponIndex < controllable->GetWeaponCount(); ++weaponIndex)
            {
                if (data.mFireWeapon == controllable->GetWeaponPtr(weaponIndex))
                {
                    break;
                }
            }
            group.WriteBits(weaponIndex, 2);
        }
        else
        {
            group.WriteBits(0, 1);
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x005bdd50
void AddSend(GameObject* pObject)
{
    float scale = 1.0f;
    const PblVector3& center = pObject->GetCollisionCenter();
    const float radius = pObject->GetCollisionRadius();
    PblVector3 offset = center - sendCenter;
    float distance = Length(offset);
    if (distance > 0.0f)
    {
        offset = offset / distance;
    }
    if (distance > radius)
    {
        distance = distance - radius;
    }
    else
    {
        distance = 0.0f;
    }
    offset = offset * distance;

    bool canTurn = true;
    EntityType type = pObject->GetEntityType();
    bool deadSystem = false;
    if (SpaceAssault::Instance()->IsCriticalSystem(pObject))
    {
        bool reported = false;
        if (netPlayers[curDst].nextUpdateTurn >
                SpaceAssault::Instance()->GetIsAliveTurnNumber(pObject) &&
            netPlayers[curDst].clientTurn > netPlayers[curDst].nextUpdateTurn)
        {
            reported = true;
        }
        if (!reported)
        {
            deadSystem = true;
            scale = 0.0f;
        }
    }

    if (!deadSystem)
    {
        switch (type)
        {
        case ENT_FLYER:
        case ENT_WALKER:
        case ENT_HOVER:
        case ENT_POST_HOVER:
        case ENT_POST_WALKER:
        case ENT_POST_FLYER:
            if (IsUnmanned(pObject))
            {
                canTurn = true;
                scale = 1.5f;
            }
            else
            {
                canTurn = false;
                if (type == ENT_FLYER)
                {
                    scale = 0.07f;
                }
                else
                {
                    scale = 0.6f;
                }
            }
            break;
        case ENT_ASTEROID:
            scale = 0.1f;
            break;
        case ENT_SOLDIER:
        case ENT_DROIDEKA:
        case ENT_TAUNTAUN:
        case ENT_DROID:
        case ENT_FLAG:
            if (type != ENT_FLAG && WasRecentlySentToGuest(pObject))
            {
                scale = 0.25f;
            }
            else if (distance > 200.0f)
            {
                scale = 2.0f;
            }
            break;
        case ENT_ARMED_DYNAMIC:
        case ENT_ARMED:
        case ENT_POWERUPSTATION:
            if (IsUnmanned(pObject))
            {
                if (distance > 50.0f)
                {
                    return;
                }
            }
            else if (distance > 200.0f)
            {
                scale = 2.0f;
            }
            break;
        case ENT_MINE:
        case ENT_POWERUP:
        case ENT_POST:
        case ENT_TRAP:
            if (distance > 35.0f)
            {
                return;
            }
            break;
        case ENT_BUILDING:
            break;
        default:
            return;
        }
    }

    float priority = 0.0f;
    if (distance > 1.0f && scale > 0.0f && !deadSystem)
    {
        priority = distance;
        float facing = DotProduct(offset, sendFront);
        if (facing < 0.0f)
        {
            priority *= canTurn ? 3.0f : 1.0f;
        }
        facing = (float)AbsFloat(facing);
        if (canTurn && facing > 0.0001f)
        {
            if (distance > 0.0f)
            {
                offset = offset / distance;
            }
            PblVector3 side(sendFront.GetZ(), 0.0f, -sendFront.GetX());
            float sideways = (float)AbsFloat(DotProduct(offset, side));
            // GLOBAL: SWBF2 0x01faab30
            static float sSideLimit = sqrtf(0.5f);
            if (sideways / facing > sSideLimit)
            {
                priority = sideways * 2.0f + priority;
            }
        }
    }
    priority = priority * scale;

    int i;
    for (i = 0; i < sendObjCount; ++i)
    {
        if (sendScoreList[i] > priority)
        {
            break;
        }
    }
    if (sendObjCount < 64)
    {
        ++sendObjCount;
    }
    for (int j = sendObjCount - 1; j > i; --j)
    {
        sendObjList[j] = sendObjList[j - 1];
        sendScoreList[j] = sendScoreList[j - 1];
    }
    if (i < 64)
    {
        sendObjList[i] = pObject;
        sendScoreList[i] = priority;
    }
}

// FUNCTION: SWBF2 0x005bd6a0
void FinishSend(GameObject** ppSends, GameObject* (*ppScopes)[64], int*)
{
    int i, j;
    PblHashTable<GameObject, 512> added;
    bool hasGuest = NetGame::GetMyGuest(curDst, 0) != -1;
    int count = 0;
    if (hasGuest)
    {
        for (i = 0; i < 64; ++i)
        {
            for (j = 0; j < 2; ++j)
            {
                GameObject* pObject = ppScopes[j][i];
                if (pObject && count < 64 && !added.Find((unsigned int)pObject))
                {
                    ppSends[count++] = pObject;
                    added.Store((unsigned int)pObject, pObject);
                }
            }
        }
        sendObjCount = count;
    }
    else
    {
        memcpy(ppSends, ppScopes[0], sendObjCount * sizeof(GameObject*));
    }

    sendObjList = ppSends;
    for (i = 0; i < sendObjCount; ++i)
    {
        GameObject* pObject = sendObjList[i];
        if (pObject)
        {
            pObject->AddSends(AddSendBefore, i);
        }
    }
    j = 0;
    for (i = 0; i < sendObjCount; ++i)
    {
        if (sendObjList[i])
        {
            if (i != j)
            {
                sendObjList[j] = sendObjList[i];
            }
            if (++j >= 63)
            {
                break;
            }
        }
    }
    sendObjCount = j;
    int sendCount = sendObjCount;

    int alwaysLeft = 0;
    int alwaysCount = 0;
    GameObject* always[64];
    memset(always, 0, sizeof(always));
    sendObjList = always;
    sendObjCount = 0;
    PblList<FlagItem>::Itor iter = FlagItem::sList->GetFirst();
    while ((FlagItem*)iter)
    {
        FlagItem& flag = *iter;
        AddSendBefore(flag.GetGameObject(), sendObjCount);
        if (flag.GetCarrier())
        {
            AddSendBefore(flag.GetCarrier(), sendObjCount);
        }
        ++iter;
    }
    CommandPostItor it = CommandPostManager::Instance()->GetItor();
    for (; it; ++it)
    {
        CommandPost* pPost = *it;
        GameObject* pObject = pPost->GetGameObject();
        if (!pObject)
        {
            continue;
        }
        if (pObject->GetEntityType() == ENT_POST_HOVER ||
             pObject->GetEntityType() == ENT_POST_WALKER ||
             pObject->GetEntityType() == ENT_POST_FLYER ||
             pObject->GetMaxHealth() < FLT_MAX)
        {
            AddSendBefore(pObject, sendObjCount);
        }
    }
    for (i = 0; i < 32; ++i)
    {
        GameObject* pActor = static_cast<GameObject*>(TargetManager::gMarkers[i].actor);
        if (!pActor)
        {
            continue;
        }
        AddSendBefore(pActor, sendObjCount);
    }
    for (i = 0; i < sendObjCount; ++i)
    {
        GameObject* pObject = sendObjList[i];
        if (pObject)
        {
            pObject->AddSends(AddSendBefore, i);
        }
    }
    j = 0;
    for (i = 0; i < sendObjCount; ++i)
    {
        if (sendObjList[i])
        {
            if (i != j)
            {
                sendObjList[j] = sendObjList[i];
            }
            if (++j >= 63)
            {
                break;
            }
        }
    }
    sendObjCount = j;
    alwaysLeft = sendObjCount;
    alwaysCount = alwaysLeft;

    for (i = 0; i < sendCount && i < 64 - alwaysLeft - 1; ++i)
    {
        for (j = 0; j < alwaysCount; ++j)
        {
            if (ppSends[i] && always[j] && ppSends[i] == always[j])
            {
                always[j] = 0;
                --alwaysLeft;
                break;
            }
        }
    }
    i = Min(sendCount, 64 - alwaysLeft - 1);
    for (j = 0; j < alwaysCount; ++j)
    {
        if (always[j])
        {
            ppSends[i++] = always[j];
        }
    }
    sendObjCount = i;
    sendObjList = ppSends;
}

extern int gSoldierLodCount;

// GLOBAL: SWBF2 0x01e62f60
BblList<HandleToIndex, 128>* handlePoolPtr;

// GLOBAL: SWBF2 0x01e64b44
unsigned int netHostTraceHandleId;

// GLOBAL: SWBF2 0x007ec500
unsigned int debugWriteHostTurn = 0x12ab;

// GLOBAL: SWBF2 0x007ec4d4
int debugWriteObjectOrdinal = 15;

// GLOBAL: SWBF2 0x007ec4d8
unsigned int debugWriteObjectTurn = 0x354;

// FUNCTION: SWBF2 0x005cc720
void WriteObjects(NetPktGroup& group)
{
    GameObject* scopes[2][64];
    float priorities[64];
    GameObject* sends[64];
    int scopeCounts[2];
    unsigned int i;
    unsigned int j;

    gSoldierLodCount = 0;
    if ((unsigned int)hostTurnNumber == debugWriteHostTurn)
    {
        ++gDebugCount;
    }
    PblVector3 center(0.0f, 0.0f, 0.0f);
    PblVector3 front(0.0f, 0.0f, 1.0f);
    float zoom = 1.0f;

    if (!isFullState)
    {
        GameObject* nearObjects[512];
        int removed[64];
        memset(scopes, 0, sizeof(scopes));
        memset(sends, 0, sizeof(sends));
        for (j = 0; j < 2; ++j)
        {
            scopeCounts[j] = 0;
        }

        int viewer = curDst;
        int spectated = NetGame::GetSpectator(curDst);
        if (spectated != -1)
        {
            viewer = spectated;
        }
        Tracker* tracker = Tracker::GetTracker(viewer);
        Trackable* trackable = tracker->GetTrackable();
        if (trackable && trackable != (Trackable*)-1 &&
            trackable->GetTracker() != (Tracker*)0xdddddddd)
        {
            trackable->SetupCameraZoomFactor(zoom);
            GameObject* pObject = trackable->GetGameObject();
            const PblMatrix& matrix = pObject->GetMatrix();
            center = matrix.GetTranslation();
            front = matrix.GetZAxis();
        }
        else
        {
            center = netPlayers[curDst].spawnCenter;
            front = netPlayers[curDst].spawnFront;
        }
        ResetSend(scopes[0], center, front, priorities);
        unsigned int nearCount = GetObjectsInScope(center, nearObjects, 512);
        for (i = 0; i < nearCount; ++i)
        {
            AddSend(nearObjects[i]);
        }
        scopeCounts[0] = sendObjCount;

        for (j = 0; j < 1; ++j)
        {
            int guest = NetGame::GetMyGuest(curDst, j);
            if (guest >= 0)
            {
                center = PblVector3(0.0f, 0.0f, 0.0f);
                front = PblVector3(0.0f, 0.0f, 1.0f);
                tracker = Tracker::GetTracker(guest);
                trackable = tracker->GetTrackable();
                if (trackable && trackable->GetTracker() &&
                    trackable->GetTracker() != (Tracker*)0xdddddddd)
                {
                    GameObject* pObject = trackable->GetGameObject();
                    const PblMatrix& matrix = pObject->GetMatrix();
                    center = matrix.GetTranslation();
                    front = matrix.GetZAxis();
                }
                ResetSend(scopes[j + 1], center, front, priorities);
                nearCount = GetObjectsInScope(center, nearObjects, 512);
                for (i = 0; i < nearCount; ++i)
                {
                    AddSend(nearObjects[i]);
                }
                scopeCounts[j + 1] = sendObjCount;
            }
        }
        gCameraFrontPos = center + front * 20.0f;
        PblValues::Record(gCameraFrontPos.GetX(), PVT_FULL);
        FinishSend(sends, scopes, scopeCounts);

        NetObjStateMap* lastMap = netPlayers[curDst].lastObjState;
        for (i = 0; i < 64; ++i)
        {
            NetObjState* state = lastMap->stateMap[i];
            if (state)
            {
                HandleToIndex& entry = handlePoolPtr->Add();
                entry.index = i;
                entry.isNew = false;
                GameObject* pObject = static_cast<GameObject*>(state->handle);
                if (pObject)
                {
                    handleToIndexMapPtr->Store(pObject->GetHandleId(), &entry);
                }
                else
                {
                    ++gDebugCount;
                }
            }
        }
        for (i = 0; i < (unsigned int)GetSendObjCount(); ++i)
        {
            GameObject* pObject = sends[i];
            PblValues::Record(pObject->GetHandleId(), PVT_FULL);
            HandleToIndex* entry = handleToIndexMapPtr->Find(pObject->GetHandleId());
            if (entry)
            {
                NetObjState* state = lastMap->Find(entry->index);
                state->usedTurn = hostTurnNumber;
            }
        }

        unsigned int removedCount = 0;
        for (i = 0; i < 64; ++i)
        {
            NetObjState* state = lastMap->stateMap[i];
            if (state && state->usedTurn != (unsigned int)hostTurnNumber)
            {
                removed[removedCount++] = i;
                if (state->handle.GetHandleId() == netHostTraceHandleId)
                {
                    netTraceIndex = -1;
                }
            }
        }
        int written;
        if (!removedCount)
        {
            group.Write(false);
        }
        else
        {
            bool isSoldier[64];
            group.Write(true);
            written = 0;
            for (i = 0; i < removedCount; ++i)
            {
                int index = removed[i];
                NetObjState* state = lastMap->stateMap[index];
                EntityClass* entityClass = Factory<Entity, EntityClass, EntityDesc>::FindNetIndex(state->classIdx);
                isSoldier[index] = entityClass->IsRtti(EntitySoldierClass::GetRtti());
                if (isSoldier[index])
                {
                    PblValues::Record(index, PVT_FULL);
                    group.WriteBits(index, 6);
                    bool deleted = static_cast<GameObject*>(state->handle) == 0;
                    group.Write(deleted);
                    ++written;
                }
            }
            for (i = 0; i < removedCount; ++i)
            {
                int index = removed[i];
                NetObjState* state = lastMap->stateMap[index];
                if (!isSoldier[index])
                {
                    PblValues::Record(index, PVT_FULL);
                    group.WriteBits(index, 6);
                    bool deleted = static_cast<GameObject*>(state->handle) == 0;
                    group.Write(deleted);
                    ++written;
                }
            }
            group.WriteBits(63, 6);
        }

        bool wroteNew = false;
        unsigned int freeIndex = 0;
        written = 0;
        for (i = 0; i < (unsigned int)GetSendObjCount(); ++i)
        {
            GameObject* pObject = sends[i];
            EntityType entType = pObject->GetEntityType();
            if (IsNeverDies(entType))
            {
                continue;
            }
            HandleToIndex* entry = handleToIndexMapPtr->Find(pObject->GetHandleId());
            bool isNew = true;
            if (entry)
            {
                NetObjState* state = lastMap->Find(entry->index);
                if ((state->handle == pObject))
                {
                    isNew = false;
                }
            }
            if (isNew)
            {
                if (!wroteNew)
                {
                    wroteNew = true;
                    group.Write(wroteNew);
                }
                while (true)
                {
                    NetObjState* state = lastMap->stateMap[freeIndex];
                    if (!state || state->usedTurn != (unsigned int)hostTurnNumber)
                    {
                        break;
                    }
                    PblValues::Record(freeIndex & 64, PVT_FULL);
                    ++freeIndex;
                }
                if (freeIndex >= 63)
                {
                    __debugbreak();
                }
                PblValues::Record(freeIndex, PVT_FULL);
                group.WriteBits(freeIndex, 6);
                entry = &handlePoolPtr->Add();
                entry->index = freeIndex++;
                entry->isNew = true;
                handleToIndexMapPtr->Store(pObject->GetHandleId(), entry);
                if (pObject->GetHandleId() == netHostTraceHandleId)
                {
                    netTraceIndex = entry->index;
                }
                WriteCreate(group, pObject);
                ++written;
            }
            if (saveObjState)
            {
                NetObjState* state = new NetObjState;
                state->handle = pObject;
                if (isNew)
                {
                    state->position = pObject->GetMatrix().GetTranslation();
                    state->team = (unsigned char)pObject->GetTeam();
                    state->classIdx = (unsigned char)pObject->GetEntityClass()->GetNetIndex();
                    SetHostWorldIndex(entry->index, pObject);
                }
                else
                {
                    NetObjState* lastState = netPlayers[curDst].lastObjState->Find(entry->index);
                    state->position = pObject->GetMatrix().GetTranslation();
                    state->team = lastState->team;
                    state->classIdx = (unsigned char)pObject->GetEntityClass()->GetNetIndex();
                }
                PblValues::Record(entry->index, PVT_FULL);
                PblValues::Record(pObject->GetHandleId(), PVT_FULL);
                netPlayers[curDst].nextObjState->Add(state, entry->index);
            }
        }
        if (!wroteNew)
        {
            group.Write(wroteNew);
        }
        else
        {
            group.WriteBits(63, 6);
        }

        unsigned int start = 6;
        for (int pass = 0; pass < 2; ++pass)
        {
            int best = -1;
            int bestSkips = 3;
            for (i = start; i < (unsigned int)GetSendObjCount(); ++i)
            {
                GameObject* pObject = sends[i];
                HandleToIndex* entry = handleToIndexMapPtr->Find(pObject->GetHandleId());
                if (!entry)
                {
                    continue;
                }
                int skips = netPlayers[curDst].skippedSends[entry->index];
                if (skips > bestSkips)
                {
                    best = i;
                    bestSkips = netPlayers[curDst].skippedSends[entry->index];
                }
            }
            if (best == -1)
            {
                break;
            }
            GameObject* pBest = sends[best];
            for (i = best; i > start; --i)
            {
                sends[i] = sends[i - 1];
            }
            sends[i] = pBest;
            start += 2;
        }

        int maxBytes = netUpdateSize * 800 / 1024;
        if (hostCommProblems)
        {
            maxBytes = 0;
        }
        if (!netWaitLate && IsMissingMoves(curDst))
        {
            maxBytes = 0;
        }
        int bytes = 0;
        written = 0;
        for (i = 0; i < (unsigned int)GetSendObjCount(); ++i)
        {
            if (written == debugWriteObjectOrdinal && (unsigned int)hostTurnNumber == debugWriteObjectTurn)
            {
                ++gDebugCount;
            }
            bytes = group.GetBytes();
            if (bytes > maxBytes)
            {
                break;
            }
            GameObject* pObject = sends[i];
            WriteObject(group, pObject);
            MarkWriteAlign(group);
            ++written;
            HandleToIndex* entry = handleToIndexMapPtr->Find(pObject->GetHandleId());
            if (entry)
            {
                netPlayers[curDst].skippedSends[entry->index] = 0;
            }
            netPlayers[curDst].nextUpdateTurn = hostTurnNumber;
        }
        for (; i < (unsigned int)GetSendObjCount(); ++i)
        {
            GameObject* pObject = sends[i];
            HandleToIndex* entry = handleToIndexMapPtr->Find(pObject->GetHandleId());
            if (entry)
            {
                ++netPlayers[curDst].skippedSends[entry->index];
            }
        }
        ++gDebugCount;
    }
    else
    {
        ResetSend(sends, center, front, priorities);
        NetEntityList::Itor it(*netEntities);
        while (it)
        {
            NetEntity& pEntity = *it;
            ++it;
            CollisionObject* pCollision = pEntity.obj->GetCollisionObject();
            if (!pCollision || !pCollision->IsInCollisionList())
            {
                continue;
            }
            GameObject* pObject = pEntity.obj;
            EntityType entType = pObject->GetEntityType();
            if (entType == ENT_SOLDIER)
            {
                AddSendFull(pEntity.obj);
            }
        }
        it = NetEntityList::Itor(*netEntities);
        while (it)
        {
            NetEntity& pEntity = *it;
            ++it;
            CollisionObject* pCollision = pEntity.obj->GetCollisionObject();
            if (!pCollision || !pCollision->IsInCollisionList())
            {
                continue;
            }
            GameObject* pObject = pEntity.obj;
            EntityType entType = pObject->GetEntityType();
            if (entType != ENT_SOLDIER)
            {
                AddSendFull(pEntity.obj);
            }
        }
        for (i = 0; i < (unsigned int)GetSendObjCount(); ++i)
        {
            GameObject* pObject = sends[i];
            WriteObject(group, pObject);
            MarkWriteAlign(group);
        }
    }
    WriteNullObject(group);
}

void SendNetEvents(NetPktGroup& group);

// FUNCTION: SWBF2 0x005bb100
void WriteUpdate(NetPktGroup& group)
{
    BeginNetLOD();
    if (!isFullState || curDst == -1)
    {
        if (isFullState)
        {
            group.WriteBits(hostTurnNumber, 32);
        }
        else
        {
            WriteTurnLowBits(group, hostTurnNumber);
        }
        WriteDestMask(group, hostPlayingMask);
        NetSetup::WriteGameUpdate(group);
        if (curDst >= 0)
        {
            hostCommProblems = NetComm::GetLastRecv(curDst) < NetComm::GetTime() - 0.5f;
            group.Write(hostCommProblems);
            bool quit = GameLoop::IsQuitRequested() || netPlayers[curDst].requestedQuit;
            group.Write(quit);
            if (quit)
            {
                bool requested = netPlayers[curDst].requestedQuit;
                group.Write(requested);
            }
            group.Write(netPlayers[curDst].mBooted);
        }
        else
        {
            hostCommProblems = false;
        }
    }
    if (curDst >= 0)
    {
        group.WriteBits(GetLag(curDst) + 128, 8);
        unsigned int clientTurnDelta = hostTurnNumber - netPlayers[curDst].mLastReceivedTurn + 128;
        group.WriteBits(clientTurnDelta, 8);
        NetPlayer& netPlayer = netPlayers[curDst];
        if (!isFullState)
        {
            if (netPlayer.nextObjState->turn == (unsigned int)-1 &&
                (netPlayer.lastObjState->turn == (unsigned int)-1 ||
                 netPlayer.lastObjState->turn < hostTurnNumber + netTurnsPerSecond))
            {
                netPlayer.nextObjState->turn = hostTurnNumber;
            }
            WriteSwitchObjRequest(group);
        }
        else
        {
            WriteTurnLowBits(group, netPlayer.clientTurn);
            WriteSwitchResponse(group, netPlayer.objSwitchRelay.switchResponse,
                                netPlayer.objSwitchRelay.switchTurn);
        }
    }

    BblList<HandleToIndex, 128> handlePool;
    PblHashTable<HandleToIndex, 128> handleToIndexMap;
    handlePoolPtr = &handlePool;
    handleToIndexMapPtr = &handleToIndexMap;
    if (!isFullState || curDst == -1)
    {
        WriteObjects(group);
        MarkWriteAlign(group);
    }
    WriteSpectatorInfo(group, curDst);
    AtleastOnce::WriteAll(group);
    if (curDst >= 0)
    {
        WritePlayerMoves(group);
        MarkWriteAlign(group);
        WritePingData(group);
        MarkWriteAlign(group);
        WriteCaptureDisplay(group, curDst);
        for (unsigned int guestIdx = 0; guestIdx < 1; ++guestIdx)
        {
            int guestDst = NetGame::GetMyGuest(curDst, guestIdx);
            if (guestDst >= 0)
            {
                WriteCaptureDisplay(group, guestDst);
            }
        }
        MarkWriteAlign(group);
        StatsManager::WriteStatsChallenge(curDst, group);
        WriteSpawnDisplay(group);
        MarkWriteAlign(group);
        PAManager::WriteObjects(group);
        MarkWriteAlign(group);
        SendReliableNetEvents(group);
        MarkWriteAlign(group);
        NetGame::SendChatMessageToClient(group);
        SendNetEvents(group);
    }
    if (!isFullState)
    {
        WriteUploadAck(group);
    }
    MarkWriteAlign(group);
}

// FUNCTION: SWBF2 0x005bede0
void SendNetEvents(NetPktGroup& group)
{
    PblHeap<int, float>::RECORD records[65];
    PblHeap<int, float> heap(64, 1000000.0f, records);
    int i = (int)netPlayers[curDst].netEventTail;
    while (i != netEventHead)
    {
        float fScore = GetNetEventScore(sServerNetEvents[i]);
        if (fScore >= 0.0f)
        {
            heap.Push(i, -fScore);
            if (heap.IsFull())
            {
                break;
            }
        }
        i = (i + 1) % 128;
    }
    int iLimit = netUpdateSize;
    iLimit -= 8;
    if (hostCommProblems)
    {
        iLimit = 0;
    }
    while (!heap.IsEmpty())
    {
        if (!isFullState && group.GetBytes() >= iLimit)
        {
            break;
        }
        WriteNetEvent(sServerNetEvents[heap.Get(0)], group);
        heap.Pop();
    }
    netPlayers[curDst].netEventTail = netEventHead;
    group.Write(false);
}

// GLOBAL: SWBF2 0x007ec4e0

int debugSendUpdateTurn = 151;

// FUNCTION: SWBF2 0x005bae80
void SendUpdate(int iPlayer)
{
    if (iPlayer == 0)
    {
        ++gDebugCount;
        if (hostTurnNumber == debugSendUpdateTurn)
        {
            ++gDebugCount;
        }
    }
    ++netPlayers[iPlayer].updateCount;
    ++netPlayers[iPlayer].packetCount;
    isFullState = false;
    curDst = iPlayer;
    NetPktGroup group;
    group.BeginWrite();
    BeginWriteAlign(group);
    WriteUpdate(group);
    group.EndWrite();
    int iBytes = group.GetBytes();
    int iBytes2 = group.GetBytes();
    (void)iBytes;
    (void)iBytes2;
    if (NetGame::IsOneWorld() &&
        (curDst == NetGame::GetLocalIndex(0) || curDst == NetGame::GetLocalIndex(1)))
    {
        FakeSendOneWorld(group);
    }
    else
    {
        NetComm::Send(curDst, PKT_GAME_UPDATE, &group);
    }
}

// FUNCTION: SWBF2 0x005c8c80
void SendToClients()
{
    if (loadGame)
    {
        return;
    }
    int iPasses = netCurMaxPlayers / 2;
    bool bTicket[2];
    bool bFull[2];
    int i;
    for (i = 0; i < 2; ++i)
    {
        bTicket[i] = NetComm::IsTicketReady((TicketHolder)0, (PipeType)i);
    }
    while (true)
    {
        for (i = 0; i < 2; ++i)
        {
            bFull[i] = NetComm::IsPipeFull((PipeType)i, netUpdateSize);
        }
        int iBest = -1;
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            if (!NetComm::IsConnected(i))
            {
                continue;
            }
            if (!NetGame::IsPlaying(i, false))
            {
                continue;
            }
            if (NetGame::GetGuestOf(i) >= 0)
            {
                continue;
            }
            if (netPlayers[i].clientOfIndex != netHostIndex)
            {
                continue;
            }
            if ((unsigned int)hostTurnNumber < netPlayers[i].ackedUpdateTurn)
            {
                continue;
            }
            PipeType ePipe = NetComm::GetPipe(i);
            if (!bTicket[ePipe])
            {
                continue;
            }
            if (bFull[ePipe])
            {
                continue;
            }
            if (!IsSendWindowOpen(i))
            {
                continue;
            }
            if (GameLoop::IsGameOver() && netPlayers[i].hasStats)
            {
                continue;
            }
            if (iBest == -1)
            {
                iBest = i;
            }
            else if (netPlayers[i].ackedUpdateTurn < netPlayers[iBest].ackedUpdateTurn)
            {
                iBest = i;
            }
            else if (netPlayers[i].ackedUpdateTurn == netPlayers[iBest].ackedUpdateTurn &&
                     (PblRandom::GetGlobalInt() & 0x100))
            {
                iBest = i;
            }
        }
        if (iBest == -1)
        {
            break;
        }
        IsNetObjectFeatureEnabled();
        SendUpdate(iBest);
        SentUpdate(iBest);
        if (--iPasses == 0)
        {
            break;
        }
    }
    float fNow = NetComm::GetTime();
    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!NetComm::IsConnected(i))
        {
            continue;
        }
        if (!(GameLoop::IsGameOver() && netPlayers[i].hasStats) && NetGame::IsPlaying(i, false))
        {
            continue;
        }
        if (NetComm::GetLastRecv(i) < fNow - 3.0f)
        {
            continue;
        }
        if (netPlayers[i].nextStats > fNow)
        {
            continue;
        }
        netPlayers[i].nextStats = fNow + 0.2f + netRandom.GetFloat() * 0.5f;
        NetPktGroup group;
        group.BeginWrite();
        bool bPlaying = NetGame::IsPlaying(i, false);
        group.Write(bPlaying);
        NetGame::SendChatMessageToClient(group);
        group.EndWrite();
        NetComm::Send(i, PKT_STATS_UPDATE, &group);
    }
    for (i = 0; i < 2; ++i)
    {
        if (bTicket[i])
        {
            NetComm::UseTicket((TicketHolder)0, (PipeType)i);
            NetComm::GetTicket((TicketHolder)0, (PipeType)i);
        }
    }
}

// FUNCTION: SWBF2 0x005c9030
void SendToRelays()
{
    if (netHosts[netHostIndex].hostMode)
    {
        return;
    }
    unsigned int uMask = 0;
    int i;
    for (i = 0; i < netCurMaxHosts; ++i)
    {
        if (!NetComm::IsConnected(i + 64))
        {
            continue;
        }
        if (netHosts[i].hostMode != 1)
        {
            continue;
        }
        uMask |= 1 << i;
    }
    if (!uMask)
    {
        return;
    }
    unsigned int uTurn = 0xffffffff;
    for (i = 0; i < netCurMaxHosts; ++i)
    {
        if (!(uMask & (1 << i)))
        {
            continue;
        }
        if (netHosts[i].hostAckedUpdateTurn < uTurn)
        {
            uTurn = netHosts[i].hostAckedUpdateTurn;
        }
    }
    unsigned int uCap = relayTurnNumber + netTurnsPerSecond / 3;
    if ((unsigned int)hostTurnNumber > uCap)
    {
        uTurn = uCap;
    }
    if (uTurn < (unsigned int)relayTurnNumber)
    {
        return;
    }

    NetPktGroup group;
    group.BeginWrite();
    BeginWriteAlign(group);
    isFullState = true;
    curDst = -1;
    saveObjState = false;
    WriteUpdate(group);
    MarkWriteAlign(group);
    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        if (hostPlayingMask & (1ui64 << i))
        {
            curDst = i;
            WriteUpdate(group);
            MarkWriteAlign(group);
        }
    }
    MarkWriteAlign(group);
    group.EndWrite();
    int iCount = group.pktCount;
    int iTurn16 = hostTurnNumber & 0xffff;
    NetPacket* pPacket = group.GetFront();
    for (i = 0; i < iCount; ++i)
    {
        SetGroup(pPacket, iTurn16, i, iCount);
        pPacket = pPacket->GetNext();
    }
    i = 0;
    while ((pPacket = group.PopFront()) != 0)
    {
        for (;;)
        {
            if (i >= netCurMaxHosts)
            {
                i = 0;
            }
            if (uMask & (1 << i))
            {
                break;
            }
            ++i;
        }
        NetComm::Send(i + 64, PKT_GAME_RELAY_UPDATE, pPacket);
    }
    relayTurnNumber = hostTurnNumber;
}

// FUNCTION: SWBF2 0x005c2270
void NetGame::CreateEvent_AddObjective(HUD::Objective*) {}

// FUNCTION: SWBF2 0x005c4640
void NetGame::EnableJournal() {}

// FUNCTION: SWBF2 0x005c49c0
void NetGame::EnablePlayback() {}

// GLOBAL: SWBF2 0x007e8238
int netHeroRulePlayer = 1;

// FUNCTION: SWBF2 0x005d45f0
void ProcessGalaxyData()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    pGalaxy->ProcessData();
}

// GLOBAL: SWBF2 0x01e65030
bool netLoginPending;

// FUNCTION: SWBF2 0x0055e330
int GetSpaceAssaultScoreLimit() { return Lua_Callbacks::gSpaceAssaultScoreLimit[Lua_Callbacks::gCurRules]; }




// SYNTHETIC: SWBF2 0x00405570
// `dynamic initializer for 'frameLockTimeBase''

// GLOBAL: SWBF2 0x01ee76c8
// frameLockTimeBase
PblTimeBase frameLockTimeBase;

// SYNTHETIC: SWBF2 0x004044d0
// PblMath::`dynamic initializer for 'fTwoPi''

// SYNTHETIC: SWBF2 0x004044f0
// PblMath::`dynamic initializer for 'fHalfPi''

// SYNTHETIC: SWBF2 0x00404510
// PblMath::`dynamic initializer for 'fOneOverPi''

// SYNTHETIC: SWBF2 0x00404530
// PblMath::`dynamic initializer for 'fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x00404550
// PblMath::`dynamic initializer for 'fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00404570
// `dynamic initializer for '_IdentityMatrix''

// SYNTHETIC: SWBF2 0x00404680
// `dynamic initializer for 'TRIGGER_TIMER_SECONDS_PER_NETTICK''

// SYNTHETIC: SWBF2 0x004046a0
// `dynamic initializer for 'TRIGGER_TIMER_TICKS_PER_SECOND''

// SYNTHETIC: SWBF2 0x004046c0
// `dynamic initializer for 'TRIGGER_TIMER_SECONDS_PER_TICK''

// SYNTHETIC: SWBF2 0x004046e0
// `dynamic initializer for 'MAX_TAP_DURATION_TICKS''

// SYNTHETIC: SWBF2 0x00404710
// `dynamic initializer for 'MAX_DOUBLETAP_INTERVAL_TICKS''



// FUNCTION: SWBF2 0x005c5cc0
AloMapDisplayMarkers::AloMapDisplayMarkers()
{
    sig = ALO_MAPDISPLAY_MARKERS;
    deltaSends = false;
    Reset();
}

// SYNTHETIC: SWBF2 0x004053e0
// `dynamic initializer for 'aloMapDisplayMarkers''
// SYNTHETIC: SWBF2 0x00769170
// `dynamic atexit destructor for 'aloMapDisplayMarkers''

// GLOBAL: SWBF2 0x01ed8198
// aloMapDisplayMarkers
AloMapDisplayMarkers aloMapDisplayMarkers;

// SYNTHETIC: SWBF2 0x00405580
// `dynamic initializer for 'frameLockStamp''

// GLOBAL: SWBF2 0x01ee7440
// frameLockStamp
PblTimeBase::Stamp frameLockStamp;

// SYNTHETIC: SWBF2 0x005d3440
// RecentGuestObjectRecord::RecentGuestObjectRecord

// SYNTHETIC: SWBF2 0x00405590
// `dynamic initializer for 'recentGuestObjects''
// SYNTHETIC: SWBF2 0x00769190
// `dynamic atexit destructor for 'recentGuestObjects''

// GLOBAL: SWBF2 0x01ed7898
// recentGuestObjects
RecentGuestObjectRecord recentGuestObjects[64][3];

// GLOBAL: SWBF2 0x01e62f12
bool netAutoAim = false;
// GLOBAL: SWBF2 0x01e62f38
bool netDropLagJumps = false;
// GLOBAL: SWBF2 0x01e62f39
bool netSplitUpdate = false;

// GLOBAL: SWBF2 0x01faa718
bool journalNeedsClose = false;
// GLOBAL: SWBF2 0x007e8e50
bool netShowNames = true;
// GLOBAL: SWBF2 0x01e62f3c
int netLagVersion = 0;
// GLOBAL: SWBF2 0x01e64f00
int netMinPlayersToAutoLaunchDedicated = 0;
// GLOBAL: SWBF2 0x007e825c
int netPlayerLimit = 64;

// GLOBAL: SWBF2 0x007e8e54
int netDedicatedPlayerLimit = 64;
// GLOBAL: SWBF2 0x007e8e58
int netTraceDst = 4;
// GLOBAL: SWBF2 0x007e8e5c
int hostVoteThreshold = 50;

// GLOBAL: SWBF2 0x007ec4f8
int debugPredictClientTurn = 1181;

// GLOBAL: SWBF2 0x007ec504
int debugPredictStepCount = 4;
// GLOBAL: SWBF2 0x01e62ed4
float spHeroSelectedTime[3] = { 0.0f, 0.0f, 0.0f };

// FUNCTION: SWBF2 0x005c4600
bool NetGame::ErrorDismissed()
{
    return NetSetup::IsErrorDismissed();
}

// FUNCTION: SWBF2 0x005c5080
void NetGame::Reboot()
{
}

// FUNCTION: SWBF2 0x005c2580
void WriteEntityClass(NetPktGroup& group, const EntityClass* pClass)
{
    const unsigned int value = pClass ? pClass->GetNetIndex() : -1;
    group.WriteBits(value, 8);
}

// FUNCTION: SWBF2 0x005c2600
void WriteWeaponClass(NetPktGroup& group, const WeaponClass* pClass)
{
    const unsigned int value = pClass ? pClass->GetNetIndex() : -1;
    group.WriteBits(value, 8);
}

// FUNCTION: SWBF2 0x005c4440
int NetGame::GetSpawnOrder(int iPlayer)
{
    if (iPlayer >= 0 && iPlayer < netCurMaxPlayers && NetGame::IsPlaying(iPlayer, false))
    {
        return netPlayers[iPlayer].mSpawnOrder;
    }
    return -1;
}
