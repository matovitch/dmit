#include "dmit/prs/reader.hpp"

#include <functional>
#include <optional>
#include <cstdint>

namespace dmit
{

namespace prs
{

Reader::Reader(const state::Tree& tree) :
    _head{&(tree.nodes().back()) - 1},
    _tail{&(tree.nodes().front())}
{}

Reader::Reader(const state::tree::Node* const head,
               const state::tree::Node* const tail) :
    _head{head},
    _tail{tail}
{}

void Reader::advance()
{
    _head -= _head->_size + 1;
}

const state::tree::Node& Reader::look() const
{
    return *_head;
}

bool Reader::isValid() const
{
    return _head > _tail;
}

bool Reader::isValidNext() const
{
    return _head - _head->_size - 1 > _tail;
}

std::optional<Reader> Reader::makeSubReader() const
{
    if (_head->_size == 0)
    {
        return std::nullopt;
    }

    return Reader{ _head - 1, _head - _head->_size - 1};
}

uint32_t Reader::size() const
{
    uint32_t size = 1;

    auto head = _head - _head->_size - 1;

    while (head > _tail)
    {
        head -= head->_size + 1;
        size++;
    }

    return size;
}

} // namespace prs
} // namespace dmit
