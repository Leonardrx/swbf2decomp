#include "Allocator.h"

namespace util
{
namespace
{
// SYNTHETIC: SWBF2 0x00408b40
// util::`anonymous namespace'::`dynamic initializer for 'gStackAllocator_''
// SYNTHETIC: SWBF2 0x00769a60
// util::`anonymous namespace'::`dynamic atexit destructor for 'gStackAllocator_''
// GLOBAL: SWBF2 0x007dfb48
// `anonymous namespace'::gStackAllocator_
StackAllocator gStackAllocator_(true, true, 32);

// GLOBAL: SWBF2 0x007dfb88
// `anonymous namespace'::gStackBlock_
StaticMemoryBlock<1024> gStackBlock_;

// SYNTHETIC: SWBF2 0x00408b50
// util::`anonymous namespace'::`dynamic initializer for 'gStaticAllocator_''
// SYNTHETIC: SWBF2 0x00769aa0
// util::`anonymous namespace'::`dynamic atexit destructor for 'gStaticAllocator_''
// GLOBAL: SWBF2 0x007dff90
// `anonymous namespace'::gStaticAllocator_
StackAllocator gStaticAllocator_(true, false, 32);

// GLOBAL: SWBF2 0x007dffd0
// `anonymous namespace'::gStaticBlock_
StaticMemoryBlock<4096> gStaticBlock_;
}

// GLOBAL: SWBF2 0x009a1a30
char* gLastError;

// FUNCTION: SWBF2 0x006e9200
void SetLastError(char* pError)
{
    gLastError = pError;
}

// FUNCTION: SWBF2 0x006e9030
void StackAllocator::Allocate(Buffer& rBuf)
{
    unsigned int uSize = rBuf.size_;
    char* pPos = pos_;
    if (uSize > (unsigned int)(pool_.data_ + pool_.size_ - pPos))
    {
        if (alloc_)
        {
            alloc_->Allocate(rBuf);
        }
        return;
    }

    rBuf.data_ = pPos;
    pos_ += uSize;
    sizes_[bpos_] = rBuf.size_;
    bpos_ = (bpos_ + 1) % 8;
}

// FUNCTION: SWBF2 0x006e9120
void StackAllocator::Deallocate(Buffer& rBuf)
{
    if (!dealloc_)
    {
        return;
    }

    if (alloc_ && ((char*)rBuf.data_ < pool_.data_
                   || (char*)rBuf.data_ >= pool_.data_ + pool_.size_))
    {
        alloc_->Deallocate(rBuf);
        return;
    }

    bpos_ = (bpos_ - 1) % 8;
    pos_ -= rBuf.size_;
    rBuf.data_ = 0;
}

// TEMPLATE: SWBF2 0x006e8ff0
// util::StaticMemoryBlock<4096>::Allocate
template void StaticMemoryBlock<4096>::Allocate(Buffer&);
// TEMPLATE: SWBF2 0x006e90e0
// util::StaticMemoryBlock<4096>::Deallocate
template void StaticMemoryBlock<4096>::Deallocate(Buffer&);

// TEMPLATE: SWBF2 0x006e9010
// util::StaticMemoryBlock<1024>::Allocate
template void StaticMemoryBlock<1024>::Allocate(Buffer&);
// TEMPLATE: SWBF2 0x006e9100
// util::StaticMemoryBlock<1024>::Deallocate
template void StaticMemoryBlock<1024>::Deallocate(Buffer&);

// SYNTHETIC: SWBF2 0x006e8fa0
// util::StackAllocator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006e9090
void* Allocate_(unsigned int uiSize, IAllocator& rAlloc)
{
    Buffer buf(0, uiSize);
    rAlloc.Allocate(buf);
    return buf.data_;
}

// GLOBAL: SWBF2 0x009a1a34
HashMultiAllocator* HashMultiAllocator::sDefault;

// FUNCTION: SWBF2 0x006e90c0
void HashMultiAllocator::ClearDefault()
{
    delete sDefault;
    sDefault = 0;
}

IAllocator& DefaultStackAllocator::GetInstance()
{
    static bool first_ = true;
    if (first_)
    {
        Buffer buf(0, 1024);
        gStackBlock_.Allocate(buf);
        gStackAllocator_.Set(buf);
        first_ = false;
    }
    return gStackAllocator_;
}

// FUNCTION: SWBF2 0x006e91b0
IAllocator& DefaultStaticAllocator::GetInstance()
{
    // GLOBAL: SWBF2 0x007e0fd8
    static bool first_ = true;
    if (first_)
    {
        Buffer buf(0, 4096);
        gStaticBlock_.Allocate(buf);
        gStaticAllocator_.Set(buf);
        first_ = false;
    }
    return gStaticAllocator_;
}

// FUNCTION: SWBF2 0x006e91f0
void ResetMemory_()
{
    HashMultiAllocator::ClearDefault();
}

}

namespace util
{

// FUNCTION: SWBF2 0x006e9180
void Deallocate_(void* p, unsigned int uiSize, util::IAllocator& rAlloc)
{
    if (p)
    {
        util::Buffer buf(p, uiSize);
        rAlloc.Deallocate(buf);
    }
}

}
