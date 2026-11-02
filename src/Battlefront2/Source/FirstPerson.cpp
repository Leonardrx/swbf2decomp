#include <d3dx9math.h>
#include <math.h>
#include "PebbleFL/Common/PblVector.h"
#include <string.h>

#include "FirstPerson.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "Character.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityCarrier.h"
#include "EntityDroideka.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "SoldierAnimationBank.h"
#include "Tracker.h"
#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "CollisionManager.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "PebbleFL/Common/PblHermite.h"

// GLOBAL: SWBF2 0x007b27a0
// __xmm@3e4ccccd3e4ccccd3e4ccccd3e4ccccd

extern bool netFrameLock;

namespace FirstPerson
{

// GLOBAL: SWBF2 0x01e55f00
FirstPersonRenderable* s_pRenderable[1];

// GLOBAL: SWBF2 0x01e55f0c
ZephyrSkeletonShared* s_pZephyrSkeletonShared;

// GLOBAL: SWBF2 0x01e51e2c
int s_iAnimBankCount;

// GLOBAL: SWBF2 0x01e55ef0
RedAnimation* s_apAnimBank[4];

// GLOBAL: SWBF2 0x01e51e30
char sModelName[64][256];
}

namespace FirstPerson
{
    // GLOBAL: SWBF2 0x01e51e28
    int sModelCount;
}
namespace FirstPerson
{
    // GLOBAL: SWBF2 0x01e55f04
    const RedModel* sModel[1];
}
namespace FirstPerson
{
    // GLOBAL: SWBF2 0x01e55f08
    int sModelId[1];
}

PblQuaternion Interpolate(const PblQuaternion& q0, const PblQuaternion& q1, float fT);


namespace FLRenderer
{
unsigned int GetViewportCount();
unsigned int GetCurrentViewport();
}

namespace GameLoop
{
    extern float sDeltaTime;
}
namespace GameLoop
{
    extern float sClientDeltaTime;
}



static const float ANIM_FRAME_SPEED = 30.0f;
static const float FIRST_PERSON_BLEND_SPEED = 3.0f;

// GLOBAL: SWBF2 0x01e55f10
float sFirstPersonOffsetX;
float sFirstPersonOffsetY = 0.35f;
float sFirstPersonOffsetZ = 0.05f;

// GLOBAL: SWBF2 0x00789760
// FirstPersonAnimName

namespace FirstPerson
{

// GLOBAL: SWBF2 0x01e55e30
ZephyrAnim* mAnim[48];

static const char* const FirstPersonAnimName[48] = {
    "humanfp_rifle_idle", "humanfp_rifle_run", "humanfp_rifle_shoot", 0, 0,
    "humanfp_rifle_reload", 0, "humanfp_rifle_jump", "humanfp_rifle_flail",
    "humanfp_handsdown", "humanfp_rifle_jump_land",
    "humanfp_bazooka_idle", "humanfp_bazooka_run", "humanfp_bazooka_shoot", 0, 0,
    "humanfp_bazooka_reload", 0, "humanfp_bazooka_jump", "humanfp_bazooka_flail",
    "humanfp_handsdown", "humanfp_bazooka_jump_land",
    "humanfp_tool_idle", "humanfp_tool_run", "humanfp_tool_shoot", 0, 0,
    "humanfp_tool_reload", "humanfp_tool_repair", "humanfp_tool_jump", "humanfp_tool_flail",
    "humanfp_handsdown", "humanfp_tool_jump_land",
    "humanfp_tool_idle", "humanfp_tool_run", "humanfp_grenade_shoot", "humanfp_grenade_shoot2",
    "humanfp_grenade_charge", "humanfp_tool_reload", "humanfp_tool_repair", "humanfp_tool_jump",
    "humanfp_tool_flail", "humanfp_handsdown", "humanfp_rifle_jump_land",
    "droidekafp_rifle_idle", "droidekafp_rifle_walk", "droidekafp_rifle_shoot",
    "droidekafp_rifle_reload",
};

struct CamPaths
{
    CamPaths();

    CamPathInfo mRollLeft;
    CamPathInfo mRollRight;
    CamPathInfo mCrouch;
    CamPathInfo mCrouchToProne;
    CamPathInfo mStandToProne;
    CamPathInfo mProneToStand;
    CamPathInfo mCrouchToStand;
    CamPathInfo mLand;
};

// FUNCTION: SWBF2 0x0051ed90
CamPaths::CamPaths()
{
}

// SYNTHETIC: SWBF2 0x00403820
// FirstPerson::`dynamic initializer for 'mCameraPaths''

// GLOBAL: SWBF2 0x01ebc300
// mCameraPaths
CamPaths mCameraPaths;

// SYNTHETIC: SWBF2 0x00444810
// PblHermiteVector3::PblHermiteVector3

// SYNTHETIC: SWBF2 0x004037d0
// FirstPerson::`dynamic initializer for 'mFPCamera''

// GLOBAL: SWBF2 0x01ebcce0
// mFPCamera
FirstPersonCameraManager mFPCamera[1];

// FUNCTION: SWBF2 0x0051ed00
bool IsActive(unsigned int)
{
    return FLRenderer::GetViewportCount() == 1 && GameLoop::FirstPersonEnabled();
}
}

namespace
{

// FUNCTION: SWBF2 0x0051ed20
bool FirstPersonLoaded_()
{
    return FLRenderer::GetViewportCount() == 1;
}
}

// FUNCTION: SWBF2 0x0051ed30
WEAPONCLASS _GetWeaponClassFromWeapon(const Weapon* pWeapon)
{
    if (pWeapon->GetClass()->mIsOffhand)
    {
        return GRENADE;
    }

    switch (SoldierAnimationBank::GetWeaponRoot(
        (SoldierAnimationBank::WEAPON)pWeapon->GetSoldierAnimationWeapon()))
    {
    case SoldierAnimationBank::RIFLE:
        return RIFLE;
    case SoldierAnimationBank::BAZOOKA:
        return BAZOOKA;
    case SoldierAnimationBank::INVALID_WEAPON:
    case SoldierAnimationBank::TOOL:
    case SoldierAnimationBank::PISTOL:
    case SoldierAnimationBank::MELEE:
        return TOOL;
    }
    return TOOL;
}

// FUNCTION: SWBF2 0x0051ecb0
PblMatrix CameraManager::GetChaseCameraTransform(int iCam)
{
    return mChaseCamera[iCam]->GetMatrix();
}

// FUNCTION: SWBF2 0x0051eda0
void FirstPersonRenderable::SetInitialPlayerId(int iPlayerId)
{
    if (mOwner)
    {
        Entity* pEntity = mOwner->GetEntity();
        if (!pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
            !pEntity->IsRtti(EntityDroideka::rttiHashEntityDroideka))
        {
            mOwner->SetInitialPlayerId(iPlayerId);
        }
    }
}

// FUNCTION: SWBF2 0x0051ee00
void FirstPersonRenderable::UpdateAimer(const PblMatrix& mMatrix)
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return;
    }

    switch (mObjectType)
    {
    case TYPE_COCKPIT:
    case TYPE_HOVER:
        UpdateCockpitAimer(mMatrix);
        break;
    case TYPE_SOLDIER:
        UpdateSoldierAimer(mMatrix);
        break;
    case TYPE_DROIDEKA:
        UpdateDroidekaAimer(mMatrix);
        break;
    }
}

