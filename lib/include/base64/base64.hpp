#pragma once

#include <cstdint>

namespace base64
{

std::size_t encodeBufferSize(const std::size_t size);

std::size_t decodeBufferSize(const uint8_t* const srce,
                             const std::size_t    size);

void encode(const uint8_t* const srce,
            const std::size_t    size,
                  uint8_t* const dest);

void decode(const uint8_t* const srce,
            const std::size_t    size,
                  uint8_t* const dest);

} // namespace base64
