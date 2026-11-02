#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <math.h>
#include <d3dx9math.h>

#include "HUDElement.h"
#include "HUDElementGroup.h"
#include "CameraManager.h"
#include "HUDManager.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedGroupElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

// FUNCTION: SWBF2 0x00549110
PblMatrix HUD::Element::CalculateGroupMatrix(HUD::ElementGroupBase* const pGroup) const
{
    const HUD::Element* pElement = pGroup;
    PblMatrix result;
    HUD::ElementGroupBase* const pParent = pElement->mGroup;
    const PblMatrix local = pElement->mElement->GetMatrix();
    if (pParent)
    {
        result = CalculateGroupMatrix(pParent);
        *(D3DXMATRIX*)&result = *(const D3DXMATRIX*)&local * *(const D3DXMATRIX*)&result;
    }
    else
    {
        result = local;
    }
    return result;
}

// GLOBAL: SWBF2 0x007e6910

char* HUD::gRelativeModeStrings[4] =
{
    "Pixels", "Screen", "Viewport", "Frame"
};

// GLOBAL: SWBF2 0x007e6908

char* HUD::Element::Factory::sBlendModeStrings[2] =
{
    "Alpha", "Additive"
};

// GLOBAL: SWBF2 0x01e56c34

float HUD::Element::sViewportWidth;

// GLOBAL: SWBF2 0x01e56c30

float HUD::Element::sViewportHeight;

// SYNTHETIC: SWBF2 0x00403fb0
// `dynamic initializer for 'HUD::Element::sList''
// SYNTHETIC: SWBF2 0x00768b20
// `dynamic atexit destructor for 'HUD::Element::sList''

// GLOBAL: SWBF2 0x007eba18
// HUD::Element::sList
PblListDouble HUD::Element::sList;

// GLOBAL: SWBF2 0x007e6904
float HUD::Element::sRuntimeScale = 1.0f;

static const unsigned int kPosition = 0x934f4e0a;
static const unsigned int kPositionRelative = 0xf9f41ffa;
static const unsigned int kRotation = 0x21ac415f;
static const unsigned int kScale = 0x82971c71;
static const unsigned int kScaleUniform = 0x745450bf;
static const unsigned int kZOrder = 0xec9f263f;
static const unsigned int kAlpha = 0x5d8b6dab;
static const unsigned int kColor = 0x3d7e6258;
static const unsigned int kColorChange = 0x3c656d30;
static const unsigned int kColorChangeRate = 0xb4660872;
static const unsigned int kColorPulseRate = 0xe459101b;
static const unsigned int kUseChangeColor = 0x849604eb;
static const unsigned int kFadeInTime = 0x5083ecd9;
static const unsigned int kFadeHoldTime = 0x8e3e2923;
static const unsigned int kFadeSustainTime = 0xff48a8f9;
static const unsigned int kFadeOutTime = 0x798dc484;
static const unsigned int kBlendMode = 0xfa784eab;
static const unsigned int kViewport = 0xe4abbac3;
static const unsigned int kEventEnable = 0xd23096a8;
static const unsigned int kEventDisable = 0x7b5f76b1;
static const unsigned int kEventChanged = 0x0d1d42cd;
static const unsigned int kEventColor = 0x49f429ae;
static const unsigned int kEventPulseRate = 0xccd945ca;
static const unsigned int kScalable = 0x0dd385e2;
static const unsigned int kScaleAll = 0x6db146a8;

static const unsigned int kEventFadeOut = 0xb8e23cbb;
static const unsigned int kEditOnly = 0xca0b9ccb;
static const unsigned int kAnimEnable = 0xbdbfd84f;
static const unsigned int kAnimDisable = 0x6242102c;
static const unsigned int kAnimChange = 0x3455b0a8;



// FUNCTION: SWBF2 0x00547010
float HUD::ConvertRelativeToPixels(RelativeMode eMode, float fValue, float fFrameSize,
                                   float fViewSize, float fScreenSize)
{
    switch (eMode)
    {
    case relativeMode_Pixels:
        return fValue;
    case relativeMode_Viewport:
        return fValue * fViewSize;
    case relativeMode_Screen:
        return fValue * fScreenSize;
    case relativeMode_Frame:
        return fValue * fFrameSize;
    }
    return fValue;
}

// FUNCTION: SWBF2 0x00547050
float HUD::ConvertPixelsToRelative(RelativeMode eMode, float fValue, float fFrameSize,
                                   float fViewSize, float fScreenSize)
{
    switch (eMode)
    {
    case relativeMode_Viewport:
        return fViewSize != 0.0f ? fValue / fViewSize : 0.0f;
    case relativeMode_Screen:
        return fScreenSize != 0.0f ? fValue / fScreenSize : 0.0f;
    case relativeMode_Frame:
        return fFrameSize != 0.0f ? fValue / fFrameSize : 0.0f;
    case relativeMode_Pixels:
        return fValue;
    }
    return fValue;
}

// FUNCTION: SWBF2 0x005470c0
float HUD::ConvertXForWideScreen(float fX, float fWidth, float fHeight)
{
    float fInvAspect = 1.0f / RedRenderer::GetScreenAspectRatio();
    float fCenter = fWidth * 0.5f;
    float fScale = (float)RedInterfaceScreen::GetScreenHeight() * fInvAspect
                 / (float)RedInterfaceScreen::GetScreenWidth();
    return fScale * (fX - fCenter) + fCenter;
}

// FUNCTION: SWBF2 0x00547150
float HUD::ConvertWideScreenX(float fX, float fWidth, float fHeight)
{
    float fInvAspect = 1.0f / RedRenderer::GetScreenAspectRatio();
    float fCenter = Element::sViewportWidth * 0.5f;
    float fScale = (float)RedInterfaceScreen::GetScreenHeight() * fInvAspect
                 / (float)RedInterfaceScreen::GetScreenWidth();
    return (fX - fCenter) / fScale + fCenter;
}

