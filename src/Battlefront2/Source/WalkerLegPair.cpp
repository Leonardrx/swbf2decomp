#include <math.h>
#include "PebbleFL/Common/PblVector.h"

#include "WalkerLegPair.h"
#include "WalkerAnimation.h"
#include "EntityWalker.h"
#include "CollisionManager.h"
#include "CollisionModel.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// SYNTHETIC: SWBF2 0x00406fe0
// `dynamic initializer for 'WalkerLegPair::sMemoryPool''
// SYNTHETIC: SWBF2 0x007697c0
// `dynamic atexit destructor for 'WalkerLegPair::sMemoryPool''

// GLOBAL: SWBF2 0x01f9a7e0
// WalkerLegPair::sMemoryPool
MemoryPool WalkerLegPair::sMemoryPool("WalkerLegPair", sizeof(WalkerLegPair));



// FUNCTION: SWBF2 0x00676770
float WalkerLegPair::GetTurnState()
{
    return mBlend->GetTurnState();
}

// FUNCTION: SWBF2 0x00676780
void WalkerLegPair::SetFrameState(float fState)
{
    mBlend->SetFrameState(fState);
}

// FUNCTION: SWBF2 0x00676790
void WalkerLegPair::ResetTurnState()
{
    mBlend->ResetTurnState();
}

// FUNCTION: SWBF2 0x006767a0
float WalkerLegPair::GetStateLimit()
{
    return mBlend->GetStateLimit();
}

// FUNCTION: SWBF2 0x00676440
WalkerLegPair::WalkerLegPair(EntityWalker* pParent, const WalkerLegPairInfo& info)
    : mParent(pParent)
{
    mBlend = new WalkerBlend(3, mParent->mZephyrSkeleton);

    mFootContact[Left] = false;
    mFootContact[Right] = false;
    mFootIntersection[Left] = 0.0f;
    mFootIntersection[Right] = 0.0f;
    mIdleBlendFoot = FootCount;
    mLastIdleBlend = 0.0f;

    mFootIndices[Left] = info.mFootIndices[Left];
    mFootIndices[Right] = info.mFootIndices[Right];
    mFootCollisionBody[Left] = info.mFootCollisionBody[Left];
    mFootCollisionBody[Right] = info.mFootCollisionBody[Right];

    mMainMask.ClearAll();
    mMainMask.Or(info.mLegMasks[Left]);
    mMainMask.Or(info.mLegMasks[Right]);
    mBlend->SetJointMask(0, mMainMask);

    mTopMask.ClearAll();
    mTopMask.Or(info.mLegTopMasks[Left]);
    mTopMask.Or(info.mLegTopMasks[Right]);
    mBlend->SetJointMask(1, mTopMask);
    mBlend->SetJointMask(2, mTopMask);

    mLegJoints[Left] = info.mLegJoints[Left];
    mLegAdjustments[Left] = PblQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
    mLegJoints[Right] = info.mLegJoints[Right];
    mLegAdjustments[Right] = PblQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
    mBlendFactor = 0.0f;
}

// FUNCTION: SWBF2 0x00676bd0
void WalkerLegPair::BuildBlendLookupTable()
{
    float fBlend = 0.0f;
    float* pEntry = mBlendLookupTable;
    mBlend->mUnits[0]->mWeight = 1.0f;
    mBlend->mUnits[2]->mWeight = 0.0f;
    mBlend->mUnits[0]->mMask.SetAll();
    mBlend->mUnits[1]->mMask.SetAll();
    mBlend->mUnits[2]->mMask.SetAll();

    const CollisionModel* pModel = mParent->GetCollisionModel();
    do
    {
        mBlend->mUnits[1]->mWeight = fBlend;
        mBlend->Blend(mParent->mZephyrPoseStatic);
        mParent->mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
        *pEntry++ = pModel->GetWorldMatrix(mFootCollisionBody[Left], gMatrixIdentity,
                                           mParent->GetPose()).m[3][1];
        fBlend += 0.125f;
    } while (fBlend <= 1.0f);

    const float fBase = mBlendLookupTable[0];
    for (int i = 0; i < 9; ++i)
    {
        mBlendLookupTable[i] -= fBase;
    }

    mBlend->mUnits[1]->mWeight = 0.0f;
    mBlend->Blend(mParent->mZephyrPoseStatic);
    mParent->mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    mBlend->mUnits[0]->mMask.SetAll();
    mBlend->mUnits[1]->mMask.SetAll();
    mBlend->mUnits[2]->mMask.SetAll();
}