// FUNCTION: SWBF2 0x0051eea0
void FirstPersonRenderable::UpdateSoldierAimer(const PblMatrix& mMatrix)
{
    if (!FirstPerson::IsActive(mCameraId) || !mOwner)
    {
        return;
    }

    mUpdatedSoldierAimer = true;

    const PblVector3& vVelocity = mOwner->GetGameObject()->GetVelocity();
    float fSpeed = 1.0f;
    float fSpeedSq = vVelocity.x * vVelocity.x + vVelocity.z * vVelocity.z;
    if (mSoldierState == RUN)
    {
        if (!(fSpeedSq < 20.0f))
        {
            fSpeed = fSpeedSq / 20.0f;
        }
        fSpeed *= 0.25f;
    }
    mZephyrPoseDynamic.Update(GameLoop::sClientDeltaTime * fSpeed);

    if (mBlendFactor > 0.0f)
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDynamic, (1.0f - mBlendFactor) * (1.0f - mBlendFactor));
        mBlendFactor -= GameLoop::sClientDeltaTime * FIRST_PERSON_BLEND_SPEED;
    }
    else
    {
        mZephyrPoseStatic.Set(mZephyrPoseDynamic, true);
    }

    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    RedPose kPose;
    kPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    const PblMatrix* pBone = kPose.Find(0x8629b159);

    PblMatrix mOffset = gMatrixIdentity;
    mOffset.RotateY(3.0f);
    mOffset.RotateZ(0.05f);
    mOffset.SetTranslation(PblVector3(sFirstPersonOffsetX, sFirstPersonOffsetY, sFirstPersonOffsetZ));

    if (mOwner->GetControllableClass()->mCockpitTension > 0.0f)
    {
        float fT = (float)exp(-(mOwner->GetControllableClass()->mCockpitTension * GameLoop::sClientDeltaTime));
        mCockpitRotation = Interpolate(PblQuaternion(mMatrix), mCockpitRotation, fT);
    }
    else
    {
        mCockpitRotation = PblQuaternion(mMatrix);
    }

    const PblMatrix mRotation(mCockpitRotation, mMatrix.GetTranslation());

    PblMatrix mBone = *pBone;
    mModelMatrix = (mOffset * mBone) * mRotation;

    mModelMatrix[0] = PblVector3(mModelMatrix.m[0][0], mModelMatrix.m[0][1], mModelMatrix.m[0][2]);
    mModelMatrix.m[0][3] = 0.0f;
    mModelMatrix[1] = PblVector3(mModelMatrix.m[1][0], mModelMatrix.m[1][1], mModelMatrix.m[1][2]);
    mModelMatrix.m[1][3] = 0.0f;
    mModelMatrix[2] = PblVector3(mModelMatrix.m[2][0], mModelMatrix.m[2][1], mModelMatrix.m[2][2]);
    mModelMatrix.m[2][3] = 0.0f;
    mWeaponMatrix = mModelMatrix;
}

// FUNCTION: SWBF2 0x0051f350
void FirstPersonRenderable::UpdateDroidekaAimer(const PblMatrix& mMatrix)
{
    if (!FirstPerson::IsActive(mCameraId) || !mOwner)
    {
        return;
    }

    float fSpeed = 1.0f;
    if (mSoldierState == DROIDEKA_SHOOT)
    {
        fSpeed = 2.0f;
    }
    mZephyrPoseDynamic.Update(GameLoop::sDeltaTime * fSpeed);
    mZephyrPoseStatic.Set(mZephyrPoseDynamic, true);

    if (mBlendFactor > 0.0f)
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDynamic, (1.0f - mBlendFactor) * (1.0f - mBlendFactor));
        mBlendFactor -= GameLoop::sClientDeltaTime * FIRST_PERSON_BLEND_SPEED;
    }
    else
    {
        mZephyrPoseStatic.Set(mZephyrPoseDynamic, true);
    }

    mZephyrSkeleton.BuildWorldMatrices(gMatrixIdentity);
    RedPose kPose;
    kPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    const PblMatrix* pBone = kPose.Find(0x8629b159);

    PblMatrix mOffset = gMatrixIdentity;
    mOffset.RotateY(3.2f);
    mOffset.SetTranslation(PblVector3(0.01f, 0.35f, -0.0f));

    if (mOwner->GetControllableClass()->mCockpitTension > 0.0f)
    {
        float fT = (float)exp(-(mOwner->GetControllableClass()->mCockpitTension * GameLoop::sDeltaTime));
        mCockpitRotation = Interpolate(PblQuaternion(mMatrix), mCockpitRotation, fT);
    }
    else
    {
        mCockpitRotation = PblQuaternion(mMatrix);
    }

    const PblMatrix mRotation(mCockpitRotation, mMatrix.GetTranslation());

    PblMatrix mBone = *pBone;
    mModelMatrix = (mOffset * mBone) * mRotation;

    const PblMatrix* pWeaponBone = kPose.Find(0x2b960099);
    Weapon* pWeapon = mOwner->GetWeaponPtr(mOwner->GetActiveWeaponChannel());
    if (pWeaponBone && pWeapon)
    {
        PblMatrix mWeapon;
        D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&mWeapon), reinterpret_cast<const D3DXMATRIX*>(pWeaponBone), reinterpret_cast<const D3DXMATRIX*>(&mModelMatrix));
        mWeaponMatrix = mWeapon;
    }
}

