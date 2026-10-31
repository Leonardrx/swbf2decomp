#include <float.h>
#include <stdio.h>
#include <string.h>

#include "HUDElementMultilineText.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Util/RedStack.h"

// FUNCTION: SWBF2 0x00557220
HUD::ElementMultilineText::Factory::Factory()
{
    mHashID = 0xD53E50D7u;
    mName = "MultilineText";

    mNumLines.Init("NumLines", mProperties, 1, 80);
    mDisplayTime.Init("DisplayTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mAlwaysScroll.Init("AlwaysScroll", mProperties, Property::type_Bool);
    mAddToTop.Init("AddToTop", mProperties, Property::type_Bool);
    mDisableOnEmpty.Init("DisableOnEmpty", mProperties, Property::type_Bool);
    mScrollSpeed.Init("ScrollSpeed", mProperties, 0.0001f, FLT_MAX, 0.01f);
    mEventText.Init("EventText", mProperties, Property::type_Event);
    mEventColor.Init("EventColor", mProperties, Property::type_Event);
}

// FUNCTION: SWBF2 0x00557190
HUD::Item* HUD::ElementMultilineText::Factory::CreateItem(const char* const pName,
                                                          ItemCallback pCallback,
                                                          void* const pUserData)
{
    HUD::Item* pItem = new ElementMultilineText(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

static const unsigned int kNumLines = 0x0269babe;
static const unsigned int kDisplayTime = 0xdaef1514;
static const unsigned int kAlwaysScroll = 0xb74de20d;
static const unsigned int kAddToTop = 0x09771c74;
static const unsigned int kDisableOnEmpty = 0xcf649993;
static const unsigned int kScrollSpeed = 0xdfc96107;
static const unsigned int kEventText = 0x7e1311a8;
static const unsigned int kEventColor = 0x49f429ae;

static const unsigned int kTextFactory = 0xBDE64E3Eu;

static inline RedTextElement& GetText(HUD::ElementMultilineText::Line& line)
{
    return line;
}

float HUD::Element::GetFadeHoldElapsed() const
{
    if (mFader.GetPhase() < HUD::Envelope::phase_Hold)
    {
        return 0.0f;
    }
    if (mFader.GetPhase() == HUD::Envelope::phase_Hold)
    {
        return mFader.GetElapsed();
    }
    return FLT_MAX;
}

// SYNTHETIC: SWBF2 0x00558b63
// ??_ELine@ElementMultilineText@HUD@@WBAA@AEPAXI@Z




// FUNCTION: SWBF2 0x00557150
bool HUD::ElementMultilineText::Line::SetProperty(const unsigned int uHashID,
                                                  const void* pValue)
{
    const bool bResult = HUD::ElementText::SetProperty(uHashID, pValue);
    if (bResult && mGroup)
    {
        mGroup->RefreshFormatting(false);
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00557460
HUD::ElementMultilineText::ElementMultilineText(const char* const pName,
                                                Factory::ItemCallback pCallback,
                                                void* const pUserData)
    : HUD::ElementGroup(pName, pCallback, pUserData),
      mLines(0),
      mNumLines(0),
      mMaxDisplayLines(0),
      mLinesPending(0),
      mNumLinesDisplayed(0),
      mNextLine(0),
      mDisplayInterval(2.0f)
{
    mAlwaysScroll = false;
    mAddToTop = false;
    mDisableOnEmpty = false;
    mState = state_Waiting;
    mScrollSpeed = 1.0f;
    Line* pFormat = &mFormat;
    if (pFormat)
    {
        pFormat->SetFactory(HUD::Item::Factory::FindByHashID(kTextFactory));
    }
    pFormat->mDynamicAlloc = false;
    pFormat->mWriteEnabled = false;
    mFormat.SetGroup(this);

    mFormat.HUD::Item::SetName("Format");
    mEventText.Init(EventText, this);
    mEventColor.Init(EventColor, this);
}

// SYNTHETIC: SWBF2 0x00558b6e
// ??_EElementMultilineText@HUD@@WBHA@AEPAXI@Z

// FUNCTION: SWBF2 0x00557630
HUD::ElementMultilineText::~ElementMultilineText()
{
    FreeLines();
}

// FUNCTION: SWBF2 0x005577b0
unsigned int HUD::ElementMultilineText::GetNextPendingLineIndex() const
{
    if (mNextLine >= mLinesPending)
    {
        return mNextLine - mLinesPending;
    }
    return mNextLine + mNumLines - mLinesPending;
}

// FUNCTION: SWBF2 0x005577e0
unsigned int HUD::ElementMultilineText::GetEldestLineIndex(unsigned int uIndex) const
{
    if (uIndex >= mNumLinesDisplayed)
    {
        return uIndex - mNumLinesDisplayed;
    }
    return uIndex + mNumLines - mNumLinesDisplayed;
}

// FUNCTION: SWBF2 0x00557810
HUD::ElementMultilineText::Line* HUD::ElementMultilineText::GetTop()
{
    if (mNumLinesDisplayed)
    {
        return &mLines[GetEldestLineIndex(GetNextPendingLineIndex())];
    }
    return 0;
}

// FUNCTION: SWBF2 0x00557840
HUD::ElementMultilineText::Line* HUD::ElementMultilineText::GetBottom()
{
    if (mNumLinesDisplayed)
    {
        unsigned int uIndex = GetNextPendingLineIndex();
        if (!uIndex)
        {
            uIndex = mNumLines;
        }
        return &mLines[uIndex - 1];
    }
    return 0;
}

// FUNCTION: SWBF2 0x00557870
bool HUD::ElementMultilineText::TopLineAtTop()
{
    Line* pTop = GetTop();
    if (pTop)
    {
        return pTop->GetPosition().y <= 0.0001f;
    }
    return false;
}

// FUNCTION: SWBF2 0x005578a0
bool HUD::ElementMultilineText::BottomLineOutOfBottomRegion(float fHeight, float fLineHeight)
{
    Line* pBottom = GetBottom();
    if (pBottom)
    {
        return fHeight - fLineHeight >= pBottom->GetPosition().y + fLineHeight;
    }
    return true;
}

// FUNCTION: SWBF2 0x005578d0
void HUD::ElementMultilineText::EnableNextLine(unsigned int uIndex)
{
    if (mLinesPending)
    {
        const float fLineHeight = (float)(int)mFormat.GetTextHeight();
        const float fHeight = (float)(int)GetHeightPixels();
        if (BottomLineOutOfBottomRegion(fHeight, fLineHeight))
        {
            const float fY = mAddToTop ? (float)(int)mNumLinesDisplayed * fLineHeight : fHeight - fLineHeight;
            mLines[uIndex].SetPosition(0.0f, fY);
            mLines[uIndex].HUD::Element::Enable(true);
            mState = state_EnablingLine;
            return;
        }
    }
    mState = state_Waiting;
}

// FUNCTION: SWBF2 0x005576e0
void HUD::ElementMultilineText::DisableAndScroll(Line& line, unsigned int uIndex)
{
    if (mNumLinesDisplayed &&
        (mAlwaysScroll ||
         (mLinesPending && (!mAddToTop || mNumLinesDisplayed == mMaxDisplayLines))))
    {
        float fHoldElapsed;
        if (line.mFader.GetPhase() < HUD::Envelope::phase_Hold)
        {
            fHoldElapsed = 0.0f;
        }
        else if (line.mFader.GetPhase() == HUD::Envelope::phase_Hold)
        {
            fHoldElapsed = line.mFader.GetElapsed();
        }
        else
        {
            fHoldElapsed = FLT_MAX;
        }
        if (fHoldElapsed > mDisplayInterval)
        {
            if (line.IsEnabled() && line.GetPosition().y <= 0.1f)
            {
                line.HUD::Element::Enable(false);
                mState = state_DisablingLine;
            }
            else
            {
                mState = state_Scrolling;
            }
            return;
        }
    }
    EnableNextLine(uIndex);
}

// FUNCTION: SWBF2 0x005579a0
void HUD::ElementMultilineText::Update(const float fElapsed)
{
    unsigned int uNext;
    if (mNextLine >= mLinesPending)
    {
        uNext = mNextLine - mLinesPending;
    }
    else
    {
        uNext = mNextLine + (mNumLines - mLinesPending);
    }
    unsigned int uEldest;
    if (uNext >= mNumLinesDisplayed)
    {
        uEldest = uNext - mNumLinesDisplayed;
    }
    else
    {
        uEldest = mNumLines - mNumLinesDisplayed + uNext;
    }

    switch (mState)
    {
    case state_Waiting:
        if (mAlwaysScroll || mNumLinesDisplayed == mMaxDisplayLines || mLinesPending ||
            !mNumLinesDisplayed)
        {
            DisableAndScroll(mLines[uEldest], uNext);
        }
        break;

    case state_DisablingLine:
        if (mLines[uEldest].mEnableState == enableState_Disabled)
        {
            mLines[uEldest].SetText((const unsigned short*)0, true);
            if (mNumLinesDisplayed && --mNumLinesDisplayed == 0 && mDisableOnEmpty)
            {
                HUD::Element::Enable(false);
            }
            mState = state_Scrolling;
        }
        break;

    case state_Scrolling:
    {
        const float fHeight = (float)(int)GetHeightPixels();
        const int iLineHeight = (int)mFormat.GetTextHeight();
        if (!GetTop() || TopLineAtTop() ||
            (!mAddToTop && mLinesPending &&
             BottomLineOutOfBottomRegion(fHeight, (float)iLineHeight)) ||
            !mLines[uEldest].IsEnabled())
        {
            EnableNextLine(uNext);
            break;
        }
        bool bAtTop = false;
        if (fElapsed == 0.0f)
        {
            break;
        }

        const float fTopY = mLines[uEldest].GetPosition().y;
        float fScroll = mScrollSpeed * fElapsed;
        if (!(fScroll < 1.0f))
        {
            fScroll = 1.0f;
        }
        float fDelta = (float)iLineHeight * fScroll;
        if (fDelta > fTopY)
        {
            fDelta = fTopY;
            bAtTop = true;
        }
        for (unsigned int i = 0; i < mNumLinesDisplayed; ++i)
        {
            Line& rLine = mLines[uEldest];
            rLine.SetPosition(rLine.GetPosition().x, rLine.GetPosition().y - fDelta);
            if (++uEldest == mNumLines)
            {
                uEldest = 0;
            }
        }
        if (bAtTop)
        {
            EnableNextLine(uNext);
        }
        break;
    }

    case state_EnablingLine:
        switch (mLines[uNext].mEnableState)
        {
        case enableState_Enabled:
            --mLinesPending;
            ++mNumLinesDisplayed;
            mState = state_Waiting;
            break;
        case enableState_Disabling:
        case enableState_Disabled:
            mLines[uNext].HUD::Element::Enable(true);
            break;
        }
        break;
    }

    HUD::ElementGroup::Update(fElapsed);

    if (mEnableState == enableState_Disabled && (mNumLinesDisplayed || mLinesPending))
    {
        for (unsigned int i = 0; i < mNumLines; ++i)
        {
            mLines[i].mFader.SetPhaseTime(HUD::Envelope::phase_Release, 0.0f);
            mLines[i].HUD::Element::Enable(false);
            mLines[i].Update(0.0001f);
            mLines[i].mFader.SetPhaseTime(HUD::Envelope::phase_Release,
                                          mFormat.mFader.GetPhaseTime(HUD::Envelope::phase_Release));
        }
        mNumLinesDisplayed = 0;
        mLinesPending = 0;
        mNextLine = 0;
    }
}

// FUNCTION: SWBF2 0x00557d30
void HUD::ElementMultilineText::RefreshTextBoxSize(unsigned int uLineHeight, unsigned int uWidth,
                                                   unsigned int uHeight)
{
    mFormat.RedTextElement::SetTextBox(uWidth, uHeight);
    mMaxDisplayLines = mFormat.RedTextElement::GetMaxLines(uLineHeight);
}

// FUNCTION: SWBF2 0x00557d70
void HUD::ElementMultilineText::FreeLines()
{
    if (mLines)
    {
        for (unsigned int i = 0; i < mNumLines; ++i)
        {
            HUD::ElementGroupBase::RemoveElement(&mLines[i]);
        }
        delete[] mLines;
        mLines = 0;
        mLinesPending = 0;
        mNumLines = 0;
        mNumLinesDisplayed = 0;
        mNextLine = 0;
    }
}

// FUNCTION: SWBF2 0x00557e10
void HUD::ElementMultilineText::RefreshFormatting(bool bAddToGroup)
{
    const unsigned int uWidth = GetWidthPixels();
    const unsigned int uLineHeight = mFormat.GetTextHeight();
    for (unsigned int i = 0; i < mNumLines; ++i)
    {
        mLines[i].mDynamicAlloc = false;
        if (bAddToGroup)
        {
            HUD::ElementGroupBase::AddElement(&mLines[i]);
        }
        mLines[i].mWriteEnabled = false;
        mLines[i] = mFormat;
        mLines[i].SetPosition(0.0f, 0.0f);
        mLines[i].SetTextBox(uWidth, uLineHeight);
    }
}

// FUNCTION: SWBF2 0x00557f10
void HUD::ElementMultilineText::SetupLines(unsigned int uNumLines)
{
    const unsigned int uWidth = GetWidthPixels();
    const unsigned int uHeight = GetHeightPixels();
    const unsigned int uLineHeight = mFormat.GetTextHeight();
    RefreshTextBoxSize(uLineHeight, uWidth, uHeight);
    const bool bChanged = (uNumLines != mNumLines);
    if (bChanged)
    {
        FreeLines();
        mLines = new Line[uNumLines];
        if (mLines)
        {
            RedTextElement::TextHAlignT eHAlign;
            RedTextElement::TextVAlignT eVAlign;
            mFormat.GetTextAlign(eHAlign, eVAlign);
            mNumLines = uNumLines;
        }
    }
    RefreshFormatting(bChanged);
}

// FUNCTION: SWBF2 0x00558070
bool HUD::ElementMultilineText::AddText(const unsigned short* const pText,
                                        const RedColor* const pColor)
{
    const unsigned int uLineHeight = mFormat.GetTextHeight();
    unsigned int uMaxLines = 80;
    RedStack stack;
    const unsigned int uFreeLines = mNumLines - mNumLinesDisplayed - mLinesPending;

    unsigned short* pcText =
        (unsigned short*)stack.Alloc((PblUnicode::ustrlen(pText) + 1) * sizeof(unsigned short));
    if (!pcText)
    {
        return false;
    }
    memcpy(pcText, pText, (PblUnicode::ustrlen(pText) + 1) * sizeof(unsigned short));

    unsigned int uWidth;
    unsigned int uHeight;
    mFormat.GetTextBox(uWidth, uHeight);
    mFormat.SetTextBox(uWidth, mNumLines * uLineHeight);
    RedTextElement::TextLine aLines[80];
    unsigned int uNumLines;
    const unsigned short* pcEnd;
    short sCursorX;
    mFormat.ClipString(pcText, aLines, uMaxLines, uNumLines, pcEnd, uLineHeight, sCursorX);
    mFormat.SetTextBox(uWidth, uHeight);

    if (uNumLines > uFreeLines)
    {
        const float fLineHeight = (float)(int)mFormat.GetTextHeight();
        for (unsigned int i = uFreeLines; i < uNumLines; ++i)
        {
            unsigned int uNext;
            if (mNextLine >= mLinesPending)
            {
                uNext = mNextLine - mLinesPending;
            }
            else
            {
                uNext = mNextLine + (mNumLines - mLinesPending);
            }
            unsigned int uEldest;
            if (uNext >= mNumLinesDisplayed)
            {
                uEldest = uNext - mNumLinesDisplayed;
            }
            else
            {
                uEldest = mNumLines - mNumLinesDisplayed + uNext;
            }
            mLines[uEldest].SetText((const unsigned short*)0, true);
            mLines[uEldest].HUD::Element::Enable(false);
            mLines[uEldest].m_bEnabled = false;
            if (mNumLinesDisplayed)
            {
                --mNumLinesDisplayed;
            }
            else if (mLinesPending)
            {
                --mLinesPending;
            }

            unsigned int uIndex;
            if (uNext >= mNumLinesDisplayed)
            {
                uIndex = uNext - mNumLinesDisplayed;
            }
            else
            {
                uIndex = mNumLines - mNumLinesDisplayed + uNext;
            }
            for (unsigned int j = 0; j < mNumLinesDisplayed; ++j)
            {
                Line& rLine = mLines[uIndex];
                rLine.SetPosition(rLine.GetPosition().x, rLine.GetPosition().y - fLineHeight);
                if (++uIndex == mNumLines)
                {
                    uIndex = 0;
                }
            }
        }
    }

    for (unsigned int i = 0; i < uNumLines; ++i)
    {
        unsigned short acLine[257];
        unsigned int uBytes = (unsigned int)((const char*)aLines[i].pcEnd -
                                             (const char*)aLines[i].pcStart);
        if (uBytes > sizeof(acLine))
        {
            uBytes = sizeof(acLine) - sizeof(unsigned short);
        }
        memcpy(acLine, aLines[i].pcStart, uBytes);
        acLine[uBytes / sizeof(unsigned short)] = 0;

        mLines[mNextLine].SetColor(mFormat.m_color);
        mLines[mNextLine].mColorDefault = RedColor(mFormat.m_color);
        mLines[mNextLine].mAlpha = mFormat.mAlpha;
        mLines[mNextLine].SetText(acLine, false);
        ++mLinesPending;
        if (++mNextLine == mNumLines)
        {
            mNextLine = 0;
        }
        if (uNumLines > uFreeLines)
        {
            unsigned int uPending;
            if (mNextLine >= mLinesPending)
            {
                uPending = mNextLine - mLinesPending;
            }
            else
            {
                uPending = mNextLine + (mNumLines - mLinesPending);
            }
            EnableNextLine(uPending);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00558480
bool HUD::ElementMultilineText::SetProperty(const unsigned int uHashID, const void* pValue)
{
    switch (uHashID)
    {
    case kNumLines:
        SetupLines(*(const unsigned int*)pValue);
        return true;
    case kDisplayTime:
        mDisplayInterval = *(const float*)pValue;
        return true;
    case kAlwaysScroll:
        mAlwaysScroll = *(const bool*)pValue;
        return true;
    case kAddToTop:
        mAddToTop = *(const bool*)pValue;
        return true;
    case kDisableOnEmpty:
        mDisableOnEmpty = *(const bool*)pValue;
        return true;
    case kScrollSpeed:
        mScrollSpeed = *(const float*)pValue;
        return true;
    case kEventText:
        Item::SetEvent(*(const char* const*)pValue, mEventText);
        return true;
    case kEventColor:
        Item::SetEvent(*(const char* const*)pValue, mEventColor);
        return true;
    }
    return HUD::ElementGroup::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00558580
bool HUD::ElementMultilineText::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kNumLines:
        *(unsigned int*)pValue = mNumLines;
        return true;
    case kDisplayTime:
        *(float*)pValue = mDisplayInterval;
        return true;
    case kAlwaysScroll:
        *(bool*)pValue = mAlwaysScroll;
        return true;
    case kAddToTop:
        *(bool*)pValue = mAddToTop;
        return true;
    case kDisableOnEmpty:
        *(bool*)pValue = mDisableOnEmpty;
        return true;
    case kScrollSpeed:
        *(float*)pValue = mScrollSpeed;
        return true;
    case kEventText:
        *(const char**)pValue = mEventText.GetRegisteredClassName(false);
        return true;
    case kEventColor:
        *(const char**)pValue = mEventColor.GetRegisteredClassName(false);
        return true;
    }
    return HUD::ElementGroup::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00558680
void HUD::ElementMultilineText::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                     void* const pUserData)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            if (mFormat.GetNameHash() == data.GetId())
            {
                PblConfig scope(config.ReadNextScope(), true);
                mFormat.Read(scope, 0, 0);
            }
            else
            {
                ReadChild(config, data, pCallback, pUserData);
            }
        }
        else
        {
            ReadData(config, data);
        }
    }
    PostReadSetup();
}

// FUNCTION: SWBF2 0x00558760
bool HUD::ElementMultilineText::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kNumLines:
        mNumLines = (int)data.GetFloatArg(0);
        return true;
    case kDisplayTime:
        mDisplayInterval = data.GetFloatArg(0);
        return true;
    case kAlwaysScroll:
        mAlwaysScroll = data.GetBoolArg(0);
        return true;
    case kAddToTop:
        mAddToTop = data.GetBoolArg(0);
        return true;
    case kDisableOnEmpty:
        mDisableOnEmpty = data.GetBoolArg(0);
        return true;
    case kScrollSpeed:
        mScrollSpeed = data.GetFloatArg(0);
        return true;
    case kEventText:
        Item::ReadEvent(data, mEventText);
        return true;
    case kEventColor:
        Item::ReadEvent(data, mEventColor);
        return true;
    }
    return HUD::ElementGroup::ReadData(config, data);
}

// FUNCTION: SWBF2 0x005588b0
void HUD::ElementMultilineText::PostReadSetup()
{
    HUD::ElementGroup::PostReadSetup();
    const unsigned int uNumLines = mNumLines;
    mNumLines = 0;
    SetupLines(uNumLines);

    char acName[256];
    _snprintf(acName, sizeof(acName), "%s.%s", GetName(), mFormat.GetName());
    mFormat.SetDisplayName(acName);
}

// FUNCTION: SWBF2 0x00558920
void HUD::ElementMultilineText::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("EventText", uTabs, mEventText.GetRegisteredClassName(false), "");
    file.WriteProperty("EventColor", uTabs, mEventColor.GetRegisteredClassName(false), "");
    file.WriteProperty("NumLines", uTabs, (int)mNumLines, 0);
    file.WriteProperty("DisplayTime", uTabs, mDisplayInterval, 2.0f);
    file.WriteProperty("AlwaysScroll", uTabs, (int)mAlwaysScroll, 0);
    file.WriteProperty("DisableOnEmpty", uTabs, (int)mDisableOnEmpty, 0);
    file.WriteProperty("ScrollSpeed", uTabs, mScrollSpeed, 1.0f);
    file.WriteProperty("AddToTop", uTabs, (int)mAddToTop, 0);
    HUD::ElementGroup::WriteData(file, uTabs);
    mFormat.Write(file, uTabs, "Format");
}

// FUNCTION: SWBF2 0x00558a40
void HUD::ElementMultilineText::EventText(const HUD::Event& event, void* const pData)
{
    unsigned short acNewline[2];
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Uint:
        ((ElementMultilineText*)pData)->AddText(RedLocalizeDB::Find(event.GetData().uintValue, false), 0);
        break;
    case EventClass::type_String:
    {
        const unsigned short* pText = event.GetData().string;
        if (pText)
        {
            ((ElementMultilineText*)pData)->AddText(pText, 0);
        }
        else
        {
            PblUnicode::usnprintf(acNewline, 2, "\n");
            ((ElementMultilineText*)pData)->AddText(acNewline, 0);
        }
        break;
    }
    default:
        return;
    }
    Element::EventChanged(event, pData);
}

// FUNCTION: SWBF2 0x00558ad0
void HUD::ElementMultilineText::EventColor(const HUD::Event& event, void* const pData)
{
    ElementMultilineText* pDisplay = (ElementMultilineText*)pData;
    if (event.GetClass()->GetType() == EventClass::type_Color)
    {
        pDisplay->mFormat.SetColor(*event.GetData().color);
        pDisplay->mFormat.mColorDefault = *event.GetData().color;
        pDisplay->mFormat.mAlpha = (float)event.GetData().color->a / 255.0f;
    }
}

// FUNCTION: SWBF2 0x00557110
void* HUD::ElementText::GetParent()
{
    return this;
}
