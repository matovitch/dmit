#include "dmit/ast/source_register.hpp"

#include <type_traits>
#include <memory>

namespace dmit::ast
{

void SourceRegister::add(TNode<node::Kind::SOURCE>& source)
{
    new (&source) (std::decay_t<decltype(source)>)();

    _sources.push_back(&source);
}

SourceRegister::~SourceRegister()
{
    for (auto source : _sources)
    {
        std::destroy_at(source);
    }
}

} // namespace dmit::ast
