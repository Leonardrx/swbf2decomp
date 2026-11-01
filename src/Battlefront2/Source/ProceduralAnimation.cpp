#include <new>
#include <d3dx9math.h>
#include <string.h>
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblHermite.h"

#include "ProceduralAnimation.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "NetGame.h"
#include "NetPacket.h"
#include "TreeGrid.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// FUNCTION: SWBF2 0x00628c00
bool* PAAnimationGroup::EnterClient()
{
    return mActive = &mClientActive;
}

// FUNCTION: SWBF2 0x00628c10
bool* PAAnimationGroup::LeaveClient()
{
    return mActive = &mHostActive;
}

// FUNCTION: SWBF2 0x00628530
void PAAnimationGroup::ClientGeometryCreated(CollisionObject* pHost, CollisionObject* pClient)
{
    if (!pHost || !pClient)
    {
        return;
    }
    for (int i = 0; i < 255; ++i)
    {
        if (sPAAnims[i].mHostGeometry == pHost)
        {
            sPAAnims[i].mClientGeometry = pClient;
        }
    }
    for (int h = 0; h < mNumHierarchies; ++h)
    {
        if (mHierarchies[h]->mRoot.mHostGeometry == pHost)
        {
            mHierarchies[h]->mRoot.mClientGeometry = pClient;
        }
        for (int n = 0; n < mHierarchies[h]->mNumObjs; ++n)
        {
            if (mHierarchies[h]->mObjs[n].mHostGeometry == pHost)
            {
                mHierarchies[h]->mObjs[n].mClientGeometry = pClient;
            }
        }
    }
}


extern PblVector3 gVectorZero;
extern bool netIsLocalTurn;
extern bool netFrameLock;

// FUNCTION: SWBF2 0x00628eb0
void PAManagerInterpolator::Destroy()
{
    if (!netFrameLock)
    {
        DeactivateThread();
    }
}

// FUNCTION: SWBF2 0x00628ed0
bool PAManagerInterpolator::Update(float)
{
    mIsFrameUpdate = true;
    if (mNextTotalTurnTime > 0.0f)
    {
        float fRemaining = mTotalTurnTime - mTotalFrameTime - mNextTotalTurnTime;
        mTotalTurnTime = mNextTotalTurnTime;
        mNextTotalTurnTime = 0.0f;
        mTotalFrameTime = 0.0f;
        PAManager::sInstance->Update(fRemaining);
    }
    float fTurnRatio = NetGame::GetTurnRatio();
    PAManager::sInstance->Update(mTotalTurnTime * fTurnRatio - mTotalFrameTime);
    mIsFrameUpdate = false;
    mTotalFrameTime = mTotalTurnTime * fTurnRatio;
    return true;
}

// FUNCTION: SWBF2 0x00629160
void PAManager::CreateClient()
{
    pamanagerInterpolator.Create();
    if (NetGame::IsOneWorld())
    {
        sInstance->DeactivateThread();
        sInstance->ActivateThread(-99, PER_TURN);
    }
    else
    {
        sClientInstance = new PAManager;
    }
}

// FUNCTION: SWBF2 0x00629200
void PAManager::DestroyClient()
{
    if (NetGame::IsOneWorld())
    {
        pamanagerInterpolator.Destroy();
    }
    delete sClientInstance;
    sClientInstance = NULL;
}

// FUNCTION: SWBF2 0x006292e0
void PAManager::EnterClient()
{
    if (NetGame::IsOneWorld())
    {
        return;
    }
    sInstance = sClientInstance;
    for (PblHashTable<PAAnimationGroup, 32>::Itor it = PAAnimationGroup::_HashTable.Begin();
         it != PAAnimationGroup::_HashTable.End(); ++it)
    {
        (*it).EnterClient();
    }
    for (int i = 0; i < 255; ++i)
    {
        sPAAnims[i].mTimer = &sPAAnims[i].mClientTimer;
        sPAAnims[i].mGeometry = &sPAAnims[i].mClientGeometry;
    }
    for (int i = 0; i < PAAnimationGroup::mNumHierarchies; ++i)
    {
        PAHierarchy& hierarchy = *PAAnimationGroup::mHierarchies[i];
        hierarchy.mRoot.mGeometry = &hierarchy.mRoot.mClientGeometry;
        for (int j = 0; j < hierarchy.mNumObjs; ++j)
        {
            hierarchy.mObjs[j].mGeometry = &hierarchy.mObjs[j].mClientGeometry;
        }
    }
}

