#pragma once

#include "nng/nng.h"

#include <cstdlib>

namespace nng
{

struct Socket
{
    ~Socket();

    nng_socket _asNng;
};

struct Buffer
{
    Buffer() = default;

    Buffer(size_t size);

    ~Buffer();

    void release();

    char*  _asBytes = nullptr;
    size_t _size;
};

Socket::~Socket()
{
    nng_close(_asNng);
}

Buffer::Buffer(size_t size) :
    _size{size}
{
    _asBytes = static_cast<char*>(nng_alloc(size));

    if(!_asBytes)
    {
        abort();
    }
}

void Buffer::release()
{
    _asBytes = nullptr;
}

Buffer::~Buffer()
{
    if (_asBytes)
    {
        nng_free(_asBytes, _size);
    }
}

} // namespace nng

NNG_DECL int nng_send(nng_socket socket, nng::Buffer* bufferPtr, int flags)
{
    int rc = nng_send(socket, bufferPtr->_asBytes, bufferPtr->_size, flags | NNG_FLAG_ALLOC);

    if (!rc)
    {
        bufferPtr->release();
    }

    return rc;
}

NNG_DECL int nng_recv(nng_socket socket, nng::Buffer* bufferPtr, int flags)
{
    return nng_recv(socket, &(bufferPtr->_asBytes), &(bufferPtr->_size), flags | NNG_FLAG_ALLOC);
}
