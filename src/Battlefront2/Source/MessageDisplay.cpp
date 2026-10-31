#include "MessageDisplay.h"
#include "HUDGameEvents.h"
#include "Character.h"
#include "GameObject.h"
#include "Team.h"
#include "NetGame.h"
#include "FLInputManager.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Graphics/RedColor.h"

// GLOBAL: SWBF2 0x01e579bc
MessageDisplay* MessageDisplay::sInstance[1][DISPLAY_END];

// FUNCTION: SWBF2 0x005a39e0
void MessageDisplay::AddMessage(const unsigned short* pText, RedColor color)
{
    unsigned int uViewport;
    DISPLAY_TYPE eType;
    GetInfo(uViewport, eType);
    switch (eType)
    {
    case DISPLAY_SYSTEM_MSG:
        HUD::GameEvents::DisplayMessage(uViewport, pText, &color);
        break;
    case DISPLAY_PLAYER_CHAT:
        HUD::GameEvents::DisplayTextChatMessage(uViewport, pText, &color);
        break;
    }
}

// FUNCTION: SWBF2 0x005a3aa0
void MessageDisplay::AddKillMessage(const Character* pVictim, const Character* pKiller,
                                    unsigned int uIcon, const Team* pTeam)
{
    if (!mCanRender || !mVisible || !pVictim)
    {
        return;
    }

    int iTeam = pKiller->mTeamNumber;
    unsigned short message[256];
    if (pKiller->IsHero())
    {
        const unsigned short* pFormat = RedLocalizeDB::Find(0x083ab1e9u, false);
        if (Team::sTeams[iTeam])
        {
            unsigned short heroName[64];
            heroName[0] = 0;
            const GameObjectClass* pClass = Team::sTeams[iTeam]->GetHeroClass();
            if (pClass)
            {
                PblUnicode::ustrlcpy(heroName, pClass->mLabel, 63);
            }
            PblUnicode::usnprintf(message, 255, pFormat, pVictim->mName, heroName);
        }
    }
    else
    {
        if (pVictim->mTeamNumber == iTeam)
        {
            PblUnicode::usnprintf(message, 255, RedLocalizeDB::Find(0xc57ae533u, false),
                                  pVictim->mName, pKiller->mName);
        }
        else
        {
            PblUnicode::usnprintf(message, 255, RedLocalizeDB::Find(0xacc1eef7u, false),
                                  pVictim->mName, pKiller->mName);
        }
    }
    if (CanProcessChatMessage())
    {
        unsigned int uLocal = NetGame::GetLocalIndex(0);
        if (uLocal <= 63 && Character::GetPtr(uLocal) == pKiller)
        {
            FLInputManager::s_instance.ProcessChatMessageInput(message);
        }
    }
    AddMessage(message, pTeam->GetColor(iTeam));
}

// FUNCTION: SWBF2 0x005a38e0
void MessageDisplay::Create()
{
    for (int iView = 0; iView < 1; ++iView)
    {
        for (int i = 0; i < DISPLAY_END; ++i)
        {
            sInstance[iView][i] = new MessageDisplay((DISPLAY_TYPE)i);
        }
    }
}

// FUNCTION: SWBF2 0x005a3950
MessageDisplay::MessageDisplay(DISPLAY_TYPE eType)
{
    mCanRender = true;
    mVisible = true;
}

// FUNCTION: SWBF2 0x005a39a0
MessageDisplay::~MessageDisplay()
{
}

// FUNCTION: SWBF2 0x005a39b0
MessageDisplay* MessageDisplay::Instance(unsigned int uViewport, DISPLAY_TYPE eType)
{
    if (uViewport < 1)
    {
        return sInstance[uViewport][eType];
    }
    return 0;
}

// FUNCTION: SWBF2 0x005a39d0
bool MessageDisplay::Update(float fDeltaTime)
{
    return true;
}

// FUNCTION: SWBF2 0x005a3a40
void MessageDisplay::GetInfo(unsigned int& uViewport, DISPLAY_TYPE& eType) const
{
    for (unsigned int i = 0; i < 1; ++i)
    {
        for (unsigned int j = 0; j < DISPLAY_END; ++j)
        {
            if (this == sInstance[i][j])
            {
                uViewport = i;
                eType = (DISPLAY_TYPE)j;
                return;
            }
        }
    }
    uViewport = 0;
    eType = (DISPLAY_TYPE)0;
}

// FUNCTION: SWBF2 0x005a3be0
void MessageDisplay::EnableViewport(unsigned int uiViewport, bool bEnable)
{
    MessageDisplay** ppDisplay = sInstance[0];
    for (int i = 0; i < DISPLAY_END; ++i, ++ppDisplay)
    {
        MessageDisplay* pDisplay = *ppDisplay;
        if (pDisplay)
        {
            unsigned int uViewport;
            DISPLAY_TYPE eType;
            pDisplay->GetInfo(uViewport, eType);
            if (uViewport == uiViewport)
            {
                pDisplay->mCanRender = bEnable;
            }
        }
    }
}

// FUNCTION: SWBF2 0x005a3920
void MessageDisplay::Destroy()
{
    MessageDisplay** ppDisplay = sInstance[0];
    for (int i = 0; i < DISPLAY_END; ++i, ++ppDisplay)
    {
        if (*ppDisplay)
        {
            delete *ppDisplay;
            *ppDisplay = 0;
        }
    }
}
