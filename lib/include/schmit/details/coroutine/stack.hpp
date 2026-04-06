#pragma once

#include <sys/mman.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace schmit_details
{

namespace coroutine
{

namespace stack
{

class Pool
{

public:

    static constexpr std::size_t NUM_GUARD_PAGES = 1;
    static constexpr std::size_t NUM_STACK_PAGES = 256;

    static std::size_t stackSizeBytes()
    {
        return NUM_STACK_PAGES * static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
    }

    Pool()
    {
        const auto pageSize = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
        _guardSize = NUM_GUARD_PAGES * pageSize;
        _stackSize = stackSizeBytes();
    }

    ~Pool()
    {
        for (std::size_t index = 0; index < _batches.size(); index++)
        {
            ::munmap(_batches[index], (1 << index) * (_guardSize + _stackSize));
        }
    }

    uint8_t* make()
    {
        if (_freeList.empty())
        {
            grow();
        }

        if (_freeList.empty())
        {
            return nullptr;
        }

        uint8_t* const memory = _freeList.back();
                                _freeList.pop_back();
        return memory;
    }

    void recycle(uint8_t* memory)
    {
        _freeList.push_back(memory);
    }

    std::size_t stackSize() const
    {
        return _stackSize;
    }

private:

    void grow()
    {
        const std::size_t batchFrameCount = 1 << _batches.size();
        const std::size_t batchSizeBytes = batchFrameCount * (_guardSize + _stackSize);

        auto memory = static_cast<uint8_t*>(::mmap(nullptr,
                                                   batchSizeBytes,
                                                   PROT_READ | PROT_WRITE,
                                                   MAP_PRIVATE | MAP_ANONYMOUS,
                                                   -1,
                                                   0));
        if (memory == MAP_FAILED)
        {
            return;
        }

        for (std::size_t frameIndex = 0; frameIndex < batchFrameCount; frameIndex++)
        {
            uint8_t* const frameBase = memory + frameIndex * (_guardSize + _stackSize);

            ::mprotect(frameBase, _guardSize, PROT_NONE);
            _freeList.push_back(frameBase + _guardSize);
        }

        _batches.push_back(memory);
    }

    std::size_t _guardSize;
    std::size_t _stackSize;
    std::vector<uint8_t*> _freeList;
    std::vector<uint8_t*> _batches;
};

} // namespace stack

class Stack
{

public:

    using Pool = stack::Pool;

    // Note, the offset is needed to make room for
    // the return address used upon coroutine exit
    static constexpr std::size_t OFFSET = 0x08;

    Stack(Pool& pool) :
        _memory{pool.make()},
        _pool{pool}
        {}

    void recycle()
    {
        if (_memory)
        {
            _pool.recycle(_memory);
            _memory = nullptr;
        }
    }

    ~Stack()
    {
        recycle();
    }

    uint8_t* base()
    {
        return _memory ? _memory + _pool.stackSizeBytes() - OFFSET : nullptr;
    }

private:

    uint8_t* _memory;
    Pool& _pool;
};

} // namespace coroutine
} // namespace schmit_details
