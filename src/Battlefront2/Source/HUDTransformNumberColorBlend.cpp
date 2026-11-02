#include "HUDTransformNumberColorBlend.h"



// FUNCTION: SWBF2 0x00568020
HUD::TransformNumberColorBlend::TransformNumberColorBlend(const char* const pName,
                                                         Factory::ItemCallback pCallback,
                                                         void* const pUserData)
    : TransformNumberColor(pName, pCallback, pUserData), mBlendMode(blendMode_Alpha), mAlpha(127)
{
    mEventInput.Init(EventInput, this);
    mEventBlend.Init(EventBlend, this);
    mEventAlpha.Init(EventAlpha, this);
}

// FUNCTION: SWBF2 0x00568210
void HUD::TransformNumberColorBlend::EventInput(const Event& event, void* const pData)
{
    const EventClass::Type eType = event.GetClass()->GetType();
    if (eType < EventClass::type_Int || eType > EventClass::type_Float)
    {
        return;
    }
    TransformNumberColorBlend* pTransform = (TransformNumberColorBlend*)pData;
    if (!pTransform->mEventClassOutput)
    {
        return;
    }
    RedColor color = pTransform->CalculateOutputColor(event.GetDataFloat());
    const unsigned char alpha = pTransform->mAlpha;
    const unsigned char* pBlend = (const unsigned char*)pTransform->mBlendColor;
    switch (pTransform->mBlendMode)
    {
    case blendMode_Alpha:
    {
        const int iOther = 255 - alpha;
        color.r = (unsigned char)(((pBlend[2] * iOther) >> 8) + ((color.r * alpha) >> 8));
        color.g = (unsigned char)(((pBlend[1] * iOther) >> 8) + ((color.g * alpha) >> 8));
        color.b = (unsigned char)(((pBlend[0] * iOther) >> 8) + ((color.b * alpha) >> 8));
        break;
    }
    case blendMode_Additive:
    {
        unsigned short r = (unsigned short)(((color.r * alpha) >> 8) + pBlend[2]);
        unsigned short g = (unsigned short)(((color.g * alpha) >> 8) + pBlend[1]);
        unsigned short b = (unsigned short)(((color.b * alpha) >> 8) + pBlend[0]);
        color.r = (unsigned char)(r > 255 ? 255 : r);
        color.g = (unsigned char)(g > 255 ? 255 : g);
        color.b = (unsigned char)(b > 255 ? 255 : b);
        break;
    }
    }
    Event eventOut(pTransform->mEventClassOutput, &color);
    eventOut.Send();
}

// FUNCTION: SWBF2 0x00568380
void HUD::TransformNumberColorBlend::EventBlend(const Event& event, void* const pData)
{
    if (event.GetClass()->GetType() == EventClass::type_Color)
    {
        *(RedColor*)((TransformNumberColorBlend*)pData)->mBlendColor = *event.GetData().color;
    }
}

// FUNCTION: SWBF2 0x005683c0
void HUD::TransformNumberColorBlend::EventAlpha(const Event& event, void* const pData)
{
    TransformNumberColorBlend* pTransform = (TransformNumberColorBlend*)pData;
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Int:
    case EventClass::type_Uint:
        pTransform->mAlpha = (unsigned char)event.GetData().uintValue;
        break;
    case EventClass::type_Float:
        pTransform->mAlpha = (unsigned char)(int)(event.GetData().floatValue * 255.0f);
        break;
    }
}

// GLOBAL: SWBF2 0x007e69d0
char* HUD::TransformNumberColorBlend::sBlendMode[2] = { "Alpha", "Additive" };

// SYNTHETIC: SWBF2 0x00568090
// HUD::TransformNumberColorBlend::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005680c0
HUD::TransformNumberColorBlend::~TransformNumberColorBlend()
{
}

// FUNCTION: SWBF2 0x005680f0
bool HUD::TransformNumberColorBlend::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case 0x5d8b6dab: /* "Alpha" */
        mAlpha = (unsigned char)(int)(data.GetFloatArg(0) * 255.0f);
        return true;
    case 0x211b0ef6: /* "EventBlend" */
        return Item::ReadEvent(data, mEventBlend);
    case 0x41b355c1: /* "EventAlpha" */
        return Item::ReadEvent(data, mEventAlpha);
    case 0xfa784eab: /* "BlendMode" */
    {
        unsigned int uMode = HUD::TranslateString((const char**)sBlendMode, 2, data.GetStringArg(0)) + 1;
        if (uMode < blendMode_Additive + 1)
        {
            mBlendMode = (BlendMode)uMode;
        }
        return true;
    }
    }
    return TransformNumberColor::ReadData(config, data);
}

// FUNCTION: SWBF2 0x00568190
void HUD::TransformNumberColorBlend::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("BlendMode", uTabs, sBlendMode[mBlendMode - 1], sBlendMode[0]);
    file.WriteProperty("EventBlend", uTabs, mEventBlend.GetRegisteredClassName(true), "");
    file.WriteProperty("EventAlpha", uTabs, mEventAlpha.GetRegisteredClassName(true), "");
    TransformNumberColor::WriteData(file, uTabs);
}
