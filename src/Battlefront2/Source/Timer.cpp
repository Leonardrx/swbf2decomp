#include "Timer.h"
#include "NetGame.h"
#include "EventManager_Timer.h"
#include "HUDGameEvents.h"
#include "StringDB.h"

namespace EventManager
{
template <>
void Event<Timer, char>::Callback::Trigger(Timer& item, char& param);
}
// SYNTHETIC: SWBF2 0x00406aa0
// `dynamic initializer for 'Timer::sMap''

// GLOBAL: SWBF2 0x01f7ced8
PblHashTable<Timer, 64> Timer::sMap;
// SYNTHETIC: SWBF2 0x00406ac0
// `dynamic initializer for 'Timer::sMemoryPool''
// SYNTHETIC: SWBF2 0x007695f0
// `dynamic atexit destructor for 'Timer::sMemoryPool''

// GLOBAL: SWBF2 0x01f99418
// Timer::sMemoryPool
MemoryPool Timer::sMemoryPool("Timer", sizeof(Timer));

// FUNCTION: SWBF2 0x00657a80
bool Timer::Update(float fDeltaTime)
{
    if (!NetGame::IsEnabled() || !NetGame::IsPreGame())
    {
        mTime -= mRate * fDeltaTime;
        if (mTime <= 0.0f)
        {
            DeactivateThread();
            EventManager::TimerEvent::mElapsed.Trigger(*this, EventManager::noContext);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00657b20
void Timer::Start()
{
    if (!IsThreadActive())
    {
        ActivateThread(10, PER_LOCAL_TURN);
    }
}

// FUNCTION: SWBF2 0x00657b40
void Timer::Stop()
{
    if (IsThreadActive())
    {
        DeactivateThread();
    }
}

// FUNCTION: SWBF2 0x00657b60
bool Timer::IsRunning() const
{
    return const_cast<Timer*>(this)->IsThreadActive();
}

// FUNCTION: SWBF2 0x00657ba0
Timer* Timer::Find(const char* pcName)
{
    return Timer::sMap.Find(PblHash(pcName));
}

// FUNCTION: SWBF2 0x006579e0
Timer::~Timer()
{
    if (HUD::GameEvents::GetObjectiveTimer() == this)
    {
        HUD::GameEvents::SetObjectiveTimer(0);
    }
    sMap.Remove(mId);
    StringDB::Remove(mId);
}

// FUNCTION: SWBF2 0x00657900
Timer::Timer(const char* pcName)
{
    mId = PblHash(pcName);
    mTime = 0.0f;
    mRate = 1.0f;
    StringDB::Store(mId, pcName);
    sMap.Store(mId, this);
}

// SYNTHETIC: SWBF2 0x006579b0
// Timer::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00657ce7
// ??_ETimer@@WBI@AEPAXI@Z

namespace EventManager
{

// FUNCTION: SWBF2 0x00657bd0
template <>
void Event<Timer, char>::Callback::Trigger(Timer& item, char& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild)
            {
                if (!pChild->IsAlive())
                {
                    pChild->Release();
                    break;
                }
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
            }
        }
    }
    Release();
}
}

// FUNCTION: SWBF2 0x00657b70
void Timer::Clear()
{
    sMap.Clear();
}
