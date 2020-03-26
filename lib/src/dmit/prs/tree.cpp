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
           const uint32_t size) :
    _kind{kind},
    _size{size}
{}

Range::Range(const uint32_t start,
             const uint32_t stop) :
    _start{start},
    _stop{stop}
{}


} // namespace tree

Tree::Tree()
{
    _nodes.emplace_back(tree::node::Kind::END_OF_TREE, 0);
}

void Tree::clear()
{
    _nodes  .clear();
    _ranges .clear();

    _nodes.emplace_back(tree::node::Kind::END_OF_TREE, 0);
}

uint32_t Tree::size() const
{
    return _nodes.size();
}

void Tree::resize(const std::size_t size)
{
    _nodes  .resize(size);
    _ranges .resize(size - 1);
}

const std::vector<tree::Node>& Tree::nodes() const
{
    return _nodes;
}

const std::vector<tree::Range>& Tree::ranges() const
{
    return _ranges;
}

void Tree::addNode(const tree::node::Kind kind,
                   const uint32_t size,
                   const uint32_t start,
                   const uint32_t stop)
{
    _nodes  .emplace_back(kind, size);
    _ranges .emplace_back(start, stop);
}

const tree::Range& Tree::range(const tree::Node& node) const
{
    return _ranges[&node - _nodes.data() - 1];
}

} // namespace state
} // namespace prs
} // namespace dmit
