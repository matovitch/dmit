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

private:

    std::vector<uint8_t> _asBytes;
};

} // namespace dmit::vm
