#include <string.h>

#include "TargetManager.h"
#include "Character.h"
#include "Controllable.h"
#include "GameObject.h"
#include "CommandPost.h"
#include "HUDGameEvents.h"
#include "LockOnManager.h"
#include "NetGame.h"
#include "Trackable.h"

namespace
{

inline const RedColor Interpolate(const RedColor& a, const RedColor& b, float t)
{
    RedColor result;
    result.b = (unsigned char)(int)(a.b * (1.0f - t) + b.b * t + 0.5f);
    result.g = (unsigned char)(int)(a.g * (1.0f - t) + b.g * t + 0.5f);
    result.r = (unsigned char)(int)(a.r * (1.0f - t) + b.r * t + 0.5f);
    result.a = (unsigned char)(int)(a.a * (1.0f - t) + b.a * t + 0.5f);
    return result;
}

}

namespace TargetManager
{

// GLOBAL: SWBF2 0x01eafba0
ClassMarker _gClassMarker[MAX_CLASS_MARKERS];

// GLOBAL: SWBF2 0x01eafb9c
Marker* _gMarkers;

// SYNTHETIC: SWBF2 0x00406600
// TargetManager::`dynamic initializer for 'gMarkers''

// GLOBAL: SWBF2 0x01f6de9c
// gMarkers
Marker* gMarkers = _gMarkers;

// SYNTHETIC: SWBF2 0x00406520
// TargetManager::`dynamic initializer for '_gPost''

// GLOBAL: SWBF2 0x01f76410
// _gPost
Post _gPost[MAX_POSTS];

// SYNTHETIC: SWBF2 0x00652170
// TargetManager::Target::Target
// SYNTHETIC: SWBF2 0x006521e0
// TargetManager::Target::~Target

// SYNTHETIC: SWBF2 0x004064f0
// TargetManager::`dynamic initializer for '_gTarget''
// SYNTHETIC: SWBF2 0x00769500
// TargetManager::`dynamic atexit destructor for '_gTarget''

// GLOBAL: SWBF2 0x01f76710
// _gTarget
Target _gTarget[MAX_TARGETS];

// SYNTHETIC: SWBF2 0x00406610
// TargetManager::`dynamic initializer for 'gColorTable''

// GLOBAL: SWBF2 0x007ec320
// gColorTable
Color gColorTable[MAX_COLORS] =
{
    { 0x00000000, { 0, 0, 0, 0 } },
    { 0xde020766, RedColor::WHITE },
    { 0x7ea5f49e, RedColor::LTGREY },
    { 0xe41b46af, RedColor::MDGREY },
    { 0x3657edc3, RedColor::DKGREY },
    { 0x568f4ba4, RedColor::BLACK },
    { 0x40f480dc, RedColor::RED },
    { 0x011decbc, RedColor::GREEN },
    { 0x82fbf5cd, RedColor::BLUE },
    { 0x4961533a, RedColor::CYAN },
    { 0x63e629e8, RedColor::MAGENTA },
    { 0x05bf6449, RedColor::YELLOW },
    { 0x45b473eb, Interpolate(RedColor::YELLOW, RedColor::RED, 0.5f) },
};

// FUNCTION: SWBF2 0x006521f0
void Marker::Clear()
{
    memset(this, 0, sizeof(*this));
}

// FUNCTION: SWBF2 0x00652210
void ClassMarker::Clear()
{
    if (classID)
    {
        for (PblHashTable<EntityEx, 1024>::Itor it = EntityEx::mIdMap.Begin();
             (EntityEx*)it != (EntityEx*)EntityEx::mIdMap.End(); ++it)
        {
            if (((EntityEx*)it)->GetEntityClass()->mId == classID)
            {
                RemoveMarker(((EntityEx*)it)->GetGameObject(), 0, teamMask);
            }
        }
        for (int i = 0; i < Character::sCharacterLimit; ++i)
        {
            Character* pCharacter = Character::GetPtr(i);
            Controllable* pUnit = pCharacter->mUnit;
            if (pUnit && pCharacter->mClassPtr->mId == classID)
            {
                RemoveMarker(pUnit->GetGameObject(), 0, teamMask);
            }
        }
    }
    memset(this, 0, sizeof(*this));
}

// FUNCTION: SWBF2 0x00652340
const RedColor& ColorByHash(unsigned int hashID)
{
    for (unsigned int i = 0; i < MAX_COLORS; ++i)
    {
        if (hashID == gColorTable[i].hashID)
        {
            return gColorTable[i].color;
        }
    }
    return gColorTable[0].color;
}

// FUNCTION: SWBF2 0x00652360
unsigned int HashByColor(RedColor& color)
{
    unsigned int uARGB = color.GetARGB();
    for (unsigned int i = 0; i < MAX_COLORS; ++i)
    {
        if (uARGB == gColorTable[i].color.GetARGB())
        {
            return gColorTable[i].hashID;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006523d0
bool UpdateInterface()
{
    if (!NetGame::IsEnabled() || NetGame::IsOneWorld())
    {
        return true;
    }
    bool bHostDedicated = NetGame::IsHost() && NetGame::IsDedicated();
    return !bHostDedicated == NetGame::IsClient();
}

extern Marker _gHostMarkers[MAX_MARKERS];
extern Marker _gClientMarkers[MAX_MARKERS];

// FUNCTION: SWBF2 0x00652d60
void CreateClient()
{
    memset(_gClientMarkers, 0, sizeof(_gClientMarkers));
}

// FUNCTION: SWBF2 0x00652d80
void DestroyClient() {}

// FUNCTION: SWBF2 0x00652d90
void EnterClient()
{
    _gMarkers = _gClientMarkers;
    gMarkers = _gClientMarkers;
}

// FUNCTION: SWBF2 0x00652db0
void LeaveClient()
{
    _gMarkers = _gHostMarkers;
    gMarkers = _gHostMarkers;
}

// FUNCTION: SWBF2 0x00652440
void Init()
{
    memset(_gTarget, 0, sizeof(_gTarget));
    memset(_gPost, 0, sizeof(_gPost));
    memset(_gClassMarker, 0, sizeof(ClassMarker));
    memset(_gHostMarkers, 0, sizeof(_gHostMarkers));
    memset(_gClientMarkers, 0, sizeof(_gClientMarkers));
    _gMarkers = _gHostMarkers;
    gMarkers = _gHostMarkers;
}

// FUNCTION: SWBF2 0x006524b0
void AddTarget(GameObject* const actor, bool alwaysVisibleToFriends)
{
    if (!actor)
    {
        return;
    }
    if (!UpdateInterface())
    {
        return;
    }
    EntityClass* pClass = ((Entity*)actor)->GetEntityClass();
    if (!pClass)
    {
        return;
    }

    CheckObjectClassMarker(actor);

    unsigned int uIndex = MAX_TARGETS;
    for (unsigned int i = 0; i < MAX_TARGETS; ++i)
    {
        if (_gTarget[i].actor == actor)
        {
            uIndex = i;
            break;
        }
        if (uIndex == MAX_TARGETS && !_gTarget[i].actor)
        {
            uIndex = i;
        }
    }

    if (uIndex != MAX_TARGETS)
    {
        Target& target = _gTarget[uIndex];
        target.actor = actor;
        target.alwaysVisibleToFriends = alwaysVisibleToFriends;
        target.mapParams = pClass->mMapParams;
        HUD::Event event(HUD::GameEvents::gPlayerEvents[0].mapRefreshTarget, uIndex);
        event.Send();
    }
}

// FUNCTION: SWBF2 0x00652600
void RemoveTarget(GameObject* const actor)
{
    if (!actor || !UpdateInterface() || !((Entity*)actor)->GetEntityClass())
    {
        return;
    }

    unsigned int i;
    for (i = 0; i < MAX_TARGETS; ++i)
    {
        Target& target = _gTarget[i];
        if (target.actor == actor)
        {
            target.actor = 0;
            target.alwaysVisibleToFriends = false;
            break;
        }
    }

    if (i != MAX_TARGETS)
    {
        HUD::Event event(HUD::GameEvents::gPlayerEvents[0].mapRefreshTarget, i);
        event.Send();
    }
}

// FUNCTION: SWBF2 0x006526a0
void AddClassMarker(unsigned int classID, unsigned int textureHash, float scale,
                    unsigned int teamMask, unsigned int colorHash, bool showHudTarget,
                    bool flashAlpha, bool pulseSize, float pulseStartScale)
{
    ClassMarker* pMarker = 0;
    for (int i = 0; i < MAX_CLASS_MARKERS; ++i)
    {
        if (_gClassMarker[i].classID == classID && _gClassMarker[i].teamMask == teamMask)
        {
            return;
        }
        if (!_gClassMarker[i].classID)
        {
            pMarker = &_gClassMarker[i];
            break;
        }
    }
    if (!pMarker)
    {
        return;
    }

    pMarker->colorHash = colorHash;
    pMarker->showHudTarget = showHudTarget;
    pMarker->scale = scale;
    pMarker->classID = classID;
    pMarker->textureHash = textureHash;
    pMarker->teamMask = teamMask;
    pMarker->flashAlpha = flashAlpha;
    pMarker->pulseSize = pulseSize;
    pMarker->pulseStartScale = pulseStartScale;

    for (PblHashTable<EntityEx, 1024>::Itor it = EntityEx::mIdMap.Begin();
         (EntityEx*)it != (EntityEx*)EntityEx::mIdMap.End(); ++it)
    {
        if (((EntityEx*)it)->GetEntityClass()->mId == classID)
        {
            AddMarker(((EntityEx*)it)->GetGameObject(), 0, textureHash, scale, teamMask,
                      colorHash, showHudTarget, flashAlpha, pulseSize, pulseStartScale);
        }
    }
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pCharacter = Character::GetPtr(i);
        Controllable* pUnit = pCharacter->mUnit;
        if (pUnit && pCharacter->mClassPtr->mId == classID)
        {
            AddMarker(pUnit->GetGameObject(), 0, textureHash, scale, teamMask, colorHash,
                      showHudTarget, flashAlpha, pulseSize, pulseStartScale);
        }
    }
}

// FUNCTION: SWBF2 0x00652890
void RemoveClassMarker(unsigned int classID, unsigned int teamMask)
{
    for (int i = 0; i < MAX_CLASS_MARKERS; ++i)
    {
        if (_gClassMarker[i].classID == classID && (_gClassMarker[i].teamMask & teamMask))
        {
            _gClassMarker[i].Clear();
            return;
        }
    }
}

// FUNCTION: SWBF2 0x006528d0
void AddPost(CommandPost* const post)
{
    if (!post || !UpdateInterface())
    {
        return;
    }
    GameObject* pObject = post->mObject;
    if (!pObject)
    {
        return;
    }
    const EntityClass* pClass = ((const Entity*)pObject)->GetEntityClass();
    if (!pClass)
    {
        return;
    }

    unsigned int uIndex = MAX_POSTS;
    for (unsigned int i = 0; i < MAX_POSTS; ++i)
    {
        if (_gPost[i].actor == post)
        {
            uIndex = i;
            break;
        }
        if (uIndex == MAX_POSTS && !_gPost[i].actor)
        {
            uIndex = i;
        }
    }

    if (uIndex != MAX_POSTS)
    {
        Post& entry = _gPost[uIndex];
        entry.actor = post;
        entry.mapParams = pClass->mMapParams;
        HUD::Event event(HUD::GameEvents::gPlayerEvents[0].mapRefreshPost, uIndex);
        event.Send();
    }
}

// FUNCTION: SWBF2 0x006529b0
void RemovePost(CommandPost* const post)
{
    if (!post || !UpdateInterface())
    {
        return;
    }
    GameObject* pObject = post->mObject;
    if (!pObject)
    {
        return;
    }
    if (!((const Entity*)pObject)->GetEntityClass())
    {
        return;
    }

    unsigned int i;
    for (i = 0; i < MAX_POSTS; ++i)
    {
        Post& entry = _gPost[i];
        if (entry.actor == post)
        {
            entry.actor = 0;
            break;
        }
    }
    HUD::Event event(HUD::GameEvents::gPlayerEvents[0].mapRefreshPost, i);
    event.Send();
}

// FUNCTION: SWBF2 0x00652a40
void AddMarker(GameObject* const actor, const RedRegion* const region, unsigned int textureHash,
               float scale, unsigned int teamMask, unsigned int colorHash,
               bool showHudTarget, bool flashAlpha, bool pulseSize, float pulseStartScale)
{
    if (actor ? region != 0 : region == 0)
    {
        return;
    }

    unsigned int uIndex = MAX_MARKERS;
    unsigned int uTeams = teamMask;
    bool bFound = false;
    for (unsigned int i = 0; i < MAX_MARKERS; ++i)
    {
        Marker& marker = _gMarkers[i];
        if (!marker.teamMask)
        {
            if (uIndex == MAX_MARKERS)
            {
                uIndex = i;
            }
            if (!uTeams)
            {
                break;
            }
            continue;
        }
        if (!((actor && marker.actor == actor) ||
              (region && marker.region == region)))
        {
            continue;
        }

        if (marker.scale == scale && marker.textureHash == textureHash)
        {
            uIndex = i;
            marker.teamMask |= teamMask;
            marker.pulseStartScale = pulseStartScale;
            marker.flashAlpha = flashAlpha;
            marker.pulseSize = pulseSize;
            bFound = true;
            break;
        }
        if (uTeams)
        {
            unsigned int uTaken = marker.teamMask & uTeams;
            if (uTaken)
            {
                marker.teamMask &= ~uTaken;
                uTeams &= ~uTaken;
                if (!marker.teamMask)
                {
                    marker.Clear();
                    if (uIndex == MAX_MARKERS)
                    {
                        uIndex = i;
                    }
                }
                if (!uTeams && uIndex < MAX_MARKERS)
                {
                    break;
                }
            }
        }
    }

    if (!bFound)
    {
        if (uIndex < MAX_MARKERS)
        {
            Marker& marker = _gMarkers[uIndex];
            marker.actor = actor;
            marker.region = const_cast<RedRegion*>(region);
            marker.teamMask = teamMask;
            marker.scale = scale;
            marker.textureHash = textureHash;
            marker.color = ColorByHash(colorHash);
            marker.pulseStartScale = pulseStartScale;
            marker.flashAlpha = flashAlpha;
            marker.pulseSize = pulseSize;
            marker.showHudTarget = showHudTarget;
        }
        else
        {
            uIndex = 0;
        }
    }

    if (actor && showHudTarget)
    {
        LockOnManager::AddTargetCommon(actor, teamMask);
    }
    if (UpdateInterface())
    {
        HUD::Event event(HUD::GameEvents::gPlayerEvents[0].mapRefreshMarker, uIndex);
        event.Send();
    }
}

// FUNCTION: SWBF2 0x00652c30
void RemoveMarker(GameObject* const actor, const RedRegion* const region, unsigned int teamMask)
{
    if ((!actor && !region) || (actor && region))
    {
        return;
    }

    unsigned int uTeams = teamMask;
    Marker* markers = _gMarkers;
    for (unsigned int i = 0; i < MAX_MARKERS; ++i)
    {
        Marker& marker = markers[i];
        if (!((actor && actor == marker.actor) ||
              (region && marker.region == region)))
        {
            continue;
        }

        unsigned int uTaken = marker.teamMask & uTeams;
        if (!uTaken)
        {
            continue;
        }
        uTeams &= ~uTaken;
        unsigned int uRemaining = marker.teamMask & ~uTaken;
        marker.teamMask = uRemaining;
        if (!uRemaining)
        {
            marker.Clear();
        }
        if (!uTeams)
        {
            break;
        }
    }

    if (UpdateInterface() && actor)
    {
        LockOnManager::RemoveTargetCommon(actor, teamMask);
    }
}

// FUNCTION: SWBF2 0x00652ce0
void CheckObjectClassMarker(GameObject* const actor)
{
    for (int i = 0; i < MAX_CLASS_MARKERS; ++i)
    {
        if (_gClassMarker[i].classID == ((Entity*)actor)->GetEntityClass()->mId)
        {
            const ClassMarker& marker = _gClassMarker[i];
            AddMarker(actor, 0, marker.textureHash, marker.scale, marker.teamMask,
                      marker.colorHash, marker.showHudTarget, marker.flashAlpha,
                      marker.pulseSize, marker.pulseStartScale);
            return;
        }
    }
}

}

namespace TargetManager
{

// SYNTHETIC: SWBF2 0x004065a0
// TargetManager::`dynamic initializer for '_gHostMarkers''
// SYNTHETIC: SWBF2 0x00769520
// TargetManager::`dynamic atexit destructor for '_gHostMarkers''

// GLOBAL: SWBF2 0x01f75f10
// _gHostMarkers
Marker _gHostMarkers[MAX_MARKERS];

// SYNTHETIC: SWBF2 0x004065d0
// TargetManager::`dynamic initializer for '_gClientMarkers''
// SYNTHETIC: SWBF2 0x00769540
// TargetManager::`dynamic atexit destructor for '_gClientMarkers''

// GLOBAL: SWBF2 0x01f7c980
// _gClientMarkers
Marker _gClientMarkers[MAX_MARKERS];

}

// SYNTHETIC: SWBF2 0x005c6370
// TargetManager::Marker::Marker
// SYNTHETIC: SWBF2 0x005c63c0
// TargetManager::Marker::~Marker
