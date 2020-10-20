#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/src/partition.hpp"

namespace dmit::sem
{

const com::UniqueId Context::ARGUMENT_OF {"@ArgumentOf"};
const com::UniqueId Context::DEFINE      {"@Define"};

Context::Context(const dmit::src::Partition& srcPartition,
                 ast::State::NodePool&       astNodePool) :
    _srcPartition{srcPartition},
    _astNodePool{astNodePool}
{}

} // namespace dmit::sem
