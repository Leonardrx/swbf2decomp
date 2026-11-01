#include "HUDElementBarSegmented.h"
#include "PebbleFL/Common/PblVector.h"

#include <d3dx9math.h>

#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

// FUNCTION: SWBF2 0x0054baf0
HUD::ElementBarSegmented::Factory::Factory()
{
    mName = "BarSegmented";
    mHashID = 0x4B4BF31Au;

    mAngleStart.Init("AngleStart", mProperties, -360.0f, 360.0f, 0.1f);
    mAngleEnd.Init("AngleEnd", mProperties, -360.0f, 360.0f, 0.1f);
    mNumSegments.Init("NumSegments", mProperties, 0, 360);
    mCircular.Init("Circular", mProperties, Property::type_Bool);

    Setup(this);
}

// SYNTHETIC: SWBF2 0x0054bc90
// HUD::ElementBarSegmented::Factory::~Factory

// SYNTHETIC: SWBF2 0x0054b860
// HUD::ElementGroup::Factory::~Factory

// FUNCTION: SWBF2 0x0054b7b0
HUD::Item* HUD::ElementGroup::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                   void* const pUserData)
{
    HUD::Item* pItem = new HUD::ElementGroup(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0054b870
HUD::Item* HUD::ElementBarSegmented::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                          void* const pUserData)
{
    HUD::Item* pItem = new HUD::ElementBarSegmented(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}



// SYNTHETIC: SWBF2 0x0054cae2
// ??_ESegment@ElementBarSegmented@HUD@@WBCA@AEPAXI@Z


// FUNCTION: SWBF2 0x0054b990
bool HUD::ElementBarSegmented::Segment::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    bool bResult = HUD::ElementBitmapBase::SetProperty(uHashID, pValue);
    if (bResult && mGroup)
    {
        mGroup->RefreshFormatting(false);

        const float fFadeIn =
            mGroup->mFader.GetPhaseTime(HUD::Envelope::phase_Attack) > mFader.GetPhaseTime(HUD::Envelope::phase_Attack)
                ? mGroup->mFader.GetPhaseTime(HUD::Envelope::phase_Attack)
                : mFader.GetPhaseTime(HUD::Envelope::phase_Attack);
        const unsigned int uNumSegments = mGroup->mNumSegments;
        for (float fTime = 0.0f; fTime < fFadeIn; fTime += 0.1f)
        {
            for (unsigned int i = 0; i < uNumSegments; ++i)
            {
                mGroup->mSegments[i].Update(0.1f);
            }
            mGroup->Update(0.1f);
        }
    }
    return bResult;
}

static const unsigned int kAngleStart = 0x04507e24;
static const unsigned int kAngleEnd = 0x7197bcbd;
static const unsigned int kNumSegments = 0x40fbdebd;
static const unsigned int kCircular = 0x026723e0;



// FUNCTION: SWBF2 0x0054bd00
HUD::ElementBarSegmented::ElementBarSegmented(const char* const pName, Factory::ItemCallback pCallback,
                                              void* const pUserData)
    : HUD::ElementGroup(pName, pCallback, pUserData)
    , HUD::ElementBar()
{
    mCircular = true;
    mSegments = 0;
    mNumSegments = 0;
    mAngleStart = 0.0f;
    mAngleEnd = 0.0f;

    HUD::Item* pSegmentAll = &mSegmentAll;
    if (pSegmentAll)
    {
        pSegmentAll->SetFactory(HUD::Item::Factory::FindByHashID(0x46544626u));
    }
    mSegmentAll.SetDynamicallyAllocated(false);
    mSegmentAll.SetWriteEnabled(false);
    mSegmentAll.SetGroup(this);
    mSegmentAll.HUD::Item::SetName("Segment");
}

// SYNTHETIC: SWBF2 0x0054caa0
// ??_EElementBarSegmented@HUD@@WCAA@AEPAXI@Z
// SYNTHETIC: SWBF2 0x0054cac1
// ??_EElementBarSegmented@HUD@@WBHA@AEPAXI@Z
// FUNCTION: SWBF2 0x0054bec0
HUD::ElementBarSegmented::~ElementBarSegmented()
{
    SetupSegments(0);
}

// SYNTHETIC: SWBF2 0x0054caab
// ?SetProperty@ElementBarSegmented@HUD@@WCAA@AE_NIQBX@Z
// FUNCTION: SWBF2 0x0054bf90
bool HUD::ElementBarSegmented::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    bool bResult;
    switch (uHashID)
    {
    case kNumSegments:
        SetupSegments(*(const unsigned int*)pValue);
        bResult = true;
        break;
    case kCircular:
        mCircular = *(const bool*)pValue;
        bResult = true;
        break;
    case kAngleStart:
        mAngleStart = *(const float*)pValue * 0.017453292f;
        bResult = true;
        break;
    case kAngleEnd:
        mAngleEnd = *(const float*)pValue * 0.017453292f;
        bResult = true;
        break;
    default:
        bResult = HUD::ElementGroupBase::SetProperty(uHashID, pValue) ||
                  HUD::ElementBar::SetProperty(uHashID, pValue);
        break;
    }
    if (bResult)
    {
        SetValue(mValue);
    }
    return bResult;
}

