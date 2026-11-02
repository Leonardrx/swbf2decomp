#include <d3dx9math.h>
#include <math.h>
#include "PebbleFL/Common/PblVector.h"
#include <stdio.h>

#include "Cloth.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/ext/hashtable.h"
#include "RedEngineFL/Util/ext/ListAllocator.h"
#include <string.h>
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "NetGame.h"

#include "PebbleFL/Common/PblRegion.h"
#include "SoundFL/source/win/sndengine.h"
#include "PebbleFL/Common/PblTime.h"
#include "FLInputManager.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatUtil.h"
#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "BootDisplay.h"
#include "BoundaryManager.h"
#include "CaptureDisplay.h"
#include "CollisionMesh.h"
#include "EntityHologram.h"
#include "GameLoop.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "LockOnManager.h"
#include "LuaScript.h"
#include "MessageDisplay.h"
#include "MuzzleFlash.h"
#include "OrdnanceGrapplingHook.h"
#include "OrdnanceTowCable.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterObject.h"
#include "PatrolPath.h"
#include "PauseMenu.h"
#include "ScreenTransition.h"
#include "SkyDome.h"
#include "SpectatorDisplay.h"
#include "StatIconDisplay.h"
#include "Tracker.h"
#include "VehicleSpawn.h"
#include "Battlefront2/Platform/Pc/pcGrassPatch.h"
#include "Battlefront2/Platform/Pc/pcLeafPatch.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Interface/RedGlyphCacheElement.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Util/ext/IAllocator.h"
#include "CollisionManager.h"
#include "CommandLine.h"
#include "GameMovie.h"
#include "ProceduralAnimation.h"
#include "ShellLoop.h"
#include "RedEngineFL/Interface/RedShadowBlob.h"

// SYNTHETIC: SWBF2 0x004023e0
// `dynamic initializer for 'EntityCloth::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766df0
// `dynamic atexit destructor for 'EntityCloth::sMemoryPool''

// GLOBAL: SWBF2 0x01eb1080
// EntityCloth::sMemoryPool
MemoryPool EntityCloth::sMemoryPool("EntityCloth", sizeof(EntityCloth));
// SYNTHETIC: SWBF2 0x004023c0
// `dynamic initializer for 'ClothData::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766de0
// `dynamic atexit destructor for 'ClothData::sMemoryPool''

// GLOBAL: SWBF2 0x01eb10d0
// ClothData::sMemoryPool
MemoryPool ClothData::sMemoryPool("ClothData", sizeof(ClothData));
// SYNTHETIC: SWBF2 0x00402400
// `dynamic initializer for 'ClothData::s_clothDataHashTable''

// GLOBAL: SWBF2 0x01eb1120
PblHashTable<ClothData, 64> ClothData::s_clothDataHashTable;

// FUNCTION: SWBF2 0x00459010
void ClothData::Init()
{
    EntityCloth::sMemoryPool.Setup(0x10, EntityCloth::sMemoryPool.mSize,
                                   EntityCloth::sMemoryPool.mGrow);
    ClothData::sMemoryPool.Setup(0x20, ClothData::sMemoryPool.mSize,
                                 ClothData::sMemoryPool.mGrow);

    s_clothDataHashTable.Clear();
}

