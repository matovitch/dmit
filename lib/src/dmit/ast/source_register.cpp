#include "dmit/ast/source_register.hpp"

#include <memory>

namespace dmit::ast
{

void SourceRegister::add(TNode<node::Kind::SOURCE>& source)
{
    new (&source) (TNode<node::Kind::SOURCE>)();

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