// FUNCTION: SWBF2 0x005471e0
HUD::Matrix::Matrix()
    : mMatrix(0),
      mRelativeMode(relativeMode_Pixels)
{
    mScale.SetAll(1.0f);
    mRotation.SetAll(0.0f);
}

// FUNCTION: SWBF2 0x00547220
void HUD::Matrix::ApplyRelativeMode(float fFrameWidth, float fFrameHeight, float fViewWidth,
                                    float fViewHeight, bool bWideScreen)
{
    PblVector3 vPos;
    vPos = mMatrix->GetTranslation();
    vPos.x = ConvertRelativeToPixels(mRelativeMode, vPos.x, fFrameWidth, fViewWidth,
                                     (float)(int)RedInterfaceScreen::GetScreenWidth());
    vPos.y = ConvertRelativeToPixels(mRelativeMode, vPos.y, fFrameHeight, fViewHeight,
                                     (float)(int)RedInterfaceScreen::GetScreenHeight());
    if (bWideScreen && CameraManager::sInstance->m_iNumCam == 1)
    {
        vPos.x = ConvertXForWideScreen(vPos.x, fViewWidth, fViewHeight);
    }
    float* pRow = mMatrix->m[3];
    *(PblVector3*)pRow = vPos;
    pRow[3] = 1.0f;
}

