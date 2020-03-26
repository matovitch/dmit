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
    addNode<tree::node::Arity::ONE,
            tree::node::Kind::END_OF_TREE>(0, 0, 0);
}

void Tree::clear()
{
    _nodes  .clear();
    _ranges .clear();

    addNode<tree::node::Arity::ONE,
            tree::node::Kind::END_OF_TREE>(0, 0, 0);
}

uint32_t Tree::size() const
{
    return _nodes.size();
}

void Tree::resize(const std::size_t size)
{
    _nodes  .resize(size);
    _ranges .resize(size);
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

} // namespace state
} // namespace prs
} // namespace dmit
