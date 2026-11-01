#include <string.h>
#include <vector>
#include <new>
#include "GameMemory.h"
#include "RedEngineFL/Memory/RedMemory.h"

#include <string.h>
#include "GameLoop.h"
#include "LuaCallbacks_IFace.h"
#include "PebbleFL/Common/PblUnicode.h"

namespace StatsManager
{

// FUNCTION: SWBF2 0x005307a0
void Update()
{
}

}

namespace StatsManager
{
extern int m_state;
}
extern unsigned char sStatsMatchData[0x40];
class NetPktGroup;

namespace StatsManager
{
void WriteStatsChallenge(int iPlayer, NetPktGroup& group);
void WriteStatsResponse(int iLocal, NetPktGroup& group);
void BeginMatch();
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x00530890
void ReadStatsChallenge(int player, NetPktGroup& group)
{
}
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x005308b0
void ReadStatsResponse(int player, NetPktGroup& group)
{
}
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x005307b0
int GetState()
{
    return StatsManager::m_state;
}
}

// FUNCTION: SWBF2 0x005307c0
void StatsManagerSetStoppedState()
{
    StatsManager::m_state = 2;
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x005308c0
bool ReadStatsGetResults(unsigned int iPlayer, LeaderBoardPlayerStatsT* pStats)
{
    memset(pStats, 0, sizeof(*pStats));
    pStats->factionStats[2].wins = 10;
    pStats->factionStats[2].losses = 10;
    return true;
}
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x005308f0
bool FindServicePlayerID()
{
    return false;
}
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x00530900
int GetLeaderBoardId()
{
    return 1;
}
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x00530910
bool ResetPlayer()
{
    return false;
}
}

// FUNCTION: SWBF2 0x00530880
void StatsManager::WriteStatsChallenge(int, NetPktGroup&) {}

// FUNCTION: SWBF2 0x005308a0
void StatsManager::WriteStatsResponse(int, NetPktGroup&) {}

// FUNCTION: SWBF2 0x00530860
void StatsManager::BeginMatch()
{
    strlcpy((char*)sStatsMatchData, GameLoop::mMissionScript, sizeof(sStatsMatchData));
}

namespace std
{
__declspec(noreturn) void __cdecl _Xbad_alloc();
__declspec(noreturn) void __cdecl _Xlength_error(const char*);
}

struct StatsLeaderboardNode
{
    StatsLeaderboardNode* _Next;
    StatsLeaderboardNode* _Prev;
    void* mPayloadFirst;
    void* mPayloadSecond;
};

struct StatsLeaderboardBucketVector
{
    void** mFirst;
    void** mLast;
    void** mEnd;
};

struct StatsLeaderboardContainer
{

    StatsLeaderboardContainer();
    ~StatsLeaderboardContainer();

    StatsLeaderboardNode* mListHead;
    unsigned int mListSize;
    StatsLeaderboardBucketVector mBuckets;
    unsigned int mBucketMask;
    unsigned int mBucketCount;
    float mMaxLoadFactor;
};
 

// SYNTHETIC: SWBF2 0x00403c20
// `dynamic initializer for 'sStatsLeaderboard''
// SYNTHETIC: SWBF2 0x00768910
// `dynamic atexit destructor for 'sStatsLeaderboard''

// GLOBAL: SWBF2 0x01ec0c20
// sStatsLeaderboard
StatsLeaderboardContainer sStatsLeaderboard;

void ClearStatsLeaderboardList();
StatsLeaderboardContainer::~StatsLeaderboardContainer()
{
    if (mBuckets.mFirst)
    {
        ::operator delete(mBuckets.mFirst);
        mBuckets.mFirst = 0;
        mBuckets.mLast = 0;
        mBuckets.mEnd = 0;
    }
    ClearStatsLeaderboardList();
    ::operator delete(mListHead);
}
namespace StatsManager
{

// GLOBAL: SWBF2 0x007e6668
int m_state = 1;
}

// GLOBAL: SWBF2 0x01e56008
bool sStatsRequestBusy;

// GLOBAL: SWBF2 0x01e55fc0
unsigned char sStatsMatchData[0x40];

// FUNCTION: SWBF2 0x00530cc0
StatsLeaderboardNode* AllocateStatsLeaderboardHead()
{
    StatsLeaderboardNode* pNode = (StatsLeaderboardNode*)
        ::operator new(sizeof(StatsLeaderboardNode));
    if (!pNode)
    {
        std::_Xbad_alloc();
    }

    pNode->_Next = pNode;
    StatsLeaderboardNode** ppPrev = &pNode->_Prev;
    if (ppPrev)
    {
        *ppPrev = pNode;
    }

    return pNode;
}

// FUNCTION: SWBF2 0x00530c80
unsigned int GrowStatsBucketCapacity(const StatsLeaderboardBucketVector* pThis, unsigned int uiCount)
{
    unsigned int uiCapacity =
        (unsigned int)(pThis->mEnd - pThis->mFirst);

    if (0x3FFFFFFFu - uiCapacity / 2 < uiCapacity)
    {
        uiCapacity = 0;
    }
    else
    {
        uiCapacity += uiCapacity / 2;
    }

    if (uiCapacity < uiCount)
    {
        uiCapacity = uiCount;
    }

    return uiCapacity;
}

