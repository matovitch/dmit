#include "base64/base64.hpp"

#include <cstdint>
#include <limits>

namespace 
{

static constexpr std::size_t DECODED_BATCH_SIZE = 3;
static constexpr std::size_t ENCODED_BATCH_SIZE = 4;
static constexpr std::size_t ENCODE_SHIFT       = 6;
static constexpr std::size_t DECODE_SHIFT       = 2;
static constexpr std::size_t ENCODE_MASK        = 0b111111;
static constexpr uint8_t     PADDING_BYTE       = static_cast<uint8_t>('=');

static constexpr uint8_t ENCODING_MAP[] = 
{ 
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
    0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
    0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x78, 0x79, 0x7a, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x2b, 0x2f
};

static constexpr uint8_t DECODING_MAP[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x3f, 0x00, 0x00, 0x00,
    0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33
};

uint8_t encodeByte(const uint32_t toEncode)
{
    return ENCODING_MAP[toEncode];
}

uint8_t decodeByte(const uint32_t toDecode)
{
    return DECODING_MAP[toDecode];
}

void encodeBatch(const uint8_t* const srce,
                       uint8_t* const dest)
{
    uint32_t batchAsUInt32 = 0;

    uint8_t* batch = reinterpret_cast<uint8_t*>(&batchAsUInt32); 

    for (std::size_t i = 1; i <= DECODED_BATCH_SIZE; i++)
    {
        batch[DECODED_BATCH_SIZE - i] = srce[i - 1];
    }

    for (std::size_t i = ENCODED_BATCH_SIZE; i > 0; i--)
    {
        dest[i - 1] = encodeByte(batchAsUInt32 & ENCODE_MASK);
        batchAsUInt32 >>= ENCODE_SHIFT;
    }
}

void decodeBatch(const uint8_t* const srce,
                       uint8_t* const dest)
{
    uint32_t batchAsUInt32 = 0;

    for (std::size_t i = 0; i < ENCODED_BATCH_SIZE; i++)
    {
        batchAsUInt32 |= (decodeByte(srce[i]) << (ENCODE_SHIFT * (ENCODED_BATCH_SIZE - i)));
    }

    batchAsUInt32 <<= DECODE_SHIFT;

    const uint8_t* const batch = reinterpret_cast<const uint8_t*>(&batchAsUInt32);

    for (std::size_t i = 0; i < DECODED_BATCH_SIZE; i++)
    {
        dest[i] = batch[DECODED_BATCH_SIZE - i];
    }
}

void encodePadding(const uint8_t* const srce,
                   const std::size_t    size,
                         uint8_t* const dest)
{
    const std::size_t remainder = size % DECODED_BATCH_SIZE;

    uint32_t pseudoBatchAsUint32 = 0;

    uint8_t* pseudoBatch = reinterpret_cast<uint8_t*>(&pseudoBatchAsUint32);

    for (std::size_t i = remainder; i > 0; i--)
    {
        pseudoBatch[remainder - i] = srce[size - i];
    }

    const std::size_t destSize = base64::encodeBufferSize(size);

    encodeBatch(pseudoBatch, dest + destSize - ENCODED_BATCH_SIZE);

    for (std::size_t i = 1; i <= DECODED_BATCH_SIZE - remainder; i++)
    {
        dest[destSize - i] = PADDING_BYTE;
    }
}

void decodePadding(const uint8_t* const srce,
                         uint8_t* const dest)
{
    uint32_t pseudoBatchAsUint32 = 0;

    uint8_t* pseudoBatch = reinterpret_cast<uint8_t*>(&pseudoBatchAsUint32);

    decodeBatch(srce, pseudoBatch);

    for (std::size_t i = 0; i < base64::decodeBufferSize(srce, ENCODED_BATCH_SIZE); i++)
    {
        dest[i] = pseudoBatch[i];
    }
}

} // namespace 

namespace base64
{

std::size_t encodeBufferSize(const std::size_t size)
{
    return (size % DECODED_BATCH_SIZE) ? (size / DECODED_BATCH_SIZE + 1) * ENCODED_BATCH_SIZE
                                       : (size / DECODED_BATCH_SIZE    ) * ENCODED_BATCH_SIZE;
}

std::size_t decodeBufferSize(const uint8_t* const srce,
                             const std::size_t    size)
{
    std::size_t decodedSize = size * DECODED_BATCH_SIZE / ENCODED_BATCH_SIZE;

    for (const uint8_t* ptr = srce + size - 1; *ptr == PADDING_BYTE; ptr--)
    {
        decodedSize--;
    }

    return decodedSize;
}

void encode(const uint8_t* const srce,
            const std::size_t    size,
                  uint8_t* const dest)
{
    for (std::size_t i = 0; i < size / DECODED_BATCH_SIZE; i++)
    {
        encodeBatch(srce + i * DECODED_BATCH_SIZE,
                    dest + i * ENCODED_BATCH_SIZE);
    }

    if (size % DECODED_BATCH_SIZE)
    {
        encodePadding(srce, size, dest);
    }    
}

void decode(const uint8_t* const srce,
            const std::size_t    size,
                  uint8_t* const dest)
{
    for (std::size_t i = 0; i < size / ENCODED_BATCH_SIZE; i++)
    {
        const uint8_t* const batchSrce = srce + i * ENCODED_BATCH_SIZE;
              uint8_t* const batchDest = dest + i * DECODED_BATCH_SIZE;

        if (batchSrce[ENCODED_BATCH_SIZE - 1] == PADDING_BYTE)
        {
            decodePadding(batchSrce,
                          batchDest);
            break;
        }

        decodeBatch(batchSrce,
                    batchDest);
    }
}

} // namespace base64
