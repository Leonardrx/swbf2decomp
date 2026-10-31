#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "MissionPlayList.h"
#include "GameLoop.h"
#include "LuaHelper.h"
#include "PlayerProfile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

using namespace MissionPlayList;

// GLOBAL: SWBF2 0x01f9d070
PlaylistEntry sPlaylist[256];
// GLOBAL: SWBF2 0x01e579c8
int sPlaylistCount;
// GLOBAL: SWBF2 0x01e579cc
int sPlaylistPosn;

// GLOBAL: SWBF2 0x01e579d0
int MissionPlayList::mFindIndex;
// GLOBAL: SWBF2 0x01e579c4
bool sbLoopForever;
// GLOBAL: SWBF2 0x01e579c5
bool sbRandomize;
// GLOBAL: SWBF2 0x01f9d06c
unsigned int sTeam1Hash;
// GLOBAL: SWBF2 0x01f9d068
unsigned int sTeam2Hash;
// GLOBAL: SWBF2 0x007e7f98
int sEras = -1;

namespace GameLoop
{
extern bool mbHistoricalRules;
}

// FUNCTION: SWBF2 0x005a4410
int MissionPlayList::GetMaxSize()
{
    return sizeof(sPlaylist) / sizeof(sPlaylist[0]);
}

// FUNCTION: SWBF2 0x005a4420
void MissionPlayList::ClearEntries()
{
    memset(sPlaylist, 0, sizeof(sPlaylist));
    sPlaylistCount = 0;
    sPlaylistPosn = 0;
    sbRandomize = false;
    sEras = -1;
}

// FUNCTION: SWBF2 0x005a4460
void MissionPlayList::SetEntry(char const* pcLuaFile, unsigned char uSide, unsigned int uiTeam1Hash,
                               unsigned int uiTeam2Hash, int iTickets1, int iTickets2)
{
    strlcpy(sPlaylist[sPlaylistCount].mLuaFile, pcLuaFile, sizeof(sPlaylist[0].mLuaFile));
    sPlaylist[sPlaylistCount].mSide = uSide;
    sPlaylist[sPlaylistCount].mTeam1Hash = uiTeam1Hash;
    sPlaylist[sPlaylistCount].mTeam2Hash = uiTeam2Hash;
    sPlaylist[sPlaylistCount].mTickets1 = iTickets1;
    sPlaylist[sPlaylistCount].mTickets2 = iTickets2;
    ++sPlaylistCount;
}