// FUNCTION: SWBF2 0x00459370
void ClothData::_Read(PblFileChunk& chunk)
{
    m_uiNumCollisionVolumes = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        char acString[128];
        switch (child._iId)
        {
        case 0x4D524658:
        {
            float x = child._pFile->ReadFloat();
            float y = child._pFile->ReadFloat();
            float z = child._pFile->ReadFloat();
            m_Transform.SetXAxis(x, y, z, 0.0f);

            x = child._pFile->ReadFloat();
            y = child._pFile->ReadFloat();
            z = child._pFile->ReadFloat();
            m_Transform.SetYAxis(x, y, z, 0.0f);

            x = child._pFile->ReadFloat();
            y = child._pFile->ReadFloat();
            z = child._pFile->ReadFloat();
            m_Transform.SetZAxis(x, y, z, 0.0f);

            x = child._pFile->ReadFloat();
            y = child._pFile->ReadFloat();
            z = child._pFile->ReadFloat();
            m_Transform.SetTranslation(x, y, z, 1.0f);
            break;
        }
        case 0x41544144:
        {
            child.ReadString(acString, sizeof(acString));
            m_uiTextureNameHash = PblHash(acString);
            m_uiNumParticles = child._pFile->Read32();
            m_pPos = (PblVector3*)RedAllocFromHeap(__RedCurrHeap, m_uiNumParticles * sizeof(PblVector3), 0);
            m_pTexCoords = (PblVector3*)RedAllocFromHeap(__RedCurrHeap, m_uiNumParticles * sizeof(PblVector3), 0);
            for (unsigned int i = 0; i < m_uiNumParticles; i++)
            {
                float fX = child._pFile->ReadFloat();
                float fY = child._pFile->ReadFloat();
                float fZ = child._pFile->ReadFloat();
                PblVector3& vPos = m_pPos[i];
                vPos.x = fX;
                vPos.y = fY;
                vPos.z = fZ;
            }
            for (unsigned int i = 0; i < m_uiNumParticles; i++)
            {
                float fU = child._pFile->ReadFloat();
                float fV = child._pFile->ReadFloat();
                PblVector3& vTex = m_pTexCoords[i];
                vTex.x = fU;
                vTex.y = fV;
                vTex.z = 0.0f;
            }
            m_uiNumFixedPoints = child._pFile->Read32();
            if (child._pFile->Read32())
            {
                m_pFixedPointsBones = (unsigned int*)RedAllocFromHeap(__RedCurrHeap, m_uiNumFixedPoints * sizeof(unsigned int), 0);
                for (unsigned int i = 0; i < m_uiNumFixedPoints; i++)
                {
                    child.ReadString(acString, sizeof(acString));
                    m_pFixedPointsBones[i] = PblTEMPHash(acString);
                }
            }
            else
            {
                m_pFixedPointsBones = 0;
            }
            m_uiNumTriangles = child._pFile->Read32();
            m_pTriangleIndices = (unsigned short*)RedAllocFromHeap(__RedCurrHeap, m_uiNumTriangles * 3 * sizeof(unsigned short), 0);
            for (unsigned int i = 0; i < m_uiNumTriangles; i++)
            {
                m_pTriangleIndices[i * 3 + 0] = (unsigned short)child._pFile->Read32();
                m_pTriangleIndices[i * 3 + 1] = (unsigned short)child._pFile->Read32();
                m_pTriangleIndices[i * 3 + 2] = (unsigned short)child._pFile->Read32();
            }
            m_uiNumStretchPairs = child._pFile->Read32();
            m_pStretchPairs = (ClothConstraint*)RedAllocFromHeap(__RedCurrHeap, m_uiNumStretchPairs * sizeof(ClothConstraint), 0);
            for (unsigned int i = 0; i < m_uiNumStretchPairs; i++)
            {
                m_pStretchPairs[i].i0 = (unsigned short)child._pFile->Read32();
                m_pStretchPairs[i].i1 = (unsigned short)child._pFile->Read32();
            }
            m_uiNumBendPairs = child._pFile->Read32();
            m_pBendPairs = (ClothConstraint*)RedAllocFromHeap(__RedCurrHeap, m_uiNumBendPairs * sizeof(ClothConstraint), 0);
            for (unsigned int i = 0; i < m_uiNumBendPairs; i++)
            {
                m_pBendPairs[i].i0 = (unsigned short)child._pFile->Read32();
                m_pBendPairs[i].i1 = (unsigned short)child._pFile->Read32();
            }
            m_uiNumCrossPairs = child._pFile->Read32();
            m_pCrossPairs = (ClothConstraint*)RedAllocFromHeap(__RedCurrHeap, m_uiNumCrossPairs * sizeof(ClothConstraint), 0);
            for (unsigned int i = 0; i < m_uiNumCrossPairs; i++)
            {
                m_pCrossPairs[i].i0 = (unsigned short)child._pFile->Read32();
                m_pCrossPairs[i].i1 = (unsigned short)child._pFile->Read32();
            }
            break;
        }
        case 0x454D414E:
            child.ReadString(acString, sizeof(acString));
            m_uiNameHash = PblHash(acString);
            break;
        case 0x544E5250:
            child.ReadString(acString, sizeof(acString));
            m_uiParentHash = PblTEMPHash(acString);
            break;
        case 0x4F464E49:
            child.ReadString(acString, sizeof(acString));
            m_uiModelHash = PblHash(acString);
            break;
        case 0x4C4C4F43:
        {
            m_uiNumCollisionVolumes = child._pFile->Read32();
            m_pCollisionVolumes = new ClothCollision[m_uiNumCollisionVolumes];
            for (unsigned int i = 0; i < m_uiNumCollisionVolumes; i++)
            {
                char acParent[128];
                child.ReadString(acParent, sizeof(acParent));
                m_pCollisionVolumes[i].parent = PblTEMPHash(acParent);
                m_pCollisionVolumes[i].type = (ClothCollisionType)child._pFile->Read32();
                const float fWidth = child._pFile->ReadFloat();
                m_pCollisionVolumes[i].width = fWidth;
                const float fHeight = child._pFile->ReadFloat();
                m_pCollisionVolumes[i].height = fHeight;
                const float fDepth = child._pFile->ReadFloat();
                m_pCollisionVolumes[i].depth = fDepth;
                float x = child._pFile->ReadFloat();
                float y = child._pFile->ReadFloat();
                float z = child._pFile->ReadFloat();
                m_pCollisionVolumes[i].localMat.SetXAxis(x, y, z, 0.0f);

                x = child._pFile->ReadFloat();
                y = child._pFile->ReadFloat();
                z = child._pFile->ReadFloat();
                m_pCollisionVolumes[i].localMat.SetYAxis(x, y, z, 0.0f);

                x = child._pFile->ReadFloat();
                y = child._pFile->ReadFloat();
                z = child._pFile->ReadFloat();
                m_pCollisionVolumes[i].localMat.SetZAxis(x, y, z, 0.0f);

                x = child._pFile->ReadFloat();
                y = child._pFile->ReadFloat();
                z = child._pFile->ReadFloat();
                m_pCollisionVolumes[i].localMat.SetTranslation(x, y, z, 1.0f);
            }
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x00459ab0
EntityClothClass::EntityClothClass(unsigned int uHash)
    : EntityClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    mWindDirection = gVectorUnitX;
    mWindSpeed = 10.0f;
    mDampening = 0.5f;
    mDrag = -2.0f;
    mParticleMass = 1.0f;
    mMaxAcceleration = 20.0f;
    mMaxVelocity = 10.0f;
    m_uiDoCrossConstraints = 1;
    m_uiDoBendConstraints = 1;
    m_uiDoStretchConstraints = 1;
    m_uiHardEdge = 0;
    m_uiTransparent = 0;
    mModelHash = 0;
    mPriority = 0.0f;
}

namespace ClothBE
{
// GLOBAL: SWBF2 0x007e5eee
bool enabled_ = true;

typedef util::HashTable<ClothBank*, unsigned int, util::TNewArrayAlloc<ClothBank*> > ClothDataT;

// SYNTHETIC: SWBF2 0x00402420
// ClothBE::`dynamic initializer for 'clothData_''
// SYNTHETIC: SWBF2 0x00766e00
// ClothBE::`dynamic atexit destructor for 'clothData_''

// GLOBAL: SWBF2 0x01eb1330
// clothData_
ClothDataT clothData_;
}

// TEMPLATE: SWBF2 0x00458cb0
// util::HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >::~HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >
template ClothBE::ClothDataT::~HashTable();

// SYNTHETIC: SWBF2 0x0045c1f0
// util::HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >::HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >

// TEMPLATE: SWBF2 0x0045c2b0
// util::HashTable_<ClothBank *,unsigned int,util::Vector<ClothBank *,util::BaseArray<ClothBank *,util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > > >::Clear
template void util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                               ClothBE::ClothDataT::KeyVecT,
                               ClothBE::ClothDataT::HashVecT>::Clear();

// TEMPLATE: SWBF2 0x0045c660
// util::BaseArray<ClothBank *,util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> > >::Reserve_

// TEMPLATE: SWBF2 0x0045c720
// util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > >::Reserve_

// SYNTHETIC: SWBF2 0x004023a0
// `dynamic initializer for 'EntityCloth::rttiHashEntityCloth''
// EntityCloth::rttiHashEntityCloth

// GLOBAL: SWBF2 0x01eb0fcc
// EntityCloth::rttiHashEntityCloth
PblHash EntityCloth::rttiHashEntityCloth("EntityCloth");
// GLOBAL: SWBF2 0x00788a70
// EntityCloth::rttiNameEntityCloth
const char EntityCloth::rttiNameEntityCloth[] = "EntityCloth";

// SYNTHETIC: SWBF2 0x004023b0
// `dynamic initializer for 'EntityClothClass::rttiHashEntityClothClass''
// EntityClothClass::rttiHashEntityClothClass

// GLOBAL: SWBF2 0x01eb062c
// EntityClothClass::rttiHashEntityClothClass
PblHash EntityClothClass::rttiHashEntityClothClass("EntityClothClass");
// GLOBAL: SWBF2 0x00788a48
// EntityClothClass::rttiNameEntityClothClass
const char EntityClothClass::rttiNameEntityClothClass[] = "EntityClothClass";







// FUNCTION: SWBF2 0x00458b40
EntityClass* EntityCloth::GetEntityClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x00458b50
const EntityClass* EntityCloth::GetEntityClass() const
{
    return mClass;
}







// FUNCTION: SWBF2 0x00458bb0
void ClothBank::Push(EntityCloth* pCloth, float fPriority)
{
    if (!Find(pCloth))
    {
        cloth_.Add(pCloth);
    }
}

// FUNCTION: SWBF2 0x00458c10
bool ClothBank::Find(EntityCloth* pCloth)
{
    return cloth_.FindFirst(pCloth) != cloth_.End();
}

// FUNCTION: SWBF2 0x00458c60
bool ClothBank::Remove(EntityCloth* pCloth)
{
    return cloth_.FindRemove(pCloth);
}

inline EntityCloth* ClothBank::GetTop(float fPriority)
{
    lowPriority_ = 1000000.0f;
    for (int i = 0; i < cloth_.GetSize(); i++)
    {
        if (cloth_[i])
        {
            float fTop = cloth_[i]->GetPriority();
            lowPriority_ = util::Min_(lowPriority_, fTop);
            if (fPriority * 0.98f > fTop)
            {
                EntityCloth* pTop = cloth_[i];
                cloth_[i] = cloth_.End()[-1];
                cloth_._numElements--;
                return pTop;
            }
        }
    }
    lowPriority_ = 0.0f;
    return 0;
}

// FUNCTION: SWBF2 0x00458d60
bool EntityCloth::IsEnabled()
{
    return ClothBE::enabled_;
}

namespace ClothBE
{
// GLOBAL: SWBF2 0x007e5efc
unsigned int clothBaseMem_ = 0x8000;
// GLOBAL: SWBF2 0x007e5f00
unsigned int clothMaxMem_ = 0x8000;
extern util::IAllocator* gMultiAllocator_;
}

// FUNCTION: SWBF2 0x00458d70
void EntityCloth::SetEnable(bool bEnable)
{
    ClothBE::enabled_ = bEnable;
}

// FUNCTION: SWBF2 0x00458d80
void EntityCloth::SetMemUseage(unsigned int uBaseMem, unsigned int uMaxMem)
{
    ClothBE::clothBaseMem_ = uBaseMem;
    ClothBE::clothMaxMem_ = uMaxMem;
    if (GetSpecialAllocator())
    {
        GetSpecialAllocator()->SetMaxMem(uMaxMem);
    }
}

// FUNCTION: SWBF2 0x00458db0
util::IAllocator* EntityCloth::GetSpecialAllocator()
{
    if (ClothBE::clothBaseMem_ + ClothBE::clothMaxMem_ < 0x1000)
    {
        return 0;
    }
    if (!ClothBE::gMultiAllocator_)
    {
        ClothBE::gMultiAllocator_ = util::ListAllocator::Create(ClothBE::clothBaseMem_, ClothBE::clothMaxMem_);
    }
    return ClothBE::gMultiAllocator_;
}

// SYNTHETIC: SWBF2 0x00459b50
// EntityClothClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00459b80
EntityClothClass::~EntityClothClass()
{
}

static const PblVector3 RotateY(const PblVector3& v, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    return PblVector3(fCos * v.x - fSin * v.z, v.y, fSin * v.x + fCos * v.z);
}

// FUNCTION: SWBF2 0x00459b90
void EntityClothClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x13901A72:
        sscanf(pszValue, "%f", &mWindSpeed);
        break;
    case 0x415E2B30:
    {
        float fAngleZ = 0.0f;
        float fAngleY = 0.0f;
        sscanf(pszValue, "%f %f", &fAngleZ, &fAngleY);
        const float fRadiansZ = fAngleZ * 0.017453292f;
        const PblVector3 vWind(-sinf(fRadiansZ), cosf(fRadiansZ), 0.0f);
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, fAngleY * 0.017453292f);
        mWindDirection = PblVector3(fCos * vWind.x - fSin * vWind.z, vWind.y, fSin * vWind.x + fCos * vWind.z);
        break;
    }
    case 0x4E994B61:
        sscanf(pszValue, "%f", &mMaxAcceleration);
        break;
    case 0x5CAF15E1:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        m_uiHardEdge = iValue > 1 ? 1 : iValue;
        break;
    }
    case 0x6EB957F3:
        sscanf(pszValue, "%f", &mParticleMass);
        break;
    case 0x8311CD99:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        m_uiDoStretchConstraints = iValue > 1 ? 1 : iValue;
        break;
    }
    case 0x94DD79B9:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        m_uiDoBendConstraints = iValue > 1 ? 1 : iValue;
        break;
    }
    case 0x94E4E309:
        sscanf(pszValue, "%f", &mPriority);
        break;
    case 0xAFE34731:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        m_uiTransparent = iValue > 1 ? 1 : iValue;
        break;
    }
    case 0xB88C0D43:
        sscanf(pszValue, "%f", &mDrag);
        break;
    case 0xC1133964:
        sscanf(pszValue, "%f", &mDampening);
        break;
    case 0xDE112858:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        m_uiDoCrossConstraints = (iValue > 1 ? 1 : iValue) != 0;
        break;
    }
    case 0xE90ECACC:
        mModelHash = PblHash(pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00459eb0
EntityClass* EntityClothClass::Derive(unsigned int uHash)
{
    return new EntityClothClass(uHash);
}

// FUNCTION: SWBF2 0x00459f10
Entity* EntityClothClass::Build(const EntityDesc& desc)
{
    if (!EntityCloth::IsEnabled())
    {
        return 0;
    }

    EntityCloth::GetSpecialAllocator()->ClearMemHash();

    unsigned int uHashID = 0;
    EntityCloth* pCloth = new EntityCloth(this, desc);
    pCloth->SetPriority(mPriority + desc.mPriority);

    ClothBank* pBank = ClothBE::clothData_[uHashID];
    if (!pBank)
    {
        pBank = new ClothBank(uHashID);
        ClothBE::clothData_[uHashID] = pBank;
    }
    if (pCloth)
    {
        pCloth->bank_ = pBank;
        if (pBank)
        {
            pBank->Push(pCloth, pCloth->GetPriority());
        }
    }
    pCloth->CreateData();
    return pCloth;
}

// FUNCTION: SWBF2 0x0045a090
void EntityCloth::CreateData_()
{
    if (!m_pPos)
    {
        Init(mClass, mDesc);
        if (m_pPos && bank_)
        {
            bank_->Push(this, GetPriority());
        }
    }
}

// FUNCTION: SWBF2 0x0045a0d0
void EntityCloth::CreateData()
{
    float fPriority = GetPriority();
    ClothBank* pBank = bank_;

    CreateData_();
    while (!m_pPos)
    {
        ClearData();
        if (!pBank)
        {
            return;
        }
        EntityCloth* pTop = pBank->GetTop(fPriority);
        if (!pTop)
        {
            return;
        }
        pTop->ClearData();
        CreateData_();
        if (m_pPos)
        {
            return;
        }
        ClearData();
    }
}

// FUNCTION: SWBF2 0x0045a1b0
void EntityCloth::SetPriority(float fPriority)
{
    priority_ = fPriority;
}

// FUNCTION: SWBF2 0x0045a1c0
float EntityCloth::GetPriority()
{
    float fFresh = 1.0f - (float)(RedRenderer::GetFrameNumber() - m_lastFrameVisible) / 30.0f;
    float fFloor = 0.1f;
    return (priority_ + 1.0f) * util::Max_(fFresh, fFloor) * 100.0f;
}

// SYNTHETIC: SWBF2 0x0045cb32
// ??_EEntityCloth@@W3AEPAXI@Z
// SYNTHETIC: SWBF2 0x0045a230
// EntityCloth::`scalar deleting destructor'

namespace util
{
void Deallocate_(void* p, unsigned int uiSize, util::IAllocator& rAlloc);
}

// FUNCTION: SWBF2 0x0045a610
EntityCloth::~EntityCloth()
{
    if (IsEnabled())
    {
        if (bank_)
        {
            bank_->Remove(this);
        }
        ClearData();
    }
}

// FUNCTION: SWBF2 0x0045a690
void EntityCloth::ClearData()
{
    DeactivateThread();
    util::Deallocate_(m_pPos, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pOldPos, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pNormal, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pAcceleration, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pFixedPos, ((ClothData*)mData)->GetNumFixedPoints_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pFixedPos = 0;
    m_pAcceleration = 0;
    m_pNormal = 0;
    m_pOldPos = 0;
    m_pPos = 0;
}


// FUNCTION: SWBF2 0x0040fe10
void ClothData::PlatformBindInit(EntityClothClass* pClass)
{
    ClothData* pData = (ClothData*)this;
    if (pData->m_pShader)
    {
        pData->m_pShader->Release();
    }

    unsigned int uiMode = 1;
    if (pClass->m_uiHardEdge)
    {
        uiMode = 3;
    }
    else if (pClass->m_uiTransparent)
    {
        uiMode = 5;
    }

    pData->m_pShader = pcRedShader::Create(kuiNormalShader, 0x222, &pData->m_uiTextureNameHash, 1, uiMode,
                                           RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x0045a260
EntityCloth::EntityCloth(EntityClothClass* pClass, const EntityDesc& desc)
    : mClass(pClass)
{
    m_pFixedPos = 0;
    m_pAcceleration = 0;
    m_pNormal = 0;
    m_pOldPos = 0;
    m_pPos = 0;
    mDesc = desc;
    bank_ = 0;
    priority_ = 0.0f;
    CreateData_();
}

// FUNCTION: SWBF2 0x0045a3a0
unsigned int ClothData::GetNumParticles_() const
{
    const ClothData* pData = (const ClothData*)this;
    return pData->m_uiNumParticles;
}

// FUNCTION: SWBF2 0x0045a3b0
unsigned int ClothData::GetNumFixedPoints_() const
{
    const ClothData* pData = (const ClothData*)this;
    return pData->m_uiNumFixedPoints;
}

// FUNCTION: SWBF2 0x0045a3c0
bool EntityCloth::Init(EntityClothClass* pClass, const EntityDesc& desc)
{
    if (m_pPos)
    {
        return true;
    }
    if (!IsEnabled())
    {
        return false;
    }

    mUpdateDt = 0.039f;
    mRenderDt = 0.0f;
    mData = ClothData::s_clothDataHashTable.Find(pClass->mId);
    ((ClothData*)mData)->PlatformBindInit(pClass);

    mOldVelocity = mVelocity = mAcceleration = PblVector3(0.0f, 0.0f, 0.0f);
    m_pPos = m_pOldPos = m_pNormal = m_pAcceleration = m_pFixedPos = 0;

    m_pPos = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pOldPos = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pNormal = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pAcceleration = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pFixedPos = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumFixedPoints_() * sizeof(PblVector3), *GetSpecialAllocator());

    if (m_pPos && m_pOldPos && m_pNormal && m_pAcceleration && m_pFixedPos)
    {
        memcpy(m_pPos, mData->m_pPos, mData->m_uiNumParticles * sizeof(PblVector3));
        memcpy(m_pOldPos, mData->m_pPos, mData->m_uiNumParticles * sizeof(PblVector3));
        SetupFixedPos();
        mOldPosition = desc.mMatrix.GetTranslation();
        SetWorldMatrix(desc.mMatrix, mUpdateDt);
        ComputeNormals();
        m_lastFrameUpdated = 0;
        m_lastFrameVisible = 0;
        ActivateThread(0, PER_LOCAL_TURN);
        return true;
    }

    ClearData();
    return false;
}

// FUNCTION: SWBF2 0x0045a770
bool EntityCloth::Update(float fDeltaTime)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return false;
    }
    mUpdateDt = (fDeltaTime > 0.039f) ? 0.039f : fDeltaTime;
    mRenderDt = 0.0f;
    return true;
}

// FUNCTION: SWBF2 0x0045a7c0
void EntityCloth::SetupFixedPos()
{
    if (!m_pPos)
    {
        DeactivateThread();
        return;
    }

    if (!mData->m_pFixedPointsBones)
    {
        return;
    }

    const RedSkeleton* pSkeleton = RedModel::_HashTable.Find(mClass->mModelHash)->GetSkeleton();
    const PblMatrix* pParent = pSkeleton->GetBoneMatrixByName(mData->m_uiParentHash);

    for (unsigned int i = 0; i < mData->m_uiNumFixedPoints; ++i)
    {
        unsigned int uiBone = mData->m_pFixedPointsBones ? mData->m_pFixedPointsBones[i] : 0;
        PblMatrix mBoneInv = *pSkeleton->GetInvBoneMatrixByName(uiBone);
        PblMatrix mLocal;
        PblMatrix mFull;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mLocal), reinterpret_cast<const D3DXMATRIX*>(&mData->m_Transform), reinterpret_cast<const D3DXMATRIX*>(pParent));
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mFull), reinterpret_cast<const D3DXMATRIX*>(&mLocal), reinterpret_cast<const D3DXMATRIX*>(&mBoneInv));
        PblVector3 vFixed;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vFixed), reinterpret_cast<const D3DXVECTOR3*>(&m_pPos[i]), reinterpret_cast<const D3DXMATRIX*>(&mFull));
        m_pFixedPos[i] = vFixed;
    }
}

