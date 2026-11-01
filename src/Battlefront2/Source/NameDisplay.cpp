#include "PebbleFL/Common/PblMath.h"
#include <d3dx9math.h>
#include "NameDisplay.h"
#include "CameraManager.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "DeathCamera.h"
#include "EntityCarrier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Team.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Graphics/RedCamera.h"



namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

extern bool netShowNames;

extern bool netFrameLock;
extern int netCurMaxPlayers;
extern RedColor gHudHighlightColor;
namespace FriendBuddyList
{
    bool __fastcall IsFriendBuddy(int iLocalIndex, int iPlayerId);
}
namespace AIDebug
{
    bool GetTestSpawnMode();
}

// GLOBAL: SWBF2 0x01faa69c

bool gShowLocalPlayerName;

// GLOBAL: SWBF2 0x007e7fe4
float xOffset = 0.51f;
// GLOBAL: SWBF2 0x007e7f9c
float yOffset = 0.54f;

Trackable* CameraManager::GetDeathCameraTarget(int iCam)
{
    if ((GameCamera*)mDeathCamera[iCam] == mCurGameCamera[iCam])
    {
        return mDeathCamera[iCam]->mOwner;
    }
    return 0;
}

Character* CameraManager::GetDeathCameraKiller(int iCam)
{
    if ((GameCamera*)mDeathCamera[iCam] == mCurGameCamera[iCam])
    {
        return mDeathCamera[iCam]->mKiller;
    }
    return 0;
}

// GLOBAL: SWBF2 0x01fa2494
NameDisplayThread* sNameDisplayThread[1];



// FUNCTION: SWBF2 0x005aa770
void NameDisplay::Create()
{
    for (int i = 0; i < 1; ++i)
    {
        sNameDisplayThread[i] = new NameDisplayThread;
        sNameDisplayThread[i]->mJoystickIndex = i;
    }
}

// FUNCTION: SWBF2 0x005aa8c0
void NameDisplay::Init()
{
    CameraManager* pCameraManager = CameraManager::sInstance;
    if (pCameraManager->m_iNumCam == 1 && !(NetGame::IsEnabled() && netShowNames))
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        unsigned int hView = CameraManager::sInstance->m_hView[i];
        sNameDisplayThread[i]->ActivateThread(0, PER_FRAME);
        for (int j = 0; j < 64; ++j)
        {
            sNameDisplayThread[i]->mPlayerNames[j].Enable(false);
            sNameDisplayThread[i]->mPlayerNames[j].AddElement(&sNameDisplayThread[i]->mPlayerNamesText[j]);
            sNameDisplayThread[i]->mPlayerNamesText[j].SetFont(RedFont::GAMEFONT_TINY);
            sNameDisplayThread[i]->mPlayerNamesText[j].SetIntercharacterSpacing(0);

            sNameDisplayThread[i]->mPlayerNamesText[j].m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
            sNameDisplayThread[i]->mPlayerNamesText[j].SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER,
                                                                    RedTextElement::TEXT_VALIGN_BOTTOM);
            FLRenderer::AddInterfaceElement(hView, &sNameDisplayThread[i]->mPlayerNames[j],
                                            RedInterfaceScreen::CENTER, RedInterfaceScreen::CENTER,
                                            false);
        }
    }
}

// FUNCTION: SWBF2 0x005aaa10
void NameDisplay::Reset()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        for (int j = 0; j < 64; ++j)
        {
            sNameDisplayThread[i]->mPlayerNamesText[j].Cleanup();
        }
    }
}