// FUNCTION: SWBF2 0x00520660
void FirstPersonRenderable::UpdateCockpitAimer(const PblMatrix& mMatrix)
{
    if (!FirstPerson::IsActive(mCameraId) || !bReadyToRender || mOwner->mTargetInfo.mIsAiming)
    {
        return;
    }

    RedPose kPose;
    PblMatrix mCamera = mMatrix;

    PblVector3 vOffset;
    if (mOwner->GetEntity()->GetGameObject()->GetEntityType() == ENT_FLYER)
    {
        EntityFlyer* pFlyer = (EntityFlyer*)mOwner->GetEntity()->GetGameObject();
        float fSpeed = pFlyer->mGetSpeedSpeed;
        EntityFlyerClass* pClass = pFlyer->mClass;
        if (fSpeed > pClass->mMinFlyerSpeed)
        {
            float fTopSpeed = pClass->mMaxFlyerSpeed > pClass->mBoostFlyerSpeed ? pClass->mMaxFlyerSpeed
                                                                                : pClass->mBoostFlyerSpeed;
            float fT = (fSpeed - pClass->mMinFlyerSpeed) / (fTopSpeed - pClass->mMinFlyerSpeed);
            if (fT < 0.0f)
            {
                fT = 0.0f;
            }
            else if (fT > 1.0f)
            {
                fT = 1.0f;
            }
            float fS = 1.0f - fT;
            vOffset = fS * PblVector3(0.0f, -1.5f, -1.7f) + fT * PblVector3(0.0f, -1.4f, -1.0f);
        }
        else
        {
            vOffset.x = 0.0f;
            vOffset.y = -1.5f;
            vOffset.z = -1.7f;
        }
    }
    else
    {
        vOffset.x = 0.0f;
        vOffset.y = -1.5f;
        vOffset.z = -1.5f;
    }

    PblMatrix mOffset = gMatrixIdentity;
    mOffset.SetTranslation(vOffset);

    if (mOwner->GetControllableClass()->mCockpitTension > 0.0f)
    {
        float fDeltaTime = GameLoop::sClientDeltaTime;
        if (fDeltaTime > 0.0f)
        {
            float fT = (float)exp(-(mOwner->GetControllableClass()->mCockpitTension * fDeltaTime));
            mCockpitRotation = Interpolate(PblQuaternion(mCamera), mCockpitRotation, fT);
        }
    }
    else
    {
        mCockpitRotation = PblQuaternion(mCamera);
    }

    PblVector3 vPos = mCamera.GetTranslation();
    if (mOwner->GetEntity()->GetGameObject()->GetEntityType() == ENT_WALKER)
    {
        const RedPose* pPose = mOwner->GetEntity()->GetGameObject()->GetPose();
        const PblMatrix* pBone = pPose->Find(0x16E27226);
        if (mCockpitTrans.x == -66.6f && mCockpitTrans.y == 66.6f && mCockpitTrans.z == -66.6f)
        {
            mCockpitTrans = pBone->GetTranslation();
        }
        vPos = mCamera.GetTranslation() + (pBone->GetTranslation() - mCockpitTrans) * 0.2f;
    }

    const PblMatrix mRotation(mCockpitRotation, vPos);
    mModelMatrix = mOffset * mRotation;
}

// FUNCTION: SWBF2 0x0051f750
FirstPersonRenderable::FirstPersonRenderable(unsigned int uCameraId, const ZephyrSkeletonShared* pShared)
    : Renderable(8)
{
    mObjectType = TYPE_NONE;
    mObjectModel = 0;
    mOwner = 0;
    mUpdatedSoldierAimer = false;
    mCurrentWeapon = 0;
    mZephyrSkeleton.Open(*pShared, mZephyrPoseStatic);
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mZephyrPoseDynamic.Open(mZephyrSkeleton);
    mWeaponClass = WEAPONCLASS_NONE;
    mSoldierState = NUM_SOLDIERANIMATIONS;
    mCameraId = uCameraId;
    mBlendFactor = 0.0f;
    mAnimPlaySpeed = 1.0f;
    fTransitionTimerMax = 0.0f;
    fTransitionTimer = 0.0f;
    mShadowVal = 1.0f;
    SetAnimation(FirstPerson::mAnim[0], true);
    mCockpitTrans = PblVector3(-66.6f, 66.6f, -66.6f);
    bReadyToRender = false;
    bStreamingModel = false;
}

// SYNTHETIC: SWBF2 0x0051f8e0
// FirstPersonRenderable::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0051f910
FirstPersonRenderable::~FirstPersonRenderable()
{
    mZephyrSkeleton.Close();
    mZephyrPoseStatic.Close();
    mZephyrPoseDynamic.Close();
    Renderable::Deactivate();
}

// FUNCTION: SWBF2 0x0051f9b0
void FirstPersonRenderable::Activate(int value)
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return;
    }

    bReadyToRender = false;
    PblMatrix mCamera = CameraManager::sInstance->mChaseCamera[mCameraId]->GetMatrix();
    mCockpitRotation = PblQuaternion(mCamera);
    mCockpitTrans = PblVector3(-66.6f, 66.6f, -66.6f);
    Renderable::Activate(value);
    mCurrentWeapon = 0;
    mBlendHandsDown = true;
}

// FUNCTION: SWBF2 0x0051fa80
void FirstPersonRenderable::Deactivate()
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return;
    }

    mOwner = 0;
    mUpdatedSoldierAimer = false;
    Renderable::Deactivate();
}

// FUNCTION: SWBF2 0x0051fab0
void FirstPersonRenderable::SetAnimation(const ZephyrAnim* pAnim, bool bLoop)
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return;
    }

    mZephyrPoseDynamic.SetAnimation(pAnim, ANIM_FRAME_SPEED);
    mZephyrPoseDynamic.SetCurTime(0.0f);
    mZephyrPoseDynamic.SetLoop(bLoop);
}

// FUNCTION: SWBF2 0x0051fb40
void FirstPersonRenderable::SetSoldierState(int iState)
{
    mEntitySoldierState = iState;
}

