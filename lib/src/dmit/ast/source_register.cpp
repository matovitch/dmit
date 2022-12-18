#include "dmit/ast/source_register.hpp"

#include <memory>

namespace dmit::ast
{

void SourceRegister::add(TNode<node::Kind::SOURCE>& source,
                         const std::filesystem::path& path,
                         const com::TStorage<uint8_t>& content)
{
    new (&source) (TNode<node::Kind::SOURCE>)(path, content);

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