// FUNCTION: SWBF2 0x0045ab20
void EntityCloth::SetWorldMatrix(const PblMatrix& mWorld, float fDeltaTime)
{
    EntityCloth* pCloth = (EntityCloth*)this;
    const float fInvDt = 1.0f / fDeltaTime;
    EntityClothClass* pClass = pCloth->mClass;

    pCloth->m_worldMat = mWorld;
    pCloth->mOldVelocity = pCloth->mVelocity;

    const PblVector3& vPos = pCloth->m_worldMat.GetTranslation();
    pCloth->mVelocity = (vPos - pCloth->mOldPosition) * fInvDt;
    float fLength2 = SquaredLength(pCloth->mVelocity);
    if (fLength2 > pClass->mMaxVelocity * pClass->mMaxVelocity)
    {
        float fScale = 1.0f / sqrtf(fLength2);
        pCloth->mVelocity = pCloth->mVelocity * fScale * pClass->mMaxVelocity;
    }
    pCloth->mOldPosition = vPos;

    pCloth->mAcceleration = (pCloth->mVelocity - pCloth->mOldVelocity) * fInvDt;
    fLength2 = SquaredLength(pCloth->mAcceleration);
    if (fLength2 > pClass->mMaxAcceleration * pClass->mMaxAcceleration)
    {
        float fScale = 1.0f / sqrtf(fLength2);
        pCloth->mAcceleration = pCloth->mAcceleration * fScale * pClass->mMaxAcceleration;
    }
}

