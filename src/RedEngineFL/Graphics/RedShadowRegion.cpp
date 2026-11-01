#include <stdio.h>
#include <string.h>

#include "RedShadowRegion.h"
#include "PebbleFL/Common/PblHashTable.h"

// GLOBAL: SWBF2 0x007e2580
PblList<RedShadowRegion> RedShadowRegion::s_List;

// GLOBAL: SWBF2 0x007e2594
PblList<RedShadowRegion> RedShadowRegion::s_VisibleList;

// SYNTHETIC: SWBF2 0x00719460
// RedShadowRegion::Info::operator=

// FUNCTION: SWBF2 0x00719310
RedShadowRegion::RedShadowRegion(RedRegionDesc& desc, const Info& info)
    : RedRegion(desc)
{
    m_info = info;
    s_List.Add(&m_Node, this);
}

// FUNCTION: SWBF2 0x00719550
void* RedShadowRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    RedShadowRegion::Info info;
    memset(&info, 0, sizeof(info));
    info.fDirectionalFactor[0] = 1.0f;
    info.fDirectionalFactor[1] = 1.0f;
    ProcessRegionName(pcName, &info);
    return new RedShadowRegion(desc, info);
}

// FUNCTION: SWBF2 0x00719640
bool RedShadowRegion::GetShadowValues(const PblVector3& vPos, float fRadius,
                                      float* const pfDirectional, RedAmbientLight& ambient,
                                      unsigned int& uEnvMapHash, float& fEnvMapFactor)
{
    RedShadowRegion* apRegion[4];
    float afFactor[4];
    int iCount = 0;
    int iMax = 0;
    float fTotal = 0.0f;

    for (PblList<RedShadowRegion>::Itor it(s_VisibleList); it != s_VisibleList.End(); ++it)
    {
        float fFactor = it->GetIntersectionFactor(vPos, fRadius);
        if (fFactor > 0.0f)
        {
            afFactor[iCount] = fFactor;
            apRegion[iCount] = it;
            if (fFactor > afFactor[iMax])
            {
                iMax = iCount;
            }
            fTotal += fFactor;
            if (++iCount >= 4)
            {
                break;
            }
        }
    }

    if (iCount > 0)
    {
        uEnvMapHash = RedLight::m_sGlobalEnvMapHash;
        fEnvMapFactor = 1.0f;

        if (iCount > 1 && fTotal > (float)(iCount - 1) * 0.9f)
        {
            pfDirectional[0] = 0.0f;
            pfDirectional[1] = 0.0f;
            ambient.m_topColor = RedColorValue::BLACK;
            ambient.m_bottomColor = RedColorValue::BLACK;
        }
        else
        {
            float fGlobal = (float)iCount - fTotal;
            fTotal += fGlobal;
            float fScale = fGlobal / fTotal;
            pfDirectional[0] = fScale;
            pfDirectional[1] = fScale;
            ambient.m_topColor = RedColorValue(RedLight::m_sGlobalAmbient->m_topColor.r * fScale,
                                               RedLight::m_sGlobalAmbient->m_topColor.g * fScale,
                                               RedLight::m_sGlobalAmbient->m_topColor.b * fScale,
                                               RedLight::m_sGlobalAmbient->m_topColor.a * fScale);
            ambient.m_bottomColor = RedColorValue(RedLight::m_sGlobalAmbient->m_bottomColor.r * fScale,
                                                  RedLight::m_sGlobalAmbient->m_bottomColor.g * fScale,
                                                  RedLight::m_sGlobalAmbient->m_bottomColor.b * fScale,
                                                  RedLight::m_sGlobalAmbient->m_bottomColor.a * fScale);
        }

        for (int i = 0; i < iCount; ++i)
        {
            const Info& info = apRegion[i]->m_info;
            float fWeight = afFactor[i] / fTotal;
            pfDirectional[0] += info.fDirectionalFactor[0] * fWeight;
            pfDirectional[1] += info.fDirectionalFactor[1] * fWeight;

            const RedColorValue& top = info.ambient.m_topColor;
            if (top.a > 0.0f)
            {
                const RedColorValue topScaled(top.r * fWeight, top.g * fWeight,
                                              top.b * fWeight, top.a * fWeight);
                ambient.m_topColor = RedColorValue(ambient.m_topColor.r + topScaled.r,
                                                   ambient.m_topColor.g + topScaled.g,
                                                   ambient.m_topColor.b + topScaled.b,
                                                   ambient.m_topColor.a + topScaled.a);
            }
            const RedColorValue& bottom = info.ambient.m_bottomColor;
            if (bottom.a > 0.0f)
            {
                const RedColorValue bottomScaled(bottom.r * fWeight, bottom.g * fWeight,
                                                 bottom.b * fWeight, bottom.a * fWeight);
                ambient.m_bottomColor = RedColorValue(ambient.m_bottomColor.r + bottomScaled.r,
                                                      ambient.m_bottomColor.g + bottomScaled.g,
                                                      ambient.m_bottomColor.b + bottomScaled.b,
                                                      ambient.m_bottomColor.a + bottomScaled.a);
            }
        }

        if (apRegion[iMax]->m_info.envMapHash)
        {
            fEnvMapFactor = afFactor[iMax];
            uEnvMapHash = apRegion[iMax]->m_info.envMapHash;
        }
    }
    return iCount > 0;
}