// FUNCTION: SWBF2 0x0051fb70
void FirstPersonRenderable::UpdateSoldier(const RedModel* pModel, Controllable* pOwner, Aimer* pAimer)
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return;
    }

    bool bChanged = false;
    bool bTransition = true;
    FirstPersonCameraManager* pCamera = &FirstPerson::mFPCamera[mCameraId];

    if (!bStreamingModel)
    {
        bReadyToRender = true;
    }
    mObjectModel = pModel;
    mObjectType = TYPE_SOLDIER;
    mOwner = pOwner;
    mAimer = pAimer;
    if (!pOwner)
    {
        return;
    }

    Weapon* pWeapon = pOwner->GetWeaponPtr(pOwner->GetWeaponIndex(pOwner->GetActiveWeaponChannel()));
    if (pOwner->GetActiveWeaponChannel() &&
        pWeapon->GetState() != Weapon::FIRE && pWeapon->GetState() != Weapon::FIRE2)
    {
        pWeapon = pOwner->GetWeaponPtr(pOwner->GetWeaponIndex(0));
    }
    WEAPONCLASS eWeaponClass = _GetWeaponClassFromWeapon(pWeapon);

    const PblVector3& vVelocity = pOwner->GetGameObject()->GetVelocity();
    SOLDIERANIMATION eAnim = mSoldierState;
    bool bLoop = false;
    float fBlend = 1.0f;
    float fSpeedSq = vVelocity.x * vVelocity.x + vVelocity.z * vVelocity.z;
    mAnimPlaySpeed = 1.0f;

    if ((eAnim == SHOOT1 || eAnim == SHOOT2) &&
        !mZephyrPoseDynamic.m_bAnimFinished && !pWeapon->GetFiredFlag())
    {
        return;
    }

    bool bWeaponPending = false;
    if (mCurrentWeapon != pWeapon && mCurrentWeapon && !pWeapon->GetFiredFlag())
    {
        if (eAnim != HANDSDOWN)
        {
            eAnim = HANDSDOWN;
            fBlend = 1.0f;
            bChanged = true;
            bWeaponPending = true;
        }
        else
        {
            if (mBlendFactor > 0.0f)
            {
                return;
            }
            bChanged = true;
        }
    }

    if (!bWeaponPending)
    {
        mCurrentWeapon = pWeapon;

        bool bWeaponAnim = true;
        switch (pWeapon->GetState())
        {
        case Weapon::FIRE:
        case Weapon::FIRE2:
            if (!pWeapon->GetFiredFlag())
            {
                bWeaponAnim = false;
            }
            else if (pWeapon->GetClass() && pWeapon->GetClass()->mNoFirstPersonFireAnim)
            {
                bLoop = true;
                eAnim = HANDSDOWN;
            }
            else
            {
                fBlend = 0.0f;
                bChanged = true;
                eAnim = pWeapon->GetState() == Weapon::FIRE ? SHOOT1 : SHOOT2;
                pWeapon->ClearFiredFlag();
                bLoop = false;
            }
            break;
        case Weapon::RELOAD:
            eAnim = RELOAD;
            bLoop = false;
            break;
        default:
            bWeaponAnim = false;
            break;
        }

        if (!bWeaponAnim)
        {
            switch (mEntitySoldierState)
            {
            case SoldierState::ROLL:
                eAnim = JUMP;
                bLoop = false;
                break;
            case SoldierState::JUMP:
                bLoop = false;
                eAnim = JUMP;
                fTransitionTimer = 0.0f;
                fTransitionTimerMax = 0.5f;
                mTransitionAnim = JUMP_LAND;
                bTransition = false;
                break;
            case SoldierState::JET_JUMP:
            case SoldierState::JET_HOVER:
                break;
            case SoldierState::FALL:
                bLoop = true;
                eAnim = FLAIL;
                fTransitionTimer = 0.0f;
                fTransitionTimerMax = 1.0f;
                mTransitionAnim = HANDSDOWN;
                bTransition = false;
                break;
            default:
                if (fSpeedSq < 0.1f)
                {
                    eAnim = IDLE;
                    if (pCamera->mCurrCamPt > -1 && pCamera->mIsBobbing)
                    {
                        pCamera->ReturnToCenter();
                        pCamera->SetBobbing(false);
                    }
                }
                else
                {
                    eAnim = RUN;
                }
                bLoop = true;
                break;
            }
        }
    }

    if (fSpeedSq >= 1.0f && pCamera->mCurrCamPt <= -1 &&
        mEntitySoldierState != SoldierState::JET_HOVER &&
        mEntitySoldierState != SoldierState::JET_JUMP && bTransition == true)
    {
        if (mEntitySoldierState == SoldierState::PRONE)
        {
            pCamera->ViewBob(0.5f);
        }
        else
        {
            pCamera->ViewBob(1.0f);
        }
    }

    if (fSpeedSq >= 0.1f && mEntitySoldierState == SoldierState::PRONE)
    {
        fTransitionTimer = 0.0f;
        eAnim = HANDSDOWN;
        fTransitionTimerMax = 0.0f;
        mTransitionAnim = HANDSDOWN;
        bLoop = true;
    }
    else if (bTransition && fTransitionTimerMax > fTransitionTimer)
    {
        eAnim = mTransitionAnim;
        fTransitionTimer += GameLoop::sDeltaTime;
    }

    if (mBlendHandsDown && bReadyToRender)
    {
        bLoop = false;
        fBlend = 0.0f;
        eAnim = HANDSDOWN;
        bChanged = true;
        mBlendHandsDown = false;
        fTransitionTimer = 0.0f;
        fTransitionTimerMax = 1.0f;
        mTransitionAnim = IDLE;
    }

    if (mSoldierState != eAnim)
    {
        bChanged = true;
    }
    mSoldierState = eAnim;
    mWeaponClass = eWeaponClass;

    if (bChanged)
    {
        SetAnimation(FirstPerson::mAnim[eWeaponClass * NUM_SOLDIERANIMATIONS + eAnim], bLoop);
        if (mUpdatedSoldierAimer)
        {
            mBlendFactor = fBlend;
        }
        else
        {
            mBlendFactor = 0.0f;
        }
    }
}

// FUNCTION: SWBF2 0x0051ffe0
void FirstPersonRenderable::UpdateCockpit(const RedModel* pModel, Controllable* pOwner, Aimer* pAimer)
{
    if (!FirstPerson::IsActive(mCameraId) || !pOwner)
    {
        return;
    }

    PblMatrix mCamera = CameraManager::sInstance->mChaseCamera[mCameraId]->GetMatrix();
    if (!bReadyToRender)
    {
        pOwner->GetTrackable()->SetupCameraTrackMatrix(mCamera, 0.0f);
        mCockpitRotation = PblQuaternion(mCamera);
        mCockpitTrans = PblVector3(-66.6f, 66.6f, -66.6f);
    }
    if (!bStreamingModel)
    {
        bReadyToRender = true;
    }
    mObjectModel = pModel;
    mOwner = pOwner;
    if (pOwner->mCharacter && pOwner->mCharacter->mUnit)
    {
        SetInitialPlayerId(pOwner->mCharacter->mUnit->mInitialPlayerId);
    }
    mAimer = pAimer;
}

// FUNCTION: SWBF2 0x00520110
void FirstPersonRenderable::UpdateDroideka(const RedModel* pModel, Controllable* pOwner, Aimer* pAimer)
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return;
    }

    bool bChanged = false;
    if (!bStreamingModel)
    {
        bReadyToRender = true;
    }
    mObjectModel = pModel;
    mObjectType = TYPE_DROIDEKA;
    mOwner = pOwner;
    mAimer = pAimer;
    if (!pOwner)
    {
        return;
    }

    Weapon* pWeapon = pOwner->GetWeaponPtr(pOwner->GetWeaponIndex(pOwner->GetActiveWeaponChannel()));
    WEAPONCLASS eWeaponClass = _GetWeaponClassFromWeapon(pWeapon);
    mAnimPlaySpeed = 1.0f;

    SOLDIERANIMATION eAnim = DROIDEKA_IDLE;
    bool bLoop;
    switch (pWeapon->GetState())
    {
    case Weapon::FIRE:
        eAnim = DROIDEKA_SHOOT;
        mAnimPlaySpeed = 0.1f;
        bLoop = true;
        break;
    case Weapon::FIRE2:
        eAnim = DROIDEKA_SHOOT;
        bLoop = false;
        break;
    case Weapon::RELOAD:
        eAnim = DROIDEKA_RELOAD;
        bLoop = false;
        break;
    default:
        if (pOwner->GetThrust() != 0.0f)
        {
            eAnim = DROIDEKA_WALK;
        }
        bLoop = true;
        break;
    }

    if (fTransitionTimerMax > fTransitionTimer)
    {
        eAnim = mTransitionAnim;
        fTransitionTimer += GameLoop::sDeltaTime;
    }

    if (mWeaponClass != eWeaponClass)
    {
        bChanged = true;
    }
    mWeaponClass = eWeaponClass;
    if (mSoldierState != eAnim)
    {
        bChanged = true;
    }
    mSoldierState = eAnim;

    if (bChanged)
    {
        mBlendFactor = 1.0f;
        if (eAnim > DROIDEKA_RELOAD)
        {
            eAnim = DROIDEKA_RELOAD;
        }
        SetAnimation(FirstPerson::mAnim[NUM_SOLDIERANIMATIONS * 4 + eAnim], bLoop);
    }
}

