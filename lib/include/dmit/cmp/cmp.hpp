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

ALIAS(::cmp_write_integer, write_integer)
ALIAS(::cmp_write_uinteger, write_uinteger)
ALIAS(::cmp_write_decimal, write_decimal)
ALIAS(::cmp_write_nil, write_nil)
ALIAS(::cmp_write_true, write_true)
ALIAS(::cmp_write_false, write_false)
ALIAS(::cmp_write_bool, write_bool)
ALIAS(::cmp_write_u8_as_bool, write_u8_as_bool)
ALIAS(::cmp_write_str, write_str)
ALIAS(::cmp_write_str_v4, write_str_v4)
ALIAS(::cmp_write_str_marker, write_str_marker)
ALIAS(::cmp_write_str_marker_v4, write_str_marker_v4)
ALIAS(::cmp_write_bin, write_bin)
ALIAS(::cmp_write_bin_marker, write_bin_marker)
ALIAS(::cmp_write_array, write_array)
ALIAS(::cmp_write_map, write_map)
ALIAS(::cmp_write_ext, write_ext)
ALIAS(::cmp_write_ext_marker, write_ext_marker)
ALIAS(::cmp_write_object, write_object)
ALIAS(::cmp_write_object_v4, write_object_v4)
ALIAS(::cmp_write_pfix, write_pfix)
ALIAS(::cmp_write_nfix, write_nfix)
ALIAS(::cmp_write_sfix, write_sfix)
ALIAS(::cmp_write_s8, write_s8)
ALIAS(::cmp_write_s16, write_s16)
ALIAS(::cmp_write_s32, write_s32)
ALIAS(::cmp_write_s64, write_s64)
ALIAS(::cmp_write_ufix, write_ufix)
ALIAS(::cmp_write_u8, write_u8)
ALIAS(::cmp_write_u16, write_u16)
ALIAS(::cmp_write_u32, write_u32)
ALIAS(::cmp_write_u64, write_u64)
ALIAS(::cmp_write_float, write_float)
ALIAS(::cmp_write_double, write_double)
ALIAS(::cmp_write_fixstr_marker, write_fixstr_marker)
ALIAS(::cmp_write_fixstr, write_fixstr)
ALIAS(::cmp_write_str8_marker, write_str8_marker)
ALIAS(::cmp_write_str8, write_str8)
ALIAS(::cmp_write_str16_marker, write_str16_marker)
ALIAS(::cmp_write_str16, write_str16)
ALIAS(::cmp_write_str32_marker, write_str32_marker)
ALIAS(::cmp_write_str32, write_str32)
ALIAS(::cmp_write_bin8_marker, write_bin8_marker)
ALIAS(::cmp_write_bin8, write_bin8)
ALIAS(::cmp_write_bin16_marker, write_bin16_marker)
ALIAS(::cmp_write_bin16, write_bin16)
ALIAS(::cmp_write_bin32_marker, write_bin32_marker)
ALIAS(::cmp_write_bin32, write_bin32)
ALIAS(::cmp_write_fixarray, write_fixarray)
ALIAS(::cmp_write_array16, write_array16)
ALIAS(::cmp_write_array32, write_array32)
ALIAS(::cmp_write_fixmap, write_fixmap)
ALIAS(::cmp_write_map16, write_map16)
ALIAS(::cmp_write_map32, write_map32)
ALIAS(::cmp_write_fixext1_marker, write_fixext1_marker)
ALIAS(::cmp_write_fixext1, write_fixext1)
ALIAS(::cmp_write_fixext2_marker, write_fixext2_marker)
ALIAS(::cmp_write_fixext2, write_fixext2)
ALIAS(::cmp_write_fixext4_marker, write_fixext4_marker)
ALIAS(::cmp_write_fixext4, write_fixext4)
ALIAS(::cmp_write_fixext8_marker, write_fixext8_marker)
ALIAS(::cmp_write_fixext8, write_fixext8)
ALIAS(::cmp_write_fixext16_marker, write_fixext16_marker)
ALIAS(::cmp_write_fixext16, write_fixext16)
ALIAS(::cmp_write_ext8_marker, write_ext8_marker)
ALIAS(::cmp_write_ext8, write_ext8)
ALIAS(::cmp_write_ext16_marker, write_ext16_marker)
ALIAS(::cmp_write_ext16, write_ext16)
ALIAS(::cmp_write_ext32_marker, write_ext32_marker)
ALIAS(::cmp_write_ext32, write_ext32)

template <class Iterator>
bool write(Iterator begin, Iterator end, cmp_ctx_t* contextPtr)
{
    // 1. Create the array

    cmp_ctx_t contextCopy;

    memcpy(&contextCopy, contextPtr, sizeof(cmp_ctx_t));

    if (!write_array32(contextPtr, 0))
    {
        return false;
    }

    // 2. Write to it

    uint32_t size = 0;

    for (auto it = begin; it != end; it++)
    {
        if (!serialize(*it, contextPtr))
        {
            return false;
        }

        size++;
    }

    // 3. Indicate the size "a posteriori"

    if (!write_array32(&contextCopy, size))
    {
        return false;
    }

    return true;
}

} // namespace dmit::cmp
