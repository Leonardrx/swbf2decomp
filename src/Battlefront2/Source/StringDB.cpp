#include <string.h>

#include "StringDB.h"
#include "RedEngineFL/Memory/engineAlloc.h"

namespace StringDB
{
// SYNTHETIC: SWBF2 0x004064d0
// StringDB::`dynamic initializer for 'mMap''

// GLOBAL: SWBF2 0x01f6df08
PblHashTable<char, 4096> mMap;

// GLOBAL: SWBF2 0x01eafb90
// mPool
char* mPool;
// GLOBAL: SWBF2 0x01eafb94
// mNext
char* mNext;
// GLOBAL: SWBF2 0x01eafb98
int mPoolSize;

// FUNCTION: SWBF2 0x00651ed0
void Init()
{
    mMap.Clear();

    mPoolSize = 0x8000;

    mPool = new char[0x8000];
    memset(mPool, 0, 0x8000);
    mNext = mPool;
}

// FUNCTION: SWBF2 0x00651f20
void Cleanup()
{
    mMap.Clear();

    operator delete(mPool);
    mPool = 0;
    mNext = 0;
}

// FUNCTION: SWBF2 0x00651ff0
bool Remove(unsigned int uiHash)
{
    if (!mMap.Find(uiHash))
    {
        return false;
    }
    mMap.Remove(uiHash);
    return true;
}

// FUNCTION: SWBF2 0x00651f60
const char* Store(unsigned int uiHash, const char* pcText)
{
    if (!uiHash)
    {
        return 0;
    }
    const char* pcFound = mMap.Find(uiHash);
    if (pcFound)
    {
        return pcFound;
    }
    unsigned int uiLen = strlen(pcText);
    char* pcCopy = mNext;
    memcpy(pcCopy, pcText, uiLen + 1);
    pcCopy[uiLen + 1] = 0;
    mMap.Store(uiHash, pcCopy);
    mNext += uiLen + 1;
    return pcCopy;
}

// FUNCTION: SWBF2 0x00652030
const char* Find(unsigned int uiHash)
{
    return mMap.Find(uiHash);
}

}
