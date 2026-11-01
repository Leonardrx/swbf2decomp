#include "RedInterfaceElement.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedGroupElement.h"
#include "PebbleFL/Common/PblMath.h"

// FUNCTION: SWBF2 0x006c0250
void RedInterfaceElement::Rotate(float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    PblVector3 vY = m_mLocal[1];
    PblVector3 vX = m_mLocal[0];
    m_mLocal[0] = vX * fCos + vY * fSin;
    m_mLocal[1] = vX * -fSin + vY * fCos;
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal.m[1][3] = 0.0f;
}

// FUNCTION: SWBF2 0x006c0370
void RedInterfaceElement::Rotate(float fX, float fY, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    PblVector3 vX = m_mLocal[0];
    PblVector3 vY = m_mLocal[1];
    m_mLocal[0] = vX * fCos + vY * fSin;
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal[1] = vX * -fSin + vY * fCos;
    m_mLocal.m[1][3] = 0.0f;
    m_mLocal[3] = m_mLocal[3] + vX * fX + vY * fY;
    m_mLocal.m[3][3] = 1.0f;
}

// GLOBAL: SWBF2 0x008fe488
unsigned int RedInterfaceElement::s_uiUnnamedCRC;

// SYNTHETIC: SWBF2 0x00407a10
// `dynamic initializer for 'RedInterfaceElement::sAllObjectsList''

// GLOBAL: SWBF2 0x008fe498
PblHashTable<RedInterfaceElement, 32768> RedInterfaceElement::sAllObjectsList;

// FUNCTION: SWBF2 0x006bfeb0
RedInterfaceElement::RedInterfaceElement(const char* pcName)
    : m_zorder(128)
    , m_bEnabled(true)
    , m_color(0xFF, 0xFF, 0xFF, 0xFF)
{
    m_mLocal.SetXAxis(1.0f, 0.0f, 0.0f, 0.0f);
    m_mLocal.SetYAxis(0.0f, 1.0f, 0.0f, 0.0f);
    m_mLocal.SetZAxis(0.0f, 0.0f, 1.0f, 0.0f);
    m_mLocal.SetTranslation(0.0f, 0.0f, 0.0f, 1.0f);

    SetColor(RedColor(0xFF, 0xFF, 0xFF, 0xFF));

    m_bInAllObjectList = false;
    m_uiFlags = 0;

    m_pHotSpot = 0;

    SetName(pcName);
}

// FUNCTION: SWBF2 0x006bff90
RedInterfaceElement::RedInterfaceElement()
    : m_zorder(128)
    , m_bEnabled(true)
    , m_color(0xFF, 0xFF, 0xFF, 0xFF)
{
    m_mLocal.SetXAxis(1.0f, 0.0f, 0.0f, 0.0f);
    m_mLocal.SetYAxis(0.0f, 1.0f, 0.0f, 0.0f);
    m_mLocal.SetZAxis(0.0f, 0.0f, 1.0f, 0.0f);
    m_mLocal.SetTranslation(0.0f, 0.0f, 0.0f, 1.0f);

    SetColor(RedColor(0xFF, 0xFF, 0xFF, 0xFF));

    const unsigned int uiCRC = s_uiUnnamedCRC;
    m_crcName = uiCRC;
    s_uiUnnamedCRC = uiCRC + 1;

    m_bInAllObjectList = false;
    m_uiFlags = 0;

    m_pHotSpot = 0;
}

// FUNCTION: SWBF2 0x006c0070
RedInterfaceElement::~RedInterfaceElement()
{
    if (m_bInAllObjectList)
    {
        m_bInAllObjectList = false;
        sAllObjectsList.Remove(m_crcName);
    }

    if (m_groupNode._pList)
    {
        m_groupNode._pList->Remove(&m_groupNode);
    }

    delete m_pHotSpot;

    SetName(0);
}

// FUNCTION: SWBF2 0x006c0170
void RedInterfaceElement::BringToFront()
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;
    m_zorder = 0;
    if (!pList)
    {
        return;
    }
    pList->Remove(&m_groupNode);
    pList->AddLast(&m_groupNode, this);
}

// FUNCTION: SWBF2 0x006c0590
void RedInterfaceElement::SendToBack()
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;
    m_zorder = 0xFF;
    if (!pList)
    {
        return;
    }
    pList->Remove(&m_groupNode);
    pList->AddFirst(&m_groupNode, this);
}

