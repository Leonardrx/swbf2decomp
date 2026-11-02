#include "RedModel.h"

#include <d3dx9math.h>

#include "RedCamera.h"
#include "RedColor.h"
#include "RedDirectionalLight.h"
#include "RedLight.h"
#include "RedPose.h"
#include "RedSegment.h"
#include "RedShadingState.h"
#include "RedSkeleton.h"
#include "PC/pcRedRenderer.h"
#include "PC/pcRedStencilShadow.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "PebbleFL/Common/PblMath.h"

namespace FLRenderer
{
float GetFarSceneRange();
}

void __cdecl PblTraceP(const char* Format, ...);
extern bool sLoadNestedLevels;

// SYNTHETIC: SWBF2 0x00407cc0
// `dynamic initializer for 'RedModel::_HashTable''

// GLOBAL: SWBF2 0x0093ebd8
PblHashTable<RedModel, 1024> RedModel::_HashTable;

// GLOBAL: SWBF2 0x007deee0
bool RedModel::pc_shareBuffers = true;

// GLOBAL: SWBF2 0x007deee8
static float sfShadowDistance = 30.0f;

// FUNCTION: SWBF2 0x006c3d60
RedModel::RedModel()
{
    _bHasTransform = false;
    _uiDrawListBytes = 0;
    _apSegments = 0;
    _iNumSegments = 0;
    _uiNumStripVtx = 0;
    _bHardSkin = false;
    _bNoProjTexture = false;
    _pSkeleton = 0;
    _iNumBones = 0;
    _pStencilShadow = 0;
    _iRefCount = 0;
}

// FUNCTION: SWBF2 0x006c3dd0
RedModel::~RedModel()
{
    if (_pSkeleton)
    {
        delete _pSkeleton;
    }
    if (_pStencilShadow)
    {
        delete _pStencilShadow;
    }
    for (int i = 0; i < _iNumSegments; ++i)
    {
        if (_apSegments[i])
        {
            delete _apSegments[i];
        }
    }
    if (_apSegments)
    {
        delete[] _apSegments;
    }
}

// SYNTHETIC: SWBF2 0x006c3eb0
// RedModel::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006c3e40
RedSkeleton::~RedSkeleton()
{
    if (m_uiNameHash)
    {
        _HashTable.Remove(m_uiNameHash);
    }
    if (m_amDefaultPose)
    {
        delete[] ((char*)m_amDefaultPose - ((unsigned char*)m_amDefaultPose)[-1]);
    }
    if (m_amDefaultPoseInv)
    {
        delete[] ((char*)m_amDefaultPoseInv - ((unsigned char*)m_amDefaultPoseInv)[-1]);
    }
    delete[] m_aConnections;
    delete[] m_auiNameHash;
}

