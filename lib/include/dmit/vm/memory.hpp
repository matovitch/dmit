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

    template <class Type>
    void store(const uint64_t address, const Type payload)
    {
        const uint8_t* payloadAsBytes = reinterpret_cast<const uint8_t*>(&payload);

        for (int i = 0; i < sizeof(Type); i++)
        {
            _asBytes[address] = payloadAsBytes[i];
        }
    }

private:

    std::vector<uint8_t> _asBytes;
};

} // namespace dmit::vm