// TEMPLATE: SWBF2 0x00719510
// PblList<RedShadowRegion>::RemoveAll
template void PblList<RedShadowRegion>::RemoveAll();

// FUNCTION: SWBF2 0x00719950
void RedShadowRegion::Init()
{
    s_VisibleList.RemoveAll();
    s_List.RemoveAll();
}

// FUNCTION: SWBF2 0x00719970
void RedShadowRegionFactory::ReadParameter(const char* pcRegionName, const char* pcParameter,
                                           unsigned int uiIndex, unsigned int uiCount,
                                           const char* pcValue, unsigned int uiValueIndex,
                                           void* pDesc) const
{
    RedShadowRegion::Info* pInfo = (RedShadowRegion::Info*)pDesc;
    const float kToFloat = 1.0f / 255.0f;
    const unsigned int uiParameter = PblHash(pcParameter);

    switch (uiParameter)
    {
    case 0x10f7c207:
    {
        float f = 0.0f;
        if (sscanf(pcValue, "%f", &f) == 1)
        {
            pInfo->fDirectionalFactor[0] = f;
        }
        break;
    }
    case 0xec06bb02:
    {
        float f = 0.0f;
        if (sscanf(pcValue, "%f", &f) == 1)
        {
            pInfo->fDirectionalFactor[1] = f;
        }
        break;
    }
    case 0x97f5abd3:
    {
        int r, g, b;
        if (sscanf(pcValue, "%d,%d,%d", &r, &g, &b) == 3)
        {
            RedColorValue c;
            c.r = (unsigned char)r * kToFloat;
            c.g = (unsigned char)g * kToFloat;
            c.b = (unsigned char)b * kToFloat;
            c.a = 1.0f;
            pInfo->ambient.m_topColor = c;
        }
        break;
    }
    case 0x7b5ec2ab:
    {
        int r, g, b;
        if (sscanf(pcValue, "%d,%d,%d", &r, &g, &b) == 3)
        {
            const RedColor color(r, g, b, 0xff);
            const RedColorValue value(color);
            pInfo->ambient.m_bottomColor = value;
        }
        break;
    }
    case 0xbb6dd0da:
    {
        char acName[256];
        if (sscanf(pcValue, "%s", acName) == 1)
        {
            pInfo->envMapHash = PblHash(acName);
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x007194f0
void RedShadowRegion::Cleanup()
{
    s_VisibleList.RemoveAll();
    s_List.RemoveAll();
}

// SYNTHETIC: SWBF2 0x006c6a70
// RedShadowRegionFactory::`scalar deleting destructor'
