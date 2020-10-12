#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/src/partition.hpp"

namespace dmit::sem
{

Context::Context(const dmit::src::Partition& srcPartition,
                 ast::State::NodePool&       astNodePool) :
    _srcPartition{srcPartition},
    _astNodePool{astNodePool}
{}

} // namespace dmit::sem
