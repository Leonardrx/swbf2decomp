#include "FreeCamera.h"

#include <math.h>
#include <d3dx9math.h>

#include "CameraManager.h"
#include "CollisionObject.h"
#include "FLInputManager.h"
#include "FLJoystick.h"
#include "GameLoop.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"

class GameObject;

class CollisionManager
{
public:
    static float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        CollisionObject*& pObject, PblVector3* pvPoint,
                        const GameObject** const ppIgnore, int iNumIgnore, int iMask, bool bFlag);
};

// GLOBAL: SWBF2 0x01e55f6a
bool FreeCamera::sQuakeControl;
// GLOBAL: SWBF2 0x01e55f6b
bool FreeCamera::sStop;
// GLOBAL: SWBF2 0x01e55fa0
bool FreeCamera::sInvertAxis;

const float FreeCamera::sMoveScale = 15.0f;
const float FreeCamera::sTurnScale = 1.5f;
const float FreeCamera::sHeightScale = 3.0f;

// GLOBAL: SWBF2 0x01e55fa2
static bool gIsFollowingObj;
// GLOBAL: SWBF2 0x01e55fa3
static bool gSetTargetObj;
// GLOBAL: SWBF2 0x01e55fa1
static bool gFollowingTethered;
// GLOBAL: SWBF2 0x01e51e27
static bool gSetTetherPosition;

// GLOBAL: SWBF2 0x01e55fa8
static RedOmniLight* s_pFreeCamLight;
// GLOBAL: SWBF2 0x01e55fa4
static int s_pFreeCamLightFreeze;

// GLOBAL: SWBF2 0x01faa478

static CollisionObject* sFollowedObject;
// GLOBAL: SWBF2 0x01faa47c

static bool sResetFollowPosition;
// GLOBAL: SWBF2 0x01faa47d

static bool sResetFollowRotation;

namespace AIDebug
{

// FUNCTION: SWBF2 0x00438670
bool AllowUpdateFreeCamSpeeds()
{
    return true;
}
}

// FUNCTION: SWBF2 0x0052d650
FreeCamera::FreeCamera(CameraManager* pManager, int iView)
    : GameCamera(pManager, iView),
      mPitch(0.0f), mYaw(0.0f), mMultiplier(1.0f), mTurnMultiplier(1.0f),
      mButtonFasterDown(false), mButtonSlowerDown(false), mVelocity(gVectorZero)
{
    mMatrix.Reset();
}

// FUNCTION: SWBF2 0x0052d630
PblVector3 FreeCamera::GetVelocity()
{
    return mVelocity;
}

// SYNTHETIC: SWBF2 0x0052d6b0
// FreeCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052d6e0
void FreeCamera::SetMatrix(const PblMatrix& rMatrix)
{
    mMatrix = rMatrix;
    mPitch = asinf(-mMatrix.m[2][1]);
    mYaw = atan2f(mMatrix.m[2][0], mMatrix.m[2][2]);
}

// FUNCTION: SWBF2 0x0052d770
PblMatrix FreeCamera::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x0052d7b0
void FreeCamera::Update(float fDeltaTime)
{
    // GLOBAL: SWBF2 0x01faa97c
    static PblVector3 sTetherOffset;

    GUIInputs* pGuiInputs = FLInputManager::s_instance.GetPlayerGuiInputs(mView);
    ProcessedInputs* pInputs = FLInputManager::s_instance.GetPlayerInputs(mView);
    if (!pGuiInputs)
    {
        return;
    }

    PblVector3 vOldPos = mMatrix.GetTranslation();

    if (gSetTetherPosition)
    {
        gSetTetherPosition = false;
        if (sFollowedObject)
        {
            const PblMatrix& mat = GetMatrix();
            const PblVector3& vPos = sFollowedObject->GetPosition();
            gFollowingTethered = true;
            sTetherOffset = vPos - mat.GetTranslation();
        }
    }

    if (gSetTargetObj)
    {
        gSetTargetObj = false;
        PblMatrix mat = GetMatrix();
        const PblVector3& vPos = mat.GetTranslation();
        PblVector3 vDir(-mat.m[2][0], -mat.m[2][1], -mat.m[2][2]);
        PblVector3 vHit(0.0f, 1.0f, 0.0f);
        CollisionObject* pObject;
        CollisionManager::RayHit(vPos, vDir, 500.0f, pObject, &vHit, NULL, 0, 0, false);
        if (pObject)
        {
            sFollowedObject = pObject;
            gIsFollowingObj = true;
        }
    }

    if (gIsFollowingObj && sFollowedObject)
    {
        if (gFollowingTethered)
        {
            mMatrix[3] = sFollowedObject->GetPosition() - sTetherOffset;
            mMatrix.m[3][3] = 1.0f;
        }

        PblMatrix mat = GetMatrix();
        PblVector3 vAt = sFollowedObject->GetPosition() + PblVector3(0.0f, sFollowedObject->GetCollisionRadius() * 0.5f, 0.0f);
        PblVector3 vZ = Normalize(mat.GetTranslation() - vAt);
        mat[2] = vZ;
        mat.m[2][3] = 0.0f;

        PblVector3 vUp(0.0f, 1.0f, 0.0f);
        PblVector3 vX = Normalize(CrossProduct(vUp, vZ));
        mat[0] = vX;
        mat.m[0][3] = 0.0f;

        PblVector3 vY = Normalize(CrossProduct(mat[2], vX));
        mat[1] = vY;
        mat.m[1][3] = 0.0f;

        SetMatrix(mat);
        return;
    }

    if (!GameLoop::IsGameOver() && !sStop)
    {
        UpdateControlSpeed(*pGuiInputs);
        UpdateOrientation(*pInputs, fDeltaTime);
        UpdateTranslation(*pInputs, fDeltaTime);
    }

    if (fDeltaTime > 0.0f)
    {
        mVelocity = (mMatrix.GetTranslation() - vOldPos) / fDeltaTime;
    }

    if (sResetFollowRotation || sResetFollowPosition)
    {
        sResetFollowPosition = false;
        sResetFollowRotation = false;
    }

    if (s_pFreeCamLight)
    {

        PblMatrix mat = GetMatrix();
        const PblVector3& vPos = mat.GetTranslation();
        PblVector3 vDir(-mat.m[2][0], -mat.m[2][1], -mat.m[2][2]);
        CollisionObject* pObject;
        float fDist = CollisionManager::RayHit(vPos, vDir, 20.0f, pObject, NULL, NULL, 0, 0x90, false) * 20.0f - 0.75f;
        PblVector3 vLightPos = vPos - mat[2] * fDist;
        if (!s_pFreeCamLightFreeze)
        {
            s_pFreeCamLight->SetPosition(vLightPos);
        }
        RedDebug::DrawSphere(vLightPos, 0.1f, RedColor::YELLOW);
    }
}

