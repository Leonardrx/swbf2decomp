#include "RedGroupElement.h"

// FUNCTION: SWBF2 0x006d68b0
RedGroupElement::RedGroupElement(const char* pcName)
    : RedInterfaceElement(pcName)
{
    mPropagateAlpha = false;
}

// FUNCTION: SWBF2 0x006d68f0
RedGroupElement::RedGroupElement()
{
    mPropagateAlpha = false;
}

// FUNCTION: SWBF2 0x006d6930
RedGroupElement::~RedGroupElement()
{
    RemoveAllElements();
}

// FUNCTION: SWBF2 0x006d6a60
void RedGroupElement::Enable(bool bEnable)
{
    Visible(bEnable);
    m_bEnabled = bEnable;
}

// FUNCTION: SWBF2 0x006d6c10
void RedGroupElement::Visible(bool bVisible)
{
    if (bVisible == (bool)m_bVisible)
    {
        return;
    }

    PblList<RedInterfaceElement>::Node* pNode = m_children.GetFirst();
    RedInterfaceElement* pChild = pNode->_pObject;
    while (pChild != m_children._head._pObject)
    {
        pChild->Visible(bVisible);
        pNode = pNode->_pNext;
        pChild = pNode->_pObject;
    }

    m_bVisible = bVisible;
}

// FUNCTION: SWBF2 0x006d6be0
void RedGroupElement::SetBlendMode(BlendModeT eBlendMode)
{
    PblList<RedInterfaceElement>::Node* pNode = m_children.GetFirst();
    RedInterfaceElement* pChild = pNode->_pObject;
    while (pChild != m_children._head._pObject)
    {
        pChild->SetBlendMode(eBlendMode);
        pNode = pNode->_pNext;
        pChild = pNode->_pObject;
    }
}

// FUNCTION: SWBF2 0x006d6a30
void RedGroupElement::Cleanup()
{
    PblList<RedInterfaceElement>::Node* pNode = m_children.GetFirst();
    RedInterfaceElement* pChild = pNode->_pObject;
    while (pChild != m_children._head._pObject)
    {
        pChild->Cleanup();
        pNode = pNode->_pNext;
        pChild = pNode->_pObject;
    }
}

// FUNCTION: SWBF2 0x006d6a90
void RedGroupElement::RemoveAllElements()
{
    while (m_children.GetCount() != 0)
    {
        m_children.GetFirst()->_pObject->Cleanup();
        m_children.Remove(m_children.GetFirst());
    }
}

// FUNCTION: SWBF2 0x006d69c0
bool RedGroupElement::AddElement(RedInterfaceElement* pElement)
{
    if (pElement->m_groupNode._pList)
    {
        return false;
    }

    PblList<RedInterfaceElement>::Node* pAt = m_children.GetFirst();
    while (pAt->_pObject != m_children._head._pObject)
    {
        const unsigned char uiOrder = pElement->m_zorder;
        const unsigned char uiOther = pAt->_pObject->m_zorder;
        if (uiOrder > uiOther)
        {
            break;
        }
        pAt = pAt->_pNext;
    }

    m_children.AddBefore(pAt, &pElement->m_groupNode, pElement);
    return true;
}

// FUNCTION: SWBF2 0x006d6ae0
bool RedGroupElement::RemoveElement(RedInterfaceElement* pElement)
{
    PblList<RedInterfaceElement>* pList = pElement->m_groupNode._pList;
    if (!pList)
    {
        return false;
    }

    pList->Remove(&pElement->m_groupNode);
    return true;
}

// FUNCTION: SWBF2 0x006d6b20
void RedGroupElement::RenderUsingContext(const PblMatrix& mContext, const RedColor& color)
{
    PblList<RedInterfaceElement>::Node* pNode = m_children.GetFirst();
    RedInterfaceElement* pChild = pNode->_pObject;
    if (pChild == m_children._head._pObject)
    {
        return;
    }

    RedColor childColor;
    childColor.a = 0xFF;

    while (pChild != m_children._head._pObject)
    {
        childColor.r = color.r;
        childColor.g = color.g;
        childColor.b = color.b;

        if (mPropagateAlpha)
        {
            unsigned char ucChildAlpha = pNode->_pObject->m_color.a;
            pNode->_pObject->m_color.a =
                (unsigned char)((m_color.a * ucChildAlpha) / 255);
            pNode->_pObject->Render(mContext, childColor);
            pNode->_pObject->m_color.a = ucChildAlpha;
        }
        else
        {
            pChild->Render(mContext, childColor);
        }

        pNode = pNode->_pNext;
        pChild = pNode->_pObject;
    }
}