// FUNCTION: SWBF2 0x005472e0
bool HUD::Matrix::ReadPosition(PblConfig::Data& data, unsigned int uHashID)
{
    if (uHashID == 0)
    {
        uHashID = data.GetId();
    }
    if (uHashID != kPosition)
    {
        return false;
    }

    int iMode;
    if (data.GetNumArgs() == 4)
    {
        if (data.IsStringArg(3))
        {
            iMode = (int)TranslateString((const char* const*)gRelativeModeStrings, 4,
                                    data.GetStringArg(3));
        }
        else
        {
            iMode = data.GetBoolArg(3) ? relativeMode_Viewport : relativeMode_Pixels;
        }
        iMode = iMode < 0 ? 0 : iMode;
    }
    else
    {
        iMode = relativeMode_Pixels;
    }
    const PblVector3 vPosition(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
    mRelativeMode = (RelativeMode)iMode;

    PblMatrix& matrix = *mMatrix;
    *(PblVector3*)&matrix.m[3][0] = vPosition;
    matrix.m[3][3] = 1.0f;
    return true;
}

// FUNCTION: SWBF2 0x005473b0
void HUD::Matrix::WritePosition(HUD::File& file, unsigned int uTabs, float fFrameWidth,
                                float fFrameHeight, float fViewWidth, float fViewHeight,
                                const char* const pName, bool bWideScreen)
{
    PblVector3 vPos;
    vPos = mMatrix->GetTranslation();
    if (vPos.x == 0.0f && vPos.y == 0.0f && vPos.z == 0.0f)
    {
        return;
    }

    if (bWideScreen)
    {
        vPos.x = ConvertWideScreenX(vPos.x, fViewWidth, fViewHeight);
    }
    file.WriteTabs(uTabs);
    file.WriteStringf("%s(%f, %f, %f, \"%s\")\n", pName ? pName : "Position",
                      ConvertPixelsToRelative(mRelativeMode, vPos.x, fFrameWidth, fViewWidth,
                                              (float)(int)RedInterfaceScreen::GetScreenWidth()),
                      ConvertPixelsToRelative(mRelativeMode, vPos.y, fFrameHeight, fViewHeight,
                                              (float)(int)RedInterfaceScreen::GetScreenHeight()),
                      vPos.z, gRelativeModeStrings[mRelativeMode]);
}

// FUNCTION: SWBF2 0x005474d0
bool HUD::Matrix::ReadRotation(PblConfig::Data& data, unsigned int)
{
    if (data.GetId() != kRotation)
    {
        return false;
    }
    SetRotation(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
    return true;
}

// FUNCTION: SWBF2 0x00547500
void HUD::Matrix::WriteRotation(HUD::File& file, unsigned int uTabs, const char* const pName)
{
    file.WriteProperty("Rotation", uTabs, mRotation.x, mRotation.y, mRotation.z,
                       0.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x00547550
bool HUD::Matrix::ReadScale(PblConfig::Data& data, unsigned int)
{
    if (data.GetId() != kScale)
    {
        return false;
    }
    SetScale(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
    return true;
}

// FUNCTION: SWBF2 0x00547580
void HUD::Matrix::WriteScale(HUD::File& file, unsigned int uTabs, const char* const pName)
{
    file.WriteProperty("Scale", uTabs, mScale.x, mScale.y, mScale.z, 1.0f, 1.0f, 1.0f);
}

// FUNCTION: SWBF2 0x005475d0
void HUD::Matrix::SetMatrix(PblMatrix* const pMatrix)
{
    mMatrix = pMatrix;
}

// FUNCTION: SWBF2 0x005475e0
const PblMatrix* HUD::Matrix::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x005475f0
void HUD::Matrix::SetRotation(float fX, float fY, float fZ)
{
    while (fX >= 360.0f)
    {
        fX -= 360.0f;
    }
    while (fX < 0.0f)
    {
        fX += 360.0f;
    }
    while (fY >= 360.0f)
    {
        fY -= 360.0f;
    }
    while (fY < 0.0f)
    {
        fY += 360.0f;
    }
    while (fZ >= 360.0f)
    {
        fZ -= 360.0f;
    }
    while (fZ < 0.0f)
    {
        fZ += 360.0f;
    }

    PblMatrix matrix = gMatrixIdentity;
    matrix.RotateX(fX * 0.017453292f);
    matrix.RotateY(fY * 0.017453292f);
    matrix.RotateZ(fZ * 0.017453292f);

    const PblVector3 vPosition = mMatrix->GetTranslation();
    *mMatrix = matrix;
    mMatrix->SetTranslation(vPosition);

    mRotation.x = fX;
    mRotation.y = fY;
    mRotation.z = fZ;
}

// FUNCTION: SWBF2 0x005477e0
void HUD::Matrix::ApplyRotation()
{
    SetRotation(mRotation.x, mRotation.y, mRotation.z);
}

// FUNCTION: SWBF2 0x00547800
void HUD::Matrix::SetScale(float fX, float fY, float fZ)
{
    mScale.x = fX;
    mScale.y = fY;
    mScale.z = fZ;
}

// FUNCTION: SWBF2 0x00547810
const PblVector3& HUD::Matrix::GetScale() const
{
    return mScale;
}

// FUNCTION: SWBF2 0x00547820
void HUD::Matrix::ApplyScale()
{
    const PblVector3 vPosition = mMatrix->GetTranslation();

    PblVector3 vAxis;
    D3DXVec3Normalize((D3DXVECTOR3*)&vAxis, (const D3DXVECTOR3*)mMatrix->m[0]);
    *(PblVector3*)mMatrix->m[0] = vAxis;
    mMatrix->m[0][3] = 0.0f;
    D3DXVec3Normalize((D3DXVECTOR3*)&vAxis, (const D3DXVECTOR3*)mMatrix->m[1]);
    *(PblVector3*)mMatrix->m[1] = vAxis;
    mMatrix->m[1][3] = 0.0f;
    D3DXVec3Normalize((D3DXVECTOR3*)&vAxis, (const D3DXVECTOR3*)mMatrix->m[2]);
    *(PblVector3*)mMatrix->m[2] = vAxis;
    mMatrix->m[2][3] = 0.0f;

    PblMatrix scaling;
    D3DXMatrixMultiply((D3DXMATRIX*)mMatrix, (D3DXMATRIX*)mMatrix,
                       D3DXMatrixScaling((D3DXMATRIX*)&scaling, mScale.x, mScale.y, mScale.z));

    mMatrix->SetTranslation(vPosition);
}

// FUNCTION: SWBF2 0x00547930
HUD::Matrix& HUD::Matrix::operator=(const Matrix& other)
{
    mScale = other.mScale;
    mRelativeMode = other.mRelativeMode;
    mRotation = other.mRotation;
    return *this;
}

// FUNCTION: SWBF2 0x00547960
const PblVector3& HUD::Matrix::GetRotation() const
{
    return mRotation;
}

// FUNCTION: SWBF2 0x00547970
void HUD::Matrix::SetRelativeMode(RelativeMode eMode)
{
    mRelativeMode = eMode;
}

// FUNCTION: SWBF2 0x00547980
HUD::RelativeMode HUD::Matrix::GetRelativeMode() const
{
    return mRelativeMode;
}

// FUNCTION: SWBF2 0x00547990
HUD::Element::Factory::Factory(unsigned int uHashID)
    : HUD::Item::Factory(uHashID)
{
    mType = type_Element;
    mName = "Element";

    mPosition.Init("Position", mProperties, Property::type_Vector3);
    mRelative.Init("PositionRelative", mProperties, HUD::gRelativeModeStrings, 0, 3);
    mRotation.Init("Rotation", mProperties, Property::type_Vector3);
    mScale.Init("Scale", mProperties, Property::type_Vector3);
    mScaleUniform.Init("ScaleUniform", mProperties, 0.0f, FLT_MAX, 0.01f);
    mScale.InitVectorRange(Property::axis_X, 0.0f, FLT_MAX, 0.1f);
    mScale.InitVectorRange(Property::axis_Y, 0.0f, FLT_MAX, 0.1f);
    mScale.InitVectorRange(Property::axis_Z, 0.0f, FLT_MAX, 0.1f);
    mZOrder.Init("ZOrder", mProperties, 0, 255);
    mAlpha.Init("Alpha", mProperties, 0.0f, 1.0f, 0.01f);
    mColor.Init("Color", mProperties, Property::type_Color);
    mColorChange.Init("ColorChange", mProperties, Property::type_Color);
    mColorChangeRate.Init("ColorChangeRate", mProperties, 0.0f, FLT_MAX, 0.001f);
    mColorPulseRate.Init("ColorPulseRate", mProperties, 0.0f, FLT_MAX, 0.001f);
    mUseChangeColor.Init("UseChangeColor", mProperties, Property::type_Bool);
    mFadeInTime.Init("FadeInTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mFadeHoldTime.Init("FadeHoldTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mFadeSustainTime.Init("FadeSustainTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mFadeOutTime.Init("FadeOutTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mBlendMode.Init("BlendMode", mProperties, sBlendModeStrings, 0, 1);
    mViewport.Init("Viewport", mProperties, 0, 4);
    mEventEnable.Init("EventEnable", mProperties, Property::type_Event);
    mEventDisable.Init("EventDisable", mProperties, Property::type_Event);
    mEventChanged.Init("EventChanged", mProperties, Property::type_Event);
    mEventColor.Init("EventColor", mProperties, Property::type_Event);
    mEventPulseRate.Init("EventPulseRate", mProperties, Property::type_Event);
    mScalable.Init("Scalable", mProperties, Property::type_Bool);
    mScaleAll.Init("ScaleAll", mProperties, 0.001f, FLT_MAX, 0.01f);
}

// FUNCTION: SWBF2 0x00547e60
HUD::Element::Element(RedInterfaceElement& element, const char* const pName,
                      Factory::ItemCallback pCallback, void* const pUserData)
    : HUD::Item(pName, 0, 0),
      mEventFadeOut(0),
      mEnableState(enableState_Disabled),
      mAlpha(1.0f),
      mElement(&element),
      mColorDefault(RedColor::WHITE),
      mColorChanged(RedColor::WHITE)
{
    mColorIntRate = 0.3f;
    mPulseColorRate = FLT_MAX;
    mPulseTime = FLT_MAX;
    mViewport = 0;
    mGroup = 0;
    mRuntimeScale = 1.0f;
    mUseChangeColor = false;
    mFaderChanged = false;
    mAnimChanged = false;
    mHUDEditOnly = false;
    mDynamicAlloc = true;
    mScalable = false;

    mEventEnable.Init(EventEnable, this);
    mEventDisable.Init(EventDisable, this);
    mEventChanged.Init(EventChanged, this);
    mEventColor.Init(EventColor, this);
    mEventPulseRate.Init(EventPulseRate, this);

    sList.AddLast(&mNode);

    if (pCallback)
    {
        pCallback(this, pUserData);
    }
}

// FUNCTION: SWBF2 0x005480b0
HUD::Element::~Element()
{
    if (mElement)
    {
        if (mGroup)
        {
            mGroup->RemoveElement(this);
        }
        else
        {
            RedGroupElement* pGroup = mElement->GetGroup();
            if (pGroup)
            {
                pGroup->RemoveElement(mElement);
            }
        }
    }
}

// FUNCTION: SWBF2 0x005481c0
void HUD::Element::SetViewport(unsigned char uViewport)
{
    if (mGroup)
    {
        mViewport = mGroup->GetViewport();
    }
    else
    {
        mViewport = uViewport;
    }
}

// FUNCTION: SWBF2 0x005481f0
unsigned char HUD::Element::GetViewport() const
{
    return mGroup ? mGroup->GetViewport() : mViewport;
}

// FUNCTION: SWBF2 0x00548210
void HUD::Element::Update(const float fTime)
{
    Envelope::Phase ePrevious = mFader.GetPhase();
    Envelope::Phase ePhase;
    if (mFaderChanged)
    {
        ePhase = ePrevious;
    }
    else
    {
        ePhase = mFader.Update(fTime);
    }
    mAnimChanged = mFaderChanged = mFaderChanged | (ePrevious != ePhase);

    if (mFaderChanged && mAnimChanged)
    {
        EnableState eState = enableState_Invalid;
        switch (ePhase)
        {
        case Envelope::phase_Inactive:
            eState = enableState_Disabled;
            break;
        case Envelope::phase_Attack:
            eState = enableState_Enabling;
            break;
        case Envelope::phase_Hold:
            eState = enableState_Enabled;
            break;
        case Envelope::phase_Sustain:
        case Envelope::phase_Release:
            eState = enableState_Disabling;
            break;
        }
        if (mEventFadeOut && (eState == enableState_Disabling || eState == enableState_Disabled))
        {
            HUD::Event event(mEventFadeOut, true);
            event.Send();
        }
        UpdateEnableState(eState);
    }

    if (mElement->IsEnabled())
    {
        const RedColor current = mElement->m_color;
        float fBlend = 1.0f;
        if (mColorIntRate != 0.0f)
        {
            fBlend = 1.0f / mColorIntRate * fTime;
            if (fBlend < 0.0f)
            {
                fBlend = 0.0f;
            }
            else if (fBlend > 1.0f)
            {
                fBlend = 1.0f;
            }
        }
        RedColor color;
        color.b = (unsigned char)(int)(mColorDefault.b * fBlend + current.b * (1.0f - fBlend) + 0.5f);
        color.g = (unsigned char)(int)(mColorDefault.g * fBlend + current.g * (1.0f - fBlend) + 0.5f);
        color.r = (unsigned char)(int)(mColorDefault.r * fBlend + current.r * (1.0f - fBlend) + 0.5f);
        color.a = (unsigned char)(int)(mColorDefault.a * fBlend + current.a * (1.0f - fBlend) + 0.5f);
        mElement->m_color = color;

        mPulseTime -= fTime;
        if (mPulseTime <= 0.0f)
        {
            mElement->SetColor(mColorChanged);
            mPulseTime = mPulseColorRate;
        }

        mElement->m_color.a = (unsigned char)(int)(mFader.CalculateValue() * 255.0f * mAlpha);
    }
}

// FUNCTION: SWBF2 0x005484a0
void HUD::Element::Enable(bool bEnable)
{
    UpdateEnableState(bEnable ? enableState_Enabling : enableState_Disabling);
    Update(0.001f);
}

// FUNCTION: SWBF2 0x005484d0
HUD::Element& HUD::Element::operator=(const Element& other)
{
    mAlpha = other.mAlpha;
    mColorDefault = other.mColorDefault;
    mColorChanged = other.mColorChanged;
    mColorIntRate = other.mColorIntRate;
    mPulseColorRate = other.mPulseColorRate;
    SetViewport(other.GetViewport());
    mUseChangeColor = other.mUseChangeColor;
    mFader = other.mFader;
    mElement->SetZOrder((unsigned char)other.mElement->m_zorder);
    mElement->SetBlendMode(other.mElement->GetBlendMode());
    mElement->SetColor(other.mElement->m_color);
    mElement->SetMatrix(other.mElement->m_mLocal);
    mHUDEditOnly = other.mHUDEditOnly;
    return *this;
}

// FUNCTION: SWBF2 0x005485e0
void HUD::Element::SetRuntimeScale(float fScale)
{
    if (!mScalable)
    {
        return;
    }

    PblMatrix matrix = mElement->m_mLocal;
    float afScale[3];
    for (int i = 0; i < 3; ++i)
    {
        const float* pRow = matrix.m[i];
        afScale[i] = PblMath::Sqrt(pRow[0] * pRow[0] + pRow[1] * pRow[1] + pRow[2] * pRow[2])
                   / mRuntimeScale * fScale;
    }
    mRuntimeScale = fScale;

    for (int i = 0; i < 3; ++i)
    {
        PblVector3 vAxis;
        D3DXVec3Normalize((D3DXVECTOR3*)&vAxis, (const D3DXVECTOR3*)matrix.m[i]);
        matrix.m[i][0] = vAxis.x * afScale[i];
        matrix.m[i][1] = vAxis.y * afScale[i];
        matrix.m[i][2] = vAxis.z * afScale[i];
        matrix.m[i][3] = 0.0f;
    }
    mElement->SetMatrix(matrix);
}

// FUNCTION: SWBF2 0x00548800
bool HUD::Element::SetProperty(const unsigned int uHashID, const void* pValue)
{
    const float* pfValue = (const float*)pValue;

    switch (uHashID)
    {
    case kScalable:
        mScalable = *(const bool*)pValue;
        return true;
    case kEventChanged:
        Item::SetEvent(*(const char* const*)pValue, mEventChanged);
        return true;
    case kRotation:
    {
        PblMatrix matrix(mElement->m_mLocal);
        mMatrix.SetMatrix(&matrix);
        mMatrix.SetRotation(pfValue[0], pfValue[1], pfValue[2]);
        mMatrix.ApplyScale();
        mElement->SetMatrix(matrix);
        mMatrix.SetMatrix(0);
        return true;
    }
    case kColorChange:
        mColorChanged = *(const RedColor*)pValue;
        return true;
    case kColor:
    {
        const RedColor* pColor = (const RedColor*)pValue;
        mElement->SetColor(RedColor(*pColor, 0xff));
        mColorDefault = RedColor(mElement->m_color);
        return true;
    }
    case kEventColor:
        Item::SetEvent(*(const char* const*)pValue, mEventColor);
        return true;
    case kAlpha:
        mAlpha = *pfValue;
        return true;
    case kFadeInTime:
        mFader.SetPhaseTime(Envelope::phase_Attack, *pfValue);
        return true;
    case kScaleAll:
        SetRuntimeScaleAll(*pfValue);
        return true;
    case kScaleUniform:
    {
        PblMatrix matrix(mElement->m_mLocal);
        mMatrix.SetMatrix(&matrix);
        mMatrix.ApplyRotation();
        mMatrix.SetScale(*pfValue, *pfValue, *pfValue);
        mMatrix.ApplyScale();
        mElement->SetMatrix(matrix);
        mMatrix.SetMatrix(0);
        return true;
    }
    case kFadeOutTime:
        mFader.SetPhaseTime(Envelope::phase_Release, *pfValue);
        return true;
    case kUseChangeColor:
        mUseChangeColor = *(const bool*)pValue;
        return true;
    case kScale:
    {
        PblMatrix matrix(mElement->m_mLocal);
        mMatrix.SetMatrix(&matrix);
        mMatrix.ApplyRotation();
        mMatrix.SetScale(pfValue[0], pfValue[1], pfValue[2]);
        mMatrix.ApplyScale();
        mElement->SetMatrix(matrix);
        mMatrix.SetMatrix(0);
        return true;
    }
    case kEventDisable:
        Item::SetEvent(*(const char* const*)pValue, mEventDisable);
        return true;
    case kFadeHoldTime:
        mFader.SetPhaseTime(Envelope::phase_Hold, *pfValue);
        return true;
    case kPosition:
        mElement->SetPosition(*(const PblVector3*)pValue);
        return true;
    case kColorChangeRate:
        mColorIntRate = *pfValue;
        return true;
    case kEventPulseRate:
        Item::SetEvent(*(const char* const*)pValue, mEventPulseRate);
        return true;
    case kEventEnable:
        Item::SetEvent(*(const char* const*)pValue, mEventEnable);
        return true;
    case kZOrder:
        mElement->SetZOrder(*(const unsigned char*)pValue);
        return true;
    case kColorPulseRate:
    {
        const float fRate = *pfValue;
        mPulseColorRate = fRate;
        mPulseTime = fRate;
        return true;
    }
    case kPositionRelative:
        mMatrix.SetRelativeMode(*(const RelativeMode*)pValue);
        return true;
    case kViewport:
        return true;
    case kBlendMode:
        mElement->SetBlendMode(*(const RedInterfaceElement::BlendModeT*)pValue);
        return true;
    case kFadeSustainTime:
        mFader.SetPhaseTime(Envelope::phase_Sustain, *pfValue);
        return true;
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x00548c50
bool HUD::Element::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kScalable:
        *(bool*)pValue = mScalable;
        return true;
    case kEventChanged:
        *(const char**)pValue = mEventChanged.GetRegisteredClassName(false);
        return true;
    case kRotation:
        *(PblVector3*)pValue = mMatrix.GetRotation();
        return true;
    case kColorChange:
        *(RedColor*)pValue = mColorChanged;
        return true;
    case kColor:
        *(RedColor*)pValue = mColorDefault;
        return true;
    case kEventColor:
        *(const char**)pValue = mEventColor.GetRegisteredClassName(false);
        return true;
    case kAlpha:
        *(float*)pValue = mAlpha;
        return true;
    case kFadeInTime:
        *(float*)pValue = mFader.GetPhaseTime(Envelope::phase_Attack);
        return true;
    case kScaleAll:
        *(float*)pValue = GetRuntimeScaleAll();
        return true;
    case kScaleUniform:
    {
        PblVector3 vScale = mMatrix.GetScale();
        float fMax = vScale.x > vScale.y ? vScale.x : vScale.y;
        *(float*)pValue = fMax > vScale.z ? fMax : vScale.z;
        return true;
    }
    case kFadeOutTime:
        *(float*)pValue = mFader.GetPhaseTime(Envelope::phase_Release);
        return true;
    case kEventDisable:
        *(const char**)pValue = mEventDisable.GetRegisteredClassName(false);
        return true;
    case kScale:
        *(PblVector3*)pValue = mMatrix.GetScale();
        return true;
    case kUseChangeColor:
        *(bool*)pValue = mUseChangeColor;
        return true;
    case kFadeHoldTime:
        *(float*)pValue = mFader.GetPhaseTime(Envelope::phase_Hold);
        return true;
    case kPosition:
        *(PblVector3*)pValue = mElement->GetPosition();
        return true;
    case kColorChangeRate:
        *(float*)pValue = mColorIntRate;
        return true;
    case kEventPulseRate:
        *(const char**)pValue = mEventPulseRate.GetRegisteredClassName(false);
        return true;
    case kEventEnable:
        *(const char**)pValue = mEventEnable.GetRegisteredClassName(false);
        return true;
    case kPositionRelative:
        *(RelativeMode*)pValue = mMatrix.GetRelativeMode();
        return true;
    case kColorPulseRate:
        *(float*)pValue = mPulseColorRate;
        return true;
    case kViewport:
        *(unsigned int*)pValue = mViewport;
        return true;
    case kZOrder:
        *(unsigned int*)pValue = mElement->m_zorder;
        return true;
    case kBlendMode:
        *(RedInterfaceElement::BlendModeT*)pValue = mElement->GetBlendMode();
        return true;
    case kFadeSustainTime:
        *(float*)pValue = mFader.GetPhaseTime(Envelope::phase_Sustain);
        return true;
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x00548f90
const char* HUD::Element::PropertyTranslateEnum(const unsigned int uHashID, const unsigned int uValue)
{
    switch (uHashID)
    {
    case kPositionRelative:
        return gRelativeModeStrings[uValue];
    case kBlendMode:
        return Factory::sBlendModeStrings[uValue];
    }
    return 0;
}

// FUNCTION: SWBF2 0x00549070
float HUD::Element::GetContainerFrameWidth() const
{
    if (mGroup)
    {
        return mGroup->GetWidthPixelsFloat();
    }
    return GetContainerViewWidth();
}

// FUNCTION: SWBF2 0x00549090
float HUD::Element::GetContainerFrameHeight() const
{
    if (mGroup)
    {
        return mGroup->GetHeightPixelsFloat();
    }
    return GetContainerViewHeight();
}

// FUNCTION: SWBF2 0x005490b0
float HUD::Element::GetContainerViewWidth() const
{
    if (GetViewport())
    {
        return sViewportWidth;
    }
    return (float)RedInterfaceScreen::GetScreenWidth();
}

// FUNCTION: SWBF2 0x005490e0
float HUD::Element::GetContainerViewHeight() const
{
    if (GetViewport())
    {
        return sViewportHeight;
    }
    return (float)RedInterfaceScreen::GetScreenHeight();
}

// FUNCTION: SWBF2 0x00549250
bool HUD::Element::ReadData(PblConfig&, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kScalable:
        mScalable = data.GetBoolArg(0);
        return true;
    case kEventChanged:
        return Item::ReadEvent(data, mEventChanged);
    case kColorChange:
    {
        const RedColor color(data.GetUint8Arg(0), data.GetUint8Arg(1), data.GetUint8Arg(2), 0xff);
        mColorChanged = color;
        return true;
    }
    case kColor:
    {
        const RedColor color(data.GetUint8Arg(0), data.GetUint8Arg(1), data.GetUint8Arg(2), 0xff);
        mElement->SetColor(color);
        mColorDefault = RedColor(mElement->m_color);
        return true;
    }
    case kFadeInTime:
        mFader.SetPhaseTime(Envelope::phase_Attack, data.GetFloatArg(0));
        return true;
    case kEventColor:
        return Item::ReadEvent(data, mEventColor);
    case kAlpha:
        mAlpha = data.GetFloatArg(0);
        return true;
    case kAnimEnable:
    case kAnimDisable:
    case kAnimChange:
        return true;
    case kFadeOutTime:
        mFader.SetPhaseTime(Envelope::phase_Release, data.GetFloatArg(0));
        return true;
    case kFadeHoldTime:
        mFader.SetPhaseTime(Envelope::phase_Hold, data.GetFloatArg(0));
        return true;
    case kFadeSustainTime:
        mFader.SetPhaseTime(Envelope::phase_Sustain, data.GetFloatArg(0));
        return true;
    case kEventDisable:
        return Item::ReadEvent(data, mEventDisable);
    case kUseChangeColor:
        mUseChangeColor = data.GetBoolArg(0);
        return true;
    case kColorChangeRate:
        mColorIntRate = data.GetFloatArg(0);
        return true;
    case kEventFadeOut:
        mEventFadeOut = Item::CreateEventA(data.GetStringArg(0), EventClass::type_Bool);
        return true;
    case kEditOnly:
        mHUDEditOnly = data.GetBoolArg(0);
        return true;
    case kEventPulseRate:
        return Item::ReadEvent(data, mEventPulseRate);
    case kZOrder:
        mElement->SetZOrder(data.GetUint8Arg(0));
        return true;
    case kEventEnable:
        return Item::ReadEvent(data, mEventEnable);
    case kColorPulseRate:
    {
        const float fRate = data.GetFloatArg(0);
        mPulseColorRate = fRate;
        mPulseTime = fRate;
        return true;
    }
    case kViewport:
    {
        int iViewport = data.GetUint8Arg(0);
        if (iViewport < 0)
        {
            iViewport = 0;
        }
        else if (iViewport > 4)
        {
            iViewport = 4;
        }
        SetViewport((unsigned char)iViewport);
        return true;
    }
    case kBlendMode:
    {
        unsigned int uMode = TranslateString((const char* const*)Factory::sBlendModeStrings, 2,
                                        data.GetStringArg(0));
        if (uMode < 2)
        {
            mElement->SetBlendMode((RedInterfaceElement::BlendModeT)uMode);
        }
        return true;
    }
    default:
    {
        PblMatrix matrix = mElement->m_mLocal;
        mMatrix.SetMatrix(&matrix);
        bool bRead = mMatrix.ReadScale(data, 0);
        bRead = mMatrix.ReadRotation(data, 0) | bRead;
        bRead = mMatrix.ReadPosition(data, 0) | bRead;
        if (bRead)
        {
            mElement->SetMatrix(matrix);
        }
        mMatrix.SetMatrix(0);
        return bRead;
    }
    }
}

// FUNCTION: SWBF2 0x00549700
void HUD::Element::PostReadSetup()
{
    PblMatrix matrix = mElement->m_mLocal;
    mMatrix.SetMatrix(&matrix);
    mMatrix.ApplyScale();
    mMatrix.ApplyRelativeMode(GetContainerFrameWidth(), GetContainerFrameHeight(),
                              GetContainerViewWidth(), GetContainerViewHeight(), InScreenGroup());
    PblMatrix result = *mMatrix.GetMatrix();
    mElement->SetMatrix(result);
}

// FUNCTION: SWBF2 0x00549800
void HUD::Element::WriteData(HUD::File& file, unsigned int uTabs)
{
    PblMatrix matrix = mElement->m_mLocal;
    mMatrix.SetMatrix(&matrix);
    file.WriteProperty("EditOnly", uTabs, (int)mHUDEditOnly, 0);
    file.WriteProperty("Viewport", uTabs, (int)GetViewport(), 0);
    mMatrix.WritePosition(file, uTabs, GetContainerFrameWidth(), GetContainerFrameHeight(),
                          GetContainerViewWidth(), GetContainerViewHeight(), 0, InScreenGroup());
    mMatrix.WriteRotation(file, uTabs, 0);
    mMatrix.WriteScale(file, uTabs, 0);
    file.WriteProperty("ZOrder", uTabs, (int)mElement->m_zorder, 128);
    file.WriteProperty("Alpha", uTabs, mAlpha, 1.0f);
    file.WriteProperty("Color", uTabs, mColorDefault.r, mColorDefault.g, mColorDefault.b,
                       255, 255, 255);
    file.WriteProperty("ColorChange", uTabs, mColorChanged.r, mColorChanged.g, mColorChanged.b,
                       255, 255, 255);
    file.WriteProperty("ColorChangeRate", uTabs, mColorIntRate, 0.3f);
    file.WriteProperty("ColorPulseRate", uTabs, mPulseColorRate, FLT_MAX);
    file.WriteProperty("UseChangeColor", uTabs, (int)mUseChangeColor, 0);
    file.WriteProperty("FadeInTime", uTabs, mFader.GetPhaseTime(Envelope::phase_Attack), 0.0f);
    file.WriteProperty("FadeHoldTime", uTabs, mFader.GetPhaseTime(Envelope::phase_Hold), FLT_MAX);
    file.WriteProperty("FadeSustainTime", uTabs, mFader.GetPhaseTime(Envelope::phase_Sustain), 0.0f);
    file.WriteProperty("FadeOutTime", uTabs, mFader.GetPhaseTime(Envelope::phase_Release), 0.0f);
    file.WriteProperty("BlendMode", uTabs, Factory::sBlendModeStrings[mElement->GetBlendMode()],
                       Factory::sBlendModeStrings[0]);
    file.WriteProperty("EventEnable", uTabs, mEventEnable.GetRegisteredClassName(true), "");
    file.WriteProperty("EventDisable", uTabs, mEventDisable.GetRegisteredClassName(true), "");
    file.WriteProperty("EventChanged", uTabs, mEventChanged.GetRegisteredClassName(true), "");
    file.WriteProperty("EventColor", uTabs, mEventColor.GetRegisteredClassName(true), "");
    file.WriteProperty("EventPulseRate", uTabs, mEventPulseRate.GetRegisteredClassName(true), "");
    if (mEventFadeOut)
    {
        file.WriteProperty("EventFadeOut", uTabs, mEventFadeOut->GetName(), "");
    }
    file.WriteProperty("Scalable", uTabs, (int)mScalable, 0);
    mMatrix.SetMatrix(0);
}

// FUNCTION: SWBF2 0x00549be0
void HUD::Element::ResetFlags()
{
    mFaderChanged = false;
    mAnimChanged = false;
}

// FUNCTION: SWBF2 0x00549bf0
void HUD::Element::UpdateEnableState(EnableState eState)
{
    RedScreenGroupElement* pScreen;
    if (mHUDEditOnly)
    {
        pScreen = &gScreenGroupEdit[GetViewport()];
    }
    else
    {
        pScreen = &gScreenGroup[GetViewport()];
    }

    bool bSetState = true;
    switch (eState)
    {
    case enableState_Enabling:
        if (mEnableState != enableState_Enabling && mEnableState != enableState_Enabled)
        {
            if (mElement->GetGroup() || pScreen->AddElement(mElement))
            {
                mElement->Enable(true);
            }
        }
        mFader.Start();
        break;
    case enableState_Disabling:
        if (mEnableState != enableState_Disabling && mEnableState != enableState_Disabled)
        {
            mFader.Stop();
        }
        else
        {
            bSetState = false;
        }
        break;
    case enableState_Disabled:
        if (mElement->m_bEnabled)
        {
            mElement->SetColorOnly(mColorDefault);
            mElement->Enable(false);
            if (pScreen == mElement->GetGroup())
            {
                pScreen->RemoveElement(mElement);
            }
        }
        break;
    }
    ResetFlags();
    if (bSetState)
    {
        mEnableState = eState;
    }
}

// FUNCTION: SWBF2 0x00549e50
void HUD::Element::SetRuntimeScaleAll(float fScale)
{
    for (PblListDouble::Itor itor(&sList); itor.GetNode() != sList.GetTerminator(); ++itor)
    {
        Element* pElement = static_cast<Element*>(itor.GetNode()->GetObjectA(&sList, offsetof(Element, mNode)));
        pElement->SetRuntimeScale(fScale);
    }
    sRuntimeScale = fScale;
}

// FUNCTION: SWBF2 0x00549ea0
float HUD::Element::GetRuntimeScaleAll()
{
    return sRuntimeScale;
}

// FUNCTION: SWBF2 0x00549eb0
void HUD::Element::UpdateAll(float fDeltaTime)
{
    for (PblListDouble::Node* pNode = HUD::Element::sList.mTerminator.mNext;
         pNode != &HUD::Element::sList.mTerminator; pNode = pNode->mNext)
    {
        HUD::Element* pElement = (HUD::Element*)((char*)pNode - offsetof(HUD::Element, mNode));
        pElement->Update(fDeltaTime);
    }
}

float HUD::Element::GetFadeInTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Attack);
}

float HUD::Element::GetFadeHoldTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Hold);
}

float HUD::Element::GetFadeSustainTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Sustain);
}

float HUD::Element::GetFadeOutTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Release);
}

