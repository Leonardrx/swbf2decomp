#include "sndfreelist.h"

#include "sndglobals.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00738610
bool FreeList::AllocBlocks(unsigned int uiNumBlocks, Block** ppFirstBlock)
{

    CheckAllocator();

    while (uiNumBlocks != 0)
    {
        --uiNumBlocks;

        const unsigned int uiDataSize =
            mEntrySize * mEntriesPerBlock + (mEntriesPerBlock + 7) / 8;

        void* const pAllocAddress =
            mAllocFunc(mAlignment + (sizeof(Block) - 1) + uiDataSize);
        if (!pAllocAddress)
        {
            return false;
        }

        char* const pEntries =
            (char*)(((mAlignment - 1 + (unsigned int)pAllocAddress) / mAlignment)
                    * mAlignment);
        Block* const pBlock = (Block*)(pEntries + mEntriesPerBlock * mEntrySize);

        Block* const pNew = new (pBlock, false) Block(pAllocAddress);

        pNew->mBitmap.Initialize((unsigned char*)pNew + sizeof(Block),
                                 mEntriesPerBlock);

        mBlockList.AddLast(&pNew->mNode);

        if (ppFirstBlock && *ppFirstBlock == 0)
        {
            *ppFirstBlock = pNew;
        }
    }

    return true;
}

// FUNCTION: SWBF2 0x00738560
void* FreeList::Alloc()
{
    for (PblListSingle::Itor itor(mBlockList);
         itor.GetNode() != mBlockList.GetTerminator(); ++itor)
    {
        Block* const pBlock = static_cast<Block*>(itor.GetNode()->GetObjectA(&mBlockList, offsetof(Block, mNode)));
        void* const pEntry = pBlock->AllocEntry(mEntrySize, mEntriesPerBlock);
        if (pEntry)
        {
            return pEntry;
        }
    }

    if (!(mFlags & flags_locked))
    {
        Block* pBlock = 0;
        if (AllocBlocks(1, &pBlock))
        {
            return pBlock->AllocEntry(mEntrySize, mEntriesPerBlock);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00738770
void FreeList::Free(void* p)
{
    PblListSingle::Itor itor(mBlockList);
    if (!mBlockList.IsEmpty())
    {
        for (; itor.GetNode() != mBlockList.GetTerminator(); ++itor)
        {
            Block* const pBlock = static_cast<Block*>(itor.GetNode()->GetObjectA(&mBlockList, offsetof(Block, mNode)));
            if (pBlock->IsValid(p, mEntrySize, mEntriesPerBlock))
            {
                pBlock->FreeEntry(p, mEntrySize, mEntriesPerBlock);
                break;
            }
        }
    }

    if ((mFlags & flags_purgeOnFree) != 0 && (mFlags & flags_locked) == 0)
    {
        PurgeBlocks();
    }
}

// FUNCTION: SWBF2 0x00738800
void FreeList::PurgeBlocks()
{
    PblListSingle::Itor itor(mBlockList);
    CheckAllocator();

    while (itor.GetNode() != mBlockList.GetTerminator())
    {
        Block* const pBlock = static_cast<Block*>(itor.GetNode()->GetObjectA(&mBlockList, offsetof(Block, mNode)));
        ++itor;

        if (pBlock->GetAllocator().GetFreeEntries() == mEntriesPerBlock)
        {
            void* const pAllocAddress = pBlock->GetAllocationAddress();
            delete pBlock;
            mFreeFunc(pAllocAddress);
        }
    }
}

}
