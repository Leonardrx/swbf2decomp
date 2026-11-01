#include "FLInputManager.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Interface/RedFont.h"

#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/input/pc/pcRedInputManager.h"
#include "lglcd/lgLcd.h"
#include "PauseMenu.h"
#include "PlayerProfile.h"

#include <string.h>
#include "FLGuiManager.h"
#include "FLJoystick.h"
#include "LuaHelper.h"

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "HUDObjectiveList.h"
#include "NetGame.h"
#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "NetState.h"
#include "EnergyBar.h"
#include "PebbleFL/Common/PblUnicode.h"

namespace PblBuildType
{
extern bool gDemoBuild;
}

// SYNTHETIC: SWBF2 0x004039d0
// `dynamic initializer for 'FLInputManager::s_instance''
// SYNTHETIC: SWBF2 0x007687a0
// `dynamic atexit destructor for 'FLInputManager::s_instance''

// GLOBAL: SWBF2 0x01ebdc50
// FLInputManager::s_instance
FLInputManager FLInputManager::s_instance;

// FUNCTION: SWBF2 0x00529ef0
FLInputManager::FLInputManager()
{
    memset(m_HWPort_Full, -1, sizeof(m_HWPort_Full));
    memset(m_bHudGroupsEnabled, 0, sizeof(m_bHudGroupsEnabled));
    mChatText[0] = 0;
    mBindAll = false;
    m_HWPort[0] = 0;
    mPrimaryPort = 0;
    mLCDPageToggle[0] = false;
    mTextTimer = 0.0f;
    mPageTimer = 0.0f;
    mPageIndex = 0;
    mIgnoreMissing = false;
    mAutoAcquire = false;
    mReadAllControllers = false;
    mAllowBindMultiple = true;
    m_MissingPort = -1;
    m_bPausedByInput = false;
    mSecondaryPort = -1;
    mShadowPrimary = -1;
    mShadowSecondary = -1;
    mActiveControllers[0] = true;
}

// FUNCTION: SWBF2 0x00529b30
FLInputManager::~FLInputManager()
{
}

// FUNCTION: SWBF2 0x0052a190
RawControllerInputs* FLInputManager::GetPlayerJoystick(int iPlayer)
{
    if (iPlayer >= 0 && iPlayer < 1)
    {
        return &mJoysticks[iPlayer];
    }

    return 0;
}

unsigned short __cdecl RawInputToActionKey(unsigned short usRawInput);

struct CustomGlyphData
{
    wchar_t unicodeValue;
    unsigned int textureHash;
};

// SYNTHETIC: SWBF2 0x004039f0
// `dynamic initializer for 'gCustomGlyphData''

// GLOBAL: SWBF2 0x007ec140
static CustomGlyphData gCustomGlyphData[7] =
{
    { 0x7fff, 0x7f0688e4u },
    { 0x8100, 0x592f1c52u },
    { 0x8101, 0x7f0688e4u },
    { 0x8200, 0x00288768u },
    { 0x8201, 0xee286b12u },
    { (wchar_t)(RawInputToActionKey('B') + 0x8000), 0xDD7D0382 },
    { (wchar_t)(RawInputToActionKey('C') + 0x8000), 0xDD7D0382 },
};

// FUNCTION: SWBF2 0x00529de0
void FLInputManager::InitCustomFontData()
{
    CustomFontData& d = RedFont::GetMagicCharFontData();

    d.nameHash_ = PblHash("SpecialFont");
    d.numGlyphs_ = 7;
    d.numTextures_ = 7;
    d.glyphHeight_ = 0x20;

    d.glyphs_ = new RedFont::Glyph[d.numGlyphs_ + 64];
    d.textureHashes_ = new unsigned int[d.numTextures_ + 64];

    d.defaultMap_.Reallocate(d.numGlyphs_ * 2);

    for (unsigned int i = 0; i < d.numGlyphs_; i++)
    {
        d.glyphs_[i].value = gCustomGlyphData[i].unicodeValue;
        d.textureHashes_[i] = gCustomGlyphData[i].textureHash;
        d.defaultMap_[d.textureHashes_[i]] = d.glyphs_[i].value;
    }

    for (unsigned int i = 0; i < d.numGlyphs_; i++)
    {
        d.glyphs_[i].texnum = (unsigned char)(0x80 + i);
        d.glyphs_[i].width = 0x32;
    }
}

