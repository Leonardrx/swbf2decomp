#pragma once

namespace GameMemory
{

// Layout of the game's memory arena: the application and Gamespy heaps at its start (InitApplicationMemory),
// then the Lua, artist and run-time heaps (BuildHeaps), with a temporary load heap inside the run-time heap.
const unsigned int kApplicationHeapSize = 0xF40000;
const unsigned int kGamespyHeapSize = 0x200000;
const unsigned int kLuaHeapSize = 0x1000000;
const unsigned int kArtistHeapSize = 0x2000;
const unsigned int kTempLoadBlockSize = 0x200000;
const unsigned int kTempLoadHeapSize = 0x1FFF00;
// Alignment passed to RedCreateHeap, and the boundary each heap built by BuildHeaps starts on.
const unsigned int kHeapAlignment = 0x10;
const unsigned int kLuaHeapAlignment = 4;
const unsigned int kHeapStartAlignment = 0x40;

void InitApplicationMemory(char* pStart, char* pEnd);

void BuildHeaps(bool bMission);

void ReleaseTempHeap();

extern int ApplicationHeap;
extern int GamespyHeap;
extern int RunTimeHeap;
extern int TempLoadHeap;
extern int LuaHeap;
extern int ArtistHeap;
extern int TEMP_GamespyHeap;
}