// FUNCTION: SWBF2 0x0045bd50
void EntityCloth::ComputeNormals()
{
    const unsigned short* pIndices = mData->m_pTriangleIndices;
    memset(m_pNormal, 0, mData->m_uiNumParticles * sizeof(PblVector3));

    const ClothData* pData = mData;
    const PblVector3* pPos = m_pPos;
    PblVector3* pN = m_pNormal;
    for (unsigned int t = 0; t < pData->m_uiNumTriangles; ++t, pIndices += 3)
    {
        const unsigned int i0 = pIndices[0];
        const unsigned int i1 = pIndices[1];
        const unsigned int i2 = pIndices[2];

        const float e1y = pPos[i1].y - pPos[i0].y;
        const float e1z = pPos[i1].z - pPos[i0].z;
        const float e2y = pPos[i2].y - pPos[i0].y;
        const float e2z = pPos[i2].z - pPos[i0].z;
        const float e2x = pPos[i2].x - pPos[i0].x;
        const float e1x = pPos[i1].x - pPos[i0].x;

        const float nx = e1y * e2z - e1z * e2y;
        const float ny = e1z * e2x - e2z * e1x;
        const float nz = e2y * e1x - e1y * e2x;

        pN[i0].x += nx;
        pN[i0].y += ny;
        pN[i0].z += nz;
        pN[i1].x += nx;
        pN[i1].y += ny;
        pN[i1].z += nz;
        pN[i2].x += nx;
        pN[i2].y += ny;
        pN[i2].z += nz;
    }

    for (unsigned int i = 0; i < mData->m_uiNumParticles; ++i)
    {
        PblVector3 vNormal;
        D3DXVec3Normalize(&vNormal, &m_pNormal[i]);
        m_pNormal[i] = vNormal;
    }
}