struct ProcessedInputMap
{
    unsigned int hashID;
    ePROCESSEDINPUT_TYPE input;
};

// GLOBAL: SWBF2 0x007e67d0
static const ProcessedInputMap gProcessedInputMap[0x22] =
{
    { 0x9e5f87d9u,  ePROCESSEDINPUT_primaryFireButtonDown },
    { 0x880ddb41u,  ePROCESSEDINPUT_secondaryFireButtonDown },
    { 0x0fd525a1u,  ePROCESSEDINPUT_sprintButtonDown },
    { 0x2c4caa39u,  ePROCESSEDINPUT_jumpButtonDown },
    { 0x48d1dc33u,  ePROCESSEDINPUT_crouchButtonDown },
    { 0x20b0229au,  ePROCESSEDINPUT_zoomButtonDown },
    { 0x9ddbe9ccu,  ePROCESSEDINPUT_viewButtonDown },
    { 0x9b656248u,  ePROCESSEDINPUT_reloadButtonDown },
    { 0xbfe61030u,  ePROCESSEDINPUT_useButtonPressed },
    { 0xf85071beu,  ePROCESSEDINPUT_squadCommandButtonPressed },
    { 0xac98902du, ePROCESSEDINPUT_acceptHeroPressed },
    { 0xcd1022bdu, ePROCESSEDINPUT_declineHeroPressed },
    { 0x1ed91b8bu, ePROCESSEDINPUT_lockTargetButtonPressed },
    { 0xc00bacc0u, ePROCESSEDINPUT_primaryNextButtonPressed },
    { 0x287ca200u, ePROCESSEDINPUT_primaryPrevButtonPressed },
    { 0x55cacad8u, ePROCESSEDINPUT_secondaryNextButtonPressed },
    { 0x356988b8u, ePROCESSEDINPUT_secondaryPrevButtonPressed },
    { 0x0b3b6548u, ePROCESSEDINPUT_playerList },
    { 0xa24bf9abu, ePROCESSEDINPUT_chat },
    { 0x20a43ce3u, ePROCESSEDINPUT_friendChat },
    { 0x43a268abu, ePROCESSEDINPUT_Comm_Chat_0 },
    { 0x42a26718u, ePROCESSEDINPUT_Comm_Chat_1 },
    { 0x45a26bd1u, ePROCESSEDINPUT_Comm_Chat_2 },
    { 0x44a26a3eu, ePROCESSEDINPUT_Comm_Chat_3 },
    { 0x47a26ef7u, ePROCESSEDINPUT_Comm_Chat_4 },
    { 0x46a26d64u, ePROCESSEDINPUT_Comm_Chat_5 },
    { 0x49a2721du, ePROCESSEDINPUT_Comm_Chat_6 },
    { 0x48a2708au, ePROCESSEDINPUT_Comm_Chat_7 },
    { 0xdfa2efb1u, ePROCESSEDINPUT_Map },
    { 0xc3b9429eu, ePROCESSEDINPUT_TakeScreenshot },
    { 0x82feda2eu, ePROCESSEDINPUT_STRAFE_AXIS },
    { 0xbd8888f0u, ePROCESSEDINPUT_MOVE_AXIS },
    { 0x9537fa8au, ePROCESSEDINPUT_TURN_AXIS },
    { 0xf8b15dcfu, ePROCESSEDINPUT_PITCH_AXIS },
};

// FUNCTION: SWBF2 0x00529c10
wchar_t GetRawFromProcessed_(ePROCESSEDINPUT_TYPE uiAction)
{
    RawControllerInputs* pBindings =
        FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    const ActionKey key = pBindings->GetCurKeyBinding(pBindings->GetInputMode(), uiAction);
    wchar_t charcode = key.mKey[0];

    if (charcode > 0x100)
    {
        charcode = (wchar_t)(charcode + 0x8000);
    }
    else
    {
        RedKeyboard* const keyboard = RedInputManager::GetKeyboard(0);
        if (keyboard)
        {
            charcode = keyboard->pcGetAsciiValue(charcode);
        }
    }

    return charcode;
}