// FUNCTION: SWBF2 0x00520280
bool HideFirstPersonHands(int iPlayer)
{
    if (!FirstPerson::IsActive(iPlayer))
    {
        return false;
    }

    int iLocal = NetGame::GetLocalIndex(iPlayer);
    if (iLocal < 0)
    {
        return false;
    }

    Trackable* pTrackable = Tracker::GetTracker(iLocal)->GetTrackable();
    if (!pTrackable)
    {
        return false;
    }
    GameObject* pObject = pTrackable->GetGameObject();
    if (!pObject)
    {
        return false;
    }
    TargetInfo* pInfo = pObject->GetTargetInfo();
    if (!pInfo)
    {
        return false;
    }

    if (pObject->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
        ((EntitySoldier*)pObject)->InNonFirstPersonState())
    {
        return true;
    }
    return pInfo->mIsAiming;
}

// FUNCTION: SWBF2 0x00520300
bool FirstPersonRenderable::IsOwnerAiming() const
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return false;
    }
    return mOwner->mTargetInfo.mIsAiming;
}

// FUNCTION: SWBF2 0x00520320
void FirstPersonRenderable::RenderSoldier(unsigned int uFlags) const
{
    if (!FirstPerson::IsActive(mCameraId) || !bReadyToRender || HideFirstPersonHands(mCameraId) || IsOwnerAiming() ||
        FLRenderer::GetCurrentViewport() != mCameraId)
    {
        return;
    }

    RedPose pose;
    pose.ConvertFromZephyrPose(mZephyrPoseStatic);

    bool bShadow = RedStencilShadow::GetShadowEnable();
    int iLight = -1;
    if (RedLight::GetGlobalDirectionalLight(0)->IsShadowCaster())
    {
        iLight = 0;
    }
    else if (RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster())
    {
        iLight = 1;
    }
    else
    {
        bShadow = false;
    }

    float fSavedFactor = 0.0f;
    if (bShadow)
    {
        fSavedFactor = RedLight::m_sGlobalDirectionalFactor[iLight];
        const GameObject* pIgnore = mOwner->GetGameObject();
        const PblVector3 vDir = -RedLight::GetGlobalDirectionalLight(iLight)->m_vDirection;
        float& fShadowVal = const_cast<FirstPersonRenderable*>(this)->mShadowVal;
        if (CollisionManager::RayTest(mWeaponMatrix.GetTranslation(), vDir, 500.0f, &pIgnore, 1, 0))
        {
            fShadowVal -= fShadowVal * 0.5f;
        }
        else
        {
            fShadowVal += (1.0f - fShadowVal) * 0.5f;
        }
        RedLight::m_sGlobalDirectionalFactor[iLight] = fShadowVal;
    }

    uFlags |= 0x80001;
    if (mObjectModel)
    {

        RedShadingPose* pShadingPose = 0;
        RedShadingPose shadingPose;
        if (mOwner)
        {
            GameObject* pObject = mOwner->GetGameObject();
            if (pObject && pObject->IsRtti(EntitySoldier::rttiHashEntitySoldier))
            {
                unsigned char ucInvisibility = static_cast<EntitySoldier*>(pObject)->GetInvisibilityAlpha();
                if (ucInvisibility)
                {
                    unsigned char ucCloak = 0xff;
                    if (ucInvisibility > 0xd8)
                    {
                        ucInvisibility = 0xd8;
                    }
                    if (ucInvisibility < 0x80)
                    {
                        ucCloak = (unsigned char)(ucInvisibility * 2);
                    }
                    pShadingPose = &shadingPose;
                    shadingPose.BindToModel(mObjectModel);
                    if (RedShadingState* pState = shadingPose.CreateDefaultShadingState(0x4cc5882a))
                    {
                        RedColor color = RedColor::WHITE;
                        color.a = ucCloak;
                        pState->GetColorParam(0xbc6f8a5e) = color;
                        pState->GetIntParam(0x7802a19a) = 1;
                    }
                }
            }
        }
        mObjectModel->Render(&mModelMatrix, &pose, RedColor::WHITE, uFlags, pShadingPose);
    }

    if (mCurrentWeapon)
    {
        const PblMatrix mWeapon(reinterpret_cast<const _D3DXMATRIXA16*>(&mWeaponMatrix));
        mCurrentWeapon->Render(mWeapon, &pose, RedColor::WHITE, uFlags, true);
    }

    if (bShadow)
    {
        RedLight::m_sGlobalDirectionalFactor[iLight] = fSavedFactor;
    }
}

// FUNCTION: SWBF2 0x00520de0
void FirstPersonRenderable::Render(int value)
{
    const unsigned int uFlags = (unsigned int)value;
    if (!FirstPerson::IsActive(mCameraId) || CameraManager::sInstance->IsFreeLook(mCameraId))
    {
        return;
    }

    switch (mObjectType)
    {
    case TYPE_COCKPIT:
    case TYPE_HOVER:
        RenderCockpit(uFlags);
        break;
    case TYPE_SOLDIER:
        RenderSoldier(uFlags);
        break;
    case TYPE_DROIDEKA:
        RenderDroideka(uFlags);
        break;
    }
}

// FUNCTION: SWBF2 0x00520fc0
void FirstPerson::PreInit()
{
    FirstPerson::sModelCount = 0;
    FirstPerson::sModel[0] = 0;
    FirstPerson::sModelId[0] = 0;
    FirstPerson::sModelId[0] = -2;
    s_pZephyrSkeletonShared = (ZephyrSkeletonShared*)-2;
    s_pRenderable[0] = 0;
}

// FUNCTION: SWBF2 0x00520fb0
void FirstPerson::ClearBanks()
{
    s_iAnimBankCount = 0;
}

// FUNCTION: SWBF2 0x00521170
void FirstPerson::Deinit()
{
    if (s_pRenderable[0])
    {
        Activate(0, false);
        delete s_pRenderable[0];
        s_pRenderable[0] = 0;
    }
    ClearBanks();
    s_pZephyrSkeletonShared = 0;
}

// FUNCTION: SWBF2 0x00520ec0
bool FirstPerson::AddBank(const char* pszName)
{
    if (!IsActive(0))
    {
        return false;
    }
    RedAnimation* pAnim = RedAnimation::Find(PblHash(pszName));
    if (!pAnim)
    {
        return false;
    }
    if (!s_pZephyrSkeletonShared)
    {
        s_pZephyrSkeletonShared = pAnim->_pSkeletonShared;
    }
    if (!pAnim->_pZephyrAnimBank)
    {
        return false;
    }
    for (int i = 0; i < s_iAnimBankCount; ++i)
    {
        if (s_apAnimBank[i] == pAnim)
        {
            return true;
        }
    }
    if (s_iAnimBankCount >= 4)
    {
        return false;
    }
    s_apAnimBank[s_iAnimBankCount++] = pAnim;
    return true;
}

