#include "dmit/prs/reader.hpp"

#include "dmit/prs/tree.hpp"

namespace dmit::prs
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

Reader Reader::makeSubReader() const
{
    return Reader{ _head - 1, _head - _head->_size - 1};
}

uint32_t Reader::size() const
{
    return (_head + 1)->_childCount;
}

} // namespace dmit::prs