// TEMPLATE: SWBF2 0x0045c180
// util::HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >::operator[]
template ClothBank*& util::HashTable<ClothBank*, unsigned int, util::TNewArrayAlloc<ClothBank*> >::operator[](
    const unsigned int& rKey);

// TEMPLATE: SWBF2 0x0045c3c0
// util::HashTable_<ClothBank *,unsigned int,util::Vector<ClothBank *,util::BaseArray<ClothBank *,util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > > >::Get
template ClothBank*& util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                                      ClothBE::ClothDataT::KeyVecT,
                                      ClothBE::ClothDataT::HashVecT>::Get(const unsigned int& rKey);

// TEMPLATE: SWBF2 0x0045c420
// util::HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >::Reallocate
template void util::HashTable<ClothBank*, unsigned int, util::TNewArrayAlloc<ClothBank*> >::Reallocate(int iSize);

// TEMPLATE: SWBF2 0x0045c600
// util::HashTable_<ClothBank *,unsigned int,util::Vector<ClothBank *,util::BaseArray<ClothBank *,util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > > >::FindIndex_
template unsigned int util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                                       ClothBE::ClothDataT::KeyVecT,
                                       ClothBE::ClothDataT::HashVecT>::FindIndex_(unsigned int uiHash) const;

// TEMPLATE: SWBF2 0x0045c7e0
// util::BaseArray<EntityCloth *,util::TNewArrayAlloc<EntityCloth *> >::Reserve_
template bool util::BaseArray<EntityCloth*, util::TNewArrayAlloc<EntityCloth*> >::Reserve_(int iCount, bool bForce);

// TEMPLATE: SWBF2 0x0045c8a0
// util::Vector<ClothBank *,util::BaseArray<ClothBank *,util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> > > >::SetSize_
template bool util::Vector<ClothBank*, util::BaseArray<ClothBank*, util::TypedArrayAllocator<ClothBank*, util::TNewArrayAlloc<ClothBank*> > > >::SetSize_(
    int iSize, bool bForce);

// TEMPLATE: SWBF2 0x0045c9a0
// util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > >::SetSize_
template bool util::Vector<unsigned int, util::BaseArray<unsigned int, util::TypedArrayAllocator<unsigned int, util::TNewArrayAlloc<ClothBank*> > > >::SetSize_(
    int iSize, bool bForce);

// TEMPLATE: SWBF2 0x0045c950
// util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> >::Allocate_
template ClothBank** util::TypedArrayAllocator<ClothBank*, util::TNewArrayAlloc<ClothBank*> >::Allocate_(int& riCount);

