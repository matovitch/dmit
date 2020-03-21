#include "dmit/prs/tree.hpp"

namespace dmit
{

namespace prs
{

namespace state
{

namespace tree
{

Node::Node(const node::Kind  kind,
           const uint32_t size,
           const uint32_t start,
           const uint32_t stop) :
    _kind{kind},
    _size{size},
    _start{start},
    _stop{stop}
{}

} // namespace tree

void Tree::clear()
{
    _nodes.clear();
}

uint32_t Tree::size() const
{
    return _nodes.size();
}

void Tree::resize(const std::size_t size)
{
    _nodes.resize(size);
}

const std::vector<tree::Node>& Tree::nodes() const
{
    return _nodes;
}

} // namespace state
} // namespace prs
} // namespace dmit