// FUNCTION: SWBF2 0x00529ca0
void InitMagicCharMap(void)
{
    util::HashTable<unsigned short, unsigned int, util::NewAllocator>& rMap =
        *RedTextElement::GetMagicCharMap();

    rMap[0xE70B21B8u] = 0x8100;
    rMap[0x1E8375FAu] = 0x8101;

    for (unsigned int i = 0; i < 0x22; i++)
    {
        rMap[gProcessedInputMap[i].hashID] = GetRawFromProcessed_(gProcessedInputMap[i].input);
    }
}

// FUNCTION: SWBF2 0x0052a080
void FLInputManager::Init(void)
{
    const unsigned char ucPorts = (unsigned char)RedInputManager::GetNumJoystickPorts();

    for (unsigned char i = 0; i < ucPorts; i++)
    {
        if (RedInputManager::GetJoystick(i)->m_connected)
        {
            s_instance.mJoysticks[0]._redJoystick[i] = RedInputManager::GetJoystick(i);
        }
        else
        {
            s_instance.mJoysticks[0]._redJoystick[i] = 0;
        }
    }

    s_instance.mJoysticks[0].mProcessAfterReading = true;
    s_instance.mJoysticks[0].mProcessDestination = s_instance.m_inputs;
    s_instance.mJoysticks[0].m_pGuiInputs = s_instance.m_guiInputs;
    s_instance.m_bIsMissing[0] = false;

    s_instance.mJoysticks[0].CloseChatMode();
    InitMagicCharMap();

    RedLogitechLCD* pPanel = RedInputManager::GetLCDPanel(0);
    if (pPanel && pPanel->mOpened)
    {
        pPanel->ClearLCD();
        pPanel->DrawLCDText("Star Wars", 0x37, 4);
        pPanel->DrawLCDText("Battlefront II", 0x32, 0x14);
        pPanel->PresentLCD();
    }
}

// FUNCTION: SWBF2 0x00529ae0
GUIInputs::GUIInputs()
{
    memset(mGUIInputs, 0, sizeof(mGUIInputs));
    memset(mLastGUIInputs, 0, sizeof(mLastGUIInputs));
    memset(mProcessedGUIInputs, 0, sizeof(mProcessedGUIInputs));
}

// FUNCTION: SWBF2 0x00529b20
GUIInputs::~GUIInputs()
{
}

// FUNCTION: SWBF2 0x0052a150
void FLInputManager::Reset()
{
    s_instance.mReadAllControllers = false;
    s_instance.m_bPausedByInput = false;
}