// FUNCTION: SWBF2 0x00530d00
void FillStatsBucketRange(void** ppFirst, void** ppLast, void* const* ppVal)
{
    for (; ppFirst < ppLast; ++ppFirst)
    {
        *ppFirst = *ppVal;
    }
}

// FUNCTION: SWBF2 0x00530d40
void FillStatsBucketCount(void** ppDest, unsigned int uiCount, void* const* ppVal)
{
    for (; uiCount != 0; uiCount--)
    {
        if (ppDest)
        {
            *ppDest = *ppVal;
        }
        ppDest++;
    }
}

// FUNCTION: SWBF2 0x00530d70
void** CopyStatsBucketRange(void** ppFirst, void** ppLast, void** ppDest)
{
    for (; ppFirst != ppLast; ++ppFirst, ++ppDest)
    {
        if (ppDest)
        {
            *ppDest = *ppFirst;
        }
    }

    return ppDest;
}

// FUNCTION: SWBF2 0x00530ab0
void InsertStatsLeaderboardBuckets(StatsLeaderboardBucketVector* pThis, void*** pppResult,
                       void** ppWhere, void**  , void* const* ppVal)
{
    const unsigned int uiOff = (unsigned int)(ppWhere - pThis->mFirst);

    if ((unsigned int)(pThis->mEnd - pThis->mLast) < 0x10u)
    {

        const unsigned int uiSize =
            (unsigned int)(pThis->mLast - pThis->mFirst);
        if (0x3FFFFFFFu - uiSize < 0x10u)
        {
            std::_Xlength_error("vector<T> too long");
        }

        const unsigned int uiCapacity = GrowStatsBucketCapacity(pThis, uiSize + 0x10);

        void** ppNew = 0;
        if (uiCapacity != 0)
        {
            if (uiCapacity > 0x3FFFFFFFu)
            {
                std::_Xbad_alloc();
            }
            ppNew = (void**)::operator new(uiCapacity * 4);
            if (!ppNew)
            {
                std::_Xbad_alloc();
            }
        }

        FillStatsBucketCount(ppNew + uiOff, 0x10, ppVal);
        CopyStatsBucketRange(pThis->mFirst, ppWhere, ppNew);
        CopyStatsBucketRange(ppWhere, pThis->mLast, ppNew + uiOff + 0x10);

        const unsigned int uiNewSize = uiSize + 0x10;
        if (pThis->mFirst)
        {
            ::operator delete(pThis->mFirst);
        }

        pThis->mFirst = ppNew;
        pThis->mEnd = ppNew + uiCapacity;
        pThis->mLast = ppNew + uiNewSize;
    }
    else
    {
        void** ppOldEnd = pThis->mLast;

        void* pVal = *ppVal;

        if ((unsigned int)(ppOldEnd - ppWhere) < 0x10u)
        {

            CopyStatsBucketRange(ppWhere, ppOldEnd, ppWhere + 0x10);
            FillStatsBucketCount(ppOldEnd, 0x10 - (unsigned int)(ppOldEnd - ppWhere), &pVal);
            pThis->mLast += 0x10;
            FillStatsBucketRange(ppWhere, pThis->mLast - 0x10, &pVal);
        }
        else
        {

            pThis->mLast =
                CopyStatsBucketRange(ppOldEnd - 0x10, ppOldEnd, ppOldEnd);

            void** ppSrc = ppOldEnd - 0x10;
            while (ppWhere != ppSrc)
            {
                ppSrc--;
                ppSrc[0x10] = ppSrc[0];
            }

            FillStatsBucketRange(ppWhere, ppWhere + 0x10, &pVal);
        }
    }

    *pppResult = pThis->mFirst + uiOff;
}

// FUNCTION: SWBF2 0x00530a00
void ClearStatsLeaderboardList()
{
    StatsLeaderboardNode* pHead = sStatsLeaderboard.mListHead;
    StatsLeaderboardNode* pNode = pHead->_Next;

    pHead->_Next = pHead;
    sStatsLeaderboard.mListHead->_Prev = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mListSize = 0;

    while (pNode != sStatsLeaderboard.mListHead)
    {
        StatsLeaderboardNode* pNext = pNode->_Next;
        ::operator delete(pNode);
        pNode = pNext;
    }
}

// FUNCTION: SWBF2 0x00530920
StatsLeaderboardContainer::StatsLeaderboardContainer()
{
    mListHead = 0;
    mListSize = 0;
    mListHead = AllocateStatsLeaderboardHead();

    mBuckets.mFirst = 0;
    mBuckets.mEnd = 0;
    mBuckets.mLast = 0;
    mMaxLoadFactor = 1.0f;

    void* pHead = mListHead;
    void** ppResult;
    InsertStatsLeaderboardBuckets(&mBuckets, &ppResult, 0, 0, &pHead);

    mBucketMask = 7;
    mBucketCount = 8;
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x00530640
void Init()
{
    StatsManager::m_state = 1;
    sStatsRequestBusy = false;

    ClearStatsLeaderboardList();

    void* pHead = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mBuckets.mLast = sStatsLeaderboard.mBuckets.mFirst;

    void** ppResult;
    InsertStatsLeaderboardBuckets(&sStatsLeaderboard.mBuckets, &ppResult,
               sStatsLeaderboard.mBuckets.mFirst, &pHead, &pHead);

    sStatsLeaderboard.mBucketMask = 7;
    sStatsLeaderboard.mBucketCount = 8;
    memset(sStatsMatchData, 0, 0x40);
}
}

