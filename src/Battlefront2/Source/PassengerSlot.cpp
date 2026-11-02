#include <d3dx9math.h>
#include "PassengerSlot.h"
#include "PebbleFL/Common/PblVector.h"

#include "NetGame.h"
#include "PlayerController.h"
#include "UnitController.h"
#include "UnitStationaryController.h"
#include "AILowLevel.h"
#include "GameObject.h"
#include "Character.h"
#include "Tracker.h"
#include "FirstPerson.h"
#include "PowerupUtility.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"


// SYNTHETIC: SWBF2 0x00405da0
// `dynamic initializer for 'PassengerSlot::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769290
// `dynamic atexit destructor for 'PassengerSlot::sMemoryPool''

// GLOBAL: SWBF2 0x01ef7ca0
// PassengerSlot::sMemoryPool
MemoryPool PassengerSlot::sMemoryPool("PassengerSlot", sizeof(PassengerSlot));

// SYNTHETIC: SWBF2 0x00405dc0
// `dynamic initializer for 'PassengerSlot::rttiHashPassengerSlot''
// PassengerSlot::rttiHashPassengerSlot

// GLOBAL: SWBF2 0x01ef7c90
// PassengerSlot::rttiHashPassengerSlot
PblHash PassengerSlot::rttiHashPassengerSlot("PassengerSlot");
// GLOBAL: SWBF2 0x0078aabc
// PassengerSlot::rttiNamePassengerSlot
const char PassengerSlot::rttiNamePassengerSlot[] = "PassengerSlot";



// FUNCTION: SWBF2 0x0060fd00
PassengerSlotClass::PassengerSlotClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mCameraSettings.mCameraMoveTensionL = 50.0f;
    mCameraSettings.mCameraMoveTensionR = 50.0f;
    mCameraSettings.mCameraMoveTensionU = 50.0f;
    mCameraSettings.mCameraMoveTensionD = 50.0f;
    mCameraSettings.mCameraMoveTensionF = 50.0f;
    mCameraSettings.mCameraMoveTensionB = 50.0f;
    mCameraSettings.mCameraAimTension = 50.0f;
    mAddHealth = 15.0f;
    mAddAmmo = 1.0f;
    mPowerupDelay = 1.0f;
    mMountMatrix = gMatrixIdentity;
}

// SYNTHETIC: SWBF2 0x0060fde0
// PassengerSlotClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0060fe10
EntityClass* PassengerSlotClass::Derive(unsigned int uHash)
{
    return 0;
}

// FUNCTION: SWBF2 0x0060fe20
Entity* PassengerSlotClass::Build(const EntityDesc& desc)
{
    return 0;
}

// FUNCTION: SWBF2 0x0060fe30
bool PassengerSlotClass::SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel)
{
    if (uProperty != 0xBFBBAB62)
    {
        if (!mCameraSettings.SetProperty(uProperty, pszValue))
        {
            return ControllableClass::SetProperty(uProperty, pszValue, pModel);
        }
        return true;
    }
    if (pModel)
    {
        PblMatrix mat;
        unsigned int uBone = PblTEMPHash(pszValue);
        if (pModel->GetParentBoneAndOffsetMatrix(uBone, mat))
        {
            mMountMatrix = mat;
        }
    }
    return true;
}

float GetDefaultFOV();

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// FUNCTION: SWBF2 0x0060fab0
void PassengerSlot::ActivatePhysics()
{
    ActivateThread(-20, PER_TURN);
}