// FUNCTION: SWBF2 0x00676630
WalkerLegPair::~WalkerLegPair()
{
    delete mBlend;
}

// FUNCTION: SWBF2 0x00676640
void WalkerLegPair::InitDone()
{
    BuildBlendLookupTable();
    mLegTransform[Left] = mParent->mZephyrSkeleton.m_pWorldMatrices[mLegJoints[Left]];
    mLegTransform[Right] = mParent->mZephyrSkeleton.m_pWorldMatrices[mLegJoints[Right]];

    const CollisionModel* pModel = mParent->GetCollisionModel();
    float fRight = pModel->GetLocalMatrix(mFootCollisionBody[Right], mParent->GetPose()).m[3][2];
    float fLeft = pModel->GetLocalMatrix(mFootCollisionBody[Left], mParent->GetPose()).m[3][2];
    mLocalZOffset = (fLeft + fRight) * 0.5f;
}

// FUNCTION: SWBF2 0x00676900
bool WalkerLegPair::FootTerrainCollision(const CollisionResult* pResult)
{
    const int iCollider = pResult->mColliderId;
    int i;
    for (i = 0; i < FootCount; ++i)
    {
        if (mFootCollisionBody[i] == iCollider)
        {
            break;
        }
    }
    if (i == FootCount)
    {
        return false;
    }
    mFootContact[i] = true;
    return true;
}

