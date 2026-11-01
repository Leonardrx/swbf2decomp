#include <float.h>

#include "HUDElementBar.h"

// FUNCTION: SWBF2 0x0054a190
void HUD::ElementBar::Factory::Setup(HUD::Item::Factory* pFactory)
{
    mValue.Init("Value", pFactory->mProperties, 0.0f, FLT_MAX, 0.01f);
    mMinValue.Init("MinValue", pFactory->mProperties, 0.0f, FLT_MAX, 0.01f);
    mMaxValue.Init("MaxValue", pFactory->mProperties, 0.0f, FLT_MAX, 0.01f);
    mEventValue.Init("EventValue", pFactory->mProperties,
                     HUD::Item::Factory::Property::type_Event);
}

static const unsigned int kValueHash      = 0x425ed3ca;
static const unsigned int kMinValueHash   = 0xf1a5a54c;
static const unsigned int kMaxValueHash   = 0x7332c626;
static const unsigned int kEventValueHash = 0x8500add4;

// FUNCTION: SWBF2 0x0054a150
const char* HUD::ElementBar::PropertyTranslateEnum(const unsigned int, const unsigned int)
{
    return 0;
}

// FUNCTION: SWBF2 0x0054a220
HUD::ElementBar::ElementBar()
    : mDefaultValue(0.0f), mValue(1.0f), mMinValue(0.0f), mMaxValue(1.0f)
{
    mEventValue.Init(EventValue, this);
}

// FUNCTION: SWBF2 0x0054a260
float HUD::ElementBar::SetValue(const float fValue)
{
    const float fRange = mMaxValue - mMinValue;
    float fClamped;
    if (mMinValue > fValue)
    {
        fClamped = mMinValue;
    }
    else if (fValue > mMaxValue)
    {
        fClamped = mMaxValue;
    }
    else
    {
        fClamped = fValue;
    }
    mValue = fClamped;

    float fResult;
    if (fRange != 0.0f)
    {
        fResult = (fClamped - mMinValue) / fRange;
    }
    else
    {
        fResult = 0.0f;
    }
    return fResult;
}

// FUNCTION: SWBF2 0x0054a2c0
bool HUD::ElementBar::SetProperty(const unsigned int uHashID, const void* pValue)
{
    switch (uHashID)
    {
    case kValueHash:
        mDefaultValue = mValue = *(const float*)pValue;
        SetValue(mValue);
        return true;
    case kMinValueHash:
        mMinValue = *(const float*)pValue;
        SetValue(mValue);
        return true;
    case kMaxValueHash:
        mMaxValue = *(const float*)pValue;
        SetValue(mValue);
        return true;
    case kEventValueHash:
        Item::SetEvent(*(const char* const*)pValue, mEventValue);
        return true;
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x0054a340
bool HUD::ElementBar::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kValueHash:
        *(float*)pValue = mValue;
        break;
    case kMinValueHash:
        *(float*)pValue = mMinValue;
        break;
    case kMaxValueHash:
        *(float*)pValue = mMaxValue;
        break;
    case kEventValueHash:
        *(const char**)pValue = mEventValue.GetRegisteredClassName(false);
        break;
    default:
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0054a3b0
void HUD::ElementBar::PostReadSetup()
{
    if (mMinValue > mMaxValue)
    {
        mMaxValue = mMinValue;
    }
    SetValue(mValue);
}

// FUNCTION: SWBF2 0x0054a3e0
bool HUD::ElementBar::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kValueHash:
        mDefaultValue = mValue = data.GetFloatArg(0);
        break;
    case kMinValueHash:
        mMinValue = data.GetFloatArg(0);
        break;
    case kMaxValueHash:
        mMaxValue = data.GetFloatArg(0);
        break;
    case kEventValueHash:
        Item::ReadEvent(data, mEventValue);
        break;
    default:
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0054a450
void HUD::ElementBar::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("Value", uTabs, mDefaultValue, 0.0f);
    file.WriteProperty("MinValue", uTabs, mMinValue, 0.0f);
    file.WriteProperty("MaxValue", uTabs, mMaxValue, 1.0f);
    file.WriteProperty("EventValue", uTabs, mEventValue.GetRegisteredClassName(true), "");
}

// FUNCTION: SWBF2 0x0054a4e0
void HUD::ElementBar::EventValue(const HUD::Event& event, void* const pData)
{
    const EventClass::Type eType = event.GetClass()->GetType();
    if (eType >= EventClass::type_Int && eType <= EventClass::type_Float)
    {
        ((ElementBar*)pData)->SetValue(event.GetDataFloat());
    }
}