// TEMPLATE: SWBF2 0x0045ca50
// util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> >::Allocate_
template unsigned int* util::TypedArrayAllocator<unsigned int, util::TNewArrayAlloc<ClothBank*> >::Allocate_(int& riCount);

// TEMPLATE: SWBF2 0x0045caa0
// util::TypedArrayAllocator<EntityCloth *,util::NewAllocator>::Allocate_
template EntityCloth** util::TypedArrayAllocator<EntityCloth*, util::NewAllocator>::Allocate_(int& riCount);

// TEMPLATE: SWBF2 0x0045cb10
// util::Max_<float>
template const float& util::Max_<float>(const float&, const float&);


// GLOBAL: SWBF2 0x007eb77c
static PblVector3 sGravity(0.0f, -9.8f, 0.0f);



// FUNCTION: SWBF2 0x0045b3b0
void EntityCloth::EnforceSphereCollision(const PblMatrix& mMatrix, float fRadius)
{
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        PblVector3 dir = m_pPos[i] - mMatrix.GetTranslation();
        float sqDist = SquaredLength(dir);
        if (sqDist < fRadius * fRadius)
        {
            m_pPos[i] = mMatrix.GetTranslation() + dir * (fRadius / (float)sqrt((double)sqDist));
        }
    }
}

// FUNCTION: SWBF2 0x0045b4e0
void EntityCloth::EnforceBoxCollision(const PblMatrix& mMatrix, float fWidth, float fHeight, float fDepth)
{
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        const PblVector3 vDelta = m_pPos[i] - mMatrix.GetTranslation();
        const float d0 = DotProduct(mMatrix.GetXAxis(), vDelta);
        const float p0 = fWidth - fabsf(d0);
        if (p0 <= 0.0f)
        {
            continue;
        }
        const float d1 = DotProduct(mMatrix.GetYAxis(), vDelta);
        const float p1 = fHeight - fabsf(d1);
        if (p1 <= 0.0f)
        {
            continue;
        }
        const float d2 = DotProduct(mMatrix.GetZAxis(), vDelta);
        const float p2 = fDepth - fabsf(d2);
        if (p2 <= 0.0f)
        {
            continue;
        }
        if (p1 > p0)
        {
            if (p2 > p0)
            {
                m_pPos[i] += mMatrix.GetXAxis() * (PblMath::Sign(d0) * p0);
            }
            else
            {
                m_pPos[i] += mMatrix.GetZAxis() * (PblMath::Sign(d2) * p2);
            }
        }
        else if (p2 > p1)
        {
            m_pPos[i] += mMatrix.GetYAxis() * (PblMath::Sign(d1) * p1);
        }
        else
        {
            m_pPos[i] += mMatrix.GetZAxis() * (PblMath::Sign(d2) * p2);
        }
    }
}

// FUNCTION: SWBF2 0x0045bf20
void EntityCloth::Render(const PblMatrix& mMatrix, const RedPose* pPose, unsigned char ucViewport, RedColor color,
                         unsigned int uiFlags, unsigned int uArg)
{
    m_lastFrameVisible = RedRenderer::GetFrameNumber();
    CreateData();
    if (!m_pPos || !IsEnabled())
    {
        return;
    }

    const PblMatrix* pParent = 0;
    if (pPose)
    {
        pParent = pPose->Find(mData->m_uiParentHash);
    }
    else if (mData->m_pFixedPointsBones)
    {
        return;
    }

    if (mUpdateDt > 0.0f)
    {
        float fRatio = NetGame::GetTurnRatio();
        if (fRatio <= 0.0f)
        {
            fRatio = 1.0f;
        }
        const float fStep = fRatio * mUpdateDt - mRenderDt;
        mRenderDt += fStep;
        if (fStep > 0.0f)
        {
            const PblMatrix& mBase = mData->m_Transform;
            PblMatrix mLocal;
            if (pParent)
            {
                mLocal = mBase * *pParent;
            }
            else
            {
                mLocal = mBase;
            }
            SetWorldMatrix(mLocal * mMatrix, fStep);
            InternalUpdate(mLocal, pPose, uArg, fStep);
        }
    }

    const float fRadius = mData->m_fBoundingRadius;
    const PblSphere& modelSphere = RedModel::_HashTable.Find(mData->m_uiModelHash)->_BoundingSphere;
    PblSphere sphere;
    sphere.SetCenter(PblVector3(modelSphere.x + m_worldMat.m[3][0], modelSphere.y + m_worldMat.m[3][1],
                                modelSphere.z + m_worldMat.m[3][2]));
    sphere.SetRadius(fRadius);

    const RedLightingState* pLightingState = 0;
    if ((int)uiFlags >= 0)
    {
        if (uiFlags & 0x4000000)
        {
            pLightingState = &RedInterfaceScreen::s_lightingState;
        }
        else
        {
            pLightingState = RedLight::GetLightingState(sphere, 4, false, false, false);
        }
    }
    PlatformRender(ucViewport, color, uiFlags, pLightingState, sphere);
}

// FUNCTION: SWBF2 0x0045adb0
bool EntityCloth::InternalUpdate(const PblMatrix& mMatrix, const RedPose* pPose, unsigned int uIterations,
                                 float fDeltaTime)
{
    if (IsEnabled() && m_lastFrameUpdated < RedRenderer::GetFrameNumber())
    {
        m_lastFrameUpdated = RedRenderer::GetFrameNumber();
        AccumulateForces(fDeltaTime);
        Verlet(fDeltaTime);
        SatisfyConstraints(mMatrix, pPose, uIterations);
        ComputeNormals();
    }
    return true;
}

// FUNCTION: SWBF2 0x0045ae10
void EntityCloth::Verlet(float fDeltaTime)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return;
    }

    const float fDt2 = fDeltaTime * fDeltaTime;
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        PblVector3& vPos = m_pPos[i];
        const PblVector3 vTemp = vPos;
        PblVector3& vOld = m_pOldPos[i];
        vPos += (vPos - vOld) + m_pAcceleration[i] * fDt2;
        vOld = vTemp;
    }
}