// FUNCTION: SWBF2 0x006293d0
void PAManager::LeaveClient()
{
    if (NetGame::IsOneWorld())
    {
        return;
    }
    sInstance = sHostInstance;
    for (PblHashTable<PAAnimationGroup, 32>::Itor it = PAAnimationGroup::_HashTable.Begin();
         it != PAAnimationGroup::_HashTable.End(); ++it)
    {
        (*it).LeaveClient();
    }
    for (int i = 0; i < 255; ++i)
    {
        sPAAnims[i].mTimer = &sPAAnims[i].mHostTimer;
        sPAAnims[i].mGeometry = &sPAAnims[i].mHostGeometry;
    }
    for (int i = 0; i < PAAnimationGroup::mNumHierarchies; ++i)
    {
        PAHierarchy& hierarchy = *PAAnimationGroup::mHierarchies[i];
        hierarchy.mRoot.mGeometry = &hierarchy.mRoot.mHostGeometry;
        for (int j = 0; j < hierarchy.mNumObjs; ++j)
        {
            hierarchy.mObjs[j].mGeometry = &hierarchy.mObjs[j].mHostGeometry;
        }
    }
}
void ReadMatrix(NetPktGroup& group, PblMatrix& matrix);
void WriteMatrix(NetPktGroup& group, const PblMatrix& matrix);

struct PAGameObjectTeam
{
    char pad_0000[0x234];
    int mTeam : 4;
};



// FUNCTION: SWBF2 0x006284e0
PAHierarchy::PAHierarchy()
{
    mRoot.mObjHash = 0;
    mRoot.mHostGeometry = 0;
    mRoot.mGeometry = &mRoot.mHostGeometry;
    mRoot.mClientGeometry = 0;
    memcpy(&mRoot.mLocalMatrix, &gMatrixIdentity, sizeof(mRoot.mLocalMatrix));
}

// FUNCTION: SWBF2 0x00628340
void __cdecl PAAnimationGroup::ReadHierarchy(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x4f464e49)
        {
            continue;
        }
        const int iNodeCount = (signed char)child._pFile->Read8();
        if (iNodeCount < 2 || PAAnimationGroup::mNumHierarchies == 32)
        {
            continue;
        }

        PAHierarchy* pHierarchy = new PAHierarchy;
        const int iNumObjs = iNodeCount - 1;
        pHierarchy->mNumObjs = iNumObjs;
        PAAnimationGroup::mHierarchies[PAAnimationGroup::mNumHierarchies++] = pHierarchy;
        PAHierarchyNode* pAlloc = new PAHierarchyNode[iNumObjs];
        PAHierarchyNode* pNodes;
        if (pAlloc)
        {
            PAHierarchyNode* pNode = pAlloc;
            for (int n = iNumObjs - 1; n >= 0; --n)
            {
                pNode->mObjHash = 0;
                pNode->mHostGeometry = 0;
                pNode->mClientGeometry = 0;
                pNode->mGeometry = &pNode->mHostGeometry;
                memcpy(&pNode->mLocalMatrix, &gMatrixIdentity, sizeof(pNode->mLocalMatrix));
                ++pNode;
            }
            pNodes = pAlloc;
        }
        else
        {
            pNodes = 0;
        }
        pHierarchy->mObjs = pNodes;
        char acName[128];
        pHierarchy->mRoot.mObjHash = PblHash(child.ReadString(acName, sizeof(acName)));
        for (int i = 0; i < pHierarchy->mNumObjs; ++i)
        {
            pHierarchy->mObjs[i].mObjHash = PblHash(child.ReadString(acName, sizeof(acName)));
        }
    }
}

// GLOBAL: SWBF2 0x01eaee2b
unsigned char sNextAvailableKey;
// GLOBAL: SWBF2 0x01eaefc4
unsigned char sNextAvailableAnim;
// SYNTHETIC: SWBF2 0x00405fc0
// `dynamic initializer for 'sPAKeys''

// GLOBAL: SWBF2 0x01efa230
// sPAKeys
PABaseKey sPAKeys[255];

// SYNTHETIC: SWBF2 0x00626be0
// PABaseKey::PABaseKey

// SYNTHETIC: SWBF2 0x00406000
// `dynamic initializer for 'sPAAnims''

// GLOBAL: SWBF2 0x01efd060
// sPAAnims
PAAnimationData sPAAnims[255];
// SYNTHETIC: SWBF2 0x00405fe0
// `dynamic initializer for 'PAAnimation::_HashTable''

