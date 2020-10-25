#pragma once

#include "dmit/com/type_flag.hpp"

#include "cmp/cmp.h"

#include "dmit/nng/nng.hpp"

#include <optional>
#include <cstdint>
#include <cstring>
#include <utility>

#define DMIT_CMP_WRITE_CONTAINER(container, context) serialize(container.begin (), \
                                                               container.end   (), \
                                                               context)
namespace dmit::cmp
{

cmp_ctx_t contextFromNngBuffer(dmit::nng::Buffer& nngBuffer);

template <class Type, class Serializer>
std::optional<dmit::nng::Buffer> asNngBuffer(const Type& value, Serializer&& serializer)
{
    // 1. Compute the buffer size

    cmp_ctx_t cmpBufferSize = {0};

    auto writerSize = [](cmp_ctx_t* ctx, const void *data, size_t count)
                      {
                          ctx->buf = (char*)(ctx->buf) + count;
                          return count;
                      };

    cmp_init(&cmpBufferSize, nullptr, nullptr, nullptr, writerSize);

    if (!serializer(value, &cmpBufferSize))
    {
        return std::nullopt;
    }

    const size_t size = *((size_t*)(&(cmpBufferSize.buf)));

    // 2. Allocate and write to the buffer

    dmit::nng::Buffer nngBuffer{size};

    cmp_ctx_t cmpBuffer = contextFromNngBuffer(nngBuffer);

    if (serializer(value, &cmpBuffer))
    {
        return std::nullopt;
    }

    return std::optional<dmit::nng::Buffer>{nngBuffer};
}

#define ALIAS(f, g) template <class... Args> auto g(Args&&... args) -> bool { return f(std::forward<Args>(args)...); }

ALIAS(::cmp_write_integer, writeInteger)
ALIAS(::cmp_write_uinteger, writeUinteger)
ALIAS(::cmp_write_decimal, writeDecimal)
ALIAS(::cmp_write_nil, writeNil)
ALIAS(::cmp_write_true, writeTrue)
ALIAS(::cmp_write_false, writeFalse)
ALIAS(::cmp_write_bool, writeBool)
ALIAS(::cmp_write_u8_as_bool, writeU8AsBool)
ALIAS(::cmp_write_str, writeStr)
ALIAS(::cmp_write_str_v4, writeStrV4)
ALIAS(::cmp_write_str_marker, writeStrMarker)
ALIAS(::cmp_write_str_marker_v4, writeStrMarkerV4)
ALIAS(::cmp_write_bin, writeBin)
ALIAS(::cmp_write_bin_marker, writeBinMarker)
ALIAS(::cmp_write_array, writeArray)
ALIAS(::cmp_write_map, writeMap)
ALIAS(::cmp_write_ext, writeExt)
ALIAS(::cmp_write_ext_marker, writeExtMarker)
ALIAS(::cmp_write_object, writeObject)
ALIAS(::cmp_write_object_v4, writeObjectV4)
ALIAS(::cmp_write_pfix, writePfix)
ALIAS(::cmp_write_nfix, writeNfix)
ALIAS(::cmp_write_sfix, writeSfix)
ALIAS(::cmp_write_s8, writeS8)
ALIAS(::cmp_write_s16, writeS16)
ALIAS(::cmp_write_s32, writeS32)
ALIAS(::cmp_write_s64, writeS64)
ALIAS(::cmp_write_ufix, writeUfix)
ALIAS(::cmp_write_u8, writeU8)
ALIAS(::cmp_write_u16, writeU16)
ALIAS(::cmp_write_u32, writeU32)
ALIAS(::cmp_write_u64, writeU64)
ALIAS(::cmp_write_float, writeFloat)
ALIAS(::cmp_write_double, writeDouble)
ALIAS(::cmp_write_fixstr_marker, writeFixstrMarker)
ALIAS(::cmp_write_fixstr, writeFixstr)
ALIAS(::cmp_write_str8_marker, writeStr8Marker)
ALIAS(::cmp_write_str8, writeStr8)
ALIAS(::cmp_write_str16_marker, writeStr16Marker)
ALIAS(::cmp_write_str16, writeStr16)
ALIAS(::cmp_write_str32_marker, writeStr32Marker)
ALIAS(::cmp_write_str32, writeStr32)
ALIAS(::cmp_write_bin8_marker, writeBin8Marker)
ALIAS(::cmp_write_bin8, writeBin8)
ALIAS(::cmp_write_bin16_marker, writeBin16Marker)
ALIAS(::cmp_write_bin16, writeBin16)
ALIAS(::cmp_write_bin32_marker, writeBin32Marker)
ALIAS(::cmp_write_bin32, writeBin32)
ALIAS(::cmp_write_fixarray, writeFixarray)
ALIAS(::cmp_write_array16, writeArray16)
ALIAS(::cmp_write_array32, writeArray32)
ALIAS(::cmp_write_fixmap, writeFixmap)
ALIAS(::cmp_write_map16, writeMap16)
ALIAS(::cmp_write_map32, writeMap32)
ALIAS(::cmp_write_fixext1_marker, writeFixext1Marker)
ALIAS(::cmp_write_fixext1, writeFixext1)
ALIAS(::cmp_write_fixext2_marker, writeFixext2Marker)
ALIAS(::cmp_write_fixext2, writeFixext2)
ALIAS(::cmp_write_fixext4_marker, writeFixext4Marker)
ALIAS(::cmp_write_fixext4, writeFixext4)
ALIAS(::cmp_write_fixext8_marker, writeFixext8Marker)
ALIAS(::cmp_write_fixext8, writeFixext8)
ALIAS(::cmp_write_fixext16_marker, writeFixext16Marker)
ALIAS(::cmp_write_fixext16, writeFixext16)
ALIAS(::cmp_write_ext8_marker, writeExt8Marker)
ALIAS(::cmp_write_ext8, writeExt8)
ALIAS(::cmp_write_ext16_marker, writeExt16Marker)
ALIAS(::cmp_write_ext16, writeExt16)
ALIAS(::cmp_write_ext32_marker, writeExt32Marker)
ALIAS(::cmp_write_ext32, writeExt32)
ALIAS(::cmp_read_char, readChar)
ALIAS(::cmp_read_short, readShort)
ALIAS(::cmp_read_int, readInt)
ALIAS(::cmp_read_long, readLong)
ALIAS(::cmp_read_integer, readInteger)
ALIAS(::cmp_read_uchar, readUchar)
ALIAS(::cmp_read_ushort, readUshort)
ALIAS(::cmp_read_uint, readUint)
ALIAS(::cmp_read_ulong, readUlong)
ALIAS(::cmp_read_uinteger, readUinteger)
ALIAS(::cmp_read_decimal, readDecimal)
ALIAS(::cmp_read_nil, readNil)
ALIAS(::cmp_read_bool, readBool)
ALIAS(::cmp_read_bool_as_u8, readBoolAsU8)
ALIAS(::cmp_read_str_size, readStrSize)
ALIAS(::cmp_read_str, readStr)
ALIAS(::cmp_read_bin_size, readBinSize)
ALIAS(::cmp_read_bin, readBin)
ALIAS(::cmp_read_array, readArray)
ALIAS(::cmp_read_map, readMap)
ALIAS(::cmp_read_ext_marker, readExtMarker)
ALIAS(::cmp_read_ext, readExt)
ALIAS(::cmp_read_object, readObject)
ALIAS(::cmp_read_pfix, readPfix)
ALIAS(::cmp_read_nfix, readNfix)
ALIAS(::cmp_read_sfix, readSfix)
ALIAS(::cmp_read_s8, readS8)
ALIAS(::cmp_read_s16, readS16)
ALIAS(::cmp_read_s32, readS32)
ALIAS(::cmp_read_s64, readS64)
ALIAS(::cmp_read_ufix, readUfix)
ALIAS(::cmp_read_u8, readU8)
ALIAS(::cmp_read_u16, readU16)
ALIAS(::cmp_read_u32, readU32)
ALIAS(::cmp_read_u64, readU64)
ALIAS(::cmp_read_float, readFloat)
ALIAS(::cmp_read_double, readDouble)
ALIAS(::cmp_read_fixext1_marker, readFixext1Marker)
ALIAS(::cmp_read_fixext1, readFixext1)
ALIAS(::cmp_read_fixext2_marker, readFixext2Marker)
ALIAS(::cmp_read_fixext2, readFixext2)
ALIAS(::cmp_read_fixext4_marker, readFixext4Marker)
ALIAS(::cmp_read_fixext4, readFixext4)
ALIAS(::cmp_read_fixext8_marker, readFixext8Marker)
ALIAS(::cmp_read_fixext8, readFixext8)
ALIAS(::cmp_read_fixext16_marker, readFixext16Marker)
ALIAS(::cmp_read_fixext16, readFixext16)
ALIAS(::cmp_read_ext8_marker, readExt8Marker)
ALIAS(::cmp_read_ext8, readExt8)
ALIAS(::cmp_read_ext16_marker, readExt16Marker)
ALIAS(::cmp_read_ext16, readExt16)
ALIAS(::cmp_read_ext32_marker, readExt32Marker)
ALIAS(::cmp_read_ext32, readExt32)
ALIAS(::cmp_read_sinteger, readSinteger)

template <class Iterator>
bool write(Iterator begin, Iterator end, cmp_ctx_t* contextPtr)
{
    // 1. Create the array

    cmp_ctx_t contextCopy;

    memcpy(&contextCopy, contextPtr, sizeof(cmp_ctx_t));

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