// FUNCTION: SWBF2 0x0045af10
void EntityCloth::AccumulateForces(float fDeltaTime)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return;
    }

    const float fWind = PblRandom::_Global.GetFloat() * mClass->mWindSpeed;
    const float fDrag = mClass->mDrag;
    const PblMatrix mInverse = m_worldMat.GetInverseFast();

    PblVector3 vWind;
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vWind), reinterpret_cast<const D3DXVECTOR3*>(&mClass->mWindDirection), reinterpret_cast<const D3DXMATRIX*>(&mInverse));
    const PblVector3 vWorldGravity = sGravity - mAcceleration * mClass->mDampening;
    PblVector3 vGravity;
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vGravity), reinterpret_cast<const D3DXVECTOR3*>(&vWorldGravity), reinterpret_cast<const D3DXMATRIX*>(&mInverse));
    PblVector3 vVelocity;
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vVelocity), reinterpret_cast<const D3DXVECTOR3*>(&mVelocity), reinterpret_cast<const D3DXMATRIX*>(&mInverse));

    const float fInvDt = 1.0f / fDeltaTime;
    const float fInvMass = 1.0f / mClass->mParticleMass;
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        const PblVector3 vDragForce = (vVelocity + (m_pPos[i] - m_pOldPos[i]) * fInvDt) * fDrag;
        const PblVector3& vNormal = m_pNormal[i];
        const float fWindForce = DotProduct(vNormal, vWind) * fWind;
        m_pAcceleration[i] = vGravity + (vNormal * fWindForce + vDragForce) * fInvMass;
    }
}

// FUNCTION: SWBF2 0x0045bc40
void EntityCloth::SatisfyConstraints(const PblMatrix& mMatrix, const RedPose* pPose, unsigned int uIterations)
{
    ClothConstraint* pStretchPairs = mData->m_pStretchPairs;
    ClothConstraint* pBendPairs = mData->m_pBendPairs;
    ClothConstraint* pCrossPairs = mData->m_pCrossPairs;
    RestoreFixedPoints(pPose);

    for (; uIterations > 0; --uIterations)
    {
        if (mClass->m_uiDoCrossConstraints)
        {
            for (unsigned int i = 0; i < mData->m_uiNumCrossPairs; ++i)
            {
                EnforceConstraint(pCrossPairs[i], 1.0f);
            }
        }
        if (mClass->m_uiDoBendConstraints)
        {
            for (unsigned int i = 0; i < mData->m_uiNumBendPairs; ++i)
            {
                EnforceConstraint(pBendPairs[i], 1.0f);
            }
        }
        if (mClass->m_uiDoStretchConstraints)
        {
            for (unsigned int i = 0; i < mData->m_uiNumStretchPairs; ++i)
            {
                EnforceConstraint(pStretchPairs[i], 1.0f);
            }
        }
        EnforceCollisions(mMatrix, pPose);
    }
}

// FUNCTION: SWBF2 0x0045a910
void EntityCloth::RestoreFixedPoints(const RedPose* pPose)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return;
    }
    if (!mData->m_pFixedPointsBones)
    {
        return;
    }

    const PblMatrix* pParent = pPose->Find(mData->m_uiParentHash);
    PblMatrix mParentInverse;
    mParentInverse.Reset();
    PblMatrix mTransformInverse;
    mTransformInverse.Reset();
    if (pParent)
    {
        mParentInverse = pParent->GetInverseFast();
        mTransformInverse = mData->m_Transform.GetInverseFast();
    }

    for (unsigned int i = 0; i < mData->m_uiNumFixedPoints; ++i)
    {
        const unsigned int uiBone = mData->m_pFixedPointsBones ? mData->m_pFixedPointsBones[i] : 0;
        const PblMatrix* pBone = pPose->Find(uiBone);
        PblMatrix mBone;
        mBone.Reset();
        if (pBone)
        {
            mBone = *pBone * mParentInverse * mTransformInverse;
        }
        PblVector3 vPos;
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vPos), reinterpret_cast<const D3DXVECTOR3*>(&m_pFixedPos[i]), reinterpret_cast<const D3DXMATRIX*>(&mBone));
        m_pPos[i] = vPos;
    }
}

