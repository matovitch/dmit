#pragma once

#include <cstdint>

namespace dmit::sql
{

struct CanaryBuffer
{
    CanaryBuffer(const int32_t size);

    int32_t size() const;

    void reset(const int32_t size);

    ~CanaryBuffer();

    const int32_t _size;
    uint8_t* const _data;
};

} // namespace dmit::sql