// FUNCTION: SWBF2 0x006c3ee0
RedShadingState* RedModel::CreateDefaultShadingState(unsigned int uiRenderTypeHash) const
{
    for (int i = 0; i < _iNumSegments; ++i)
    {
        if (_apSegments[i]->GetRenderTypeHash() == uiRenderTypeHash)
        {
            return (RedShadingState*)_apSegments[i]->CreateShadingState();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c3f30
RedShadingState* RedModel::CreateShadingState(unsigned int uiMaterialNameHash) const
{
    for (int i = 0; i < _iNumSegments; ++i)
    {
        if (_apSegments[i]->m_uiMaterialNameHash == uiMaterialNameHash)
        {
            return (RedShadingState*)_apSegments[i]->CreateShadingState();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c3f80
unsigned int RedModel::GetBoneIndex(unsigned int uiName) const
{
    const RedSkeleton* pSkeleton = _pSkeleton;
    if (pSkeleton)
    {
        for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
        {
            if (uiName == pSkeleton->m_auiNameHash[i])
            {
                return i;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c3fc0
unsigned int RedModel::GetBoneIndex(const char* pcName) const
{
    const unsigned int uiName = PblTEMPHash(pcName);
    const RedSkeleton* pSkeleton = _pSkeleton;
    if (pSkeleton)
    {
        for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
        {
            if (uiName == pSkeleton->m_auiNameHash[i])
            {
                return i;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c4010
bool RedModel::GetBoneMatrix(unsigned int uiBone, PblMatrix& mat) const
{
    const RedSkeleton* pSkeleton = _pSkeleton;
    if (pSkeleton)
    {
        for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
        {
            if (pSkeleton->m_auiNameHash[i] == uiBone)
            {
                mat = pSkeleton->m_amDefaultPose[i];
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006c4080
bool RedModel::GetHierarchicalBoneMatrix(unsigned int uiBone, PblMatrix& mat) const
{
    const RedSkeleton* pSkeleton = _pSkeleton;
    if (pSkeleton)
    {
        for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
        {
            if (pSkeleton->m_auiNameHash[i] == uiBone)
            {
                int iParent = pSkeleton->GetParentBone(i);
                const PblMatrix& bone = pSkeleton->m_amDefaultPose[i];
                if (iParent >= 0)
                {
                    const PblMatrix& parent = pSkeleton->m_amDefaultPose[iParent];
                    PblMatrix result = bone * parent.GetInverseFast();
                    mat = result;
                }
                else
                {
                    mat = bone;
                }
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006c4190
unsigned int RedModel::GetParentBone(unsigned int uiBone) const
{
    const RedSkeleton* pSkeleton = _pSkeleton;
    if (pSkeleton)
    {
        for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
        {
            if (pSkeleton->m_auiNameHash[i] == uiBone)
            {
                int iParent = pSkeleton->GetParentBone(i);
                return iParent >= 0 ? pSkeleton->m_auiNameHash[iParent] : 0;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c41e0
unsigned int RedModel::GetParentBoneAndOffset(unsigned int uiBone, PblVector3& vOffset) const
{
    const RedSkeleton* pSkeleton = _pSkeleton;
    if (pSkeleton)
    {
        for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
        {
            if (pSkeleton->m_auiNameHash[i] == uiBone)
            {
                int iParent = pSkeleton->GetParentBone(i);
                if (iParent < 0)
                {
                    return 0;
                }
                const PblMatrix& bone = pSkeleton->m_amDefaultPose[i];
                const PblMatrix& parent = pSkeleton->m_amDefaultPose[iParent];
                PblVector3 v = parent.GetInverseFast().ApplyTransform(bone.GetTranslation());
                vOffset = v;
                return _pSkeleton->m_auiNameHash[iParent];
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c42b0
unsigned int RedModel::GetParentBoneAndOffsetMatrix(unsigned int uiBone, PblMatrix& mat) const
{
    const RedSkeleton* pSkeleton = _pSkeleton;
    if (pSkeleton)
    {
        for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
        {
            if (pSkeleton->m_auiNameHash[i] == uiBone)
            {
                int iParent = pSkeleton->GetParentBone(i);
                if (iParent < 0)
                {
                    return 0;
                }
                const PblMatrix& bone = pSkeleton->m_amDefaultPose[i];
                const PblMatrix& parent = pSkeleton->m_amDefaultPose[iParent];
                PblMatrix result = bone * parent.GetInverseFast();
                mat = result;
                return _pSkeleton->m_auiNameHash[iParent];
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c43a0
bool RedModel::HasTransparentSegment() const
{
    for (int i = 0; i < _iNumSegments; ++i)
    {
        if (_apSegments[i]->IsTransparent())
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006c4490
void RedModel::Render(const PblMatrix* pMatrix, const RedPose* pPose, const RedColor& color,
                      unsigned int uiFlags, RedShadingPose* pShadingPose) const
{
    const PblMatrix* apBoneMatrices[128];
    bool abCulled[512];
    int iNumVisible = 0;

    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fFarSceneRange = FLRenderer::GetFarSceneRange();

    for (int i = 0; i < _iNumSegments; ++i)
    {
        abCulled[i] = false;
        const RedSegment* pSegment = _apSegments[i];
        if (!pPose && pSegment->m_uiNumBones <= 0 && !(uiFlags & 0x4000000))
        {
            const PblBox& box = pSegment->mBox;
            PblVector3 vHalf;
            vHalf.x = (box._vMax.x - box._vMin.x) * 0.5f;
            vHalf.y = (box._vMax.y - box._vMin.y) * 0.5f;
            vHalf.z = (box._vMax.z - box._vMin.z) * 0.5f;
            float fRadius = (float)sqrt((double)(vHalf.x * vHalf.x + vHalf.y * vHalf.y + vHalf.z * vHalf.z));
            PblVector3 vCenter;
            vCenter.x = box._vMin.x + vHalf.x;
            vCenter.y = box._vMin.y + vHalf.y;
            vCenter.z = box._vMin.z + vHalf.z;
            vCenter = pMatrix->ApplyTransform(vCenter);
            float fFar = -1.0f;
            if (fFarSceneRange != 0.0f && (uiFlags & 0x20000))
            {
                fFar = fFarSceneRange;
            }
            if (pCamera->ClassifySphere(vCenter, fRadius, fFar) == -1)
            {
                abCulled[i] = true;
                continue;
            }
        }
        ++iNumVisible;
    }
    if (!iNumVisible)
    {
        return;
    }

    if (!RedRenderer::GetLightingEnable())
    {
        uiFlags |= 0x10;
    }
    if (!(uiFlags & 0x80000010))
    {
        if (uiFlags & 0x4000000)
        {
            RedRenderer::SetLightingState(RedInterfaceScreen::s_lightingState);
            uiFlags |= 0x200;
        }
        else
        {
            float fSphereRadius = _BoundingSphere.w;
            PblSphere sphere;
            sphere.SetCenter(pMatrix->ApplyTransform(_BoundingSphere.GetCenter()));
            sphere.SetRadius(fSphereRadius);
            RedRenderer::SetLightingState(*RedLight::GetLightingState(
                sphere, 4, (uiFlags & 0x80) != 0, (_uiFlags & 1) != 0, _bNoProjTexture));
        }
    }

    RedColor colorCopy = color;

    const PblMatrix& camMatrix = RedRenderer::GetCamera()->GetMatrix();
    const PblVector3 vDelta(pMatrix->m[3][0] - camMatrix.m[3][0], pMatrix->m[3][1] - camMatrix.m[3][1],
                            pMatrix->m[3][2] - camMatrix.m[3][2]);
    float fDistance = -(camMatrix.m[2][1] * vDelta.y + camMatrix.m[2][0] * vDelta.x +
                        camMatrix.m[2][2] * vDelta.z);
    fDistance = Max(fDistance, 0.0f);
    fDistance -= RedRenderer::GetCamera()->GetNearPlane();
    float fRadius = _BoundingSphere.w;

    const PblMatrix* pWorld;
    if (uiFlags & 0x1000000)
    {
        pWorld = pMatrix;
    }
    else
    {
        pWorld = RedRenderer::AllocMatrixInCache(*pMatrix);
    }

    if (pPose)
    {
        pPose->BindToSkeleton(_pSkeleton);
        memset(apBoneMatrices, 0, _pSkeleton->m_uiNumBones * sizeof(apBoneMatrices[0]));
    }

    if (color.a > 0)
    {
        for (int i = 0; i < _iNumSegments; ++i)
        {
            if (abCulled[i])
            {
                continue;
            }

            RedShadingState* pState = 0;
            if (pShadingPose)
            {
                if (_apSegments[i]->m_uiMaterialNameHash)
                {
                    pState = pShadingPose->Find(_apSegments[i]->m_uiMaterialNameHash);
                }
                if (!pState)
                {
                    pState = pShadingPose->FindDefault(_apSegments[i]->GetRenderTypeHash());
                }
            }

            RedSegment* pSegment = _apSegments[i];
            unsigned int uiNumBones = pSegment->m_uiNumBones;
            if (uiNumBones)
            {
                PblMatrix* pBoneMatrices = RedRenderer::AllocBoneMatricesInCache(uiNumBones);
                if (pPose)
                {
                    if (_bHardSkin)
                    {
                        pPose->GetHardSkinMatrices(pBoneMatrices, _apSegments[i]->m_puiBoneIndexMap,
                                                   uiNumBones);
                    }
                    else
                    {
                        pPose->GetSkinMatrices(pBoneMatrices, _apSegments[i]->m_puiBoneIndexMap,
                                               uiNumBones);
                    }
                }
                else
                {
                    _pSkeleton->GetSkinMatrices(pBoneMatrices, _apSegments[i]->m_puiBoneIndexMap,
                                                uiNumBones);
                }
                const RedColor* pColor =
                    (_apSegments[i]->IsTransparent() || color.a < 0xff) ? &colorCopy : &color;
                _apSegments[i]->Render(pWorld, pBoneMatrices, uiNumBones, uiFlags, *pColor, pState);
            }
            else if (pPose)
            {
                unsigned int uiBone = pSegment->m_uiBoneIndex;
                if (!apBoneMatrices[uiBone])
                {
                    PblMatrix mBone = pPose->_GetBoneMatrix(uiBone) * *pMatrix;
                    apBoneMatrices[uiBone] = RedRenderer::AllocMatrixInCache(mBone);
                }
                const RedColor* pColor =
                    (_apSegments[i]->IsTransparent() || color.a < 0xff) ? &colorCopy : &color;
                _apSegments[i]->Render(apBoneMatrices[uiBone], 0, 0, uiFlags & ~0x1000000, *pColor,
                                       pState);
            }
            else
            {
                const RedColor* pColor =
                    (_apSegments[i]->IsTransparent() || color.a < 0xff) ? &colorCopy : &color;
                _apSegments[i]->Render(pWorld, 0, 0, uiFlags, *pColor, pState);
            }
        }
    }

    if (_pStencilShadow && (uiFlags & 0x10000) && !(uiFlags & 0x220000))
    {
        float fShadowRadius = fRadius + 0.5f;
        bool bZFail;
        if (fShadowRadius > fDistance)
        {
            bZFail = true;
        }
        else
        {
            const PblVector3& vLightDir = RedLight::m_sGlobalDirectionals[0]->m_vDirection;
            float fDot = -(camMatrix.m[2][1] * vLightDir.y + camMatrix.m[2][0] * vLightDir.x +
                           camMatrix.m[2][2] * vLightDir.z);
            bZFail = fShadowRadius - RedRenderer::GetStencilShadowLength() * fDot > fDistance;
        }
        RedRenderer::SetStencilShadowZFail(bZFail);
        _pStencilShadow->Render(pWorld, pPose ? apBoneMatrices : 0, pPose,
                                fRadius + sfShadowDistance > fDistance);
    }
}

// FUNCTION: SWBF2 0x006c43e0
void RedModel::Init()
{
}

// FUNCTION: SWBF2 0x006c43f0
RedModel* RedModel::Read(PblFileChunk& chunk)
{
    RedModel* pModel = new RedModel;
    pModel->_Read(chunk);
    if (!_HashTable.Find(pModel->_uiNameHash))
    {
        _HashTable.Store(pModel->_uiNameHash, pModel);
        return pModel;
    }
    if (sLoadNestedLevels)
    {
        PblTraceP("FIXME: Duplicate RedModel name exists in another level file: %08x\n",
                  pModel->_uiNameHash);
    }
    return _HashTable.Find(pModel->_uiNameHash);
}

// FUNCTION: SWBF2 0x006c4b90
void RedModel::SetupPose(RedPose& pose) const
{
    if (_pSkeleton)
    {
        _pSkeleton->SetupPose(pose);
    }
}

// FUNCTION: SWBF2 0x006c4bb0
void RedModel::_Read(PblFileChunk& chunk)
{
    char acNodeName[256];
    char acName[256];
    int iSegment = 0;

    _uiFlags = 0;
    _iNumSegments = 0;
    _pSkeleton = RedSkeleton::s_pCurrentSkeleton;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4D524658:
        {
            PblVector3 vRight, vUp, vFront, vOrigin;
            vRight.x = child._pFile->ReadFloat();
            vRight.y = child._pFile->ReadFloat();
            vRight.z = child._pFile->ReadFloat();
            vUp.x = child._pFile->ReadFloat();
            vUp.y = child._pFile->ReadFloat();
            vUp.z = child._pFile->ReadFloat();
            vFront.x = child._pFile->ReadFloat();
            vFront.y = child._pFile->ReadFloat();
            vFront.z = child._pFile->ReadFloat();
            vOrigin.x = child._pFile->ReadFloat();
            vOrigin.y = child._pFile->ReadFloat();
            vOrigin.z = child._pFile->ReadFloat();

            _Matrix = PblMatrix(vRight, vUp, vFront, vOrigin);
            _bHasTransform = true;
            break;
        }
        case 0x454D414E:
            child.ReadString(acName, 256);
            _uiNameHash = PblHash(acName);
            if (_HashTable.Find(_uiNameHash))
            {
                return;
            }
            break;
        case 0x45444F4E:
            child.ReadString(acNodeName, 256);
            if (acNodeName[0] == 'h' && acNodeName[1] == 'p' && acNodeName[2] == '_')
            {
                char* pc = acNodeName;
                while (pc[1])
                {
                    ++pc;
                }
                while (*pc == '_' || (*pc >= '0' && *pc <= '9'))
                {
                    *pc-- = 0;
                }
            }
            PblTEMPHash(acNodeName);
            break;
        case 0x4F464E49:
        {
            int iSkin = child._pFile->Read32();
            if (iSkin == 2)
            {
                _bHardSkin = RedRenderer::pcGetRenderCaps().skinning == 0;
            }
            else
            {
                _bHardSkin = iSkin == 1;
            }
            _bNoProjTexture = child._pFile->Read32() != 0;
            _iNumSegments = child._pFile->Read32();
            _apSegments = _iNumSegments ? new RedSegment*[(unsigned int)_iNumSegments] : 0;
            if (_iNumSegments > 2)
            {
                PblTraceP("Model with %d segments [%s]\n", _iNumSegments, acName);
            }
            child._pFile->Read32();
            float fMinX = child._pFile->ReadFloat();
            float fMinY = child._pFile->ReadFloat();
            float fMinZ = child._pFile->ReadFloat();
            float fMaxX = child._pFile->ReadFloat();
            float fMaxY = child._pFile->ReadFloat();
            float fMaxZ = child._pFile->ReadFloat();
            float fTotalMinX = child._pFile->ReadFloat();
            float fTotalMinY = child._pFile->ReadFloat();
            float fTotalMinZ = child._pFile->ReadFloat();
            float fTotalMaxX = child._pFile->ReadFloat();
            float fTotalMaxY = child._pFile->ReadFloat();
            float fTotalMaxZ = child._pFile->ReadFloat();
            _iNumBones = child._pFile->Read32();
            _uiNumStripVtx = child._pFile->Read32();
            _BoundingBox = PblBox(fMinX, fMinY, fMinZ, fMaxX, fMaxY, fMaxZ);
            _TotalBoundingBox = PblBox(fTotalMinX, fTotalMinY, fTotalMinZ,
                                       fTotalMaxX, fTotalMaxY, fTotalMaxZ);
            float dx = _TotalBoundingBox._vMax.x - _TotalBoundingBox._vMin.x;
            float dy = _TotalBoundingBox._vMax.y - _TotalBoundingBox._vMin.y;
            float dz = _TotalBoundingBox._vMax.z - _TotalBoundingBox._vMin.z;
            PblSphere sphere;
            sphere.x = (_TotalBoundingBox._vMax.x + _TotalBoundingBox._vMin.x) * 0.5f;
            sphere.y = (_TotalBoundingBox._vMax.y + _TotalBoundingBox._vMin.y) * 0.5f;
            sphere.z = (_TotalBoundingBox._vMax.z + _TotalBoundingBox._vMin.z) * 0.5f;
            sphere.w = (float)sqrt((double)(dx * dx + dy * dy + dz * dz)) * 0.5f;
            _BoundingSphere = sphere;
            break;
        }
        case 0x52485053:
        {
            float x = child._pFile->ReadFloat();
            float y = child._pFile->ReadFloat();
            float z = child._pFile->ReadFloat();
            float r = child._pFile->ReadFloat();
            _BoundingSphere.x = x;
            _BoundingSphere.y = y;
            _BoundingSphere.z = z;
            _BoundingSphere.w = r;
            break;
        }
        case 0x6D676573:
            _apSegments[iSegment] = RedSegment::Read(child, this);
            if (_apSegments[iSegment]->IsVertexLit())
            {
                _uiFlags |= 1;
            }
            else if (_apSegments[iSegment]->IsLit())
            {
                _uiFlags |= 2;
            }
            ++iSegment;
            break;
        case 0x77646873:
            if (!_pStencilShadow)
            {
                _pStencilShadow = new RedStencilShadow;
                _pStencilShadow->m_pModel = this;
            }
            _pStencilShadow->Read(child);
            break;
        }
    }

    int iNumTransparent = 0;
    int i = 0;
    while (i < _iNumSegments - iNumTransparent)
    {
        if (_apSegments[i]->IsTransparent())
        {
            int iLast = _iNumSegments - iNumTransparent++ - 1;
            RedSegment* pSegment = _apSegments[i];
            _apSegments[i] = _apSegments[iLast];
            _apSegments[iLast] = pSegment;
        }
        else
        {
            ++i;
        }
    }
}