void StatsManagerSetStoppedState();

namespace StatsManager
{

// FUNCTION: SWBF2 0x005307d0
void Stop()
{
    if (StatsManager::m_state == 7 || StatsManager::m_state == 3)
    {
        StatsManagerSetStoppedState();
    }
    else if (StatsManager::m_state == 0)
    {
        StatsManager::m_state = 1;
    }
    sStatsRequestBusy = false;

    ClearStatsLeaderboardList();

    void* pHead = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mBuckets.mLast = sStatsLeaderboard.mBuckets.mFirst;

    void** ppResult;
    InsertStatsLeaderboardBuckets(&sStatsLeaderboard.mBuckets, &ppResult,
               sStatsLeaderboard.mBuckets.mFirst, &pHead, &pHead);

    sStatsLeaderboard.mBucketMask = 7;
    sStatsLeaderboard.mBucketCount = 8;
    if (StatsManager::m_state == 1)
    {
        StatsManager::Init();
    }
}
}

// GLOBAL: SWBF2 0x01e55fb7
bool sStatsHeapAllocated;

// GLOBAL: SWBF2 0x01e55fbc
void* sStatsHeapBlockA;

// GLOBAL: SWBF2 0x01e56000
void* sStatsHeapBlockB;

// GLOBAL: SWBF2 0x01e56004
void* sStatsHeapBlockC;

// GLOBAL: SWBF2 0x01e5600c
void* sStatsHeapBlockD;

// GLOBAL: SWBF2 0x01e56010
void* sStatsHeapBlockE;

// FUNCTION: SWBF2 0x00530620
void __fastcall FreeGamespyBlock(void* pBlock)
{
    RedFreeToHeap(GameMemory::GamespyHeap, pBlock, 0);
}

namespace StatsManager
{

// FUNCTION: SWBF2 0x005306b0
void Cleanup()
{
    StatsManager::m_state = 1;
    sStatsRequestBusy = false;

    ClearStatsLeaderboardList();

    void* pHead = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mBuckets.mLast = sStatsLeaderboard.mBuckets.mFirst;

    void** ppResult;
    InsertStatsLeaderboardBuckets(&sStatsLeaderboard.mBuckets, &ppResult,
               sStatsLeaderboard.mBuckets.mFirst, &pHead, &pHead);

    sStatsLeaderboard.mBucketMask = 7;
    sStatsLeaderboard.mBucketCount = 8;

    if (sStatsHeapAllocated)
    {
        sStatsHeapAllocated = false;
        if (sStatsHeapBlockB)
        {
            FreeGamespyBlock(sStatsHeapBlockB);
            sStatsHeapBlockB = 0;
        }
        if (sStatsHeapBlockE)
        {
            FreeGamespyBlock(sStatsHeapBlockE);
            sStatsHeapBlockE = 0;
        }
        if (sStatsHeapBlockD)
        {
            FreeGamespyBlock(sStatsHeapBlockD);
            sStatsHeapBlockD = 0;
        }
        if (sStatsHeapBlockC)
        {
            FreeGamespyBlock(sStatsHeapBlockC);
            sStatsHeapBlockC = 0;
        }
        if (sStatsHeapBlockA)
        {
            FreeGamespyBlock(sStatsHeapBlockA);
            sStatsHeapBlockA = 0;
        }
    }
}
}
// FUNCTION: SWBF2 0x00530a90
void __stdcall IgnoreStatsBucketArguments(int, int) {}

// FUNCTION: SWBF2 0x00530aa0
void* __fastcall ReturnStatsBucketPointer(void* pThis) { return pThis; }

// FUNCTION: SWBF2 0x00530c70
void* __fastcall ReturnStatsVectorPointer(void* pThis) { return pThis; }

// FUNCTION: SWBF2 0x00530cf0
void __stdcall IgnoreStatsVectorArgument(int) {}

// FUNCTION: SWBF2 0x00530a70
void __stdcall IgnoreStatsListArguments(int, int)
{
}

// FUNCTION: SWBF2 0x00530a80
void* __fastcall ReturnStatsListPointer(void* pThis)
{
    return pThis;
}

// SYNTHETIC: SWBF2 0x00403c50
// `dynamic initializer for 'sStatsScratchBytes''
// SYNTHETIC: SWBF2 0x007689a0
// `dynamic atexit destructor for 'sStatsScratchBytes''

// GLOBAL: SWBF2 0x01ec0c14
// sStatsScratchBytes
std::vector<unsigned char> sStatsScratchBytes;
