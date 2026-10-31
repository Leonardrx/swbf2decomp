#ifndef SWBF2_SUPPORT_PBLREF_H
#define SWBF2_SUPPORT_PBLREF_H


// VTABLE: SWBF2 0x00771bcc
class PblRef
{
public:

    PblRef() : mValidFlag(1), mAliveFlag(1), mRefCount(1)
    {
    }

    virtual void Kill();

    // SYNTHETIC: SWBF2 0x0050b410
    // PblRef::~PblRef
    virtual ~PblRef()
    {
        mValidFlag = 0;
        mAliveFlag = 0;
    }

    unsigned short AddRef() const
    {
        ++mRefCount;
        return mRefCount;
    }

    // FUNCTION: SWBF2 0x0040cbb0
    unsigned short Release() const
    {
        --mRefCount;
        if (mRefCount == 0)
        {
            delete this;
            return 0;
        }
        return mRefCount;
    }

    bool IsAlive() const
    {
        return mAliveFlag;
    }

    unsigned short mValidFlag : 1;
    unsigned short mAliveFlag : 1;
    mutable unsigned short mRefCount : 14;
};
 

 #endif