// GLOBAL: SWBF2 0x01efce10
PblHashTable<PAAnimation, 32> PAAnimation::_HashTable;
// SYNTHETIC: SWBF2 0x00406020
// `dynamic initializer for 'PAAnimationGroup::_HashTable''

// GLOBAL: SWBF2 0x01efcf40
PblHashTable<PAAnimationGroup, 32> PAAnimationGroup::_HashTable;

// GLOBAL: SWBF2 0x01eaef3c
int PAAnimationGroup::mNumHierarchies;
// GLOBAL: SWBF2 0x01eaef40
PAHierarchy* PAAnimationGroup::mHierarchies[32];

// GLOBAL: SWBF2 0x01eaee34
PAManager* PAManager::sHostInstance;
// GLOBAL: SWBF2 0x01eaefc0
PAManager* PAManager::sClientInstance;
// GLOBAL: SWBF2 0x01eaef38
PAManager* PAManager::sInstance;

// SYNTHETIC: SWBF2 0x00406040
// `dynamic initializer for 'pamanagerInterpolator''
// SYNTHETIC: SWBF2 0x00769350
// `dynamic atexit destructor for 'pamanagerInterpolator''

// GLOBAL: SWBF2 0x01efcf14
// pamanagerInterpolator
PAManagerInterpolator pamanagerInterpolator;

// SYNTHETIC: SWBF2 0x00626c00
// PAManagerInterpolator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00626c30
PAAnimation::PAAnimation()
{
    mNumRotKeys = 0;
    mStartRotKey = 0;
    mLoop = false;
    mLocalTranslation = false;
    mTotalTime = 0.0f;
    mNumPosKeys = 0;
    mStartPosKey = 0;
}

// SYNTHETIC: SWBF2 0x00626c50
// PAAnimation::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00626c80
PAAnimation::~PAAnimation()
{
}