// FUNCTION: SWBF2 0x00676940
void WalkerLegPair::SetTerrainHeight(int iIndex, float fHeight)
{
    for (int i = 0; i < FootCount; ++i)
    {
        if (mFootCollisionBody[i] == iIndex)
        {
            mFootTerrainHeight[i] = fHeight;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x00676db0
void WalkerLegPair::SetupGroundCollision(const PblMatrix& mMatrix, float fPitch, const PblVector3* const pFeet)
{
    float fLegLength = mParent->mClass->mLegRayHitLength;
    PblVector3 vOrigin = mMatrix.GetTranslation();
    vOrigin.y -= sinf(fPitch * 2.0f) * mLocalZOffset;
    float fRayLength = fLegLength * 2.0f;

    for (int i = 0; i < FootCount; ++i)
    {
        PblVector3 vStart = vOrigin;
        vStart.x = vStart.x + mMatrix.m[0][0] * pFeet[i].x + mMatrix.m[2][0] * pFeet[i].z;
        vStart.y = vStart.y + mMatrix.m[0][1] * pFeet[i].x + mMatrix.m[2][1] * pFeet[i].z + fLegLength;
        vStart.z = vStart.z + mMatrix.m[0][2] * pFeet[i].x + mMatrix.m[2][2] * pFeet[i].z;
        CollisionObject* pHitObject;
        float fHit = CollisionManager::RayHit(vStart, PblVector3(0.0f, -1.0f, 0.0f), fRayLength, pHitObject,
                                              NULL, NULL, 0, 0x90, false);
        mFootHeight[i] = (1.0f - fHit * 2.0f) * fLegLength;
    }

    mFootHeightDifference = mFootHeight[Left] - mFootHeight[Right];
    float fBlend = Clamp(mFootHeightDifference * -0.6f, -1.0f, 1.0f);
    mBlendFactor += (fBlend - mBlendFactor) * 0.4f;

    if (mParent->mClass->mOrientRoll)
    {
        mPushValue = (mFootHeight[Right] + mFootHeight[Left]) * 0.5f;
    }
    else
    {
        float fLow = mFootHeight[Right] > mFootHeight[Left] ? mFootHeight[Left] : mFootHeight[Right];
        mPushValue = fLow;
        float fHigh = mFootHeight[Left] > mFootHeight[Right] ? mFootHeight[Left] : mFootHeight[Right];
        float fHalf = fLegLength * 0.5f;
        if (fHigh > fHalf)
        {
            float fT = (fHigh - fHalf) / (fLegLength * 0.4f);
            if (fT > 1.0f)
            {
                fT = 1.0f;
            }
            mPushValue = (fHigh - fLow) * fT + fLow;
        }
    }
}

// FUNCTION: SWBF2 0x00676760
void WalkerLegPair::Update(float fDt)
{
    mBlend->Update(fDt);
}

// FUNCTION: SWBF2 0x006767b0
void WalkerLegPair::ApplyBlend()
{
    AdjustBlending();
    mBlend->Blend(mParent->mZephyrPoseStatic);
}

// FUNCTION: SWBF2 0x006767d0
void WalkerLegPair::CorrectJoints()
{
    PblQuaternion kAdjust(mParent->mPitchOffset.mCos, -mParent->mPitchOffset.mSin, 0.0f, 0.0f);
    AdjustLeg(Left, kAdjust);
    AdjustLeg(Right, kAdjust);
}

// FUNCTION: SWBF2 0x00676830
void WalkerLegPair::RestoreJoints()
{
    for (int i = 0; i < FootCount; ++i)
    {
        PblQuaternion& kQuat = mParent->mZephyrPoseStatic.m_pJoints[(unsigned short)mLegJoints[i]].kQuat;
        kQuat = kQuat * mLegAdjustments[i].Inverse();
    }
}

// FUNCTION: SWBF2 0x006768f0
void WalkerLegPair::UpdateDone()
{
    mFootContact[Left] = false;
    mFootContact[Right] = false;
}

// FUNCTION: SWBF2 0x00676970
void WalkerLegPair::StateChanged(int iState)
{
    mBlend->SetJointMask(0, mMainMask);
    mBlend->SetJointMask(1, mTopMask);
    mBlend->SetJointMask(2, mTopMask);
}

// FUNCTION: SWBF2 0x006769c0
void WalkerLegPair::AdjustLeg(eFoot eWhich, const PblQuaternion& kAdjust)
{
    int iJoint = mLegJoints[eWhich];
    if (iJoint < 0)
    {
        return;
    }

    PblQuaternion& kJoint = mParent->mZephyrPoseStatic.m_pJoints[(unsigned short)iJoint].kQuat;
    PblQuaternion kRot = kJoint;
    for (char iParent = mParent->mZephyrSkeleton.m_pShared->m_pJoints[iJoint].m_iParent; iParent;
         iParent = mParent->mZephyrSkeleton.m_pShared->m_pJoints[iParent].m_iParent)
    {
        const PblQuaternion& kParentRot = mParent->mZephyrPoseStatic.m_pJoints[(unsigned short)iParent].kQuat;
        kRot = kParentRot * kRot;
    }

    mLegAdjustments[eWhich] = kRot.Inverse() * kAdjust * kRot;
    kJoint = kJoint * mLegAdjustments[eWhich];
}

// FUNCTION: SWBF2 0x00676b10
void WalkerLegPair::AdjustBlending()
{
    if (mBlend->mCount > 1)
    {
        if (mBlendFactor < 0.0f)
        {
            mBlend->mUnits[0]->mWeight = 1.0f;
            mBlend->mUnits[1]->mWeight = -mBlendFactor;
            mBlend->mUnits[2]->mWeight = 0.0f;
            return;
        }
        if (mBlendFactor > 0.0f)
        {
            mBlend->mUnits[0]->mWeight = 1.0f;
            mBlend->mUnits[1]->mWeight = 0.0f;
            mBlend->mUnits[2]->mWeight = mBlendFactor;
            return;
        }
    }
    mBlend->mUnits[0]->mWeight = 1.0f;
    mBlend->mUnits[1]->mWeight = 0.0f;
    mBlend->mUnits[2]->mWeight = 0.0f;
}
