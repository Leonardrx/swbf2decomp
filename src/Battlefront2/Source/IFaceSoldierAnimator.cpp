#include "IFaceSoldierAnimator.h"
#include "PebbleFL/Common/PblVector.h"
#include "SoldierAnimatorClass.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnimBank.h"



// SYNTHETIC: SWBF2 0x004040d0
// `dynamic initializer for 'IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator''
// IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator

// GLOBAL: SWBF2 0x01ec601c
// IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator
PblHash IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator("IFaceSoldierAnimator");
// GLOBAL: SWBF2 0x00789c74
// IFaceSoldierAnimator::rttiNameIFaceSoldierAnimator
const char IFaceSoldierAnimator::rttiNameIFaceSoldierAnimator[] = "IFaceSoldierAnimator";







// FUNCTION: SWBF2 0x0056a180
void IFaceSoldierAnimator::SetUpperLowerAnimations(unsigned int uUpperAnimHash, unsigned int uLowerAnimHash,
                                                   float fTransitionTime)
{
    Close();
    mAnimHash = uUpperAnimHash;
    mLowerAnimHash = uLowerAnimHash;
    mQueuedAnim = 0;
    mQueuedLowerAnim = 0;
    mAnimTransitionTimeTotal = fTransitionTime;
    mAnimTransitionTime = 0.0f;
    Open();
}


// FUNCTION: SWBF2 0x0056a240
IFaceSoldierAnimator::IFaceSoldierAnimator(const Red3DModelAnimator* pAnimator)
    : mLowerAnimHash(0),
      mQueuedLowerAnim(0),
      mZephyrAnimationLower(0)
{
    for (int i = 0; i < 5; ++i)
    {
        mAnimBankHash[i] = pAnimator->mAnimBankHash[i];
    }
    mLooping = pAnimator->mLooping;
}

// FUNCTION: SWBF2 0x0056a2e0
void IFaceSoldierAnimator::Open()
{
    if (mAnimHash)
    {
        Red3DModelAnimator::Open();

        ZephyrSkeletonShared* pSkelShared = RedAnimation::Find(mAnimBankHash[0])->_pSkeletonShared;
        SoldierAnimatorClass::SetupJointTransformAnimationFlag(pSkelShared);
        SoldierAnimatorClass::SetupBodyMasks(pSkelShared, mUpperBodyAnimMask, mLowerBodyAnimMask);

        mZephyrPoseDynLower.Open(mZephyrSkeleton);
        mZephyrAnimationLower = const_cast<ZephyrAnim*>(FindAnimation(mLowerAnimHash));
        mZephyrPoseDynLower.SetAnimation(mZephyrAnimationLower, 30.0f);
        if (mAnimationTime >= 0.0f)
        {
            mZephyrPoseDynLower.SetCurTime(mAnimationTime / mAnimationTimeTotal);
        }
        mZephyrPoseDynLower.SetLoop(mLooping);
        mZephyrPoseDynLower.Update(0.0f);

        if (mAnimTransitionTimeTotal <= 0.0f)
        {
            mZephyrPoseStatic.Set(mZephyrPoseDyn, mUpperBodyAnimMask, true);
            mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
        }
        mPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    }
}

// FUNCTION: SWBF2 0x0056a3f0
void IFaceSoldierAnimator::Close()
{
    if (mAnimHash)
    {
        Red3DModelAnimator::Close();
        mZephyrPoseDynLower.Close();
    }
}

// FUNCTION: SWBF2 0x0056a410
void IFaceSoldierAnimator::Update(float fElapsed)
{
    mAnimationTime += fElapsed;
    if (mAnimationTime > mAnimationTimeTotal)
    {
        mAnimationTime -= mAnimationTimeTotal;
        if (mQueuedAnim)
        {
            unsigned int uUpperAnimHash = mQueuedAnim;
            unsigned int uLowerAnimHash = mQueuedLowerAnim;
            Close();
            mAnimHash = uUpperAnimHash;
            mLowerAnimHash = uLowerAnimHash;
            mQueuedAnim = 0;
            mQueuedLowerAnim = 0;
            mAnimTransitionTimeTotal = 0.3f;
            mAnimTransitionTime = 0.0f;
            Open();
            mQueuedLowerAnim = 0;
            mQueuedAnim = 0;
        }
    }
    mZephyrPoseDyn.SetCurTime(mAnimationTime / mAnimationTimeTotal);
    mZephyrPoseDyn.Update(0.0f);
    mZephyrPoseDynLower.SetCurTime(mAnimationTime / mAnimationTimeTotal);
    mZephyrPoseDynLower.Update(0.0f);
    if (mAnimHash && mAnimTransitionTimeTotal >= 0.0f)
    {
        mAnimTransitionTime += fElapsed;
    }
}

// FUNCTION: SWBF2 0x0056a590
RedPose* IFaceSoldierAnimator::UpdatePose()
{
    if (mAnimationTime < 0.0f)
    {
        int iNumFrames = mZephyrAnimation->m_u16NumFrames;
        mZephyrPoseDyn.SetFrameNum(RedRenderer::GetFrameNumber() % iNumFrames);
        mZephyrPoseDyn.Update(0.0f);
        iNumFrames = mZephyrAnimation->m_u16NumFrames;
        mZephyrPoseDynLower.SetFrameNum(RedRenderer::GetFrameNumber() % iNumFrames);
        mZephyrPoseDynLower.Update(0.0f);
    }
    if (mAnimTransitionTimeTotal > 0.0f)
    {
        if (mAnimTransitionTime >= mAnimTransitionTimeTotal)
        {
            mAnimTransitionTimeTotal = 0.0f;
            mZephyrPoseStatic.Set(mZephyrPoseDyn, mUpperBodyAnimMask, true);
            mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
        }
        else
        {
            mZephyrPoseStatic.Blend(mZephyrPoseDyn, mUpperBodyAnimMask,
                                    mAnimTransitionTime / mAnimTransitionTimeTotal);
            mZephyrPoseStatic.Blend(mZephyrPoseDynLower, mLowerBodyAnimMask,
                                    mAnimTransitionTime / mAnimTransitionTimeTotal);
        }
    }
    else
    {
        mZephyrPoseStatic.Set(mZephyrPoseDyn, mUpperBodyAnimMask, true);
        mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
    }
    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    return &mPose;
}