// FUNCTION: SWBF2 0x00520f50
const ZephyrAnim* FirstPerson::FindAnimation(const char* pszName)
{
    if (!IsActive(0))
    {
        return 0;
    }
    unsigned int uiHash = PblTEMPHash(pszName);
    for (int i = 0; i < s_iAnimBankCount; ++i)
    {
        if (s_apAnimBank[i]->_pZephyrAnimBank)
        {
            const ZephyrAnim* pAnim = s_apAnimBank[i]->_pZephyrAnimBank->Find(uiHash);
            if (pAnim)
            {
                return pAnim;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00521000
void FirstPerson::Init()
{
    if (!FirstPersonLoaded_())
    {
        return;
    }
    s_pZephyrSkeletonShared = 0;
    AddBank("humanfp");
    const ZephyrAnim* pDefault = FindAnimation("humanfp_tool_idle");
    unsigned int i;
    for (i = 0; i < WEAPONCLASS_NONE * NUM_SOLDIERANIMATIONS; ++i)
    {
        if (FirstPersonAnimName[i])
        {
            mAnim[i] = (ZephyrAnim*)FindAnimation(FirstPersonAnimName[i]);
        }
        if (!mAnim[i])
        {
            mAnim[i] = (ZephyrAnim*)pDefault;
        }
    }
    for (; i < 48; ++i)
    {
        mAnim[i] = (ZephyrAnim*)pDefault;
    }
    for (unsigned int uCam = 0; uCam < CameraManager::Instance()->GetNumCameras(); ++uCam)
    {
        s_pRenderable[uCam] = new FirstPersonRenderable(uCam, s_pZephyrSkeletonShared);
    }
}

// FUNCTION: SWBF2 0x00521100
void FirstPerson::PostLoad()
{
    if (!FirstPersonLoaded_())
    {
        return;
    }
    const ZephyrAnim* pDefault = FindAnimation("humanfp_tool_idle");
    if (!AddBank("droidekafp"))
    {
        return;
    }
    for (unsigned int i = WEAPONCLASS_NONE * NUM_SOLDIERANIMATIONS; i < 48; ++i)
    {
        if (FirstPersonAnimName[i])
        {
            mAnim[i] = (ZephyrAnim*)FindAnimation(FirstPersonAnimName[i]);
        }
        if (!mAnim[i])
        {
            mAnim[i] = (ZephyrAnim*)pDefault;
        }
    }
}

// FUNCTION: SWBF2 0x005211b0
void FirstPerson::Activate(unsigned int uPlayer, bool bActive)
{
    if (!IsActive(uPlayer))
    {
        return;
    }

    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsOneWorld() && NetGame::IsHost()))
    {
        if (s_pRenderable[uPlayer])
        {
            if (bActive)
            {
                s_pRenderable[uPlayer]->Activate(255);
                mFPCamera[uPlayer].mCurrCamPt = -1;
            }
            else
            {
                s_pRenderable[uPlayer]->SetInitialPlayerId(-1);
                s_pRenderable[uPlayer]->Deactivate();
            }
        }
    }
}

// FUNCTION: SWBF2 0x005213c0
int FirstPerson::AddGeometry(const char* pszName)
{
    if (!IsActive(0))
    {
        return -1;
    }
    for (int i = 0; i < FirstPerson::sModelCount; ++i)
    {
        if (!_strnicmp(pszName, sModelName[i], 255))
        {
            return i;
        }
    }
    if (FirstPerson::sModelCount < 64)
    {
        strncpy(sModelName[FirstPerson::sModelCount], pszName, 255);
        sModelName[FirstPerson::sModelCount][255] = 0;
        return FirstPerson::sModelCount++;
    }
    return -1;
}



namespace FirstPerson
{

// GLOBAL: SWBF2 0x01ebc2c0
PblMatrix mCamMatrix;
}

// FUNCTION: SWBF2 0x00521de0
void FirstPersonCameraManager::Update(PblMatrix& rMatrix)
{
    if (!FirstPerson::IsActive(0))
    {
        return;
    }
    mCurrCamTime -= GameLoop::GetDeltaTime();
    if (mCurrCamPt != -1)
    {
        float fT = 1.0f - mCurrCamTime / mCamTime[mCurrCamPt];
        if (fT > 1.0f)
        {
            fT = 1.0f;
        }
        if (mIsReturning)
        {
            PblVector3 vLin(mLinVec.x * (1.0f - fT), mLinVec.y * (1.0f - fT), mLinVec.z * (1.0f - fT));
            PblVector3 vOffset;
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vOffset), reinterpret_cast<const D3DXVECTOR3*>(&vLin), reinterpret_cast<const D3DXMATRIX*>(&rMatrix));
            rMatrix.m[3][0] += vOffset.x;
            rMatrix.m[3][1] += vOffset.y;
            rMatrix.m[3][2] += vOffset.z;
            rMatrix.m[3][3] = 1.0f;
        }
        else
        {
            PblVector3 vPos = mHermPos[mCurrCamPt].getValue(fT);
            PblVector3 vOffset;
            D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vOffset), reinterpret_cast<const D3DXVECTOR3*>(&vPos), reinterpret_cast<const D3DXMATRIX*>(&rMatrix));
            rMatrix.m[3][0] += vOffset.x;
            rMatrix.m[3][1] += vOffset.y;
            rMatrix.m[3][2] += vOffset.z;
            rMatrix.m[3][3] = 1.0f;
            mCurRotVec = mHermRot[mCurrCamPt].getValue(fT);
            mCameraPos = rMatrix.GetTranslation();

            PblMatrix mRotation = gMatrixIdentity;
            mRotation.RotateX(mCurRotVec.x);
            mRotation.RotateY(mCurRotVec.y);
            mRotation.RotateZ(mCurRotVec.z);
            rMatrix = mRotation * rMatrix;
        }
        if (fT == 1.0f)
        {
            if (++mCurrCamPt < mNumPoints)
            {
                mCurrCamTime = mCamTime[mCurrCamPt];
            }
            else
            {
                mCurRotVec = PblVector3(0.0f, 0.0f, 0.0f);
                mCurrCamPt = -1;
                mIsGoingProne = false;
                mIsReturning = false;
            }
        }
    }
    mCameraPos = rMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x00521450
void FirstPerson::SetSoldierState(unsigned int uPlayer, int iState)
{
    if (IsActive(uPlayer) && uPlayer < 1)
    {
        s_pRenderable[uPlayer]->SetSoldierState(iState);
    }
}

// FUNCTION: SWBF2 0x00521250
void FirstPerson::Update(unsigned int uPlayer, eViewObjectType eType, int iArg,
                         Controllable* pOwner, Aimer* pAimer)
{
    if (!IsActive(uPlayer))
    {
        return;
    }
    if (NetGame::IsEnabled() && !NetGame::IsClient() && !(NetGame::IsOneWorld() && NetGame::IsHost()))
    {
        return;
    }
    if (!s_pRenderable[uPlayer])
    {
        return;
    }
    if (iArg != FirstPerson::sModelId[uPlayer] && GameLoop::mNumCameras == 1)
    {
        if (iArg == -1 || iArg == -2)
        {
            FirstPerson::sModel[uPlayer] = 0;
            s_pRenderable[uPlayer]->bReadyToRender = false;
            s_pRenderable[uPlayer]->bStreamingModel = false;
            FirstPerson::sModelId[uPlayer] = iArg;
        }
        else
        {
            FirstPerson::sModel[uPlayer] = LoadDynamicModel(sModelName[iArg], true);
            if (FirstPerson::sModel[uPlayer])
            {
                s_pRenderable[uPlayer]->bStreamingModel = false;
                FirstPerson::sModelId[uPlayer] = iArg;
            }
            else
            {
                s_pRenderable[uPlayer]->bReadyToRender = false;
                s_pRenderable[uPlayer]->bStreamingModel = true;
            }
        }
    }
    switch (eType)
    {
    case TYPE_SOLDIER:
        s_pRenderable[uPlayer]->UpdateSoldier(FirstPerson::sModel[uPlayer], pOwner, pAimer);
        break;
    case TYPE_HOVER:
        s_pRenderable[uPlayer]->mObjectType = TYPE_HOVER;
        s_pRenderable[uPlayer]->UpdateCockpit(FirstPerson::sModel[uPlayer], pOwner, pAimer);
        break;
    case TYPE_COCKPIT:
        s_pRenderable[uPlayer]->mObjectType = TYPE_COCKPIT;
        s_pRenderable[uPlayer]->UpdateCockpit(FirstPerson::sModel[uPlayer], pOwner, pAimer);
        break;
    case TYPE_DROIDEKA:
        s_pRenderable[uPlayer]->UpdateDroideka(FirstPerson::sModel[uPlayer], pOwner, pAimer);
        break;
    }
}