// SYNTHETIC: SWBF2 0x0054cacc
// ?GetProperty@ElementBarSegmented@HUD@@WCAA@AE_NIPAX@Z
// FUNCTION: SWBF2 0x0054c070
bool HUD::ElementBarSegmented::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kNumSegments:
        *(unsigned int*)pValue = mNumSegments;
        return true;
    case kCircular:
        *(bool*)pValue = mCircular;
        return true;
    case kAngleStart:
        *(float*)pValue = mAngleStart * 57.295776f;
        return true;
    case kAngleEnd:
        *(float*)pValue = mAngleEnd * 57.295776f;
        return true;
    default:
        return HUD::ElementGroupBase::GetProperty(uHashID, pValue) ||
               HUD::ElementBar::GetProperty(uHashID, pValue);
    }
}

// FUNCTION: SWBF2 0x0054c130
float HUD::ElementBarSegmented::SetValue(const float fValue)
{
    const float fValueApplied = HUD::ElementBar::SetValue(fValue);
    const float fWidth = GetWidthPixelsFloat();
    const float fHeight = GetHeightPixelsFloat();
    PblMatrix matrix;
    unsigned int i = 0;

    if (mCircular)
    {
        const float fHalfWidth = fWidth * 0.5f;
        const float fHalfHeight = fHeight * 0.5f;
        const float fSpan = (float)fabs((double)(mAngleEnd - mAngleStart));
        const float fStep = fSpan / (float)(int)mNumSegments;
        const float fLit = fSpan * fValueApplied + mAngleStart;

        for (float fAngle = mAngleStart; fAngle < fLit && i < mNumSegments; fAngle += fStep, ++i)
        {
            float fSin, fCos;
            PblMath::SinCos(&fSin, &fCos, fAngle);
            const float fX = fSin * fHalfWidth + fHalfWidth;
            const float fCosHeight = fCos * fHalfHeight;
            const float fY = -(0.75f / RedRenderer::GetScreenAspectRatio() * fCosHeight) + fHalfHeight;

            matrix.SetRotateZ(fAngle);
            matrix.m[3][0] = fX;
            matrix.m[3][1] = fY;
            matrix.m[3][2] = 0.0f;
            matrix.m[3][3] = 1.0f;

            Segment& seg = mSegments[i];
            if (!seg.IsEnabled())
            {
                seg.Changed();
            }
            seg.HUD::Element::Enable(true);
            seg.SetMatrix(matrix);
        }
    }
    else
    {
        float fPos = 0.0f;
        const float fStep = (float)(int)fWidth / (float)(int)mNumSegments;
        const float fLit = fWidth * fValueApplied + fPos;
        matrix = gMatrixIdentity;

        for (; fPos < fLit && i < mNumSegments; fPos += fStep, ++i)
        {
            matrix.m[3][0] = fPos;
            matrix.m[3][1] = 0.0f;
            matrix.m[3][2] = 0.0f;
            matrix.m[3][3] = 1.0f;

            Segment& seg = mSegments[i];
            if (!seg.IsEnabled())
            {
                seg.Changed();
            }
            seg.HUD::Element::Enable(true);
            seg.SetMatrix(matrix);
        }
    }

    for (; i < mNumSegments; ++i)
    {
        Segment& seg = mSegments[i];
        if (seg.IsEnabled())
        {
            seg.Changed();
            seg.HUD::Element::Enable(false);
        }
    }

    Changed();
    return fValueApplied;
}

