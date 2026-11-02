#include "GameMemory.h"

#include "LuaMemory.h"
#include "RedEngineFL/Memory/RedMemory.h"

namespace GameMemory
{
// GLOBAL: SWBF2 0x01e5614c
int ApplicationHeap;
// GLOBAL: SWBF2 0x01e56168
// GamespyHeap
int GamespyHeap;
// GLOBAL: SWBF2 0x01e56160
// RunTimeHeap
int RunTimeHeap;
// GLOBAL: SWBF2 0x01e56164
// TempLoadHeap
int TempLoadHeap;

// GLOBAL: SWBF2 0x01e56144
int LuaHeap;
// GLOBAL: SWBF2 0x01e5613c
int ArtistHeap;

// GLOBAL: SWBF2 0x01e56138
int TEMP_GamespyHeap;
}

// GLOBAL: SWBF2 0x01e56154
char* gArenaStart;
// GLOBAL: SWBF2 0x01e56140
char* gArenaEnd;
// GLOBAL: SWBF2 0x01f9c2b0
char* GamespyHeapstart;
// GLOBAL: SWBF2 0x01f9c2b4
char* GamespyHeapend;
// GLOBAL: SWBF2 0x01e56148
char* g_Heap3Start;
// GLOBAL: SWBF2 0x01e56150
void* gTempHeapBlock;
namespace GameMemory
{

// GLOBAL: SWBF2 0x01e56158
int gTempGamespyBlock;
}
// GLOBAL: SWBF2 0x01e5615c

char* gArtistHeapStart;

// The next kHeapStartAlignment boundary at or after pAddress.
static inline char* AlignHeapStart(const void* pAddress)
{
    return (char*)(((unsigned int)pAddress + (GameMemory::kHeapStartAlignment - 1)) &
                   ~(GameMemory::kHeapStartAlignment - 1));
}

// FUNCTION: SWBF2 0x00533b20
void GameMemory::InitApplicationMemory(char* pStart, char* pEnd)
{
    gArenaStart = pStart;
    gArenaEnd = pEnd;
    gArenaStart = (char*)RedInitHeap(pStart, pEnd, 9);

    if (ApplicationHeap)
    {
        RedDestroyHeap(ApplicationHeap);
        ApplicationHeap = 0;
    }

    char* pAppEnd = gArenaStart + kApplicationHeapSize;
    ApplicationHeap = RedCreateHeap(gArenaStart, pAppEnd, kHeapAlignment, "Application");

    GamespyHeapstart = pAppEnd;
    GamespyHeapend = pAppEnd + kGamespyHeapSize;
    GamespyHeap = RedCreateHeap(pAppEnd, pAppEnd + kGamespyHeapSize, kHeapAlignment, "Gamespy");

    g_Heap3Start = GamespyHeapend;
    RedSetCurrentHeap(ApplicationHeap);
}

// FUNCTION: SWBF2 0x00533bc0
void GameMemory::BuildHeaps(bool bMission)
{
    if (TempLoadHeap)
    {
        RedMarkHeapPermanent(TempLoadHeap, false);
        RedDestroyHeap(TempLoadHeap);
        TempLoadHeap = 0;
    }

    if (RunTimeHeap)
    {
        RedMarkHeapPermanent(RunTimeHeap, false);
        if (TEMP_GamespyHeap)
        {
            RedDestroyHeap(TEMP_GamespyHeap);
            TEMP_GamespyHeap = 0;
            GameMemory::gTempGamespyBlock = 0;
        }
        RedDestroyHeap(RunTimeHeap);
        RunTimeHeap = 0;
    }

    if (LuaHeap)
    {
        RedMarkHeapPermanent(LuaHeap, false);
        RedDestroyHeap(LuaHeap);
        LuaHeap = 0;
    }

    if (ArtistHeap)
    {
        RedMarkHeapPermanent(ArtistHeap, false);
        RedDestroyHeap(ArtistHeap);
        ArtistHeap = 0;
    }

    char* pLuaStart = AlignHeapStart(g_Heap3Start);
    char* pLuaEnd = pLuaStart + kLuaHeapSize;
    LuaHeap = RedCreateHeap(pLuaStart, pLuaEnd, kLuaHeapAlignment, "Lua");

    char* pArtistStart = AlignHeapStart(pLuaEnd);
    gArtistHeapStart = pArtistStart;
    ArtistHeap = RedCreateHeap(pArtistStart, pArtistStart + kArtistHeapSize, kHeapAlignment, "Artist");

    char* pRunTimeStart = AlignHeapStart(pArtistStart + kArtistHeapSize);
    RunTimeHeap = RedCreateHeap(pRunTimeStart, gArenaEnd, kHeapAlignment, "Runtime");
    RedSetCurrentHeap(RunTimeHeap);

    LuaMemory::CreatePools(bMission);

    gTempHeapBlock = RedAllocFromHeap(RunTimeHeap, kTempLoadBlockSize, 1);
    TempLoadHeap = RedCreateHeap(gTempHeapBlock, (char*)gTempHeapBlock + kTempLoadHeapSize,
                                 kHeapAlignment, "Temp");
    RedSetTempHeap(TempLoadHeap);
}

// FUNCTION: SWBF2 0x00533d50
void GameMemory::ReleaseTempHeap()
{
    if (!gTempHeapBlock)
    {
        return;
    }

    if (TempLoadHeap)
    {
        RedDestroyHeap(TempLoadHeap);
        RedSetTempHeap(-1);
        TempLoadHeap = 0;
    }
    RedFreeToHeap(RunTimeHeap, gTempHeapBlock, 0);
    gTempHeapBlock = 0;
}
