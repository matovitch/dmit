#include "dmit/prs/tree.hpp"

#include <cstdint>
#include <vector>

namespace dmit::prs::state
{

namespace tree
{

Node::Node(const node::Kind kind,
           const uint32_t size,
           const uint32_t childCount,
           const uint32_t start,
           const uint32_t stop) :
    _kind{kind},
    _size{size},
    _childCount{childCount},
    _start{start},
    _stop{stop}
{}

} // namespace tree

Tree::Tree()
{
    _nodes.emplace_back(tree::node::Kind::END_OF_TREE, 0, 0, 0, 0);
}

void Tree::clear()
{
    _nodes.clear();
    _nodes.emplace_back(tree::node::Kind::END_OF_TREE, 0, 0, 0, 0);
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

void Tree::addNode(const tree::node::Kind kind,
                   const uint32_t size,
                   const uint32_t childCount,
                   const uint32_t start,
                   const uint32_t stop)
{
    _nodes.emplace_back(kind, size, childCount, start, stop);
}

} // namespace dmit::prs::state
