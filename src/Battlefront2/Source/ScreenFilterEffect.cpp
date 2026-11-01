#include "ScreenFilterEffect.h"
#include "GameState.h"

namespace FLRenderer
{

unsigned int GetCurrentViewport();
}

// GLOBAL: SWBF2 0x01eaf024
ScreenFilterEffect* ScreenFilterEffect::s_pInstance;

// FUNCTION: SWBF2 0x00633e60
void ScreenFilterEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new ScreenFilterEffect();
    }
}

// FUNCTION: SWBF2 0x00633ef0
ScreenFilterEffect::ScreenFilterEffect()
    : FLEffect(0xE32EAA23u, 9)
{
    Enable(true);
}

// FUNCTION: SWBF2 0x00634000
void ScreenFilterEffect::Init()
{
    m_data[0].uiState = 0;
    m_filterTimeStamp.Mark(&GameState::_TimeBase);
    PlatformInit();
}

// FUNCTION: SWBF2 0x00633ed0
void ScreenFilterEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// SYNTHETIC: SWBF2 0x00633f70
// ScreenFilterEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00633fa0
ScreenFilterEffect::~ScreenFilterEffect()
{
}

// FUNCTION: SWBF2 0x00634240
void ScreenFilterEffect::Render(int value)
{
    if (!((unsigned int)value & 0x200000) && m_data[FLRenderer::GetCurrentViewport()].uiState)
    {
        PlatformRender((unsigned int)value);
    }
}

// FUNCTION: SWBF2 0x00633e50
bool ScreenFilterEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x00634030
void ScreenFilterEffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x00634040
void ScreenFilterEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == 0xAF8BB8CEu)
        {
            Enable(data.GetUint32Arg(0) != 0);
        }
    }
}

// FUNCTION: SWBF2 0x006340c0
void ScreenFilterEffect::TimedFilter(int iViewport, float fDuration, float fFadeInTime,
                                     float fFadeOutTime, RedColor color, unsigned int uiTextureHash)
{
    if (fFadeInTime >= 0.0f)
    {
        m_data[iViewport].uiState = 2;
        m_data[iViewport].fIntensity = 0.0f;
    }
    else
    {
        m_data[iViewport].uiState = 1;
        m_data[iViewport].fIntensity = 1.0f;
    }
    float fNow = m_filterTimeStamp.GetElapsed();
    m_data[iViewport].fStartFadeInTime = fNow;
    m_data[iViewport].uiTextureHash = uiTextureHash;
    m_data[iViewport].fStopFadeInTime = fNow + fFadeInTime;
    m_data[iViewport].fStopFadeOutTime = fNow + fDuration;
    m_data[iViewport].filterColor = color;
    m_data[iViewport].fStartFadeOutTime = m_data[iViewport].fStopFadeOutTime - fFadeOutTime;
}

// FUNCTION: SWBF2 0x00634160
void ScreenFilterEffect::Update(float)
{
    float fNow = m_filterTimeStamp.GetElapsed();
    FilterData& data = m_data[0];
    if (!data.uiState)
    {
        return;
    }
    if (data.uiState == 1 && data.fStartFadeOutTime > 0.0f && fNow >= data.fStartFadeOutTime)
    {
        data.fIntensity = 1.0f;
        data.uiState = 4;
    }
    else if (data.uiState == 2)
    {
        if (fNow >= data.fStopFadeInTime)
        {
            data.fIntensity = 1.0f;
            data.uiState = 1;
        }
        else
        {
            data.fIntensity = (fNow - data.fStartFadeInTime) / (data.fStopFadeInTime - data.fStartFadeInTime);
        }
    }
    else if (data.uiState == 4)
    {
        if (fNow >= data.fStopFadeOutTime)
        {
            data.fIntensity = 0.0f;
            data.uiState = 0;
        }
        else
        {
            data.fIntensity = 1.0f - (fNow - data.fStartFadeOutTime) / (data.fStopFadeOutTime - data.fStartFadeOutTime);
        }
    }
}
