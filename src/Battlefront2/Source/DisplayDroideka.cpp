#include "DisplayDroideka.h"
#include "PebbleFL/Common/PblVector.h"

#include <d3dx9math.h>

#include "EntityDroideka.h"
#include "HUDElementBarSegmented.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

#include "RedEngineFL/Graphics/RedShadingState.h"



// FUNCTION: SWBF2 0x0048cb30
void DroidekaElement::SetScale(float fScale)
{
    mScale = fScale * 0.9f;
}

// FUNCTION: SWBF2 0x0048cb50
PblVector3 DroidekaElement::GetPos()
{
    return PblVector3(0.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x0048cb70
void DroidekaElement::BuildWorldMatrices()
{
    ZephyrPoseStatic<32> kPose;
    mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
    kPose.Open(mZephyrSkeleton);
    kPose.Set(mZephyrPoseDyn, false);

    if (mState == EntityDroideka::Rolling)
    {
        ZephyrTransform& kSpine = kPose.m_pJoints[(unsigned short)mClass->mSpineIdx];
        PblQuaternion qRoll(mBallOrientation.GetCos(), mBallOrientation.GetSin(), 0.0f, 0.0f);
        kSpine.kQuat *= qRoll;

        PblVector3 vCentre = PblVector3(0.0f, mClass->mBallRadius, 0.0f) - kSpine.kTrans;
        mTranslationOffset = vCentre - kSpine.kQuat.RotateVector(vCentre);
        kSpine.kTrans.x = mTranslationOffset.x + kSpine.kTrans.x;
        kSpine.kTrans.y = mTranslationOffset.y + kSpine.kTrans.y;
        kSpine.kTrans.z = mTranslationOffset.z + kSpine.kTrans.z;
    }
    else
    {
        ZephyrTransform& kRoot = kPose.m_pJoints[0];
        kRoot.kQuat *= PblQuaternion(mTurnOffset.GetCos(), 0.0f, mTurnOffset.GetSin(), 0.0f);

        PblQuaternion qPitch(mPitchOffset.GetCos(), mPitchOffset.GetSin(), 0.0f, 0.0f);
        kRoot.kQuat *= qPitch;

        mTranslationOffset = mPivotOffset - qPitch.RotateVector(mPivotOffset);
        kRoot.kTrans.x = mTranslationOffset.x + kRoot.kTrans.x;
        kRoot.kTrans.y = mTranslationOffset.y + kRoot.kTrans.y;
        kRoot.kTrans.z = mTranslationOffset.z + kRoot.kTrans.z;
    }

    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);

    if (mState == EntityDroideka::Rolling)
    {
        ZephyrTransform& kSpine = kPose.m_pJoints[(unsigned short)mClass->mSpineIdx];
        kSpine.kTrans.x -= mTranslationOffset.x;
        kSpine.kTrans.y -= mTranslationOffset.y;
        kSpine.kTrans.z -= mTranslationOffset.z;

        PblQuaternion qRoll;
        qRoll._fX = mBallOrientation.GetSin();
        qRoll._fY = 0.0f;
        qRoll._fZ = 0.0f;
        qRoll._fS = mBallOrientation.GetCos();
        kSpine.kQuat *= qRoll.Inverse();
    }
    else
    {
        ZephyrTransform& kRoot = kPose.m_pJoints[0];
        kRoot.kTrans.x -= mTranslationOffset.x;
        kRoot.kTrans.y -= mTranslationOffset.y;
        kRoot.kTrans.z -= mTranslationOffset.z;

        PblQuaternion qPitch;
        qPitch._fX = mPitchOffset.GetSin();
        qPitch._fY = 0.0f;
        qPitch._fZ = 0.0f;
        qPitch._fS = mPitchOffset.GetCos();
        kRoot.kQuat *= qPitch.Inverse();

        PblQuaternion qTurn;
        qTurn._fX = 0.0f;
        qTurn._fY = mTurnOffset.GetSin();
        qTurn._fZ = 0.0f;
        qTurn._fS = mTurnOffset.GetCos();
        kRoot.kQuat *= qTurn.Inverse();
    }
}

// FUNCTION: SWBF2 0x0048cff0
void DroidekaElement::RenderUsingContext(const PblMatrix& mContext, const RedColor& color)
{
    if (!m_bEnabled)
    {
        return;
    }

    PblMatrix mScreen = mContext;
    mScreen[1] = mScreen[1] * -1.0f;
    mScreen.m[1][3] = 0.0f;

    float fScale = 0.75f / RedRenderer::GetScreenAspectRatio() * mDepth;
    mScreen.m[3][3] = 1.0f;
    mScreen[3] = mScreen[3] * fScale;

    PblMatrix mWorld(mRotation, mTranslation);

    PblMatrix mScaling;
    D3DXMatrixMultiply((D3DXMATRIX*)&mWorld, (const D3DXMATRIX*)&mWorld,
                       D3DXMatrixScaling((D3DXMATRIX*)&mScaling, mScale, mScale, mScale));
    D3DXMatrixMultiply((D3DXMATRIX*)&mScaling, (const D3DXMATRIX*)&mWorld,
                       (const D3DXMATRIX*)&mScreen);
    mScreen = mScaling;

    BuildWorldMatrices();
    mModel->GetRedModel(0)->Render(&mScreen, &mPose, color, 0x4000000, 0);
}

// FUNCTION: SWBF2 0x0048d1f0
DroidekaElement::~DroidekaElement()
{
}

// SYNTHETIC: SWBF2 0x0048d260
// DroidekaElement::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048d290
DroidekaElement::DroidekaElement()
    : mTime(0.0f), mState(EntityDroideka::Rolling), mPivotOffset(0.0f, 0.0f, 0.0f)
{
}

// FUNCTION: SWBF2 0x0048d390
void DroidekaElement::Reset()
{
    SetState(EntityDroideka::Rolling);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    mModel->GetRedModel(0)->SetupPose(mPose);
}

// FUNCTION: SWBF2 0x0048d3d0
bool DroidekaElement::Setup()
{
    if (!mClass)
    {
        return false;
    }

    mZephyrSkeleton.Open(*mClass->mAnimObj->GetZephyrSkeletonShared());
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    Reset();
    return true;
}

// FUNCTION: SWBF2 0x0048d410
bool DroidekaElement::Update(float fElapsed)
{
    if (mState == EntityDroideka::Idle)
    {
        NextState(0);
    }

    if (fElapsed > 0.0f)
    {
        float fRemaining = mClass->mTimeTotals[mState] - mTime;
        if (fElapsed >= fRemaining)
        {
            mZephyrPoseDyn.Update(fRemaining);
            mTime += fRemaining;
            fElapsed -= fRemaining;
        }
        mZephyrPoseDyn.Update(fElapsed);
        mTime += fElapsed;
    }
    return true;
}

// FUNCTION: SWBF2 0x0048d4d0
void DroidekaElement::NextState(int)
{
    SetState(EntityDroideka::sStateTable[mState].next[0]);
}

// FUNCTION: SWBF2 0x0048d4f0
void DroidekaElement::SetState(int iState)
{
    if (iState == EntityDroideka::Dead)
    {
        mState = iState;
        return;
    }

    ZephyrAnim* pAnim = mClass->mAnimations[iState];
    if (pAnim)
    {
        mTime = 0.0f;
        mZephyrPoseDyn.SetLoop(0);
        mZephyrPoseDyn.PlayAnimForward();
        mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
        mZephyrPoseDyn.SetCurTime(0.0f);
        mZephyrPoseDyn.Update(0.0f);
    }
    BuildWorldMatrices();
    mState = iState;
}

// FUNCTION: SWBF2 0x0048d5c0
void DroidekaElement::UpdateState(bool bRoll)
{
    if (bRoll)
    {
        if (mState == EntityDroideka::Idle || mState == EntityDroideka::RollToIdle)
        {
            SetState(EntityDroideka::IdleToRoll);
        }
    }
    else
    {
        switch (mState)
        {
        case EntityDroideka::IdleToRoll:
        case EntityDroideka::Rolling:
            SetState(EntityDroideka::RollToIdle);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0048d600
void DroidekaElement::SetSelected(bool bSelected)
{
    UpdateState(!bSelected);
}