// FUNCTION: SWBF2 0x0045b1b0
void EntityCloth::EnforceConstraint(ClothConstraint& constraint, float fRestitution)
{
    const unsigned int i0 = constraint.i0;
    const unsigned int i1 = constraint.i1;
    PblVector3& v0 = m_pPos[i0];
    PblVector3& v1 = m_pPos[i1];
    const PblVector3 vDelta = v1 - v0;
    const float fLength = PblMath::Sqrt(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    const float fDiff = (fLength - constraint.restLength) / fLength;

    const unsigned int uiNumFixed = mData->m_uiNumFixedPoints;
    if (i0 < uiNumFixed)
    {
        v1 -= vDelta * fDiff;
    }
    else if (i1 < uiNumFixed)
    {
        v0 += vDelta * fDiff;
    }
    else
    {
        const PblVector3 vHalf = vDelta * 0.5f;
        v0 += vHalf * fDiff;
        v1 -= vHalf * fDiff;
    }
}

// FUNCTION: SWBF2 0x0045ba40
void EntityCloth::EnforceCollisions(const PblMatrix& mMatrix, const RedPose* pPose)
{
    ClothCollision* pCollision = mData->m_pCollisionVolumes;
    for (unsigned int i = 0; i < mData->m_uiNumCollisionVolumes; ++i, ++pCollision)
    {
        const PblMatrix* pBone = 0;
        if (pPose)
        {
            pBone = pPose->Find(pCollision->parent);
        }

        PblMatrix mCollision;
        if (pBone)
        {
            const PblMatrix mTemp = pCollision->localMat * *pBone;
            const PblMatrix mInverse = mMatrix.GetInverseFast();
            mCollision = mTemp * mInverse;
        }
        else
        {
            PblMatrix mBone = gMatrixIdentity;
            RedModel::_HashTable.Find(mData->m_uiModelHash)->GetBoneMatrix(pCollision->parent, mBone);
            const PblMatrix mTemp = pCollision->localMat * mBone;
            const PblMatrix mInverse = mMatrix.GetInverseFast();
            mCollision = mTemp * mInverse;
        }

        const PblMatrix mLocal = mCollision;
        switch (pCollision->type)
        {
        case CLOTH_COLLISION_SPHERE:
            EnforceSphereCollision(mLocal, pCollision->width);
            break;
        case CLOTH_COLLISION_CYLINDER:
            EnforceCylinderCollision(mLocal, pCollision->height, pCollision->width);
            break;
        case CLOTH_COLLISION_BOX:
            EnforceBoxCollision(mLocal, pCollision->width, pCollision->height, pCollision->depth);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0045b7c0
void EntityCloth::EnforceCylinderCollision(const PblMatrix& mMatrix, float fHeight, float fRadius)
{
    const float fLength = fHeight * 2.0f;
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        PblVector3& vPos = m_pPos[i];
        const float fDX = vPos.x - mMatrix.m[3][0];
        const float fDY = vPos.y - mMatrix.m[3][1];
        const float fDZ = vPos.z - mMatrix.m[3][2];
        const float fDepth =
            fLength - fabsf(mMatrix.m[1][0] * fDX + mMatrix.m[1][1] * fDY + mMatrix.m[1][2] * fDZ);
        if (fDepth <= 0.0f)
        {
            continue;
        }
        const PblVector3 vRadial(mMatrix.m[0][0] * fDX + mMatrix.m[0][1] * fDY + mMatrix.m[0][2] * fDZ, 0.0f,
                                 mMatrix.m[2][0] * fDX + mMatrix.m[2][1] * fDY + mMatrix.m[2][2] * fDZ);
        const float fDist = PblMath::Sqrt(vRadial.x * vRadial.x + vRadial.y * vRadial.y + vRadial.z * vRadial.z);
        const float fPenetration = fRadius - fDist;
        if (fPenetration <= 0.0f)
        {
            continue;
        }
        if (fDepth > fPenetration)
        {
            const float fScale = fPenetration / fDist;
            vPos.x += (mMatrix.m[0][0] * vRadial.x + mMatrix.m[2][0] * vRadial.z) * fScale;
            vPos.y += (mMatrix.m[0][1] * vRadial.x + mMatrix.m[2][1] * vRadial.z) * fScale;
            vPos.z += (mMatrix.m[0][2] * vRadial.x + mMatrix.m[2][2] * vRadial.z) * fScale;
        }
        else
        {
            vPos.x += mMatrix.m[1][0] * fDepth;
            vPos.y += mMatrix.m[1][1] * fDepth;
            vPos.z += mMatrix.m[1][2] * fDepth;
        }
    }
}

// FUNCTION: SWBF2 0x00458df0
void EntityCloth::ClearSpecialAllocator()
{
    if (ClothBE::gMultiAllocator_)
    {
        delete ClothBE::gMultiAllocator_;
    }
    ClothBE::gMultiAllocator_ = 0;

    for (ClothBE::ClothDataT::Itor it = ClothBE::clothData_.Begin();
         it != ClothBE::clothData_.End(); ++it)
    {
        delete *it;
    }

    ClothBE::clothData_.Clear();
    ClothBE::enabled_ = true;
}

// FUNCTION: SWBF2 0x00458f00
ClothData::ClothData()
{
    m_uiNumParticles = 0;
    m_uiNumFixedPoints = 0;
    m_uiNumTriangles = 0;
    m_uiNumStretchPairs = 0;
    m_uiNumBendPairs = 0;
    m_uiNumCrossPairs = 0;
    m_uiNameHash = 0;
    m_uiParentHash = 0;
    m_pPos = 0;
    m_pTexCoords = 0;
    m_pTriangleIndices = 0;
    m_pStretchPairs = 0;
    m_pBendPairs = 0;
    m_pCrossPairs = 0;
    m_fBoundingRadius = 0.0f;
    m_pShader = 0;
}

// FUNCTION: SWBF2 0x00458f80
ClothData::~ClothData()
{
    PlatformCleanup();
    RedFreeToHeap(__RedCurrHeap, m_pPos, 0);
    RedFreeToHeap(__RedCurrHeap, m_pTexCoords, 0);
    RedFreeToHeap(__RedCurrHeap, m_pTriangleIndices, 0);
    RedFreeToHeap(__RedCurrHeap, m_pStretchPairs, 0);
    RedFreeToHeap(__RedCurrHeap, m_pBendPairs, 0);
    RedFreeToHeap(__RedCurrHeap, m_pCrossPairs, 0);
    if (m_pFixedPointsBones)
    {
        RedFreeToHeap(__RedCurrHeap, m_pFixedPointsBones, 0);
    }
}

// FUNCTION: SWBF2 0x00459060
void ClothData::Cleanup()
{
    s_clothDataHashTable.Clear();
}

// FUNCTION: SWBF2 0x00459080
void ClothData::PostReadInit()
{
    for (unsigned int i = 0; i < m_uiNumStretchPairs; ++i)
    {
        const PblVector3& v1 = m_pPos[m_pStretchPairs[i].i1];
        const PblVector3& v0 = m_pPos[m_pStretchPairs[i].i0];
        PblVector3 vDelta(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
        m_pStretchPairs[i].restLength = PblMath::Sqrt(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    }
    for (unsigned int i = 0; i < m_uiNumBendPairs; ++i)
    {
        const PblVector3& v1 = m_pPos[m_pBendPairs[i].i1];
        const PblVector3& v0 = m_pPos[m_pBendPairs[i].i0];
        PblVector3 vDelta(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
        m_pBendPairs[i].restLength = PblMath::Sqrt(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    }
    for (unsigned int i = 0; i < m_uiNumCrossPairs; ++i)
    {
        const PblVector3& v1 = m_pPos[m_pCrossPairs[i].i1];
        const PblVector3& v0 = m_pPos[m_pCrossPairs[i].i0];
        PblVector3 vDelta(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
        m_pCrossPairs[i].restLength = PblMath::Sqrt(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    }

    PblBox box;
    for (unsigned int i = 0; i < m_uiNumParticles; ++i)
    {
        box.AddPoint(m_pPos[i]);
    }
    PblVector3 vDiagonal(box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y, box._vMax.z - box._vMin.z);
    float fRadius = PblMath::Sqrt(vDiagonal.x * vDiagonal.x + vDiagonal.y * vDiagonal.y + vDiagonal.z * vDiagonal.z) * 0.5f;
    m_fBoundingRadius = fRadius + fRadius;
}

// TEMPLATE: SWBF2 0x0045c310
// util::HashTable_<ClothBank *,unsigned int,util::Vector<ClothBank *,util::BaseArray<ClothBank *,util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > > > >::Begin
template util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                          ClothBE::ClothDataT::KeyVecT, ClothBE::ClothDataT::HashVecT>::Itor
util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                 ClothBE::ClothDataT::KeyVecT, ClothBE::ClothDataT::HashVecT>::Begin();

// FUNCTION: SWBF2 0x004592e0
void ClothData::Read(PblFileChunk& chunk)
{
    ClothData* pData = new ClothData;
    pData->_Read(chunk);
    if (ClothData::s_clothDataHashTable.Find(pData->m_uiNameHash))
    {
        delete pData;
        return;
    }
    ClothData::s_clothDataHashTable.Store(pData->m_uiNameHash, pData);
    pData->PostReadInit();
}

// FUNCTION: SWBF2 0x00437500
PblRegion* __cdecl FindClothRegion(unsigned int uId)
{
    return PblRegion::Find(uId);
}

namespace ClothBE
{
// GLOBAL: SWBF2 0x01e30354
util::IAllocator* gMultiAllocator_;
}