// FUNCTION: SWBF2 0x005215c0
void FirstPerson::UpdateCamera(unsigned int uPlayer, PblMatrix& rMatrix)
{
    if (!IsActive(uPlayer))
    {
        return;
    }
    mFPCamera[uPlayer].Update(rMatrix);
    mCamMatrix = rMatrix;
    if (netFrameLock || !NetGame::IsEnabled())
    {
        s_pRenderable[uPlayer]->UpdateAimer(rMatrix);
    }
}

// FUNCTION: SWBF2 0x00521650
void FirstPerson::UpdateAimer(unsigned int uPlayer, const PblMatrix& rMatrix)
{
    if (IsActive(uPlayer))
    {
        s_pRenderable[uPlayer]->UpdateAimer(rMatrix);
    }
}

// FUNCTION: SWBF2 0x00520e60
void FirstPersonRenderable::SetAnimAndTransition(SOLDIERANIMATION aAnim,
                                                SOLDIERANIMATION aTrans, float aTime)
{
    if (!FirstPerson::IsActive(mCameraId))
    {
        return;
    }
    fTransitionTimer = 0.0f;
    int iWeaponClass = mWeaponClass;
    if (iWeaponClass == WEAPONCLASS_NONE)
    {
        iWeaponClass = RIFLE;
    }
    fTransitionTimerMax = aTime;
    mTransitionAnim = aTrans;
    SetAnimation(FirstPerson::mAnim[iWeaponClass * NUM_SOLDIERANIMATIONS + aAnim], false);
}

// FUNCTION: SWBF2 0x0051fb50
void FirstPersonRenderable::ClearFiringState()
{
    if (mSoldierState == SHOOT1 || mSoldierState == SHOOT2)
    {
        mSoldierState = IDLE;
    }
}

// FUNCTION: SWBF2 0x00521480
void FirstPerson::ClearFiringState(unsigned int uPlayer)
{
    if (IsActive(uPlayer) && uPlayer < 1)
    {
        s_pRenderable[uPlayer]->ClearFiringState();
    }
}

// FUNCTION: SWBF2 0x005214a0
void FirstPerson::RollLeft(unsigned int uPlayer)
{
    if (IsActive(uPlayer))
    {
        s_pRenderable[uPlayer]->SetAnimAndTransition(HANDSDOWN, JUMP_LAND, mFPCamera[uPlayer].RollLeft());
    }
}

// FUNCTION: SWBF2 0x005214e0
void FirstPerson::RollRight(unsigned int uPlayer)
{
    if (IsActive(uPlayer))
    {
        s_pRenderable[uPlayer]->SetAnimAndTransition(HANDSDOWN, JUMP_LAND, mFPCamera[uPlayer].RollRight());
    }
}

// FUNCTION: SWBF2 0x00521520
void FirstPerson::ProneToStand(unsigned int uPlayer)
{
    if (IsActive(uPlayer))
    {
        s_pRenderable[uPlayer]->SetAnimAndTransition(HANDSDOWN, JUMP_LAND, mFPCamera[uPlayer].ProneToStand());
    }
}

// FUNCTION: SWBF2 0x00521560
void FirstPerson::CrouchToStand(unsigned int uPlayer)
{
    if (IsActive(uPlayer))
    {
        mFPCamera[uPlayer].CrouchToStand();
    }
}

// FUNCTION: SWBF2 0x00521590
void FirstPerson::Land(unsigned int uPlayer)
{
    if (IsActive(uPlayer))
    {
        mFPCamera[uPlayer].Land();
    }
}

// FUNCTION: SWBF2 0x00520b20
void FirstPersonRenderable::RenderCockpit(unsigned int uFlags) const
{
    if (FirstPerson::IsActive(mCameraId) && bReadyToRender &&
        FLRenderer::GetCurrentViewport() == mCameraId && !mOwner->mTargetInfo.mIsAiming &&
        mObjectModel)
    {
        mObjectModel->Render(&mModelMatrix, 0, RedColor::WHITE, uFlags | 0x80001, 0);
    }
}

// FUNCTION: SWBF2 0x00520b90
void FirstPersonRenderable::RenderDroideka(unsigned int uFlags) const
{
    if (!FirstPerson::IsActive(mCameraId) || !bReadyToRender ||
        HideFirstPersonHands(mCameraId) || IsOwnerAiming() ||
        FLRenderer::GetCurrentViewport() != mCameraId)
    {
        return;
    }

    RedPose pose;
    pose.ConvertFromZephyrPose(mZephyrPoseStatic);

    bool bShadow = RedStencilShadow::GetShadowEnable();
    int iLight = -1;
    if (RedLight::GetGlobalDirectionalLight(0)->IsShadowCaster())
    {
        iLight = 0;
    }
    else if (RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster())
    {
        iLight = 1;
    }
    else
    {
        bShadow = false;
    }

    float fSavedFactor = 0.0f;
    if (bShadow)
    {
        fSavedFactor = RedLight::m_sGlobalDirectionalFactor[iLight];
        const GameObject* pIgnore = mOwner->GetGameObject();
        const PblVector3& vLight = RedLight::GetGlobalDirectionalLight(iLight)->m_vDirection;
        PblVector3 vDir(-vLight.x, -vLight.y, -vLight.z);
        float& fShadowVal = const_cast<FirstPersonRenderable*>(this)->mShadowVal;
        if (CollisionManager::RayTest(mWeaponMatrix.GetTranslation(), vDir, 500.0f, &pIgnore, 1, 0))
        {
            fShadowVal -= fShadowVal * 0.5f;
        }
        else
        {
            fShadowVal += (1.0f - fShadowVal) * 0.5f;
        }
        RedLight::m_sGlobalDirectionalFactor[iLight] = fShadowVal;
    }

    uFlags |= 0x80001;
    if (mObjectModel)
    {
        mObjectModel->Render(&mModelMatrix, &pose, RedColor::WHITE, uFlags, 0);
    }
    Weapon* pWeapon = mOwner->GetWeaponPtr(mOwner->GetActiveWeaponChannel());
    if (pWeapon)
    {
        PblMatrix weaponMatrix(reinterpret_cast<const _D3DXMATRIXA16*>(&mWeaponMatrix));
        pWeapon->Render(weaponMatrix, &pose, RedColor::WHITE, uFlags, true);
    }

    if (bShadow)
    {
        RedLight::m_sGlobalDirectionalFactor[iLight] = fSavedFactor;
    }
}

