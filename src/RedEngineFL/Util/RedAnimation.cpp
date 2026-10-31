#include "RedAnimation.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "Zephyr/Source/ZephyrEvent.h"
#include "PebbleFL/Common/PblChunk.h"
#include <new>
#include <string.h>
#include "Zephyr/Source/ZephyrSkinShared.h"

// SYNTHETIC: SWBF2 0x004086d0
// `dynamic initializer for 'RedAnimation::_HashTable''

// GLOBAL: SWBF2 0x0099e808
PblHashTable<RedAnimation, 1024> RedAnimation::_HashTable;

// FUNCTION: SWBF2 0x006dd620
RedAnimation::RedAnimation()
    : _uiNameHash(0)
    , _pSkeleton(0)
    , _pSkeletonShared(0)
    , _pZephyrAnimBank(0)
    , _pEventBank(0)
{
}

// FUNCTION: SWBF2 0x006dd6a0
RedAnimation::~RedAnimation()
{
}

// FUNCTION: SWBF2 0x006dd660
ZephyrEventBank::ZephyrEventBank()
{
    _Lists = 0;
    _iNumLists = 0;
}

// FUNCTION: SWBF2 0x006dd940
void RedAnimation::_ReadEvents(PblMemChunk& chunk)
{
    if (_pEventBank)
    {
        return;
    }

    int iNumAnims = chunk.Read32();
    if (!iNumAnims)
    {
        return;
    }

    _pEventBank = new ZephyrEventBank;
    _pEventBank->Init(static_cast<short>(iNumAnims));
    char name[100];
    for (int i = 0; i < iNumAnims; ++i)
    {
        chunk.ReadString(name, sizeof(name));
        unsigned int uiAnimHash = PblHash(name);
        short iNumEvents = static_cast<short>(chunk.Read32());
        const ZephyrAnim* pAnim = _pZephyrAnimBank->Find(uiAnimHash);
        int iNumFrames = pAnim->GetNumFrames();
        ZephyrEventList* pEvents = _pEventBank->New(uiAnimHash);
        pEvents->Init(iNumEvents);
        for (int j = 0; j < iNumEvents; ++j)
        {
            int iFrame = chunk.Read32();
            chunk.ReadString(name, sizeof(name));
            pEvents->AddEvent(static_cast<float>(iFrame) /
                                  static_cast<float>(iNumFrames), PblHash(name));
        }
    }
}

// FUNCTION: SWBF2 0x006ddab0
void RedAnimation::_ReadZaa(PblMemChunk& kChunk)
{
    if (!_pZephyrAnimBank)
    {
        _kZephyrAnimBankReader.ReadFile(kChunk);
        _pZephyrAnimBank = _kZephyrAnimBankReader.GetBank();
    }
}

// FUNCTION: SWBF2 0x006ddae0
void RedAnimation::_ReadZaf(PblMemChunk& kChunk)
{
    if (!_pSkeletonShared)
    {
        _kZephyrSkinReader.ReadFile(kChunk);
        _pSkeletonShared = _kZephyrSkinReader.GetSkin()->GetSkeleton();
    }
}

// FUNCTION: SWBF2 0x006dd6e0
void RedAnimation::ReadZaa(PblMemChunk& kChunk)
{
    RedAnimation* pAnim = 0;
    while (!kChunk.IsDone())
    {
        PblMemChunk kChild = kChunk.ReadNextChild();
        switch (kChild.GetId())
        {
        case 0x454D414E:
        {
            char szName[48];
            kChild.ReadString(szName, 48);
            unsigned int uiHash = PblHash(szName);
            pAnim = RedAnimation::_HashTable.Find(uiHash);
            if (!pAnim)
            {
                pAnim = new RedAnimation;
                pAnim->_uiNameHash = uiHash;
                RedAnimation::_HashTable.Store(uiHash, pAnim);
            }
            break;
        }
        case 0x544E5645:
            pAnim->_ReadEvents(kChild);
            break;
        case 0x5F4E4942:
            pAnim->_ReadZaa(kChild);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006dd820
void RedAnimation::ReadZaf(PblMemChunk& kChunk)
{
    RedAnimation* pAnim = 0;
    while (!kChunk.IsDone())
    {
        PblMemChunk kChild = kChunk.ReadNextChild();
        switch (kChild.GetId())
        {
        case 0x454D414E:
        {
            char szName[48];
            kChild.ReadString(szName, 48);
            unsigned int uiHash = PblHash(szName);
            pAnim = RedAnimation::_HashTable.Find(uiHash);
            if (!pAnim)
            {
                pAnim = new RedAnimation;
                pAnim->_uiNameHash = uiHash;
                RedAnimation::_HashTable.Store(uiHash, pAnim);
            }
            break;
        }
        case 0x5F4E4942:
            pAnim->_ReadZaf(kChild);
            break;
        }
    }
}
