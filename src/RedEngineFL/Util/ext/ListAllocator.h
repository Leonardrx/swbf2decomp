#pragma once

#include "Allocator.h"
#include "basevector.h"
#include "baselist.h"

namespace util
{

// VTABLE: SWBF2 0x00783958
class DummyNonAllocator : public IAllocator
{
public:
    virtual void Allocate(Buffer& rBuf);
    virtual void Deallocate(Buffer& rBuf);
};

// VTABLE: SWBF2 0x00783978
class ListAllocator : public IAllocator
{
public:
    virtual void Allocate(Buffer& rBuf) = 0;
    virtual void Deallocate(Buffer& rBuf) = 0;
    virtual void SetMaxMem(int iMax) = 0;
    // FUNCTION: SWBF2 0x006e9310
    virtual ~ListAllocator() {}
    virtual void UnitTest_() = 0;

    static ListAllocator* Create(unsigned int uBaseMem, unsigned int uMaxMem);

protected:
    ListAllocator() {}
};



 
 

}
