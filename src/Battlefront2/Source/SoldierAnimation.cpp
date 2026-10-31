#include "SoldierAnimation.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "SoldierAnimationData.h"

// SYNTHETIC: SWBF2 0x00406390
// `dynamic initializer for 'SoldierAnimation::sMemoryPool''
// SYNTHETIC: SWBF2 0x007694b0
// `dynamic atexit destructor for 'SoldierAnimation::sMemoryPool''

// GLOBAL: SWBF2 0x01f6ddf8
// SoldierAnimation::sMemoryPool
MemoryPool SoldierAnimation::sMemoryPool("SoldierAnimation", sizeof(SoldierAnimation));

// FUNCTION: SWBF2 0x0063c2d0
unsigned short SoldierAnimation::GetAnimationNumFrames() const
{
    return m_pZephyrAnim ? m_pZephyrAnim->GetNumFrames() : 0;
}

// FUNCTION: SWBF2 0x0063c290
SoldierAnimation::SoldierAnimation(const ZephyrAnim* pAnim, const SoldierAnimationData* pAnimData, SCOPE eScope)
{
    Init(pAnim, pAnimData, eScope);
}

// FUNCTION: SWBF2 0x0063c2b0
void SoldierAnimation::Init(const ZephyrAnim* pAnim, const SoldierAnimationData* pAnimData, SCOPE eScope)
{
    m_pZephyrAnim = pAnim;
    m_pAnimData = pAnimData;
    m_uiData = (unsigned int)eScope << 30;
}

bool SoldierAnimation::IsLooping() const
{
    return m_pAnimData->IsLooping();
}

// FUNCTION: SWBF2 0x0063c2e0
void SoldierAnimation::SetupDynamicPose(ZephyrPoseDyn<32>& pose, float fTime) const
{
    if (m_pZephyrAnim != pose.m_kAnim.m_pkAnim)
    {
        pose.SetAnimation(m_pZephyrAnim, 30.0f);
        pose.SetCurTime(0.0f);
        pose.SetLoop(m_pAnimData->IsLooping());
        if (fTime <= 0.0f)
        {
            pose.Update(0.0f);
        }
    }
    if (fTime > 0.0f)
    {
        pose.Update(fTime);
    }
}