// FUNCTION: SWBF2 0x0052a160
int FLInputManager::GetBindCount() const
{
    int iCount = 0;
    for (int i = 0; i < 1; i++)
    {
        if (mActiveControllers[i])
        {
            iCount++;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x0052a180
void FLInputManager::RescanForMissing()
{
    m_MissingPort = -1;
}

// FUNCTION: SWBF2 0x0052a1c0
ProcessedInputs* FLInputManager::GetPlayerInputs(int iPlayer)
{
    if (iPlayer >= 0 && iPlayer < 1)
    {
        return &m_inputs[iPlayer];
    }

    return 0;
}

// FUNCTION: SWBF2 0x0052a1e0
GUIInputs* FLInputManager::GetPlayerGuiInputs(int iPlayer)
{
    if (iPlayer >= 0 && iPlayer < 1)
    {
        return &m_guiInputs[iPlayer];
    }

    return 0;
}

// FUNCTION: SWBF2 0x0052a210
bool FLInputManager::IsControllerBound(int iPlayer) const
{
    if (iPlayer >= 0 && iPlayer < 1)
    {
        return mActiveControllers[iPlayer];
    }

    return false;
}

// FUNCTION: SWBF2 0x0052a230
void FLInputManager::ProcessChatMessageInput(const unsigned short* pwText)
{
    mTextTimer = 6.0f;
    PblUnicode::ustrlcpy(mChatText, pwText, 0x200);
}

// FUNCTION: SWBF2 0x0052a260
bool CanProcessChatMessage(void)
{
    const RedLogitechLCD* pPanel = FLInputManager::s_instance.mJoysticks[0].mLCDPanel;
    return pPanel && pPanel->mOpened;
}

// GLOBAL: SWBF2 0x009cef76
bool gDemoBindControllers;

namespace ObjectiveManager
{
HUD::ObjectiveList* GetObjectiveList();
}


// FUNCTION: SWBF2 0x0052a280
bool FLInputManager::Update(float fDeltaTime)
{
    // GLOBAL: SWBF2 0x01faa474
    static int sDelay;

    BindPlayerInput();
    RedInputManager::Update(fDeltaTime);

    if (mAutoAcquire)
    {
        for (unsigned char i = 0; i < RedInputManager::GetNumJoystickPorts(); i++)
        {
            if (RedInputManager::GetJoystick(i)->m_connected && i < 1)
            {
                mActiveControllers[i] = 1;
                m_HWPort[i] = i;
                if (mPrimaryPort < 0)
                {
                    mPrimaryPort = i;
                }
                if (mSecondaryPort < 0 && mPrimaryPort != i)
                {
                    mSecondaryPort = i;
                }
            }
        }
    }

    if (PblBuildType::gDemoBuild && gDemoBindControllers)
    {
        for (unsigned char i = 0; i < RedInputManager::GetNumJoystickPorts(); i++)
        {
            if (RedInputManager::GetJoystick(i)->m_connected && i < 1)
            {
                mActiveControllers[i] = 1;
            }
        }
    }

    for (int iPlayer = 0; iPlayer < 1; iPlayer++)
    {
        mJoysticks[iPlayer].ReadLatest(fDeltaTime);

        float& fTextTimer = mTextTimer;
        fTextTimer -= fDeltaTime;
        if (fTextTimer < 0.0f)
        {
            fTextTimer = 0.0f;
        }
        float& fPageTimer = mPageTimer;
        fPageTimer -= fDeltaTime;
        if (fPageTimer < 0.0f)
        {
            fPageTimer = 0.0f;
        }
        int& iPageIndex = mPageIndex;

        RedLogitechLCD* pLcd = s_instance.mJoysticks[0].mLCDPanel;
        if (!pLcd || !pLcd->mOpened)
        {
            continue;
        }

        const int iLocal = NetGame::GetLocalIndex(0);
        Character* pCharacter = 0;
        if (iLocal >= 0 && iLocal < 64)
        {
            pCharacter = Character::GetPtr(iLocal);
        }

        if (pCharacter)
        {
            Controllable* pControllable = pCharacter->GetControllable();
            if (pControllable)
            {
                if ((pLcd->mSoftButtons & 1) && !(pLcd->mPrevSoftButtons & 1))
                {
                    mLCDPageToggle[0] = !mLCDPageToggle[0];
                }

                if ((pLcd->mSoftButtons & 2) && !(pLcd->mPrevSoftButtons & 2))
                {
                    if (fPageTimer <= 0.0f)
                    {
                        iPageIndex = 0;
                    }
                    else
                    {
                        iPageIndex++;
                    }
                    fPageTimer = 4.0f;
                }

                if (fPageTimer > 0.0f)
                {
                    HUD::ObjectiveList* pList = ObjectiveManager::GetObjectiveList();
                    if (pList && pList->GetOwner())
                    {
                        pList->BeginIter();
                        int iCount = 0;
                        const int iTeam = pList->GetOwner()->mTeamNumber;
                        HUD::Objective* pObjective;
                        for (pObjective = pList->GetCurrentIter(); pObjective;
                             pList->IncrementIter(), pObjective = pList->GetCurrentIter())
                        {
                            if (pObjective->_state == HUD::Objective::ACTIVE &&
                                pObjective->_team == iTeam)
                            {
                                if (iCount == iPageIndex)
                                {
                                    break;
                                }
                                iCount++;
                            }
                        }

                        if (pObjective)
                        {
                            pLcd->DrawLCDWrappedText(pObjective->_textElement.m_text.GetData());
                        }
                        else
                        {
                            fPageTimer = 0.0f;
                        }
                    }
                    continue;
                }

                for (int iChannel = 0, y = 2; y < 30; iChannel++, y += 14)
                {
                    const int iWeapon = pControllable->GetWeaponIndex(iChannel);
                    if (iWeapon < 0)
                    {
                        continue;
                    }
                    const Weapon* pWeapon = pControllable->GetWeaponPtr(iWeapon);
                    if (!pWeapon)
                    {
                        continue;
                    }

                    const WeaponClass* pClass = pWeapon->mClass;
                    if (!pClass->mLabel)
                    {
                        continue;
                    }

                    const float fRounds = (float)pClass->mAmmoCounterClass.mRoundsPerClip;

                    const AmmoCounter* pAmmo = pWeapon->m_pAmmoCounter;
                    const float fNumClips = pAmmo->GetNumClips();

                    const bool bNoClips =
                        fRounds >= (float)INT_MAX || pClass->mAmmoCounterClass.mAmmoPerRound == 0.0f;
                    const bool bInfinite = fNumClips == FLT_MAX;

                    int iClip;
                    int iTotal;
                    if (bNoClips)
                    {
                        iClip = -1;
                        iTotal = -1;
                    }
                    else
                    {
                        const float fCurClip = pAmmo->mCurClip;
                        iClip = (int)(fCurClip * fRounds);
                        iTotal = bInfinite ? -1 : (int)((fCurClip + fNumClips) * fRounds);
                    }

                    char acBuffer[32];
                    if (iClip == -1)
                    {
                        if (iTotal == -1)
                        {
                            sprintf(acBuffer, "* : ");
                        }
                        else
                        {
                            sprintf(acBuffer, "%d : ", iTotal);
                        }
                    }
                    else if (iTotal == -1)
                    {
                        sprintf(acBuffer, "%d / * : ", iClip);
                    }
                    else
                    {
                        sprintf(acBuffer, "%d / %d : ", iClip, iTotal);
                    }

                    unsigned short awLine[256];
                    PblUnicode::ustrncnv(awLine, acBuffer, 256);
                    PblUnicode::ustrlcat(awLine, pClass->mLabel, 256);
                    pLcd->DrawLCDWideText(awLine, 5, y);
                }

                GameObject* pObject = pControllable->GetGameObject();
                if (pObject)
                {
                    const Damageable* pDamageable = static_cast<const Damageable*>(pObject);
                    const int iWidth = pLcd->mWidth;

                    if (pDamageable->GetMaxShield() > 0.0f)
                    {
                        const int iLen = (int)((float)iWidth * pDamageable->GetCurShield() /
                                               pDamageable->GetMaxShield());
                        if (iLen > 0)
                        {
                            pLcd->DrawLCDLine(0, 32, iLen - 1, 32);
                        }
                    }

                    if (pDamageable->GetMaxHealth() > 0.0f)
                    {
                        const int iLen = (int)((float)iWidth * pDamageable->mCurHealth /
                                               pDamageable->GetMaxHealth());
                        if (iLen > 0)
                        {
                            pLcd->DrawLCDLine(0, 35, iLen - 1, 35);
                            pLcd->DrawLCDLine(0, 36, iLen - 1, 36);
                            pLcd->DrawLCDLine(0, 37, iLen - 1, 37);
                        }
                    }

                    const EnergyBar* pBar = pControllable->GetEnergyBar();
                    if (pBar)
                    {
                        const float fMax = pBar->GetEnergyMax();
                        if (fMax > 0.0f)
                        {
                            const int iLen =
                                (int)((float)iWidth * pBar->GetEnergy() / fMax);
                            if (iLen > 0)
                            {
                                pLcd->DrawLCDLine(0, 40, iLen - 1, 40);
                                pLcd->DrawLCDLine(0, 41, iLen - 1, 41);
                            }
                        }
                    }
                    continue;
                }
            }

            if (fTextTimer > 0.0f)
            {
                pLcd->DrawLCDWrappedText(mChatText);
                continue;
            }
        }

        pLcd->DrawLCDText("Star Wars", 55, 4);
        pLcd->DrawLCDText("Battlefront II", 50, 20);
    }

    m_rumble[0].Update(fDeltaTime);

    if ((++sDelay & 7) == 0)
    {
        InitMagicCharMap();
    }

    return true;
}

// FUNCTION: SWBF2 0x0052a890
void FLInputManager::UnbindController(int iPlayer)
{
    iPlayer--;

    if (iPlayer >= 0 && iPlayer < 1)
    {
        mActiveControllers[iPlayer] = 0;
        m_HWPort[iPlayer] = -1;
        m_HWPort_Full[iPlayer] = -1;

        mJoysticks[iPlayer].ClearInputsToOff();
        m_inputs[iPlayer].Clear();
        m_guiInputs[iPlayer].Clear();

        if (iPlayer == mPrimaryPort)
        {
            mPrimaryPort = -1;
            mShadowPrimary = -1;
        }
        if (iPlayer == mSecondaryPort)
        {
            mSecondaryPort = -1;
            mShadowSecondary = -1;
        }
        if (m_MissingPort == iPlayer)
        {
            m_MissingPort = -1;
            GuiManager* pGui = GuiManager::Instance();
            pGui->CloseError(false, false);
            LuaHelper::CallTableProcWThis(pGui->GetCurrentScreen(), "fnPostMissingController");
        }
    }

    if (!mActiveControllers[0])
    {
        mBindAll = 0;
        mIgnoreMissing = 0;
    }
    mActiveControllers[0] = 1;
    m_HWPort[0] = 0;
    mPrimaryPort = 0;
}

// FUNCTION: SWBF2 0x0052aa20
void FLInputManager::BindPlayerInput()
{
    const unsigned char ucPorts = (unsigned char)RedInputManager::GetNumJoystickPorts();

    for (unsigned char i = 0; i < ucPorts; i++)
    {
        if (RedInputManager::GetJoystick(i)->m_connected)
        {
            mJoysticks[0]._redJoystick[i] = RedInputManager::GetJoystick(i);
        }
        else
        {
            mJoysticks[0]._redJoystick[i] = 0;
        }
    }

    PlayerPrefs::mAllUserPrefs[0].mJoystick = 0;
    m_HWPort[0] = 0;

    if (RedInputManager::GetNumMousePorts())
    {
        s_instance.mJoysticks[0]._redMouse = RedInputManager::GetMouse(0);
    }
    if (RedInputManager::GetNumKeybaordPorts())
    {
        s_instance.mJoysticks[0]._redKeyboard = RedInputManager::GetKeyboard(0);
    }
    if (RedInputManager::GetNumLCDPorts())
    {
        s_instance.mJoysticks[0].mLCDPanel = RedInputManager::GetLCDPanel(0);
    }
}

// FUNCTION: SWBF2 0x0052ab80
void FLInputManager::SetHotController(int iJoystick)
{
    if (mShadowPrimary >= 0 || iJoystick < 0)
    {
        RestorePrimarySecondary();
        if (iJoystick < 0)
        {
            return;
        }
    }
    if (iJoystick < 1 && m_HWPort[iJoystick] >= 0)
    {
        mShadowPrimary = mPrimaryPort;
        mPrimaryPort = iJoystick;
    }
}

// FUNCTION: SWBF2 0x0052abd0
void FLInputManager::RestorePrimarySecondary()
{
    int iPrimary = mShadowPrimary;
    if (iPrimary < 0)
    {
        return;
    }
    mPrimaryPort = iPrimary;
    int iSecondary = mShadowSecondary;
    mShadowPrimary = -1;
    if (iSecondary < 0)
    {
        return;
    }
    mSecondaryPort = iSecondary;
    mShadowSecondary = -1;
}

bool is_rumble_enable(int iPlayer);

// FUNCTION: SWBF2 0x0052ad10
void FLInputManager::StopIntermittentRumble(int iPlayer)
{
    if (is_rumble_enable(iPlayer))
    {
        const int iJoystick = NetGame::GetJoystickIndex(iPlayer);
        if (iJoystick >= 0)
        {
            m_rumble[iJoystick].StopIntermittent();
        }
    }
}

// TEMPLATE: SWBF2 0x0052b190
// util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::NewAllocator> > >::SetSize_
template bool util::Vector<unsigned int, util::BaseArray<unsigned int,
    util::TypedArrayAllocator<unsigned int, util::NewAllocator> > >::SetSize_(int, bool);
// TEMPLATE: SWBF2 0x0052b250
// util::BaseArray<unsigned short,util::TypedArrayAllocator<unsigned short,util::NewAllocator> >::Reserve_
template bool util::BaseArray<unsigned short,
    util::TypedArrayAllocator<unsigned short, util::NewAllocator> >::Reserve_(int, bool);
// TEMPLATE: SWBF2 0x0052b320
// util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::NewAllocator> >::Reserve_
template bool util::BaseArray<unsigned int,
    util::TypedArrayAllocator<unsigned int, util::NewAllocator> >::Reserve_(int, bool);
// TEMPLATE: SWBF2 0x0052b440
// util::TypedArrayAllocator<unsigned int,util::NewAllocator>::Allocate_
template unsigned int* util::TypedArrayAllocator<unsigned int, util::NewAllocator>::Allocate_(int&);

// FUNCTION: SWBF2 0x0052ab40
void FLInputManager::SwapPrimarySecondary()
{
    if (mPrimaryPort < 0 || mSecondaryPort < 0)
    {
        return;
    }
    if (mShadowPrimary < 0)
    {
        mShadowPrimary = mPrimaryPort;
        mShadowSecondary = mSecondaryPort;
    }
    int iPrimary = mPrimaryPort;
    mPrimaryPort = mSecondaryPort;
    mSecondaryPort = iPrimary;
}







// FUNCTION: SWBF2 0x0052aae0
void FLInputManager::SetReadAll(bool bReadAll)
{
    if (mReadAllControllers == (unsigned char)bReadAll)
    {
        return;
    }

    mReadAllControllers = bReadAll;
    if (!bReadAll && GetBindCount() <= 1 && GetPrimaryPort() >= 0)
    {
        mActiveControllers[GetPrimaryPort()] = true;
    }
    if (mReadAllControllers && GetPrimaryPort() >= 0)
    {
        mActiveControllers[GetPrimaryPort()] = true;
    }

    BindPlayerInput();
    GuiManager::DebounceJoystick(0);
}

// FUNCTION: SWBF2 0x0052ac50
int FLInputManager::GetPlayerFromHWPort(const int hwPort) const
{
    const int* phwPort = m_HWPort;
    int iPlayer = 0;
    for (unsigned int i = 0; i < 1; ++i, ++phwPort)
    {
        if (*phwPort == hwPort)
        {
            return iPlayer;
        }
        if (*phwPort >= 0)
        {
            ++iPlayer;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x0052ad50
void FLInputManager::StopAllRumble()
{
    for (int i = 0; i < 1; i++)
    {
        m_rumble[i].StopAllRumble();
    }
}

// FUNCTION: SWBF2 0x0052ad60
void FLInputManager::EnableRumble(int iPort, bool bEnable)
{
    m_rumble[iPort].Enable(bEnable);
}

// FUNCTION: SWBF2 0x0052aa00
bool FLInputManager::AnyMissing() const
{
    for (unsigned char i = 0; i < 1; i++)
    {
        if (s_instance.m_bIsMissing[i])
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0052ac10
int FLInputManager::GetHWPort(int iViewport) const
{
    int iBound = 0;
    for (unsigned int i = 0; i < 1; i++)
    {
        if (m_HWPort[i] >= 0)
        {
            if (iViewport == iBound)
            {
                return m_HWPort[i];
            }
            iBound++;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x0052ac90
bool is_rumble_enable(int iPlayer)
{
    bool bEnabled = true;
    if (NetGame::IsEnabled())
    {
        if (!NetGame::OnClient())
        {
            if (!NetGame::IsHost())
            {
                bEnabled = false;
            }
            else if (netIsDedicated || !netIsOneWorld)
            {
                bEnabled = false;
            }
        }
        else if (NetGame::IsSpectator(iPlayer))
        {
            bEnabled = false;
        }
    }
    return bEnabled;
}
