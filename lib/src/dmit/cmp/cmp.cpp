#include "dmit/cmp/cmp.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/com/storage.hpp"

extern "C"
{
    #include "cmp/cmp.h"
}

#include <cstring>
#include <cstdint>

namespace dmit::cmp
{

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

} // namespace

bool readBytes(cmp_ctx_t* ctx, void* data, uint32_t limit)
{
    return ctx->read(ctx, data, limit);
}

cmp_ctx_t contextFromNngBuffer(nng::Buffer& nngBuffer)
{
    cmp_ctx_t cmpBuffer = {0};

    cmp_init(&cmpBuffer, nngBuffer._asBytes, reader, skipper, writer);

    return cmpBuffer;
}

cmp_ctx_t contextFromBytes(com::TStorage<uint8_t>& bytes)
{
    cmp_ctx_t cmpBuffer = {0};

    cmp_init(&cmpBuffer, bytes.data(), reader, skipper, writer);

    return cmpBuffer;
}

} // namespace dmit::cmp
