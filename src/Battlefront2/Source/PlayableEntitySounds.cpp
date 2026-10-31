#include "PlayableEntitySounds.h"

// FUNCTION: SWBF2 0x00619b60
PlayableEntitySounds::PlayableEntitySounds(PlayableEntitySoundsClass* pClass)
    : mPESClass(pClass)
{
}

// FUNCTION: SWBF2 0x00619b80
PlayableEntitySounds::~PlayableEntitySounds()
{
}

// FUNCTION: SWBF2 0x00619b90
PlayableEntitySoundsClass::PlayableEntitySoundsClass()
{
}

// FUNCTION: SWBF2 0x00619c10
PlayableEntitySoundsClass::PlayableEntitySoundsClass(const PlayableEntitySoundsClass* pBase)
{
    for (int i = 0; i < command_count; ++i)
    {
        mAICommands[i] = pBase->mAICommands[i];
        mPlayerCommands[i] = pBase->mPlayerCommands[i];
    }
    mSCResponseYessirSound = pBase->mSCResponseYessirSound;
    mSCResponseNosirSound = pBase->mSCResponseNosirSound;
}

// FUNCTION: SWBF2 0x00619d00
bool PlayableEntitySoundsClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x150a38e9: mAICommands[command_fieldMoveOut].SetID(pszValue, false, false); return true;
    case 0x8018b09d: mAICommands[command_fieldHold].SetID(pszValue, false, false); return true;
    case 0x1d988641: mAICommands[command_fieldFollow].SetID(pszValue, false, false); return true;
    case 0xe0999151: mAICommands[command_driverGetIn].SetID(pszValue, false, false); return true;
    case 0x3b62b992: mAICommands[command_driverGetOut].SetID(pszValue, false, false); return true;
    case 0x2512bb6d: mAICommands[command_passengerMoveOut].SetID(pszValue, false, false); return true;
    case 0xa473e5ea: mAICommands[command_passengerStop].SetID(pszValue, false, false); return true;
    case 0xf9d92705: mAICommands[command_passengerGetIn].SetID(pszValue, false, false); return true;
    case 0x45912fae: mAICommands[command_passengerGetOut].SetID(pszValue, false, false); return true;
    case 0x22987695: mAICommands[command_gunnerAllClear].SetID(pszValue, false, false); return true;
    case 0x1aec5f13: mAICommands[command_gunnerSteady].SetID(pszValue, false, false); return true;
    case 0x7ef094d0: mAICommands[command_gunnerGetIn].SetID(pszValue, false, false); return true;
    case 0xdf526f61: mAICommands[command_gunnerGetOut].SetID(pszValue, false, false); return true;

    case 0xe0e7aa4f: mPlayerCommands[command_fieldMoveOut].SetID(pszValue, false, false); return true;
    case 0xb2a1a19f: mPlayerCommands[command_fieldHold].SetID(pszValue, false, false); return true;
    case 0x491b3917: mPlayerCommands[command_fieldFollow].SetID(pszValue, false, false); return true;
    case 0x47041baf: mPlayerCommands[command_driverGetIn].SetID(pszValue, false, false); return true;
    case 0x4efe8d94: mPlayerCommands[command_driverGetOut].SetID(pszValue, false, false); return true;
    case 0x5b7c85cb: mPlayerCommands[command_passengerMoveOut].SetID(pszValue, false, false); return true;
    case 0x5d42f4c8: mPlayerCommands[command_passengerStop].SetID(pszValue, false, false); return true;
    case 0x6c89065f: mPlayerCommands[command_passengerGetIn].SetID(pszValue, false, false); return true;
    case 0xf260f2a4: mPlayerCommands[command_passengerGetOut].SetID(pszValue, false, false); return true;
    case 0xa3d45943: mPlayerCommands[command_gunnerAllClear].SetID(pszValue, false, false); return true;
    case 0x981f3d5d: mPlayerCommands[command_gunnerSteady].SetID(pszValue, false, false); return true;
    case 0xd1f9b472: mPlayerCommands[command_gunnerGetIn].SetID(pszValue, false, false); return true;
    case 0xc27c316f: mPlayerCommands[command_gunnerGetOut].SetID(pszValue, false, false); return true;

    case 0x8e131444: mSCResponseYessirSound.SetID(pszValue, false, false); return true;
    case 0x60681218: mSCResponseNosirSound.SetID(pszValue, false, false); return true;

    default:
        return false;
    }
}
