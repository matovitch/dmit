#pragma once

#include <cstdint>
#include <vector>

namespace dmit::vm
{

class Memory
{

public:

    void grow(const uint64_t size);

    uint64_t size() const;

    template <class Type>
    Type load(const uint64_t address) const
    {
        const uint8_t* const ptr = _asBytes.data() + address;

        return *(reinterpret_cast<const Type*>(ptr));
    }

private:

    std::vector<uint8_t> _asBytes;
};

} // namespace dmit::vm
