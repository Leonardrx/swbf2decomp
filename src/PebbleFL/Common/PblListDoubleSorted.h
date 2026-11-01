#pragma once

#include <stddef.h>

#include "PblListDouble.h"

class PblListDoubleSorted : public PblListDouble
{
public:
    class Node
    {
    public:

        Node(float fSortValue = 0.0f) : mSortValue(fSortValue)
        {
        }

        // FUNCTION: SWBF2 0x004d26d0
        ~Node()
        {
        }

        // FUNCTION: SWBF2 0x004d2620
        void* GetObjectA(PblListDoubleSorted const* pList, unsigned int uOffset) const
        {
            if ((void const*)this == (void const*)pList)
            {
                return 0;
            }
            return (char*)this - uOffset;
        }

        Node* GetNext(PblListDoubleSorted* const pList) const
        {
            PblListDouble::Node* pNext = mNode.mNext;
            return pNext == &pList->mTerminator
                ? (Node*)pList
                : (Node*)((char*)pNext - offsetof(Node, mNode));
        }

        Node* GetPrevious(PblListDoubleSorted* const pList) const
        {
            PblListDouble::Node* pPrevious = mNode.mPrev;
            return pPrevious == &pList->mTerminator
                ? 0
                : (Node*)((char*)pPrevious - offsetof(Node, mNode));
        }

        float mSortValue;
        PblListDouble::Node mNode;
    };

    class Itor : public PblListDouble::Itor
    {
    public:
        Itor(Node* pNode) : PblListDouble::Itor(&pNode->mNode) {}
        Itor(const Itor& itor) : PblListDouble::Itor(itor) {}
        Itor(const PblListDoubleSorted* pList) : PblListDouble::Itor(pList) {}
        Itor(const PblListDoubleSorted& list) : PblListDouble::Itor(list) {}
        Itor() {}

        Node* GetNode(const PblListDoubleSorted* pList)
        {
            PblListDouble::Node* pNode = PblListDouble::Itor::GetNode();
            return pNode == &pList->mTerminator
                ? (Node*)pList
                : (Node*)((char*)pNode - offsetof(Node, mNode));
        }
    };

    Itor Begin() const { return Itor(this); }
    Itor End() { return Itor(GetLast()); }

    PblListDoubleSorted(bool bSortOnAdd = false)
    {
        mFlags = bSortOnAdd ? 1 : 0;
    }

    void SetSortOnAdd(bool bSortOnAdd)
    {
        if (bSortOnAdd)
        {
            mFlags |= 1;
        }
        else
        {
            mFlags = (unsigned char)(mFlags & ~1u);
        }
    }

    // FUNCTION: SWBF2 0x004d26c0
    ~PblListDoubleSorted()
    {
    }

    Node const* GetTerminator() const
    {
        return (Node const*)this;
    }

    // FUNCTION: SWBF2 0x004fbe10
    Node* GetFirst() const
    {
        PblListDouble::Node* pNext = mTerminator.mNext;
        return pNext != &mTerminator
            ? (Node*)((char*)pNext - offsetof(Node, mNode))
            : (Node*)pNext;
    }

    Node* GetLast() const
    {
        PblListDouble::Node* pPrevious = mTerminator.mPrev;
        return pPrevious == &mTerminator
            ? (Node*)pPrevious
            : (Node*)((char*)pPrevious - offsetof(Node, mNode));
    }

    // FUNCTION: SWBF2 0x004d2640
    void Add(Node* pNode)
    {
        if (mFlags & 1)
        {
            Itor itor(this);
            for (;;)
            {
                Node* const current = itor.GetNode(this);
                if (current == GetTerminator())
                {
                    break;
                }
                if (pNode->mSortValue > current->mSortValue)
                {
                    AddBefore(&current->mNode, &pNode->mNode);
                    return;
                }
                ++itor;
            }
            AddLast(&pNode->mNode);
        }
        else
        {
            AddLast(&pNode->mNode);
        }
    }

    // FUNCTION: SWBF2 0x005a9420
    void Sort()
    {
        PblListDouble::Node* pLink = mTerminator.mNext;
        Node* pPrev = (Node*)pLink->GetObjectA(this, offsetof(Node, mNode));

        pLink = pLink->mNext;
        Node* pNode = (Node*)pLink->GetObjectA(this, offsetof(Node, mNode));
        while (pNode)
        {
            if (pNode->mSortValue > pPrev->mSortValue)
            {
                Node* pAfter;
                PblListDouble::Node* pScan = &pNode->mNode;
                do
                {
                    pScan = pScan->mPrev;
                    pAfter = (Node*)pScan->GetObjectA(this, offsetof(Node, mNode));
                } while (pAfter && pNode->mSortValue > pAfter->mSortValue);

                pNode->mNode.Remove();

                PblListDouble::Node* pAt = pAfter ? pAfter->mNode.mNext : mTerminator.mNext;
                AddBefore(pAt, &pNode->mNode);
            }

            pPrev = pNode;
            pLink = pNode->mNode.mNext;
            pNode = (Node*)pLink->GetObjectA(this, offsetof(Node, mNode));
        }
    }

    unsigned char mFlags;
};
 
 
