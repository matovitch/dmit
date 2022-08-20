#pragma once

#include "dmit/com/type_flag.hpp"

extern "C"
{
    #include "cmp/cmp.h"
}

#include "dmit/nng/nng.hpp"

#include <optional>
#include <cstdint>
#include <cstring>
#include <utility>

#define DMIT_CMP_WRITE_CONTAINER(container, context) serialize(container.begin (), \
                                                               container.end   (), \
                                                               context)

#define DMIT_CMP_ALIAS(f, g) template <class... Args>                   \
                             auto g(Args&&... args) -> bool             \
                             {                                          \
                                 return f(std::forward<Args>(args)...); \
                             }

namespace dmit::cmp
{

DMIT_CMP_ALIAS(::cmp_write_integer, writeInteger)
DMIT_CMP_ALIAS(::cmp_write_uinteger, writeUinteger)
DMIT_CMP_ALIAS(::cmp_write_decimal, writeDecimal)
DMIT_CMP_ALIAS(::cmp_write_nil, writeNil)
DMIT_CMP_ALIAS(::cmp_write_true, writeTrue)
DMIT_CMP_ALIAS(::cmp_write_false, writeFalse)
DMIT_CMP_ALIAS(::cmp_write_bool, writeBool)
DMIT_CMP_ALIAS(::cmp_write_u8_as_bool, writeU8AsBool)
DMIT_CMP_ALIAS(::cmp_write_str, writeStr)
DMIT_CMP_ALIAS(::cmp_write_str_v4, writeStrV4)
DMIT_CMP_ALIAS(::cmp_write_str_marker, writeStrMarker)
DMIT_CMP_ALIAS(::cmp_write_str_marker_v4, writeStrMarkerV4)
DMIT_CMP_ALIAS(::cmp_write_bin, writeBin)
DMIT_CMP_ALIAS(::cmp_write_bin_marker, writeBinMarker)
DMIT_CMP_ALIAS(::cmp_write_array, writeArray)
DMIT_CMP_ALIAS(::cmp_write_map, writeMap)
DMIT_CMP_ALIAS(::cmp_write_ext, writeExt)
DMIT_CMP_ALIAS(::cmp_write_ext_marker, writeExtMarker)
DMIT_CMP_ALIAS(::cmp_write_object, writeObject)
DMIT_CMP_ALIAS(::cmp_write_object_v4, writeObjectV4)
DMIT_CMP_ALIAS(::cmp_write_pfix, writePfix)
DMIT_CMP_ALIAS(::cmp_write_nfix, writeNfix)
DMIT_CMP_ALIAS(::cmp_write_sfix, writeSfix)
DMIT_CMP_ALIAS(::cmp_write_s8, writeS8)
DMIT_CMP_ALIAS(::cmp_write_s16, writeS16)
DMIT_CMP_ALIAS(::cmp_write_s32, writeS32)
DMIT_CMP_ALIAS(::cmp_write_s64, writeS64)
DMIT_CMP_ALIAS(::cmp_write_ufix, writeUfix)
DMIT_CMP_ALIAS(::cmp_write_u8, writeU8)
DMIT_CMP_ALIAS(::cmp_write_u16, writeU16)
DMIT_CMP_ALIAS(::cmp_write_u32, writeU32)
DMIT_CMP_ALIAS(::cmp_write_u64, writeU64)
DMIT_CMP_ALIAS(::cmp_write_float, writeFloat)
DMIT_CMP_ALIAS(::cmp_write_double, writeDouble)
DMIT_CMP_ALIAS(::cmp_write_fixstr_marker, writeFixstrMarker)
DMIT_CMP_ALIAS(::cmp_write_fixstr, writeFixstr)
DMIT_CMP_ALIAS(::cmp_write_str8_marker, writeStr8Marker)
DMIT_CMP_ALIAS(::cmp_write_str8, writeStr8)
DMIT_CMP_ALIAS(::cmp_write_str16_marker, writeStr16Marker)
DMIT_CMP_ALIAS(::cmp_write_str16, writeStr16)
DMIT_CMP_ALIAS(::cmp_write_str32_marker, writeStr32Marker)
DMIT_CMP_ALIAS(::cmp_write_str32, writeStr32)
DMIT_CMP_ALIAS(::cmp_write_bin8_marker, writeBin8Marker)
DMIT_CMP_ALIAS(::cmp_write_bin8, writeBin8)
DMIT_CMP_ALIAS(::cmp_write_bin16_marker, writeBin16Marker)
DMIT_CMP_ALIAS(::cmp_write_bin16, writeBin16)
DMIT_CMP_ALIAS(::cmp_write_bin32_marker, writeBin32Marker)
DMIT_CMP_ALIAS(::cmp_write_bin32, writeBin32)
DMIT_CMP_ALIAS(::cmp_write_fixarray, writeFixarray)
DMIT_CMP_ALIAS(::cmp_write_array16, writeArray16)
DMIT_CMP_ALIAS(::cmp_write_array32, writeArray32)
DMIT_CMP_ALIAS(::cmp_write_fixmap, writeFixmap)
DMIT_CMP_ALIAS(::cmp_write_map16, writeMap16)
DMIT_CMP_ALIAS(::cmp_write_map32, writeMap32)
DMIT_CMP_ALIAS(::cmp_write_fixext1_marker, writeFixext1Marker)
DMIT_CMP_ALIAS(::cmp_write_fixext1, writeFixext1)
DMIT_CMP_ALIAS(::cmp_write_fixext2_marker, writeFixext2Marker)
DMIT_CMP_ALIAS(::cmp_write_fixext2, writeFixext2)
DMIT_CMP_ALIAS(::cmp_write_fixext4_marker, writeFixext4Marker)
DMIT_CMP_ALIAS(::cmp_write_fixext4, writeFixext4)
DMIT_CMP_ALIAS(::cmp_write_fixext8_marker, writeFixext8Marker)
DMIT_CMP_ALIAS(::cmp_write_fixext8, writeFixext8)
DMIT_CMP_ALIAS(::cmp_write_fixext16_marker, writeFixext16Marker)
DMIT_CMP_ALIAS(::cmp_write_fixext16, writeFixext16)
DMIT_CMP_ALIAS(::cmp_write_ext8_marker, writeExt8Marker)
DMIT_CMP_ALIAS(::cmp_write_ext8, writeExt8)
DMIT_CMP_ALIAS(::cmp_write_ext16_marker, writeExt16Marker)
DMIT_CMP_ALIAS(::cmp_write_ext16, writeExt16)
DMIT_CMP_ALIAS(::cmp_write_ext32_marker, writeExt32Marker)
DMIT_CMP_ALIAS(::cmp_write_ext32, writeExt32)
DMIT_CMP_ALIAS(::cmp_read_char, readChar)
DMIT_CMP_ALIAS(::cmp_read_short, readShort)
DMIT_CMP_ALIAS(::cmp_read_int, readInt)
DMIT_CMP_ALIAS(::cmp_read_long, readLong)
DMIT_CMP_ALIAS(::cmp_read_integer, readInteger)
DMIT_CMP_ALIAS(::cmp_read_uchar, readUchar)
DMIT_CMP_ALIAS(::cmp_read_ushort, readUshort)
DMIT_CMP_ALIAS(::cmp_read_uint, readUint)
DMIT_CMP_ALIAS(::cmp_read_ulong, readUlong)
DMIT_CMP_ALIAS(::cmp_read_uinteger, readUinteger)
DMIT_CMP_ALIAS(::cmp_read_decimal, readDecimal)
DMIT_CMP_ALIAS(::cmp_read_nil, readNil)
DMIT_CMP_ALIAS(::cmp_read_bool, readBool)
DMIT_CMP_ALIAS(::cmp_read_bool_as_u8, readBoolAsU8)
DMIT_CMP_ALIAS(::cmp_read_str_size, readStrSize)
DMIT_CMP_ALIAS(::cmp_read_str, readStr)
DMIT_CMP_ALIAS(::cmp_read_bin_size, readBinSize)
DMIT_CMP_ALIAS(::cmp_read_bin, readBin)
DMIT_CMP_ALIAS(::cmp_read_array, readArray)
DMIT_CMP_ALIAS(::cmp_read_map, readMap)
DMIT_CMP_ALIAS(::cmp_read_ext_marker, readExtMarker)
DMIT_CMP_ALIAS(::cmp_read_ext, readExt)
DMIT_CMP_ALIAS(::cmp_read_object, readObject)
DMIT_CMP_ALIAS(::cmp_read_pfix, readPfix)
DMIT_CMP_ALIAS(::cmp_read_nfix, readNfix)
DMIT_CMP_ALIAS(::cmp_read_sfix, readSfix)
DMIT_CMP_ALIAS(::cmp_read_s8, readS8)
DMIT_CMP_ALIAS(::cmp_read_s16, readS16)
DMIT_CMP_ALIAS(::cmp_read_s32, readS32)
DMIT_CMP_ALIAS(::cmp_read_s64, readS64)
DMIT_CMP_ALIAS(::cmp_read_ufix, readUfix)
DMIT_CMP_ALIAS(::cmp_read_u8, readU8)
DMIT_CMP_ALIAS(::cmp_read_u16, readU16)
DMIT_CMP_ALIAS(::cmp_read_u32, readU32)
DMIT_CMP_ALIAS(::cmp_read_u64, readU64)
DMIT_CMP_ALIAS(::cmp_read_float, readFloat)
DMIT_CMP_ALIAS(::cmp_read_double, readDouble)
DMIT_CMP_ALIAS(::cmp_read_fixext1_marker, readFixext1Marker)
DMIT_CMP_ALIAS(::cmp_read_fixext1, readFixext1)
DMIT_CMP_ALIAS(::cmp_read_fixext2_marker, readFixext2Marker)
DMIT_CMP_ALIAS(::cmp_read_fixext2, readFixext2)
DMIT_CMP_ALIAS(::cmp_read_fixext4_marker, readFixext4Marker)
DMIT_CMP_ALIAS(::cmp_read_fixext4, readFixext4)
DMIT_CMP_ALIAS(::cmp_read_fixext8_marker, readFixext8Marker)
DMIT_CMP_ALIAS(::cmp_read_fixext8, readFixext8)
DMIT_CMP_ALIAS(::cmp_read_fixext16_marker, readFixext16Marker)
DMIT_CMP_ALIAS(::cmp_read_fixext16, readFixext16)
DMIT_CMP_ALIAS(::cmp_read_ext8_marker, readExt8Marker)
DMIT_CMP_ALIAS(::cmp_read_ext8, readExt8)
DMIT_CMP_ALIAS(::cmp_read_ext16_marker, readExt16Marker)
DMIT_CMP_ALIAS(::cmp_read_ext16, readExt16)
DMIT_CMP_ALIAS(::cmp_read_ext32_marker, readExt32Marker)
DMIT_CMP_ALIAS(::cmp_read_ext32, readExt32)
DMIT_CMP_ALIAS(::cmp_read_sinteger, readSinteger)

cmp_ctx_t contextFromNngBuffer(nng::Buffer& nngBuffer);

// TODO Make a PR onto cmp to improve this
bool readBytes(cmp_ctx_t* ctx, void* data, uint32_t limit);

template <class Serializer, class... Args>
std::optional<nng::Buffer> asNngBuffer(Serializer&& serializer, Args&&... args)
{
    // 1. Compute the buffer size

    cmp_ctx_t cmpBufferSize = {0};

    auto writerSize = [](cmp_ctx_t* ctx, const void *data, size_t count)
                      {
                          ctx->buf = (char*)(ctx->buf) + count;
                          return count;
                      };

    cmp_init(&cmpBufferSize, nullptr, nullptr, nullptr, writerSize);

    if (!serializer(&cmpBufferSize, std::forward<Args>(args)...))
    {
        return std::nullopt;
    }

    const size_t size = *((size_t*)(&(cmpBufferSize.buf)));

    // 2. Allocate and write to the buffer

    nng::Buffer nngBuffer{size};

    cmp_ctx_t cmpBuffer = contextFromNngBuffer(nngBuffer);

    if (!serializer(&cmpBuffer, std::forward<Args>(args)...))
    {
        return std::nullopt;
    }

    return std::optional<nng::Buffer>{std::move(nngBuffer)};
}

template <class Iterator>
bool write(Iterator begin, Iterator end, cmp_ctx_t* contextPtr)
{
    // 1. Create the array

    cmp_ctx_t contextCopy;

    std::memcpy(&contextCopy, contextPtr, sizeof(cmp_ctx_t));

    if (!writeArray32(contextPtr, 0))
    {
        return false;
    }

    // 2. Write to it

    uint32_t size = 0;

    for (auto it = begin; it != end; it++)
    {
        if (!write(*it, contextPtr))
        {
            return false;
        }

        size++;
    }

    // 3. Indicate the size "a posteriori"

    if (!writeArray32(&contextCopy, size))
    {
        return false;
    }

    return true;
}

} // namespace dmit::cmp
