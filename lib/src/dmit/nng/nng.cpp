#include "dmit/nng/nng.hpp"

#include "nng/nng.h"

#include <cstdint>
#include <cstdlib>

namespace dmit::nng
{

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

Buffer::Buffer(Buffer&& toMove) :
    _asBytes {toMove._asBytes },
    _size    {toMove._size    }
{
    toMove.release();
}

Buffer& Buffer::operator=(Buffer&& toMove)
{
    _asBytes = toMove._asBytes;
    _size     =toMove._size;

    toMove.release();

    return *this;
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

} // namespace dmit::nng

int nng_send(nng_socket socket, dmit::nng::Buffer* bufferPtr, int flags)
{
    int rc = nng_send(socket, bufferPtr->_asBytes, bufferPtr->_size, flags | NNG_FLAG_ALLOC);

    if (!rc)
    {
        bufferPtr->release();
    }

    return rc;
}

int nng_recv(nng_socket socket, dmit::nng::Buffer* bufferPtr, int flags)
{
    return nng_recv(socket, &(bufferPtr->_asBytes), &(bufferPtr->_size), flags | NNG_FLAG_ALLOC);
}
