#include "dmit/prs/tree.hpp"

#include "dmit/prs/subscriber.hpp"
#include "dmit/prs/reader.hpp"
#include "dmit/prs/stack.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/enum.hpp"

#include <optional>
#include <cstdint>
#include <vector>

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

const std::vector<tree::Node>& Tree::nodes() const
{
    return _nodes;
}

void Tree::addNode(const tree::node::Arity arity,
                   const tree::node::Kind  kind,
                   const uint32_t size,
                   const uint32_t start,
                   const uint32_t stop)
{
    if (arity == tree::node::Arity::ONE || _nodes.back()._size < size - 1)
    {
        _nodes.emplace_back(kind, size, start, stop);
    }
}

} // namespace state

namespace subscriber
{

namespace tree
{

Writer::Writer(const com::TEnumIntegerType<NodeKind> nodeKind,
               const com::TEnumIntegerType<NodeArity> nodeArity) :
    _nodeKind  {nodeKind},
    _nodeArity {nodeArity}
{}

void Writer::onStart(const Reader& reader, Stack& stack, State& state) const
{
    stack._treeSize     = state._tree.size();
    stack._readerOffset = reader.offset(); 
}

void Writer::onEnd(const std::optional<Reader>& readerOpt, const Stack& stack, State& state) const
{
    if (!readerOpt)
    {
        return;
    }

    const auto size = state._tree.size() - stack._treeSize;

    state._tree.addNode(_nodeArity,
                        _nodeKind,
                        size,
                        stack._readerOffset,
                        readerOpt.value().offset());
}

} // namespace tree

} // namespace subscriber

} // namespace prs

} // namespace dmit
