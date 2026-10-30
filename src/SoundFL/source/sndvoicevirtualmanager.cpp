#include "sndvoicevirtualmanager.h"

#include <stddef.h>

#include "sndvoicebase.h"
#include "sndvoicevirtual.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00748900
VoiceBase* VoiceVirtualManager::GetFreeVoice()
{
    for (PblListSingle::Itor itor(mVoiceList);
         itor.GetNode() != mVoiceList.GetTerminator(); ++itor)
    {
        VoiceBase* const pVoice = (VoiceBase*)itor.GetNode()->GetObjectA(
            &mVoiceList, offsetof(VoiceBase, mNode));
        if (!pVoice->GetLocked())
        {
            return pVoice;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00748920
void VoiceVirtualManager::InsertVoice(VoiceVirtual* pVoice)
{
    pVoice->mNode.mNode.Remove();
    pVoice->DetermineClosestListener();
    pVoice->CalculatePriority(false);
    mActiveList.Add(&pVoice->mNode);
}

// FUNCTION: SWBF2 0x00748990
void VoiceVirtualManager::RemoveVoice(VoiceVirtual* pVoice)
{
    VoiceBase* pActive = pVoice->mVoice;
    if (pActive)
    {
        pActive->SourceTransport::Stop();
        pActive->SetSample(0);
        pVoice->SetVoice(0);
    }
    pVoice->mNode.mNode.Remove();
    mInactiveList.Add(&pVoice->mNode);
}

// FUNCTION: SWBF2 0x00748a10
void VoiceVirtualManager::Update(float fDeltaTime)
{
    unsigned int uiNumActive = 0;

    PblListDoubleSorted::Itor itor = mInactiveList.Begin();
    while (itor.GetNode(&mInactiveList) != mInactiveList.GetTerminator())
    {
        PblListDoubleSorted::Node* pNode = itor.GetNode(&mInactiveList);
        VoiceVirtual* pVoice = (VoiceVirtual*)pNode->GetObjectA(&mInactiveList, offsetof(VoiceVirtual, mNode));
        ++itor;
        pVoice->Update(fDeltaTime);
    }

    for (itor = mActiveList.Begin(); itor.GetNode(&mActiveList) != mActiveList.GetTerminator(); ++itor)
    {
        PblListDoubleSorted::Node* pNode = itor.GetNode(&mActiveList);
        VoiceVirtual* pVoice = (VoiceVirtual*)pNode->GetObjectA(&mActiveList, offsetof(VoiceVirtual, mNode));
        pVoice->DetermineClosestListener();
        pVoice->CalculatePriority(false);
        ++uiNumActive;
    }

    if (uiNumActive > mNumManagedVoices)
    {
        mActiveList.Sort();
    }

    for (itor = mActiveList.End(); itor.GetNode(&mActiveList) != mActiveList.GetTerminator();)
    {
        --uiNumActive;
        PblListDoubleSorted::Node* pNode = itor.GetNode(&mActiveList);
        VoiceVirtual* pVoice = (VoiceVirtual*)pNode->GetObjectA(&mActiveList, offsetof(VoiceVirtual, mNode));
        VoiceBase* pHardwareVoice = pVoice->GetVoice();
        --itor;
        if (uiNumActive < mNumManagedVoices)
        {
            if (!pHardwareVoice)
            {
                pVoice->SetVoice(GetFreeVoice());
            }
        }
        else if (pHardwareVoice)
        {
            pHardwareVoice->SourceTransport::Stop();
            pVoice->SetVoice(0);
        }
        pVoice->Update(fDeltaTime);
    }
}

}