// FUNCTION: SWBF2 0x00626c90
void PAAnimation::Read(PblFileChunk& chunk)
{
    PAAnimation* pAnim = NULL;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        unsigned int uiId = child._iId;
        if (uiId == 0x4B534F50 || uiId == 0x4B544F52)
        {
            if (!pAnim)
            {
                return;
            }
            PABaseKey key;
            key.mTime = child._pFile->ReadFloat();
            key.mVal.x = child._pFile->ReadFloat();
            key.mVal.y = child._pFile->ReadFloat();
            key.mVal.z = child._pFile->ReadFloat();
            switch (child._pFile->Read8())
            {
            case 1:
                key.mTransition = PA_LINEAR;
                break;
            case 2:
                key.mTransition = PA_SPLINE;
                break;
            default:
                key.mTransition = PA_POP;
                break;
            }
            key.mSlope0.x = child._pFile->ReadFloat();
            key.mSlope0.y = child._pFile->ReadFloat();
            key.mSlope0.z = child._pFile->ReadFloat();
            key.mSlope1.x = child._pFile->ReadFloat();
            key.mSlope1.y = child._pFile->ReadFloat();
            key.mSlope1.z = child._pFile->ReadFloat();
            if (uiId == 0x4B544F52)
            {
                pAnim->AddRotationKey(key);
            }
            else
            {
                pAnim->AddPositionKey(key);
            }
        }
        else if (uiId == 0x4F464E49)
        {
            char acName[128];
            char* pcName = child.ReadString(acName, 128);
            float fTotalTime = child._pFile->ReadFloat();
            bool bLoop = child._pFile->Read8() != 0;
            bool bLocalTranslation = child._pFile->Read8() != 0;
            pAnim = new PAAnimation;
            if (!pAnim)
            {
                return;
            }
            pAnim->SetAnimationInfo(bLoop, bLocalTranslation, fTotalTime);
            unsigned int uiHash = PblHash(pcName);
            if (!_HashTable.Find(uiHash))
            {
                _HashTable.Store(uiHash, pAnim);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00626e80
void PAAnimation::SetAnimationInfo(bool bLoop, bool bLocalTranslation, float fTotalTime)
{
    mLoop = bLoop;
    mLocalTranslation = bLocalTranslation;
    mTotalTime = fTotalTime;
}

// FUNCTION: SWBF2 0x00626ea0
bool PAAnimation::AddPositionKey(const PABaseKey& key)
{
    if (sNextAvailableKey >= 255)
    {
        return false;
    }
    if (!mNumPosKeys)
    {
        mStartPosKey = sNextAvailableKey;
    }
    ++mNumPosKeys;
    sPAKeys[sNextAvailableKey++] = key;
    return true;
}

// FUNCTION: SWBF2 0x00626f10
bool PAAnimation::AddRotationKey(const PABaseKey& key)
{
    if (sNextAvailableKey >= 255)
    {
        return false;
    }
    if (!mNumRotKeys)
    {
        mStartRotKey = sNextAvailableKey;
    }
    ++mNumRotKeys;
    sPAKeys[sNextAvailableKey++] = key;
    return true;
}

// FUNCTION: SWBF2 0x00626f80
bool PAAnimation::GetKeys(float fTime, PABaseKey& key0, PABaseKey& key1, bool bRotation, unsigned char,
                          unsigned char* pKeyIndex) const
{
    unsigned char ucNumKeys;
    unsigned char ucStartKey;
    if (bRotation)
    {
        ucNumKeys = mNumRotKeys;
        ucStartKey = mStartRotKey;
    }
    else
    {
        ucNumKeys = mNumPosKeys;
        ucStartKey = mStartPosKey;
    }
    if (!ucNumKeys)
    {
        return false;
    }

    if (mLoop)
    {
        while (fTime > mTotalTime)
        {
            fTime -= mTotalTime;
        }
    }
    else if (fTime > mTotalTime)
    {
        if (pKeyIndex)
        {
            *pKeyIndex = ucNumKeys - 1;
        }
        key1 = sPAKeys[ucStartKey + ucNumKeys - 1];
        key0 = key1;
        return true;
    }

    for (unsigned char i = 0; i < ucNumKeys; ++i)
    {
        if (fTime >= sPAKeys[ucStartKey + i].mTime)
        {
            if (i == ucNumKeys - 1)
            {
                if (pKeyIndex)
                {
                    *pKeyIndex = i;
                }
                key0 = sPAKeys[ucStartKey + i];
                if (mLoop)
                {
                    key1 = sPAKeys[ucStartKey];
                }
                else
                {
                    key1 = key0;
                }
                return true;
            }
            if (sPAKeys[ucStartKey + i + 1].mTime > fTime)
            {
                if (pKeyIndex)
                {
                    *pKeyIndex = i;
                }
                key0 = sPAKeys[ucStartKey + i];
                key1 = sPAKeys[ucStartKey + i + 1];
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006271c0
void PAAnimation::InterpolateKeys(PABaseKey key0, PABaseKey key1, float fTime, PABaseKey& result) const
{
    float fElapsed = fTime - key0.mTime;
    float fSpan = 1.0f;
    if (key1.mTime > key0.mTime)
    {
        fSpan = key1.mTime - key0.mTime;
    }
    else if (mTotalTime > key0.mTime)
    {
        fSpan = mTotalTime - key0.mTime;
    }

    switch (key0.mTransition)
    {
    case PA_POP:
        result = key0;
        break;
    case PA_LINEAR:
    {
        float fT = fElapsed / fSpan;
        result.mVal = key0.mVal * (1.0f - fT) + key1.mVal * fT;
        break;
    }
    case PA_SPLINE:
    {
        PblHermiteVector3 spline(key0.mVal, key1.mVal, key0.mSlope0, key0.mSlope1, fSpan);
        result.mVal = spline.getValue(fElapsed);
        break;
    }
    }
}

// FUNCTION: SWBF2 0x00627370
void PAAnimation::CalculateNextMatrix(float fDeltaTime, float& fTime, const PblMatrix& start,
                                      PblMatrix& result) const
{
    if (!mLoop && fDeltaTime < 0.0f && fTime >= mTotalTime && pamanagerInterpolator.mIsFrameUpdate)
    {
        return;
    }

    if (!mLocalTranslation)
    {
        fTime += fDeltaTime;
        if (mLoop)
        {
            while (fTime > mTotalTime)
            {
                fTime -= mTotalTime;
            }
        }
        else if (fTime > mTotalTime)
        {
            fTime = mTotalTime;
        }

        result = start;
        PABaseKey key1;
        PABaseKey key0;
        PABaseKey key;
        if (GetKeys(fTime, key0, key1, true, 0, NULL))
        {
            InterpolateKeys(key0, key1, fTime, key);
            PblMatrix rot = gMatrixIdentity;
            PblMatrix temp;
            rot.RotateX(key.mVal.x);
            rot.RotateY(key.mVal.y);
            rot.RotateZ(key.mVal.z);
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&temp), reinterpret_cast<const D3DXMATRIX*>(&rot), reinterpret_cast<const D3DXMATRIX*>(&result));
            result = temp;
        }
        if (GetKeys(fTime, key0, key1, false, 0, NULL))
        {
            InterpolateKeys(key0, key1, fTime, key);
            PblVector3 vOrigin = start.GetTranslation();
            vOrigin += start.GetXAxis() * key.mVal.x;
            vOrigin += start.GetYAxis() * key.mVal.y;
            vOrigin += start.GetZAxis() * key.mVal.z;
            result.SetTranslation(vOrigin);
        }
        return;
    }

    if (mLoop)
    {
        while (fTime > mTotalTime)
        {
            fTime -= mTotalTime;
        }
    }
    else if (fTime > mTotalTime)
    {
        fTime = mTotalTime;
    }

    unsigned char ucKey;
    PblVector3 vLastPos;
    PABaseKey key1;
    PABaseKey key0;
    PABaseKey key;
    if (GetKeys(fTime, key0, key1, false, 0, &ucKey))
    {
        InterpolateKeys(key0, key1, fTime, key);
        vLastPos = key.mVal;
    }

    float fCurTime = fTime;
    float fEndTime = fTime + fDeltaTime;
    do
    {
        if (mTotalTime >= fCurTime && fCurTime + 0.0166667f > mTotalTime)
        {
            result.SetTranslation(start.GetTranslation());
            vLastPos.x = 0.0f;
            vLastPos.y = 0.0f;
            vLastPos.z = 0.0f;
        }

        float fNextTime = fCurTime + 0.0166667f;
        float fKeyTime = fNextTime;
        if (fNextTime > fEndTime)
        {
            fKeyTime = fEndTime;
        }
        if (mLoop)
        {
            while (fKeyTime > mTotalTime)
            {
                fKeyTime -= mTotalTime;
            }
        }

        PABaseKey rotKey1;
        PABaseKey rotKey0;
        if (GetKeys(fKeyTime, rotKey0, rotKey1, true, 0, &ucKey))
        {
            PblMatrix rot = gMatrixIdentity;
            PABaseKey rotKey;
            InterpolateKeys(rotKey0, rotKey1, fKeyTime, rotKey);
            PblMatrix temp;
            rot.RotateX(rotKey.mVal.x);
            rot.RotateY(rotKey.mVal.y);
            rot.RotateZ(rotKey.mVal.z);
            PblVector3 vPos = result.GetTranslation();
            D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&temp), reinterpret_cast<const D3DXMATRIX*>(&rot), reinterpret_cast<const D3DXMATRIX*>(&start));
            result = temp;
            result.SetTranslation(vPos);
        }

        PblVector3 vDelta = gVectorZero;
        if (GetKeys(fKeyTime, key0, key1, false, 0, &ucKey))
        {
            InterpolateKeys(key0, key1, fKeyTime, key);
            vDelta = key.mVal - vLastPos;
            vLastPos = key.mVal;
        }

        PblVector3 vPos = result.GetTranslation();
        vPos += result.GetXAxis() * vDelta.x;
        vPos += result.GetYAxis() * vDelta.y;
        vPos += result.GetZAxis() * vDelta.z;
        result.SetTranslation(vPos);

        fCurTime = fNextTime;
    } while (fEndTime > fCurTime);

    fTime = fEndTime;
}

// FUNCTION: SWBF2 0x00627f10
void PAAnimation::Update(float fDeltaTime, float& fTime, CollisionObject* pObject, const PblMatrix& start) const
{
    if (pObject->CanProcedurallyAnimate())
    {
        PblMatrix matrix = pObject->GetMatrix();
        CalculateNextMatrix(fDeltaTime, fTime, start, matrix);
        pObject->SetMatrix(matrix);
    }
    PAAnimationGroup::UpdateTreeGrid(pObject);
}

// FUNCTION: SWBF2 0x00627fb0
PAAnimationGroup::PAAnimationGroup()
    : mActive(&mHostActive), mHostActive(false), mClientActive(false), mEndTime(-1.0f), mStartAnim(0),
      mNumAnims(0), mStopWhenControlled(false), mDisableHierarchies(false)
{
}

// SYNTHETIC: SWBF2 0x00627fe0
// PAAnimationGroup::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00628010
PAAnimationGroup::~PAAnimationGroup()
{
}

// FUNCTION: SWBF2 0x00628020
bool PAAnimationGroup::AddAnimation(unsigned int uiAnimHash, unsigned int uiObjHash)
{
    const unsigned char ucAnim = sNextAvailableAnim;
    if (ucAnim >= 255)
    {
        return false;
    }
    if (!mNumAnims)
    {
        mStartAnim = ucAnim;
    }
    ++mNumAnims;
    sPAAnims[ucAnim].mAnimHash = uiAnimHash;
    sPAAnims[ucAnim].mObjHash = uiObjHash;
    sPAAnims[ucAnim].mTimer = &sPAAnims[ucAnim].mHostTimer;
    *sPAAnims[ucAnim].mTimer = 0.0f;
    sPAAnims[ucAnim].mClientTimer = 0.0f;
    sPAAnims[ucAnim].mInitialMatrix = gMatrixIdentity;
    sPAAnims[ucAnim].mGeometry = &sPAAnims[ucAnim].mHostGeometry;
    *sPAAnims[ucAnim].mGeometry = NULL;
    sPAAnims[ucAnim].mClientGeometry = NULL;
    sNextAvailableAnim = ucAnim + 1;
    return true;
}

// FUNCTION: SWBF2 0x006280f0
void PAAnimationGroup::Play()
{
    *mActive = true;
}

// FUNCTION: SWBF2 0x00628100
void PAAnimationGroup::Pause()
{
    *mActive = false;
}

// FUNCTION: SWBF2 0x00628110
void PAAnimationGroup::SetCurrentTime(float fTime)
{
    for (int i = mStartAnim; i < mStartAnim + mNumAnims; ++i)
    {
        *sPAAnims[i].mTimer = fTime;
    }
}

// FUNCTION: SWBF2 0x00628150
void PAAnimationGroup::SetEndTime(float fTime)
{
    mEndTime = fTime;
}

// FUNCTION: SWBF2 0x00628160
void PAAnimationGroup::UpdateStartPositions()
{
    for (int i = mStartAnim; i < mStartAnim + mNumAnims; ++i)
    {
        if (*sPAAnims[i].mGeometry)
        {
            sPAAnims[i].mInitialMatrix = (*sPAAnims[i].mGeometry)->GetMatrix();
        }
    }
}

// FUNCTION: SWBF2 0x006281d0
void PAAnimationGroup::Read(PblFileChunk& chunk)
{
    PAAnimationGroup* pGroup = NULL;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        char acName[128];
        switch (child._iId)
        {
        case 0x49484F4E:
            pGroup->mDisableHierarchies = true;
            break;
        case 0x4D494E41:
        {
            if (!pGroup)
            {
                return;
            }
            unsigned int uiAnimHash = PblHash(child.ReadString(acName, 128));
            unsigned int uiObjHash = PblHash(child.ReadString(acName, 128));
            pGroup->AddAnimation(uiAnimHash, uiObjHash);
            break;
        }
        case 0x4F464E49:
        {
            char* pcName = child.ReadString(acName, 128);
            pGroup = new PAAnimationGroup;
            if (!pGroup)
            {
                return;
            }
            if (!_HashTable.Find(PblHash(pcName)))
            {
                _HashTable.Store(PblHash(pcName), pGroup);
            }
            *pGroup->mActive = child._pFile->Read8() != 0;
            pGroup->mStopWhenControlled = child._pFile->Read8() != 0;
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x006285c0
void PAAnimationGroup::GeometryCreated(unsigned int uObjHash, CollisionObject* pObject,
                                       const PblMatrix& matrix)
{
    if (!uObjHash)
    {
        return;
    }

    for (int i = 0; i < 255; ++i)
    {
        if (sPAAnims[i].mObjHash == uObjHash)
        {
            *sPAAnims[i].mGeometry = pObject;
            sPAAnims[i].mInitialMatrix = matrix;
            pObject->mProcedurallyAnimated = true;
        }
    }

    for (int h = 0; h < mNumHierarchies; ++h)
    {
        if (mHierarchies[h]->mRoot.mObjHash == uObjHash)
        {
            *mHierarchies[h]->mRoot.mGeometry = pObject;
            pObject->mProcedurallyAnimated = true;
        }
        for (int n = 0; n < mHierarchies[h]->mNumObjs; ++n)
        {
            if (mHierarchies[h]->mObjs[n].mObjHash == uObjHash)
            {
                *mHierarchies[h]->mObjs[n].mGeometry = pObject;
                pObject->mProcedurallyAnimated = true;
            }
            CollisionObject* pRoot = *mHierarchies[h]->mRoot.mGeometry;
            if (pRoot && *mHierarchies[h]->mObjs[n].mGeometry)
            {
                PblMatrix inverse = pRoot->GetMatrix().GetInverseFast();
                mHierarchies[h]->mObjs[n].mLocalMatrix =
                    (*mHierarchies[h]->mObjs[n].mGeometry)->GetMatrix() * inverse;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00628820
void PAAnimationGroup::GeometryDeleted(CollisionObject* pObject)
{
    for (int i = 0; i < 255; ++i)
    {
        if (*sPAAnims[i].mGeometry == pObject)
        {
            *sPAAnims[i].mGeometry = 0;
        }
    }

    for (int h = 0; h < mNumHierarchies; ++h)
    {
        PAHierarchy* pHierarchy = mHierarchies[h];
        if (*pHierarchy->mRoot.mGeometry == pObject)
        {
            *pHierarchy->mRoot.mGeometry = 0;
        }
        for (int n = 0; n < pHierarchy->mNumObjs; ++n)
        {
            if (*pHierarchy->mObjs[n].mGeometry == pObject)
            {
                *pHierarchy->mObjs[n].mGeometry = 0;
            }
        }
    }
}

// FUNCTION: SWBF2 0x006288e0
void PAAnimationGroup::UpdateTreeGrid(CollisionObject* pObject)
{
    if (pObject && CollisionManager::sInstance && CollisionManager::sInstance->mTreeGrid &&
        pObject->IsInCollisionList())
    {
        const float f2dRadius = pObject->m2dRadius;
        CollisionManager::sInstance->mTreeGrid->Update(pObject, pObject->GetCollisionCenter(),
                                                       pObject->GetCollisionRadius() * 2.0f,
                                                       f2dRadius * 2.0f);
    }
}

// FUNCTION: SWBF2 0x00628960
void PAAnimationGroup::UpdateAnimations(float fDeltaTime)
{
    if (!*mActive || !mNumAnims)
    {
        return;
    }

    for (int i = mStartAnim; i < mStartAnim + mNumAnims; ++i)
    {
        PAAnimationData& data = sPAAnims[i];
        PblMatrix& initial = data.mInitialMatrix;
        PAAnimation* pAnim = PAAnimation::_HashTable.Find(data.mAnimHash);
        if (!pAnim || !*data.mGeometry)
        {
            continue;
        }

        if (mStopWhenControlled && (*data.mGeometry)->GetGameObject() &&
            ((PAGameObjectTeam*)(*data.mGeometry)->GetGameObject())->mTeam)
        {
            *data.mTimer = 0.0f;
            initial = (*data.mGeometry)->GetMatrix();
        }
        else if (mEndTime > 0.0f)
        {
            if (*data.mTimer >= mEndTime)
            {
                *mActive = false;
            }
            else if (*data.mTimer + fDeltaTime > mEndTime)
            {
                pAnim->Update(mEndTime - *data.mTimer, *data.mTimer, *data.mGeometry, initial);
                *mActive = false;
            }
            else
            {
                pAnim->Update(fDeltaTime, *data.mTimer, *data.mGeometry, initial);
            }
        }
        else
        {
            pAnim->Update(fDeltaTime, *data.mTimer, *data.mGeometry, initial);
        }

        PblMatrix matrix = (*data.mGeometry)->GetMatrix();
        if (mDisableHierarchies)
        {
            continue;
        }

        for (int h = 0; h < mNumHierarchies; ++h)
        {
            if (mHierarchies[h]->mRoot.mObjHash != data.mObjHash)
            {
                continue;
            }
            for (int n = 0; n < mHierarchies[h]->mNumObjs; ++n)
            {
                CollisionObject* pObject = *mHierarchies[h]->mObjs[n].mGeometry;
                if (pObject && pObject->CanProcedurallyAnimate())
                {
                    (*mHierarchies[h]->mObjs[n].mGeometry)->SetMatrix(mHierarchies[h]->mObjs[n].mLocalMatrix * matrix);
                    UpdateTreeGrid(*mHierarchies[h]->mObjs[n].mGeometry);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x00628c20
void PAAnimationGroup::WriteObject(NetPktGroup& group)
{
    group.Write(*mActive);
    if (!*mActive || !mNumAnims)
    {
        return;
    }

    for (int i = mStartAnim; i < mStartAnim + mNumAnims; ++i)
    {
        if (!*sPAAnims[i].mGeometry)
        {
            continue;
        }

        PAAnimation* pAnim = PAAnimation::_HashTable.Find(sPAAnims[i].mAnimHash);
        int iBits = 1;
        const int iDeciseconds = (int)(pAnim->mTotalTime * 10.0f);
        while ((1 << (iBits - 1)) < iDeciseconds && iBits < 32)
        {
            ++iBits;
        }
        group.WriteFixed(*sPAAnims[i].mTimer, 0.0f, pAnim->mTotalTime, iBits);
        if (pAnim->mLocalTranslation)
        {
            WriteMatrix(group, (*sPAAnims[i].mGeometry)->GetMatrix());
        }
    }
}

// FUNCTION: SWBF2 0x00628d30
void PAAnimationGroup::ReadObject(NetPktGroup& group)
{
    group.Read(*mActive);
    if (!*mActive || !mNumAnims)
    {
        return;
    }

    for (int i = mStartAnim; i < mStartAnim + mNumAnims; ++i)
    {
        if (!*sPAAnims[i].mGeometry)
        {
            continue;
        }
        PAAnimation* pAnim = PAAnimation::_HashTable.Find(sPAAnims[i].mAnimHash);
        int iBits = 1;
        const int iDeciseconds = (int)(pAnim->mTotalTime * 10.0f);
        while ((1 << (iBits - 1)) < iDeciseconds && iBits < 32)
        {
            ++iBits;
        }
        group.ReadFixed(*sPAAnims[i].mTimer, 0.0f, pAnim->mTotalTime, iBits);
        if (pAnim->mLocalTranslation)
        {
            PblMatrix matrix;
            ReadMatrix(group, matrix);
            if ((*sPAAnims[i].mGeometry)->CanProcedurallyAnimate())
            {
                (*sPAAnims[i].mGeometry)->SetMatrix(matrix);
            }
        }
        UpdateTreeGrid(*sPAAnims[i].mGeometry);
    }
}

// FUNCTION: SWBF2 0x00629540
void PAManager::ReadObjects(NetPktGroup& group)
{
    for (PblHashTable<PAAnimationGroup, 32>::Itor i = PAAnimationGroup::_HashTable.Begin();
         i != PAAnimationGroup::_HashTable.End(); ++i)
    {
        (*i).ReadObject(group);
    }
}

// FUNCTION: SWBF2 0x006294c0
void PAManager::WriteObjects(NetPktGroup& group)
{
    for (PblHashTable<PAAnimationGroup, 32>::Itor i = PAAnimationGroup::_HashTable.Begin();
         i != PAAnimationGroup::_HashTable.End(); ++i)
    {
        (*i).WriteObject(group);
    }
}

// FUNCTION: SWBF2 0x00628e70
void PAManagerInterpolator::Create()
{
    mIsFrameUpdate = false;
    mTotalTurnTime = 0.0f;
    mNextTotalTurnTime = 0.0f;
    mTotalFrameTime = 0.0f;
    if (!netFrameLock)
    {
        ActivateThread(0, PER_FRAME);
    }
}

// FUNCTION: SWBF2 0x00628f60
PAManager::PAManager()
{
    ActivateThread(0, PER_TURN);
    if (NetGame::IsEnabled() && NetGame::IsOneWorld() && NetGame::IsHost())
    {
        pamanagerInterpolator.Create();
    }
}

// SYNTHETIC: SWBF2 0x00628ff0
// PAManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00629020
PAManager::~PAManager()
{
}

// FUNCTION: SWBF2 0x00629030
void PAManager::Create()
{
    sInstance = sHostInstance = new PAManager;
    sClientInstance = NULL;
    PAAnimationGroup::mNumHierarchies = 0;
    for (int i = 0; i < 255; ++i)
    {
        sPAAnims[i].mHostGeometry = NULL;
        sPAAnims[i].mGeometry = &sPAAnims[i].mHostGeometry;
    }
}

// FUNCTION: SWBF2 0x006290c0
void PAManager::Destroy()
{
    if (NetGame::IsEnabled() && NetGame::IsOneWorld() && NetGame::IsHost())
    {
        pamanagerInterpolator.Destroy();
    }
    delete sHostInstance;
    sHostInstance = NULL;
    sNextAvailableKey = 0;
    PAAnimation::_HashTable.Clear();
    sNextAvailableAnim = 0;
    PAAnimationGroup::_HashTable.Clear();
    PAAnimationGroup::mNumHierarchies = 0;
}

// FUNCTION: SWBF2 0x00629230
bool PAManager::Update(float fDeltaTime)
{
    if (netIsLocalTurn && !pamanagerInterpolator.mIsFrameUpdate)
    {
        pamanagerInterpolator.mNextTotalTurnTime += fDeltaTime;
    }

    for (PblHashTable<PAAnimationGroup, 32>::Itor i = PAAnimationGroup::_HashTable.Begin();
         i != PAAnimationGroup::_HashTable.End(); ++i)
    {
        (*i).UpdateAnimations(fDeltaTime);
    }
    return true;
}