// FUNCTION: SWBF2 0x00549200
bool HUD::Element::InScreenGroup() const
{
    if (mElement->GetGroup())
    {
        const RedScreenGroupElement* pScreen = &gScreenGroup[GetViewport()];
        if (mElement->GetGroup() != pScreen)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00549fe0
void HUD::Element::EventEnable(const HUD::Event& event, void* const pData)
{
    Element* pElement = (Element*)pData;
    pElement->UpdateEnableState(enableState_Enabling);
    pElement->Update(0.001f);
}

// FUNCTION: SWBF2 0x0054a010
void HUD::Element::EventDisable(const HUD::Event& event, void* const pData)
{
    Element* pElement = (Element*)pData;
    pElement->UpdateEnableState(enableState_Disabling);
    pElement->Update(0.001f);
}

// FUNCTION: SWBF2 0x0054a040
void HUD::Element::Changed()
{
    if (mUseChangeColor)
    {
        mElement->SetColorOnly(mColorChanged);
    }
}

// FUNCTION: SWBF2 0x0054a070
void HUD::Element::EventChanged(const HUD::Event& event, void* const pData)
{
    ((Element*)pData)->Changed();
}

// FUNCTION: SWBF2 0x0054a080
void HUD::Element::EventColor(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() == EventClass::type_Color)
    {
        ((Element*)pData)->mColorDefault = *event.GetData().color;
    }
}

// FUNCTION: SWBF2 0x0054a0c0
void HUD::Element::EventPulseRate(const HUD::Event& event, void* const pData)
{
    Element* pElement = (Element*)pData;
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Int:
    case EventClass::type_Uint:
    case EventClass::type_Float:
    {
        if (fabs(event.GetDataFloat() - pElement->mPulseColorRate) > 0.0001f)
        {
            pElement->mPulseTime = event.GetDataFloat();
        }
        pElement->mPulseColorRate = event.GetDataFloat();
        break;
    }
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00548fd0
PblMatrix HUD::Element::CalculateScreenMatrix() const
{
    PblMatrix result = mElement->GetMatrix();
    if (mGroup)
    {
        const PblMatrix parent = CalculateGroupMatrix(mGroup);
        PblMatrix combined = parent * result;
        result = combined;
    }
    return result;
}

// FUNCTION: SWBF2 0x00549d20
HUD::Element* HUD::Element::GetFirst()
{
    PblListDouble::Node* node = sList.mTerminator.mNext;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549d40
HUD::Element* HUD::Element::GetLast()
{
    PblListDouble::Node* node = sList.mTerminator.mPrev;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549d70
HUD::Element* HUD::Element::GetNext(Element* const pElement)
{
    PblListDouble::Node* node = pElement->mNode.mNext;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549d90
HUD::Element* HUD::Element::GetPrevious(Element* const pElement)
{
    PblListDouble::Node* node = pElement->mNode.mPrev;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549db0
HUD::Element* HUD::Element::FindByHashID(unsigned int uHashID)
{
    for (Element* pElement = GetFirst(); pElement; pElement = GetNext(pElement))
    {
        if (pElement->GetNameHash() == uHashID)
        {
            return pElement;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00549de0
void HUD::Element::SetupViewportDimensions(unsigned int uNumViewports, bool bVerticalSplit)
{
    sViewportHeight = (float)(int)RedInterfaceScreen::GetScreenHeight();
    sViewportWidth = (float)(int)RedInterfaceScreen::GetScreenWidth();
    switch (uNumViewports)
    {
    case 2:
        if (bVerticalSplit)
        {
            sViewportWidth *= 0.5f;
        }
        else
        {
            sViewportHeight *= 0.5f;
        }
        break;
    case 3:
    case 4:
        sViewportWidth *= 0.5f;
        sViewportHeight *= 0.5f;
        break;
    }
}

// FUNCTION: SWBF2 0x00549f00
void HUD::Element::DestroyAll()
{
    PblListDouble dynamicList;

    PblListDouble::Node* pNode = sList.mTerminator.mNext;
    while (pNode != &sList.mTerminator)
    {
        Element* pElement = (Element*)((char*)pNode - offsetof(Element, mNode));
        pNode = pNode->mNext;
        if (pElement->mDynamicAlloc)
        {
            pElement->mNode.Remove();
            dynamicList.AddLast(&pElement->mNode);
        }
    }

    while (dynamicList.mTerminator.mNext != &dynamicList.mTerminator)
    {
        PblListDouble::Node* pFirst = dynamicList.mTerminator.mNext;
        Element* pElement = pFirst == &dynamicList.mTerminator
                                ? 0
                                : (Element*)((char*)pFirst - offsetof(Element, mNode));
        delete pElement;
    }

    sList.Clear();
}
