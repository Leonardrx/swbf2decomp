#pragma once

#include <stddef.h>

class Thread;

enum ThreadType
{
    PER_FRAME = 0,
    PER_LOCAL_TURN = 1,
    PER_TURN = 2
};

template <class TYPE, class KEY>
class PblOrderedList
{
public:
    class Node
    {
    public:
        Node() : _pList(0), _pNext(0), _pPrev(0), _pObject(0), _iOrder(0) {}

        PblOrderedList* _pList;
        Node* _pNext;
        Node* _pPrev;
        TYPE* _pObject;
        KEY _iOrder;
    };

    class Itor
    {
    public:
        Itor() : _pNode(0) {}
        Itor(const PblOrderedList* pList) : _pNode(pList->GetFirst()) {}
        Itor(const PblOrderedList& list) : _pNode(list.GetFirst()) {}
        Itor(Node* pNode) : _pNode(pNode) {}

        TYPE* operator->() const { return _pNode->_pObject; }
        TYPE& operator()() const { return *_pNode->_pObject; }
        TYPE& operator*() const { return *_pNode->_pObject; }
        operator TYPE*() const { return _pNode->_pObject; }
        const Itor& operator++() { _pNode = _pNode->_pNext; return *this; }
        const Itor& operator--() { _pNode = _pNode->_pPrev; return *this; }
        Node* GetNode() { return _pNode; }

    private:
        Node* _pNode;
    };

    PblOrderedList()
    {
        _head._pPrev = &_head;
        _head._pNext = &_head;
        _iCount = 0;
    }

    void Add(Node* pNode, TYPE* pObject, KEY iOrder);
    void Clear();

    int GetCount() const { return _iCount; }
    Node* GetFirst() const { return _head._pNext; }
    Node* GetLast() const { return _head._pPrev; }
    Itor Begin() const { return Itor(GetFirst()); }
    Itor End() const { return Itor(const_cast<Node*>(&_head)); }

    void Remove(Node* pNode)
    {
        --_iCount;
        pNode->_pList = 0;
        pNode->_pObject = 0;
        pNode->_pNext->_pPrev = pNode->_pPrev;
        pNode->_pPrev->_pNext = pNode->_pNext;
    }

    Node _head;
    int _iCount;
};

template <class TYPE, class KEY>
inline void PblOrderedList<TYPE, KEY>::Add(Node* pNode, TYPE* pObject, KEY iOrder)
{
    Node* pAt = _head._pNext;
    ++_iCount;
    _head._iOrder = 0x7FFFFFFF;

    while (iOrder > pAt->_iOrder)
    {
        pAt = pAt->_pNext;
    }
    while (iOrder == pAt->_iOrder && pNode > pAt)
    {
        pAt = pAt->_pNext;
    }

    pNode->_pList = this;
    pNode->_pNext = pAt;
    pNode->_iOrder = iOrder;
    pNode->_pObject = pObject;
    pNode->_pPrev = pAt->_pPrev;
    pAt->_pPrev = pNode;
    pNode->_pPrev->_pNext = pNode;
}

template <class TYPE, class KEY>
void PblOrderedList<TYPE, KEY>::Clear()
{
    while (_iCount > 0)
    {
        Remove(_head._pNext);
    }
}

// VTABLE: SWBF2 0x007af5a8
class Thread
{
public:
    Thread();

    virtual ~Thread();
    virtual bool Update(float fDeltaTime) = 0;
    virtual void ActivateThread(int iOrder, ThreadType eThreadType);
    virtual void DeactivateThread();
    // FUNCTION: SWBF2 0x00429200
    virtual bool IsThreadActive() { return mThreadListNode._pList != 0; }

    typedef PblOrderedList<Thread, int>::Node ThreadListNode;
    typedef PblOrderedList<Thread, int> ThreadList;

    ThreadListNode mThreadListNode;

    static ThreadListNode* sExecutingThread;

    static void EnterClient();
    static void LeaveClient();
    static void Reset();

    static void UpdateAll(float fDeltaTime, bool bFrame, bool bLocalTurn);
};
 
 
 
 
 
 
 
 
 

class ThreadGroup
{
public:
    ThreadGroup(int iPriority);
    ~ThreadGroup();

    ThreadGroup* FindCreate(int iPriority);
    void AddBefore(ThreadGroup* pNext);
    void Remove();
    void Clear();
    void Update(float fDeltaTime);
    void UpdateAll(float fDeltaTime);

    int mPriority;
    ThreadGroup* mNext;
    ThreadGroup* mPrev;
    PblOrderedList<Thread, int> mThreads;
};
 
 
