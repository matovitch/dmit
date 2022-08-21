#pragma once

#include "dmit/vm/storage.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

namespace dmit::vm
{

class StackOp
{

public:

    using Storage = TStorage<uint64_t>;

    StackOp() = delete;

    StackOp(Storage&);

    void push(const uint64_t);

    void drop();

    uint64_t look() const;

    int64_t offset() const;

    const uint8_t* asBytes() const;

    template <class Type>
    Type load(const int64_t address) const
    {
        return *(reinterpret_cast<const Type*>(_base - address));
    }

    template <class Type>
    void store(const int64_t address, const Type value)
    {
        std::memcpy(reinterpret_cast<uint8_t*>(_base - address),
                    reinterpret_cast<const uint8_t*>(&value), sizeof(Type));
    }

private:
          uint64_t*       _head;
          uint64_t* const _base;
    const uint64_t* const _tail;
};

} // namespace dmit::vm
