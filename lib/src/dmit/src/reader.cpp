#include "dmit/src/reader.hpp"

#include <cstdint>

namespace dmit::src
{

Reader::Reader(const uint8_t* const head,
               const uint8_t* const tail) :
    _head{head},
    _tail{tail}
{}

Reader::operator bool() const
{
    return _head < _tail;
}

void Reader::operator++()
{
    _head++;
}

const uint8_t Reader::operator*() const
{
    return *_head;
}

uint32_t Reader::offset() const
{
    return static_cast<uint32_t>(_tail - _head);
}

} // namespace dmit::src
