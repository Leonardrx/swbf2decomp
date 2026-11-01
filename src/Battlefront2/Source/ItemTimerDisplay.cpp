#include "PebbleFL/Common/PblMath.h"
#include <d3dx9math.h>
#include "ItemTimerDisplay.h"

#include <float.h>
#include <math.h>

#include "CameraManager.h"
#include "Character.h"
#include "CollisionObject.h"
#include "EntityItem.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Interface/RedFont.h"

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

// GLOBAL: SWBF2 0x01f9c2e0
ItemTimerDisplay::DisplayThread* sDisplayThread[1];

// FUNCTION: SWBF2 0x0056a6f0
void ItemTimerDisplay::Create()
{
    for (int i = 0; i < 1; ++i)
    {
        sDisplayThread[i] = new DisplayThread;
        sDisplayThread[i]->mJoystickIndex = i;
    }
}

// FUNCTION: SWBF2 0x0056a840
void ItemTimerDisplay::Init()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        const unsigned int hView = CameraManager::sInstance->m_hView[i];
        sDisplayThread[i]->ActivateThread(0, PER_FRAME);
        sDisplayThread[i]->mItemTimerCount = 0;

        for (int j = 0; j < 8; ++j)
        {
            sDisplayThread[i]->mItemArray[j] = 0;
            sDisplayThread[i]->mItemGroup[j].Enable(false);
            sDisplayThread[i]->mItemGroup[j].AddElement(&sDisplayThread[i]->mItemText[j]);

            sDisplayThread[i]->mItemText[j].SetFont(RedFont::GAMEFONT_TINY);
            sDisplayThread[i]->mItemText[j].SetIntercharacterSpacing(0);
            sDisplayThread[i]->mItemText[j].m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
            sDisplayThread[i]->mItemText[j].SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER,
                                                         RedTextElement::TEXT_VALIGN_BOTTOM);

            FLRenderer::AddInterfaceElement(hView, &sDisplayThread[i]->mItemGroup[j], RedInterfaceScreen::CENTER,
                                            RedInterfaceScreen::CENTER, false);
        }
    }
}

// FUNCTION: SWBF2 0x0056a970
void ItemTimerDisplay::Reset()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            sDisplayThread[i]->mItemText[j].Cleanup();
        }
    }
}

// FUNCTION: SWBF2 0x0056a9c0
void ItemTimerDisplay::AddItemTimer(EntityItem* pItem)
{
    if (NetGame::IsEnabled() && !NetGame::IsClient() &&
        (!NetGame::IsOneWorld() || !NetGame::IsHost() || NetGame::IsDedicated()))
    {
        return;
    }
    if (pItem->NoDisplayLifeTimer())
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        if (sDisplayThread[i]->mItemTimerCount >= 8)
        {
            continue;
        }

        Character* pPlayer = Character::FindPlayer(NetGame::GetLocalIndex(i));
        Character* pOwner = pItem->mOwner;
        if ((!pOwner || pOwner == pPlayer) && pItem->GetLifeTimer() < FLT_MAX)
        {
            DisplayThread* pThread = sDisplayThread[i];
            pThread->mItemArray[pThread->mItemTimerCount++] = pItem;
        }
    }
}

// FUNCTION: SWBF2 0x0056aac0
void ItemTimerDisplay::RemoveItemTimer(EntityItem* pItem)
{
    if (NetGame::IsEnabled() && !NetGame::IsClient() &&
        (!NetGame::IsOneWorld() || !NetGame::IsHost() || NetGame::IsDedicated()))
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        DisplayThread* pThread = sDisplayThread[i];
        int j = 0;
        for (; j < pThread->mItemTimerCount; ++j)
        {
            if (pThread->mItemArray[j] == pItem)
            {
                break;
            }
        }
        if (j < pThread->mItemTimerCount)
        {
            --pThread->mItemTimerCount;
            for (; j < pThread->mItemTimerCount; ++j)
            {
                pThread->mItemArray[j] = pThread->mItemArray[j + 1];
            }
        }
    }
}

// FUNCTION: SWBF2 0x0056ab90
void ItemTimerDisplay::HideAll()
{
    if (NetGame::IsEnabled() && !NetGame::IsClient() &&
        (!NetGame::IsOneWorld() || !NetGame::IsHost() || NetGame::IsDedicated()))
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        for (int j = 0; j < sDisplayThread[i]->mItemTimerCount; ++j)
        {
            sDisplayThread[i]->mItemGroup[j].Enable(false);
        }
    }
}

// FUNCTION: SWBF2 0x0056ac50
bool ItemTimerDisplay::DisplayThread::Update(float fDeltaTime)
{
    RedCamera* pCamera = CameraManager::sInstance->GetCamera(mJoystickIndex);

    for (int i = 0; i < 8; ++i)
    {
        RedScreenGroupElement* pGroup = &mItemGroup[i];
        pGroup->Enable(false);
    }

    if (GameLoop::IsGameOver())
    {
        return true;
    }

    for (int i = 0; i < mItemTimerCount; ++i)
    {
        const EntityItem* pObject = mItemArray[i];
        float fHeight = pObject->GetCollisionRadius() * 1.2f;
        PblVector3 vOffset = gVectorUnitY * fHeight;
        const PblVector3& vCenter = pObject->GetCollisionCenter();
        PblVector3 vPos = vCenter + vOffset;

        PblVector3 vScreen = pCamera->TransformWorldPointToProjectionSpace(vPos);
        if (vScreen.z < -1.0f || vScreen.z > 1.0f)
        {
            continue;
        }

        const PblMatrix& mCamera = pCamera->GetMatrix();
        PblVector3 vCamPos = mCamera.GetTranslation();
        PblVector3 vDelta(vPos.x - vCamPos.x, vPos.y - vCamPos.y, vPos.z - vCamPos.z);
        PblVector3 vDir;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vDir), reinterpret_cast<const D3DXVECTOR3*>(&vDelta));
        if (DotProduct(vDir, -mCamera.GetZAxis()) <= 0.0f)
        {
            continue;
        }

        float fDist = PblMath::Sqrt((vPos.y - vCamPos.y) * (vPos.y - vCamPos.y) +
                                    (vPos.x - vCamPos.x) * (vPos.x - vCamPos.x) +
                                    (vPos.z - vCamPos.z) * (vPos.z - vCamPos.z));
        unsigned char uAlpha = 0xff;
        if (fDist > 20.0f)
        {
            int iAlpha = 255 - ((int)fDist - 20) * 4;
            uAlpha = (unsigned char)(iAlpha < 0 ? 0 : iAlpha);
            if (!uAlpha)
            {
                continue;
            }
        }

        RedTextElement* pText = &mItemText[i];
        RedColor color;
        color.b = 0xf2;
        color.g = 0xa7;
        color.r = 0x43;
        color.a = uAlpha;
        pText->SetColor(color);
        pText->SetTextFormatted("%d", (int)ceilf(mItemArray[i]->GetLifeTimer()));

        RedScreenGroupElement* pGroup = &mItemGroup[i];
        pGroup->SetScreenRelativePos(vScreen.x * 0.5f + 0.5f, vScreen.y * 0.5f + 0.5f);
        pGroup->Enable(true);
    }
    return true;
}
