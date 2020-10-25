#pragma once

#include "dmit/com/type_flag.hpp"

#include "cmp/cmp.h"

#include "nng/nng.hpp"

#include <functional>
#include <cstdint>
#include <cstring>

namespace dmit::srl
{

struct Serializable {};

template <class Iterator>
bool serialize(Iterator begin, Iterator end, cmp_ctx_t* contextPtr)
{
    cmp_ctx_t contextCopy;

    memcpy(&contextCopy, contextPtr, sizeof(cmp_ctx_t));

    if (!cmp_write_array32(contextPtr, 0))
    {
        return false;
    }

    uint32_t size = 0;

    for (auto it = begin; it != end; it++)
    {
        if (!serialize(*it, contextPtr))
        {
            return false;
        }

        size++;
    }

    if (!cmp_write_array32(&contextCopy, size))
    {
        return false;
    }

    return true;
}

namespace
{

bool reader(cmp_ctx_t* ctx, void *data, size_t limit) {
    memcpy(data, ctx->buf, limit);
    ctx->buf = (char*)(ctx->buf) + limit;
    return true;
}

bool skipper(cmp_ctx_t* ctx, size_t count) {
    ctx->buf = (char*)(ctx->buf) + count;
    return true;
}

size_t writer(cmp_ctx_t* ctx, const void *data, size_t count) {
    memcpy(ctx->buf, data, count);
    ctx->buf = (char*)(ctx->buf) + count;
    return count;
}

size_t writerSize(cmp_ctx_t* ctx, const void *data, size_t count) {
    ctx->buf = (char*)(ctx->buf) + count;
    return count;
}

size_t bufferSize(cmp_ctx_t* ctx)
{
    return *((size_t*)(&(ctx->buf)));
}

} // namespace

cmp_ctx_t cmpContextFromNngBuffer(nng::Buffer& nngBuffer)
{
    cmp_ctx_t cmpBuffer = {0};

    cmp_init(&cmpBuffer, nngBuffer._asBytes, reader, skipper, writer);

    return cmpBuffer;
}

template <class Type, class Serializer>
std::optional<nng::Buffer> asNngBuffer(const Type& value, Serializer&& serializer)
{
    cmp_ctx_t cmpBufferSize = {0};

    cmp_init(&cmpBufferSize, nullptr, nullptr, nullptr, writerSize);

    if (!serializer(value, &cmpBufferSize))
    {
        return std::nullopt;
    }

    const size_t size = bufferSize(&cmpBufferSize);

    nng::Buffer nngBuffer{size};

    cmp_ctx_t cmpBuffer = cmpContextFromNngBuffer(nngBuffer);

    if (serializer(value, &cmpBuffer))
    {
        return std::nullopt;
    }

    return std::optional<nng::Buffer>{nngBuffer};
}

} // namespace dmit::srl

#define DMIT_SRL_CONTAINER_SERIALIZE(container, context) serialize(container.begin (), \
                                                                   container.end   (), \
                                                                   context)
