#pragma once

#include <cstdint>

namespace dmit::com::base64
{

void encode(const uint8_t* const srce,
            const std::size_t    size,
                  uint8_t* const dest);

void decode(const uint8_t* const srce,
            const std::size_t    size,
                  uint8_t* const dest);

static constexpr std::size_t K_DECODED_BATCH_SIZE = 3;
static constexpr std::size_t K_ENCODED_BATCH_SIZE = 4;

constexpr std::size_t encodeBufferSize(const std::size_t size)
{
    return (size % K_DECODED_BATCH_SIZE) ? (size / K_DECODED_BATCH_SIZE + 1) * K_ENCODED_BATCH_SIZE
                                         : (size / K_DECODED_BATCH_SIZE    ) * K_ENCODED_BATCH_SIZE;
}

std::size_t decodeBufferSize(const uint8_t* const srce,
                             const std::size_t    size);

} // namespace dmit::com::base64