// FUNCTION: SWBF2 0x00521670
float FirstPersonCameraManager::SetCameraPath(PblVector3* pPos, PblVector3* pPosSlope,
                                              PblVector3* pRot, PblVector3* pRotSlope,
                                              float* pTime, unsigned int uPoints)
{
    FirstPerson::IsActive(0);
    mNumPoints = uPoints;
    float fTotalTime = 0.0f;
    for (unsigned int i = 0; i < uPoints; ++i)
    {
        if (i == 0)
        {
            if (mCurrCamPt != -1)
            {
                float fT = 1.0f - mCurrCamTime / mCamTime[mCurrCamPt];
                mHermPos[0].Set(mHermPos[mCurrCamPt].getValue(fT), pPos[0], mHermPos[mCurrCamPt].getSlope(fT), pPosSlope[0], 1.0f);
                mHermRot[0].Set(mHermRot[mCurrCamPt].getValue(fT), pRot[0], mHermRot[mCurrCamPt].getSlope(fT), pRotSlope[0], 1.0f);
            }
            else
            {
                const PblVector3 vZero(0.0f, 0.0f, 0.0f);
                mHermPos[0].Set(vZero, pPos[0], vZero, pPosSlope[0], 1.0f);
                mHermRot[0].Set(mCurRotVec, pRot[0], vZero, pRotSlope[0], 1.0f);
            }
        }
        else
        {
            mHermPos[i].Set(pPos[i - 1], pPos[i], pPosSlope[i - 1], pPosSlope[i], 1.0f);
            mHermRot[i].Set(pRot[i - 1], pRot[i], pRotSlope[i - 1], pRotSlope[i], 1.0f);
        }
        mCamTime[i] = pTime[i];
        fTotalTime += pTime[i];
    }
    mCurrCamTime = mCamTime[0];
    mCurrCamPt = 0;
    return fTotalTime;
}

// FUNCTION: SWBF2 0x00522220
float FirstPersonCameraManager::RollLeft()
{
    if (!FirstPerson::IsActive(0))
    {
        return 0.0f;
    }
    CamPathInfo* pPath = &FirstPerson::mCameraPaths.mRollLeft;
    return SetCameraPath(pPath->Vec, pPath->Slope, pPath->Ore,
                         pPath->RSlope, pPath->Time, pPath->numPoints);
}

// FUNCTION: SWBF2 0x00522260
float FirstPersonCameraManager::RollRight()
{
    if (!FirstPerson::IsActive(0))
    {
        return 0.0f;
    }
    CamPathInfo* pPath = &FirstPerson::mCameraPaths.mRollRight;
    return SetCameraPath(pPath->Vec, pPath->Slope, pPath->Ore,
                         pPath->RSlope, pPath->Time, pPath->numPoints);
}

// FUNCTION: SWBF2 0x005222a0
float FirstPersonCameraManager::Land()
{
    if (!FirstPerson::IsActive(0))
    {
        return 0.0f;
    }
    CamPathInfo* pPath = &FirstPerson::mCameraPaths.mLand;
    return SetCameraPath(pPath->Vec, pPath->Slope, pPath->Ore,
                         pPath->RSlope, pPath->Time, pPath->numPoints);
}

// FUNCTION: SWBF2 0x005222e0
float FirstPersonCameraManager::ProneToStand()
{
    if (!FirstPerson::IsActive(0))
    {
        return 0.0f;
    }
    mIsBobbing = false;
    mIsReturning = false;
    CamPathInfo* pPath = &FirstPerson::mCameraPaths.mProneToStand;
    return SetCameraPath(pPath->Vec, pPath->Slope, pPath->Ore,
                         pPath->RSlope, pPath->Time, pPath->numPoints);
}

// FUNCTION: SWBF2 0x00522330
float FirstPersonCameraManager::CrouchToStand()
{
    if (!FirstPerson::IsActive(0))
    {
        return 0.0f;
    }
    mIsBobbing = false;
    mIsReturning = false;
    CamPathInfo* pPath = &FirstPerson::mCameraPaths.mCrouchToStand;
    return SetCameraPath(pPath->Vec, pPath->Slope, pPath->Ore,
                         pPath->RSlope, pPath->Time, pPath->numPoints);
}

// FUNCTION: SWBF2 0x00521a90
void FirstPersonCameraManager::ReturnToCenter()
{
    if (!FirstPerson::IsActive(0) || mCurrCamPt == -1)
    {
        return;
    }

    const float fT = 1.0f - mCurrCamTime / mCamTime[mCurrCamPt];
    mLinVec = mHermPos[mCurrCamPt].getValue(fT);
    mCurrCamTime = 0.0f;
    mCurrCamPt = 0;
    mCamTime[0] = 0.3f;
    mNumPoints = 1;
    mIsReturning = true;
}

// FUNCTION: SWBF2 0x00521bb0
void FirstPersonCameraManager::ViewBob(float fScale)
{
    if (!FirstPerson::IsActive(0))
    {
        return;
    }

    PblVector3 vPos[4];
    vPos[0] = PblVector3(-0.0f, fScale * 0.1f, 0.0f);
    vPos[1] = PblVector3(0.0f, -0.0f, 0.0f);
    vPos[2] = PblVector3(0.0f, fScale * 0.1f, 0.0f);
    vPos[3] = PblVector3(0.0f, 0.0f, 0.0f);
    PblVector3 vPosSlope[4];
    vPosSlope[0] = PblVector3(0.1f, 0.1f, 0.0f);
    vPosSlope[1] = PblVector3(0.2f, 0.2f, 0.0f);
    vPosSlope[2] = PblVector3(0.2f, 0.1f, 0.0f);
    vPosSlope[3] = PblVector3(0.0f, 0.0f, 0.0f);
    PblVector3 vRot[4];
    vRot[0] = PblVector3(0.0f, 0.0f, 0.0f);
    vRot[1] = PblVector3(0.0f, 0.0f, 0.0f);
    vRot[2] = PblVector3(0.0f, 0.0f, 0.0f);
    vRot[3] = PblVector3(0.0f, 0.0f, 0.0f);
    mIsBobbing = true;
    PblVector3 vRotSlope[4];
    vRotSlope[0] = PblVector3(0.0f, 0.0f, 0.0f);
    vRotSlope[1] = PblVector3(0.0f, 0.0f, 0.0f);
    vRotSlope[2] = PblVector3(0.0f, 0.0f, 0.0f);
    vRotSlope[3] = PblVector3(0.0f, 0.0f, 0.0f);
    float fTime[4];
    fTime[0] = 0.2f;
    fTime[1] = 0.2f;
    fTime[2] = 0.2f;
    fTime[3] = 0.2f;
    SetCameraPath(vPos, vPosSlope, vRot, vRotSlope, fTime, 4);
}