// FUNCTION: SWBF2 0x005a44c0
bool MissionPlayList::AddEntry(char const* pcLuaFile, unsigned char uSide, char const* pcTeam1,
                               char const* pcTeam2, int iTickets1, int iTickets2)
{
    if (sPlaylistCount < 256)
    {
        unsigned int uiTeam1Hash = 0;
        if (pcTeam1)
        {
            uiTeam1Hash = PblHash(pcTeam1);
        }
        unsigned int uiTeam2Hash = 0;
        if (pcTeam2)
        {
            uiTeam2Hash = PblHash(pcTeam2);
        }
        SetEntry(pcLuaFile, uSide, uiTeam1Hash, uiTeam2Hash, iTickets1, iTickets2);

        int iEra = -1;
        unsigned int uLen = strlen(pcLuaFile) - 1;
        for (unsigned int i = 0; i < uLen; ++i)
        {
            if (isdigit(pcLuaFile[i]) && isalpha(pcLuaFile[i + 1]))
            {
                char c = pcLuaFile[i + 1] | 0x20;
                if (c == 'c')
                {
                    iEra = 0;
                    break;
                }
                if (c == 'g')
                {
                    iEra = 1;
                    break;
                }
            }
        }

        if (sEras == -1 || sEras == iEra)
        {
            sEras = iEra;
        }
        else
        {
            sEras = 2;
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x005a45b0
void MissionPlayList::SetLoopForever(bool bLoop)
{
    sbLoopForever = bLoop;
}

// FUNCTION: SWBF2 0x005a45c0
int MissionPlayList::GetCount()
{
    return sPlaylistCount;
}

// FUNCTION: SWBF2 0x005a45d0
void MissionPlayList::RandomizeOrder()
{
    sbRandomize = true;
    int iCount = sPlaylistCount;
    if (iCount < 2)
    {
        return;
    }

    for (int i = 0; i < iCount * 7; ++i)
    {
        unsigned int uA = (PblRandom::GetGlobalInt() >> 3) % iCount;
        unsigned int uB = PblRandom::GetGlobalInt() % iCount;
        PlaylistEntry temp = sPlaylist[uA];
        sPlaylist[uA] = sPlaylist[uB];
        sPlaylist[uB] = temp;
    }
}

// FUNCTION: SWBF2 0x005a4760
bool MissionPlayList::PeekNextEntry(char const*& pcLuaFile, unsigned int& uiTeam1Hash,
                                    unsigned int& uiTeam2Hash)
{
    if (sPlaylistPosn >= sPlaylistCount)
    {
        if (sbLoopForever)
        {
            sPlaylistPosn = 0;
            if (sbRandomize)
            {
                RandomizeOrder();
            }
        }
        else
        {
            return false;
        }
    }
    pcLuaFile = sPlaylist[sPlaylistPosn].mLuaFile;
    uiTeam1Hash = sPlaylist[sPlaylistPosn].mTeam1Hash;
    uiTeam2Hash = sPlaylist[sPlaylistPosn].mTeam2Hash;
    return true;
}

// FUNCTION: SWBF2 0x005a47d0
bool MissionPlayList::SelectNextEntry()
{
    if (sPlaylistPosn >= sPlaylistCount)
    {
        if (sbLoopForever)
        {
            sPlaylistPosn = 0;
            if (sbRandomize)
            {
                RandomizeOrder();
            }
        }
        else
        {
            return false;
        }
    }
    GameLoop::SetNextMission(sPlaylist[sPlaylistPosn].mLuaFile);

    const int iSide = sPlaylist[sPlaylistPosn].mSide;
    PlayerPrefs::mAllUserPrefs[1].mTeamNum = GameLoop::mbHistoricalRules ? iSide : PlayerPrefs::mAllUserPrefs[1].mTeamNum;
    PlayerPrefs::mAllUserPrefs[0].mTeamNum = iSide;
    sTeam1Hash = sPlaylist[sPlaylistPosn].mTeam1Hash;
    sTeam2Hash = sPlaylist[sPlaylistPosn].mTeam2Hash;
    PushToLua();
    ++sPlaylistPosn;
    return true;
}

// FUNCTION: SWBF2 0x005a4860
void MissionPlayList::GetCurrentTeamNames(unsigned int& uiTeam1Hash, unsigned int& uiTeam2Hash)
{
    uiTeam1Hash = sTeam1Hash;
    uiTeam2Hash = sTeam2Hash;
}

// FUNCTION: SWBF2 0x005a4870
void MissionPlayList::SetCurrentTeamNames(unsigned int uiTeam1Hash, unsigned int uiTeam2Hash)
{
    if (sTeam1Hash != uiTeam1Hash || sTeam2Hash != uiTeam2Hash)
    {
        sTeam1Hash = uiTeam1Hash;
        sTeam2Hash = uiTeam2Hash;
        PushToLua();
    }
}

// FUNCTION: SWBF2 0x005a48a0
void MissionPlayList::SetClientEntry(char const* pcLuaFile, unsigned int uiTeam1Hash,
                                     unsigned int uiTeam2Hash)
{
    ClearEntries();
    SetEntry(pcLuaFile, 1, uiTeam1Hash, uiTeam2Hash, 0, 0);
}

// FUNCTION: SWBF2 0x005a48d0
static void SetOneName(unsigned int uiHash, const char* pcKey)
{
    const unsigned short* pusName = RedLocalizeDB::Find(uiHash, true);
    if (pusName)
    {
        LuaHelper::TableWriteVal(pcKey, pusName);
        return;
    }
    char acHex[16];
    unsigned short ausHex[16];
    _snprintf(acHex, sizeof(acHex), "%08X", uiHash);
    PblUnicode::ustrncnv(ausHex, acHex, 16);
    LuaHelper::TableWriteVal(pcKey, ausHex);
}

// FUNCTION: SWBF2 0x005a4930
void MissionPlayList::PushToLua()
{
    int iTop = lua_gettop(LuaHelper::mState);
    lua_getglobal(LuaHelper::mState, "ifs_missionselect");
    if (lua_isnil(LuaHelper::mState, -1))
    {
        lua_pop(LuaHelper::mState, 1);
        lua_newtable(LuaHelper::mState);
        lua_setglobal(LuaHelper::mState, "ifs_missionselect");
        lua_getglobal(LuaHelper::mState, "ifs_missionselect");
    }
    SetOneName(sTeam1Hash, "Team1Name");
    SetOneName(sTeam2Hash, "Team2Name");
    lua_pop(LuaHelper::mState, 1);
    (void)iTop;
    lua_gettop(LuaHelper::mState);
}

// FUNCTION: SWBF2 0x005a4a90
int MissionPlayList::GetEntryCount()
{
    return sPlaylistCount;
}

// FUNCTION: SWBF2 0x005a4aa0
int MissionPlayList::GetCurrentEntry()
{
    int iEntry = sPlaylistPosn - 1;
    if (iEntry < 0)
    {
        iEntry = 0;
    }
    return iEntry;
}

// FUNCTION: SWBF2 0x005a4ab0
void MissionPlayList::GetEntry(int iEntry, char const*& pcLuaFile, int& iTickets1, int& iTickets2)
{
    PlaylistEntry& entry = sPlaylist[iEntry];
    pcLuaFile = entry.mLuaFile;
    iTickets1 = entry.mTickets1;
    iTickets2 = entry.mTickets2;
}

// FUNCTION: SWBF2 0x005a4ae0
void MissionPlayList::AddEntry(char const* pcLuaFile, int iTickets1, int iTickets2)
{
    int iCount = sPlaylistCount;
    if (iCount >= 256)
    {
        return;
    }
    for (int i = iCount - 1; i >= sPlaylistPosn; --i)
    {
        sPlaylist[i + 1] = sPlaylist[i];
    }
    sPlaylistCount = sPlaylistPosn;
    AddEntry(pcLuaFile, 1, 0, 0, iTickets1, iTickets2);
    sPlaylistCount = iCount + 1;
}

// FUNCTION: SWBF2 0x005a4ba0
bool MissionPlayList::DropEntry(char const* pcLuaFile)
{
    const int iOldCount = sPlaylistCount;
    if (iOldCount <= 1)
    {
        return false;
    }

    int iDrop = GetCurrentEntry();
    if (pcLuaFile)
    {
        int iEntry = iDrop;
        for (int i = 0; i < iOldCount; ++i, ++iEntry)
        {
            if (iEntry >= iOldCount)
            {
                iEntry = 0;
            }
            char const* pcName;
            int iTickets1, iTickets2;
            GetEntry(iEntry, pcName, iTickets1, iTickets2);
            if (!_stricmp(pcLuaFile, pcName))
            {
                iDrop = iEntry;
                break;
            }
        }
    }

    int iCount = sPlaylistCount - 1;
    for (int i = iDrop; i < iCount; ++i)
    {
        sPlaylist[i] = sPlaylist[i + 1];
    }
    sPlaylistCount = iCount;
    if (sPlaylistPosn > iCount)
    {
        --sPlaylistPosn;
    }
    return true;
}

// FUNCTION: SWBF2 0x005a4c60
bool MissionPlayList::NextEntry(char const* pcLuaFile)
{
    if (!pcLuaFile)
    {
        return false;
    }

    const int iCount = sPlaylistCount;
    int iFound = -1;
    int i = 0;
    int iEntry = GetCurrentEntry();
    for (; i < iCount; ++i, ++iEntry)
    {
        if (iEntry >= iCount)
        {
            iEntry = 0;
        }
        char const* pcName;
        int iTickets1, iTickets2;
        GetEntry(iEntry, pcName, iTickets1, iTickets2);
        if (!_stricmp(pcLuaFile, pcName))
        {
            iFound = iEntry;
            break;
        }
    }
    if (iFound == -1)
    {
        return false;
    }
    sPlaylistPosn = iFound;
    return true;
}

// FUNCTION: SWBF2 0x005a4ce0
void MissionPlayListStepBack()
{
    if (sPlaylistPosn > 0)
    {
        --sPlaylistPosn;
    }
}

// FUNCTION: SWBF2 0x005a4a20
unsigned int MissionPlayList::GetFirstMission()
{
    mFindIndex = 0;
    if (mFindIndex <= sPlaylistCount)
    {
        return PblHash(sPlaylist[mFindIndex].mLuaFile);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005a4a50
unsigned int MissionPlayList::GetNextMission()
{
    ++mFindIndex;
    if (mFindIndex <= sPlaylistCount)
    {
        return PblHash(sPlaylist[mFindIndex].mLuaFile);
    }
    return 0;
}