// FUNCTION: SWBF2 0x006c0750
void RedInterfaceElement::SetMatrixPosScale(float fX, float fY, float fScaleX,
                                            float fScaleY)
{
    m_mLocal.SetXAxis(fScaleX, 0.0f, 0.0f, 0.0f);
    m_mLocal.SetYAxis(0.0f, fScaleY, 0.0f, 0.0f);
    m_mLocal.SetZAxis(0.0f, 0.0f, 1.0f, 0.0f);
    m_mLocal.SetTranslation(fX, fY, 0.0f, 1.0f);
}

// FUNCTION: SWBF2 0x006c0510
void RedInterfaceElement::Scale(float fScaleX, float fScaleY)
{
    m_mLocal[0] = fScaleX * m_mLocal[0];
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal[1] = fScaleY * m_mLocal[1];
    m_mLocal.m[1][3] = 0.0f;
}

// FUNCTION: SWBF2 0x006c07e0
void RedInterfaceElement::SetName(const char* pcName)
{
    m_crcName = PblHash(pcName);
}

// FUNCTION: SWBF2 0x006c0880
void RedInterfaceElement::SetZOrder(unsigned char uZOrder)
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;

    m_zorder = uZOrder;
    if (!pList)
    {
        return;
    }

    pList->Remove(&m_groupNode);

    PblList<RedInterfaceElement>::Itor it(*pList);
    while (it != pList->End())
    {
        unsigned char uOther = it->m_zorder;
        if (uZOrder >= uOther)
        {
            break;
        }
        ++it;
    }

    pList->AddBefore(it.GetNode(), &m_groupNode, this);
}

// FUNCTION: SWBF2 0x006c0820
void RedInterfaceElement::SetPosition(float fX, float fY)
{
    m_mLocal.SetTranslation(fX, fY, m_mLocal.m[3][2], 1.0f);
}

// FUNCTION: SWBF2 0x006c0850
void RedInterfaceElement::SetPosition(float fX, float fY, float fZ)
{
    m_mLocal.m[3][0] = fX;
    m_mLocal.m[3][1] = fY;
    m_mLocal.m[3][2] = fZ;
    m_mLocal.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x006c0800
void RedInterfaceElement::SetPosition(const PblVector3& vPos)
{
    m_mLocal.SetTranslation(vPos);
}

// FUNCTION: SWBF2 0x006c0210
RedGroupElement* RedInterfaceElement::GetGroup() const
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;
    if (pList)
    {
        return (RedGroupElement*)((char*)pList - offsetof(RedGroupElement, m_children));
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c0130
void RedInterfaceElement::AddToSearchList(const char* pcName)
{
    SetName(pcName);
    sAllObjectsList.Store(m_crcName, this);
    m_bInAllObjectList = true;
}

// FUNCTION: SWBF2 0x006c01d0
RedInterfaceElement* RedInterfaceElement::Find(unsigned int uiNameHash)
{
    return sAllObjectsList.Find(uiNameHash);
}

// FUNCTION: SWBF2 0x006c01f0
RedInterfaceElement* RedInterfaceElement::Find(const char* pcName)
{
    return Find((unsigned int)PblHash(pcName));
}

// FUNCTION: SWBF2 0x006c0220
void RedInterfaceElement::Reset()
{
    sAllObjectsList.Clear();
    s_uiUnnamedCRC = 0;
}

// FUNCTION: SWBF2 0x006c05f0
void RedInterfaceElement::SetMatrix(const PblMatrix& matrix)
{
    m_mLocal.SetXAxis(matrix(0, 0), matrix(0, 1), matrix(0, 2), 0.0f);
    m_mLocal.SetYAxis(matrix(1, 0), matrix(1, 1), matrix(1, 2), 0.0f);
    m_mLocal.SetZAxis(matrix(2, 0), matrix(2, 1), matrix(2, 2), 0.0f);
    m_mLocal.SetTranslation(matrix(3, 0), matrix(3, 1), matrix(3, 2), 1.0f);
}

// FUNCTION: SWBF2 0x006c0680
void RedInterfaceElement::SetMatrixPosRotScale(float fX, float fY, float fRotation, float fScaleX,
                                               float fScaleY)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fRotation);
    m_mLocal.SetXAxis(fCos * fScaleX, fSin * fScaleX, 0.0f, 0.0f);
    m_mLocal.SetYAxis(-(fSin * fScaleY), fCos * fScaleY, 0.0f, 0.0f);
    m_mLocal.SetZAxis(0.0f, 0.0f, 1.0f, 0.0f);
    m_mLocal.SetTranslation(fX, fY, 0.0f, 1.0f);
}