// FUNCTION: SWBF2 0x0054c4d0
void HUD::ElementBarSegmented::RefreshFormatting(bool bAddToGroup)
{
    for (unsigned int i = 0; i < mNumSegments; ++i)
    {
        mSegments[i].SetDynamicallyAllocated(false);
        mSegments[i].SetWriteEnabled(false);
        if (bAddToGroup)
        {
            HUD::ElementGroupBase::AddElement(&mSegments[i]);
        }
        if (mSegments[i].IsEnabled())
        {
            mSegments[i].HUD::Element::Enable(false);
        }
        mSegments[i] = mSegmentAll;
    }
    SetValue(mValue);
}

// FUNCTION: SWBF2 0x0054c5a0
void HUD::ElementBarSegmented::SetupSegments(unsigned int uNumSegments)
{
    const bool bChanged = (uNumSegments != mNumSegments);
    if (bChanged)
    {
        if (mSegments)
        {
            for (unsigned int i = 0; i < mNumSegments; ++i)
            {
                HUD::ElementGroupBase::RemoveElement(&mSegments[i]);
            }
            delete[] mSegments;
            mSegments = 0;
            mNumSegments = 0;
        }
        if (uNumSegments)
        {
            mSegments = new Segment[uNumSegments];
            if (mSegments)
            {
                mNumSegments = uNumSegments;
            }
        }
    }
    RefreshFormatting(bChanged);
}

// SYNTHETIC: SWBF2 0x0054cad7
// ?PostReadSetup@ElementBarSegmented@HUD@@OCAA@AEXXZ
// FUNCTION: SWBF2 0x0054c790
void HUD::ElementBarSegmented::PostReadSetup()
{
    const unsigned int uNumSegments = mNumSegments;
    mNumSegments = 0;
    SetupSegments(uNumSegments);
    HUD::ElementGroupBase::PostReadSetup();
    HUD::ElementBar::PostReadSetup();

    char buffer[256];
    _snprintf(buffer, sizeof(buffer), "%s.%s", GetName(), mSegmentAll.GetName());
    mSegmentAll.SetDisplayName(buffer);
}

// FUNCTION: SWBF2 0x0054c810
void HUD::ElementBarSegmented::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                    void* const pUserData)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            if (mSegmentAll.GetNameHash() == data.GetId())
            {
                PblConfig scope(config.ReadNextScope(), true);
                mSegmentAll.Read(scope, 0, 0);
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

// SYNTHETIC: SWBF2 0x0054caed
// ?ReadData@ElementBarSegmented@HUD@@OCAA@AE_NAAVPblConfig@@AAVData@3@@Z
// FUNCTION: SWBF2 0x0054c8f0
bool HUD::ElementBarSegmented::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kNumSegments:
        mNumSegments = (unsigned int)data.GetFloatArg(0);
        return true;
    case kCircular:
        mCircular = data.GetFloatArg(0) != 0.0f;
        return true;
    case kAngleStart:
        mAngleStart = data.GetFloatArg(0) * 0.017453292f;
        return true;
    case kAngleEnd:
        mAngleEnd = data.GetFloatArg(0) * 0.017453292f;
        return true;
    }
    return HUD::ElementBar::ReadData(config, data) || HUD::ElementGroupBase::ReadData(config, data);
}

// SYNTHETIC: SWBF2 0x0054cab6
// ?WriteData@ElementBarSegmented@HUD@@OCAA@AEXAAVFile@2@I@Z

// FUNCTION: SWBF2 0x0054c9e0
void HUD::ElementBarSegmented::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("AngleStart", uTabs, mAngleStart * 57.295776f, 0.0f);
    file.WriteProperty("AngleEnd", uTabs, mAngleEnd * 57.295776f, 0.0f);
    file.WriteProperty("NumSegments", uTabs, (int)mNumSegments, 0);
    file.WriteProperty("Circular", uTabs, mCircular ? 1 : 0, 1);
    HUD::ElementBar::WriteData(file, uTabs);
    HUD::ElementGroupBase::WriteData(file, uTabs);
    mSegmentAll.Write(file, uTabs, "Segment");
}
