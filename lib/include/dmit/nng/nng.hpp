#pragma once

#include "nng/nng.h"

#include <cstdint>

namespace dmit::nng
{

struct Socket
{
    ~Socket();

    nng_socket _asNng;
};

struct Buffer
{
    Buffer() = default;

    Buffer(const Buffer&) = delete;

    Buffer(Buffer&&);

    Buffer& operator=(Buffer&&);

    Buffer(size_t size);

    ~Buffer();

    void release();

    char*  _asBytes = nullptr;
    size_t _size;
};

} // namespace dmit::nng

NNG_DECL int nng_send(nng_socket socket, dmit::nng::Buffer* bufferPtr, int flags);

NNG_DECL int nng_recv(nng_socket socket, dmit::nng::Buffer* bufferPtr, int flags);