// FUNCTION: SWBF2 0x005aaa60
bool NameDisplayThread::Update(float)
{
    int iLocalIndex = NetGame::GetLocalIndex(mJoystickIndex);
    int iSpectator = NetGame::GetSpectator(iLocalIndex);
    if (iSpectator != -1)
    {
        iLocalIndex = iSpectator;
    }

    Character* pLocalCharacter = 0;
    Character* pKiller = 0;
    GameObject* pLocalObject = 0;
    if (Character::sCharacters)
    {
        pLocalCharacter = Character::FindPlayer(iLocalIndex);
        Controllable* pLocalUnit = 0;
        if (pLocalCharacter)
        {
            pLocalUnit = pLocalCharacter->GetVehicle();
            if (!pLocalUnit)
            {
                pLocalUnit = pLocalCharacter->GetUnit();
            }
        }
        if (pLocalUnit)
        {
            pLocalObject = pLocalUnit->GetGameObject();
        }
    }
    if (!pLocalObject)
    {
        Trackable* pTarget = CameraManager::sInstance->GetDeathCameraTarget(mJoystickIndex);
        if (pTarget)
        {
            pLocalObject = pTarget->GetGameObject();
            pKiller = CameraManager::sInstance->GetDeathCameraKiller(mJoystickIndex);
        }
    }

    RedCamera* pCamera = CameraManager::sInstance->mRedCamera[mJoystickIndex];
    int iNumSlots = (int)CameraManager::sInstance->m_iNumCam;
    if (iNumSlots < netCurMaxPlayers)
    {
        iNumSlots = netCurMaxPlayers;
    }
    for (int i = 0; i < iNumSlots; ++i)
    {
        RedScreenGroupElement& group = mPlayerNames[i];
        RedTextElement& text = mPlayerNamesText[i];
        group.Enable(false);
        if (!pLocalObject)
        {
            continue;
        }
        if (!gShowLocalPlayerName && i == NetGame::GetLocalIndex(mJoystickIndex))
        {
            continue;
        }
        Character* pCharacter = Character::FindPlayer(i);
        if (!pCharacter || !pCharacter->IsPlayer())
        {
            continue;
        }
        Controllable* pUnit = pCharacter->GetVehicle();
        if (!pUnit)
        {
            pUnit = pCharacter->GetUnit();
        }
        if (!pUnit || pUnit->GetPlayerId() < 0)
        {
            continue;
        }
        GameObject* pObject = pUnit->GetGameObject();
        if (!pObject)
        {
            continue;
        }
        CollisionObject* pCollision = pObject->GetCollisionObject();
        if (!pCollision || AIDebug::GetTestSpawnMode())
        {
            continue;
        }

        int iLocalTeam = pLocalObject->GetTeam();
        if (iLocalTeam == pLocalObject->GetPerceivedTeam() && iLocalTeam != pObject->GetTeam())
        {
            if (pObject->GetPerceivedTeam() != pObject->GetTeam())
            {
                continue;
            }
            if (pLocalCharacter && !pKiller)
            {
                if (iSpectator == -1)
                {
                    Controllable* pLocalUnit = pLocalCharacter->GetControllable();
                    if (!pLocalUnit)
                    {
                        continue;
                    }
                    if (!(const GameObject*)pLocalUnit->mReticuleTarget[pLocalUnit->GetWpnChannel()])
                    {
                        continue;
                    }
                    if (pLocalUnit->mReticuleTarget[pLocalUnit->GetWpnChannel()] != pObject)
                    {
                        Controllable* pOnFoot = pCharacter->GetUnit();
                        if (!pOnFoot ||
                            pLocalUnit->mReticuleTarget[pLocalUnit->GetWpnChannel()] != pOnFoot->GetGameObject())
                        {
                            continue;
                        }
                    }
                }
            }
            else if (pKiller != pCharacter)
            {
                continue;
            }
        }
        if (pLocalObject == pObject)
        {
            continue;
        }

        float fHeight = pCollision->GetCollisionRadius() * 1.2f;
        PblVector3 vPos = pObject->GetMatrix().GetTranslation();
        if (!netFrameLock && pCollision->GetGameObject())
        {
            GameObject* pHost = pCollision->GetGameObject();
            PblMatrix mHost(pHost->GetMatrix());
            PblVector3 vOffset = vPos - mHost.GetTranslation();
            gameObjectInterpolator.GetMatrix(pHost->GetNetUniqueId(), mHost);
            vPos = mHost.GetTranslation() + vOffset;
        }
        vPos += PblVector3(0.0f, fHeight, 0.0f);

        PblVector3 vScreen = pCamera->TransformWorldPointToProjectionSpace(vPos);
        if (vScreen.z < -1.0f || vScreen.z > 1.0f)
        {
            continue;
        }
        const PblMatrix& mCamera = pCamera->GetMatrix();
        PblVector3 vEye = mCamera.GetTranslation();
        PblVector3 vToUnit = vPos - vEye;
        PblVector3 vDir;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vToUnit));
        const PblVector3 vBack = -mCamera.GetZAxis();
        if (vDir.y * vBack.y + vDir.x * vBack.x + vDir.z * vBack.z <= 0.0f)
        {
            continue;
        }

        Entity* pEntity = pUnit->GetEntity();
        float fFadeRange = pEntity && pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer) ? 350.0f : 20.0f;
        float fDistance = Length(vPos - vEye);
        unsigned char ucAlpha = 0xff;
        if (fDistance > fFadeRange)
        {
            int iAlpha = 335 - ((int)fDistance << 2);
            ucAlpha = iAlpha < 0 ? 0 : (unsigned char)iAlpha;
            if (!ucAlpha)
            {
                continue;
            }
        }

        if (pLocalCharacter)
        {
            text.m_color = pLocalCharacter->GetTeamPtr()->GetColor(pCharacter->GetTeamNumber());
            if (NetGame::IsEnabled() && pCharacter->GetTeamNumber() == pLocalCharacter->GetTeamNumber() &&
                FriendBuddyList::IsFriendBuddy(iLocalIndex, i))
            {
                RedColor white(gHudHighlightColor);
                text.m_color = white;
            }
        }
        else
        {
            text.m_color = pCharacter->GetTeamPtr()->GetColor(pLocalObject->GetTeam());
        }
        text.m_color.a = ucAlpha;
        text.SetText(pCharacter->GetName(), true);

        int iOtherRiders = 0;
        Controllable* pVehicle = pCharacter->GetVehicle();
        if (pVehicle)
        {
            const PblList<Controllable>& riders = pVehicle->GetGameObject()->GetControllableList();
            for (PblList<Controllable>::Itor it = riders.Begin(); (Controllable*)it; ++it)
            {
                Controllable* pRider = it;
                if ((pRider->IsPilotable() || pRider->IsRemoteVehicle()) && pRider->GetPilot())
                {
                    if (pRider == pVehicle)
                    {
                        break;
                    }
                    ++iOtherRiders;
                }
            }
        }
        if (iOtherRiders > 0)
        {
            group.SetScreenRelativePos(xOffset + vScreen.x * 0.5f, yOffset + vScreen.y * 0.5f);
        }
        else
        {
            group.SetScreenRelativePos(vScreen.x * 0.5f + 0.5f, vScreen.y * 0.5f + 0.5f);
        }
        group.Enable(true);
    }
    return true;
}