// FUNCTION: SWBF2 0x0060fac0
void PassengerSlot::DeactivatePhysics()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x0060fad0
bool PassengerSlot::UpdateIndirect(float fDeltaTime)
{
    UnitControllerPtr(mCtrl)->mLowLevel.UpdateIndirect(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x0060faf0
void PassengerSlot::SetupController()
{
    Controller* pController = 0;
    if (mPlayerId >= 0)
    {
        pController = new PlayerController(this);
        Track(mPilot->mPlayerId);
    }
    else if (mPilot && !(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        pController = new UnitStationaryController(this);
    }
    if (pController)
    {
        AddController(pController, mPilot->mPlayerId < 0);
    }
    ClearControllable();
}

// FUNCTION: SWBF2 0x0060fc20
CameraTrackSetting* PassengerSlot::GetCameraTrackSettings()
{
    return &mClass->mCameraSettings;
}

// FUNCTION: SWBF2 0x0060fc30
void PassengerSlot::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    PblVector3 vDir(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z);
    rMatrix = DirectionMatrix(vDir, mAimMatrix.GetYAxis(), mEyePoint);
}

// FUNCTION: SWBF2 0x0060fcd0
void PassengerSlot::SetupCameraZoomFactor(float& rZoom)
{
    rZoom = 1.0f;
}

// FUNCTION: SWBF2 0x0060fce0
void PassengerSlot::SetupCameraFOV(float& rFOV)
{
    rFOV = GetDefaultFOV();
}

// FUNCTION: SWBF2 0x0060f560
PassengerSlot::PassengerSlot(GameObject* pParent, PassengerSlotClass* pClass, RedPose*)
    : EntityEx(0, pClass), Controllable(pClass ? static_cast<ControllableClass*>(pClass) : 0, -1),
      mPowerupTimer(0.0f), mParent(pParent), mClass(pClass)
{
    mAimMatrix = mClass->mMountMatrix * mParent->GetMatrix();
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mAimMatrix);
    mEyeDir = mAimMatrix.GetZAxis();
}

// SYNTHETIC: SWBF2 0x0060f530
// PassengerSlot::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x0060ff08
// ??_EPassengerSlot@@WCE@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0060ff10
// ??_EPassengerSlot@@WM@AEPAXI@Z
// FUNCTION: SWBF2 0x0060f6a0
PassengerSlot::~PassengerSlot()
{
}



// FUNCTION: SWBF2 0x0060f490
unsigned int PassengerSlot::GetDerivedRtti() const { return rttiHashPassengerSlot; }

// FUNCTION: SWBF2 0x0060f4a0
const char* PassengerSlot::GetDerivedRttiName() const { return rttiNamePassengerSlot; }

// SYNTHETIC: SWBF2 0x0060fef8
// ?GetEntity@PassengerSlot@@WM@AEPAVEntity@@XZ
// FUNCTION: SWBF2 0x0060f4b0
Entity* PassengerSlot::GetEntity() { return this; }

// SYNTHETIC: SWBF2 0x0060ff00
// ?GetEntity@PassengerSlot@@WM@BEPBVEntity@@XZ
// FUNCTION: SWBF2 0x0060f4c0
const Entity* PassengerSlot::GetEntity() const { return this; }

// SYNTHETIC: SWBF2 0x0060fee8
// ?GetGameObject@PassengerSlot@@WCE@AEPAVGameObject@@XZ
// FUNCTION: SWBF2 0x0060f4d0
GameObject* PassengerSlot::GetGameObject() { return mParent; }

// SYNTHETIC: SWBF2 0x0060fef0
// ?GetGameObject@PassengerSlot@@WCE@BEPBVGameObject@@XZ
// FUNCTION: SWBF2 0x0060f4e0
const GameObject* PassengerSlot::GetGameObject() const { return mParent; }

// FUNCTION: SWBF2 0x0060f4f0
Trackable* PassengerSlot::GetTrackable() { return this; }

// FUNCTION: SWBF2 0x0060f500
const Controllable* PassengerSlot::GetControllable() const { return this; }

// FUNCTION: SWBF2 0x0060f510
Controllable* PassengerSlot::GetControllable() { return this; }

// FUNCTION: SWBF2 0x0060f520
const PblMatrix& PassengerSlot::GetMatrix() const { return mAimMatrix; }

// FUNCTION: SWBF2 0x0060f700
void PassengerSlot::Init() {}

// FUNCTION: SWBF2 0x0060f780
ControllableClass* PassengerSlot::GetControllableClass()
{
    return mClass ? static_cast<ControllableClass*>(mClass) : 0;
}

// FUNCTION: SWBF2 0x0060f760
const ControllableClass* PassengerSlot::GetControllableClass() const
{
    return mClass ? static_cast<const ControllableClass*>(mClass) : 0;
}

// FUNCTION: SWBF2 0x0060f710
void PassengerSlot::Kill()
{
    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = NetGame::GetJoystickIndex(mPlayerId);
        if (iJoystick >= 0)
        {
            FirstPerson::Activate(iJoystick, false);
            FirstPerson::Update(iJoystick, TYPE_COCKPIT, -2, 0, 0);
        }
    }
    Controllable::Kill();
}

// FUNCTION: SWBF2 0x0060f7a0
bool PassengerSlot::Update(float fDeltaTime)
{
    Controllable::Update(fDeltaTime);

    Controllable* pPilot = GetPilot();

    if (pPilot)
    {
        if (mControlView.IsPressed() && !IsForcedThirdPerson() && !IsForcedFirstPerson() &&
            mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
        {
            SwitchControlView(mPlayerId);
        }

        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            if (mControlUse.IsPressed() || mParent->mDisableTime > 0.0f)
            {
                if (IsRemoteVehicle())
                {
                    pPilot->ExitRemote();
                }
                else if (IsPilotable())
                {
                    if (pPilot->mCharacter && pPilot->mCharacter->mRemote)
                    {
                        pPilot->ExitRemote();
                    }
                    pPilot->ExitVehicle(true, true);
                }
            }
            else if (mParent->GetControllableList().GetCount() > 1 && mControlSwitch[0] != 0.0f)
            {
                pPilot->CyclePosition(mControlSwitch[0] > 0.0f);
            }
        }
    }

    mAimMatrix = mClass->mMountMatrix * mParent->GetMatrix();
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mAimMatrix);
    mEyeDir = mAimMatrix.GetZAxis();
    mPowerupTimer += fDeltaTime;
    if (mPowerupTimer >= mClass->mPowerupDelay)
    {
        mPowerupTimer -= mClass->mPowerupDelay;
        float fAddHealth = mClass->mAddHealth;
        float fAddAmmo = mClass->mAddAmmo;
        if (pPilot)
        {
            GameObject* pPilotObject = mPilot->GetGameObject();
            if (pPilotObject)
            {
                PowerupUtility::ApplyHealth(pPilotObject, fAddHealth, 0, false);
                PowerupUtility::ApplyAmmo(pPilotObject, fAddAmmo, false);
            }
        }
    }
    if (pPilot)
    {
        mClass->UpdatePilotAnimation(pPilot, 0, mParent->GetMatrix(), mParent->GetVelocity(),
                                     fDeltaTime, -1.0f, -1.0f, -1.0f);
    }
    return true;
}