// FUNCTION: SWBF2 0x0052dd80
void FreeCamera::UpdateControlSpeed(const GUIInputs& rInputs)
{
    if (!AIDebug::AllowUpdateFreeCamSpeeds())
    {
        return;
    }

    if (mButtonFasterDown && !rInputs.IsFreeEyeMoveFasterButtonDown())
    {
        mMultiplier *= 1.25f;
        mButtonFasterDown = false;
    }
    else if (rInputs.IsFreeEyeMoveFasterButtonDown())
    {
        mButtonFasterDown = true;
    }

    if (mButtonSlowerDown && !rInputs.IsFreeEyeMoveSlowerButtonDown())
    {
        mMultiplier *= 0.8f;
        mButtonSlowerDown = false;
    }
    else if (rInputs.IsFreeEyeMoveSlowerButtonDown())
    {
        mButtonSlowerDown = true;
    }

    if (mTurnButtonFasterDown && !rInputs.IsFreeEyeTurnFasterButtonDown())
    {
        mTurnMultiplier *= 1.25f;
        mTurnButtonFasterDown = false;
    }
    else if (rInputs.IsFreeEyeTurnFasterButtonDown())
    {
        mTurnButtonFasterDown = true;
    }

    if (mTurnButtonSlowerDown && !rInputs.IsFreeEyeTurnSlowerButtonDown())
    {
        mTurnMultiplier *= 0.8f;
        mTurnButtonSlowerDown = false;
    }
    else if (rInputs.IsFreeEyeTurnSlowerButtonDown())
    {
        mTurnButtonSlowerDown = true;
    }
}

// FUNCTION: SWBF2 0x0052dea0
void FreeCamera::UpdateOrientation(const ProcessedInputs& rInputs, float fDeltaTime)
{
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(mView);
    float fMouseY = pJoystick->GetMouseY();
    if (sInvertAxis)
    {
        fMouseY = -fMouseY;
    }

    float fTurn = mTurnMultiplier * fDeltaTime * sTurnScale;
    mYaw -= pJoystick->GetMouseX() * fTurn;
    mPitch -= fTurn * fMouseY;

    PblMatrix mRot;
    mRot.SetRotateX(mPitch);
    mRot.RotateY(mYaw);

    for (int i = 0; i < 3; i++)
    {
        mMatrix[i] = mRot[i];
        mMatrix.m[i][3] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x0052dfa0
void FreeCamera::UpdateTranslation(const ProcessedInputs& rInputs, float fDeltaTime)
{
    GUIInputs* pGuiInputs = FLInputManager::s_instance.GetPlayerGuiInputs(mView);
    int iHeight = (int)pGuiInputs->IsFreeEyeUpButtonDown() - (int)pGuiInputs->IsFreeEyeDownButtonDown();
    float fHeight = (float)iHeight * sHeightScale;

    PblVector3 vMove(rInputs.GetStrafeAxis() * sMoveScale,
                     fHeight,
                     -rInputs.GetMoveAxis() * sMoveScale);

    PblMatrix mRotY;
    const D3DXMATRIX* pMatrix;
    if (sQuakeControl)
    {
        pMatrix = (const D3DXMATRIX*)&mMatrix;
    }
    else
    {
        mRotY.SetRotateY(mYaw);
        pMatrix = reinterpret_cast<const D3DXMATRIX*>(&mRotY);
    }

    PblVector3 vOut;
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vOut),
                            reinterpret_cast<const D3DXVECTOR3*>(&vMove), pMatrix);
    vMove = vOut;

    float fScale = mMultiplier * fDeltaTime;
    vMove *= fScale;
    mMatrix[3] = mMatrix[3] + vMove;
    mMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0052e0e0
void FreeCamera::SetupCamera(RedCamera* pCamera, float fDeltaTime)
{
    pCamera->SetMatrix(mMatrix);
    pCamera->SetZoom(1.0f);
}

// FUNCTION: SWBF2 0x0052e110
void FreeCamera::SetupIconVisibility(int iView)
{
    mManager->m_uiVisibleIconMask[iView] = 0;
}
