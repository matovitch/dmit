#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"

namespace dmit::sem
{

const com::UniqueId Context::ARGUMENT_OF {"@ArgumentOf"};
const com::UniqueId Context::DEFINE      {"@Define"};

Context::Context(ast::State::NodePool& astNodePool) :
    _astNodePool{astNodePool}
{}

} // namespace dmit::sem
